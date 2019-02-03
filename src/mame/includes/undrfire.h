// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, David Graves
#ifndef MAME_INCLUDES_UNDRFIRE_H
#define MAME_INCLUDES_UNDRFIRE_H

#pragma once

#include "machine/eepromser.h"
#include "video/tc0100scn.h"
#include "video/tc0480scp.h"
#include "emupal.h"

class undrfire_state : public driver_device
{
public:
	undrfire_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_tc0100scn(*this, "tc0100scn"),
		m_tc0480scp(*this, "tc0480scp"),
		m_eeprom(*this, "eeprom"),
		m_ram(*this, "ram"),
		m_shared_ram(*this, "shared_ram"),
		m_spriteram(*this, "spriteram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void undrfire(machine_config &config);
	void cbombers(machine_config &config);

	void init_undrfire();
	void init_cbombers();

protected:
	enum
	{
		TIMER_INTERRUPT5
	};

	virtual void video_start() override;

private:
	struct uf_tempsprite
	{
		int gfx;
		int code,color;
		int flipx,flipy;
		int x,y;
		int zoomx,zoomy;
		int primask;
	};

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_subcpu;
	required_device<tc0100scn_device> m_tc0100scn;
	required_device<tc0480scp_device> m_tc0480scp;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	optional_shared_ptr<uint32_t> m_ram;
	optional_shared_ptr<uint32_t> m_shared_ram;
	uint16_t m_port_sel;
	int m_frame_counter;
	std::unique_ptr<uf_tempsprite[]> m_spritelist;
	uint16_t m_rotate_ctrl[8];
	uint8_t m_dislayer[6];
	required_shared_ptr<uint32_t> m_spriteram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	DECLARE_WRITE8_MEMBER(coin_word_w);
	DECLARE_READ16_MEMBER(shared_ram_r);
	DECLARE_WRITE16_MEMBER(shared_ram_w);
	DECLARE_READ32_MEMBER(undrfire_lightgun_r);
	DECLARE_WRITE32_MEMBER(rotate_control_w);
	DECLARE_WRITE32_MEMBER(motor_control_w);
	DECLARE_WRITE32_MEMBER(cbombers_cpua_ctrl_w);
	DECLARE_READ_LINE_MEMBER(frame_counter_r);
	uint32_t screen_update_undrfire(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_cbombers(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(undrfire_interrupt);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect,const int *primasks,int x_offs,int y_offs);
	void draw_sprites_cbombers(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect,const int *primasks,int x_offs,int y_offs);

	void cbombers_cpua_map(address_map &map);
	void cbombers_cpub_map(address_map &map);
	void undrfire_map(address_map &map);
};

#endif // MAME_INCLUDES_UNDRFIRE_H
