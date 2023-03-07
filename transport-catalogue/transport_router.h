#pragma once
#include "json.h"
#include "json_builder.h"
#include "transport_catalogue.h"
#include "router.h"

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
          
        json::Node GetResultJson(const json::Node& node) const;

        ~Router() {
            delete router_ptr_;
        }

    private:
        void BuildGraph(const transport_catalogue::TransportCatalogue& tc);

        int bus_wait_time_ = 0;
        double bus_velocity_ = 0;

        Graph graph_;

        std::map<std::string_view, graph::VertexId> stop_ids_;
        std::map<graph::VertexId, std::string_view> id_stop;
        std::vector<std::pair<std::string_view, int>> edge_info_;

        graph::Router<double>* router_ptr_ = nullptr;
    };

} // namespace transport_router
