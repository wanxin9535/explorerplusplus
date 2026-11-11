// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#pragma once

#include "../Helper/ShellItemContextMenuDelegate.h"
#include "../Helper/WeakPtr.h"

class PinnedItemsModel;
class ResourceLoader;
class ShellBrowserImpl;

// When the shell context menu is being shown for one or more items in a ShellBrowserImpl instance,
// this class handles the view-specific items (e.g. the "rename" item is view-specific, since it
// acts on the view).
class ShellBrowserContextMenuDelegate : public ShellItemContextMenuDelegate
{
public:
	ShellBrowserContextMenuDelegate(WeakPtr<ShellBrowserImpl> shellBrowserWeak,
		PinnedItemsModel *pinnedItemsModel, const ResourceLoader *resourceLoader);

	void UpdateMenuEntries(PCIDLIST_ABSOLUTE directory, const std::vector<PidlChild> &items,
		ShellContextMenuBuilder *builder) override;
	bool MaybeHandleShellMenuItem(PCIDLIST_ABSOLUTE directory, const std::vector<PidlChild> &items,
		const std::wstring &verb) override;
	void HandleCustomMenuItem(PCIDLIST_ABSOLUTE directory, const std::vector<PidlChild> &items,
		UINT menuItemId) override;
	std::wstring GetHelpTextForCustomItem(UINT menuItemId) override;

private:
	static constexpr UINT PIN_TO_TOP_MENU_ITEM_ID = 1;
	static constexpr UINT UNPIN_FROM_TOP_MENU_ITEM_ID = 2;

	std::vector<PidlAbsolute> BuildItemList(PCIDLIST_ABSOLUTE pidlParent,
		const std::vector<PidlChild> &pidlItems) const;

	const WeakPtr<ShellBrowserImpl> m_shellBrowserWeak;
	PinnedItemsModel *const m_pinnedItemsModel;
	const ResourceLoader *const m_resourceLoader;
};
