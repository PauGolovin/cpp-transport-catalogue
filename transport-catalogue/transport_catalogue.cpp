#include "transport_catalogue.h"

#include <stdexcept>
#include <algorithm>

using namespace std::literals;

using Bus = TransportCatalogue::Bus;
using Stop = TransportCatalogue::Stop;

void TransportCatalogue::AddBus(std::string_view name, std::vector<std::string_view> stops) {
	std::vector<const Stop*> stops_result;
	stops_result.reserve(stops.size());
	for (const auto& stop : stops) {
		auto stop_ptr = FindStop(stop);
		if (stop_ptr == nullptr) {
			throw std::invalid_argument("Error. Stop "s + std::string{ stop } + " wasn't found in catalogue."s);
			return;
		}
		stops_result.push_back(stop_ptr);
	}
	auto route_distance = DefineRouteDistance(stops_result);
	auto it = all_buses_.insert({ std::string{ name }, stops_result, std::get<0>(route_distance),
		std::get<1>(route_distance), std::get<2>(route_distance) });
	for (const auto stop : stops_result) {
		buses_of_stop_[stop].insert(it.first->name);
	}
}

void TransportCatalogue::AddStop(std::string_view name, Coordinates& coordinates, std::vector<std::string_view> distances) {
	auto stop_ptr = FindStop(name);
	if (stop_ptr == nullptr) {
		all_stops_.push_back({ std::string{name}, {coordinates.lat, coordinates.lng} });
	}
	else {
		stop_ptr->coordinates.lat = coordinates.lat;
		stop_ptr->coordinates.lng = coordinates.lng;
	}
	for (auto distance_data : distances) {
		auto pos_m = distance_data.find('m');
		double distance = std::stod(std::string{ distance_data.substr(0, pos_m) });
		auto pos_stop = pos_m + 5;
		std::string_view stop_for_distance = distance_data.substr(pos_stop);

		stop_ptr = FindStop(stop_for_distance);
		if (stop_ptr == nullptr) {
			all_stops_.push_back({ std::string{stop_for_distance}, {} });
			stop_ptr = FindStop(stop_for_distance);
		}
		std::pair<const Stop*, const Stop*> stop_pair{ FindStop(name), stop_ptr };
		if (!all_distances_.count(stop_pair)) {
			all_distances_[stop_pair] = distance;
			all_distances_[{stop_pair.second, stop_pair.first}] = distance;
		}
		else {
			all_distances_[stop_pair] = distance;
		}
	}
}

const Bus* TransportCatalogue::FindBus(const std::string_view name) const {
	auto it = std::find_if(all_buses_.begin(), all_buses_.end(), [&name](const Bus& bus) {
		return bus.name == name;
		});
	if (it == all_buses_.end()) {
		return nullptr;
	}
	const Bus* result = &(*it);
	return result;
}

Stop* TransportCatalogue::FindStop(std::string_view name) {
	auto it = std::find_if(all_stops_.begin(), all_stops_.end(), [&name](const Stop& stop) {
		return stop.name == name;
		});
	if (it == all_stops_.end()) {
		return nullptr;
	}
	Stop* result = &(*it);
	return result;
}

const Bus& TransportCatalogue::GetBus(const std::string_view name) const {
	auto ptr = FindBus(name);
	if (ptr == nullptr) {
		return null_bus_;
	}
	return *ptr;
}

const std::set<std::string_view>& TransportCatalogue::GetStop(const Stop* stop) const {
	if (!buses_of_stop_.count(stop)) {
		return null_stop;
	}
	return buses_of_stop_.at(stop);
}

std::tuple<int, double, double> TransportCatalogue::DefineRouteDistance(std::vector<const Stop*>& stops) {
	int route_distance = 0;
	double raw_route_distance = 0;
	double curvature = 0;
	for (size_t i = 0; i < stops.size() - 1; ++i) {
		std::pair<const Stop*, const Stop*> stops_for_distance{ stops[i], stops[i + 1] };
		double raw_part_distance = ComputeDistance(stops[i]->coordinates, stops[i + 1]->coordinates);
		raw_route_distance += raw_part_distance;
		if (all_distances_.count(stops_for_distance)) {
			route_distance += all_distances_[stops_for_distance];
		}
		else {
			all_distances_.insert({ stops_for_distance, raw_part_distance });
			route_distance += raw_part_distance;
		}
	}
	curvature = route_distance / raw_route_distance;
	return { route_distance, raw_route_distance, curvature };
}

size_t TransportCatalogue::BusHasher::operator()(const Bus& bus) const {
	std::hash<std::string> hasher;
	return hasher(bus.name);
}

size_t TransportCatalogue::StopHasher::operator()(const Stop* stop) const {
	std::hash<std::string> hasher;
	return static_cast<size_t>(hasher(stop->name) + stop->coordinates.lat * 229 + stop->coordinates.lng * 229 * 229);
}

size_t TransportCatalogue::DistanceHasher::operator()(std::pair<const Stop*, const Stop*> stops) const {
	std::hash<std::string> hasher;
	return static_cast<size_t>(hasher(stops.first->name) + hasher(stops.second->name) * 229);
}