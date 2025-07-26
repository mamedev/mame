// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Acho A. Tang, R. Belmont
#ifndef MAME_KONAMI_K051960_H
#define MAME_KONAMI_K051960_H

#pragma once

enum
{
	K051960_PLANEORDER_BASE = 0,
	K051960_PLANEORDER_MIA,
	K051960_PLANEORDER_GRADIUS3
};


#define K051960_CB_MEMBER(_name) void _name(int *code, int *color, int *priority, bool *shadow)


class k051960_device : public device_t, public device_gfx_interface, public device_video_interface
{
	static const gfx_layout spritelayout;
	static const gfx_layout spritelayout_reverse;
	static const gfx_layout spritelayout_gradius3;
	DECLARE_GFXDECODE_MEMBER(gfxinfo);
	DECLARE_GFXDECODE_MEMBER(gfxinfo_reverse);
	DECLARE_GFXDECODE_MEMBER(gfxinfo_gradius3);

public:
	using sprite_delegate = device_delegate<void (int *code, int *color, int *priority, bool *shadow)>;

	k051960_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration
	auto irq_handler() { return m_irq_handler.bind(); }
	//auto firq_handler() { return m_firq_handler.bind(); } // unused
	auto nmi_handler() { return m_nmi_handler.bind(); }

	auto k051937_shadow_mode() { return m_shadow_config_cb.bind(); }
	void set_priority_shadows(bool pri) { m_priority_shadows = pri; }

	/*
	The sprite callback is passed:
	- code (range 00-1FFF, output of the pins CA5-CA17)
	- color (range 00-FF, output of the pins OC0-OC7). Note that most of the
	  time COL7 seems to be "shadow", but not always (e.g. Aliens).
	The callback must put:
	- in code the resulting sprite number
	- in color the resulting color index
	- if necessary, in priority the priority of the sprite wrt tilemaps
	- if necessary, alter shadow to indicate whether the sprite has shadows enabled.
	  shadow is preloaded with color & 0x80 so it doesn't need to be changed unless
	  the game has special treatment (Aliens)
	*/

	template <typename... T> void set_sprite_callback(T &&... args) { m_k051960_cb.set(std::forward<T>(args)...); }
	void set_plane_order(int order);

	// public interface
	u8 k051960_r(offs_t offset);
	void k051960_w(offs_t offset, u8 data);

	u8 k051937_r(offs_t offset);
	void k051937_w(offs_t offset, u8 data);

	void k051960_sprites_draw(bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, int min_priority, int max_priority);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// internal state
	u8 m_ram[0x400];
	u8 m_buffer[0x400];

	required_region_ptr<u8> m_sprite_rom;

	sprite_delegate m_k051960_cb;
	devcb_write_line m_shadow_config_cb;

	devcb_write_line m_irq_handler;
	devcb_write_line m_firq_handler;
	devcb_write_line m_nmi_handler;

	emu_timer *m_firq_scanline;
	emu_timer *m_nmi_scanline;

	u8 m_spriterombank[3];
	u8 m_romoffset;
	u8 m_control;
	emu_timer *m_sprites_busy;
	u8 m_shadow_config;
	bool m_priority_shadows;

	u8 k051960_fetchromdata(offs_t offset);

	void vblank_callback(screen_device &screen, bool state);
	TIMER_CALLBACK_MEMBER(firq_scanline);
	TIMER_CALLBACK_MEMBER(nmi_scanline);
};

DECLARE_DEVICE_TYPE(K051960, k051960_device)

#endif // MAME_KONAMI_K051960_H
