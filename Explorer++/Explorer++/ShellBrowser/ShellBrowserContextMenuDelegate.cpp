// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#include "stdafx.h"
#include "ShellBrowserContextMenuDelegate.h"
#include "ShellBrowserImpl.h"
#include "DirectoryOperationsHelper.h"
#include "MainResource.h"
#include "PinnedItemsModel.h"
#include "ResourceLoader.h"
#include "../Helper/ShellHelper.h"

ShellBrowserContextMenuDelegate::ShellBrowserContextMenuDelegate(
	WeakPtr<ShellBrowserImpl> shellBrowserWeak, PinnedItemsModel *pinnedItemsModel,
	const ResourceLoader *resourceLoader) :
	m_shellBrowserWeak(shellBrowserWeak),
	m_pinnedItemsModel(pinnedItemsModel),
	m_resourceLoader(resourceLoader)
{
}

void ShellBrowserContextMenuDelegate::UpdateMenuEntries(PCIDLIST_ABSOLUTE directory,
	const std::vector<PidlChild> &items, ShellContextMenuBuilder *builder)
{
	if (!m_shellBrowserWeak || !m_pinnedItemsModel || !m_resourceLoader || items.empty()
		|| !IsFilesystemFolder(directory))
	{
		return;
	}

	PidlAbsolute parentDirectory(directory);
	auto fullItems = BuildItemList(directory, items);
	bool hasPinnedItems = false;
	bool hasUnpinnedItems = false;

	for (const auto &item : fullItems)
	{
		if (!DoesItemHaveAttributes(item.Raw(), SFGAO_FILESYSTEM))
		{
			return;
		}

		if (m_pinnedItemsModel->IsPinned(parentDirectory, item))
		{
			hasPinnedItems = true;
		}
		else
		{
			hasUnpinnedItems = true;
		}
	}

	UINT position = 0;

	if (hasUnpinnedItems)
	{
		builder->AddStringItem(PIN_TO_TOP_MENU_ITEM_ID,
			m_resourceLoader->LoadString(IDS_PINNED_ITEMS_PIN_TO_TOP), position++, true);
	}

	if (hasPinnedItems)
	{
		builder->AddStringItem(UNPIN_FROM_TOP_MENU_ITEM_ID,
			m_resourceLoader->LoadString(IDS_PINNED_ITEMS_UNPIN_FROM_TOP), position, true);
	}
}

bool ShellBrowserContextMenuDelegate::MaybeHandleShellMenuItem(PCIDLIST_ABSOLUTE directory,
	const std::vector<PidlChild> &items, const std::wstring &verb)
{
	if (!m_shellBrowserWeak)
	{
		return false;
	}

	if (verb == L"rename")
	{
		m_shellBrowserWeak->StartRenamingItems(BuildItemList(directory, items));

		return true;
	}
	else if (verb == L"copy")
	{
		m_shellBrowserWeak->CopyItemsToClipboard(BuildItemList(directory, items),
			ClipboardAction::Copy);

		return true;
	}
	else if (verb == L"cut")
	{
		m_shellBrowserWeak->CopyItemsToClipboard(BuildItemList(directory, items),
			ClipboardAction::Cut);

		return true;
	}

	return false;
}

std::vector<PidlAbsolute> ShellBrowserContextMenuDelegate::BuildItemList(
	PCIDLIST_ABSOLUTE pidlParent, const std::vector<PidlChild> &pidlItems) const
{
	std::vector<PidlAbsolute> fullPidls;

	for (const auto &child : pidlItems)
	{
		fullPidls.push_back(pidlParent + child);
	}

	return fullPidls;
}

void ShellBrowserContextMenuDelegate::HandleCustomMenuItem(PCIDLIST_ABSOLUTE directory,
	const std::vector<PidlChild> &items, UINT menuItemId)
{
	if (!m_pinnedItemsModel || (menuItemId != PIN_TO_TOP_MENU_ITEM_ID
			&& menuItemId != UNPIN_FROM_TOP_MENU_ITEM_ID))
	{
		DCHECK(false);
		return;
	}

	bool pinned = (menuItemId == PIN_TO_TOP_MENU_ITEM_ID);
	m_pinnedItemsModel->SetItemsPinned(PidlAbsolute(directory), BuildItemList(directory, items), pinned);
}

std::wstring ShellBrowserContextMenuDelegate::GetHelpTextForCustomItem(UINT menuItemId)
{
	switch (menuItemId)
	{
	case PIN_TO_TOP_MENU_ITEM_ID:
		return m_resourceLoader->LoadString(IDS_PINNED_ITEMS_PIN_TO_TOP_HELP_TEXT);

	case UNPIN_FROM_TOP_MENU_ITEM_ID:
		return m_resourceLoader->LoadString(IDS_PINNED_ITEMS_UNPIN_FROM_TOP_HELP_TEXT);

	default:
		DCHECK(false);
		return L"";
	}
}
