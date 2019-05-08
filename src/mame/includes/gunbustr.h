// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, David Graves
#ifndef MAME_INCLUDES_GUNBUSTR_H
#define MAME_INCLUDES_GUNBUSTR_H

#pragma once

#include "machine/eepromser.h"
#include "video/tc0480scp.h"
#include "emupal.h"

struct gb_tempsprite
{
	int gfx;
	int code,color;
	int flipx,flipy;
	int x,y;
	int zoomx,zoomy;
	int primask;
};

class gunbustr_state : public driver_device
{
public:
	gunbustr_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_tc0480scp(*this, "tc0480scp"),
		m_ram(*this,"ram"),
		m_spriteram(*this,"spriteram"),
		m_eeprom(*this, "eeprom"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spritemap(*this, "spritemap"),
		m_io_light_x(*this, "LIGHT%u_X", 0U),
		m_io_light_y(*this, "LIGHT%u_Y", 0U)
	{
		m_coin_lockout = true;
	}

	void gunbustr(machine_config &config);

	void init_gunbustrj();
	void init_gunbustr();

private:
	enum
	{
		TIMER_GUNBUSTR_INTERRUPT5
	};

	required_device<cpu_device> m_maincpu;
	required_device<tc0480scp_device> m_tc0480scp;
	required_shared_ptr<u32> m_ram;
	required_shared_ptr<u32> m_spriteram;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_region_ptr<u16> m_spritemap;

	required_ioport_array<2> m_io_light_x;
	required_ioport_array<2> m_io_light_y;

	bool m_coin_lockout;
	std::unique_ptr<gb_tempsprite[]> m_spritelist;
	emu_timer *m_interrupt5_timer;

	void motor_control_w(u32 data);
	DECLARE_READ32_MEMBER(gun_r);
	void gun_w(u32 data);
	DECLARE_READ32_MEMBER(main_cycle_r);
	void coin_word_w(u8 data);
	virtual void video_start() override;
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(gunbustr_interrupt);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect,const u32 *primasks,int x_offs,int y_offs);

	void gunbustr_map(address_map &map);

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};

#endif // MAME_INCLUDES_GUNBUSTR_H
