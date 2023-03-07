#include "transport_router.h"

using namespace std::literals;

namespace transport_router {

	const double FACTOR_TO_CONVER_TO_MINUTE = 1.0 / 1000.0 * 60.0;

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
			for (int index_from = 0; index_from < (bus->stops.size() - 1); ++index_from) {
				for (int span_count = 1; span_count < (bus->stops.size() - 1); ++span_count) {
					size_t index_to = index_from + span_count;
					if (index_to >= bus->stops.size()) {
						continue;
					}

					auto [id_from, id_to] = GetStopsIds(bus, index_from, index_to);

					double distance = 0;

					for (int k = index_from; k < (index_from + span_count); ++k) {
						auto& stop_from_local = bus->stops[k];
						auto& stop_to_local = bus->stops[k + 1];

						distance += distances.at({ stop_from_local, stop_to_local });
					}

					double time = distance / bus_velocity_ * FACTOR_TO_CONVER_TO_MINUTE + bus_wait_time_;

					edge_info_.push_back({ bus->name, span_count });

					graph::Edge<double> edge{ id_from, id_to, time };

					graph_.AddEdge(edge);
				}
			}
		}
	}

	std::pair<graph::VertexId, graph::VertexId> Router::GetStopsIds(const transport_catalogue::TransportCatalogue::Bus* bus,
		int index_from, int index_to) {
		auto& stop_from = bus->stops[index_from];
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

		return { id_from, id_to };
	}

	std::optional<graph::Router<double>::RouteInfo> Router::GetRouteInfo(std::string_view from_name, std::string_view to_name) const {

		if (!stop_ids_.count(from_name) || !stop_ids_.count(to_name)) {
			return std::nullopt;
		}

		return router_ptr_->BuildRoute(stop_ids_.at(from_name), stop_ids_.at(to_name));
	}

	const Router::Graph& Router::GetGraph() const {
		return graph_;
	}

	const std::map<graph::VertexId, std::string_view>& Router::GetIdStop() const {
		return id_stop;
	}

	const std::vector<std::pair<std::string_view, int>>& Router::GetEdgeInfo() const {
		return edge_info_;
	}
}
