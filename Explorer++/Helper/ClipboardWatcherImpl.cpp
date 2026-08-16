// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#include "stdafx.h"
#include "ClipboardWatcherImpl.h"
#include "MessageWindowHelper.h"
#include "WindowSubclass.h"

ClipboardWatcherImpl::ClipboardWatcherImpl() :
	m_hwnd(MessageWindowHelper::CreateMessageOnlyWindow()),
	m_windowSubclass(std::make_unique<WindowSubclass>(m_hwnd.get(),
		std::bind_front(&ClipboardWatcherImpl::Subclass, this)))
{
	auto res = AddClipboardFormatListener(m_hwnd.get());
	DCHECK(res);
}

ClipboardWatcherImpl::~ClipboardWatcherImpl()
{
	auto res = RemoveClipboardFormatListener(m_hwnd.get());
	DCHECK(res);
}

LRESULT ClipboardWatcherImpl::Subclass(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CLIPBOARDUPDATE:
		NotifyClipboardUpdated();
		return 0;
	}

	return DefSubclassProc(hwnd, msg, wParam, lParam);
}
