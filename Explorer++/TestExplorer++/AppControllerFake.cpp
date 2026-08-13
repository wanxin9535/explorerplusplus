// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#include "pch.h"
#include "AppControllerFake.h"

void AppControllerFake::TryExit()
{
	m_exitRequested = true;
}

bool AppControllerFake::WasExitRequested() const
{
	return m_exitRequested;
}
