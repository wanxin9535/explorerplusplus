// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#pragma once

#include "BrowserWindowFactory.h"

class App;

class BrowserWindowFactoryImpl : public BrowserWindowFactory
{
public:
	BrowserWindowFactoryImpl(App *app, HINSTANCE resourceInstance);

	BrowserWindow *CreateBrowserWindow(const WindowStorageData *storageData) override;

private:
	App *const m_app;
	const HINSTANCE m_resourceInstance;
};
