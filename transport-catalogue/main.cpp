#include "json_reader.h"

#include <sstream>
#include <cassert>

using namespace std;
using namespace transport_catalogue;

void Test() {
    TransportCatalogue tc;
    stringstream oss;
    istringstream iss{
        "{\n"
        "\"base_requests\": [\n"
        "{\n"
        "\"type\": \"Bus\",\n"
        "\"name\" : \"114\",\n"
        "\"stops\" : [\"Marine Station\", \"Riviera Bridge\"] ,\n"
        "\"is_roundtrip\" : false\n"
        "},\n"
        "{\n"
        "\"type\": \"Stop\",\n"
        "\"name\" : \"Riviera Bridge\",\n"
        "\"latitude\" : 43.587795,\n"
        "\"longitude\" : 39.716901,\n"
        "\"road_distances\" : {\"Marine Station\": 850}\n"
        "},\n"
        "{\n"
        "\"type\": \"Stop\",\n"
        "\"name\" : \"Marine Station\",\n"
        "\"latitude\" : 43.581969,\n"
        "\"longitude\" : 39.719848,\n"
        "\"road_distances\" : {\"Riviera Bridge\": 850}\n"
        "}\n"
        "],\n"
        "\"stat_requests\": [\n"
        "{ \"id\": 1, \"type\" : \"Stop\", \"name\" : \"Riviera Bridge\" },\n"
        "{ \"id\": 2, \"type\" : \"Bus\", \"name\" : \"114\" }\n"
        "]\n"
        "}"
    };
    json_reader::InputCommand(iss, oss, tc);
    istringstream answer{
        "[\n"
        "\t{\n"
        "\t\t\"buses\" : [\n"
        "\t\t\t\"114\"\n"
        "\t\t],\n"
        "\t\t\"request_id\" : 1\n"
        "\t},\n"
        "\t{\n"
        "\t\t\"curvature\" : 1.23199,\n"
        "\t\t\"request_id\" : 2,\n"
        "\t\t\"route_length\" : 1700,\n"
        "\t\t\"stop_count\" : 3,\n"
        "\t\t\"unique_stop_count\" : 2\n"
        "\t}\n"
        "]\n"
    };
    assert(oss.str() == answer.str());
    cout << "Test successfull" << endl;
}

int main() {
    //Test();
    TransportCatalogue tc;
    json_reader::InputCommand(cin, cout, tc);
}