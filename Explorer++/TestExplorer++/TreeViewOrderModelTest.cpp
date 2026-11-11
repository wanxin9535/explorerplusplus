// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#include "pch.h"
#include "TreeViewOrderModel.h"
#include "PidlTestHelper.h"
#include <gtest/gtest.h>

using namespace testing;

TEST(TreeViewOrderModelTest, MovesOneSiblingAtATime)
{
	TreeViewOrderModel model;
	PidlAbsolute parent = CreateSimplePidlForTest(L"C:\\Tree");
	PidlAbsolute first = CreateSimplePidlForTest(L"C:\\Tree\\First");
	PidlAbsolute second = CreateSimplePidlForTest(L"C:\\Tree\\Second");
	PidlAbsolute third = CreateSimplePidlForTest(L"C:\\Tree\\Third");
	std::vector<PidlAbsolute> currentOrder = { first, second, third };

	EXPECT_FALSE(model.MoveItem(parent, first, TreeViewOrderModel::Direction::Up, currentOrder));
	EXPECT_TRUE(model.MoveItem(parent, third, TreeViewOrderModel::Direction::Up, currentOrder));
	EXPECT_EQ(model.GetItemIndex(parent, first), 0u);
	EXPECT_EQ(model.GetItemIndex(parent, third), 1u);
	EXPECT_EQ(model.GetItemIndex(parent, second), 2u);

	currentOrder = { first, third, second };
	EXPECT_TRUE(model.MoveItem(parent, first, TreeViewOrderModel::Direction::Down, currentOrder));
	EXPECT_EQ(model.GetItemIndex(parent, first), 1u);
	EXPECT_EQ(model.GetItemIndex(parent, third), 0u);
	EXPECT_EQ(model.GetItemIndex(parent, second), 2u);
	EXPECT_FALSE(model.MoveItem(parent, second, TreeViewOrderModel::Direction::Down, currentOrder));
}

TEST(TreeViewOrderModelTest, OrdersAreScopedToTheirParent)
{
	TreeViewOrderModel model;
	PidlAbsolute parent1 = CreateSimplePidlForTest(L"C:\\Tree\\Parent1");
	PidlAbsolute parent2 = CreateSimplePidlForTest(L"C:\\Tree\\Parent2");
	PidlAbsolute item1 = CreateSimplePidlForTest(L"C:\\Tree\\Parent1\\Item");
	PidlAbsolute item2 = CreateSimplePidlForTest(L"C:\\Tree\\Parent2\\Item");

	model.SetItemOrders({ { parent1, { item1 } } });

	EXPECT_EQ(model.GetItemIndex(parent1, item1), 0u);
	EXPECT_FALSE(model.GetItemIndex(parent2, item1));
	EXPECT_FALSE(model.GetItemIndex(parent1, item2));
}

TEST(TreeViewOrderModelTest, StoresOrderForTheTreeRoot)
{
	TreeViewOrderModel model;
	PidlAbsolute first = CreateSimplePidlForTest(L"C:\\RootItem1");
	PidlAbsolute second = CreateSimplePidlForTest(L"C:\\RootItem2");

	model.SetItemOrders({ { PidlAbsolute(), { second, first } } });

	EXPECT_EQ(model.GetItemIndex(PidlAbsolute(), second), 0u);
	EXPECT_EQ(model.GetItemIndex(PidlAbsolute(), first), 1u);
	EXPECT_FALSE(model.GetItemIndex(first, second));
}

TEST(TreeViewOrderModelTest, DirectoryRenameRebasesChildOrders)
{
	TreeViewOrderModel model;
	PidlAbsolute parent = CreateSimplePidlForTest(L"C:\\Tree");
	PidlAbsolute oldDirectory = CreateSimplePidlForTest(L"C:\\Tree\\Old");
	PidlAbsolute newDirectory = CreateSimplePidlForTest(L"C:\\Tree\\New");
	PidlAbsolute oldChild = CreateSimplePidlForTest(L"C:\\Tree\\Old\\Child");
	PidlAbsolute newChild = CreateSimplePidlForTest(L"C:\\Tree\\New\\Child");
	PidlAbsolute oldItem = CreateSimplePidlForTest(L"C:\\Tree\\Old\\Child\\Item");
	PidlAbsolute newItem = CreateSimplePidlForTest(L"C:\\Tree\\New\\Child\\Item");

	model.SetItemOrders({ { parent, { oldDirectory } }, { oldDirectory, { oldChild } },
		{ oldChild, { oldItem } } });
	model.RenameItem(parent, oldDirectory, newDirectory, true);

	EXPECT_FALSE(model.GetItemIndex(parent, oldDirectory));
	EXPECT_EQ(model.GetItemIndex(parent, newDirectory), 0u);
	EXPECT_EQ(model.GetItemIndex(newDirectory, newChild), 0u);
	EXPECT_EQ(model.GetItemIndex(newChild, newItem), 0u);
}

TEST(TreeViewOrderModelTest, RemovingDirectoryRemovesItsOrder)
{
	TreeViewOrderModel model;
	PidlAbsolute parent = CreateSimplePidlForTest(L"C:\\Tree");
	PidlAbsolute directory = CreateSimplePidlForTest(L"C:\\Tree\\Directory");
	PidlAbsolute child = CreateSimplePidlForTest(L"C:\\Tree\\Directory\\Child");
	PidlAbsolute item = CreateSimplePidlForTest(L"C:\\Tree\\Directory\\Child\\Item");

	model.SetItemOrders({ { parent, { directory } }, { directory, { child } }, { child, { item } } });
	model.RemoveItem(parent, directory, true);

	EXPECT_FALSE(model.GetItemIndex(parent, directory));
	EXPECT_FALSE(model.GetItemIndex(directory, child));
	EXPECT_FALSE(model.GetItemIndex(child, item));
}
