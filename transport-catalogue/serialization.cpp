#include "serialization.h"

using namespace std;

namespace Serialization {
	using Path = filesystem::path;

	void Serialize(const transport_catalogue::TransportCatalogue& tc,
		const renderer::MapRenderer& mr,
		const transport_router::Router& router,
		std::string_view path_s) {

		Path path(path_s);
		ofstream out(path, ios::binary);

		serialize::TransportCatalogue serialized_tc;
		detail::SerializeTransportCatalogue(tc, serialized_tc);
		detail::SerializeMapRenderer(mr, serialized_tc);
		detail::SerializeRouter(router, serialized_tc);

		serialized_tc.SerializeToOstream(&out);
	}

	void Deserialize(transport_catalogue::TransportCatalogue& tc,
		renderer::MapRenderer& mr,
		transport_router::Router& router,
		std::string_view path_s) {

		Path path(path_s);
		ifstream in(path, ios::binary);
		if (!in) {
			throw;
		}

		serialize::TransportCatalogue deserialized_tc;
		if (!deserialized_tc.ParseFromIstream(&in)) {
			throw;
		}
		detail::DeserializeTransportCatalogue(tc, deserialized_tc);
		detail::DeserializeMapRenderer(mr, deserialized_tc);
		detail::DeserializeRouter(router, deserialized_tc);
	}


	namespace detail {
		void SerializeTransportCatalogue(const transport_catalogue::TransportCatalogue& tc, serialize::TransportCatalogue& serialized_tc) {

			auto stops = tc.GetAllStops();
			auto buses = tc.GetAllBuses_not_sorted();
			auto distances = tc.GetDistancesToSerialization();

			// сериализуем остановки
			for (const auto& stop : stops) {
				// сериализуем остановку
				serialize::Stop* serialized_stop = serialized_tc.add_stop();
				// записываем имя
				*(serialized_stop->mutable_name()) = stop->name;
				// записываем координаты
				serialized_stop->add_coordinate(stop->coordinates.lat);
				serialized_stop->add_coordinate(stop->coordinates.lng);
				// записываем расстояния до остановок
				if (distances.count(stop)) {
					for (const auto& [stop_to, distance] : distances.at(stop)) {
						serialized_stop->add_near_stop(stop_to->name);
						serialized_stop->add_distance(distance);
					}
				}
			}

			// сериализуем автобусы
			for (const auto& bus : buses) {
				// сериализуем автобус
				serialize::Bus* serialized_bus = serialized_tc.add_bus();
				// записываем имя
				*(serialized_bus->mutable_name()) = bus->name;
				// записываем остановки
				for (const auto& stop : bus->stops) {
					serialized_bus->add_stop(stop->name);
				}
				// записываем тип маршрута
				serialized_bus->set_is_roundtrip(bus->is_roundtrip);
			}
		}//-----

		void DeserializeTransportCatalogue(transport_catalogue::TransportCatalogue& tc, serialize::TransportCatalogue& deserialized_tc) {

			// читаем остановки
			for (const auto& dstop : *deserialized_tc.mutable_stop()) {
				tc.AddStop(dstop.name(), { dstop.coordinate(0), dstop.coordinate(1) });
				// читаем расстояния
				for (int i = 0; i < dstop.near_stop_size(); ++i) {
					tc.SetDistance(dstop.name(), dstop.near_stop(i), dstop.distance(i));
				}
			}
			// читаем автобусы
			for (const auto& dbus : *deserialized_tc.mutable_bus()) {
				std::string name = dbus.name();
				std::vector<std::string> stops;
				bool is_roundtrip = dbus.is_roundtrip();
				for (const auto& stop : dbus.stop()) {
					stops.push_back(stop);
				}
				tc.AddBus(name, stops, is_roundtrip);
			}
		}//-----

		void SerializeColor(const json::Node& color_settings, serialize::Color* color) {
			if (color_settings.IsArray()) {
				const json::Array& arr = color_settings.AsArray();
				if (arr.size() == 3) {
					serialize::RGB* rgb = color->mutable_rgb();
					rgb->set_red(arr[0].AsInt());
					rgb->set_green(arr[1].AsInt());
					rgb->set_blue(arr[2].AsInt());
				}
				else if (arr.size() == 4) {
					serialize::RGBA* rgba = color->mutable_rgba();
					rgba->set_red(arr[0].AsInt());
					rgba->set_green(arr[1].AsInt());
					rgba->set_blue(arr[2].AsInt());
					rgba->set_opacity(arr[3].AsDouble());
				}
			}
			else {
				color->set_name(color_settings.AsString());
			}
		}

