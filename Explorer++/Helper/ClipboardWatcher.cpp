// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#include "stdafx.h"
#include "ClipboardWatcher.h"

boost::signals2::connection ClipboardWatcher::AddClipboardUpdatedObserver(
	const ClipboardUpdatedSignal::slot_type &observer)
{
	return m_clipboardUpdatedSignal.connect(observer);
}

void ClipboardWatcher::NotifyClipboardUpdated()
{
	m_clipboardUpdatedSignal();
}
