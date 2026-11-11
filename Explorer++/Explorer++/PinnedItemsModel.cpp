// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#include "stdafx.h"
#include "PinnedItemsModel.h"
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

void AddChangedDirectory(std::vector<PidlAbsolute> &directories, const PidlAbsolute &directory)
{
	if (std::find(directories.begin(), directories.end(), directory) == directories.end())
	{
		directories.push_back(directory);
	}
}

}

void PinnedItemsModel::SetPinnedItems(const std::vector<PinnedItem> &pinnedItems)
{
	m_pinnedItems.clear();

	for (const auto &pinnedItem : pinnedItems)
	{
		if (pinnedItem.parentDirectory.HasValue() && pinnedItem.item.HasValue())
		{
			m_pinnedItems.insert(pinnedItem);
		}
	}
}

std::vector<PinnedItem> PinnedItemsModel::GetPinnedItems() const
{
	return std::vector<PinnedItem>(m_pinnedItems.begin(), m_pinnedItems.end());
}

bool PinnedItemsModel::IsPinned(const PidlAbsolute &parentDirectory, const PidlAbsolute &item) const
{
	return m_pinnedItems.contains(PinnedItem{ parentDirectory, item });
}

void PinnedItemsModel::SetItemsPinned(const PidlAbsolute &parentDirectory,
	const std::vector<PidlAbsolute> &items, bool pinned)
{
	if (!parentDirectory.HasValue())
	{
		return;
	}

	bool changed = false;

	for (const auto &item : items)
	{
		if (!item.HasValue())
		{
			continue;
		}

		PinnedItem pinnedItem{ parentDirectory, item };

		if (pinned)
		{
			changed |= m_pinnedItems.insert(pinnedItem).second;
		}
		else
		{
			changed |= (m_pinnedItems.erase(pinnedItem) > 0);
		}
	}

	if (changed)
	{
		m_pinnedItemsChangedSignal(parentDirectory);
	}
}

void PinnedItemsModel::RenameItem(const PidlAbsolute &parentDirectory,
	const PidlAbsolute &oldItem, const PidlAbsolute &newItem, bool isDirectory)
{
	if (!parentDirectory.HasValue() || !oldItem.HasValue() || !newItem.HasValue()
		|| oldItem == newItem)
	{
		return;
	}

	PinnedItems updatedItems;
	std::vector<PidlAbsolute> changedDirectories;
	bool changed = false;

	for (const auto &pinnedItem : m_pinnedItems)
	{
		PinnedItem updatedItem = pinnedItem;
		bool itemChanged = false;

		if (pinnedItem.parentDirectory == parentDirectory && pinnedItem.item == oldItem)
		{
			updatedItem.item = newItem;
			itemChanged = true;
		}

		if (isDirectory && (pinnedItem.parentDirectory == oldItem
				|| oldItem.IsAncestor(pinnedItem.parentDirectory)))
		{
			auto rebasedParent = RebasePidl(oldItem, newItem, pinnedItem.parentDirectory);
			auto rebasedItem = RebasePidl(oldItem, newItem, pinnedItem.item);

			if (rebasedParent && rebasedItem)
			{
				updatedItem.parentDirectory = *rebasedParent;
				updatedItem.item = *rebasedItem;
				itemChanged = true;
			}
			else
			{
				AddChangedDirectory(changedDirectories, pinnedItem.parentDirectory);
				changed = true;
				continue;
			}
		}

		if (itemChanged)
		{
			AddChangedDirectory(changedDirectories, pinnedItem.parentDirectory);
			AddChangedDirectory(changedDirectories, updatedItem.parentDirectory);
			changed = true;
		}

		updatedItems.insert(std::move(updatedItem));
	}

	if (!changed)
	{
		return;
	}

	m_pinnedItems.swap(updatedItems);

	for (const auto &directory : changedDirectories)
	{
		m_pinnedItemsChangedSignal(directory);
	}
}

void PinnedItemsModel::RemoveItem(const PidlAbsolute &parentDirectory, const PidlAbsolute &item,
	bool isDirectory)
{
	if (!parentDirectory.HasValue() || !item.HasValue())
	{
		return;
	}

	PinnedItems remainingItems;
	std::vector<PidlAbsolute> changedDirectories;
	bool changed = false;

	for (const auto &pinnedItem : m_pinnedItems)
	{
		bool remove = pinnedItem.parentDirectory == parentDirectory && pinnedItem.item == item;

		if (isDirectory && (pinnedItem.parentDirectory == item
				|| item.IsAncestor(pinnedItem.parentDirectory)))
		{
			remove = true;
		}

		if (remove)
		{
			AddChangedDirectory(changedDirectories, pinnedItem.parentDirectory);
			changed = true;
		}
		else
		{
			remainingItems.insert(pinnedItem);
		}
	}

	if (!changed)
	{
		return;
	}

	m_pinnedItems.swap(remainingItems);

	for (const auto &directory : changedDirectories)
	{
		m_pinnedItemsChangedSignal(directory);
	}
}

boost::signals2::connection PinnedItemsModel::AddPinnedItemsChangedObserver(
	const PinnedItemsChangedSignal::slot_type &observer)
{
	return m_pinnedItemsChangedSignal.connect(observer);
}
