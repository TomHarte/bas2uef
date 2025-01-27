#pragma once

#include <initializer_list>
#include <optional>
#include <unordered_map>

/// Implements a fairly-vanilla retrieval tree.
///
/// @c NodeKeyT the type that is used to index the tree.
/// @c ValueT the type stored at nodes which complete a value.
template <typename NodeKeyT, typename ValueT>
struct Trie {
	Trie() = default;
	constexpr Trie(std::initializer_list<std::pair<const NodeKeyT *, ValueT>> values) noexcept {
		for(const auto &value: values) {
			insert(value.first, value.second);
		}
	}

	/// @returns This node's child corresponding to @c key if one exists; @c nullptr otherwise.
	const Trie *find(const NodeKeyT key) const {
		const auto it = children_.find(key);
		if(it == children_.end()) return nullptr;
		return &it->second;
	}

	/// @returns The value stored at this node if one exists; @c std::nullopt otherwise.
	const std::optional<ValueT> &value() const {
		return value_;
	}

private:
	constexpr void insert(const NodeKeyT *key, const ValueT &value) {
		if(*key) {
			children_[*key].insert(key + 1, value);
		} else {
			value_ = value;
		}
	}

	std::unordered_map<NodeKeyT, Trie> children_;
	std::optional<ValueT> value_;
};
