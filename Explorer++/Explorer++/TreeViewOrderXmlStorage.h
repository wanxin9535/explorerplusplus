// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#pragma once

#include <msxml.h>

class TreeViewOrderModel;

namespace TreeViewOrderXmlStorage
{

void Load(IXMLDOMNode *rootNode, TreeViewOrderModel *model);
void Save(IXMLDOMDocument *xmlDocument, IXMLDOMNode *rootNode,
	const TreeViewOrderModel *model);

}
