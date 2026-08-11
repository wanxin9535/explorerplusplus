// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#pragma once

#include "BetterEnumsWrapper.h"

BETTER_ENUM(SortDirection, int,
	Ascending = 0,
	Descending = 1
)

constexpr SortDirection InvertSortDirection(SortDirection direction)
{
	return direction == +SortDirection::Ascending ? SortDirection::Descending
												  : SortDirection::Ascending;
}
