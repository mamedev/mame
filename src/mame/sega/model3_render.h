// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_SEGA_MODEL3_RENDER_H
#define MAME_SEGA_MODEL3_RENDER_H

#pragma once

#include "model3_pci.h"

#include <array>

// Decodes Real3D texture uploads and hands graphics memory to the renderer
class model3_real3d_renderer_device : public device_t
{
public:
	struct memory_view
	{
		std::span<const u32> display_list, culling, polygon, vrom;
		std::span<const u32, 4> buffer_config;
		std::array<std::span<const u16>, 2> texture;
	};
	using sync_delegate = device_delegate<void ()>;
	using invalidate_delegate = device_delegate<void (u32)>;
	// the view is only valid during the callback
	using submit_delegate = device_delegate<void (const memory_view &)>;

	model3_real3d_renderer_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// binds the graphics device's texture, VROM and submit callbacks
	void set_graphics(model3_real3d_pci_device &graphics);
	// invalidation discards every cached texture on the given page
	template <typename... T> void set_sync_callback(T &&... args) { m_sync_callback.set(std::forward<T>(args)...); }
	template <typename... T> void set_invalidate_callback(T &&... args) { m_invalidate_callback.set(std::forward<T>(args)...); }
	template <typename... T> void set_submit_callback(T &&... args) { m_submit_callback.set(std::forward<T>(args)...); }

	// 2048x1024 texel sheet
	std::span<const u16> texture_ram(u32 page) const;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_pre_save() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;

private:
	struct level { u32 x, y, width, height; std::size_t offset; };

	static constexpr u32 WIDTH = 2048;
	static constexpr u32 HEIGHT = 1024;

	void texture(u32 header, u32 length, std::span<const u32> source);
	void vrom(u32 address, u32 header, u32 third, std::span<const u32> source);
	bool upload(u32 header, std::span<const u32> source);
	void write_level(const level &mip, u32 header, std::span<const u32> source);
	void submit();
	void synchronize();
	void invalidate_all();

	required_device<model3_real3d_pci_device> m_graphics;
	memory_share_array_creator<u16, 2> m_texture_ram;
	sync_delegate m_sync_callback;
	invalidate_delegate m_invalidate_callback;
	submit_delegate m_submit_callback;
};

DECLARE_DEVICE_TYPE(MODEL3_REAL3D_RENDERER, model3_real3d_renderer_device)

#endif // MAME_SEGA_MODEL3_RENDER_H
