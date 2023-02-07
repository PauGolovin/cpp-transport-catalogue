#include "json.h"

using namespace std;

namespace json {

    // ----- Load-Funcs -----
    namespace {

        Node LoadNode(istream& input);

        std::string LoadWord(istream& input) {
            std::string str;
            while (std::isalpha(input.peek())) {
                str.push_back(static_cast<char>(input.get()));
            }
            return str;
        }

        Node LoadArray(std::istream& input) {
            std::vector<Node> result;
            for (char c; input >> c && c != ']';) {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }
            if (!input) {
                throw ParsingError("Array parsing error"s);
            }
            return Node{ std::move(result) };
        }

        Node LoadString(std::istream& input) {
            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    ++it;
                    break;
                }
                else if (ch == '\\') {
                    ++it;
                    if (it == end) {
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    switch (escaped_char) {
                    case 'n':
                        s.push_back('\n');
                        break;
                    case 't':
                        s.push_back('\t');
                        break;
                    case 'r':
                        s.push_back('\r');
                        break;
                    case '"':
                        s.push_back('"');
                        break;
                    case '\\':
                        s.push_back('\\');
                        break;
                    default:
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                }
                else if (ch == '\n' || ch == '\r') {
                    throw ParsingError("Unexpected end of line"s);
                }
                else {
                    s.push_back(ch);
                }
                ++it;
            }
            return Node{ std::move(s) };
        }

        Node LoadDict(std::istream& input) {
            using namespace std::literals;
            Dict result;
            for (char ch; input >> ch && ch != '}';) {
                if (ch == '"') {
                    std::string key = LoadString(input).AsString();
                    if (input >> ch && ch == ':') {
                        if (result.find(key) != result.end()) {
                            throw ParsingError("This key is already used "s + key);
                        }
                        result.emplace(key, LoadNode(input));
                    }
                    else {
                        throw ParsingError("Error of map building. See key "s + key);
                    }
                }
                else if (ch != ',') {
                    throw ParsingError("Error of map building. Expected \",\"."s);
                }
            }
            if (!input) {
                throw ParsingError("Dict parsu-ing error."s);
            }
            return Node{ std::move(result) };
        }

        Node LoadNumber(std::istream& input) {
            using namespace std::literals;

            std::string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek())) {
                    read_char();
                }
            };

            if (input.peek() == '-') {
                read_char();
            }
            // Парсим целую часть числа
            if (input.peek() == '0') {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            }
            else {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
            if (int ch = input.peek(); ch == 'e' || ch == 'E') {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    // Сначала пробуем преобразовать строку в int
                    try {
                        return Node{ std::stoi(parsed_num) };
                    }
                    catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return Node{ std::stod(parsed_num) };
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        Node LoadBool(istream& input) {
            using namespace std::literals;
            std::string str = LoadWord(input);
            if (str == "true"s) {
                return Node{ true };
            }
            if (str == "false"s) {
                return Node{ false };
            }
            throw ParsingError("Failed to convert "s + str + " to bool"s);
        }

        Node LoadNull(istream& input) {
            using namespace std::literals;
            std::string str = LoadWord(input);
            if (str == "null"s) {
                return Node();
            }
            throw ParsingError("Failed to convert "s + str + " to null"s);
        }

        Node LoadNode(std::istream& input) {
            char c;
            if (!(input >> c)) {
                throw ParsingError("Unexpected EOF"s);
            }
            switch (c) {
            case '[':
                return LoadArray(input);
            case '{':
                return LoadDict(input);
            case '"':
                return LoadString(input);
            case 't':
                input.putback(c);
                return LoadBool(input);
            case 'f':
                input.putback(c);
                return LoadBool(input);
            case 'n':
                input.putback(c);
                return LoadNull(input);
            default:
                input.putback(c);
                return LoadNumber(input);
            }
        }

    }  // namespace

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    struct OstreamNodesPrinter {
        std::ostream& out;
        int& tab;
        void operator()(std::nullptr_t) {
            out << "null"s;
        }
        void operator()(const int node) {
            out << node;
        }
        void operator()(const double node) {
            out << node;
        }
        void operator()(const std::string& node) {
            out << "\""s;
            for (auto it = node.begin(); it != node.end(); ++it) {
                switch (*it)
                {
                case '\r':
                    out << "\\r"sv;
                    break;
                case '\n':
                    out << "\\n"sv;
                    break;
                case '"':
                    out << "\\\""sv;
                    break;
                case '\\':
                    out << "\\\\"sv;
                    break;
                default:
                    out << *it;
                    break;
                }
            }
            out << "\""s;
        }
        void operator()(bool node) {
            out << boolalpha << node;
        }
        void operator()(const Array& node) {
            out << "["s << endl;
            ++tab;
            bool is_first = true;
            for (const auto& value : node) {
                if (!is_first) {
                    out << ",\n";
                }
                is_first = false;
                for (int i = 0; i < tab; ++i) {
                    out << "\t"s;
                }
                std::visit(OstreamNodesPrinter{ out, tab }, value.GetValue());
            }
            --tab;
            out << "\n"s;
            for (int i = 0; i < tab; ++i) {
                out << "\t"s;
            }
            if (tab == 0) {
                out << "]\n"s;
            }
            else {
                out << "]"s;
            }
        }
        void operator()(const Dict& node) {
            out << "{"s << endl;
            ++tab;
            bool is_first = true;
            for (const auto& [key, value] : node) {
                if (!is_first) {
                    out << ",\n";
                }
                is_first = false;
                for (int i = 0; i < tab; ++i) {
                    out << "\t"s;
                }
                out << "\""s << key << "\" : "s;
                std::visit(OstreamNodesPrinter{ out, tab }, value.GetValue());
            }
            --tab;
            out << "\n"s;
            for (int i = 0; i < tab; ++i) {
                out << "\t"s;
            }
            if (tab == 0) {
                out << "}\n"s;
            }
            else {
                out << "}"s;
            }
        }
    };

    void Print(const Document& doc, std::ostream& output) {
        int tab = 0;
        std::visit(OstreamNodesPrinter{ output, tab }, doc.GetRoot().GetValue());
    }

}  // namespace json