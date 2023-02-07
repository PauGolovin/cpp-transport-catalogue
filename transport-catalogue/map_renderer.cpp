#include "map_renderer.h"

using namespace std::literals;
using namespace transport_catalogue;

namespace renderer {
    void MapRenderer::SetMapRenderer(const json::Node& render_settings) {
        if (render_settings.IsNull()) return;
        const json::Dict& settings_map = render_settings.AsMap();
        width_ = settings_map.at("width"s).AsDouble();
        height_ = settings_map.at("height"s).AsDouble();
        padding_ = settings_map.at("padding"s).AsDouble();
        stop_radius_ = settings_map.at("stop_radius"s).AsDouble();
        line_width_ = settings_map.at("line_width"s).AsDouble();
        bus_label_font_size_ = settings_map.at("bus_label_font_size"s).AsInt();
        const json::Array& bus_label_offset = settings_map.at("bus_label_offset"s).AsArray();
        bus_label_offset_ = { bus_label_offset[0].AsDouble(),
                              bus_label_offset[1].AsDouble() };
        stop_label_font_size_ = settings_map.at("stop_label_font_size"s).AsInt();
        const json::Array& stop_label_offset = settings_map.at("stop_label_offset"s).AsArray();
        stop_label_offset_ = { stop_label_offset[0].AsDouble(),
                               stop_label_offset[1].AsDouble() };
        if (settings_map.at("underlayer_color"s).IsArray()) {
            const json::Array& arr = settings_map.at("underlayer_color"s).AsArray();
            if (arr.size() == 3) {
                svg::Rgb rgb_colors(arr[0].AsInt(), arr[1].AsInt(), arr[2].AsInt());
                underlayer_color_ = rgb_colors;
            }
            else if (arr.size() == 4) {
                svg::Rgba rgba_colors(arr[0].AsInt(), arr[1].AsInt(), arr[2].AsInt(), arr[3].AsDouble());
                underlayer_color_ = rgba_colors;
            }
            else throw std::logic_error("Color error"s);
        }
        else if (settings_map.at("underlayer_color"s).IsString()) {
            underlayer_color_ = settings_map.at("underlayer_color"s).AsString();
        }
        else throw std::logic_error("Color error"s);
        underlayer_width_ = settings_map.at("underlayer_width"s).AsDouble();
        const json::Array& color_palette = settings_map.at("color_palette"s).AsArray();
        for (const json::Node& node : color_palette) {
            if (node.IsArray()) {
                const json::Array& arr = node.AsArray();
                if (arr.size() == 3) {
                    svg::Rgb rgb_colors(arr[0].AsInt(), arr[1].AsInt(), arr[2].AsInt());
                    color_palette_.push_back(rgb_colors);
                }
                else if (arr.size() == 4) {
                    svg::Rgba rgba_colors(arr[0].AsInt(), arr[1].AsInt(), arr[2].AsInt(), arr[3].AsDouble());
                    color_palette_.push_back(rgba_colors);
                }
                else throw std::logic_error("Color error"s);
            }
            else if (node.IsString()) {
                color_palette_.push_back(node.AsString());
            }
            else throw std::logic_error("Color error"s);
        }
    }
    std::vector<svg::Polyline> MapRenderer::GetBusLines(const std::vector<const TransportCatalogue::Bus*>& buses, const SphereProjector& sp) const {
        std::vector<svg::Polyline> result;
        unsigned color_num = 0;
        for (const auto& bus_ptr : buses) {
            if (bus_ptr->stops.size() == 0) continue;
            svg::Polyline line;
            for (auto stop : bus_ptr->stops) {
                line.AddPoint(sp(stop->coordinates));
            }
            line.SetFillColor("none"s);
            line.SetStrokeColor(color_palette_[color_num]);
            line.SetStrokeWidth(line_width_);
            line.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            line.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            if (color_num < (color_palette_.size() - 1)) {
                ++color_num;
            }
            else color_num = 0;
            result.push_back(line);
        }
        return result;
    }
    std::vector<svg::Text> MapRenderer::GetBusNames(const std::vector<const TransportCatalogue::Bus*>& buses, const SphereProjector& sp) const {
        std::vector<svg::Text> result;
        unsigned color_num = 0;
        for (auto& bus_ptr : buses) {
            if (bus_ptr->stops.size() == 0) continue;
            svg::Text text_underlayer;
            svg::Text text;
            text_underlayer.SetData(bus_ptr->name);
            text.SetData(bus_ptr->name);
            text.SetFillColor(color_palette_[color_num]);
            if (color_num < (color_palette_.size() - 1)) {
                ++color_num;
            }
            else {
                color_num = 0;
            }
            text_underlayer.SetFillColor(underlayer_color_);
            text_underlayer.SetStrokeColor(underlayer_color_);
            text.SetFontFamily("Verdana"s);
            text_underlayer.SetFontFamily("Verdana"s);
            text.SetFontSize(bus_label_font_size_);
            text_underlayer.SetFontSize(bus_label_font_size_);
            text.SetFontWeight("bold"s);
            text_underlayer.SetFontWeight("bold"s);
            text.SetOffset(bus_label_offset_);
            text_underlayer.SetOffset(bus_label_offset_);
            text_underlayer.SetStrokeWidth(underlayer_width_);
            text_underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            text_underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            text.SetPosition(sp(bus_ptr->stops[0]->coordinates));
            text_underlayer.SetPosition(sp(bus_ptr->stops[0]->coordinates));
            result.push_back(text_underlayer);
            result.push_back(text);

            if (!bus_ptr->is_roundtrip) {
                size_t final_stop_num = (bus_ptr->stops.size() - 1) / 2;
                if (bus_ptr->stops[0]->name != bus_ptr->stops[final_stop_num]->name) {
                    svg::Text text2 = text;
                    svg::Text text2_underlayer = text_underlayer;
                    text2.SetPosition(sp(bus_ptr->stops[final_stop_num]->coordinates));
                    text2_underlayer.SetPosition(sp(bus_ptr->stops[final_stop_num]->coordinates));
                    result.push_back(text2_underlayer);
                    result.push_back(text2);
                }
            }
        }
        return result;
    }
    std::vector<svg::Circle> MapRenderer::GetStopCircles(const std::map<std::string_view, const TransportCatalogue::Stop*>& stops, const SphereProjector& sp) const {
        std::vector<svg::Circle> result;
        for (auto& [stop_name, stop_ptr] : stops) {
            svg::Circle circle;
            circle.SetCenter(sp(stop_ptr->coordinates));
            circle.SetRadius(stop_radius_);
            circle.SetFillColor("white"s);
            result.push_back(circle);
        }
        return result;
    }
    std::vector<svg::Text> MapRenderer::GetStopNames(const std::map<std::string_view, const TransportCatalogue::Stop*>& stops, const SphereProjector& sp) const {
        std::vector<svg::Text> result;
        for (auto& [stop_name, stop_ptr] : stops) {
            svg::Text text, text_underlayer;
            text.SetPosition(sp(stop_ptr->coordinates));
            text.SetOffset(stop_label_offset_);
            text.SetFontSize(stop_label_font_size_);
            text.SetFontFamily("Verdana"s);
            text.SetData(stop_ptr->name);
            text.SetFillColor("black"s);
            text_underlayer.SetPosition(sp(stop_ptr->coordinates));
            text_underlayer.SetOffset(stop_label_offset_);
            text_underlayer.SetFontSize(stop_label_font_size_);
            text_underlayer.SetFontFamily("Verdana"s);
            text_underlayer.SetData(stop_ptr->name);
            text_underlayer.SetFillColor(underlayer_color_);
            text_underlayer.SetStrokeColor(underlayer_color_);
            text_underlayer.SetStrokeWidth(underlayer_width_);
            text_underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            text_underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            result.push_back(text_underlayer);
            result.push_back(text);
        }
        return result;
    }

