// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#include "stdafx.h"
#include "TreeViewOrderModel.h"
#include <algorithm>
#include <optional>
#include <vector>

namespace
{

std::optional<PidlAbsolute> RebasePidl(const PidlAbsolute &oldPrefix,
	const PidlAbsolute &newPrefix, const PidlAbsolute &pidl)
{
	if (pidl == oldPrefix)
	{
		return newPrefix;
	}

	if (!oldPrefix.IsAncestor(pidl))
	{
		return std::nullopt;
	}

	std::vector<PidlChild> relativeItems;
	PidlAbsolute remainingPidl = pidl;

	while (remainingPidl != oldPrefix)
	{
		auto lastItem = remainingPidl.GetLastItem();

		if (!lastItem.HasValue() || !remainingPidl.RemoveLastItem())
		{
			return std::nullopt;
		}

		relativeItems.push_back(std::move(lastItem));
	}

	PidlAbsolute rebasedPidl = newPrefix;

	for (auto itr = relativeItems.rbegin(); itr != relativeItems.rend(); itr++)
	{
		rebasedPidl += *itr;
	}

	return rebasedPidl;
}

void AddChangedParent(std::vector<PidlAbsolute> &parents, const PidlAbsolute &parent)
{
	if (std::none_of(parents.begin(), parents.end(), [&parent](const auto &currentParent)
			{ return (!currentParent.HasValue() && !parent.HasValue())
				|| (currentParent.HasValue() && parent.HasValue() && currentParent == parent); }))
	{
		parents.push_back(parent);
	}
}

bool ContainsItem(const std::vector<PidlAbsolute> &items, const PidlAbsolute &item)
{
	return std::find(items.begin(), items.end(), item) != items.end();
}

}

void TreeViewOrderModel::SetItemOrders(const std::vector<TreeViewItemOrder> &itemOrders)
{
	m_itemOrders.clear();

	for (const auto &itemOrder : itemOrders)
	{
		std::vector<PidlAbsolute> items;

		for (const auto &item : itemOrder.items)
		{
			if (item.HasValue() && !ContainsItem(items, item))
			{
				items.push_back(item);
			}
		}

		if (items.empty())
		{
			continue;
		}

		auto existingOrder = std::find_if(m_itemOrders.begin(), m_itemOrders.end(),
			[this, &itemOrder](const auto &currentOrder)
			{ return AreParentsEqual(currentOrder.parent, itemOrder.parent); });

		if (existingOrder == m_itemOrders.end())
		{
			m_itemOrders.push_back({ itemOrder.parent, std::move(items) });
		}
		else
		{
			existingOrder->items = std::move(items);
		}
	}
}

std::vector<TreeViewItemOrder> TreeViewOrderModel::GetItemOrders() const
{
	return m_itemOrders;
}

bool TreeViewOrderModel::HasOrder(const PidlAbsolute &parent) const
{
	return std::any_of(m_itemOrders.begin(), m_itemOrders.end(),
		[this, &parent](const auto &itemOrder)
		{ return AreParentsEqual(itemOrder.parent, parent); });
}

std::optional<size_t> TreeViewOrderModel::GetItemIndex(const PidlAbsolute &parent,
	const PidlAbsolute &item) const
{
	auto order = std::find_if(m_itemOrders.begin(), m_itemOrders.end(),
		[this, &parent](const auto &currentOrder)
		{ return AreParentsEqual(currentOrder.parent, parent); });

	if (order == m_itemOrders.end())
	{
		return std::nullopt;
	}

	auto itemItr = std::find(order->items.begin(), order->items.end(), item);

	if (itemItr == order->items.end())
	{
		return std::nullopt;
	}

	return static_cast<size_t>(std::distance(order->items.begin(), itemItr));
}

bool TreeViewOrderModel::MoveItem(const PidlAbsolute &parent, const PidlAbsolute &item,
	Direction direction, const std::vector<PidlAbsolute> &currentOrder)
{
	auto itemItr = std::find(currentOrder.begin(), currentOrder.end(), item);

	if (itemItr == currentOrder.end())
	{
		return false;
	}

	size_t itemIndex = static_cast<size_t>(std::distance(currentOrder.begin(), itemItr));
	size_t newIndex;

	switch (direction)
	{
	case Direction::Up:
		if (itemIndex == 0)
		{
			return false;
		}

		newIndex = itemIndex - 1;
		break;

	case Direction::Down:
		if (itemIndex + 1 == currentOrder.size())
		{
			return false;
		}

		newIndex = itemIndex + 1;
		break;

	default:
		assert(false);
		return false;
	}

	auto updatedOrder = currentOrder;
	std::swap(updatedOrder[itemIndex], updatedOrder[newIndex]);
	SetItemOrder(parent, updatedOrder);
	return true;
}

