#pragma once

#include <cstdint>
#include <iostream>
#include <sstream>
#include <memory>
#include <string>
#include <vector>
#include <string_view>
#include <optional>
#include <variant>

namespace svg {

    struct Point {
        Point() = default;
        Point(double x, double y)
            : x(x)
            , y(y) {
        }
        double x = 0;
        double y = 0;
    };

    // ------- Color -------

    struct Rgb {
        Rgb() = default;
        Rgb(uint8_t r, uint8_t g, uint8_t b);
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };
    struct Rgba {
        Rgba() = default;
        Rgba(uint8_t r, uint8_t g, uint8_t b, double op);
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        double opacity = 1.0;
    };

    using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
    inline const Color NoneColor{ "none" };

    struct OstreamColorPrinter {
        std::ostream& out;

        void operator()(std::monostate) const;
        void operator()(std::string color) const;
        void operator()(Rgb color) const;
        void operator()(Rgba color) const;
    };

    std::ostream& operator<<(std::ostream& os, const Color& color);

    // ------- Props -------

    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };
    std::ostream& operator<<(std::ostream& os, StrokeLineCap line_cap);

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };
    std::ostream& operator<<(std::ostream& os, StrokeLineJoin line_join);

    template <typename Owner>
    class PathProps {
    public:
        Owner& SetFillColor(Color color) {
            fill_color_ = std::move(color);
            return AsOwner();
        }
        Owner& SetStrokeColor(Color color) {
            stroke_color_ = std::move(color);
            return AsOwner();
        }
        Owner& SetStrokeWidth(double width) {
            stroke_width_ = width;
            return AsOwner();
        }
        Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
            stroke_linecap_ = std::move(line_cap);
            return AsOwner();
        }
        Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
            stroke_linejoin_ = std::move(line_join);
            return AsOwner();
        }

    protected:
        virtual ~PathProps() = default;

        void RenderAttrs(std::ostream& out) const {
            using namespace std::literals;
            if (fill_color_) {
                std::ostringstream strm;
                std::visit(OstreamColorPrinter{ strm }, *fill_color_);
                out << " fill=\""sv << strm.str() << "\""sv;
            }
            if (stroke_color_) {
                std::ostringstream strm;
                std::visit(OstreamColorPrinter{ strm }, *stroke_color_);
                out << " stroke=\""sv << strm.str() << "\""sv;
            }
            if (stroke_width_) {
                out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
            }
            if (stroke_linecap_) {
                out << " stroke-linecap=\""sv << *stroke_linecap_ << "\""sv;
            }
            if (stroke_linecap_) {
                out << " stroke-linejoin=\""sv << *stroke_linejoin_ << "\""sv;
            }
        }

    private:
        Owner& AsOwner() {
            return static_cast<Owner&>(*this);
        }

        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> stroke_width_;
        std::optional<StrokeLineCap> stroke_linecap_;
        std::optional<StrokeLineJoin> stroke_linejoin_;
    };

    /*
     * ?????????????????????????????? ??????????????????, ???????????????? ???????????????? ?????? ???????????? SVG-?????????????????? ?? ??????????????????.
     * ???????????? ???????????? ???? ?????????? ????????????, ?????????????? ???????????????? ?? ?????? ?????????????? ?????? ???????????? ????????????????
     */
    struct RenderContext {
        RenderContext(std::ostream& out)
            : out(out) {
        }

        RenderContext(std::ostream& out, int indent_step, int indent = 0)
            : out(out)
            , indent_step(indent_step)
            , indent(indent) {
        }

        RenderContext Indented() const {
            return { out, indent_step, indent + indent_step };
        }

        void RenderIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };

    /*
     * ?????????????????????? ?????????????? ?????????? Object ???????????? ?????? ???????????????????????????????? ????????????????
     * ???????????????????? ?????????? SVG-??????????????????
     * ?????????????????? ?????????????? "?????????????????? ??????????" ?????? ???????????? ?????????????????????? ????????
     */
    class Object {
    public:
        void Render(const RenderContext& context) const;

        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };

    // ----- ObjectContainer -----

    class ObjectContainer {
    public:
        virtual ~ObjectContainer() = default;
        template<typename T>
        void Add(const T& object) {
            std::unique_ptr<T> obj = std::make_unique<T>(object);
            AddPtr(std::move(obj));
        }

        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
    };

    // -------- Drawable ---------

    class Drawable {
    public:
        virtual ~Drawable() = default;
        virtual void Draw(ObjectContainer& container) const = 0;
    };


    /*
     * ?????????? Circle ???????????????????? ?????????????? <circle> ?????? ?????????????????????? ??????????
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
     */
    class Circle final : public Object, public PathProps<Circle> {
    public:
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point center_;
        double radius_ = 1.0;
    };

    /*
     * ?????????? Polyline ???????????????????? ?????????????? <polyline> ?????? ?????????????????????? ?????????????? ??????????
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
     */
    class Polyline : public Object, public PathProps<Polyline> {
    public:
        // ?????????????????? ?????????????????? ?????????????? ?? ?????????????? ??????????
        Polyline& AddPoint(Point point);

    private:
        void RenderObject(const RenderContext& context) const override;
        std::vector<Point> points_;
    };

    /*
     * ?????????? Text ???????????????????? ?????????????? <text> ?????? ?????????????????????? ????????????
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
     */
    class Text : public Object, public PathProps<Text> {
    public:
        // ???????????? ???????????????????? ?????????????? ?????????? (???????????????? x ?? y)
        Text& SetPosition(Point pos);

        // ???????????? ???????????????? ???????????????????????? ?????????????? ?????????? (???????????????? dx, dy)
        Text& SetOffset(Point offset);

        // ???????????? ?????????????? ???????????? (?????????????? font-size)
        Text& SetFontSize(uint32_t size);

        // ???????????? ???????????????? ???????????? (?????????????? font-family)
        Text& SetFontFamily(std::string font_family);

        // ???????????? ?????????????? ???????????? (?????????????? font-weight)
        Text& SetFontWeight(std::string font_weight);

        // ???????????? ?????????????????? ???????????????????? ?????????????? (???????????????????????? ???????????? ???????? text)
        Text& SetData(std::string data);

    private:
        void RenderObject(const RenderContext& context) const override;
        Point position_;
        Point offset_;
        uint32_t size_ = 1;
        std::optional<std::string> font_family_;
        std::optional<std::string> font_weight_;
        std::string data_ = "";
    };

    class Document : public ObjectContainer {
    public:
        // ?????????????????? ?? svg-???????????????? ????????????-?????????????????? svg::Object
        void AddPtr(std::unique_ptr<Object>&& obj);

        // ?????????????? ?? ostream svg-?????????????????????????? ??????????????????
        void Render(std::ostream& out) const;

        // ???????????? ???????????? ?? ????????????, ?????????????????????? ?????? ???????????????????? ???????????? Document
    private:
        std::vector<std::unique_ptr<Object>> objects_;
    };

}  // namespace svg