#pragma once
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

#include <fstream>
#include <filesystem>
#include <string_view>

#include "transport_catalogue.pb.h"

namespace Serialization {
	void Serialize(const transport_catalogue::TransportCatalogue& tc, 
		const renderer::MapRenderer& mr, 
		const transport_router::Router& router,
		std::string_view path_s);

	void Deserialize(transport_catalogue::TransportCatalogue& tc, 
		renderer::MapRenderer& mr, 
		transport_router::Router& router,
		std::string_view path_s);

	namespace detail {
		void SerializeTransportCatalogue(const transport_catalogue::TransportCatalogue& tc, serialize::TransportCatalogue& serialized_tc);
		void DeserializeTransportCatalogue(transport_catalogue::TransportCatalogue& tc, serialize::TransportCatalogue& deserialized_tc);

		void SerializeMapRenderer(const renderer::MapRenderer& mr, serialize::TransportCatalogue& serialized_tc);
		void DeserializeMapRenderer(renderer::MapRenderer& mr, serialize::TransportCatalogue& deserialized_tc);

		void SerializeRouter(const transport_router::Router& router, serialize::TransportCatalogue& serialized_tc);
		void DeserializeRouter(transport_router::Router& router, serialize::TransportCatalogue& deserialized_tc);
	}
}
