// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#pragma once

class PinnedItemsModel;

namespace PinnedItemsRegistryStorage
{

void Load(HKEY applicationKey, PinnedItemsModel *model);
void Save(HKEY applicationKey, const PinnedItemsModel *model);

}
