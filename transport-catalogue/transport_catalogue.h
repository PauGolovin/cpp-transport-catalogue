#pragma once
#include <deque>
#include <vector>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "geo.h"

class TransportCatalogue {
public:
	TransportCatalogue() = default;

	struct Stop {
		std::string name;
		Coordinates coordinates;
	};

	struct Bus {
		std::string name;
		std::vector<const Stop*> stops;
		int route_distance = 0;
		double raw_route_distance = 0;
		double curvature = 0;
		bool operator==(const Bus& other) const {
			return this->name == other.name;
		}
	};

	void AddBus(std::string_view name, std::vector<std::string_view> stops);
	void AddStop(std::string_view name, Coordinates& coordinates, std::vector<std::string_view> distances);
	const Bus* FindBus(const std::string_view name) const;
	Stop* FindStop(std::string_view name);
	const Bus& GetBus(const std::string_view name) const;
	const std::set<std::string_view>& GetStop(const Stop* stop) const;

private:
	struct BusHasher {
		size_t operator()(const Bus& bus) const;
	};
	struct StopHasher {
		size_t operator()(const Stop* stop) const;
	};
	struct DistanceHasher {
		size_t operator()(std::pair<const Stop*, const Stop*> stops) const;
	};

	std::tuple<int, double, double> DefineRouteDistance(std::vector<const Stop*>& stops);

	std::set<std::string_view> null_stop = std::set<std::string_view>();
	Bus null_bus_ = Bus();
	std::deque<Stop> all_stops_;
	std::unordered_set<Bus, BusHasher> all_buses_;
	std::unordered_map<const Stop*, std::set<std::string_view>, StopHasher> buses_of_stop_;
	std::unordered_map<std::pair<const Stop*, const Stop*>, double, DistanceHasher> all_distances_;
};