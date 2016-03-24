// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  targetmanager.cpp - BGFX render target manager
//
//  Maintains a string-to-entry mapping for any registered
//  render targets.
//
//============================================================

#include <bgfx/bgfx.h>

#include <vector>

#include "modules/lib/osdobj_common.h"

#include "targetmanager.h"

#include "target.h"

const int32_t target_manager::MAX_SCREENS = 100;

target_manager::target_manager(osd_options& options, texture_manager& textures)
    : m_textures(textures)
    , m_options(options)
    , m_screen_count(0)
{
    m_guest_width = new uint16_t[MAX_SCREENS];
    m_guest_height = new uint16_t[MAX_SCREENS];
    memset(m_guest_width, 0, sizeof(uint16_t) * MAX_SCREENS);
    memset(m_guest_height, 0, sizeof(uint16_t) * MAX_SCREENS);
}

target_manager::~target_manager()
{
	for (std::pair<std::string, bgfx_target*> target : m_targets)
	{
        if (target.second != nullptr)
        {
            delete target.second;
        }
	}
	m_targets.clear();
    delete [] m_guest_width;
    delete [] m_guest_height;
}

bgfx_target* target_manager::create_target(std::string name, bgfx::TextureFormat::Enum format, uint32_t width, uint32_t height, uint32_t prescale_x, uint32_t prescale_y, uint32_t style, bool double_buffer, bool filter, bool output)
{
	bgfx_target* target = new bgfx_target(name, format, width, height, prescale_x, prescale_y, style, -1, double_buffer, filter, width > 0 && height > 0, output);
	m_targets[name] = target;

	m_textures.add_provider(name, target);
	return target;
}

bgfx_target* target_manager::create_backbuffer(void *handle, uint32_t width, uint32_t height)
{
	bgfx_target* target = new bgfx_target(handle, width, height);
	m_targets["backbuffer"] = target;
	return target;
}

bgfx_target* target_manager::target(std::string name)
{
	std::map<std::string, bgfx_target*>::iterator iter = m_targets.find(name);
	if (iter != m_targets.end())
	{
		return iter->second;
	}

	return nullptr;
}

void target_manager::update_guest_targets(int32_t screen, uint16_t width, uint16_t height)
{
    if (screen < 0 || screen >= MAX_SCREENS)
    {
        return;
    }

	if (width != m_guest_width[screen] || height != m_guest_height[screen])
	{
		m_guest_width[screen] = width;
		m_guest_height[screen] = height;
        ensure_guest_targets();
        rebuild_guest_targets(screen);
	}
}

void target_manager::ensure_guest_targets()
{
    // Build a list of all guest targets that end in 0 *or* don't end in a number
    std::vector<std::string> existing_names;
    for (std::pair<std::string, bgfx_target*> target_pair : m_targets)
    {
        bgfx_target* target = target_pair.second;
        if (target == nullptr || target->style() != TARGET_STYLE_GUEST)
        {
            continue;
        }

        // If the last character is not a digit, add it to the list
        std::string name = target->name();
        char last_char = name[name.length() - 1];
        if ((last_char & 0xf0) != 0x30) {
            existing_names.push_back(name);
        }

        // If the last character is a zero, and the stripped name isn't already in the list, strip it off and add it to the list
        if (last_char == 0x30)
        {
            std::string shortened_name = name.substr(0, name.length() - 1);
            bool exists = false;
            for (std::string other_name : existing_names)
            {
                if (other_name == shortened_name)
                {
                    exists = true;
                    break;
                }
            }
            if (!exists)
            {
                existing_names.push_back(shortened_name);
            }
        }
    }

    for (std::string name : existing_names)
    {
        create_nonexistent_targets(name);
    }
}

void target_manager::rebuild_guest_targets(int32_t screen)
{
    std::vector<bgfx_target*> to_resize;
    for (std::pair<std::string, bgfx_target*> target : m_targets) {
        bgfx_target* target_ptr = target.second;
        if (target_ptr == nullptr) {
            continue;
        }
        bool is_guest = target_ptr->style() == TARGET_STYLE_GUEST;
        bool is_desired_screen = target_ptr->index() == screen || target_ptr->index() == -1;
        std::string name = target_ptr->name();
        bool is_indexed = (name[name.length() - 1] & 0xf0) == 0x30;
        if (is_guest && is_desired_screen && is_indexed) {
            to_resize.push_back(target.second);
        }
    }

    for (bgfx_target* target : to_resize) {
        std::string name = target->name();
        const bgfx::TextureFormat::Enum format = target->format();
        const bool double_buffered = target->double_buffered();
        const bool filter = target->filter();
        const uint32_t prescale_x = target->prescale_x();
        const uint32_t prescale_y = target->prescale_y();
        delete target;

        const uint16_t width = m_guest_width[screen];
        const uint16_t height = m_guest_height[screen];
        const bool init = width > 0 && height > 0;
        m_targets[name] = new bgfx_target(name, format, width, height, prescale_x, prescale_y, TARGET_STYLE_GUEST, screen, double_buffered, filter, init);
        m_textures.add_provider(name, m_targets[name]);
    }
}

void target_manager::update_screen_count(uint32_t count)
{
    if (count != m_screen_count)
    {
        uint32_t old_count = m_screen_count;
        m_screen_count = count;
        if (m_screen_count > old_count)
        {
            create_nonexistent_targets("output");
            create_nonexistent_targets("previous");
        }
    }
}

void target_manager::create_nonexistent_targets(std::string name)
{
    bool double_buffered = true;
    bool filter = true;
    if (m_targets[name + "0"] != nullptr)
    {
        double_buffered = m_targets[name + "0"]->double_buffered();
        filter = m_targets[name + "0"]->filter();
    }

    for (uint32_t screen = 0; screen < m_screen_count; screen++)
    {
        create_guest_if_nonexistent(name, screen, double_buffered, filter);
    }
}

bool target_manager::create_guest_if_nonexistent(std::string name, int32_t screen, bool double_buffered, bool filter)
{
    std::string full_name = name + std::to_string(screen);
    if (m_targets[full_name] != nullptr)
    {
        return false;
    }

    bool init = m_guest_width[screen] > 0 && m_guest_height[screen] > 0;
    m_targets[full_name] = new bgfx_target(full_name, bgfx::TextureFormat::RGBA8, m_guest_width[screen], m_guest_height[screen], m_options.bgfx_prescale_x(), m_options.bgfx_prescale_y(), TARGET_STYLE_GUEST, screen, double_buffered, filter, init);
    return true;
}