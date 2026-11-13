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

constexpr wchar_t TREE_VIEW_ORDERS_KEY_PATH[] = L"TreeViewOrdersV2";
constexpr wchar_t SETTING_GROUP_COUNT[] = L"GroupCount";
constexpr wchar_t SETTING_PARENT_PATH_COUNT[] = L"ParentPathCount";
constexpr wchar_t SETTING_ITEM_COUNT[] = L"ItemCount";
constexpr wchar_t SETTING_PIDLS[] = L"Pidls";
constexpr DWORD MAX_TREE_VIEW_ORDER_GROUPS_TO_STORE = 100000;
constexpr DWORD MAX_TREE_VIEW_ORDER_ITEMS_TO_STORE = 100000;
constexpr DWORD MAX_TREE_VIEW_PARENT_PATH_LENGTH = 1024;

bool SavePidls(HKEY groupKey, const wchar_t *settingName,
	const std::vector<PidlAbsolute> &pidls, DWORD maximumCount, DWORD &savedCount)
{
	wil::unique_hkey pidlsKey;
	HRESULT hr = wil::reg::create_unique_key_nothrow(groupKey, settingName, pidlsKey,
		wil::reg::key_access::readwrite);

	if (FAILED(hr))
	{
		return false;
	}

	savedCount = 0;

	for (const auto &pidl : pidls)
	{
		if (savedCount >= maximumCount)
		{
			break;
		}

		wil::unique_hkey itemKey;
		hr = wil::reg::create_unique_key_nothrow(pidlsKey.get(), std::to_wstring(savedCount).c_str(),
			itemKey, wil::reg::key_access::readwrite);

		if (FAILED(hr)
			|| RegistrySettings::SavePidl(itemKey.get(), L"Pidl", pidl.Raw()) != ERROR_SUCCESS)
		{
			continue;
		}

		savedCount++;
	}

	return true;
}

std::vector<PidlAbsolute> LoadPidls(HKEY groupKey, const wchar_t *settingName, DWORD itemCount,
	DWORD maximumCount)
{
	if (itemCount > maximumCount)
	{
		return {};
	}

	wil::unique_hkey pidlsKey;
	HRESULT hr = wil::reg::open_unique_key_nothrow(groupKey, settingName, pidlsKey,
		wil::reg::key_access::read);

	if (FAILED(hr))
	{
		return {};
	}

	std::vector<PidlAbsolute> pidls;

	for (DWORD itemIndex = 0; itemIndex < itemCount; itemIndex++)
	{
		wil::unique_hkey itemKey;
		hr = wil::reg::open_unique_key_nothrow(pidlsKey.get(), std::to_wstring(itemIndex).c_str(),
			itemKey, wil::reg::key_access::read);

		if (FAILED(hr))
		{
			continue;
		}

		PidlAbsolute pidl;
		if (RegistrySettings::ReadPidl(itemKey.get(), L"Pidl", pidl) == ERROR_SUCCESS)
		{
			pidls.push_back(std::move(pidl));
		}
	}

	return pidls;
}

bool SaveOrder(HKEY ordersKey, DWORD groupIndex, const TreeViewItemOrder &itemOrder)
{
	wil::unique_hkey groupKey;
	HRESULT hr = wil::reg::create_unique_key_nothrow(ordersKey, std::to_wstring(groupIndex).c_str(),
		groupKey, wil::reg::key_access::readwrite);

	if (FAILED(hr))
	{
		return false;
	}

	DWORD parentPathCount = 0;
	if (!SavePidls(groupKey.get(), L"ParentPath", itemOrder.parentPath,
			MAX_TREE_VIEW_PARENT_PATH_LENGTH, parentPathCount)
		|| RegistrySettings::SaveDword(groupKey.get(), SETTING_PARENT_PATH_COUNT,
			parentPathCount) != ERROR_SUCCESS)
	{
		return false;
	}

	DWORD itemCount = 0;
	return SavePidls(groupKey.get(), SETTING_PIDLS, itemOrder.items,
		MAX_TREE_VIEW_ORDER_ITEMS_TO_STORE, itemCount)
		&& RegistrySettings::SaveDword(groupKey.get(), SETTING_ITEM_COUNT, itemCount)
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

	DWORD parentPathCount = 0;
	DWORD itemCount = 0;

	if (RegistrySettings::ReadDword(groupKey.get(), SETTING_PARENT_PATH_COUNT, parentPathCount)
			!= ERROR_SUCCESS
		|| parentPathCount > MAX_TREE_VIEW_PARENT_PATH_LENGTH
		|| RegistrySettings::ReadDword(groupKey.get(), SETTING_ITEM_COUNT, itemCount)
			!= ERROR_SUCCESS
		|| itemCount > MAX_TREE_VIEW_ORDER_ITEMS_TO_STORE)
	{
		return;
	}

	TreeViewItemOrder itemOrder;
	itemOrder.parentPath = LoadPidls(groupKey.get(), L"ParentPath", parentPathCount,
		MAX_TREE_VIEW_PARENT_PATH_LENGTH);

	if (itemOrder.parentPath.size() != parentPathCount)
	{
		return;
	}

	itemOrder.items = LoadPidls(groupKey.get(), SETTING_PIDLS, itemCount,
		MAX_TREE_VIEW_ORDER_ITEMS_TO_STORE);

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
		// Version 1 stored only a parent PIDL and cannot distinguish repeated namespace entries.
		model->SetItemOrders({});
		return;
	}

	DWORD groupCount = 0;
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
