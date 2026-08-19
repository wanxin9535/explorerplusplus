// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#include "stdafx.h"
#include "Explorer++.h"
#include "AppServices.h"

BrowserList *Explorerplusplus::GetBrowserList()
{
	return m_browserList;
}

TabList *Explorerplusplus::GetTabList()
{
	return m_appServices->GetTabList();
}

Plugins::PluginMenuManager *Explorerplusplus::GetPluginMenuManager()
{
	return &m_pluginMenuManager;
}

AcceleratorManager *Explorerplusplus::GetAcceleratorManager()
{
	return m_acceleratorManager;
}

Plugins::PluginCommandManager *Explorerplusplus::GetPluginCommandManager()
{
	return &m_pluginCommandManager;
}
