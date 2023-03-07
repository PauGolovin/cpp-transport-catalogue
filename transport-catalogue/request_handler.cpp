#include "request_handler.h"

using namespace std::literals;

namespace stat_reader {

	json::Node RequestHandler::MakeOutputNode(const json::Node& node) {
		if (node.AsDict().at("type"s).AsString() == "Bus"s) {
			return MakeBusNode(node);
		}
		if (node.AsDict().at("type"s).AsString() == "Stop"s) {
			return MakeStopNode(node);
		}
		if (node.AsDict().at("type"s).AsString() == "Map"s) {
			return MakeMapNode(node);
		}
		if (node.AsDict().at("type"s).AsString() == "Route"s) {
			return MakeRouteNode(node);
		}
		throw std::logic_error("invalid query"s);
	}

	json::Node RequestHandler::MakeBusNode(const json::Node& node) {
		const transport_catalogue::TransportCatalogue::Bus* bus = tc_.FindBus(node.AsDict().at("name"s).AsString());
		if (bus == nullptr) {
			return json::Builder{}.
					StartDict().
						Key("request_id"s).Value(node.AsDict().at("id"s).AsInt()).
						Key("error_message"s).Value("not found"s).
					EndDict().
				Build();
		}

		std::vector<const transport_catalogue::TransportCatalogue::Stop*> unique_stops(bus->stops.begin(), bus->stops.end());
		std::sort(unique_stops.begin(), unique_stops.end());
		auto last = std::unique(unique_stops.begin(), unique_stops.end());
		unique_stops.erase(last, unique_stops.end());

		return json::Builder{}.
				StartDict().
					Key("curvature"s).Value(bus->distances.curvature).
					Key("request_id"s).Value(node.AsDict().at("id"s).AsInt()).
					Key("route_length"s).Value(bus->distances.route_distance).
					Key("stop_count"s).Value(static_cast<int>(bus->stops.size())).
					Key("unique_stop_count"s).Value(static_cast<int>(unique_stops.size())).
				EndDict().
			Build();

	}

	json::Node RequestHandler::MakeStopNode(const json::Node& node) {
		const transport_catalogue::TransportCatalogue::Stop* stop = tc_.GetStop(node.AsDict().at("name"s).AsString());
		if (stop == nullptr) {
			return json::Builder{}.
					StartDict().
						Key("request_id"s).Value(node.AsDict().at("id"s).AsInt()).
						Key("error_message"s).Value("not found"s).
					EndDict().
				Build();
		}
		auto buses = tc_.GetBusesOfStop(stop);
		json::Array buses_result;
		for (const auto bus : buses) {
			buses_result.push_back(std::string{ bus });
		}

		return json::Builder{}.
				StartDict().
					Key("buses"s).Value(buses_result).
					Key("request_id"s).Value(node.AsDict().at("id"s).AsInt()).
				EndDict().
			Build();
	}

	json::Node RequestHandler::MakeMapNode(const json::Node& node) {
		std::stringstream ss;
		mr_.PrintSvgDocument(ss);
		return json::Builder{}.
				StartDict().
					Key("map"s).Value(ss.str()).
					Key("request_id"s).Value(node.AsDict().at("id"s).AsInt()).
				EndDict().
			Build();
	}

	json::Node RequestHandler::MakeRouteNode(const json::Node& node) {
		if (router_ == nullptr) {
			router_ = new transport_router::Router(tc_);
		}
		return router_->GetResultJson(node);
	}

	
}