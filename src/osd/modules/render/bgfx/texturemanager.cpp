// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  texturemanager.cpp - BGFX texture manager
//
//  Maintains a string-to-entry mapping for any registered
//  textures.
//
//============================================================

#include <bgfx/bgfx.h>

#include "texturemanager.h"
#include "texture.h"
#include "bgfxutil.h"
#include "fileio.h"
#include "rendutil.h"
#include "modules/render/copyutil.h"

texture_manager::~texture_manager()
{
	for (std::pair<std::string, bgfx_texture_handle_provider*> texture : m_textures)
	{
		if (texture.second != nullptr && !(texture.second)->is_target())
		{
			delete texture.second;
		}
	}
	m_textures.clear();

	for (std::pair<uint64_t, sequenced_handle> mame_texture : m_mame_textures)
	{
		bgfx::destroy(mame_texture.second.handle);
	}
	m_mame_textures.clear();
}

void texture_manager::add_provider(std::string name, bgfx_texture_handle_provider* provider)
{
	auto iter = m_textures.find(name);
	if (iter != m_textures.end())
	{
		iter->second = provider;
	}
	else
	{
		m_textures[name] = provider;
	}
}

bgfx_texture* texture_manager::create_texture(std::string name, bgfx::TextureFormat::Enum format, uint32_t width, uint32_t height, void* data, uint32_t flags)
{
	auto* texture = new bgfx_texture(name, format, width, height, flags, data);
	m_textures[name] = texture;
	return texture;
}

bgfx_texture* texture_manager::create_png_texture(std::string path, std::string file_name, std::string texture_name, uint32_t flags, uint32_t screen)
{
	bitmap_argb32 bitmap;
	emu_file file(path, OPEN_FLAG_READ);
	if (!file.open(file_name))
	{
		render_load_png(bitmap, file);
		file.close();
	}

	if (bitmap.width() == 0 || bitmap.height() == 0)
	{
		printf("Unable to load PNG '%s' from path '%s'\n", file_name.c_str(), path.c_str());
		return nullptr;
	}

	auto *data = new uint8_t[bitmap.width() * bitmap.height() * 4];
	auto *data32 = reinterpret_cast<uint32_t *>(data);

	const uint32_t width = bitmap.width();
	const uint32_t height = bitmap.height();
	const uint32_t rowpixels = bitmap.rowpixels();
	auto* base = reinterpret_cast<uint32_t *>(bitmap.raw_pixptr(0));
	for (int y = 0; y < height; y++)
	{
		copy_util::copyline_argb32(data32 + y * width, base + y * rowpixels, width, nullptr);
	}

	if (screen >= 0)
	{
		texture_name += std::to_string(screen);
	}
	bgfx_texture* texture = create_texture(texture_name, bgfx::TextureFormat::BGRA8, width, height, data, flags);

	delete [] data;

	return texture;
}

bgfx::TextureHandle texture_manager::create_or_update_mame_texture(uint32_t format, int width, int height
	, int rowpixels, const rgb_t *palette, void *base, uint32_t seqid, uint32_t flags, uint64_t key, uint64_t old_key)
{
	bgfx::TextureHandle handle = BGFX_INVALID_HANDLE;
	if (old_key != ~0ULL)
	{
		auto iter = m_mame_textures.find(old_key);
		if (iter != m_mame_textures.end())
		{
			handle = iter->second.handle;
			if (handle.idx == bgfx::kInvalidHandle)
				return handle;

			if (iter->second.width == width && iter->second.height == height)
			{
				// Size matches, so let's just swap the old handle into the new location
				m_mame_textures[key] = { handle, seqid, width, height };
				m_mame_textures[old_key] = { BGFX_INVALID_HANDLE, 0, 0, 0 };

				if (iter->second.seqid == seqid)
				{
					// Everything matches, just return the existing handle
					return handle;
				}
				else
				{
					bgfx::TextureFormat::Enum dst_format = bgfx::TextureFormat::BGRA8;
					uint16_t pitch = width;
					int width_div_factor = 1;
					int width_mul_factor = 1;
					const bgfx::Memory* mem = bgfx_util::mame_texture_data_to_bgfx_texture_data(dst_format, format, rowpixels, height, palette, base, pitch, width_div_factor, width_mul_factor);
					bgfx::updateTexture2D(handle, 0, 0, 0, 0, (uint16_t)((rowpixels * width_mul_factor) / width_div_factor), (uint16_t)height, mem, pitch);
					return handle;
				}
			}
			bgfx::destroy(handle);
			m_mame_textures[old_key] = { BGFX_INVALID_HANDLE, 0, 0, 0 };
		}
	}
	else
	{
		auto iter = m_mame_textures.find(key);
		if (iter != m_mame_textures.end())
		{
			handle = iter->second.handle;
			if (handle.idx == bgfx::kInvalidHandle)
				return handle;

			if (iter->second.width == width && iter->second.height == height)
			{
				if (iter->second.seqid == seqid)
				{
					return handle;
				}
				else
				{
					bgfx::TextureFormat::Enum dst_format = bgfx::TextureFormat::BGRA8;
					uint16_t pitch = width;
					int width_div_factor = 1;
					int width_mul_factor = 1;
					const bgfx::Memory* mem = bgfx_util::mame_texture_data_to_bgfx_texture_data(dst_format, format, rowpixels, height, palette, base, pitch, width_div_factor, width_mul_factor);
					bgfx::updateTexture2D(handle, 0, 0, 0, 0, (uint16_t)((rowpixels * width_mul_factor) / width_div_factor), (uint16_t)height, mem, pitch);
					return handle;
				}
			}
			bgfx::destroy(handle);
		}
	}

	bgfx::TextureFormat::Enum dst_format = bgfx::TextureFormat::BGRA8;
	uint16_t pitch = width;
	int width_div_factor = 1;
	int width_mul_factor = 1;
	const bgfx::Memory* mem = bgfx_util::mame_texture_data_to_bgfx_texture_data(dst_format, format, rowpixels, height, palette, base, pitch, width_div_factor, width_mul_factor);
	const uint16_t adjusted_width = (uint16_t)((rowpixels * width_mul_factor) / width_div_factor);
	handle = bgfx::createTexture2D(adjusted_width, height, false, 1, dst_format, flags, nullptr);
	bgfx::updateTexture2D(handle, 0, 0, 0, 0, adjusted_width, (uint16_t)height, mem, pitch);

	m_mame_textures[key] = { handle, seqid, width, height };
	return handle;
}

bgfx::TextureHandle texture_manager::handle(std::string name)
{
	bgfx::TextureHandle handle = BGFX_INVALID_HANDLE;
	auto iter = m_textures.find(name);
	if (iter != m_textures.end())
	{
		handle = (iter->second)->texture();
	}

	assert(handle.idx != bgfx::kInvalidHandle);
	return handle;
}

bgfx_texture_handle_provider* texture_manager::provider(std::string name)
{
	auto iter = m_textures.find(name);
	if (iter != m_textures.end())
	{
		return iter->second;
	}

	return nullptr;
}

void texture_manager::remove_provider(std::string name, bool delete_provider)
{
	if (provider(name) != nullptr)
	{
		if (delete_provider)
		{
			delete m_textures[name];
		}
		m_textures.erase(name);
	}
}
