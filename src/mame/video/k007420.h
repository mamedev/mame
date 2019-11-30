// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Acho A. Tang, R. Belmont
#ifndef MAME_VIDEO_K007420_H
#define MAME_VIDEO_K007420_H

#pragma once

#include "emupal.h"


class k007420_device : public device_t
{
public:
	using sprite_delegate = device_delegate<void (int *code, int *color)>;

	k007420_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_palette_tag(T &&tag) { m_palette.set_tag(std::forward<T>(tag)); }
	void set_bank_limit(int limit) { m_banklimit = limit; }
	template <typename... T> void set_sprite_callback(T &&... args) { m_callback.set(std::forward<T>(args)...); }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	void sprites_draw(bitmap_ind16 &bitmap, const rectangle &cliprect, gfx_element *gfx);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
private:
	// internal state
	std::unique_ptr<uint8_t[]>        m_ram;

	int          m_flipscreen;    // current code uses the 7342 flipscreen!!
	uint8_t        m_regs[8];   // current code uses the 7342 regs!! (only [2])
	required_device<palette_device> m_palette;
	int                m_banklimit;
	sprite_delegate m_callback;
};

DECLARE_DEVICE_TYPE(K007420, k007420_device)

// function definition for a callback
#define K007420_CALLBACK_MEMBER(_name)     void _name(int *code, int *color)


#endif // MAME_VIDEO_K007420_H