    svg::Document MapRenderer::GetSvgDocument() const {
        const std::vector<const TransportCatalogue::Bus*> buses = tc_.GetAllBuses();
        std::map<std::string_view, const TransportCatalogue::Stop*> all_stops;
        std::vector<geo::Coordinates> all_coords;
        svg::Document result;
        for (const auto& bus_ptr : buses) {
            if (bus_ptr->stops.size() == 0) continue;
            for (const auto stop : bus_ptr->stops) {
                all_stops[stop->name] = stop;
                all_coords.push_back(stop->coordinates);
            }
        }
        SphereProjector sp(all_coords.begin(), all_coords.end(), width_, height_, padding_);
        for (const auto& line : GetBusLines(buses, sp)) {
            result.Add(line);
        }
        for (const auto& name : GetBusNames(buses, sp)) {
            result.Add(name);
        }
        for (const auto& circle : GetStopCircles(all_stops, sp)) {
            result.Add(circle);
        }
        for (const auto& name : GetStopNames(all_stops, sp)) {
            result.Add(name);
        }
        return result;
    }

    void MapRenderer::PrintSvgDocument(std::ostream& os) const {
        svg::Document doc = GetSvgDocument();
        doc.Render(os);
    }


    bool IsZero(double value) {
        return std::abs(value) < EPSILON;
    }

    svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
        return { (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                (max_lat_ - coords.lat) * zoom_coeff_ + padding_ };
    }
}