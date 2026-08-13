// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#include "stdafx.h"
#include "AppServices.h"

ColorRuleModel *AppServices::GetColorRuleModel()
{
	return Get<ColorRuleModel>();
}

const ColorRuleModel *AppServices::GetColorRuleModel() const
{
	return Get<ColorRuleModel>();
}

void AppServices::SetColorRuleModel(ColorRuleModel *colorRuleModel)
{
	Set(colorRuleModel);
}

Config *AppServices::GetConfig()
{
	return Get<Config>();
}

const Config *AppServices::GetConfig() const
{
	return Get<Config>();
}

void AppServices::SetConfig(Config *config)
{
	Set(config);
}

ModelessDialogList *AppServices::GetModelessDialogList()
{
	return Get<ModelessDialogList>();
}

const ModelessDialogList *AppServices::GetModelessDialogList() const
{
	return Get<ModelessDialogList>();
}

void AppServices::SetModelessDialogList(ModelessDialogList *modelessDialogList)
{
	Set(modelessDialogList);
}

NavigationEvents *AppServices::GetNavigationEvents()
{
	return Get<NavigationEvents>();
}

const NavigationEvents *AppServices::GetNavigationEvents() const
{
	return Get<NavigationEvents>();
}

void AppServices::SetNavigationEvents(NavigationEvents *navigationEvents)
{
	Set(navigationEvents);
}

PlatformContext *AppServices::GetPlatformContext()
{
	return Get<PlatformContext>();
}

const PlatformContext *AppServices::GetPlatformContext() const
{
	return Get<PlatformContext>();
}

void AppServices::SetPlatformContext(PlatformContext *platformContext)
{
	Set(platformContext);
}

ResourceLoader *AppServices::GetResourceLoader()
{
	return Get<ResourceLoader>();
}

const ResourceLoader *AppServices::GetResourceLoader() const
{
	return Get<ResourceLoader>();
}

void AppServices::SetResourceLoader(ResourceLoader *resourceLoader)
{
	Set(resourceLoader);
}

ShellBrowserEvents *AppServices::GetShellBrowserEvents()
{
	return Get<ShellBrowserEvents>();
}

const ShellBrowserEvents *AppServices::GetShellBrowserEvents() const
{
	return Get<ShellBrowserEvents>();
}

void AppServices::SetShellBrowserEvents(ShellBrowserEvents *shellBrowserEvents)
{
	Set(shellBrowserEvents);
}

TabEvents *AppServices::GetTabEvents()
{
	return Get<TabEvents>();
}

const TabEvents *AppServices::GetTabEvents() const
{
	return Get<TabEvents>();
}

void AppServices::SetTabEvents(TabEvents *tabEvents)
{
	Set(tabEvents);
}

TabList *AppServices::GetTabList()
{
	return Get<TabList>();
}

const TabList *AppServices::GetTabList() const
{
	return Get<TabList>();
}

void AppServices::SetTabList(TabList *tabList)
{
	Set(tabList);
}

TabRestorer *AppServices::GetTabRestorer()
{
	return Get<TabRestorer>();
}

const TabRestorer *AppServices::GetTabRestorer() const
{
	return Get<TabRestorer>();
}

void AppServices::SetTabRestorer(TabRestorer *tabRestorer)
{
	Set(tabRestorer);
}

void AppServices::CheckFullyInitialized() const
{
	std::apply([](const auto *...services) { CHECK((services && ...)); }, m_services);
}

template <class T>
T *AppServices::Get()
{
	auto *service = std::get<T *>(m_services);
	CHECK(service);
	return service;
}

template <class T>
const T *AppServices::Get() const
{
	const auto *service = std::get<T *>(m_services);
	CHECK(service);
	return service;
}

template <class T>
void AppServices::Set(T *service)
{
	CHECK(service);

	auto &currentService = std::get<T *>(m_services);
	CHECK(!currentService);

	currentService = service;
}
