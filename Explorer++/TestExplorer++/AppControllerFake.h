// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#pragma once

#include "AppController.h"

class AppControllerFake : public AppController
{
public:
	// AppController
	void TryExit() override;

	bool WasExitRequested() const;

private:
	bool m_exitRequested = false;
};
