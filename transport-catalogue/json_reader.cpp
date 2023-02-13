#include "json_reader.h"

using namespace std::literals;

namespace json_reader {

	void InputCommand(std::istream& is, std::ostream& os, transport_catalogue::TransportCatalogue& tc) {
		json::Document doc = json::Load(is);
		InputQueue iq(tc);
		renderer::MapRenderer mr(tc);
		StatQueue sq(tc, mr, os);
		for (const auto [key, queries] : doc.GetRoot().AsDict()) {
			if (key == "base_requests") {
				for (auto query : queries.AsArray()) {
					iq.DistributeCommand(query);
				}
			}
			else if (key == "stat_requests"s) {
				for (auto query : queries.AsArray()) {
					sq.DistributeCommand(query);
				}
			}
			else if (key == "render_settings"s) {
				mr.SetMapRenderer(queries);
			}
			else {
				throw std::invalid_argument("Invalid query array"s);
			}
		}
		iq.QueuePromotion();
		sq.QueuePromotion();
	}

	// ----- InputQueue -----

	void InputQueue::DistributeCommand(json::Node& node) {
		std::string request_type = node.AsDict().at("type"s).AsString();
		if (request_type == "Stop"s) {
			stops_queue_.push_back(std::move(node));
			return;
		}
		if (request_type == "Bus"s) {
			bus_queue_.push_back(std::move(node));
			return;
		}
		throw std::invalid_argument("Invalid query"s);
	}

	void InputQueue::QueuePromotion() {
		AddStops();
		AddBuses();
	}

	void InputQueue::AddStops() {
		while (!stops_queue_.empty()) {
			auto name = stops_queue_.front().AsDict().at("name"s).AsString();
			auto lat = stops_queue_.front().AsDict().at("latitude"s).AsDouble();
			auto lng = stops_queue_.front().AsDict().at("longitude"s).AsDouble();
			tc_.AddStop(name, { lat, lng });
			if (stops_queue_.front().AsDict().find("road_distances"s) != stops_queue_.front().AsDict().end()) {
				for (const auto [stop, distance] : stops_queue_.front().AsDict().at("road_distances"s).AsDict()) {
					tc_.SetDistance(stops_queue_.front().AsDict().at("name"s).AsString(), stop, distance.AsInt());
				}
			}
			stops_queue_.pop_front();
		}
	}

	std::vector<std::string> MakeVectorOfStops(const json::Node& node) {
		std::vector<std::string> result;
		auto stops = node.AsDict().at("stops"s).AsArray();
		for (const auto stop : stops) {
			result.push_back(stop.AsString());
		}
		if (!node.AsDict().at("is_roundtrip"s).AsBool()) {
			size_t first_position = result.size() - 2;
			for (int i = first_position; i >= 0; --i) {
				result.push_back(result[i]);
			}
		}
		return result;
	}

	void InputQueue::AddBuses() {
		while (!bus_queue_.empty()) {
			tc_.AddBus(bus_queue_.front().AsDict().at("name"s).AsString(), MakeVectorOfStops(bus_queue_.front()), 
				bus_queue_.front().AsDict().at("is_roundtrip"s).AsBool());
			bus_queue_.pop_front();
		}
	}

	// ----- StatQueue -----

	void StatQueue::DistributeCommand(json::Node& node) {
		output_queue_.push_back(std::move(node));
	}

	void StatQueue::QueuePromotion() {
		stat_reader::RequestHandler rh(tc_, mr_);
		if (output_queue_.empty()) {
			return;
		}
		json::Array result;
		while (!output_queue_.empty()) {
			result.push_back(rh.MakeOutputNode(output_queue_.front()));
			output_queue_.pop_front();
		}
		json::Print(json::Document{ json::Node{result} }, os_);
	}
}