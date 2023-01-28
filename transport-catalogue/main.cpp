#include <iostream>
#include <cassert>
#include <sstream>
#include <string>
#include <stdexcept>
#include "input_reader.h"

using namespace std;

void InputCommand(istream& is, ostream& os, TransportCatalogue& tc) {
	int query_count = 0;
	string KOCTblLb;
	while (getline(is, KOCTblLb)) {
		query_count = stoi(KOCTblLb);
		QueryQueue qq(tc);
		string query;
		for (int i = 0; i < query_count; ++i) {
			getline(is, query);
			qq.DistributeCommand(query);
		}
		qq.QueuePromotion(os);
	}
}

void FullTest() {
	TransportCatalogue tc;
	stringstream oss;
	istringstream iss1{
		"13\n"
		"Stop Tolstopaltsevo: 55.611087, 37.20829, 3900m to Marushkino\n"
		"Stop Marushkino: 55.595884, 37.209755, 9900m to Rasskazovka, 100m to Marushkino\n"
		"Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye\n"
		"Bus 750: Tolstopaltsevo - Marushkino - Marushkino - Rasskazovka\n"
		"Stop Rasskazovka: 55.632761, 37.333324, 9500m to Marushkino\n"
		"Stop Biryulyovo Zapadnoye: 55.574371, 37.6517, 7500m to Rossoshanskaya ulitsa, 1800m to Biryusinka, 2400m to Universam\n"
		"Stop Biryusinka: 55.581065, 37.64839, 750m to Universam\n"
		"Stop Universam: 55.587655, 37.645687, 5600m to Rossoshanskaya ulitsa, 900m to Biryulyovo Tovarnaya\n"
		"Stop Biryulyovo Tovarnaya: 55.592028, 37.653656, 1300m to Biryulyovo Passazhirskaya\n"
		"Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164, 1200m to Biryulyovo Zapadnoye\n"
		"Bus 828: Biryulyovo Zapadnoye > Universam > Rossoshanskaya ulitsa > Biryulyovo Zapadnoye\n"
		"Stop Rossoshanskaya ulitsa: 55.595579, 37.605757\n"
		"Stop Prazhskaya: 55.611678, 37.603831"
	};
	InputCommand(iss1, oss, tc);
	istringstream iss2{
		"6\n"
		"Bus 256\n"
		"Bus 750\n"
		"Bus 751\n"
		"Stop Samara\n"
		"Stop Prazhskaya\n"
		"Stop Biryulyovo Zapadnoye"
	};
	InputCommand(iss2, oss, tc);
	vector<string> answer{
		"Bus 256: 6 stops on route, 5 unique stops, 5950 route length, 1.36124 curvature",
		"Bus 750: 7 stops on route, 3 unique stops, 27400 route length, 1.30853 curvature",
		"Bus 751: not found",
		"Stop Samara: not found",
		"Stop Prazhskaya: no buses",
		"Stop Biryulyovo Zapadnoye: buses 256 828"
	};
	vector<string> output_data;
	string str;
	while (getline(oss, str)) {
		output_data.push_back(str);
	}
	assert(answer == output_data);
	cout << "FullTest successful!"s << endl;
}

int main() {
	/*InputTests::TInputTests();
	FullTest();*/
	TransportCatalogue tc;
	InputCommand(cin, cout, tc);
	return 0;
}