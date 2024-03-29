#pragma once

#include <algorithm>

#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json_builder.h"
#include "router.h"
#include "transport_router.h"

namespace stat_reader {

	class RequestHandler {
	public:
		RequestHandler(const transport_catalogue::TransportCatalogue& tc, renderer::MapRenderer& mr, const transport_router::Router& router)
			: tc_(tc)
			, mr_(mr)
			, router_(router) 
		{}

		json::Node MakeOutputNode(const json::Node& node);

	private:
		json::Node MakeBusNode(const json::Node& node);
		json::Node MakeStopNode(const json::Node& node);
		json::Node MakeMapNode(const json::Node& node);
		json::Node MakeRouteNode(const json::Node& node);

		const transport_catalogue::TransportCatalogue& tc_;
		renderer::MapRenderer& mr_;
		const transport_router::Router& router_;
	};
}