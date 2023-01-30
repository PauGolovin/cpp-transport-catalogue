#include "transport_catalogue.h"

#include <stdexcept>
#include <algorithm>

using namespace std::literals;
using namespace transportCatalogue;

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
	auto distance = DefineRouteDistance(stops_result);
	all_buses_.push_back({ std::string{ name }, stops_result, distance });
	for (const auto stop : stops_result) {
		buses_of_stop_[stop].insert(&all_buses_.back());
	}
}
void TransportCatalogue::AddStop(std::string_view name, const Coordinates& coordinates) {
	auto stop_ptr = FindStop(name);
	if (stop_ptr == nullptr) {
		all_stops_.push_back({ std::string{name}, {coordinates.lat, coordinates.lng} });
	}
	else {
		stop_ptr->coordinates.lat = coordinates.lat;
		stop_ptr->coordinates.lng = coordinates.lng;
	}
}
void TransportCatalogue::AddDistance(std::string_view name_from, std::string_view name_to, double distance) {
	auto name_from_ptr = FindStop(name_from);
	auto name_to_ptr = FindStop(name_to);
	if (name_to_ptr == nullptr) {
		AddStop(std::string{ name_to }, {});
		name_to_ptr = FindStop(name_to);
	}
	std::pair<const Stop*, const Stop*> stop_pair{ name_from_ptr, name_to_ptr };
	if (!all_distances_.count(stop_pair)) {
		all_distances_[stop_pair] = distance;
		all_distances_[{stop_pair.second, stop_pair.first}] = distance;
	}
	else {
		all_distances_[stop_pair] = distance;
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
const std::set<std::string_view> TransportCatalogue::GetBusesOfStop(const Stop* stop) const {
	std::set<std::string_view> result;
	if (!buses_of_stop_.count(stop)) {
		return result;
	}
	for (const auto bus : buses_of_stop_.at(stop)) {
		result.insert(bus->name);
	}
	return result;
}

Bus::Distance TransportCatalogue::DefineRouteDistance(std::vector<const Stop*>& stops) {
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

size_t TransportCatalogue::BusPtrHasher::operator()(const Bus* bus) const {
	std::hash<std::string> hasher;
	return hasher(bus->name);
}
size_t TransportCatalogue::StopHasher::operator()(const Stop* stop) const {
	std::hash<std::string> hasher;
	return static_cast<size_t>(hasher(stop->name) + stop->coordinates.lat * 229 + stop->coordinates.lng * 229 * 229);
}
size_t TransportCatalogue::DistanceHasher::operator()(std::pair<const Stop*, const Stop*> stops) const {
	std::hash<std::string> hasher;
	return static_cast<size_t>(hasher(stops.first->name) + hasher(stops.second->name) * 229);
}