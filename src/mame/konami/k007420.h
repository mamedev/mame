// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Acho A. Tang, R. Belmont
#ifndef MAME_KONAMI_K007420_H
#define MAME_KONAMI_K007420_H

#pragma once

#include "emupal.h"


class k007420_device : public device_t, public device_gfx_interface
{
public:
	using sprite_delegate = device_delegate<void (uint32_t &code, uint32_t &color)>;

	k007420_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	template<typename T> k007420_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&palette_tag, const gfx_decode_entry *gfxinfo)
		: k007420_device(mconfig, tag, owner, clock)
	{
		set_info(gfxinfo);
		set_palette(std::forward<T>(palette_tag));
	}

	void set_bank_limit(uint32_t limit) { m_banklimit = limit; }
	template <typename... T> void set_sprite_callback(T &&... args) { m_callback.set(std::forward<T>(args)...); }

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);
	void sprites_draw(bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// internal state
	std::unique_ptr<uint8_t[]> m_ram;

	bool            m_flipscreen;   // current code uses the 7342 flipscreen!!
	uint8_t         m_regs[8];      // current code uses the 7342 regs!! (only [2])
	uint32_t        m_banklimit;
	sprite_delegate m_callback;
};

DECLARE_DEVICE_TYPE(K007420, k007420_device)

#endif // MAME_KONAMI_K007420_H
