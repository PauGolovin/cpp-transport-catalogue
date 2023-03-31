#include "transport_router.h"

using namespace std::literals;

namespace transport_router {

	const double FACTOR_TO_CONVER_TO_MINUTE = 1.0 / 1000.0 * 60.0;

	void Router::SetRouter(const transport_catalogue::TransportCatalogue& tc) {
		Graph graph(tc.GetStopsCount());
		graph_ = std::move(graph);
		auto [bus_wait_time, bus_velocity] = tc.GetRoutingSettings();
		bus_wait_time_ = bus_wait_time;
		bus_velocity_ = bus_velocity;
		BuildGraph(tc);
		if (router_ptr_ != nullptr) {
			delete router_ptr_;
			router_ptr_ = nullptr;
		}
		router_ptr_ = new graph::Router<double>(graph_);
		is_built = true;
	}

	void Router::SetRouter(int bus_wait_time, double bus_velocity, Graph&& graph,
		std::map<std::string, graph::VertexId>&& stop_ids,
		std::vector<std::pair<std::string, int>>&& edge_info) {

		bus_wait_time_ = bus_wait_time;
		bus_velocity_ = bus_velocity;
		graph_ = std::move(graph);
		stop_ids_ = std::move(stop_ids);
		edge_info_ = std::move(edge_info);

		for (const auto& [stop, id] : stop_ids_) {
			id_stop_.insert({ id, stop });
		}

		router_ptr_ = new graph::Router<double>(graph_);
		is_built = true;
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

		std::string name_from = stop_from->name;
		std::string name_to = stop_to->name;

		graph::VertexId id_from;
		if (!stop_ids_.count(name_from)) {
			id_from = stop_ids_.size();
			stop_ids_[name_from] = id_from;
			id_stop_[id_from] = name_from;
		}
		else {
			id_from = stop_ids_.at(name_from);
		}

		graph::VertexId id_to;
		if (!stop_ids_.count(name_to)) {
			id_to = stop_ids_.size();
			stop_ids_[name_to] = id_to;
			id_stop_[id_to] = name_to;
		}
		else {
			id_to = stop_ids_.at(name_to);
		}

		return { id_from, id_to };
	}

	std::optional<graph::Router<double>::RouteInfo> Router::GetRouteInfo(std::string_view from_name, std::string_view to_name) const {

		if (!stop_ids_.count(std::string{ from_name }) || !stop_ids_.count(std::string{ to_name })) {
			return std::nullopt;
		}

		return router_ptr_->BuildRoute(stop_ids_.at(std::string{ from_name }), stop_ids_.at(std::string{ to_name }));
	}

	std::pair<int, double> Router::GetRouterSettings() const {
		return { bus_wait_time_, bus_velocity_ };
	}

	const Router::Graph& Router::GetGraph() const {
		return graph_;
	}

	const std::map<std::string, graph::VertexId>& Router::GetStopId() const {
		return stop_ids_;
	}

	const std::vector<std::pair<std::string, int>>& Router::GetEdgeInfo() const {
		return edge_info_;
	}

	const std::map<graph::VertexId, std::string_view>& Router::GetIdStop() const {
		return id_stop_;
	}

	bool Router::IsBuilt() const {
		return is_built;
	}
}
