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

	SaveLocation GetSaveLocation() const override;
	void SetSaveLocation(SaveLocation saveLocation) override;
	bool WasExitRequested() const;

private:
	SaveLocation m_saveLocation = SaveLocation::Registry;
	bool m_exitRequested = false;
};
