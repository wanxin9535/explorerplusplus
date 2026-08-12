// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#include "stdafx.h"
#include "AppServices.h"

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
