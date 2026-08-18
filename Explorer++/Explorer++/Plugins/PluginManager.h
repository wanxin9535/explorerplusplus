// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#pragma once

#include "PluginInterface.h"
#include "Plugins/LuaPlugin.h"

struct Config;

namespace std
{
namespace filesystem
{
class path;
}
}

namespace Plugins
{
class PluginManager
{
public:
	PluginManager(PluginInterface *pluginInterface, const Config *config);

	void LoadAllPlugins(const std::filesystem::path &pluginDirectory);

private:
	static constexpr wchar_t MANIFEST_NAME[] = L"plugin.json";

	bool AttemptToLoadPlugin(const std::filesystem::path &directory);
	bool RegisterPlugin(const std::filesystem::path &directory, const Manifest &manifest);
	static std::vector<ShortcutKey> ConvertPluginShortcutKeys(
		const std::vector<Plugins::PluginShortcutKey> &pluginShortcutKeys);
	void ApplyShortcutKeys(const std::vector<ShortcutKey> &shortcutKeys);

	PluginInterface *m_pluginInterface;
	const Config *const m_config;

	std::vector<std::unique_ptr<Plugins::LuaPlugin>> m_plugins;
};
}
