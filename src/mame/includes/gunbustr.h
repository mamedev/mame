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
		m_palette(*this, "palette")
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
	required_shared_ptr<uint32_t> m_ram;
	required_shared_ptr<uint32_t> m_spriteram;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	bool m_coin_lockout;
	std::unique_ptr<gb_tempsprite[]> m_spritelist;
	uint32_t m_mem[2];
	emu_timer *m_interrupt5_timer;

	DECLARE_WRITE32_MEMBER(motor_control_w);
	DECLARE_READ32_MEMBER(gunbustr_gun_r);
	DECLARE_WRITE32_MEMBER(gunbustr_gun_w);
	DECLARE_READ32_MEMBER(main_cycle_r);
	DECLARE_WRITE8_MEMBER(coin_word_w);
	virtual void video_start() override;
	uint32_t screen_update_gunbustr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(gunbustr_interrupt);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect,const int *primasks,int x_offs,int y_offs);

	void gunbustr_map(address_map &map);

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};

#endif // MAME_INCLUDES_GUNBUSTR_H
