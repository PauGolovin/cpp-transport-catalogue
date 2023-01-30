#include "stat_reader.h"

#include <iostream>
#include <algorithm>
#include <iomanip>

using namespace std::literals;
using namespace statReader;

void QueryQueue::DistributeCommand(std::string& command) {
	auto pos = command.find_first_not_of(' ');
	switch (command[pos]) {
	case 'S':
	{
		output_queue_.push_back(move(command));
		break;
	}
	case 'B':
	{
		output_queue_.push_back(move(command));
		break;
	}
	default:
		throw std::invalid_argument("Error. Invalid query "s + command);
		break;
	}
}
void QueryQueue::QueuePromotion() {
	while (!output_queue_.empty()) {
		auto pos = output_queue_.front().find_first_not_of(' ');
		switch (output_queue_.front()[pos]) {
		case 'S':
			OutputStop(detail::ParseOutputQuery(output_queue_.front()));
			break;
		case 'B':
			OutputBus(detail::ParseOutputQuery(output_queue_.front()));
			break;
		}
		output_queue_.pop_front();
	}
}

void QueryQueue::OutputBus(std::string_view bus_name) {
	const transportCatalogue::TransportCatalogue::Bus* bus = tc_.FindBus(bus_name);
	if (bus == nullptr) {
		os_ << "Bus "s << bus_name << ": not found"s << std::endl;
		return;
	}
	std::vector<const transportCatalogue::TransportCatalogue::Stop*> unique_stops(bus->stops.begin(), bus->stops.end());
	std::sort(unique_stops.begin(), unique_stops.end());
	auto last = std::unique(unique_stops.begin(), unique_stops.end());
	unique_stops.erase(last, unique_stops.end());
	os_ << "Bus "s << bus_name << ": "s << bus->stops.size() << " stops on route, "s <<
		unique_stops.size() << " unique stops, "s << std::setprecision(6) << bus->distances.route_distance << " route length, "s <<
		bus->distances.curvature << " curvature" << std::endl;
}
void QueryQueue::OutputStop(std::string_view stop_name) {
	const transportCatalogue::TransportCatalogue::Stop* stop = tc_.FindStop(stop_name);
	if (stop == nullptr) {
		os_ << "Stop "s << stop_name << ": not found"s << std::endl;
		return;
	}
	auto buses = tc_.GetBusesOfStop(stop);
	if (buses.size() == 0) {
		os_ << "Stop "s << stop_name << ": no buses"s << std::endl;
		return;
	}
	os_ << "Stop "s << stop_name << ": buses"s;
	for (const auto bus : buses) {
		os_ << " "s << bus;
	}
	os_ << std::endl;
}

std::string_view detail::ParseOutputQuery(std::string_view query) {
	auto pos = query.find_first_not_of(' ');
	pos = query.find(' ', pos);
	pos = query.find_first_not_of(' ', pos);
	return query.substr(pos);
}