// license:BSD-3-Clause
// copyright-holders:David Graves
/*************************************************************************

    Operation Thunderbolt

*************************************************************************/
#ifndef MAME_TAITO_OTHUNDER_H
#define MAME_TAITO_OTHUNDER_H

#pragma once

#include "taitosnd.h"
#include "machine/eepromser.h"
#include "taitoio.h"
#include "sound/flt_vol.h"
#include "tc0100scn.h"
#include "tc0110pcr.h"
#include "emupal.h"


class othunder_state : public driver_device
{
public:
	othunder_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_spriteram(*this,"spriteram"),
		m_sprmap_rom(*this,"sprmap_rom"),
		m_z80bank(*this,"z80bank"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_eeprom(*this, "eeprom"),
		m_tc0220ioc(*this, "tc0220ioc"),
		m_tc0100scn(*this, "tc0100scn"),
		m_tc0110pcr(*this, "tc0110pcr"),
		m_tc0140syt(*this, "tc0140syt"),
		m_2610_0l(*this, "2610.0l"),
		m_2610_0r(*this, "2610.0r"),
		m_2610_1l(*this, "2610.1l"),
		m_2610_1r(*this, "2610.1r"),
		m_2610_2l(*this, "2610.2l"),
		m_2610_2r(*this, "2610.2r"),
		m_gfxdecode(*this, "gfxdecode"),
		m_recoil_piston(*this, "Player%u_Recoil_Piston", 1U)
	{ }

	void othunder(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, const u32 *primasks, int y_offs);

	void irq_ack_w(offs_t offset, u16 data);
	void eeprom_w(u8 data);
	void coins_w(u8 data);
	void adc_eoc_w(int state);
	void sound_bankswitch_w(u8 data);
	void tc0310fam_w(offs_t offset, u8 data);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_w(int state);

	void othunder_map(address_map &map) ATTR_COLD;
	void z80_sound_map(address_map &map) ATTR_COLD;

	/* memory pointers */
	required_shared_ptr<u16> m_spriteram;
	required_region_ptr<u16> m_sprmap_rom;
	required_memory_bank m_z80bank;

	/* video-related */
	struct tempsprite
	{
		u32 code = 0, color = 0;
		bool flipx = false, flipy = false;
		int x = 0, y = 0;
		int zoomx = 0, zoomy = 0;
		u32 primask = 0;
	};

	std::unique_ptr<tempsprite[]> m_spritelist;

	/* misc */
	int        m_pan[4]{};

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<tc0220ioc_device> m_tc0220ioc;
	required_device<tc0100scn_device> m_tc0100scn;
	required_device<tc0110pcr_device> m_tc0110pcr;
	required_device<tc0140syt_device> m_tc0140syt;
	required_device<filter_volume_device> m_2610_0l;
	required_device<filter_volume_device> m_2610_0r;
	required_device<filter_volume_device> m_2610_1l;
	required_device<filter_volume_device> m_2610_1r;
	required_device<filter_volume_device> m_2610_2l;
	required_device<filter_volume_device> m_2610_2r;
	required_device<gfxdecode_device> m_gfxdecode;

	output_finder<2> m_recoil_piston;
};

#endif // MAME_TAITO_OTHUNDER_H
