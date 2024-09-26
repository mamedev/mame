// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, David Graves
#ifndef MAME_TAITO_UNDRFIRE_H
#define MAME_TAITO_UNDRFIRE_H

#pragma once

#include "machine/eepromser.h"
#include "tc0100scn.h"
#include "tc0360pri.h"
#include "tc0480scp.h"
#include "emupal.h"

class undrfire_state : public driver_device
{
public:
	undrfire_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_tc0620scc(*this, "tc0620scc"),
		m_tc0480scp(*this, "tc0480scp"),
		m_tc0360pri(*this, "tc0360pri"),
		m_eeprom(*this, "eeprom"),
		m_ram(*this, "ram"),
		m_shared_ram(*this, "shared_ram"),
		m_spriteram(*this, "spriteram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spritemap(*this, "spritemap"),
		m_spritemaphi(*this, "spritemaphi"),
		m_in_gunx(*this, "GUNX%u", 1U),
		m_in_guny(*this, "GUNY%u", 1U),
		m_io_fake(*this, "FAKE"),
		m_lamp_start(*this, "P%u_lamp_start", 1U),
		m_gun_recoil(*this, "P%u_gun_recoil", 1U),
		m_lamp(*this, "Lamp_%u", 1U),
		m_wheel_vibration(*this, "Wheel_vibration")
	{ }

	void undrfire(machine_config &config);
	void cbombers(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	struct uf_tempsprite
	{
		u8 gfx = 0;
		u32 code = 0, color = 0;
		bool flipx = 0, flipy = 0;
		int x = 0, y = 0;
		int zoomx = 0, zoomy = 0;
		u32 primask = 0;
	};

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_subcpu;
	required_device<tc0620scc_device> m_tc0620scc;
	required_device<tc0480scp_device> m_tc0480scp;
	optional_device<tc0360pri_device> m_tc0360pri; // cbombers
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	optional_shared_ptr<u32> m_ram;
	optional_shared_ptr<u16> m_shared_ram;
	u16 m_port_sel = 0;
	int m_frame_counter = 0;
	std::unique_ptr<uf_tempsprite[]> m_spritelist;
	u16 m_rotate_ctrl[8]{};
#ifdef MAME_DEBUG
	u8 m_dislayer[6] = { 0, 0, 0, 0, 0, 0 };
#endif
	required_shared_ptr<u32> m_spriteram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_region_ptr<u16> m_spritemap;
	optional_region_ptr<u8> m_spritemaphi;

	optional_ioport_array<2> m_in_gunx;
	optional_ioport_array<2> m_in_guny;
	optional_ioport m_io_fake;
	output_finder<2> m_lamp_start;
	output_finder<2> m_gun_recoil;
	output_finder<6> m_lamp;
	output_finder<> m_wheel_vibration;

	void coin_word_w(u8 data);
	u16 shared_ram_r(offs_t offset);
	void shared_ram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u32 undrfire_lightgun_r(offs_t offset);
	void rotate_control_w(offs_t offset, u16 data);
	void motor_control_w(u8 data);
	void cbombers_cpua_ctrl_w(u32 data);
	int frame_counter_r();
	u32 screen_update_undrfire(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_cbombers(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(undrfire_interrupt);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect,const u32 *primasks,int x_offs,int y_offs);
	void draw_sprites_cbombers(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect,const u8* pritable,int x_offs,int y_offs);

	void cbombers_cpua_map(address_map &map) ATTR_COLD;
	void cbombers_cpub_map(address_map &map) ATTR_COLD;
	void undrfire_map(address_map &map) ATTR_COLD;
};

#endif // MAME_TAITO_UNDRFIRE_H
