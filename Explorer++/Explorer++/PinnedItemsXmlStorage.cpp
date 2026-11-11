// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#include "stdafx.h"
#include "PinnedItemsXmlStorage.h"
#include "PinnedItemsModel.h"
#include "../Helper/PidlHelper.h"
#include "../Helper/StringHelper.h"
#include "../Helper/XMLSettings.h"
#include <wil/com.h>
#include <optional>
#include <vector>

namespace
{

constexpr wchar_t PINNED_ITEMS_NODE_NAME[] = L"PinnedItems";
constexpr wchar_t PINNED_ITEM_NODE_NAME[] = L"PinnedItem";
constexpr wchar_t PARENT_DIRECTORY_ATTRIBUTE[] = L"ParentDirectory";
constexpr wchar_t ITEM_ATTRIBUTE[] = L"Item";

std::optional<PidlAbsolute> LoadPidl(IXMLDOMNamedNodeMap *attributeMap,
	const wchar_t *attributeName)
{
	std::wstring encodedPidl;
	HRESULT hr = XMLSettings::GetStringFromMap(attributeMap, attributeName, encodedPidl);

	if (hr != S_OK)
	{
		return std::nullopt;
	}

	auto encodedPidlNarrow = WstrToStr(encodedPidl);

	if (!encodedPidlNarrow)
	{
		return std::nullopt;
	}

	auto pidl = DecodePidlFromBase64(*encodedPidlNarrow);

	if (!pidl.HasValue())
	{
		return std::nullopt;
	}

	return pidl;
}

std::optional<PinnedItem> LoadPinnedItem(IXMLDOMNode *pinnedItemNode)
{
	wil::com_ptr_nothrow<IXMLDOMNamedNodeMap> attributeMap;
	HRESULT hr = pinnedItemNode->get_attributes(&attributeMap);

	if (hr != S_OK)
	{
		return std::nullopt;
	}

	auto parentDirectory = LoadPidl(attributeMap.get(), PARENT_DIRECTORY_ATTRIBUTE);
	auto item = LoadPidl(attributeMap.get(), ITEM_ATTRIBUTE);

	if (!parentDirectory || !item)
	{
		return std::nullopt;
	}

	return PinnedItem{ *parentDirectory, *item };
}

void LoadFromNode(IXMLDOMNode *pinnedItemsNode, PinnedItemsModel *model)
{
	wil::com_ptr_nothrow<IXMLDOMNodeList> pinnedItemNodes;
	auto queryString = wil::make_bstr_nothrow(PINNED_ITEM_NODE_NAME);
	HRESULT hr = pinnedItemsNode->selectNodes(queryString.get(), &pinnedItemNodes);

	if (hr != S_OK)
	{
		model->SetPinnedItems({});
		return;
	}

	std::vector<PinnedItem> pinnedItems;
	wil::com_ptr_nothrow<IXMLDOMNode> pinnedItemNode;

	while (pinnedItemNodes->nextNode(&pinnedItemNode) == S_OK)
	{
		auto pinnedItem = LoadPinnedItem(pinnedItemNode.get());

		if (pinnedItem)
		{
			pinnedItems.push_back(*pinnedItem);
		}
	}

	model->SetPinnedItems(pinnedItems);
}

bool AddPidlAttribute(IXMLDOMDocument *xmlDocument, IXMLDOMElement *pinnedItemNode,
	const wchar_t *attributeName, const PidlAbsolute &pidl)
{
	auto encodedPidl = EncodePidlToBase64(pidl);
	auto encodedPidlWide = StrToWstr(encodedPidl);

	if (!encodedPidlWide)
	{
		return false;
	}

	XMLSettings::AddAttributeToNode(xmlDocument, pinnedItemNode, attributeName,
		*encodedPidlWide);
	return true;
}

void SaveToNode(IXMLDOMDocument *xmlDocument, IXMLDOMElement *pinnedItemsNode,
	const PinnedItemsModel *model)
{
	for (const auto &pinnedItem : model->GetPinnedItems())
	{
		wil::com_ptr_nothrow<IXMLDOMElement> pinnedItemNode;
		auto nodeName = wil::make_bstr_nothrow(PINNED_ITEM_NODE_NAME);
		HRESULT hr = xmlDocument->createElement(nodeName.get(), &pinnedItemNode);

		if (hr != S_OK)
		{
			continue;
		}

		if (!AddPidlAttribute(xmlDocument, pinnedItemNode.get(), PARENT_DIRECTORY_ATTRIBUTE,
			pinnedItem.parentDirectory)
			|| !AddPidlAttribute(xmlDocument, pinnedItemNode.get(), ITEM_ATTRIBUTE, pinnedItem.item))
		{
			continue;
		}

		XMLSettings::AppendChildToParent(pinnedItemNode.get(), pinnedItemsNode);
	}
}

}

namespace PinnedItemsXmlStorage
{

void Load(IXMLDOMNode *rootNode, PinnedItemsModel *model)
{
	wil::com_ptr_nothrow<IXMLDOMNode> pinnedItemsNode;
	auto queryString = wil::make_bstr_nothrow(PINNED_ITEMS_NODE_NAME);
	HRESULT hr = rootNode->selectSingleNode(queryString.get(), &pinnedItemsNode);

	if (hr != S_OK)
	{
		model->SetPinnedItems({});
		return;
	}

	LoadFromNode(pinnedItemsNode.get(), model);
}

void Save(IXMLDOMDocument *xmlDocument, IXMLDOMNode *rootNode, const PinnedItemsModel *model)
{
	wil::com_ptr_nothrow<IXMLDOMElement> pinnedItemsNode;
	auto nodeName = wil::make_bstr_nothrow(PINNED_ITEMS_NODE_NAME);
	HRESULT hr = xmlDocument->createElement(nodeName.get(), &pinnedItemsNode);

	if (hr != S_OK)
	{
		return;
	}

	SaveToNode(xmlDocument, pinnedItemsNode.get(), model);
	XMLSettings::AppendChildToParent(pinnedItemsNode.get(), rootNode);
}

}
