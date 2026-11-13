// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#pragma once

#include "../Helper/Pidl.h"
#include <boost/signals2.hpp>
#include <optional>
#include <vector>

using TreeViewPath = std::vector<PidlAbsolute>;

struct TreeViewItemOrder
{
	// The path of nodes from the visible tree root to the parent. An empty path represents the
	// invisible root of the navigation tree.
	TreeViewPath parentPath;
	std::vector<PidlAbsolute> items;
};

// Stores manually ordered siblings in the navigation tree. The order is independent of the sort
// order used by the list view.
class TreeViewOrderModel
{
public:
	enum class Direction
	{
		Up,
		Down
	};

	using OrderChangedSignal = boost::signals2::signal<void(const TreeViewPath &)>;

	void SetItemOrders(const std::vector<TreeViewItemOrder> &itemOrders);
	std::vector<TreeViewItemOrder> GetItemOrders() const;
	bool HasOrder(const TreeViewPath &parentPath) const;
	std::optional<size_t> GetItemIndex(const TreeViewPath &parentPath,
		const PidlAbsolute &item) const;
	bool MoveItem(const TreeViewPath &parentPath, size_t itemIndex, Direction direction,
		const std::vector<PidlAbsolute> &currentOrder);
	void RenameItem(const PidlAbsolute &oldItem, const PidlAbsolute &newItem, bool isDirectory);
	void RemoveItem(const TreeViewPath &parentPath, const PidlAbsolute &item, bool isDirectory);
	boost::signals2::connection AddOrderChangedObserver(
		const OrderChangedSignal::slot_type &observer);

private:
	void SetItemOrder(const TreeViewPath &parentPath, const std::vector<PidlAbsolute> &items);

	std::vector<TreeViewItemOrder> m_itemOrders;
	OrderChangedSignal m_orderChangedSignal;
};
