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

        Router() = delete;

        Router(const transport_catalogue::TransportCatalogue& tc);

        std::optional<graph::Router<double>::RouteInfo> GetRouteInfo(std::string_view from_name, std::string_view to_name) const;

        const Graph& GetGraph() const;

        const std::map<graph::VertexId, std::string_view>& GetIdStop() const;

        const std::vector<std::pair<std::string_view, int>>& GetEdgeInfo() const;

        ~Router() {
            delete router_ptr_;
        }

    private:
        void BuildGraph(const transport_catalogue::TransportCatalogue& tc);

        std::pair<graph::VertexId, graph::VertexId> GetStopsIds(const transport_catalogue::TransportCatalogue::Bus* bus, 
            int index_from, int index_to);

        int bus_wait_time_ = 0;
        double bus_velocity_ = 0;

        Graph graph_;

        std::map<std::string_view, graph::VertexId> stop_ids_;
        std::map<graph::VertexId, std::string_view> id_stop;
        std::vector<std::pair<std::string_view, int>> edge_info_;

        graph::Router<double>* router_ptr_ = nullptr;
    };

} // namespace transport_router
