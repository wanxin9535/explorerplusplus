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
	TreeViewPath parentPath = { CreateSimplePidlForTest(L"C:\\Tree") };
	PidlAbsolute first = CreateSimplePidlForTest(L"C:\\Tree\\First");
	PidlAbsolute second = CreateSimplePidlForTest(L"C:\\Tree\\Second");
	PidlAbsolute third = CreateSimplePidlForTest(L"C:\\Tree\\Third");
	std::vector<PidlAbsolute> currentOrder = { first, second, third };

	EXPECT_FALSE(model.MoveItem(parentPath, 0, TreeViewOrderModel::Direction::Up, currentOrder));
	EXPECT_TRUE(model.MoveItem(parentPath, 2, TreeViewOrderModel::Direction::Up, currentOrder));
	EXPECT_EQ(model.GetItemIndex(parentPath, first), 0u);
	EXPECT_EQ(model.GetItemIndex(parentPath, third), 1u);
	EXPECT_EQ(model.GetItemIndex(parentPath, second), 2u);

	currentOrder = { first, third, second };
	EXPECT_TRUE(model.MoveItem(parentPath, 0, TreeViewOrderModel::Direction::Down, currentOrder));
	EXPECT_EQ(model.GetItemIndex(parentPath, first), 1u);
	EXPECT_EQ(model.GetItemIndex(parentPath, third), 0u);
	EXPECT_EQ(model.GetItemIndex(parentPath, second), 2u);
	EXPECT_FALSE(model.MoveItem(parentPath, 2, TreeViewOrderModel::Direction::Down, currentOrder));
}

TEST(TreeViewOrderModelTest, OrdersAreScopedToTheirFullParentPath)
{
	TreeViewOrderModel model;
	PidlAbsolute desktopRoot = CreateSimplePidlForTest(L"C:\\ShellDesktop");
	PidlAbsolute home = CreateSimplePidlForTest(L"C:\\ShellDesktop\\Home");
	TreeViewPath firstBranch = { home };
	TreeViewPath secondBranch = { desktopRoot, home };
	PidlAbsolute firstItem = CreateSimplePidlForTest(L"C:\\ShellDesktop\\Home\\Desktop");
	PidlAbsolute secondItem = CreateSimplePidlForTest(L"C:\\ShellDesktop\\Home\\Documents");

	model.SetItemOrders({ { firstBranch, { firstItem, secondItem } } });

	EXPECT_TRUE(model.HasOrder(firstBranch));
	EXPECT_FALSE(model.HasOrder(secondBranch));
	EXPECT_EQ(model.GetItemIndex(firstBranch, firstItem), 0u);
	EXPECT_FALSE(model.GetItemIndex(secondBranch, firstItem));
	EXPECT_FALSE(model.HasOrder({ CreateSimplePidlForTest(L"C:\\DifferentParent") }));
}

TEST(TreeViewOrderModelTest, MovesOnlyTheSelectedOccurrenceWhenPidlsAreRepeated)
{
	TreeViewOrderModel model;
	PidlAbsolute desktopRoot = CreateSimplePidlForTest(L"C:\\ShellDesktop");
	PidlAbsolute home = CreateSimplePidlForTest(L"C:\\ShellDesktop\\Home");
	PidlAbsolute desktop = CreateSimplePidlForTest(L"C:\\ShellDesktop\\Home\\Desktop");
	PidlAbsolute documents = CreateSimplePidlForTest(L"C:\\ShellDesktop\\Home\\Documents");
	TreeViewPath quickAccessBranch = { home };
	TreeViewPath shellNamespaceBranch = { desktopRoot, home };
	std::vector<PidlAbsolute> items = { desktop, documents };
	model.SetItemOrders({ { quickAccessBranch, items }, { shellNamespaceBranch, items } });

	EXPECT_TRUE(model.MoveItem(quickAccessBranch, 1, TreeViewOrderModel::Direction::Up, items));

	EXPECT_EQ(model.GetItemIndex(quickAccessBranch, documents), 0u);
	EXPECT_EQ(model.GetItemIndex(quickAccessBranch, desktop), 1u);
	EXPECT_EQ(model.GetItemIndex(shellNamespaceBranch, desktop), 0u);
	EXPECT_EQ(model.GetItemIndex(shellNamespaceBranch, documents), 1u);
}

