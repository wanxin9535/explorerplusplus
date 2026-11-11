// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#pragma once

#include <Windows.h>

class TreeViewOrderModel;

namespace TreeViewOrderRegistryStorage
{

void Load(HKEY applicationKey, TreeViewOrderModel *model);
void Save(HKEY applicationKey, const TreeViewOrderModel *model);

}
