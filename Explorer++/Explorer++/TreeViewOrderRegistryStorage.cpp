// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#include "stdafx.h"
#include "TreeViewOrderRegistryStorage.h"
#include "TreeViewOrderModel.h"
#include "../Helper/RegistrySettings.h"
#include <wil/registry.h>
#include <vector>

namespace
{

constexpr wchar_t TREE_VIEW_ORDERS_KEY_PATH[] = L"TreeViewOrders";
constexpr wchar_t SETTING_GROUP_COUNT[] = L"GroupCount";
constexpr wchar_t SETTING_ROOT[] = L"Root";
constexpr wchar_t SETTING_PARENT_DIRECTORY[] = L"ParentDirectory";
constexpr wchar_t SETTING_ITEM_COUNT[] = L"ItemCount";
constexpr wchar_t SETTING_ITEM[] = L"Item";
constexpr DWORD MAX_TREE_VIEW_ORDER_GROUPS_TO_STORE = 100000;
constexpr DWORD MAX_TREE_VIEW_ORDER_ITEMS_TO_STORE = 100000;

bool SaveOrder(HKEY ordersKey, DWORD groupIndex, const TreeViewItemOrder &itemOrder)
{
	wil::unique_hkey groupKey;
	HRESULT hr = wil::reg::create_unique_key_nothrow(ordersKey, std::to_wstring(groupIndex).c_str(),
		groupKey, wil::reg::key_access::readwrite);

	if (FAILED(hr))
	{
		return false;
	}

	bool isRoot = !itemOrder.parent.HasValue();

	if (RegistrySettings::SaveDword(groupKey.get(), SETTING_ROOT, isRoot ? 1 : 0) != ERROR_SUCCESS)
	{
		return false;
	}

	if (!isRoot
		&& RegistrySettings::SavePidl(groupKey.get(), SETTING_PARENT_DIRECTORY,
			itemOrder.parent.Raw()) != ERROR_SUCCESS)
	{
		return false;
	}

	DWORD itemCount = 0;

	for (const auto &item : itemOrder.items)
	{
		if (itemCount >= MAX_TREE_VIEW_ORDER_ITEMS_TO_STORE)
		{
			break;
		}

		wil::unique_hkey itemKey;
		hr = wil::reg::create_unique_key_nothrow(groupKey.get(), std::to_wstring(itemCount).c_str(),
			itemKey, wil::reg::key_access::readwrite);

		if (FAILED(hr)
			|| RegistrySettings::SavePidl(itemKey.get(), SETTING_ITEM, item.Raw()) != ERROR_SUCCESS)
		{
			continue;
		}

		itemCount++;
	}

	return RegistrySettings::SaveDword(groupKey.get(), SETTING_ITEM_COUNT, itemCount)
		== ERROR_SUCCESS;
}

void LoadOrder(HKEY ordersKey, DWORD groupIndex, std::vector<TreeViewItemOrder> &itemOrders)
{
	wil::unique_hkey groupKey;
	HRESULT hr = wil::reg::open_unique_key_nothrow(ordersKey, std::to_wstring(groupIndex).c_str(),
		groupKey, wil::reg::key_access::read);

	if (FAILED(hr))
	{
		return;
	}

	DWORD isRoot;
	if (RegistrySettings::ReadDword(groupKey.get(), SETTING_ROOT, isRoot) != ERROR_SUCCESS)
	{
		return;
	}

	TreeViewItemOrder itemOrder;

	if (isRoot == 0
		&& RegistrySettings::ReadPidl(groupKey.get(), SETTING_PARENT_DIRECTORY, itemOrder.parent)
			!= ERROR_SUCCESS)
	{
		return;
	}

	DWORD itemCount;
	if (RegistrySettings::ReadDword(groupKey.get(), SETTING_ITEM_COUNT, itemCount) != ERROR_SUCCESS
		|| itemCount > MAX_TREE_VIEW_ORDER_ITEMS_TO_STORE)
	{
		return;
	}

	for (DWORD itemIndex = 0; itemIndex < itemCount; itemIndex++)
	{
		wil::unique_hkey itemKey;
		hr = wil::reg::open_unique_key_nothrow(groupKey.get(), std::to_wstring(itemIndex).c_str(),
			itemKey, wil::reg::key_access::read);

		if (FAILED(hr))
		{
			continue;
		}

		PidlAbsolute item;
		if (RegistrySettings::ReadPidl(itemKey.get(), SETTING_ITEM, item) == ERROR_SUCCESS)
		{
			itemOrder.items.push_back(std::move(item));
		}
	}

	if (!itemOrder.items.empty())
	{
		itemOrders.push_back(std::move(itemOrder));
	}
}

}

namespace TreeViewOrderRegistryStorage
{

void Load(HKEY applicationKey, TreeViewOrderModel *model)
{
	wil::unique_hkey ordersKey;
	HRESULT hr = wil::reg::open_unique_key_nothrow(applicationKey, TREE_VIEW_ORDERS_KEY_PATH,
		ordersKey, wil::reg::key_access::read);

	if (FAILED(hr))
	{
		model->SetItemOrders({});
		return;
	}

	DWORD groupCount;
	if (RegistrySettings::ReadDword(ordersKey.get(), SETTING_GROUP_COUNT, groupCount) != ERROR_SUCCESS
		|| groupCount > MAX_TREE_VIEW_ORDER_GROUPS_TO_STORE)
	{
		model->SetItemOrders({});
		return;
	}

	std::vector<TreeViewItemOrder> itemOrders;

	for (DWORD groupIndex = 0; groupIndex < groupCount; groupIndex++)
	{
		LoadOrder(ordersKey.get(), groupIndex, itemOrders);
	}

	model->SetItemOrders(itemOrders);
}

void Save(HKEY applicationKey, const TreeViewOrderModel *model)
{
	wil::unique_hkey ordersKey;
	HRESULT hr = wil::reg::create_unique_key_nothrow(applicationKey, TREE_VIEW_ORDERS_KEY_PATH,
		ordersKey, wil::reg::key_access::readwrite);

	if (FAILED(hr))
	{
		return;
	}

	DWORD groupCount = 0;

	for (const auto &itemOrder : model->GetItemOrders())
	{
		if (groupCount >= MAX_TREE_VIEW_ORDER_GROUPS_TO_STORE)
		{
			break;
		}

		if (SaveOrder(ordersKey.get(), groupCount, itemOrder))
		{
			groupCount++;
		}
	}

	RegistrySettings::SaveDword(ordersKey.get(), SETTING_GROUP_COUNT, groupCount);
}

}
