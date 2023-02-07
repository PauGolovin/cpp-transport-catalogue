#pragma once

#include <iostream>

#include "json.h"
#include "transport_catalogue.h"
#include "request_handler.h"
#include "map_renderer.h"

namespace json_reader {
	void InputCommand(std::istream& is, std::ostream& os, transport_catalogue::TransportCatalogue& tc);

	class QueueQuery {
		virtual void DistributeCommand(json::Node& node) = 0;
		virtual void QueuePromotion() = 0;
	};
	// Обработка запросов на заполнение БД
	class InputQueue : public QueueQuery {
	public:
		explicit InputQueue(transport_catalogue::TransportCatalogue& tc) : tc_(tc) {}
		void DistributeCommand(json::Node& node) override;
		void QueuePromotion() override;

	private:
		transport_catalogue::TransportCatalogue& tc_;
		std::deque<json::Node> stops_queue_;
		std::deque<json::Node> bus_queue_;

		void AddStops();
		void AddBuses();
	};
	// Обработка на запросы к БД и получение ответов в ostream
	class StatQueue : public QueueQuery {
	public:
		explicit StatQueue(transport_catalogue::TransportCatalogue& tc, renderer::MapRenderer& mr, std::ostream& os) 
			: tc_(tc)
			, mr_(mr)
			, os_(os) 
		{}
		void DistributeCommand(json::Node& node) override;
		void QueuePromotion() override;

	private:
		transport_catalogue::TransportCatalogue& tc_;
		renderer::MapRenderer& mr_;
		std::ostream& os_;
		std::deque<json::Node> output_queue_;
	};
}