// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#pragma once

#include "AcceleratorManager.h"
#include "AppServices.h"
#include "Bookmarks/BookmarkTree.h"
#include "BrowserList.h"
#include "ColorRuleModel.h"
#include "Config.h"
#include "MainRebarStorage.h"
#include "PlatformContextFake.h"
#include "ShellBrowser/NavigationEvents.h"
#include "ShellBrowser/ShellBrowserEvents.h"
#include "TabEvents.h"
#include "TabRestorer.h"
#include "TabStorage.h"
#include "Win32ResourceLoader.h"
#include "WindowStorage.h"
#include "../Helper/CachedIcons.h"
#include "../Helper/Pidl.h"
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

class BrowserWindowFake;
class Tab;

class BrowserTestBase : public testing::Test
{
protected:
	BrowserTestBase();
	~BrowserTestBase();

	BrowserWindowFake *AddBrowser(const WindowStorageData &storageData = {});
	void RemoveBrowser(BrowserWindowFake *browser);

	static void NavigateTab(Tab *tab, const std::wstring &path, PidlAbsolute *outputPidl = nullptr);

	AppServices m_appServices;
	PlatformContextFake m_platformContext;

	Config m_config;
	AcceleratorManager m_acceleratorManager;
	BookmarkTree m_bookmarkTree;
	CachedIcons m_cachedIcons;
	Win32ResourceLoader m_resourceLoader;
	ColorRuleModel m_colorRuleModel;

	TabEvents m_tabEvents;
	ShellBrowserEvents m_shellBrowserEvents;
	NavigationEvents m_navigationEvents;

	BrowserList m_browserList;

	TabRestorer m_tabRestorer;

private:
	void SetUpAppServices();

	std::vector<std::unique_ptr<BrowserWindowFake>> m_browsers;
};
