// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#pragma once

// This allows application-level commands to be executed, without having to take a dependency on
// App.
class AppController
{
public:
	virtual ~AppController() = default;

	virtual void TryExit() = 0;
};
