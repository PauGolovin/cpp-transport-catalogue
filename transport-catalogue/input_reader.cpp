#include "input_reader.h"
#include "stat_reader.h"
#include <stdexcept>
#include <cassert>
#include <iostream>

using namespace std::literals;
using namespace InputParsing;

void QueryQueue::DistributeCommand(std::string& command) {
	// проверка на пробелы (на всякий случай)
	auto pos = command.find_first_not_of(' ');
	switch (command[pos]) {
	case 'S':
	{
		auto pos_colon = command.find(':');
		if (pos_colon != command.npos) {
			stops_queue_.push_back(move(command));
		}
		else {
			output_queue_.push_back(move(command));
		}
		break;
	}
	case 'B':
	{
		auto pos_colon = command.find(':');
		if (pos_colon != command.npos) {
			bus_queue_.push_back(move(command));
		}
		else {
			output_queue_.push_back(move(command));
		}
		break;
	}
	default:
		throw std::invalid_argument("Error. Invalid query "s + command);
		break;
	}
}

void QueryQueue::QueuePromotion(std::ostream& os) {
	AddStops();
	AddBuses();
	MakeOutput(os);
}

void QueryQueue::AddStops() {
	while (!stops_queue_.empty()) {
		auto stop = InputParsing::ParseStopQuery(stops_queue_.front());
		tc_.AddStop(std::get<0>(stop), std::get<1>(stop), std::get<2>(stop));
		stops_queue_.pop_front();
	}
}

void QueryQueue::AddBuses() {
	while (!bus_queue_.empty()) {
		auto bus = InputParsing::ParseBusQuery(bus_queue_.front());
		tc_.AddBus(bus.first, bus.second);
		bus_queue_.pop_front();
	}
}

void QueryQueue::MakeOutput(std::ostream& os) {
	while (!output_queue_.empty()) {
		std::string_view query{ output_queue_.front() };
		auto pos = query.find_first_not_of(' ');
		auto out_line = InputParsing::ParseOutputQuery(output_queue_.front());
		if (query[pos] == 'B') {
			StatReader::OutputBus(os, out_line, tc_);
		}
		if (query[pos] == 'S') {
			StatReader::OutputStop(os, out_line, tc_);
		}
		output_queue_.pop_front();
	}
}

std::tuple<size_t, size_t, size_t> QueryQueue::GetSizesForTest() {
	return { stops_queue_.size(), bus_queue_.size(), output_queue_.size() };
}

using StopData = std::tuple<std::string_view, Coordinates, std::vector<std::string_view>>;
StopData InputParsing::ParseStopQuery(std::string_view query) {
	// начало слова Stop (на всякий случай)
	auto pos_S = query.find_first_not_of(' ');
	// начало имени
	auto pos_space = query.find(' ', pos_S);
	// конец имени
	auto pos_colon = query.find(':', pos_space);
	// разделитель координат
	auto pos_comma = query.find(',', pos_colon);
	// пока обрезаю строку четко как в примере
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

std::pair<std::string_view, std::vector<std::string_view>> InputParsing::ParseBusQuery(std::string_view query) {
	// определяем тип записи маршрута
	char separator;
	auto circular_mark = query.find('>');
	if (circular_mark != query.npos) {
		separator = '>';
	}
	else {
		separator = '-';
	}
	// начало слова Bus (на всякий случай)
	auto pos_B = query.find_first_not_of(' ');
	// начало имени
	auto pos_space = query.find(' ', pos_B);
	// конец имени
	auto pos_colon = query.find(':', pos_space);
	// пока обрезаю строку четко как в примере
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

std::string_view InputParsing::ParseOutputQuery(std::string_view query) {
	auto pos = query.find_first_not_of(' ');
	pos = query.find(' ', pos);
	pos = query.find_first_not_of(' ', pos);
	return query.substr(pos);
}




// Закомментил для будущих исправлений (пока не надо)

//void InputTests::TInputTests() {
//	//TParseStopQuery();
//	TParseBusQuery();
//	TParseOutputQuery();
//	TDistributeCommand();
//	std::cout << "InputTest successful!" << std::endl;
//}
////void InputTests::TParseStopQuery() {
////	std::string test_str = "Stop Rasskazovka: 55.632761, 37.333324"s;
////	std::pair < std::string, Coordinates> answer{ "Rasskazovka"s , {55.632761, 37.333324} };
////	auto result = ParseStopQuery(test_str);
////	assert(answer.first == result.first && answer.second.lat == result.second.lat && answer.second.lng == result.second.lng);
////}
//void InputTests::TParseBusQuery() {
//	{
//		std::string_view test_str = "Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Zapadnoye"s;
//		std::pair<std::string_view, std::vector<std::string_view>> answer{ "256"s, {"Biryulyovo Zapadnoye"s,
//			"Biryusinka"s, "Universam"s, "Biryulyovo Zapadnoye"s} };
//		auto result = ParseBusQuery(test_str);
//		assert(answer.first == result.first && answer.second == result.second);
//	}
//	{
//		std::string_view test_str = "Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka"s;
//		std::pair<std::string_view, std::vector<std::string_view>> answer{ "750"s, {"Tolstopaltsevo"s,
//			"Marushkino"s, "Rasskazovka"s, "Marushkino"s, "Tolstopaltsevo"s} };
//		auto result = ParseBusQuery(test_str);
//		assert(answer.first == result.first && answer.second == result.second);
//	}
//}
//void InputTests::TParseOutputQuery() {
//	std::string_view test_str = "Bus 256"s;
//	std::string_view answer{ "256"s };
//	std::string_view result = ParseOutputQuery(test_str);
//	assert(answer == result);
//}
//void InputTests::TDistributeCommand() {
//	TransportCatalogue test_tc;
//	QueryQueue test_qq(test_tc);
//	std::string test_stop1 = "Stop Tolstopaltsevo : 55.611087, 37.208290"s;
//	std::string test_stop2 = "Stop Marushkino : 55.595884, 37.209755"s;
//	std::string test_add_bus1 = "Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka"s;
//	std::string test_find_bus1 = "Bus 256"s;
//	std::string test_find_bus2 = "Bus 750"s;
//	std::string test_find_bus3 = "Bus 751"s;
//		
//	test_qq.DistributeCommand(test_find_bus1);
//	test_qq.DistributeCommand(test_add_bus1);
//	test_qq.DistributeCommand(test_stop1);
//	test_qq.DistributeCommand(test_find_bus2);
//	test_qq.DistributeCommand(test_stop2);
//	test_qq.DistributeCommand(test_find_bus3);
//
//	auto [stops, buses, output] = test_qq.GetSizesForTest();
//	assert(stops == 2 && buses == 1 && output == 3);
//}
