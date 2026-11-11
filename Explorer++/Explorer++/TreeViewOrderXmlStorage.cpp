// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#include "stdafx.h"
#include "TreeViewOrderXmlStorage.h"
#include "TreeViewOrderModel.h"
#include "../Helper/PidlHelper.h"
#include "../Helper/StringHelper.h"
#include "../Helper/XMLSettings.h"
#include <wil/com.h>
#include <optional>
#include <vector>

namespace
{

constexpr wchar_t TREE_VIEW_ORDERS_NODE_NAME[] = L"TreeViewOrders";
constexpr wchar_t TREE_VIEW_ORDER_NODE_NAME[] = L"TreeViewOrder";
constexpr wchar_t ITEM_NODE_NAME[] = L"Item";
constexpr wchar_t ROOT_ATTRIBUTE[] = L"Root";
constexpr wchar_t PARENT_DIRECTORY_ATTRIBUTE[] = L"ParentDirectory";
constexpr wchar_t ITEM_ATTRIBUTE[] = L"Pidl";

std::optional<PidlAbsolute> LoadPidl(IXMLDOMNamedNodeMap *attributeMap, const wchar_t *attributeName)
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

std::optional<PidlAbsolute> LoadItem(IXMLDOMNode *itemNode)
{
	wil::com_ptr_nothrow<IXMLDOMNamedNodeMap> attributeMap;
	HRESULT hr = itemNode->get_attributes(&attributeMap);

	if (hr != S_OK)
	{
		return std::nullopt;
	}

	return LoadPidl(attributeMap.get(), ITEM_ATTRIBUTE);
}

std::optional<TreeViewItemOrder> LoadOrder(IXMLDOMNode *orderNode)
{
	wil::com_ptr_nothrow<IXMLDOMNamedNodeMap> attributeMap;
	HRESULT hr = orderNode->get_attributes(&attributeMap);

	if (hr != S_OK)
	{
		return std::nullopt;
	}

	std::wstring rootValue;
	hr = XMLSettings::GetStringFromMap(attributeMap.get(), ROOT_ATTRIBUTE, rootValue);

	TreeViewItemOrder itemOrder;

	if (hr != S_OK || rootValue != L"true")
	{
		auto parentDirectory = LoadPidl(attributeMap.get(), PARENT_DIRECTORY_ATTRIBUTE);

		if (!parentDirectory)
		{
			return std::nullopt;
		}

		itemOrder.parent = *parentDirectory;
	}

	wil::com_ptr_nothrow<IXMLDOMNodeList> itemNodes;
	auto queryString = wil::make_bstr_nothrow(ITEM_NODE_NAME);
	hr = orderNode->selectNodes(queryString.get(), &itemNodes);

	if (hr != S_OK)
	{
		return std::nullopt;
	}

	wil::com_ptr_nothrow<IXMLDOMNode> itemNode;

	while (itemNodes->nextNode(&itemNode) == S_OK)
	{
		auto item = LoadItem(itemNode.get());

		if (item)
		{
			itemOrder.items.push_back(*item);
		}
	}

	if (itemOrder.items.empty())
	{
		return std::nullopt;
	}

	return itemOrder;
}

void LoadFromNode(IXMLDOMNode *ordersNode, TreeViewOrderModel *model)
{
	wil::com_ptr_nothrow<IXMLDOMNodeList> orderNodes;
	auto queryString = wil::make_bstr_nothrow(TREE_VIEW_ORDER_NODE_NAME);
	HRESULT hr = ordersNode->selectNodes(queryString.get(), &orderNodes);

	if (hr != S_OK)
	{
		model->SetItemOrders({});
		return;
	}

	std::vector<TreeViewItemOrder> itemOrders;
	wil::com_ptr_nothrow<IXMLDOMNode> orderNode;

	while (orderNodes->nextNode(&orderNode) == S_OK)
	{
		auto itemOrder = LoadOrder(orderNode.get());

		if (itemOrder)
		{
			itemOrders.push_back(std::move(*itemOrder));
		}
	}

	model->SetItemOrders(itemOrders);
}

bool AddPidlAttribute(IXMLDOMDocument *xmlDocument, IXMLDOMElement *node,
	const wchar_t *attributeName, const PidlAbsolute &pidl)
{
	auto encodedPidl = EncodePidlToBase64(pidl);
	auto encodedPidlWide = StrToWstr(encodedPidl);

	if (!encodedPidlWide)
	{
		return false;
	}

	XMLSettings::AddAttributeToNode(xmlDocument, node, attributeName, *encodedPidlWide);
	return true;
}

void SaveOrder(IXMLDOMDocument *xmlDocument, IXMLDOMElement *ordersNode,
	const TreeViewItemOrder &itemOrder)
{
	wil::com_ptr_nothrow<IXMLDOMElement> orderNode;
	auto orderNodeName = wil::make_bstr_nothrow(TREE_VIEW_ORDER_NODE_NAME);
	HRESULT hr = xmlDocument->createElement(orderNodeName.get(), &orderNode);

	if (hr != S_OK)
	{
		return;
	}

	if (!itemOrder.parent.HasValue())
	{
		XMLSettings::AddAttributeToNode(xmlDocument, orderNode.get(), ROOT_ATTRIBUTE, L"true");
	}
	else if (!AddPidlAttribute(xmlDocument, orderNode.get(), PARENT_DIRECTORY_ATTRIBUTE,
			itemOrder.parent))
	{
		return;
	}

	for (const auto &item : itemOrder.items)
	{
		wil::com_ptr_nothrow<IXMLDOMElement> itemNode;
		auto itemNodeName = wil::make_bstr_nothrow(ITEM_NODE_NAME);
		hr = xmlDocument->createElement(itemNodeName.get(), &itemNode);

		if (hr != S_OK || !AddPidlAttribute(xmlDocument, itemNode.get(), ITEM_ATTRIBUTE, item))
		{
			continue;
		}

		XMLSettings::AppendChildToParent(itemNode.get(), orderNode.get());
	}

	XMLSettings::AppendChildToParent(orderNode.get(), ordersNode);
}

}

namespace TreeViewOrderXmlStorage
{

void Load(IXMLDOMNode *rootNode, TreeViewOrderModel *model)
{
	wil::com_ptr_nothrow<IXMLDOMNode> ordersNode;
	auto queryString = wil::make_bstr_nothrow(TREE_VIEW_ORDERS_NODE_NAME);
	HRESULT hr = rootNode->selectSingleNode(queryString.get(), &ordersNode);

	if (hr != S_OK)
	{
		model->SetItemOrders({});
		return;
	}

	LoadFromNode(ordersNode.get(), model);
}

void Save(IXMLDOMDocument *xmlDocument, IXMLDOMNode *rootNode,
	const TreeViewOrderModel *model)
{
	wil::com_ptr_nothrow<IXMLDOMElement> ordersNode;
	auto nodeName = wil::make_bstr_nothrow(TREE_VIEW_ORDERS_NODE_NAME);
	HRESULT hr = xmlDocument->createElement(nodeName.get(), &ordersNode);

	if (hr != S_OK)
	{
		return;
	}

	for (const auto &itemOrder : model->GetItemOrders())
	{
		SaveOrder(xmlDocument, ordersNode.get(), itemOrder);
	}

	XMLSettings::AppendChildToParent(ordersNode.get(), rootNode);
}

}