TEST(TreeViewOrderModelTest, StoresOrderForTheTreeRoot)
{
	TreeViewOrderModel model;
	PidlAbsolute first = CreateSimplePidlForTest(L"C:\\RootItem1");
	PidlAbsolute second = CreateSimplePidlForTest(L"C:\\RootItem2");

	model.SetItemOrders({ { {}, { second, first } } });

	EXPECT_EQ(model.GetItemIndex({}, second), 0u);
	EXPECT_EQ(model.GetItemIndex({}, first), 1u);
	EXPECT_FALSE(model.GetItemIndex({ first }, second));
}

TEST(TreeViewOrderModelTest, DirectoryRenameRebasesDescendantPaths)
{
	TreeViewOrderModel model;
	PidlAbsolute root = CreateSimplePidlForTest(L"C:\\Tree");
	PidlAbsolute oldDirectory = CreateSimplePidlForTest(L"C:\\Tree\\Old");
	PidlAbsolute newDirectory = CreateSimplePidlForTest(L"C:\\Tree\\New");
	PidlAbsolute oldChild = CreateSimplePidlForTest(L"C:\\Tree\\Old\\Child");
	PidlAbsolute newChild = CreateSimplePidlForTest(L"C:\\Tree\\New\\Child");
	PidlAbsolute oldItem = CreateSimplePidlForTest(L"C:\\Tree\\Old\\Child\\Item");
	PidlAbsolute newItem = CreateSimplePidlForTest(L"C:\\Tree\\New\\Child\\Item");
	TreeViewPath rootPath = { root };
	TreeViewPath oldDirectoryPath = { root, oldDirectory };
	TreeViewPath newDirectoryPath = { root, newDirectory };
	TreeViewPath oldChildPath = { root, oldDirectory, oldChild };
	TreeViewPath newChildPath = { root, newDirectory, newChild };

	model.SetItemOrders({ { rootPath, { oldDirectory } }, { oldDirectoryPath, { oldChild } },
		{ oldChildPath, { oldItem } } });
	model.RenameItem(oldDirectory, newDirectory, true);

	EXPECT_FALSE(model.GetItemIndex(rootPath, oldDirectory));
	EXPECT_EQ(model.GetItemIndex(rootPath, newDirectory), 0u);
	EXPECT_EQ(model.GetItemIndex(newDirectoryPath, newChild), 0u);
	EXPECT_EQ(model.GetItemIndex(newChildPath, newItem), 0u);
}

TEST(TreeViewOrderModelTest, RemovingDirectoryRemovesItsDescendantOrders)
{
	TreeViewOrderModel model;
	PidlAbsolute root = CreateSimplePidlForTest(L"C:\\Tree");
	PidlAbsolute directory = CreateSimplePidlForTest(L"C:\\Tree\\Directory");
	PidlAbsolute child = CreateSimplePidlForTest(L"C:\\Tree\\Directory\\Child");
	PidlAbsolute item = CreateSimplePidlForTest(L"C:\\Tree\\Directory\\Child\\Item");
	TreeViewPath rootPath = { root };
	TreeViewPath directoryPath = { root, directory };
	TreeViewPath childPath = { root, directory, child };

	model.SetItemOrders({ { rootPath, { directory } }, { directoryPath, { child } },
		{ childPath, { item } } });
	model.RemoveItem(rootPath, directory, true);

	EXPECT_FALSE(model.GetItemIndex(rootPath, directory));
	EXPECT_FALSE(model.GetItemIndex(directoryPath, child));
	EXPECT_FALSE(model.GetItemIndex(childPath, item));
}
