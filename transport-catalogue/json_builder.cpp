#include "json_builder.h"

using namespace std::literals;

namespace json {

	KeyItemContext Builder::Key(std::string key) {
		if (root_.has_value()) {
			throw std::logic_error("Build finished."s);
		}
		if (queue_.empty() || !std::holds_alternative<Dict>(queue_.top())) {
			throw std::logic_error("Dict wasn't created."s);
		}
		if (is_last_command_is_key_ == true) {
			throw std::logic_error("Two keys in row."s);
		}

		keys_.push(std::move(key));
		is_last_command_is_key_ = true;
		return { *this };
	}

	Builder& Builder::Value(Node::Value value) {
		if (root_.has_value()) {
			throw std::logic_error("Build finished."s);
		}
		is_last_command_is_key_ = false;

		AddNode(std::move(MakeNode(value)));
		return *this;
	}

	DictItemContext Builder::StartDict() {
		if (root_) {
			throw std::logic_error("Build finished."s);
		}
		is_last_command_is_key_ = false;

		queue_.push(Dict());
		return { *this };
	}
	Builder& Builder::EndDict() {
		if (root_) {
			throw std::logic_error("Build finished."s);
		}
		is_last_command_is_key_ = false;

		if (queue_.empty() || !std::holds_alternative<Dict>(queue_.top())) {
			throw std::logic_error("Dict wasn't created."s);
		}

		Node as_dict = MakeNode(queue_.top());
		queue_.pop();
		AddNode(std::move(as_dict));
		return *this;
	}

	ArrayItemContext Builder::StartArray() {
		if (root_) {
			throw std::logic_error("Build finished."s);
		}
		is_last_command_is_key_ = false;

		queue_.push(Array());
		return { *this };
	}
	Builder& Builder::EndArray() {
		if (root_) {
			throw std::logic_error("Build finished."s);
		}
		is_last_command_is_key_ = false;

		if (queue_.empty() || !std::holds_alternative<Array>(queue_.top())) {
			throw std::logic_error("Array wasn't created."s);
		}

		Node as_arr = MakeNode(queue_.top());
		queue_.pop();
		AddNode(std::move(as_arr));
		return *this;
	}

	Node Builder::Build() {
		if (root_ && queue_.empty() && keys_.empty()) {
			return *root_;
		}
		else throw std::logic_error("Cannot build json."s);
	}

	Node Builder::MakeNode(const Node::Value& value) const {
		Node node;
		if (std::holds_alternative<std::string>(value)) {
			node = Node(std::get<std::string>(value));
		}
		else if (std::holds_alternative<int>(value)) {
			node = Node(std::get<int>(value));
		}
		else if (std::holds_alternative<bool>(value)) {
			node = Node(std::get<bool>(value));
		}
		else if (std::holds_alternative<double>(value)) {
			node = Node(std::get<double>(value));
		}
		else if (std::holds_alternative<Array>(value)) {
			node = Node(std::get<Array>(value));
		}
		else if (std::holds_alternative<Dict>(value)) {
			node = Node(std::get<Dict>(value));
		}
		else {
			node = Node(nullptr);
		}
		return node;
	}

	void Builder::AddNode(Node&& node) {
		if (queue_.empty()) {
			root_ = node;
			return;
		}
		if (std::holds_alternative<Dict>(queue_.top())) {
			if (keys_.empty()) {
				throw std::logic_error("Key wasn't created."s);
			}
			auto& as_dict = std::get<Dict>(queue_.top());
			as_dict.insert({ keys_.top(), node });
			keys_.pop();
			return;
		}
		if (std::holds_alternative<Array>(queue_.top())) {
			auto& as_array = std::get<Array>(queue_.top());
			as_array.push_back(node);
			return;
		}

		throw std::logic_error("Cannot insert node to the json."s);
	}

	// ItemContext Methods

	KeyItemContext ItemContext::Key(std::string key) {
		return builder_.Key(key);
	}
	DictItemContext ItemContext::StartDict() {
		return builder_.StartDict();
	}
	Builder& ItemContext::EndDict() {
		return builder_.EndDict();
	}
	ArrayItemContext ItemContext::StartArray() {
		return builder_.StartArray();
	}
	Builder& ItemContext::EndArray() {
		return builder_.EndArray();
	}

	// Value - methods

	ValueItemContext_Key KeyItemContext::Value(Node::Value value) {
		builder_.Value(value);
		return { builder_ };
	}
	ValueItemContext_Arr ArrayItemContext::Value(Node::Value value) {
		builder_.Value(value);
		return { builder_ };
	}
	ValueItemContext_Arr ValueItemContext_Arr::Value(Node::Value value) {
		builder_.Value(value);
		return { builder_ };
	}
}
