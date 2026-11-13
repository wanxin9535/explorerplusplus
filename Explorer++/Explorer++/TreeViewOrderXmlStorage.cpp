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

constexpr wchar_t TREE_VIEW_ORDERS_NODE_NAME[] = L"TreeViewOrdersV2";
constexpr wchar_t TREE_VIEW_ORDER_NODE_NAME[] = L"TreeViewOrder";
constexpr wchar_t PARENT_NODE_NAME[] = L"Parent";
constexpr wchar_t ITEM_NODE_NAME[] = L"Item";
constexpr wchar_t PIDL_ATTRIBUTE[] = L"Pidl";

std::optional<PidlAbsolute> LoadPidl(IXMLDOMNode *node, const wchar_t *attributeName)
{
	wil::com_ptr_nothrow<IXMLDOMNamedNodeMap> attributeMap;
	HRESULT hr = node->get_attributes(&attributeMap);

	if (hr != S_OK)
	{
		return std::nullopt;
	}

	std::wstring encodedPidl;
	hr = XMLSettings::GetStringFromMap(attributeMap.get(), attributeName, encodedPidl);

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

std::vector<PidlAbsolute> LoadPidls(IXMLDOMNode *node, const wchar_t *childNodeName)
{
	wil::com_ptr_nothrow<IXMLDOMNodeList> childNodes;
	auto queryString = wil::make_bstr_nothrow(childNodeName);
	HRESULT hr = node->selectNodes(queryString.get(), &childNodes);

	if (hr != S_OK)
	{
		return {};
	}

	std::vector<PidlAbsolute> pidls;
	wil::com_ptr_nothrow<IXMLDOMNode> childNode;

	while (childNodes->nextNode(&childNode) == S_OK)
	{
		auto pidl = LoadPidl(childNode.get(), PIDL_ATTRIBUTE);

		if (pidl)
		{
			pidls.push_back(std::move(*pidl));
		}
	}

	return pidls;
}

std::optional<TreeViewItemOrder> LoadOrder(IXMLDOMNode *orderNode)
{
	TreeViewItemOrder itemOrder;
	itemOrder.parentPath = LoadPidls(orderNode, PARENT_NODE_NAME);
	itemOrder.items = LoadPidls(orderNode, ITEM_NODE_NAME);

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
	const PidlAbsolute &pidl)
{
	auto encodedPidl = EncodePidlToBase64(pidl);
	auto encodedPidlWide = StrToWstr(encodedPidl);

	if (!encodedPidlWide)
	{
		return false;
	}

	XMLSettings::AddAttributeToNode(xmlDocument, node, PIDL_ATTRIBUTE, *encodedPidlWide);
	return true;
}

bool AppendPidlNode(IXMLDOMDocument *xmlDocument, IXMLDOMElement *parentNode,
	const wchar_t *nodeName, const PidlAbsolute &pidl)
{
	wil::com_ptr_nothrow<IXMLDOMElement> node;
	auto name = wil::make_bstr_nothrow(nodeName);
	HRESULT hr = xmlDocument->createElement(name.get(), &node);

	if (hr != S_OK || !AddPidlAttribute(xmlDocument, node.get(), pidl))
	{
		return false;
	}

	XMLSettings::AppendChildToParent(node.get(), parentNode);
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

	for (const auto &parent : itemOrder.parentPath)
	{
		if (!AppendPidlNode(xmlDocument, orderNode.get(), PARENT_NODE_NAME, parent))
		{
			return;
		}
	}

	for (const auto &item : itemOrder.items)
	{
		if (!AppendPidlNode(xmlDocument, orderNode.get(), ITEM_NODE_NAME, item))
		{
			return;
		}
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
		// Version 1 stored only a parent PIDL and cannot distinguish repeated namespace entries.
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
