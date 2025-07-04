// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Acho A. Tang, R. Belmont
#ifndef MAME_KONAMI_K007121_H
#define MAME_KONAMI_K007121_H

#pragma once


class k007121_device : public device_t, public device_gfx_interface, public device_video_interface
{
public:
	using dirtytiles_delegate = device_delegate<void()>;

	template<typename T, typename U>
	k007121_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, const gfx_decode_entry *gfxinfo, T &&palette_tag, U &&screen_tag)
		: k007121_device(mconfig, tag, owner, clock)
	{
		set_info(gfxinfo);
		set_palette(std::forward<T>(palette_tag));
		set_screen(std::forward<U>(screen_tag));
	}
	k007121_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto set_irq_cb() { return m_irq_cb.bind(); }
	//auto set_firq_cb() { return m_firq_cb.bind(); } // unused
	auto set_nmi_cb() { return m_nmi_cb.bind(); }
	auto set_flipscreen_cb() { return m_flipscreen_cb.bind(); }
	template <typename... T> void set_dirtytiles_cb(T &&... args) { m_dirtytiles_cb.set(std::forward<T>(args)...); }

	void set_spriteram(uint8_t *spriteram) { m_spriteram = spriteram; }

	bool flipscreen() { return m_flipscreen; }
	uint8_t ctrl_r(offs_t offset) { return m_ctrlram[offset & 7]; } // not from addressmap
	void ctrl_w(offs_t offset, uint8_t data);

	// scroll RAM (bits 1-7 are unused for 2nd half, but combatsc tests all 8 bits)
	uint8_t scroll_r(offs_t offset) { return m_scrollram[offset & 0x3f]; }
	void scroll_w(offs_t offset, uint8_t data) { m_scrollram[offset & 0x3f] = data; }

	void sprites_draw(bitmap_ind16 &bitmap, const rectangle &cliprect, int base_color, int global_x_offset, int bank_base, bitmap_ind8 &priority_bitmap, uint32_t pri_mask);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// internal state
	uint8_t m_ctrlram[8];
	uint8_t m_scrollram[0x40];
	bool m_flipscreen;
	uint8_t *m_spriteram;
	uint8_t m_sprites_buffer[0x800];
	emu_timer *m_scanline_timer;

	devcb_write_line m_flipscreen_cb;
	devcb_write_line m_irq_cb;
	devcb_write_line m_firq_cb;
	devcb_write_line m_nmi_cb;
	dirtytiles_delegate m_dirtytiles_cb;

	void sprites_buffer();
	TIMER_CALLBACK_MEMBER(scanline);
};

DECLARE_DEVICE_TYPE(K007121, k007121_device)

#endif // MAME_KONAMI_K007121_H