void TreeViewOrderModel::RenameItem(const PidlAbsolute &parent, const PidlAbsolute &oldItem,
	const PidlAbsolute &newItem, bool isDirectory)
{
	if (!oldItem.HasValue() || !newItem.HasValue() || oldItem == newItem)
	{
		return;
	}

	std::vector<TreeViewItemOrder> updatedOrders;
	std::vector<PidlAbsolute> changedParents;

	for (const auto &itemOrder : m_itemOrders)
	{
		TreeViewItemOrder updatedOrder = itemOrder;
		bool changed = false;

		if (AreParentsEqual(itemOrder.parent, parent))
		{
			for (auto &item : updatedOrder.items)
			{
				if (item == oldItem)
				{
					item = newItem;
					changed = true;
				}
			}
		}

		if (isDirectory)
		{
			if (itemOrder.parent.HasValue()
				&& (itemOrder.parent == oldItem || oldItem.IsAncestor(itemOrder.parent)))
			{
				auto rebasedParent = RebasePidl(oldItem, newItem, itemOrder.parent);

				if (rebasedParent)
				{
					updatedOrder.parent = *rebasedParent;
					changed = true;
				}
			}

			for (auto &item : updatedOrder.items)
			{
				if (item == oldItem || oldItem.IsAncestor(item))
				{
					auto rebasedItem = RebasePidl(oldItem, newItem, item);

					if (rebasedItem)
					{
						item = *rebasedItem;
						changed = true;
					}
				}
			}
		}

		if (changed)
		{
			AddChangedParent(changedParents, itemOrder.parent);
			AddChangedParent(changedParents, updatedOrder.parent);
		}

		if (!updatedOrder.items.empty())
		{
			updatedOrders.push_back(std::move(updatedOrder));
		}
	}

	std::vector<TreeViewItemOrder> mergedOrders;

	for (auto &itemOrder : updatedOrders)
	{
		auto existingOrder = std::find_if(mergedOrders.begin(), mergedOrders.end(),
			[this, &itemOrder](const auto &currentOrder)
			{ return AreParentsEqual(currentOrder.parent, itemOrder.parent); });

		if (existingOrder == mergedOrders.end())
		{
			mergedOrders.push_back(std::move(itemOrder));
			continue;
		}

		for (const auto &item : itemOrder.items)
		{
			if (!ContainsItem(existingOrder->items, item))
			{
				existingOrder->items.push_back(item);
			}
		}
	}

	if (changedParents.empty())
	{
		return;
	}

	m_itemOrders = std::move(mergedOrders);

	for (const auto &changedParent : changedParents)
	{
		m_orderChangedSignal(changedParent);
	}
}

void TreeViewOrderModel::RemoveItem(const PidlAbsolute &parent, const PidlAbsolute &item,
	bool isDirectory)
{
	if (!item.HasValue())
	{
		return;
	}

	std::vector<PidlAbsolute> changedParents;

	for (auto orderItr = m_itemOrders.begin(); orderItr != m_itemOrders.end();)
	{
		bool removeOrder = isDirectory && orderItr->parent.HasValue()
			&& (orderItr->parent == item || item.IsAncestor(orderItr->parent));

		if (removeOrder)
		{
			AddChangedParent(changedParents, orderItr->parent);
			orderItr = m_itemOrders.erase(orderItr);
			continue;
		}

		if (AreParentsEqual(orderItr->parent, parent))
		{
			auto itemItr = std::find(orderItr->items.begin(), orderItr->items.end(), item);

			if (itemItr != orderItr->items.end())
			{
				orderItr->items.erase(itemItr);
				AddChangedParent(changedParents, parent);
			}

			if (orderItr->items.empty())
			{
				orderItr = m_itemOrders.erase(orderItr);
				continue;
			}
		}

		++orderItr;
	}

	for (const auto &changedParent : changedParents)
	{
		m_orderChangedSignal(changedParent);
	}
}

boost::signals2::connection TreeViewOrderModel::AddOrderChangedObserver(
	const OrderChangedSignal::slot_type &observer)
{
	return m_orderChangedSignal.connect(observer);
}

bool TreeViewOrderModel::AreParentsEqual(const PidlAbsolute &parent1, const PidlAbsolute &parent2)
{
	if (parent1.HasValue() != parent2.HasValue())
	{
		return false;
	}

	return !parent1.HasValue() || parent1 == parent2;
}

void TreeViewOrderModel::SetItemOrder(const PidlAbsolute &parent,
	const std::vector<PidlAbsolute> &items)
{
	std::vector<PidlAbsolute> validItems;

	for (const auto &item : items)
	{
		if (item.HasValue() && !ContainsItem(validItems, item))
		{
			validItems.push_back(item);
		}
	}

	auto order = std::find_if(m_itemOrders.begin(), m_itemOrders.end(),
		[this, &parent](const auto &currentOrder)
		{ return AreParentsEqual(currentOrder.parent, parent); });

	if (validItems.empty())
	{
		if (order != m_itemOrders.end())
		{
			m_itemOrders.erase(order);
			m_orderChangedSignal(parent);
		}

		return;
	}

	if (order != m_itemOrders.end())
	{
		if (order->items == validItems)
		{
			return;
		}

		order->items = std::move(validItems);
	}
	else
	{
		m_itemOrders.push_back({ parent, std::move(validItems) });
	}

	m_orderChangedSignal(parent);
}
