// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#include "stdafx.h"
#include "PinnedItemsRegistryStorage.h"
#include "PinnedItemsModel.h"
#include "../Helper/RegistrySettings.h"
#include <optional>
#include <wil/registry.h>
#include <vector>

namespace
{

constexpr wchar_t PINNED_ITEMS_KEY_PATH[] = L"PinnedItems";
constexpr wchar_t SETTING_COUNT[] = L"Count";
constexpr wchar_t SETTING_PARENT_DIRECTORY[] = L"ParentDirectory";
constexpr wchar_t SETTING_ITEM[] = L"Item";
constexpr DWORD MAX_PINNED_ITEMS_TO_STORE = 100000;

std::optional<PinnedItem> LoadPinnedItem(HKEY pinnedItemKey)
{
	PinnedItem pinnedItem;
	auto res = RegistrySettings::ReadPidl(pinnedItemKey, SETTING_PARENT_DIRECTORY,
		pinnedItem.parentDirectory);

	if (res != ERROR_SUCCESS)
	{
		return std::nullopt;
	}

	res = RegistrySettings::ReadPidl(pinnedItemKey, SETTING_ITEM, pinnedItem.item);

	if (res != ERROR_SUCCESS)
	{
		return std::nullopt;
	}

	return pinnedItem;
}

void LoadFromKey(HKEY pinnedItemsKey, PinnedItemsModel *model)
{
	DWORD itemCount;
	auto res = RegistrySettings::ReadDword(pinnedItemsKey, SETTING_COUNT, itemCount);

	if (res != ERROR_SUCCESS || itemCount > MAX_PINNED_ITEMS_TO_STORE)
	{
		model->SetPinnedItems({});
		return;
	}

	std::vector<PinnedItem> pinnedItems;

	for (DWORD index = 0; index < itemCount; index++)
	{
		wil::unique_hkey childKey;
		HRESULT hr = wil::reg::open_unique_key_nothrow(pinnedItemsKey,
			std::to_wstring(index).c_str(), childKey);

		if (FAILED(hr))
		{
			continue;
		}

		auto pinnedItem = LoadPinnedItem(childKey.get());

		if (pinnedItem)
		{
			pinnedItems.push_back(*pinnedItem);
		}
	}

	model->SetPinnedItems(pinnedItems);
}

bool SavePinnedItem(HKEY pinnedItemKey, const PinnedItem &pinnedItem)
{
	return RegistrySettings::SavePidl(pinnedItemKey, SETTING_PARENT_DIRECTORY,
		pinnedItem.parentDirectory.Raw()) == ERROR_SUCCESS
		&& RegistrySettings::SavePidl(pinnedItemKey, SETTING_ITEM, pinnedItem.item.Raw())
			== ERROR_SUCCESS;
}

void SaveToKey(HKEY pinnedItemsKey, const PinnedItemsModel *model)
{
	DWORD itemCount = 0;

	for (const auto &pinnedItem : model->GetPinnedItems())
	{
		if (itemCount >= MAX_PINNED_ITEMS_TO_STORE)
		{
			break;
		}

		wil::unique_hkey childKey;
		HRESULT hr = wil::reg::create_unique_key_nothrow(pinnedItemsKey,
			std::to_wstring(itemCount).c_str(), childKey, wil::reg::key_access::readwrite);

		if (FAILED(hr) || !SavePinnedItem(childKey.get(), pinnedItem))
		{
			continue;
		}

		itemCount++;
	}

	RegistrySettings::SaveDword(pinnedItemsKey, SETTING_COUNT, itemCount);
}

}

namespace PinnedItemsRegistryStorage
{

void Load(HKEY applicationKey, PinnedItemsModel *model)
{
	wil::unique_hkey pinnedItemsKey;
	HRESULT hr = wil::reg::open_unique_key_nothrow(applicationKey, PINNED_ITEMS_KEY_PATH,
		pinnedItemsKey, wil::reg::key_access::read);

	if (FAILED(hr))
	{
		model->SetPinnedItems({});
		return;
	}

	LoadFromKey(pinnedItemsKey.get(), model);
}

void Save(HKEY applicationKey, const PinnedItemsModel *model)
{
	wil::unique_hkey pinnedItemsKey;
	HRESULT hr = wil::reg::create_unique_key_nothrow(applicationKey, PINNED_ITEMS_KEY_PATH,
		pinnedItemsKey, wil::reg::key_access::readwrite);

	if (FAILED(hr))
	{
		return;
	}

	SaveToKey(pinnedItemsKey.get(), model);
}

}
