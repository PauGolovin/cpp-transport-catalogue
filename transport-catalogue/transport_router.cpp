#include "transport_router.h"

using namespace std::literals;

namespace transport_router {

	Router::Router(const transport_catalogue::TransportCatalogue& tc) 
		: graph_(tc.GetStopsCount())
	{
		auto [bus_wait_time, bus_velocity] = tc.GetRoutingSettings();
		bus_wait_time_ = bus_wait_time;
		bus_velocity_ = bus_velocity;
		BuildGraph(tc);
		router_ptr_ = new graph::Router<double>(graph_);
	}

	void Router::BuildGraph(const transport_catalogue::TransportCatalogue& tc) {
		auto& distances = tc.GetDistances();
		auto& buses = tc.GetAllBuses();

		for (const auto& bus : buses) {
			for (int i = 0; i < (bus->stops.size() - 1); ++i) {
				for (int j = 1; j < (bus->stops.size() - 1); ++j) {
					size_t index_to = i + j;
					if (index_to >= bus->stops.size()) {
						continue;
					}

					auto& stop_from = bus->stops[i];
					auto& stop_to = bus->stops[index_to];

					std::string_view name_from = stop_from->name;
					std::string_view name_to = stop_to->name;

					graph::VertexId id_from;
					if (!stop_ids_.count(name_from)) {
						id_from = stop_ids_.size();
						stop_ids_[name_from] = id_from;
						id_stop[id_from] = name_from;
					}
					else {
						id_from = stop_ids_.at(name_from);
					}

					graph::VertexId id_to;
					if (!stop_ids_.count(name_to)) {
						id_to = stop_ids_.size();
						stop_ids_[name_to] = id_to;
						id_stop[id_to] = name_to;
					}
					else {
						id_to = stop_ids_.at(name_to);
					}

					double distance = 0;
					for (int k = i; k < (i + j); ++k) {
						auto& stop_from_local = bus->stops[k];
						auto& stop_to_local = bus->stops[k + 1];

						distance += distances.at({ stop_from_local, stop_to_local });
					}

					double time = distance / 1000 / bus_velocity_ * 60 + bus_wait_time_;

					edge_info_.push_back({ bus->name, j });

					graph::Edge<double> edge{ id_from, id_to, time };

					graph_.AddEdge(edge);
				}
			}
		}
	}

	json::Node Router::GetResultJson(const json::Node& node) const {
		std::string_view from_name = node.AsDict().at("from"s).AsString();
		std::string_view to_name = node.AsDict().at("to"s).AsString();

		if (!stop_ids_.count(from_name) || !stop_ids_.count(to_name)) {
			return json::Builder{}.
					StartDict().
						Key("request_id"s).Value(node.AsDict().at("id"s).AsInt()).
						Key("error_message"s).Value("not found"s).
					EndDict().
				Build();
		}

		auto route = router_ptr_->BuildRoute(stop_ids_.at(from_name), stop_ids_.at(to_name));

		if (!route) {
			return json::Builder{}.
					StartDict().
						Key("request_id"s).Value(node.AsDict().at("id"s).AsInt()).
						Key("error_message"s).Value("not found"s).
					EndDict().
				Build();
		}

		auto json_route = json::Builder{};
		json_route.StartDict().
			Key("request_id"s).Value(node.AsDict().at("id"s).AsInt()).
			Key("total_time"s).Value(route->weight).
			Key("items"s).StartArray();

		for (const auto& edge : route->edges) {
			auto edge_from_graph = graph_.GetEdge(edge);

			json_route.StartDict().
				Key("type"s).Value("Wait"s).
				Key("stop_name"s).Value(std::string{ id_stop.at(edge_from_graph.from) }).
				Key("time"s).Value(bus_wait_time_).
				EndDict();

			auto [bus_name, stops_count] = edge_info_[edge];

			json_route.StartDict().
				Key("type"s).Value("Bus"s).
				Key("bus"s).Value(std::string{ bus_name }).
				Key("span_count"s).Value(stops_count).
				Key("time"s).Value(edge_from_graph.weight - bus_wait_time_).
				EndDict();
		}

		json_route.EndArray().EndDict();

		return json_route.Build();
	}
}
