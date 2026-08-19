// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#pragma once

class AcceleratorManager;
class BrowserList;
class TabEvents;
class TabList;

namespace Plugins
{

class PluginCommandManager;
class PluginMenuManager;

}

class PluginInterface
{
public:
	virtual ~PluginInterface() = default;

	virtual TabEvents *GetTabEvents() = 0;
	virtual BrowserList *GetBrowserList() = 0;
	virtual TabList *GetTabList() = 0;
	virtual Plugins::PluginMenuManager *GetPluginMenuManager() = 0;
	virtual AcceleratorManager *GetAcceleratorManager() = 0;
	virtual Plugins::PluginCommandManager *GetPluginCommandManager() = 0;
};
