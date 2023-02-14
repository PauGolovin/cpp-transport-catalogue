#pragma once

#include <optional>
#include <stack>

#include "json.h"

namespace json {



	class Builder {
	public:
		class KeyItemContext;
		class DictItemContext;
		class ArrayItemContext;

		class ItemContext {
		protected:
			Builder& builder_;
		public:
			ItemContext(Builder& builder) : builder_(builder) {}

			KeyItemContext Key(std::string key);
			DictItemContext StartDict();
			Builder& EndDict();
			ArrayItemContext StartArray();
			Builder& EndArray();
		};

	private:
		std::optional<Node> root_;
		std::stack<Node::Value> queue_;
		std::stack<std::string> keys_;
		bool is_last_command_is_key_ = false;
	public:
		Builder() = default;

		KeyItemContext Key(std::string key);
		Builder& Value(Node::Value value);
		DictItemContext StartDict();
		Builder& EndDict();
		ArrayItemContext StartArray();
		Builder& EndArray();
		Node Build();
	private:
		Node MakeNode(const Node::Value& value) const;
		void AddNode(Node&& node);
	};

	// Вложенные классы:

	class Builder::KeyItemContext : public Builder::ItemContext {
	public:
		KeyItemContext(Builder& builder) : ItemContext(builder) {}

		KeyItemContext Key(std::string key) = delete;
		Builder& EndDict() = delete;
		Builder& EndArray() = delete;

		DictItemContext Value(Node::Value value);
	};

	class Builder::DictItemContext : public Builder::ItemContext {
	public:
		DictItemContext(Builder& builder) : ItemContext(builder) {}

		DictItemContext StartDict() = delete;
		ArrayItemContext StartArray() = delete;
		Builder& EndArray() = delete;
	};

	class Builder::ArrayItemContext : public Builder::ItemContext {
	public:
		ArrayItemContext(Builder& builder) : ItemContext(builder) {}

		KeyItemContext Key(std::string key) = delete;
		Builder& EndDict() = delete;

		ArrayItemContext Value(Node::Value value);
	};
}