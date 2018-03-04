// license:BSD-3-Clause
// copyright-holders:Luca Elia
#ifndef MAME_INCLUDES_SETA2_H
#define MAME_INCLUDES_SETA2_H

#pragma once


#include "machine/tmp68301.h"
#include "machine/eepromser.h"
#include "machine/intelfsh.h"
#include "machine/ticket.h"
#include "machine/timer.h"
#include "machine/upd4992.h"
#include "sound/okim9810.h"
#include "sound/x1_010.h"
#include "screen.h"

class seta2_state : public driver_device
{
public:
	seta2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_sub(*this,"sub"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),

		m_tmp68301(*this, "tmp68301"),
		m_oki(*this, "oki"),
		m_eeprom(*this, "eeprom"),
		m_flash(*this, "flash"),
		m_rtc(*this, "rtc"),
		m_dispenser(*this, "dispenser"),

		m_nvram(*this, "nvram"),
		m_spriteram(*this, "spriteram", 0),
		m_tileram(*this, "tileram", 0),
		m_vregs(*this, "vregs", 0),
		m_funcube_outputs(*this, "funcube_outputs"),
		m_funcube_leds(*this, "funcube_leds")
	{ }

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_sub;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	optional_device<tmp68301_device> m_tmp68301;
	optional_device<okim9810_device> m_oki;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;
	optional_device<intelfsh16_device> m_flash;
	optional_device<upd4992_device> m_rtc;
	optional_device<ticket_dispenser_device> m_dispenser;

	optional_shared_ptr<uint16_t> m_nvram;
	optional_shared_ptr<uint16_t> m_spriteram;
	optional_shared_ptr<uint16_t> m_tileram;
	optional_shared_ptr<uint16_t> m_vregs;
	optional_shared_ptr<uint16_t> m_funcube_outputs;
	optional_shared_ptr<uint16_t> m_funcube_leds;

	int m_xoffset;
	int m_yoffset;
	int m_keyboard_row;
	std::unique_ptr<uint16_t[]> m_buffered_spriteram;

	uint64_t m_funcube_coin_start_cycles;
	uint8_t m_funcube_hopper_motor;

	DECLARE_WRITE16_MEMBER(spriteram16_word_w);
	DECLARE_READ16_MEMBER(spriteram16_word_r);
	DECLARE_WRITE16_MEMBER(vregs_w);
	DECLARE_READ32_MEMBER(oki_read);
	DECLARE_WRITE32_MEMBER(oki_write);
	DECLARE_WRITE16_MEMBER(sound_bank_w);

	DECLARE_WRITE16_MEMBER(grdians_lockout_w);

	DECLARE_READ16_MEMBER(mj4simai_p1_r);
	DECLARE_READ16_MEMBER(mj4simai_p2_r);
	DECLARE_WRITE16_MEMBER(mj4simai_keyboard_w);

	DECLARE_READ16_MEMBER(pzlbowl_protection_r);
	DECLARE_READ16_MEMBER(pzlbowl_coins_r);
	DECLARE_WRITE16_MEMBER(pzlbowl_coin_counter_w);

	DECLARE_WRITE16_MEMBER(reelquak_leds_w);
	DECLARE_WRITE16_MEMBER(reelquak_coin_w);

	DECLARE_WRITE16_MEMBER(samshoot_coin_w);

	DECLARE_WRITE16_MEMBER(telpacfl_lamp1_w);
	DECLARE_WRITE16_MEMBER(telpacfl_lamp2_w);
	DECLARE_WRITE16_MEMBER(telpacfl_lockout_w);

	DECLARE_READ16_MEMBER(gundamex_eeprom_r);
	DECLARE_WRITE16_MEMBER(gundamex_eeprom_w);

	DECLARE_READ32_MEMBER(funcube_nvram_dword_r);
	DECLARE_WRITE32_MEMBER(funcube_nvram_dword_w);
	DECLARE_READ32_MEMBER(funcube_debug_r);
	DECLARE_READ16_MEMBER(funcube_coins_r);
	DECLARE_WRITE16_MEMBER(funcube_leds_w);
	DECLARE_READ16_MEMBER(funcube_outputs_r);
	DECLARE_WRITE16_MEMBER(funcube_outputs_w);
	DECLARE_READ16_MEMBER(funcube_battery_r);

	DECLARE_DRIVER_INIT(funcube3);
	DECLARE_DRIVER_INIT(funcube);
	DECLARE_DRIVER_INIT(funcube2);

	DECLARE_MACHINE_START(mj4simai);
	DECLARE_MACHINE_START(funcube);
	DECLARE_MACHINE_RESET(funcube);

	virtual void video_start() override;
	DECLARE_VIDEO_START(yoffset);
	DECLARE_VIDEO_START(xoffset);
	DECLARE_VIDEO_START(xoffset1);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(seta2_interrupt);
	INTERRUPT_GEN_MEMBER(samshoot_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(funcube_interrupt);

	void funcube_debug_outputs();
	void seta2(machine_config &config);
	void funcube(machine_config &config);
	void funcube3(machine_config &config);
	void funcube2(machine_config &config);
	void grdians(machine_config &config);
	void myangel(machine_config &config);
	void mj4simai(machine_config &config);
	void penbros(machine_config &config);
	void pzlbowl(machine_config &config);
	void myangel2(machine_config &config);
	void reelquak(machine_config &config);
	void ablastb(machine_config &config);
	void gundamex(machine_config &config);
	void telpacfl(machine_config &config);
	void samshoot(machine_config &config);
	void namcostr(machine_config &config);
	void ablastb_map(address_map &map);
	void funcube2_map(address_map &map);
	void funcube2_sub_io(address_map &map);
	void funcube_map(address_map &map);
	void funcube_sub_io(address_map &map);
	void funcube_sub_map(address_map &map);
	void grdians_map(address_map &map);
	void gundamex_map(address_map &map);
	void mj4simai_map(address_map &map);
	void myangel2_map(address_map &map);
	void myangel_map(address_map &map);
	void namcostr_map(address_map &map);
	void penbros_base_map(address_map &map);
	void penbros_map(address_map &map);
	void pzlbowl_map(address_map &map);
	void reelquak_map(address_map &map);
	void samshoot_map(address_map &map);
	void telpacfl_map(address_map &map);
};


class staraudi_state : public seta2_state
{
public:
	staraudi_state(const machine_config &mconfig, device_type type, const char *tag) :
		seta2_state(mconfig, type, tag),
		m_rgbram(*this, "rgbram", 0)
	{
	}
	static constexpr feature_type unemulated_features() { return feature::CAMERA | feature::PRINTER; }

	DECLARE_WRITE16_MEMBER(staraudi_camera_w);
	DECLARE_WRITE16_MEMBER(staraudi_lamps1_w);
	DECLARE_WRITE16_MEMBER(staraudi_lamps2_w);
	DECLARE_READ16_MEMBER(staraudi_tileram_r);
	DECLARE_WRITE16_MEMBER(staraudi_tileram_w);

	uint32_t staraudi_screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void staraudi(machine_config &config);
	void staraudi_map(address_map &map);
protected:
	virtual void driver_start() override;

	void staraudi_debug_outputs();

private:
	void draw_rgbram(bitmap_ind16 &bitmap);

	required_shared_ptr<uint16_t> m_rgbram;

	uint16_t m_lamps1 = 0, m_lamps2 = 0, m_cam = 0;
};

#endif // MAME_INCLUDES_SETA2_H
