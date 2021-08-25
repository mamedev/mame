// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  texture.cpp - Texture abstraction for BGFX layer
//
//============================================================

#include <cstring>

#include "texture.h"

bgfx_texture::bgfx_texture(std::string name, bgfx::TextureFormat::Enum format, uint16_t width, uint16_t height, uint32_t flags, void* data)
	: m_name(name)
	, m_format(format)
	, m_width(width)
	, m_height(height)
	, m_rowpixels(width)
	, m_convert_stride(1)
{
	bgfx::TextureInfo info;
	bgfx::calcTextureSize(info, width, height, 1, false, false, 1, format);
	if (data != nullptr)
	{
		m_texture = bgfx::createTexture2D(width, height, false, 1, format, flags, bgfx::copy(data, info.storageSize));
	}
	else
	{
		m_texture = bgfx::createTexture2D(width, height, false, 1, format, flags);

		const bgfx::Memory* memory = bgfx::alloc(info.storageSize);
		memset(memory->data, 0, info.storageSize);
		bgfx::updateTexture2D(m_texture, 0, 0, 0, 0, width, height, memory, info.storageSize / height);
	}
}

bgfx_texture::bgfx_texture(std::string name, bgfx::TextureFormat::Enum format, uint16_t width, uint16_t height, const bgfx::Memory* data, uint32_t flags, uint16_t pitch, uint16_t rowpixels, int convert_stride)
	: m_name(name)
	, m_format(format)
	, m_width(width)
	, m_height(height)
	, m_rowpixels(rowpixels ? rowpixels : width)
	, m_convert_stride(convert_stride)
{
	bgfx::TextureInfo info;
	bgfx::calcTextureSize(info, m_rowpixels / m_convert_stride, height, 1, false, false, 1, format);
	m_texture = bgfx::createTexture2D(m_rowpixels / m_convert_stride, height, false, 1, format, flags, nullptr);
	bgfx::updateTexture2D(m_texture, 0, 0, 0, 0, m_rowpixels / m_convert_stride, height, data, pitch);
}

bgfx_texture::~bgfx_texture()
{
	bgfx::destroy(m_texture);
}

void bgfx_texture::update(const bgfx::Memory *data, uint16_t pitch)
{
	bgfx::updateTexture2D(m_texture, 0, 0, 0, 0, m_rowpixels / m_convert_stride, m_height, data, pitch);
}
