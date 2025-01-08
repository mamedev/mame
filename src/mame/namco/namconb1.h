// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/***************************************************************************

    Namco System NB-1 hardware

***************************************************************************/
#ifndef MAME_NAMCO_NAMCONB1_H
#define MAME_NAMCO_NAMCONB1_H

#pragma once

#include "machine/eeprompar.h"
#include "namcomcu.h"
#include "machine/timer.h"
#include "screen.h"
#include "namco_c116.h"
#include "namco_c355spr.h"
#include "namco_c123tmap.h"
#include "namco_c169roz.h"


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
		m_screen(*this, "screen"),
		m_mcu(*this, "mcu"),
		m_eeprom(*this, "eeprom"),
		m_p1(*this, "P1"),
		m_p2(*this, "P2"),
		m_p3(*this, "P3"),
		m_p4(*this, "P4"),
		m_misc(*this, "MISC"),
		m_spritebank32(*this, "spritebank32"),
		m_namconb_shareram(*this, "namconb_share"),
		m_update_to_line_before_posirq(false)
	{ }

	void namconb1(machine_config &config);

	void init_sws95();
	void init_sws97();
	void init_sws96();
	void init_vshoot();
	void init_nebulray();
	void init_gslgr94j();
	void init_gslgr94u();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	int m_gametype = 0;
	enum
	{
		/* Namco NB1 */
		NAMCONB1_NEBULRAY = 1,
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
	required_device<screen_device> m_screen;
	required_device<m37710_cpu_device> m_mcu;
	required_device<eeprom_parallel_28xx_device> m_eeprom;
	required_ioport m_p1;
	required_ioport m_p2;
	optional_ioport m_p3;
	optional_ioport m_p4;
	required_ioport m_misc;
	required_shared_ptr<u32> m_spritebank32;
	required_shared_ptr<u16> m_namconb_shareram;

	u8 m_vbl_irq_level = 0;
	u8 m_pos_irq_level = 0;
	u8 m_unk_irq_level = 0;
	u16 m_count = 0;
	u8 m_port6 = 0;
	std::unique_ptr<u32[]> m_spritebank32_delayed;
	bool m_update_to_line_before_posirq;

	u32 randgen_r();
	void srand_w(u32 data);
	void namconb1_cpureg_w(offs_t offset, u8 data);
	u8 namconb1_cpureg_r(offs_t offset);
	u32 custom_key_r(offs_t offset);
	u32 share_r(offs_t offset);
	void share_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void mcu_shared_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u8 port6_r();
	void port6_w(u8 data);
	u8 port7_r();
	template <int Bit> u16 dac_bit_r();

	u32 screen_update_namconb1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);

	TIMER_DEVICE_CALLBACK_MEMBER(scantimer);
	TIMER_DEVICE_CALLBACK_MEMBER(mcu_irq0_cb);
	TIMER_DEVICE_CALLBACK_MEMBER(mcu_irq2_cb);

	int NB1objcode2tile(int code);
	void NB1TilemapCB(u16 code, int &tile, int &mask);
	void namcoc75_am(address_map &map) ATTR_COLD;
	void namconb1_am(address_map &map) ATTR_COLD;
};

class gunbulet_state : public namconb1_state
{
public:
	gunbulet_state(const machine_config &mconfig, device_type type, const char *tag) :
		namconb1_state(mconfig, type, tag),
		m_light_x(*this, "LIGHT%u_X", 0U),
		m_light_y(*this, "LIGHT%u_Y", 0U)
	{ }

	void gunbulet(machine_config &config);

	void init_gunbulet();

private:
	required_ioport_array<2> m_light_x;
	required_ioport_array<2> m_light_y;

	u32 gun_r(offs_t offset);

	void gunbulet_am(address_map &map) ATTR_COLD;
};

class namconb2_state : public namconb1_state
{
public:
	namconb2_state(const machine_config &mconfig, device_type type, const char *tag) :
		namconb1_state(mconfig, type, tag),
		m_c169roz(*this, "c169roz"),
		m_tilebank32(*this, "tilebank32"),
		m_rozbank32(*this, "rozbank32")
	{ }

	void namconb2(machine_config &config);
	void outfxies(machine_config &config);
	void machbrkr(machine_config &config);

	void init_machbrkr();
	void init_outfxies();

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<namco_c169roz_device> m_c169roz; // NB2 only, not NB1
	required_shared_ptr<u32> m_tilebank32;
	required_shared_ptr<u32> m_rozbank32;

	u32 m_tilemap_tile_bank[4]{};

	void namconb2_cpureg_w(offs_t offset, u8 data);
	u8 namconb2_cpureg_r(offs_t offset);

	void rozbank32_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 screen_update_namconb2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	int NB2objcode2tile_machbrkr(int code);
	int NB2objcode2tile_outfxies(int code);
	void NB2TilemapCB_machbrkr(u16 code, int &tile, int &mask);
	void NB2TilemapCB_outfxies(u16 code, int &tile, int &mask);
	void NB2RozCB_machbrkr(u16 code, int &tile, int &mask, int which);
	void NB2RozCB_outfxies(u16 code, int &tile, int &mask, int which);
	void namconb2_am(address_map &map) ATTR_COLD;
};

#endif // MAME_NAMCO_NAMCONB1_H
