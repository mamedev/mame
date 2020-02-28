// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/***************************************************************************

    Namco System NB-1 hardware

***************************************************************************/
#ifndef MAME_INCLUDES_NAMCONB1_H
#define MAME_INCLUDES_NAMCONB1_H

#pragma once

#include "machine/eeprompar.h"
#include "machine/namcomcu.h"
#include "machine/timer.h"
#include "screen.h"
#include "video/namco_c116.h"
#include "video/namco_c355spr.h"
#include "video/namco_c123tmap.h"
#include "video/namco_c169roz.h"


class namconb1_state : public driver_device
{
public:
	namconb1_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_gametype(0),
		m_maincpu(*this, "maincpu"),
		m_c116(*this, "c116"),
		m_c123tmap(*this, "c123tmap"),
		m_c355spr(*this, "c355spr"),
		m_c169roz(*this, "c169roz"),
		m_screen(*this, "screen"),
		m_mcu(*this, "mcu"),
		m_eeprom(*this, "eeprom"),
		m_p1(*this, "P1"),
		m_p2(*this, "P2"),
		m_p3(*this, "P3"),
		m_p4(*this, "P4"),
		m_misc(*this, "MISC"),
		m_light0_x(*this, "LIGHT0_X"),
		m_light0_y(*this, "LIGHT0_Y"),
		m_light1_x(*this, "LIGHT1_X"),
		m_light1_y(*this, "LIGHT1_Y"),
		m_spritebank32(*this, "spritebank32"),
		m_tilebank32(*this, "tilebank32"),
		m_rozbank32(*this, "rozbank32"),
		m_namconb_shareram(*this, "namconb_share")
	{ }

	void namconb1(machine_config &config);
	void namconb2(machine_config &config);
	void outfxies(machine_config &config);
	void machbrkr(machine_config &config);

	void init_sws95();
	void init_machbrkr();
	void init_sws97();
	void init_sws96();
	void init_vshoot();
	void init_nebulray();
	void init_gunbulet();
	void init_gslgr94j();
	void init_outfxies();
	void init_gslgr94u();

private:
	int m_gametype;
	enum
	{
		/* Namco NB1 */
		NAMCONB1_NEBULRAY = 0x2000,
		NAMCONB1_GUNBULET,
		NAMCONB1_GSLGR94U,
		NAMCONB1_GSLGR94J,
		NAMCONB1_SWS95,
		NAMCONB1_SWS96,
		NAMCONB1_SWS97,
		NAMCONB1_VSHOOT,

		/* Namco NB2 */
		NAMCONB2_OUTFOXIES,
		NAMCONB2_MACH_BREAKERS,
	};

	required_device<cpu_device> m_maincpu;
	required_device<namco_c116_device> m_c116;
	required_device<namco_c123tmap_device> m_c123tmap;
	required_device<namco_c355spr_device> m_c355spr;
	optional_device<namco_c169roz_device> m_c169roz; // NB1 only, not NA1
	required_device<screen_device> m_screen;
	required_device<m37710_cpu_device> m_mcu;
	required_device<eeprom_parallel_28xx_device> m_eeprom;
	required_ioport m_p1;
	required_ioport m_p2;
	optional_ioport m_p3;
	optional_ioport m_p4;
	required_ioport m_misc;
	optional_ioport m_light0_x;
	optional_ioport m_light0_y;
	optional_ioport m_light1_x;
	optional_ioport m_light1_y;
	required_shared_ptr<uint32_t> m_spritebank32;
	optional_shared_ptr<uint32_t> m_tilebank32;
	optional_shared_ptr<uint32_t> m_rozbank32;
	required_shared_ptr<uint16_t> m_namconb_shareram;

	uint8_t m_vbl_irq_level;
	uint8_t m_pos_irq_level;
	uint8_t m_unk_irq_level;
	uint16_t m_count;
	uint8_t m_port6;
	uint32_t m_tilemap_tile_bank[4];

	DECLARE_READ32_MEMBER(randgen_r);
	DECLARE_WRITE32_MEMBER(srand_w);
	DECLARE_WRITE8_MEMBER(namconb1_cpureg_w);
	DECLARE_WRITE8_MEMBER(namconb2_cpureg_w);
	DECLARE_READ8_MEMBER(namconb1_cpureg_r);
	DECLARE_READ8_MEMBER(namconb2_cpureg_r);
	DECLARE_READ32_MEMBER(custom_key_r);
	DECLARE_READ32_MEMBER(gunbulet_gun_r);
	DECLARE_READ32_MEMBER(share_r);
	DECLARE_WRITE32_MEMBER(share_w);
	DECLARE_WRITE16_MEMBER(mcu_shared_w);
	DECLARE_READ8_MEMBER(port6_r);
	DECLARE_WRITE8_MEMBER(port6_w);
	DECLARE_READ8_MEMBER(port7_r);
	template <int Bit> uint16_t dac_bit_r();

	DECLARE_WRITE32_MEMBER(rozbank32_w);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	DECLARE_VIDEO_START(namconb1);
	DECLARE_VIDEO_START(machbrkr);
	DECLARE_VIDEO_START(outfxies);
	void video_update_common(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int bROZ);
	uint32_t screen_update_namconb1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_namconb2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	TIMER_DEVICE_CALLBACK_MEMBER(scantimer);
	TIMER_DEVICE_CALLBACK_MEMBER(mcu_irq0_cb);
	TIMER_DEVICE_CALLBACK_MEMBER(mcu_irq2_cb);

	int NB1objcode2tile(int code);
	int NB2objcode2tile_machbrkr(int code);
	int NB2objcode2tile_outfxies(int code);
	void NB1TilemapCB(uint16_t code, int *tile, int *mask);
	void NB2TilemapCB_machbrkr(uint16_t code, int *tile, int *mask);
	void NB2TilemapCB_outfxies(uint16_t code, int *tile, int *mask);
	void NB2RozCB_machbrkr(uint16_t code, int *tile, int *mask, int which);
	void NB2RozCB_outfxies(uint16_t code, int *tile, int *mask, int which);
	void namcoc75_am(address_map &map);
	void namconb1_am(address_map &map);
	void namconb2_am(address_map &map);
};

#endif // MAME_INCLUDES_NAMCONB1_H
