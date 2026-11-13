// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#include "pch.h"
#include "TreeViewOrderModel.h"
#include "TreeViewOrderRegistryStorage.h"
#include "TreeViewOrderXmlStorage.h"
#include "RegistryStorageTestHelper.h"
#include "XmlStorageTestHelper.h"
#include "PidlTestHelper.h"
#include <gtest/gtest.h>

namespace
{

std::vector<TreeViewItemOrder> BuildItemOrders()
{
	PidlAbsolute desktop = CreateSimplePidlForTest(L"C:\\ShellDesktop");
	PidlAbsolute home = CreateSimplePidlForTest(L"C:\\ShellDesktop\\Home");
	PidlAbsolute first = CreateSimplePidlForTest(L"C:\\ShellDesktop\\Home\\Parent\\First");
	PidlAbsolute second = CreateSimplePidlForTest(L"C:\\ShellDesktop\\Home\\Parent\\Second");
	PidlAbsolute repeatedItem = CreateSimplePidlForTest(L"C:\\ShellDesktop\\Home\\Desktop");
	TreeViewPath quickAccessPath = { home };
	TreeViewPath shellNamespacePath = { desktop, home };

	return { { { desktop, home }, { second, first } }, { { desktop }, { home } },
		{ {}, { CreateSimplePidlForTest(L"C:\\RootItem2"),
			CreateSimplePidlForTest(L"C:\\RootItem1") } },
		{ quickAccessPath, { repeatedItem } }, { shellNamespacePath, { repeatedItem } } };
}

void ExpectSameOrders(const TreeViewOrderModel &expected, const TreeViewOrderModel &actual)
{
	auto expectedOrders = expected.GetItemOrders();
	auto actualOrders = actual.GetItemOrders();
	ASSERT_EQ(actualOrders.size(), expectedOrders.size());

	for (const auto &expectedOrder : expectedOrders)
	{
		auto actualOrder = std::find_if(actualOrders.begin(), actualOrders.end(),
			[&expectedOrder](const auto &currentOrder)
			{ return currentOrder.parentPath == expectedOrder.parentPath; });

		ASSERT_NE(actualOrder, actualOrders.end());
		ASSERT_EQ(actualOrder->items.size(), expectedOrder.items.size());

		for (size_t index = 0; index < expectedOrder.items.size(); index++)
		{
			EXPECT_EQ(actualOrder->items[index], expectedOrder.items[index]);
		}
	}
}

}

class TreeViewOrderRegistryStorageTest : public RegistryStorageTest
{
};

TEST_F(TreeViewOrderRegistryStorageTest, SaveAndLoad)
{
	TreeViewOrderModel referenceModel;
	referenceModel.SetItemOrders(BuildItemOrders());

	TreeViewOrderRegistryStorage::Save(m_applicationTestKey.get(), &referenceModel);

	TreeViewOrderModel loadedModel;
	TreeViewOrderRegistryStorage::Load(m_applicationTestKey.get(), &loadedModel);
	ExpectSameOrders(referenceModel, loadedModel);
}

TEST_F(TreeViewOrderRegistryStorageTest, SavingFewerOrdersDoesNotRestoreOldOrders)
{
	TreeViewOrderModel model;
	model.SetItemOrders(BuildItemOrders());
	TreeViewOrderRegistryStorage::Save(m_applicationTestKey.get(), &model);

	auto itemOrders = BuildItemOrders();
	model.SetItemOrders({ itemOrders[1] });
	TreeViewOrderRegistryStorage::Save(m_applicationTestKey.get(), &model);

	TreeViewOrderModel loadedModel;
	TreeViewOrderRegistryStorage::Load(m_applicationTestKey.get(), &loadedModel);
	ExpectSameOrders(model, loadedModel);
}

class TreeViewOrderXmlStorageTest : public XmlStorageTest
{
};

TEST_F(TreeViewOrderXmlStorageTest, SaveAndLoad)
{
	TreeViewOrderModel referenceModel;
	referenceModel.SetItemOrders(BuildItemOrders());
	auto xmlDocumentData = CreateXmlDocument();

	TreeViewOrderXmlStorage::Save(xmlDocumentData.xmlDocument.get(), xmlDocumentData.rootNode.get(),
		&referenceModel);

	TreeViewOrderModel loadedModel;
	TreeViewOrderXmlStorage::Load(xmlDocumentData.rootNode.get(), &loadedModel);
	ExpectSameOrders(referenceModel, loadedModel);
}
