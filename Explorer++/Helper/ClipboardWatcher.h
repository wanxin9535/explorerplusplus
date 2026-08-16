// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#pragma once

#include <boost/signals2.hpp>

class ClipboardWatcher
{
public:
	using ClipboardUpdatedSignal = boost::signals2::signal<void()>;

	virtual ~ClipboardWatcher() = default;

	boost::signals2::connection AddClipboardUpdatedObserver(
		const ClipboardUpdatedSignal::slot_type &observer);

protected:
	void NotifyClipboardUpdated();

private:
	ClipboardUpdatedSignal m_clipboardUpdatedSignal;
};