		void SerializeMapRenderer(const renderer::MapRenderer& mr, serialize::TransportCatalogue& serialized_tc) {
			auto render_settings_ = mr.GetRenderSettings();
			serialize::RenderSettings* render_settings = serialized_tc.mutable_render_settings();
			// записываем скалярные типы настроек
			render_settings->set_width(render_settings_.AsDict().at("width").AsDouble());
			render_settings->set_height(render_settings_.AsDict().at("height").AsDouble());
			render_settings->set_padding(render_settings_.AsDict().at("padding").AsDouble());
			render_settings->set_stop_radius(render_settings_.AsDict().at("stop_radius").AsDouble());
			render_settings->set_line_width(render_settings_.AsDict().at("line_width").AsDouble());
			render_settings->set_bus_label_font_size(render_settings_.AsDict().at("bus_label_font_size").AsInt());
			render_settings->set_stop_label_font_size(render_settings_.AsDict().at("stop_label_font_size").AsInt());
			render_settings->set_underlayer_width(render_settings_.AsDict().at("underlayer_width").AsDouble());
			// записываем оффсеты
			serialize::Point* bus_label_offset = render_settings->mutable_bus_label_offset();
			serialize::Point* stop_label_offset = render_settings->mutable_stop_label_offset();

			bus_label_offset->set_x(render_settings_.AsDict().at("bus_label_offset"s).AsArray()[0].AsDouble());
			bus_label_offset->set_y(render_settings_.AsDict().at("bus_label_offset"s).AsArray()[1].AsDouble());
			stop_label_offset->set_x(render_settings_.AsDict().at("stop_label_offset"s).AsArray()[0].AsDouble());
			stop_label_offset->set_y(render_settings_.AsDict().at("stop_label_offset"s).AsArray()[1].AsDouble());
			// записываем цвета
			serialize::Color* underlayer_color = render_settings->mutable_underlayer_color();
			SerializeColor(render_settings_.AsDict().at("underlayer_color"s), underlayer_color);

			const json::Array& color_palette = render_settings_.AsDict().at("color_palette"s).AsArray();
			for (const json::Node& node : color_palette) {
				serialize::Color* color = render_settings->add_color_palette();
				SerializeColor(node, color);
			}
		}//-----

		json::Node PointToNode(const svg::Point& p) {
			return json::Node(json::Array{ {p.x}, {p.y} });
		}

		json::Node ColorToNode(const serialize::Color& color) {
			if (color.has_rgb()) {
				return json::Node(json::Array{ {color.rgb().red()}, {color.rgb().green()}, {color.rgb().blue()} });
			}
			else if (color.has_rgba()) {
				return json::Node(json::Array{ {color.rgba().red()}, {color.rgba().green()}, {color.rgba().blue()}, {color.rgba().opacity()} });
			}
			else {
				if (color.name() == ""s) {
					return json::Node("none"s);
				}
				return json::Node(color.name());
			}
		}

		json::Node ColorToNode(const google::protobuf::RepeatedPtrField<serialize::Color>& color_vector) {
			json::Array result;
			result.reserve(color_vector.size());
			for (const auto& color : color_vector) {
				result.emplace_back(ColorToNode(color));
			}
			return json::Node(std::move(result));
		}

		void DeserializeMapRenderer(renderer::MapRenderer& mr, serialize::TransportCatalogue& deserialized_tc) {

			serialize::RenderSettings* render_settings = deserialized_tc.mutable_render_settings();
			svg::Point bus_label_offset{ render_settings->bus_label_offset().x(), render_settings->bus_label_offset().y() };
			svg::Point stop_label_offset{ render_settings->stop_label_offset().x(), render_settings->stop_label_offset().y() };

			mr.SetMapRenderer(json::Node(json::Dict{
					{{"width"s},{render_settings->width()}},
					{{"height"s},{render_settings->height()}},
					{{"padding"s},{render_settings->padding()}},
					{{"stop_radius"s},{render_settings->stop_radius()}},
					{{"line_width"s},{render_settings->line_width()}},
					{{"bus_label_font_size"s},{render_settings->bus_label_font_size()}},
					{{"bus_label_offset"s}, PointToNode(bus_label_offset)},
					{{"stop_label_offset"s}, PointToNode(stop_label_offset)},
					{{"stop_label_font_size"s},{render_settings->stop_label_font_size()}},
					{{"underlayer_color"s},ColorToNode(render_settings->underlayer_color())},
					{{"underlayer_width"s},{render_settings->underlayer_width()}},
					{{"color_palette"s},ColorToNode(render_settings->color_palette())},
				}));
		}//-----

