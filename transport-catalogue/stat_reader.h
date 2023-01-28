#pragma once
#include <string>

#include "transport_catalogue.h"
namespace StatReader {
	void OutputBus(std::ostream& os, std::string_view bus_name, TransportCatalogue& tc);

	void OutputStop(std::ostream& os, std::string_view stop_name, TransportCatalogue& tc);
}