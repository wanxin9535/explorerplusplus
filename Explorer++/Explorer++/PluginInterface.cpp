// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#include "stdafx.h"
#include "Explorer++.h"

TabContainer *Explorerplusplus::GetTabContainer()
{
	return GetActivePane()->GetTabContainer();
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
