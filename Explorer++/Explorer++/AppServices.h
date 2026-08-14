// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#pragma once

#include <tuple>

class AcceleratorManager;
class AppController;
class BrowserList;
class ColorRuleModel;
struct Config;
class DarkModeColorProvider;
class DarkModeManager;
class DriveModel;
class FrequentLocationsModel;
class HistoryModel;
class ModelessDialogList;
class NavigationEvents;
class PlatformContext;
class ResourceLoader;
class ShellBrowserEvents;
class TabEvents;
class TabList;
class TabRestorer;

namespace Applications
{

class ApplicationModel;

}

namespace CommandLine
{

struct Settings;

}

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
	AcceleratorManager *GetAcceleratorManager();
	const AcceleratorManager *GetAcceleratorManager() const;
	void SetAcceleratorManager(AcceleratorManager *acceleratorManager);

	AppController *GetAppController();
	const AppController *GetAppController() const;
	void SetAppController(AppController *appController);

	Applications::ApplicationModel *GetApplicationModel();
	const Applications::ApplicationModel *GetApplicationModel() const;
	void SetApplicationModel(Applications::ApplicationModel *applicationModel);

	BrowserList *GetBrowserList();
	const BrowserList *GetBrowserList() const;
	void SetBrowserList(BrowserList *browserList);

	ColorRuleModel *GetColorRuleModel();
	const ColorRuleModel *GetColorRuleModel() const;
	void SetColorRuleModel(ColorRuleModel *colorRuleModel);

	const CommandLine::Settings *GetCommandLineSettings() const;
	void SetCommandLineSettings(const CommandLine::Settings *commandLineSettings);

	Config *GetConfig();
	const Config *GetConfig() const;
	void SetConfig(Config *config);

	DarkModeColorProvider *GetDarkModeColorProvider();
	const DarkModeColorProvider *GetDarkModeColorProvider() const;
	void SetDarkModeColorProvider(DarkModeColorProvider *darkModeColorProvider);

	DarkModeManager *GetDarkModeManager();
	const DarkModeManager *GetDarkModeManager() const;
	void SetDarkModeManager(DarkModeManager *darkModeManager);

	DriveModel *GetDriveModel();
	const DriveModel *GetDriveModel() const;
	void SetDriveModel(DriveModel *driveModel);

	FrequentLocationsModel *GetFrequentLocationsModel();
	const FrequentLocationsModel *GetFrequentLocationsModel() const;
	void SetFrequentLocationsModel(FrequentLocationsModel *frequentLocationsModel);

	HistoryModel *GetHistoryModel();
	const HistoryModel *GetHistoryModel() const;
	void SetHistoryModel(HistoryModel *historyModel);

	ModelessDialogList *GetModelessDialogList();
	const ModelessDialogList *GetModelessDialogList() const;
	void SetModelessDialogList(ModelessDialogList *modelessDialogList);

	NavigationEvents *GetNavigationEvents();
	const NavigationEvents *GetNavigationEvents() const;
	void SetNavigationEvents(NavigationEvents *navigationEvents);

	PlatformContext *GetPlatformContext();
	const PlatformContext *GetPlatformContext() const;
	void SetPlatformContext(PlatformContext *platformContext);

	ResourceLoader *GetResourceLoader();
	const ResourceLoader *GetResourceLoader() const;
	void SetResourceLoader(ResourceLoader *resourceLoader);

	ShellBrowserEvents *GetShellBrowserEvents();
	const ShellBrowserEvents *GetShellBrowserEvents() const;
	void SetShellBrowserEvents(ShellBrowserEvents *shellBrowserEvents);

	TabEvents *GetTabEvents();
	const TabEvents *GetTabEvents() const;
	void SetTabEvents(TabEvents *tabEvents);

	TabList *GetTabList();
	const TabList *GetTabList() const;
	void SetTabList(TabList *tabList);

	TabRestorer *GetTabRestorer();
	const TabRestorer *GetTabRestorer() const;
	void SetTabRestorer(TabRestorer *tabRestorer);

	void CheckFullyInitialized() const;

private:
	template <class T>
	T *Get();

	template <class T>
	const T *Get() const;

	template <class T>
	void Set(T *service);

	std::tuple<AcceleratorManager *, AppController *, Applications::ApplicationModel *,
		BrowserList *, ColorRuleModel *, const CommandLine::Settings *, Config *,
		DarkModeColorProvider *, DarkModeManager *, DriveModel *, FrequentLocationsModel *,
		HistoryModel *, ModelessDialogList *, NavigationEvents *, PlatformContext *,
		ResourceLoader *, ShellBrowserEvents *, TabEvents *, TabList *, TabRestorer *>
		m_services;
};
