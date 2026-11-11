// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#include "pch.h"
#include "PinnedItemsModel.h"
#include "PidlTestHelper.h"
#include <gtest/gtest.h>

using namespace testing;

TEST(PinnedItemsModelTest, PinsAreScopedToParentDirectory)
{
	PinnedItemsModel model;
	PidlAbsolute parent1 = CreateSimplePidlForTest(L"C:\\Parent1");
	PidlAbsolute parent2 = CreateSimplePidlForTest(L"C:\\Parent2");
	PidlAbsolute item1 = CreateSimplePidlForTest(L"C:\\Parent1\\OneDrive");
	PidlAbsolute item2 = CreateSimplePidlForTest(L"C:\\Parent2\\OneDrive");

	model.SetItemsPinned(parent1, { item1 }, true);

	EXPECT_TRUE(model.IsPinned(parent1, item1));
	EXPECT_FALSE(model.IsPinned(parent1, item2));
	EXPECT_FALSE(model.IsPinned(parent2, item1));

	model.SetItemsPinned(parent2, { item2 }, true);

	EXPECT_TRUE(model.IsPinned(parent2, item2));
	EXPECT_EQ(model.GetPinnedItems().size(), 2u);
}

TEST(PinnedItemsModelTest, PinAndUnpinAreIdempotent)
{
	PinnedItemsModel model;
	PidlAbsolute parent = CreateSimplePidlForTest(L"C:\\Parent");
	PidlAbsolute item = CreateSimplePidlForTest(L"C:\\Parent\\Item.txt");

	model.SetItemsPinned(parent, { item }, true);
	model.SetItemsPinned(parent, { item }, true);
	EXPECT_EQ(model.GetPinnedItems().size(), 1u);

	model.SetItemsPinned(parent, { item }, false);
	model.SetItemsPinned(parent, { item }, false);
	EXPECT_TRUE(model.GetPinnedItems().empty());
}

TEST(PinnedItemsModelTest, RenameKeepsItemPinned)
{
	PinnedItemsModel model;
	PidlAbsolute parent = CreateSimplePidlForTest(L"C:\\Parent");
	PidlAbsolute oldItem = CreateSimplePidlForTest(L"C:\\Parent\\Old.txt");
	PidlAbsolute newItem = CreateSimplePidlForTest(L"C:\\Parent\\New.txt");
	model.SetItemsPinned(parent, { oldItem }, true);

	model.RenameItem(parent, oldItem, newItem, false);

	EXPECT_FALSE(model.IsPinned(parent, oldItem));
	EXPECT_TRUE(model.IsPinned(parent, newItem));
	EXPECT_EQ(model.GetPinnedItems().size(), 1u);
}

TEST(PinnedItemsModelTest, RenamingDirectoryRebasesChildPins)
{
	PinnedItemsModel model;
	PidlAbsolute parent = CreateSimplePidlForTest(L"C:\\Parent");
	PidlAbsolute oldDirectory = CreateSimplePidlForTest(L"C:\\Parent\\Old");
	PidlAbsolute newDirectory = CreateSimplePidlForTest(L"C:\\Parent\\New");
	PidlAbsolute oldChildDirectory = CreateSimplePidlForTest(L"C:\\Parent\\Old\\Child");
	PidlAbsolute oldItem = CreateSimplePidlForTest(L"C:\\Parent\\Old\\Child\\Item.txt");
	PidlAbsolute newChildDirectory = CreateSimplePidlForTest(L"C:\\Parent\\New\\Child");
	PidlAbsolute newItem = CreateSimplePidlForTest(L"C:\\Parent\\New\\Child\\Item.txt");
	model.SetPinnedItems({ { parent, oldDirectory }, { oldDirectory, oldChildDirectory },
		{ oldChildDirectory, oldItem } });

	model.RenameItem(parent, oldDirectory, newDirectory, true);

	EXPECT_FALSE(model.IsPinned(parent, oldDirectory));
	EXPECT_TRUE(model.IsPinned(parent, newDirectory));
	EXPECT_FALSE(model.IsPinned(oldDirectory, oldChildDirectory));
	EXPECT_TRUE(model.IsPinned(newDirectory, newChildDirectory));
	EXPECT_FALSE(model.IsPinned(oldChildDirectory, oldItem));
	EXPECT_TRUE(model.IsPinned(newChildDirectory, newItem));
}

TEST(PinnedItemsModelTest, RemovingDirectoryRemovesItsPins)
{
	PinnedItemsModel model;
	PidlAbsolute parent = CreateSimplePidlForTest(L"C:\\Parent");
	PidlAbsolute directory = CreateSimplePidlForTest(L"C:\\Parent\\Directory");
	PidlAbsolute childDirectory = CreateSimplePidlForTest(L"C:\\Parent\\Directory\\Child");
	PidlAbsolute childItem = CreateSimplePidlForTest(L"C:\\Parent\\Directory\\Child\\Item.txt");
	model.SetPinnedItems({ { parent, directory }, { directory, childDirectory },
		{ childDirectory, childItem } });

	model.RemoveItem(parent, directory, true);

	EXPECT_TRUE(model.GetPinnedItems().empty());
}

TEST(PinnedItemsModelTest, ChangedSignalOnlyFiresWhenStateChanges)
{
	PinnedItemsModel model;
	PidlAbsolute parent = CreateSimplePidlForTest(L"C:\\Parent");
	PidlAbsolute item = CreateSimplePidlForTest(L"C:\\Parent\\Item.txt");
	MockFunction<void(const PidlAbsolute &)> callback;
	model.AddPinnedItemsChangedObserver(callback.AsStdFunction());
	EXPECT_CALL(callback, Call(parent)).Times(2);

	model.SetItemsPinned(parent, { item }, true);
	model.SetItemsPinned(parent, { item }, true);
	model.SetItemsPinned(parent, { item }, false);
	model.SetItemsPinned(parent, { item }, false);
}
