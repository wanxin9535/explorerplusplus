// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#pragma once

#include "../Helper/Pidl.h"
#include <boost/signals2.hpp>
#include <optional>
#include <vector>

struct TreeViewItemOrder
{
	// An empty PIDL represents the invisible root of the navigation tree.
	PidlAbsolute parent;
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

	using OrderChangedSignal = boost::signals2::signal<void(const PidlAbsolute &)>;

	void SetItemOrders(const std::vector<TreeViewItemOrder> &itemOrders);
	std::vector<TreeViewItemOrder> GetItemOrders() const;
	bool HasOrder(const PidlAbsolute &parent) const;
	std::optional<size_t> GetItemIndex(const PidlAbsolute &parent, const PidlAbsolute &item) const;
	bool MoveItem(const PidlAbsolute &parent, const PidlAbsolute &item, Direction direction,
		const std::vector<PidlAbsolute> &currentOrder);
	void RenameItem(const PidlAbsolute &parent, const PidlAbsolute &oldItem,
		const PidlAbsolute &newItem, bool isDirectory);
	void RemoveItem(const PidlAbsolute &parent, const PidlAbsolute &item, bool isDirectory);
	boost::signals2::connection AddOrderChangedObserver(
		const OrderChangedSignal::slot_type &observer);

private:
	static bool AreParentsEqual(const PidlAbsolute &parent1, const PidlAbsolute &parent2);
	void SetItemOrder(const PidlAbsolute &parent, const std::vector<PidlAbsolute> &items);

	std::vector<TreeViewItemOrder> m_itemOrders;
	OrderChangedSignal m_orderChangedSignal;
};
