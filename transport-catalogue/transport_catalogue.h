#pragma once
#include <deque>
#include <vector>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "geo.h"

namespace transport_catalogue {
	class TransportCatalogue {
	public:
		TransportCatalogue() = default;

		struct Stop {
			std::string name;
			geo::Coordinates coordinates;
		};

		struct Bus {
			std::string name;
			std::vector<const Stop*> stops;
			struct Distance {
				int route_distance = 0;
				double raw_route_distance = 0;
				double curvature = 0;
			};
			Distance distances;
			bool is_roundtrip;
			bool operator==(const Bus& other) const {
				return this->name == other.name;
			}
		};

		void AddBus(std::string_view name, std::vector<std::string> stops, bool is_roundtrip);
		void AddStop(std::string_view name, const geo::Coordinates& coordinates);
		void SetDistance(std::string_view name_from, std::string_view name_to, double distance);
		const Bus* FindBus(const std::string_view name) const;
		const Stop* GetStop(std::string_view name) const;
		const std::set<std::string_view> GetBusesOfStop(const Stop* stop) const;

		const std::vector<const Bus*> GetAllBuses() const;

	private:
		Stop* FindStop(std::string_view name);

		struct BusPtrHasher {
			size_t operator()(const Bus* bus) const;
		};
		struct StopHasher {
			size_t operator()(const Stop* stop) const;
		};
		struct DistanceHasher {
			size_t operator()(std::pair<const Stop*, const Stop*> stops) const;
		};

		Bus::Distance DefineRouteDistance(std::vector<const Stop*>& stops);

		std::deque<Stop> all_stops_;
		std::deque<Bus> all_buses_;
		std::unordered_map<const Stop*, std::unordered_set<const Bus*, BusPtrHasher>, StopHasher> buses_of_stop_;
		std::unordered_map<std::pair<const Stop*, const Stop*>, double, DistanceHasher> all_distances_;
	};
}