#pragma once

#include <optional>
#include <stack>

#include "json.h"

namespace json {

	class KeyItemContext;
	class DictItemContext;
	class ArrayItemContext;

	class Builder {
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

	class ValueItemContext_Key;
	class ValueItemContext_Arr;

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

	class KeyItemContext : public ItemContext {
	public:
		KeyItemContext(Builder& builder) : ItemContext(builder) {}

		KeyItemContext Key(std::string key) = delete;
		Builder& EndDict() = delete;
		Builder& EndArray() = delete;

		ValueItemContext_Key Value(Node::Value value);
	};

	class DictItemContext : public ItemContext {
	public:
		DictItemContext(Builder& builder) : ItemContext(builder) {}

		DictItemContext StartDict() = delete;
		ArrayItemContext StartArray() = delete;
		Builder& EndArray() = delete;
	};

	class ArrayItemContext : public ItemContext {
	public:
		ArrayItemContext(Builder& builder) : ItemContext(builder) {}

		KeyItemContext Key(std::string key) = delete;
		Builder& EndDict() = delete;

		ValueItemContext_Arr Value(Node::Value value);
	};

	class ValueItemContext_Key : public ItemContext {
	public:
		ValueItemContext_Key(Builder& builder) : ItemContext(builder) {}

		DictItemContext StartDict() = delete;
		ArrayItemContext StartArray() = delete;
		Builder& EndArray() = delete;
	};

	class ValueItemContext_Arr : public ItemContext {
	public:
		ValueItemContext_Arr(Builder& builder) : ItemContext(builder) {}

		KeyItemContext Key(std::string key) = delete;
		Builder& EndDict() = delete;

		ValueItemContext_Arr Value(Node::Value value);
	};
}