#pragma once

#include "transport_catalogue.h"
#include "router.h"

#include <map>
#include <optional>
#include <string>
#include <string_view>

namespace transport_router {

    class Router {
    public:
        using Graph = graph::DirectedWeightedGraph<double>;
        using Stop = transport_catalogue::TransportCatalogue::Stop*;

        Router() = default;

        void SetRouter(const transport_catalogue::TransportCatalogue& tc);

        void SetRouter(int bus_wait_time, double bus_velocity, Graph&& graph, 
            std::map<std::string, graph::VertexId>&& stop_ids,
            std::vector<std::pair<std::string, int>>&& edge_info);

        std::optional<graph::Router<double>::RouteInfo> GetRouteInfo(std::string_view from_name, std::string_view to_name) const;

        std::pair<int, double> GetRouterSettings() const;

        const Graph& GetGraph() const;

        const std::map<std::string, graph::VertexId>& GetStopId() const;

        const std::vector<std::pair<std::string, int>>& GetEdgeInfo() const;

        const std::map<graph::VertexId, std::string_view>& GetIdStop() const;

        bool IsBuilt() const;

        ~Router() {
            delete router_ptr_;
        }

    private:
        bool is_built = false;

        void BuildGraph(const transport_catalogue::TransportCatalogue& tc);

        std::pair<graph::VertexId, graph::VertexId> GetStopsIds(const transport_catalogue::TransportCatalogue::Bus* bus,
            int index_from, int index_to);

        int bus_wait_time_ = 0;
        double bus_velocity_ = 0;

        Graph graph_;

        std::map<std::string, graph::VertexId> stop_ids_;
        std::vector<std::pair<std::string, int>> edge_info_;

        std::map<graph::VertexId, std::string_view> id_stop_;

        graph::Router<double>* router_ptr_ = nullptr;
    };

} // namespace transport_router
