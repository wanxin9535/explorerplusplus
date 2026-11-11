// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#pragma once

#include <msxml.h>

class PinnedItemsModel;

namespace PinnedItemsXmlStorage
{

void Load(IXMLDOMNode *rootNode, PinnedItemsModel *model);
void Save(IXMLDOMDocument *xmlDocument, IXMLDOMNode *rootNode, const PinnedItemsModel *model);

}
