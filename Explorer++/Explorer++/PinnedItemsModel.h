// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#pragma once

#include "../Helper/Pidl.h"
#include <boost/functional/hash.hpp>
#include <boost/signals2.hpp>
#include <unordered_set>
#include <vector>

struct PinnedItem
{
	PidlAbsolute parentDirectory;
	PidlAbsolute item;

	bool operator==(const PinnedItem &) const = default;
};

// Stores filesystem items that should appear before unpinned items in their parent directory.
class PinnedItemsModel
{
public:
	using PinnedItemsChangedSignal = boost::signals2::signal<void(const PidlAbsolute &)>;

	void SetPinnedItems(const std::vector<PinnedItem> &pinnedItems);
	std::vector<PinnedItem> GetPinnedItems() const;
	bool IsPinned(const PidlAbsolute &parentDirectory, const PidlAbsolute &item) const;
	void SetItemsPinned(const PidlAbsolute &parentDirectory,
		const std::vector<PidlAbsolute> &items, bool pinned);
	void RenameItem(const PidlAbsolute &parentDirectory, const PidlAbsolute &oldItem,
		const PidlAbsolute &newItem, bool isDirectory);
	void RemoveItem(const PidlAbsolute &parentDirectory, const PidlAbsolute &item,
		bool isDirectory);
	boost::signals2::connection AddPinnedItemsChangedObserver(
		const PinnedItemsChangedSignal::slot_type &observer);

private:
	struct PinnedItemHash
	{
		std::size_t operator()(const PinnedItem &pinnedItem) const
		{
			std::size_t seed = boost::hash<PidlAbsolute>{}(pinnedItem.parentDirectory);
			boost::hash_combine(seed, pinnedItem.item);
			return seed;
		}
	};

	using PinnedItems = std::unordered_set<PinnedItem, PinnedItemHash>;

	PinnedItems m_pinnedItems;
	PinnedItemsChangedSignal m_pinnedItemsChangedSignal;
};
