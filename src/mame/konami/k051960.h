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


#define K051960_CB_MEMBER(_name)   void _name(int *code, int *color, int *priority, bool *shadow)


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

	k051960_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto irq_handler() { return m_irq_handler.bind(); }

	auto nmi_handler() { return m_nmi_handler.bind(); }


	// static configuration
	template <typename... T> void set_sprite_callback(T &&... args) { m_k051960_cb.set(std::forward<T>(args)...); }
	void set_plane_order(int order);
	void set_shadow_inv(bool inv);

	/*
	The callback is passed:
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

	u8 k051960_r(offs_t offset);
	void k051960_w(offs_t offset, u8 data);

	u8 k051937_r(offs_t offset);
	void k051937_w(offs_t offset, u8 data);

	void k051960_sprites_draw(bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, int min_priority, int max_priority);

	TIMER_CALLBACK_MEMBER(scanline_callback);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// internal state
	std::unique_ptr<uint8_t[]>   m_ram;

	required_region_ptr<uint8_t> m_sprite_rom;

	emu_timer *m_scanline_timer;

	sprite_delegate m_k051960_cb;

	devcb_write_line m_irq_handler;
	// TODO: is this even used by anything?
	devcb_write_line m_firq_handler;
	devcb_write_line m_nmi_handler;

	uint8_t m_spriterombank[3];
	uint8_t m_romoffset;
	bool    m_spriteflip, m_readroms;
	uint8_t m_shadow_config;
	bool    m_inv_shadow;
	bool    m_nmi_enabled;

	int k051960_fetchromdata( int byte );
};

DECLARE_DEVICE_TYPE(K051960, k051960_device)

#endif // MAME_KONAMI_K051960_H
