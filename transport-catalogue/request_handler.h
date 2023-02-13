#pragma once

#include <algorithm>

#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json_builder.h"

namespace stat_reader {

	class RequestHandler {
	public:
		RequestHandler(const transport_catalogue::TransportCatalogue& tc, renderer::MapRenderer& mr) : tc_(tc), mr_(mr){}

		json::Node MakeOutputNode(const json::Node& node);
	private:
		json::Node MakeBusNode(const json::Node& node);
		json::Node MakeStopNode(const json::Node& node);
		json::Node MakeMapNode(const json::Node& node);

		const transport_catalogue::TransportCatalogue& tc_;
		renderer::MapRenderer& mr_;
	};
}