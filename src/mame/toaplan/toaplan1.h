// license:BSD-3-Clause
// copyright-holders:Darren Olafson, Quench
/***************************************************************************
                ToaPlan game hardware from 1988-1991
                ------------------------------------
****************************************************************************/
#ifndef MAME_TOAPLAN_TOAPLAN1_H
#define MAME_TOAPLAN_TOAPLAN1_H

#pragma once

#include "toaplan_bcu.h"
#include "toaplan_dsp.h"
#include "toaplan_fcu.h"
#include "toaplan_video_controller.h"

#include "cpu/m68000/m68000.h"
#include "sound/ymopl.h"

#include "emupal.h"
#include "screen.h"


class toaplan1_state : public driver_device
{
public:
	toaplan1_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_ymsnd(*this, "ymsnd"),
		m_fcu(*this, "fcu"),
		m_bcu(*this, "bcu"),
		m_vctrl(*this, "vctrl"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette_%u", 0U),
		m_paletteram(*this, "paletteram_%u", 0U, 0x800U, ENDIANNESS_BIG),
		m_sharedram(*this, "sharedram"),
		m_spriteram(*this, "spriteram", 0x800, ENDIANNESS_BIG),
		m_spritesizeram(*this, "spritesizeram", 0x80, ENDIANNESS_BIG),
		m_dswb_io(*this, "DSWB"),
		m_tjump_io(*this, "TJUMP")
	{ }

	void truxton(machine_config &config) ATTR_COLD;
	void outzone(machine_config &config) ATTR_COLD;
	void vimana(machine_config &config) ATTR_COLD;
	void outzonecv(machine_config &config) ATTR_COLD;
	void hellfire(machine_config &config) ATTR_COLD;
	void zerowing(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD {}
	virtual void machine_reset() override ATTR_COLD;

	required_device<m68000_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<ym3812_device> m_ymsnd;
	required_device<toaplan_fcu_device> m_fcu;
	required_device<toaplan_bcu_device> m_bcu;
	required_device<toaplan_video_controller_device> m_vctrl;
	required_device<screen_device> m_screen;
	required_device_array<palette_device, 2> m_palette;

	memory_share_array_creator<u16, 2> m_paletteram;

	required_shared_ptr<u8> m_sharedram;
	memory_share_creator<u16> m_spriteram;
	memory_share_creator<u16> m_spritesizeram;

	optional_ioport m_dswb_io;
	optional_ioport m_tjump_io;

	u8 shared_r(offs_t offset);
	void shared_w(offs_t offset, u8 data);
	void reset_sound_w(u8 data);
	void coin_w(u8 data);

	u8 vimana_dswb_invert_r();
	u8 vimana_tjump_invert_r();

	void pri_cb(u8 priority, u32 &pri_mask);
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);

	void log_vram();
	virtual void reset_sound();
	void reset_callback(int state);

	void common_video_config(machine_config &config) ATTR_COLD;

	void tile_offset_fcu_flip_map(address_map &map) ATTR_COLD;

	void hellfire_main_map(address_map &map) ATTR_COLD;
	void hellfire_sound_io_map(address_map &map) ATTR_COLD;
	void outzone_main_map(address_map &map) ATTR_COLD;
	void outzone_sound_io_map(address_map &map) ATTR_COLD;
	void outzonecv_main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void truxton_main_map(address_map &map) ATTR_COLD;
	void truxton_sound_io_map(address_map &map) ATTR_COLD;
	void vimana_hd647180_io_map(address_map &map) ATTR_COLD;
	void vimana_hd647180_mem_map(address_map &map) ATTR_COLD;
	void vimana_main_map(address_map &map) ATTR_COLD;
	void zerowing_main_map(address_map &map) ATTR_COLD;
	void zerowing_sound_io_map(address_map &map) ATTR_COLD;
};

class toaplan1_demonwld_state : public toaplan1_state
{
public:
	toaplan1_demonwld_state(const machine_config &mconfig, device_type type, const char *tag) :
		toaplan1_state(mconfig, type, tag),
		m_dsp(*this, "dsp")
	{
	}

	void demonwld(machine_config &config) ATTR_COLD;

private:
	required_device<toaplan_dsp_device> m_dsp;

	void dsp_ctrl_w(u8 data);

	void dsp_host_addr_cb(u16 data, u32 &seg, u32 &addr);
	u16 dsp_host_read_cb(u32 seg, u32 addr);
	bool dsp_host_write_cb(u32 seg, u32 addr, u16 data);

	void main_map(address_map &map) ATTR_COLD;
	void sound_io_map(address_map &map) ATTR_COLD;
};

#endif // MAME_TOAPLAN_TOAPLAN1_H
