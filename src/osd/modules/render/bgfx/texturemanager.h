// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  texturemanager.h - BGFX texture manager
//
//  Maintains a string-to-entry mapping for any registered
//  textures.
//
//============================================================

#ifndef MAME_RENDER_BGFX_TEXTUREMANAGER_H
#define MAME_RENDER_BGFX_TEXTUREMANAGER_H

#pragma once

#include "bitmap.h"

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <utility>

#include <bgfx/bgfx.h>


class bgfx_texture_handle_provider;
class bgfx_texture;

class texture_manager
{
public:
	texture_manager();
	~texture_manager();

	bgfx_texture* create_texture(const std::string &name, bgfx::TextureFormat::Enum format, uint32_t width, uint32_t width_margin, uint32_t height, void* data = nullptr, uint32_t flags = BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP);
	bgfx_texture* create_png_texture(const std::string &path, const std::string &file_name, std::string texture_name, uint32_t flags = BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP, uint32_t screen = -1);
	void add_provider(const std::string &name, std::unique_ptr<bgfx_texture_handle_provider> &&texture);
	void add_provider(const std::string &name, bgfx_texture_handle_provider &texture);
	void remove_provider(const std::string &name);
	bgfx::TextureHandle create_or_update_mame_texture(uint32_t format, int width, int width_margin, int height,
		int rowpixels, const rgb_t *palette, void *base, uint32_t seqid, uint32_t flags, uint64_t key, uint64_t old_key);

	// Getters
	bgfx::TextureHandle handle(const std::string &name);
	bgfx_texture_handle_provider* provider(const std::string &name);

private:
	bgfx_texture* create_texture(const std::string &name);

	struct sequenced_handle
	{
		bgfx::TextureHandle handle;
		uint32_t seqid;
		int width;
		int height;
	};

	std::map<std::string, std::pair<bgfx_texture_handle_provider *, std::unique_ptr<bgfx_texture_handle_provider> > > m_textures;
	std::map<uint64_t, sequenced_handle> m_mame_textures;
};

#endif // MAME_RENDER_BGFX_TEXTUREMANAGER_H
