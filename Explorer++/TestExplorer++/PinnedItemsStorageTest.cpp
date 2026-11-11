// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#include "pch.h"
#include "PinnedItemsModel.h"
#include "PinnedItemsRegistryStorage.h"
#include "PinnedItemsXmlStorage.h"
#include "RegistryStorageTestHelper.h"
#include "XmlStorageTestHelper.h"
#include "PidlTestHelper.h"
#include <gtest/gtest.h>

namespace
{

std::vector<PinnedItem> BuildPinnedItems()
{
	PidlAbsolute parent1 = CreateSimplePidlForTest(L"C:\\Pins\\One");
	PidlAbsolute parent2 = CreateSimplePidlForTest(L"C:\\Pins\\Two");

	return { { parent1, CreateSimplePidlForTest(L"C:\\Pins\\One\\First.txt") },
		{ parent1, CreateSimplePidlForTest(L"C:\\Pins\\One\\Second.txt") },
		{ parent2, CreateSimplePidlForTest(L"C:\\Pins\\Two\\Third.txt") } };
}

void ExpectSamePins(const PinnedItemsModel &expected, const PinnedItemsModel &actual)
{
	auto expectedItems = expected.GetPinnedItems();
	auto actualItems = actual.GetPinnedItems();
	ASSERT_EQ(actualItems.size(), expectedItems.size());

	for (const auto &item : expectedItems)
	{
		EXPECT_TRUE(actual.IsPinned(item.parentDirectory, item.item));
	}
}

}

class PinnedItemsRegistryStorageTest : public RegistryStorageTest
{
};

TEST_F(PinnedItemsRegistryStorageTest, SaveAndLoad)
{
	PinnedItemsModel referenceModel;
	referenceModel.SetPinnedItems(BuildPinnedItems());

	PinnedItemsRegistryStorage::Save(m_applicationTestKey.get(), &referenceModel);

	PinnedItemsModel loadedModel;
	PinnedItemsRegistryStorage::Load(m_applicationTestKey.get(), &loadedModel);
	ExpectSamePins(referenceModel, loadedModel);
}

TEST_F(PinnedItemsRegistryStorageTest, SavingFewerItemsDoesNotRestoreOldPins)
{
	PinnedItemsModel model;
	auto pinnedItems = BuildPinnedItems();
	model.SetPinnedItems(pinnedItems);
	PinnedItemsRegistryStorage::Save(m_applicationTestKey.get(), &model);

	model.SetPinnedItems({ pinnedItems[0] });
	PinnedItemsRegistryStorage::Save(m_applicationTestKey.get(), &model);

	PinnedItemsModel loadedModel;
	PinnedItemsRegistryStorage::Load(m_applicationTestKey.get(), &loadedModel);
	ExpectSamePins(model, loadedModel);
}

class PinnedItemsXmlStorageTest : public XmlStorageTest
{
};

TEST_F(PinnedItemsXmlStorageTest, SaveAndLoad)
{
	PinnedItemsModel referenceModel;
	referenceModel.SetPinnedItems(BuildPinnedItems());
	auto xmlDocumentData = CreateXmlDocument();

	PinnedItemsXmlStorage::Save(xmlDocumentData.xmlDocument.get(), xmlDocumentData.rootNode.get(),
		&referenceModel);

	PinnedItemsModel loadedModel;
	PinnedItemsXmlStorage::Load(xmlDocumentData.rootNode.get(), &loadedModel);
	ExpectSamePins(referenceModel, loadedModel);
}
