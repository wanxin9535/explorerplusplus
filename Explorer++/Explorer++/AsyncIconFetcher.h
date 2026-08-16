// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#pragma once

#include "../Helper/ShellHelper.h"
#include "../Helper/WeakPtr.h"
#include "../Helper/WeakPtrFactory.h"
#include <boost/core/noncopyable.hpp>
#include <concurrencpp/concurrencpp.h>
#include <shtypes.h>
#include <stop_token>

class CachedIcons;
class Runtime;

class AsyncIconFetcher : private boost::noncopyable
{
public:
	AsyncIconFetcher(const Runtime *runtime, CachedIcons *cachedIcons);

	[[nodiscard]] concurrencpp::lazy_result<std::optional<ShellIconInfo>> GetIconIndexAsync(
		PCIDLIST_ABSOLUTE pidl, std::stop_token stopToken);
	int GetCachedIconIndexOrDefault(PCIDLIST_ABSOLUTE pidl) const;
	std::optional<int> MaybeGetCachedIconIndex(PCIDLIST_ABSOLUTE pidl) const;
	int GetDefaultIconIndex(PCIDLIST_ABSOLUTE pidl) const;

private:
	static concurrencpp::lazy_result<std::optional<ShellIconInfo>> GetIconIndexAsyncImpl(
		WeakPtr<AsyncIconFetcher> weakSelf, PidlAbsolute pidl, std::stop_token stopToken);

	const Runtime *const m_runtime;
	CachedIcons *const m_cachedIcons;
	int m_defaultFileIconIndex;
	int m_defaultFolderIconIndex;

	WeakPtrFactory<AsyncIconFetcher> m_weakPtrFactory{ this };
};
