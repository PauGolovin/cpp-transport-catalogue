#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

    class Node;
    // Сохраните объявления Dict и Array без изменения
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

    class Node final : private Value {
    public:
        /* Реализуйте Node, используя std::variant */
        using variant::variant;

        // ----- Constructors -----
        // 
        // ----- Is-Funcs -----
        bool IsInt() const {
            return std::holds_alternative<int>(*this);
        }
        bool IsPureDouble() const {
            return std::holds_alternative<double>(*this);
        }
        bool IsDouble() const {
            return IsInt() || IsPureDouble();
        }
        bool IsBool() const {
            return std::holds_alternative<bool>(*this);
        }
        bool IsNull() const {
            return std::holds_alternative<std::nullptr_t>(*this);
        }
        bool IsArray() const {
            return std::holds_alternative<Array>(*this);
        }
        bool IsString() const {
            return std::holds_alternative<std::string>(*this);
        }
        bool IsMap() const {
            return std::holds_alternative<Dict>(*this);
        }
        // ----- As-Funcs -----

        int AsInt() const {
            using namespace std::literals;
            if (!IsInt()) {
                throw std::logic_error("Node isn't int"s);
            }
            return std::get<int>(*this);
        }
        double AsDouble() const {
            using namespace std::literals;
            if (!IsDouble()) {
                throw std::logic_error("Node isn't double"s);
            }
            return IsPureDouble() ? std::get<double>(*this) : AsInt();
        }
        bool AsBool() const {
            using namespace std::literals;
            if (!IsBool()) {
                throw std::logic_error("Node isn't bool"s);
            }
            return std::get<bool>(*this);
        }
        const Array& AsArray() const {
            using namespace std::literals;
            if (!IsArray()) {
                throw std::logic_error("Node isn't array"s);
            }

            return std::get<Array>(*this);
        }
        const std::string& AsString() const {
            using namespace std::literals;
            if (!IsString()) {
                throw std::logic_error("Node isn't string"s);
            }
            return std::get<std::string>(*this);
        }
        const Dict& AsMap() const {
            using namespace std::literals;
            if (!IsMap()) {
                throw std::logic_error("Node isn't dict"s);
            }
            return std::get<Dict>(*this);
        }
        // -----
        bool operator==(const Node& rhs) const {
            return GetValue() == rhs.GetValue();
        }
        // -----
        const Value& GetValue() const {
            return *this;
        }
    };

    inline bool operator!=(const Node& lhs, const Node& rhs) {
        return !(lhs == rhs);
    }

    class Document {
    public:
        explicit Document(Node root)
            : root_(std::move(root)) {
        }

        const Node& GetRoot() const {
            return root_;
        }

    private:
        Node root_;
    };

    inline bool operator==(const Document& lhs, const Document& rhs) {
        return lhs.GetRoot() == rhs.GetRoot();
    }

    inline bool operator!=(const Document& lhs, const Document& rhs) {
        return !(lhs == rhs);
    }

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

}  // namespace json