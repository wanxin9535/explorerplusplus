// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#pragma once

#include <tuple>

class ColorRuleModel;
struct Config;
class PlatformContext;
class ResourceLoader;

// Provides access to various application services. A "service" here refers to an object instance
// that will be available for the lifetime of the application.
//
// This is useful in situations where a high-level class might indirectly require access to a number
// of different services. Passing an instance of this class can help stop constructors for
// higher-level classes from becoming bloated, at the cost of having less explicit dependencies (it
// may no longer be as clear exactly which dependencies a high-level component needs).
//
// Using this class, as opposed to App, is also advantageous, as App performs a lot of work that
// should only be done when running the application. So, a class that has a dependency on App can't
// be tested. This class more cleanly separates the code that deals with service retrieval from the
// code that deals with setting up and running the application.
//
// Since this class doesn't own anything, it also helps to facilitate testing, since a client class
// can create a service instance in whatever manner they want, then assign it here.
class AppServices
{
public:
	ColorRuleModel *GetColorRuleModel();
	const ColorRuleModel *GetColorRuleModel() const;
	void SetColorRuleModel(ColorRuleModel *colorRuleModel);

	Config *GetConfig();
	const Config *GetConfig() const;
	void SetConfig(Config *config);

	PlatformContext *GetPlatformContext();
	const PlatformContext *GetPlatformContext() const;
	void SetPlatformContext(PlatformContext *platformContext);

	ResourceLoader *GetResourceLoader();
	const ResourceLoader *GetResourceLoader() const;
	void SetResourceLoader(ResourceLoader *resourceLoader);

	void CheckFullyInitialized() const;

private:
	template <class T>
	T *Get();

	template <class T>
	const T *Get() const;

	template <class T>
	void Set(T *service);

	std::tuple<ColorRuleModel *, Config *, PlatformContext *, ResourceLoader *> m_services;
};
