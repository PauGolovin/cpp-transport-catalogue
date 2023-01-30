#include "input_reader.h"
#include "stat_reader.h"
#include <stdexcept>
#include <iostream>

using namespace std::literals;
using namespace inputReader;

void QueryQueue::DistributeCommand(std::string& command) {
	auto pos = command.find_first_not_of(' ');
	switch (command[pos]) {
	case 'S':
	{
		stops_queue_.push_back(move(command));
		break;
	}
	case 'B':
	{
		bus_queue_.push_back(move(command));
		break;
	}
	default:
		throw std::invalid_argument("Error. Invalid query "s + command);
		break;
	}
}
void QueryQueue::QueuePromotion() {
	AddStops();
	AddBuses();
}
void QueryQueue::AddStops() {
	while (!stops_queue_.empty()) {
		auto stop = detail::ParseStopQuery(stops_queue_.front());
		tc_.AddStop(std::get<0>(stop), std::get<1>(stop));
		for (std::string_view distance_query : std::get<2>(stop)) {
			auto distance_data = detail::ParseDistanceQuery(distance_query);
			tc_.AddDistance(std::get<0>(stop), distance_data.first, distance_data.second);
		}
		stops_queue_.pop_front();
	}
}
void QueryQueue::AddBuses() {
	while (!bus_queue_.empty()) {
		auto bus = detail::ParseBusQuery(bus_queue_.front());
		tc_.AddBus(bus.first, bus.second);
		bus_queue_.pop_front();
	}
}

using StopData = std::tuple<std::string_view, Coordinates, std::vector<std::string_view>>;
StopData detail::ParseStopQuery(std::string_view query) {
	auto pos_S = query.find_first_not_of(' ');
	// начало имени
	auto pos_space = query.find(' ', pos_S);
	// конец имени
	auto pos_colon = query.find(':', pos_space);
	// разделитель координат
	auto pos_comma = query.find(',', pos_colon);
	++pos_space;
	std::string_view name = query.substr(pos_space, pos_colon - pos_space);
	pos_colon += 2;
	double latitude = std::stod(std::string{ query.substr(pos_colon, pos_comma - pos_colon) });
	pos_comma += 2;
	// проверка на еще одну запятую
	auto pos_comma_end = query.find(',', pos_comma);
	double longitude = std::stod(pos_comma_end == query.npos ? std::string{ query.substr(pos_comma) } :
		std::string{ query.substr(pos_comma, pos_comma_end - pos_comma) });
	// блок данных расстояний
	pos_comma = pos_comma_end;
	std::vector<std::string_view> distances;
	while (pos_comma != query.npos) {
		pos_comma += 2;
		pos_comma_end = query.find(',', pos_comma);
		distances.push_back(pos_comma_end == query.npos ? query.substr(pos_comma) :
			query.substr(pos_comma, pos_comma_end - pos_comma));
		pos_comma = pos_comma_end;
	}
	return { name, { latitude, longitude }, distances };
}
std::pair<std::string_view, std::vector<std::string_view>> detail::ParseBusQuery(std::string_view query) {
	// определяем тип записи маршрута
	char separator;
	auto circular_mark = query.find('>');
	if (circular_mark != query.npos) {
		separator = '>';
	}
	else {
		separator = '-';
	}
	auto pos_B = query.find_first_not_of(' ');
	// начало имени
	auto pos_space = query.find(' ', pos_B);
	// конец имени
	auto pos_colon = query.find(':', pos_space);
	++pos_space;
	std::string_view name = query.substr(pos_space, pos_colon - pos_space);

	std::vector<std::string_view> stops;
	++pos_colon;
	auto pos_stop = query.find_first_not_of(' ', pos_colon);
	while (pos_stop != query.npos) {
		auto pos_separator = query.find(separator, pos_stop);
		stops.push_back((pos_separator == query.npos) ? query.substr(pos_stop) :
			query.substr(pos_stop, pos_separator - pos_stop - 1));
		if (pos_separator == query.npos) {
			break;
		}
		++pos_separator;
		pos_stop = query.find_first_not_of(' ', pos_separator);
	}

	if (separator == '-') {
		size_t first_position = stops.size() - 2;
		for (int i = first_position; i >= 0; --i) {
			stops.push_back(stops[i]);
		}
	}
	return { name, stops };
}
std::pair<std::string_view, double> detail::ParseDistanceQuery(std::string_view query) {
	auto pos_m = query.find('m');
	double distance = std::stod(std::string{ query.substr(0, pos_m) });
	auto pos_stop = pos_m + 5;
	std::string_view stop_for_distance = query.substr(pos_stop);
	return { stop_for_distance, distance };
}