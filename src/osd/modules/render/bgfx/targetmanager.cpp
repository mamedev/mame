// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  targetmanager.cpp - BGFX render target manager
//
//  Maintains a per-screen string-to-entry mapping for any
//  registered render targets.
//
//============================================================

#include "targetmanager.h"

#include "bgfxutil.h"
#include "target.h"

#include "modules/lib/osdobj_common.h"

#include <utility>


const int32_t target_manager::MAX_SCREENS = 100;

target_manager::target_manager(texture_manager& textures)
	: m_textures(textures)
	, m_screen_count(0)
{
}

target_manager::~target_manager()
{
	for (auto iter = m_targets.begin(); iter != m_targets.end(); iter = m_targets.erase(iter))
		m_textures.remove_provider(iter->first);
}

bgfx_target* target_manager::create_target(
		std::string &&name,
		bgfx::TextureFormat::Enum format,
		uint16_t width,
		uint16_t height,
		uint16_t xprescale,
		uint16_t yprescale,
		uint32_t style,
		bool double_buffer,
		bool filter,
		uint16_t scale,
		uint32_t screen)
{
	const std::string full_name = name + std::to_string(screen);

	auto iter = m_targets.find(full_name);
	if (iter != m_targets.end())
	{
		iter->second.reset();
		m_textures.remove_provider(full_name);
	}

	auto target = std::make_unique<bgfx_target>(std::move(name), format, width, height, xprescale, yprescale, style, double_buffer, filter, scale, screen);
	if (iter != m_targets.end())
		iter->second = std::move(target);
	else
		iter = m_targets.emplace(full_name, std::move(target)).first;
	m_textures.add_provider(full_name, *iter->second);

	return iter->second.get();
}

void target_manager::destroy_target(const std::string &name, uint32_t screen)
{
	const std::string full_name = (screen < 0) ? name : (name + std::to_string(screen));
	const auto found = m_targets.find(full_name);
	if (found != m_targets.end())
	{
		m_targets.erase(found);
		m_textures.remove_provider(full_name);
	}
}

bgfx_target* target_manager::create_backbuffer(void *handle, uint16_t width, uint16_t height)
{
	auto target = std::make_unique<bgfx_target>(handle, width, height);
	return (m_targets["backbuffer"] = std::move(target)).get();
}

bgfx_target* target_manager::target(uint32_t screen, const std::string &name)
{
	const std::string full_name = name + std::to_string(screen);
	const auto found = m_targets.find(full_name);
	if (found != m_targets.end())
	{
		return found->second.get();
	}
	else
	{
		osd_printf_verbose("Warning: Attempting to retrieve a nonexistent target '%s' for screen %d\n", name, screen);
		return nullptr;
	}
}

bool target_manager::update_target_sizes(uint32_t screen, uint16_t width, uint16_t height, uint32_t style, uint16_t user_prescale, uint16_t max_prescale_size)
{
	if (style == TARGET_STYLE_CUSTOM)
		return false;

	std::vector<osd_dim> &sizes = (style == TARGET_STYLE_GUEST) ? m_guest_dims : m_native_dims;

	// Ensure that there's an entry to fill
	while (sizes.size() <= screen)
	{
		sizes.emplace_back(0, 0);
	}

	if (width != sizes[screen].width() || height != sizes[screen].height())
	{
		sizes[screen] = osd_dim(width, height);
		rebuild_targets(screen, style, user_prescale, max_prescale_size);
		return true;
	}

	return false;
}

void target_manager::rebuild_targets(uint32_t screen, uint32_t style, uint16_t user_prescale, uint16_t max_prescale_size)
{
	if (style == TARGET_STYLE_CUSTOM) return;

	std::vector<bgfx_target*> to_resize;
	for (const auto &[name, target] : m_targets)
	{
		if (target && (target->style() == style) && (target->screen_index() == screen))
			to_resize.push_back(target.get());
	}

	std::vector<osd_dim>& sizes = style == TARGET_STYLE_GUEST ? m_guest_dims : m_native_dims;
	for (bgfx_target* target : to_resize)
	{
		std::string name = target->name();
		const bgfx::TextureFormat::Enum format = target->format();
		const bool double_buffered = target->double_buffered();
		const bool filter = target->filter();
		const uint16_t scale = target->scale();
		const uint16_t width(sizes[screen].width());
		const uint16_t height(sizes[screen].height());
		uint16_t xprescale = user_prescale;
		uint16_t yprescale = user_prescale;
		bgfx_util::find_prescale_factor(width, height, max_prescale_size, xprescale, yprescale);

		create_target(std::move(name), format, width, height, xprescale, yprescale, style, double_buffered, filter, scale, screen);
	}
}

void target_manager::update_screen_count(uint32_t count, uint16_t user_prescale, uint16_t max_prescale_size)
{
	// Ensure that there's an entry to fill
	while (count > m_native_dims.size())
	{
		m_native_dims.emplace_back(osd_dim(0, 0));
	}

	if (count != m_screen_count)
	{
		uint32_t old_count = m_screen_count;
		m_screen_count = count;
		if (m_screen_count > old_count)
		{
			for (uint32_t screen = old_count; screen < m_screen_count; screen++)
			{
				create_output_if_nonexistent(screen, user_prescale, max_prescale_size);
			}
		}
	}
}

void target_manager::create_output_if_nonexistent(uint32_t screen, uint16_t user_prescale, uint16_t max_prescale_size)
{
	if (m_targets.find("output" + std::to_string(screen)) != m_targets.end())
	{
		return;
	}

	uint16_t width(m_native_dims[screen].width());
	uint16_t height(m_native_dims[screen].height());
	uint16_t xprescale = user_prescale;
	uint16_t yprescale = user_prescale;
	bgfx_util::find_prescale_factor(width, height, max_prescale_size, xprescale, yprescale);

	create_target("output", bgfx::TextureFormat::BGRA8, width, height, xprescale, yprescale, TARGET_STYLE_NATIVE, false, false, 1, screen);
}

uint16_t target_manager::width(uint32_t style, uint32_t screen)
{
	std::vector<osd_dim>& sizes = style == TARGET_STYLE_GUEST ? m_guest_dims : m_native_dims;
	return screen < sizes.size() ? sizes[screen].width() : 0;
}

uint16_t target_manager::height(uint32_t style, uint32_t screen)
{
	std::vector<osd_dim>& sizes = style == TARGET_STYLE_GUEST ? m_guest_dims : m_native_dims;
	return screen < sizes.size() ? sizes[screen].height() : 0;
}
