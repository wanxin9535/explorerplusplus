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

void AddChangedPath(std::vector<TreeViewPath> &paths, const TreeViewPath &path)
{
	if (std::find(paths.begin(), paths.end(), path) == paths.end())
	{
		paths.push_back(path);
	}
}

bool ContainsItem(const std::vector<PidlAbsolute> &items, const PidlAbsolute &item)
{
	return std::find(items.begin(), items.end(), item) != items.end();
}

bool IsPathWithinItem(const TreeViewPath &path, const PidlAbsolute &item)
{
	return std::any_of(path.begin(), path.end(), [&item](const auto &pathItem)
		{ return item == pathItem || item.IsAncestor(pathItem); });
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
			[&itemOrder](const auto &currentOrder)
			{ return currentOrder.parentPath == itemOrder.parentPath; });

		if (existingOrder == m_itemOrders.end())
		{
			m_itemOrders.push_back({ itemOrder.parentPath, std::move(items) });
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

bool TreeViewOrderModel::HasOrder(const TreeViewPath &parentPath) const
{
	return std::any_of(m_itemOrders.begin(), m_itemOrders.end(),
		[&parentPath](const auto &itemOrder)
		{ return itemOrder.parentPath == parentPath; });
}

std::optional<size_t> TreeViewOrderModel::GetItemIndex(const TreeViewPath &parentPath,
	const PidlAbsolute &item) const
{
	auto order = std::find_if(m_itemOrders.begin(), m_itemOrders.end(),
		[&parentPath](const auto &currentOrder)
		{ return currentOrder.parentPath == parentPath; });

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

bool TreeViewOrderModel::MoveItem(const TreeViewPath &parentPath, size_t itemIndex,
	Direction direction, const std::vector<PidlAbsolute> &currentOrder)
{
	if (itemIndex >= currentOrder.size())
	{
		return false;
	}

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
	SetItemOrder(parentPath, updatedOrder);
	return true;
}

void TreeViewOrderModel::RenameItem(const PidlAbsolute &oldItem, const PidlAbsolute &newItem,
	bool isDirectory)
{
	if (!oldItem.HasValue() || !newItem.HasValue() || oldItem == newItem)
	{
		return;
	}

	std::vector<TreeViewItemOrder> updatedOrders;
	std::vector<TreeViewPath> changedPaths;

	for (const auto &itemOrder : m_itemOrders)
	{
		TreeViewItemOrder updatedOrder = itemOrder;
		bool changed = false;

		for (auto &item : updatedOrder.items)
		{
			if (item == oldItem)
			{
				item = newItem;
				changed = true;
			}
		}

		if (isDirectory)
		{
			for (auto &pathItem : updatedOrder.parentPath)
			{
				if (pathItem == oldItem || oldItem.IsAncestor(pathItem))
				{
					auto rebasedPathItem = RebasePidl(oldItem, newItem, pathItem);

					if (rebasedPathItem)
					{
						pathItem = *rebasedPathItem;
						changed = true;
					}
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
			AddChangedPath(changedPaths, itemOrder.parentPath);
			AddChangedPath(changedPaths, updatedOrder.parentPath);
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
			[&itemOrder](const auto &currentOrder)
			{ return currentOrder.parentPath == itemOrder.parentPath; });

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

	if (changedPaths.empty())
	{
		return;
	}

	m_itemOrders = std::move(mergedOrders);

	for (const auto &changedPath : changedPaths)
	{
		m_orderChangedSignal(changedPath);
	}
}

void TreeViewOrderModel::RemoveItem(const TreeViewPath &parentPath,
	const PidlAbsolute &item, bool isDirectory)
{
	if (!item.HasValue())
	{
		return;
	}

	std::vector<TreeViewPath> changedPaths;

	for (auto orderItr = m_itemOrders.begin(); orderItr != m_itemOrders.end();)
	{
		if (isDirectory && IsPathWithinItem(orderItr->parentPath, item))
		{
			AddChangedPath(changedPaths, orderItr->parentPath);
			orderItr = m_itemOrders.erase(orderItr);
			continue;
		}

		bool changed = false;

		for (auto itemItr = orderItr->items.begin(); itemItr != orderItr->items.end();)
		{
			bool remove = *itemItr == item || (isDirectory && item.IsAncestor(*itemItr));

			if (remove)
			{
				itemItr = orderItr->items.erase(itemItr);
				changed = true;
			}
			else
			{
				++itemItr;
			}
		}

		if (changed)
		{
			AddChangedPath(changedPaths, orderItr->parentPath);
		}

		if (orderItr->items.empty())
		{
			orderItr = m_itemOrders.erase(orderItr);
			continue;
		}

		++orderItr;
	}

	if (changedPaths.empty())
	{
		// The parent path is included so the affected branch is re-sorted even if no explicit order
		// entry remains for it.
		AddChangedPath(changedPaths, parentPath);
	}

	for (const auto &changedPath : changedPaths)
	{
		m_orderChangedSignal(changedPath);
	}
}

boost::signals2::connection TreeViewOrderModel::AddOrderChangedObserver(
	const OrderChangedSignal::slot_type &observer)
{
	return m_orderChangedSignal.connect(observer);
}

void TreeViewOrderModel::SetItemOrder(const TreeViewPath &parentPath,
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
		[&parentPath](const auto &currentOrder)
		{ return currentOrder.parentPath == parentPath; });

	if (validItems.empty())
	{
		if (order != m_itemOrders.end())
		{
			m_itemOrders.erase(order);
			m_orderChangedSignal(parentPath);
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
		m_itemOrders.push_back({ parentPath, std::move(validItems) });
	}

	m_orderChangedSignal(parentPath);
}
