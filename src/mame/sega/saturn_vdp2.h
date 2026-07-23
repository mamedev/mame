// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_SEGA_SATURN_VDP2_H
#define MAME_SEGA_SATURN_VDP2_H

#pragma once

#include "screen.h"

class saturn_vdp2_device : public device_t
{
public:
	// construction/destruction
	saturn_vdp2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void regs_map(address_map &map);

	void set_is_pal(bool is_pal) { m_is_pal = is_pal; }

	template <typename T> void set_screen_tag(T &&tag) { m_screen.set_tag(std::forward<T>(tag)); }

	// TODO: follows stuff that eventually needs to be privatized
	void flip_odd_bit() { m_odd_bit ^= 1; };
	u8 get_hreso() { return m_hreso; }
	u8 get_vreso() { return m_vreso; }
	bool get_disp() { return m_disp; }
	bool get_bdclmd() { return m_bdclmd; }
	u8 get_lsmd() { return m_lsmd; }
	int get_vblank_start_position();
	int get_ystep_count();
	bool get_vramsz() { return m_vramsz; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_clock_changed() override;
private:
	required_device<screen_device> m_screen;

	// CRTC
	bool m_is_pal;
	u16 m_tvmd, m_old_tvmd;
	u8 m_disp, m_bdclmd, m_lsmd, m_vreso, m_hreso;
	bool m_odd_bit;
	u16 m_exten;
	bool m_exlten, m_exsyen, m_dasel, m_exbgen;

	u16 m_hcounter_latch, m_vcounter_latch;

	bool m_exltfg, m_exsyfg;

	// size = 313 for PAL
	u16 true_vcount[313][4];

	void reconfigure_crtc();

	int get_vblank();
	int get_hblank();
	int get_hcounter();
	int get_vcounter();
	int get_vblank_duration();
	int get_hblank_duration();
	int get_pixel_clock();

	bool m_vramsz;
};

DECLARE_DEVICE_TYPE(SATURN_VDP2, saturn_vdp2_device)


#endif // MAME_SEGA_SATURN_VDP2_H
