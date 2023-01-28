#pragma once
#include <string>
#include <vector>
#include <deque>
#include <tuple>
#include "geo.h"
#include "transport_catalogue.h"

// Распределительный пункт ВСЕХ входных строк
class QueryQueue {
public:
	explicit QueryQueue(TransportCatalogue& tc)
		: tc_(tc) {}

	void DistributeCommand(std::string& command);

	void QueuePromotion(std::ostream& os);

	// для теста, потом удалить
	std::tuple<size_t, size_t, size_t> GetSizesForTest();

private:
	TransportCatalogue& tc_;
	std::deque<std::string> stops_queue_;
	std::deque<std::string> bus_queue_;
	std::deque<std::string> output_queue_;

	void AddStops();
	void AddBuses();
	void MakeOutput(std::ostream& os);
};
namespace InputParsing {
	using StopData = std::tuple<std::string_view, Coordinates, std::vector<std::string_view>>;
	StopData ParseStopQuery(std::string_view query);
	std::pair<std::string_view, std::vector<std::string_view>> ParseBusQuery(std::string_view query);
	std::string_view ParseOutputQuery(std::string_view query);
}

//namespace InputTests {
//	void TInputTests();
//	//void TParseStopQuery();
//	void TParseBusQuery();
//	void TParseOutputQuery();
//	void TDistributeCommand();
//}