#include <iostream>
#include <algorithm>
#include <iomanip>

#include "stat_reader.h"

using namespace std::literals;

void StatReader::OutputBus(std::ostream& os, std::string_view bus_name, TransportCatalogue& tc) {
	const TransportCatalogue::Bus& bus = tc.GetBus(bus_name);
	if (bus_name != bus.name) {
		os << "Bus "s << bus_name << ": not found"s << std::endl;
		return;
	}
	std::vector<const TransportCatalogue::Stop*> unique_stops(bus.stops.begin(), bus.stops.end());
	std::sort(unique_stops.begin(), unique_stops.end());
	auto last = std::unique(unique_stops.begin(), unique_stops.end());
	unique_stops.erase(last, unique_stops.end());
	os << "Bus "s << bus_name << ": "s << bus.stops.size() << " stops on route, "s <<
		unique_stops.size() << " unique stops, "s << std::setprecision(6) << bus.route_distance << " route length, "s <<
		bus.curvature << " curvature" << std::endl;
}

void StatReader::OutputStop(std::ostream& os, std::string_view stop_name, TransportCatalogue& tc) {
	const TransportCatalogue::Stop* stop = tc.FindStop(stop_name);
	if (stop == nullptr) {
		os << "Stop "s << stop_name << ": not found"s << std::endl;
		return;
	}
	auto buses = tc.GetStop(stop);
	if (buses.size() == 0) {
		os << "Stop "s << stop_name << ": no buses"s << std::endl;
		return;
	}
	os << "Stop "s << stop_name << ": buses"s;
	for (const auto bus : buses) {
		os << " "s << bus;
	}
	os << std::endl;
}