// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#include "stdafx.h"
#include "BrowserWindowFactoryImpl.h"
#include "Explorer++.h"

BrowserWindowFactoryImpl::BrowserWindowFactoryImpl(App *app, HINSTANCE resourceInstance) :
	m_app(app),
	m_resourceInstance(resourceInstance)
{
}

BrowserWindow *BrowserWindowFactoryImpl::CreateBrowserWindow(const WindowStorageData *storageData)
{
	return Explorerplusplus::Create(m_app, m_resourceInstance, storageData);
}
