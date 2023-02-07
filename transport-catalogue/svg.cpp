#include "svg.h"

namespace svg {

    using namespace std::literals;

    Rgb::Rgb(uint8_t r, uint8_t g, uint8_t b)
        : red(r)
        , green(g)
        , blue(b)
    {}
    Rgba::Rgba(uint8_t r, uint8_t g, uint8_t b, double op)
        : red(r)
        , green(g)
        , blue(b)
        , opacity(op)
    {}

    void OstreamColorPrinter::operator()(std::monostate) const {
        using namespace std::literals;
        out << "none"s;
    }
    void OstreamColorPrinter::operator()(std::string color) const {
        out << color;
    }
    void OstreamColorPrinter::operator()(Rgb color) const {
        using namespace std::literals;
        out << "rgb("s << std::to_string(color.red) << ","s <<
            std::to_string(color.green) << ","s <<
            std::to_string(color.blue) << ")"s;
    }
    void OstreamColorPrinter::operator()(Rgba color) const {
        using namespace std::literals;
        out << "rgba("s << std::to_string(color.red) << ","s <<
            std::to_string(color.green) << ","s <<
            std::to_string(color.blue) << ","s <<
            color.opacity << ")"s;
    }

    std::ostream& operator<<(std::ostream& os, const Color& color) {
        std::visit(OstreamColorPrinter{ os }, color);
        return os;
    }

    std::ostream& operator<<(std::ostream& os, StrokeLineCap line_cap) {
        using namespace std::literals;
        switch (line_cap) {
        case StrokeLineCap::BUTT:
            os << "butt"s;
            break;
        case StrokeLineCap::ROUND:
            os << "round"s;
            break;
        case StrokeLineCap::SQUARE:
            os << "square"s;
            break;
        }
        return os;
    }

    std::ostream& operator<<(std::ostream& os, StrokeLineJoin line_join) {
        using namespace std::literals;
        switch (line_join) {
        case StrokeLineJoin::ARCS:
            os << "arcs"s;
            break;
        case StrokeLineJoin::BEVEL:
            os << "bevel"s;
            break;
        case StrokeLineJoin::MITER:
            os << "miter"s;
            break;
        case StrokeLineJoin::MITER_CLIP:
            os << "miter-clip"s;
            break;
        case StrokeLineJoin::ROUND:
            os << "round"s;
            break;
        }
        return os;
    }

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\" "sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    // ---------- Polyline ----------------

    Polyline& Polyline::AddPoint(Point point) {
        points_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        bool is_first = true;
        for (const auto& point : points_) {
            if (!is_first) {
                out << " "sv;
            }
            is_first = false;
            out << point.x << ","sv << point.y;
        }
        out << "\" "sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    // ---------- Text --------------------

    Text& Text::SetPosition(Point pos) {
        position_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) {
        size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = font_family;
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = font_weight;
        return *this;
    }

    Text& Text::SetData(std::string data) {
        data_ = data;
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text x=\""sv << position_.x << "\" y=\""sv << position_.y << "\" "sv;
        out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv;
        out << "font-size=\""sv << size_ << "\""sv;
        if (font_family_) {
            out << " font-family=\""sv << *font_family_ << "\""sv;
        }
        if (font_weight_) {
            out << " font-weight=\""sv << *font_weight_ << "\""sv;
        }
        RenderAttrs(context.out);
        out << ">"sv << data_ << "</text>"sv;
    }

    // ---------- Document ----------------

    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.push_back(std::move(obj));
    }

    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
        for (const auto& obj : objects_) {
            obj.get()->Render(out);
        }
        out << "</svg>"sv << std::endl;
    }

}  // namespace svg