		void SerializeGraph(const graph::DirectedWeightedGraph<double>& g, serialize::Graph* s_g) {
			size_t vertex_count = g.GetVertexCount();
			size_t edge_count = g.GetEdgeCount();
			for (size_t i = 0; i < edge_count; ++i) {
				const graph::Edge<double>& edge = g.GetEdge(i);
				serialize::Edge* s_edge = s_g->add_edge();
				s_edge->set_from(edge.from);
				s_edge->set_to(edge.to);
				s_edge->set_weight(edge.weight);
			}
			for (size_t i = 0; i < vertex_count; ++i) {
				serialize::Vertex* s_vertex = s_g->add_vertex();
				for (const auto& edge_id : g.GetIncidentEdges(i)) {
					s_vertex->add_edge_id(edge_id);
				}
			}
		}

		void SerializeRouter(const transport_router::Router& router, serialize::TransportCatalogue& serialized_tc) {
			serialize::Router* serialized_router = serialized_tc.mutable_router();
			// сериализуем настройки
			serialize::RouterSettings* router_settings = serialized_router->mutable_router_settings();
			auto [bus_wait_time, bus_velocity] = router.GetRouterSettings();
			router_settings->set_bus_wait_time(bus_wait_time);
			router_settings->set_bus_velocity(bus_velocity);
			// сериализуем контейнеры
			auto stop_id = router.GetStopId();
			auto edge_info = router.GetEdgeInfo();
			for (const auto& [stop_name, id] : stop_id) {
				serialize::StopId* item = serialized_router->add_stop_id();
				item->set_name(stop_name);
				item->set_id(static_cast<int>(id));
			}

			for (const auto& [edge_name, id] : edge_info) {
				serialize::EdgeInfo* item = serialized_router->add_edge_info();
				item->set_name(edge_name);
				item->set_id(static_cast<int>(id));
			}
			// сериализуем граф
			SerializeGraph(router.GetGraph(), serialized_router->mutable_graph());

		}//-----

		graph::DirectedWeightedGraph<double> DeserializeGraph(serialize::Graph* d_g) {
			graph::DirectedWeightedGraph<double> g(d_g->vertex_size());
			for (size_t i = 0; i < d_g->edge_size(); ++i) {
				const serialize::Edge& e = d_g->edge(i);
				graph::Edge<double> edge{ static_cast<graph::VertexId>(e.from()), static_cast<graph::VertexId>(e.to()), e.weight() };
				g.AddEdge(edge);
			}
			return g;
		}

		void DeserializeRouter(transport_router::Router& router, serialize::TransportCatalogue& deserialized_tc) {
			serialize::Router* deserialized_router = deserialized_tc.mutable_router();
			// десериализуем настройки
			int bus_wait_time = deserialized_router->router_settings().bus_wait_time();
			double bus_velocity = deserialized_router->router_settings().bus_velocity();
			// десериализуем контейнеры
			std::map<std::string, graph::VertexId> stop_id;
			std::vector<std::pair<std::string, int>> edge_info;

			for (const auto& stop_id_ : deserialized_router->stop_id()) {
				stop_id[stop_id_.name()] = static_cast<graph::VertexId>(stop_id_.id());
			}

			for (const auto& edge_info_ : deserialized_router->edge_info()) {
				edge_info.push_back({ edge_info_.name(), edge_info_.id() });
			}
			// десериализуем граф
			graph::DirectedWeightedGraph<double> graph = DeserializeGraph(deserialized_router->mutable_graph());
			router.SetRouter(bus_wait_time, bus_velocity, std::move(graph), std::move(stop_id), std::move(edge_info));
		}//-----
	}
}