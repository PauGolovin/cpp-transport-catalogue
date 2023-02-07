#include "request_handler.h"

using namespace std::literals;

namespace stat_reader {

	json::Node RequestHandler::MakeOutputNode(const json::Node& node) {
		if (node.AsMap().at("type"s).AsString() == "Bus"s) {
			return MakeBusNode(node);
		}
		if (node.AsMap().at("type"s).AsString() == "Stop"s) {
			return MakeStopNode(node);
		}
		if (node.AsMap().at("type"s).AsString() == "Map"s) {
			return MakeMapNode(node);
		}
		throw std::logic_error("invalid query"s);
	}

	json::Node RequestHandler::MakeBusNode(const json::Node& node) {
		const transport_catalogue::TransportCatalogue::Bus* bus = tc_.FindBus(node.AsMap().at("name"s).AsString());
		if (bus == nullptr) {
			return json::Node{ json::Dict{{"request_id"s, node.AsMap().at("id"s)}, {"error_message"s, "not found"s}} };
		}

		std::vector<const transport_catalogue::TransportCatalogue::Stop*> unique_stops(bus->stops.begin(), bus->stops.end());
		std::sort(unique_stops.begin(), unique_stops.end());
		auto last = std::unique(unique_stops.begin(), unique_stops.end());
		unique_stops.erase(last, unique_stops.end());

		json::Dict result;
		result.emplace("curvature"s, bus->distances.curvature);
		result.emplace("request_id"s, node.AsMap().at("id"s));
		result.emplace("route_length"s, bus->distances.route_distance);
		result.emplace("stop_count"s, static_cast<int>(bus->stops.size()));
		result.emplace("unique_stop_count"s, static_cast<int>(unique_stops.size()));

		return json::Node{ std::move(result) };
	}

	json::Node RequestHandler::MakeStopNode(const json::Node& node) {
		const transport_catalogue::TransportCatalogue::Stop* stop = tc_.GetStop(node.AsMap().at("name"s).AsString());
		if (stop == nullptr) {
			return json::Node{ json::Dict{{"request_id"s, node.AsMap().at("id"s)}, {"error_message"s, "not found"s}} };
		}
		auto buses = tc_.GetBusesOfStop(stop);
		json::Array buses_result;
		for (const auto bus : buses) {
			buses_result.push_back(std::string{ bus });
		}
		return json::Node{ json::Dict{ {"buses"s, buses_result}, {"request_id"s, node.AsMap().at("id"s)} } };
	}

	json::Node RequestHandler::MakeMapNode(const json::Node& node) {
		std::stringstream ss;
		mr_.PrintSvgDocument(ss);
		return json::Node{ json::Dict{{"map"s, ss.str()}, {"request_id"s, node.AsMap().at("id"s)}} };
	}
}