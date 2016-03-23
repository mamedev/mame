// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  target.cpp - Render target abstraction for BGFX layer
//
//============================================================

#include "emu.h"

#include "target.h"

bgfx_target::bgfx_target(std::string name, bgfx::TextureFormat::Enum format, uint16_t width, uint16_t height, uint32_t prescale_x, uint32_t prescale_y, uint32_t style, int32_t index, bool double_buffer, bool filter, bool init, bool output)
	: m_name(name)
	, m_format(format)
	, m_targets(nullptr)
	, m_textures(nullptr)
	, m_width(width)
	, m_height(height)
    , m_prescale_x(prescale_x)
    , m_prescale_y(prescale_y)
    , m_double_buffer(double_buffer)
	, m_style(style)
	, m_filter(filter)
    , m_output(output)
    , m_index(index)
    , m_current_page(0)
	, m_initialized(false)
	, m_page_count(double_buffer ? 2 : 1)
{
	if (init)
	{
		uint32_t wrap_mode = BGFX_TEXTURE_U_CLAMP | BGFX_TEXTURE_V_CLAMP;
		uint32_t filter_mode = filter ? (BGFX_TEXTURE_MIN_ANISOTROPIC | BGFX_TEXTURE_MAG_ANISOTROPIC) : (BGFX_TEXTURE_MIN_POINT | BGFX_TEXTURE_MAG_POINT | BGFX_TEXTURE_MIP_POINT);

		m_textures = new bgfx::TextureHandle[m_page_count];
		m_targets = new bgfx::FrameBufferHandle[m_page_count];
		for (int page = 0; page < m_page_count; page++)
		{
			m_textures[page] = bgfx::createTexture2D(width * prescale_x, height * prescale_y, 1, format, wrap_mode | filter_mode | BGFX_TEXTURE_RT);
            assert(m_textures[page].idx != 0xffff);
			m_targets[page] = bgfx::createFrameBuffer(1, &m_textures[page], false);
            assert(m_targets[page].idx != 0xffff);
        }

		m_initialized = true;
	}
}

bgfx_target::bgfx_target(void *handle, uint16_t width, uint16_t height)
	: m_name("backbuffer")
	, m_format(bgfx::TextureFormat::Unknown)
	, m_targets(nullptr)
	, m_textures(nullptr)
	, m_width(width)
	, m_height(height)
    , m_prescale_x(1)
    , m_prescale_y(1)
    , m_double_buffer(false)
	, m_style(TARGET_STYLE_CUSTOM)
	, m_filter(false)
    , m_index(-1)
	, m_current_page(0)
    , m_initialized(true)
	, m_page_count(0)
{
	m_targets = new bgfx::FrameBufferHandle[1];
	m_targets[0] = bgfx::createFrameBuffer(handle, width, height);

	// No backing texture
}

bgfx_target::~bgfx_target()
{
	if (!m_initialized)
    {
        return;
    }

	if (m_page_count > 0)
	{
		for (int page = 0; page < m_page_count; page++)
		{
			bgfx::destroyFrameBuffer(m_targets[page]);
            bgfx::destroyTexture(m_textures[page]);
        }
		delete [] m_textures;
		delete [] m_targets;
	}
	else
	{
		bgfx::destroyFrameBuffer(m_targets[0]);
		delete [] m_targets;
	}
}

void bgfx_target::page_flip()
{
	if (!m_initialized) return;

	if (m_double_buffer)
	{
		m_current_page = 1 - m_current_page;
	}
}

bgfx::FrameBufferHandle bgfx_target::target()
{
	if (!m_initialized) return BGFX_INVALID_HANDLE;
	return m_targets[m_current_page];
}

bgfx::TextureHandle bgfx_target::texture() const
{
	if (!m_initialized) return BGFX_INVALID_HANDLE;

    if (m_double_buffer)
    {
        return m_textures[1 - m_current_page];
    }
    else
    {
        return m_textures[m_current_page];
    }
}

