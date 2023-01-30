#pragma once
#include <string>

#include "transport_catalogue.h"

namespace statReader {
	class QueryQueue {
	public:
		explicit QueryQueue(transportCatalogue::TransportCatalogue& tc, std::ostream& os) : tc_(tc), os_(os) {}
		void DistributeCommand(std::string& command);
		void QueuePromotion();
	private:
		transportCatalogue::TransportCatalogue& tc_;
		std::ostream& os_;
		std::deque<std::string> output_queue_;

		void OutputBus(std::string_view bus_name);
		void OutputStop(std::string_view stop_name);
	};
	namespace detail {
		std::string_view ParseOutputQuery(std::string_view query);
	}
}