#include "json_reader.h"
#include "serialization.h"

#include <sstream>
#include <cassert>
#include <iostream>

using namespace std;
using namespace transport_catalogue;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {
        TransportCatalogue tc;
        renderer::MapRenderer mr(tc);
        transport_router::Router router;
        json::Document doc = json::Load(cin);
        json_reader::InputCommand(doc, cout, tc, mr, router);

        string path = doc.GetRoot().AsDict().at("serialization_settings"s).AsDict().at("file"s).AsString();
        Serialization::Serialize(tc, mr, router, path);
    }
    else if (mode == "process_requests"sv) {
        json::Document doc = json::Load(cin);
        string path = doc.GetRoot().AsDict().at("serialization_settings"s).AsDict().at("file"s).AsString();
        TransportCatalogue tc;
        renderer::MapRenderer mr(tc);
        transport_router::Router router;
        Serialization::Deserialize(tc, mr, router, path);

        json_reader::InputCommand(doc, cout, tc, mr, router);
    }
    else {
        PrintUsage();
        return 1;
    }
}