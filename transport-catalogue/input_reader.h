#pragma once
#include <string>
#include <vector>
#include <deque>
#include <tuple>
#include "geo.h"
#include "transport_catalogue.h"

namespace inputReader {
	class QueryQueue {
	public:
		explicit QueryQueue(transportCatalogue::TransportCatalogue& tc) : tc_(tc) {}
		void DistributeCommand(std::string& command);
		void QueuePromotion();

	private:
		transportCatalogue::TransportCatalogue& tc_;
		std::deque<std::string> stops_queue_;
		std::deque<std::string> bus_queue_;

		void AddStops();
		void AddBuses();
	};
	namespace detail {
		using StopData = std::tuple<std::string_view, Coordinates, std::vector<std::string_view>>;
		StopData ParseStopQuery(std::string_view query);
		std::pair<std::string_view, std::vector<std::string_view>> ParseBusQuery(std::string_view query);
		std::pair<std::string_view, double> ParseDistanceQuery(std::string_view query);
	}
}