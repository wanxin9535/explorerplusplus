// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#include "stdafx.h"
// clang-format off
#include "ShellTreeViewContextMenuDelegate.h"
// clang-format on
#include "ShellTreeView.h"
#include "MainResource.h"
#include "ResourceLoader.h"
#include "../Helper/Pidl.h"
#include "../Helper/ShellContextMenuBuilder.h"

namespace
{

constexpr UINT MOVE_ITEM_UP_MENU_ITEM_ID = 1;
constexpr UINT MOVE_ITEM_DOWN_MENU_ITEM_ID = 2;

}

ShellTreeViewContextMenuDelegate::ShellTreeViewContextMenuDelegate(ShellTreeView *shellTreeView,
	const ResourceLoader *resourceLoader) :
	m_shellTreeView(shellTreeView),
	m_resourceLoader(resourceLoader)
{
}

void ShellTreeViewContextMenuDelegate::UpdateMenuEntries(PCIDLIST_ABSOLUTE directory,
	const std::vector<PidlChild> &items, ShellContextMenuBuilder *builder)
{
	if (items.size() != 1)
	{
		return;
	}

	PidlAbsolute item = directory + items[0];
	std::wstring text = m_resourceLoader->LoadString(IDS_SHELL_TREE_VIEW_MOVE_UP);
	builder->AddStringItem(MOVE_ITEM_UP_MENU_ITEM_ID, text, 0, true);
	builder->EnableItem(MOVE_ITEM_UP_MENU_ITEM_ID,
		m_shellTreeView->CanMoveItemOneStep(item.Raw(), ShellTreeView::MoveDirection::Up));

	text = m_resourceLoader->LoadString(IDS_SHELL_TREE_VIEW_MOVE_DOWN);
	builder->AddStringItem(MOVE_ITEM_DOWN_MENU_ITEM_ID, text, 1, true);
	builder->EnableItem(MOVE_ITEM_DOWN_MENU_ITEM_ID,
		m_shellTreeView->CanMoveItemOneStep(item.Raw(), ShellTreeView::MoveDirection::Down));

	builder->AddSeparator(2, true);
}

bool ShellTreeViewContextMenuDelegate::MaybeHandleShellMenuItem(PCIDLIST_ABSOLUTE directory,
	const std::vector<PidlChild> &items, const std::wstring &verb)
{
	// The context menu should only ever be shown for a single item in the treeview.
	CHECK_EQ(items.size(), 1u);

	auto pidlComplete = directory + items[0];

	if (verb == L"rename")
	{
		m_shellTreeView->StartRenamingItem(pidlComplete.Raw());

		return true;
	}
	else if (verb == L"copy")
	{
		m_shellTreeView->CopyItemToClipboard(pidlComplete.Raw(), ClipboardAction::Copy);

		return true;
	}
	else if (verb == L"cut")
	{
		m_shellTreeView->CopyItemToClipboard(pidlComplete.Raw(), ClipboardAction::Cut);

		return true;
	}

	return false;
}

void ShellTreeViewContextMenuDelegate::HandleCustomMenuItem(PCIDLIST_ABSOLUTE directory,
	const std::vector<PidlChild> &items, UINT menuItemId)
{
	CHECK_EQ(items.size(), 1u);

	PidlAbsolute item = directory + items[0];

	switch (menuItemId)
	{
	case MOVE_ITEM_UP_MENU_ITEM_ID:
		m_shellTreeView->MoveItemOneStep(item.Raw(), ShellTreeView::MoveDirection::Up);
		break;

	case MOVE_ITEM_DOWN_MENU_ITEM_ID:
		m_shellTreeView->MoveItemOneStep(item.Raw(), ShellTreeView::MoveDirection::Down);
		break;

	default:
		DCHECK(false);
		break;
	}
}

std::wstring ShellTreeViewContextMenuDelegate::GetHelpTextForCustomItem(UINT menuItemId)
{
	UNREFERENCED_PARAMETER(menuItemId);

	return L"";
}
