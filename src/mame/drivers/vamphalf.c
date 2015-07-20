// license:BSD-3-Clause
// copyright-holders:Angelo Salese, David Haywood, Pierpaolo Prazzoli, Tomasz Slanina
/********************************************************************

 Driver for common Hyperstone based games

 by bits from Angelo Salese, David Haywood,
    Pierpaolo Prazzoli and Tomasz Slanina

 Games Supported:


    Minigame Cool Collection        (c) 1999 SemiCom
    Jumping Break                   (c) 1999 F2 System
    Lup Lup Puzzle                  (c) 1999 Omega System       (version 3.0 and 2.9)
    Puzzle Bang Bang                (c) 1999 Omega System       (version 2.8 and 2.9)
    Super Lup Lup Puzzle            (c) 1999 Omega System       (version 4.0)
    Vamf 1/2                        (c) 1999 Danbi & F2 System  (Europe version)
    Vamp 1/2                        (c) 1999 Danbi & F2 System  (Korea version)
    Date Quiz Go Go Episode 2       (c) 2000 SemiCom
    Mission Craft                   (c) 2000 Sun                (version 2.7 and 2.4)
    Mr. Dig                         (c) 2000 Sun
    Diet Family                     (c) 2001 SemiCom
    Final Godori                    (c) 2001 SemiCom            (version 2.20.5915)
    Wivern Wings / Wyvern Wings     (c) 2001 SemiCom
    Mr. Kicker                      (c) 2001 SemiCom
    Toy Land Adventure              (c) 2001 SemiCom
    Age Of Heroes - Silkroad 2      (c) 2001 Unico              (v0.63 - 2001/02/07)
    Boong-Ga Boong-Ga (Spank 'em)   (c) 2001 Taff System

 Real games bugs:
 - dquizgo2: bugged video test

 Notes:

 Mr Kicker: The F-E1-16-010 PCB version still needs proper speed ups

 Mr Kicker: Doesn't boot without a valid default eeprom, but no longer seems to fail
            after you get a high score (since eeprom rewrite).

 Boong-Ga Boong-Ga: the test mode is usable with a standard input configuration like the "common" one

 The Semicom boards (at least) have a strange visible area, with the display output cutting off 4 lines
 before you'd expect.  It has been confirmed on real hardware that these lines are simply never output,
 no amount of stretching the screen renders them as visible.  The games are also programmed around this
 assumption in many places, with visible sprite clipping issues at screen edges otherwise.  This does
 result in some graphics also being cut off slightly, but that is correct.

 Undumped Semicom games on similar hardware:
   Red Wyvern - A semi-sequel or update?
 Same time era, but unknown hardware:
   Choice III: Joker's Dream (c) 2001

TODO:
- boonggab: simulate photo sensors with a "stroke strength"
- boonggab: what are sensors bit used for? are they used in the japanese version?
- wyvernsg: fails a protection check after ~1 hour of play?

*********************************************************************/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "cpu/e132xs/e132xs.h"
#include "machine/eepromser.h"
#include "machine/nvram.h"
#include "sound/qs1000.h"
#include "sound/2151intf.h"
#include "sound/okim6295.h"


class vamphalf_state : public driver_device
{
public:
	vamphalf_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_qs1000(*this, "qs1000"),
			m_oki(*this, "oki"),
			m_oki2(*this, "oki_2"),
			m_eeprom(*this, "eeprom"),
			m_gfxdecode(*this, "gfxdecode"),
			m_palette(*this, "palette"),
			m_tiles(*this,"tiles"),
			m_wram(*this,"wram"),
			m_tiles32(*this,"tiles32"),
			m_wram32(*this,"wram32") {
			m_has_extra_gfx = 0;
		}

	required_device<cpu_device> m_maincpu;
	optional_device<qs1000_device> m_qs1000;
	optional_device<okim6295_device> m_oki;
	optional_device<okim6295_device> m_oki2;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	optional_shared_ptr<UINT16> m_tiles;
	optional_shared_ptr<UINT16> m_wram;
	optional_shared_ptr<UINT32> m_tiles32;
	optional_shared_ptr<UINT32> m_wram32;

	// driver init configuration
	int m_flip_bit;
	int m_palshift;
	int m_has_extra_gfx;
	UINT16 m_semicom_prot_data[2];

	int m_flipscreen;
	int m_semicom_prot_idx;
	int m_semicom_prot_which;
	UINT16 m_finalgdr_backupram_bank;
	UINT8 *m_finalgdr_backupram;
	UINT8 m_qs1000_data;

	DECLARE_WRITE16_MEMBER(flipscreen_w);
	DECLARE_WRITE32_MEMBER(flipscreen32_w);
	DECLARE_WRITE16_MEMBER(jmpbreak_flipscreen_w);
	DECLARE_READ32_MEMBER(wyvernwg_prot_r);
	DECLARE_WRITE32_MEMBER(wyvernwg_prot_w);
	DECLARE_READ32_MEMBER(finalgdr_prot_r);
	DECLARE_WRITE32_MEMBER(finalgdr_prot_w);
	DECLARE_WRITE32_MEMBER(finalgdr_prize_w);
	DECLARE_WRITE16_MEMBER(boonggab_prize_w);
	DECLARE_WRITE16_MEMBER(boonggab_lamps_w);
	DECLARE_CUSTOM_INPUT_MEMBER(boonggab_photo_sensors_r);

	DECLARE_READ16_MEMBER(vamphalf_speedup_r);
	DECLARE_READ16_MEMBER(vamphafk_speedup_r);
	DECLARE_READ16_MEMBER(misncrft_speedup_r);
	DECLARE_READ16_MEMBER(misncrfta_speedup_r);
	DECLARE_READ16_MEMBER(coolmini_speedup_r);
	DECLARE_READ16_MEMBER(suplup_speedup_r);
	DECLARE_READ16_MEMBER(luplup_speedup_r);
	DECLARE_READ16_MEMBER(luplup29_speedup_r);
	DECLARE_READ16_MEMBER(puzlbang_speedup_r);
	DECLARE_READ16_MEMBER(puzlbanga_speedup_r);
	DECLARE_READ32_MEMBER(wivernwg_speedup_r);
	DECLARE_READ32_MEMBER(wyvernwg_speedup_r);
	DECLARE_READ32_MEMBER(wyvernwga_speedup_r);
	DECLARE_READ32_MEMBER(finalgdr_speedup_r);
	DECLARE_READ32_MEMBER(mrkickera_speedup_r);
	DECLARE_READ16_MEMBER(mrkicker_speedup_r);
	DECLARE_READ16_MEMBER(dquizgo2_speedup_r);
	DECLARE_READ32_MEMBER(aoh_speedup_r);
	DECLARE_READ16_MEMBER(jmpbreak_speedup_r);
	DECLARE_READ16_MEMBER(mrdig_speedup_r);
	DECLARE_READ16_MEMBER(dtfamily_speedup_r);
	DECLARE_READ16_MEMBER(toyland_speedup_r);
	DECLARE_READ16_MEMBER(boonggab_speedup_r);

	DECLARE_READ16_MEMBER(eeprom_r);
	DECLARE_READ32_MEMBER(eeprom32_r);
	DECLARE_WRITE16_MEMBER(eeprom_w);
	DECLARE_WRITE32_MEMBER(eeprom32_w);
	DECLARE_WRITE32_MEMBER(finalgdr_eeprom_w);
	DECLARE_WRITE32_MEMBER(finalgdr_backupram_bank_w);
	DECLARE_READ32_MEMBER(finalgdr_backupram_r);
	DECLARE_WRITE32_MEMBER(finalgdr_backupram_w);

	DECLARE_WRITE32_MEMBER(finalgdr_oki_bank_w);
	DECLARE_WRITE32_MEMBER(aoh_oki_bank_w);
	DECLARE_WRITE16_MEMBER(boonggab_oki_bank_w);
	DECLARE_WRITE16_MEMBER(mrkicker_oki_bank_w);
	DECLARE_WRITE32_MEMBER(wyvernwg_snd_w);
	DECLARE_WRITE16_MEMBER(misncrft_snd_w);
	DECLARE_READ8_MEMBER(qs1000_p1_r);
	DECLARE_WRITE8_MEMBER(qs1000_p3_w);

	virtual void video_start();
	DECLARE_DRIVER_INIT(vamphalf);
	DECLARE_DRIVER_INIT(vamphafk);
	DECLARE_DRIVER_INIT(coolmini);
	DECLARE_DRIVER_INIT(mrkickera);
	DECLARE_DRIVER_INIT(mrdig);
	DECLARE_DRIVER_INIT(jmpbreak);
	DECLARE_DRIVER_INIT(dtfamily);
	DECLARE_DRIVER_INIT(dquizgo2);
	DECLARE_DRIVER_INIT(suplup);
	DECLARE_DRIVER_INIT(luplup);
	DECLARE_DRIVER_INIT(luplup29);
	DECLARE_DRIVER_INIT(puzlbang);
	DECLARE_DRIVER_INIT(toyland);
	DECLARE_DRIVER_INIT(aoh);
	DECLARE_DRIVER_INIT(finalgdr);
	DECLARE_DRIVER_INIT(misncrft);
	DECLARE_DRIVER_INIT(boonggab);
	DECLARE_DRIVER_INIT(wyvernwg);
	DECLARE_DRIVER_INIT(yorijori);
	DECLARE_DRIVER_INIT(mrkicker);

	UINT32 screen_update_common(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_aoh(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap);
	void draw_sprites_aoh(screen_device &screen, bitmap_ind16 &bitmap);
	void handle_flipped_visible_area(screen_device &screen);
};

READ16_MEMBER(vamphalf_state::eeprom_r)
{
	if(offset)
	{
		return m_eeprom->do_read();
	}
	else
		return 0;
}

READ32_MEMBER(vamphalf_state::eeprom32_r)
{
	return m_eeprom->do_read();
}

WRITE16_MEMBER(vamphalf_state::eeprom_w)
{
	if(offset)
	{
		m_eeprom->di_write(data & 0x01);
		m_eeprom->cs_write((data & 0x04) ? ASSERT_LINE : CLEAR_LINE );
		m_eeprom->clk_write((data & 0x02) ? ASSERT_LINE : CLEAR_LINE );

		// data & 8?
	}
}

WRITE32_MEMBER(vamphalf_state::eeprom32_w)
{
	m_eeprom->di_write(data & 0x01);
	m_eeprom->cs_write((data & 0x04) ? ASSERT_LINE : CLEAR_LINE );
	m_eeprom->clk_write((data & 0x02) ? ASSERT_LINE : CLEAR_LINE );
}

WRITE32_MEMBER(vamphalf_state::finalgdr_eeprom_w)
{
	m_eeprom->di_write((data & 0x4000) >> 14);
	m_eeprom->cs_write((data & 0x1000) ? ASSERT_LINE : CLEAR_LINE );
	m_eeprom->clk_write((data & 0x2000) ? ASSERT_LINE : CLEAR_LINE );
}

WRITE16_MEMBER(vamphalf_state::flipscreen_w)
{
	if(offset)
	{
		m_flipscreen = data & m_flip_bit;
	}
}

WRITE32_MEMBER(vamphalf_state::flipscreen32_w)
{
	m_flipscreen = data & m_flip_bit;
}

WRITE16_MEMBER(vamphalf_state::jmpbreak_flipscreen_w)
{
	m_flipscreen = data & 0x8000;
}



READ32_MEMBER(vamphalf_state::wyvernwg_prot_r)
{
	m_semicom_prot_idx--;
	return (m_semicom_prot_data[m_semicom_prot_which] & (1 << m_semicom_prot_idx)) >> m_semicom_prot_idx;
}

WRITE32_MEMBER(vamphalf_state::wyvernwg_prot_w)
{
	m_semicom_prot_which = data & 1;
	m_semicom_prot_idx = 8;
}

READ32_MEMBER(vamphalf_state::finalgdr_prot_r)
{
	m_semicom_prot_idx--;
	return (m_semicom_prot_data[m_semicom_prot_which] & (1 << m_semicom_prot_idx)) ? 0x8000 : 0;
}

WRITE32_MEMBER(vamphalf_state::finalgdr_prot_w)
{
/*
41C6
967E
446B
F94B
*/
	if(data == 0x41c6 || data == 0x446b)
		m_semicom_prot_which = 0;
	else
		m_semicom_prot_which =  1;

	m_semicom_prot_idx = 8;
}

WRITE32_MEMBER(vamphalf_state::finalgdr_oki_bank_w)
{
	m_oki->set_bank_base(0x40000 * ((data & 0x300) >> 8));
}

WRITE32_MEMBER(vamphalf_state::finalgdr_backupram_bank_w)
{
	m_finalgdr_backupram_bank = (data & 0xff000000) >> 24;
}

READ32_MEMBER(vamphalf_state::finalgdr_backupram_r)
{
	return m_finalgdr_backupram[offset + m_finalgdr_backupram_bank * 0x80] << 24;
}

WRITE32_MEMBER(vamphalf_state::finalgdr_backupram_w)
{
	m_finalgdr_backupram[offset + m_finalgdr_backupram_bank * 0x80] = data >> 24;
}

WRITE32_MEMBER(vamphalf_state::finalgdr_prize_w)
{
	if(data & 0x1000000)
	{
		// prize 1
	}

	if(data & 0x2000000)
	{
		// prize 2
	}

	if(data & 0x4000000)
	{
		// prize 3
	}
}

WRITE32_MEMBER(vamphalf_state::aoh_oki_bank_w)
{
	m_oki2->set_bank_base(0x40000 * (data & 0x3));
}

WRITE16_MEMBER(vamphalf_state::boonggab_oki_bank_w)
{
	if(offset)
		m_oki->set_bank_base(0x40000 * (data & 0x7));
}


WRITE16_MEMBER(vamphalf_state::mrkicker_oki_bank_w)
{
	m_oki->set_bank_base(0x40000 * (data & 0x3));
}

WRITE16_MEMBER(vamphalf_state::boonggab_prize_w)
{
	if(offset)
	{
		// data & 0x01 == motor 1 on
		// data & 0x02 == motor 2 on
		// data & 0x04 == motor 3 on
		// data & 0x08 == prize power 1 on
		// data & 0x10 == prize lamp 1 off
		// data & 0x20 == prize lamp 2 off
		// data & 0x40 == prize lamp 3 off
	}
}

WRITE16_MEMBER(vamphalf_state::boonggab_lamps_w)
{
	if(offset == 1)
	{
		// data & 0x0001 == lamp  7 on (why is data & 0x8000 set too?)
		// data & 0x0002 == lamp  8 on
		// data & 0x0004 == lamp  9 on
		// data & 0x0008 == lamp 10 on
		// data & 0x0010 == lamp 11 on
		// data & 0x0020 == lamp 12 on
		// data & 0x0040 == lamp 13 on
	}
	else if(offset == 3)
	{
		// data & 0x0100 == lamp  0 on
		// data & 0x0200 == lamp  1 on
		// data & 0x0400 == lamp  2 on
		// data & 0x0800 == lamp  3 on
		// data & 0x1000 == lamp  4 on
		// data & 0x2000 == lamp  5 on
		// data & 0x4000 == lamp  6 on
	}
}



WRITE32_MEMBER( vamphalf_state::wyvernwg_snd_w )
{
	m_qs1000_data = data & 0xff;
	m_qs1000->set_irq(ASSERT_LINE);
	machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(100));
}

WRITE16_MEMBER( vamphalf_state::misncrft_snd_w )
{
	m_qs1000_data = data & 0xff;
	m_qs1000->set_irq(ASSERT_LINE);
	machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(100));
}

READ8_MEMBER( vamphalf_state::qs1000_p1_r )
{
	return m_qs1000_data;
}

WRITE8_MEMBER( vamphalf_state::qs1000_p3_w )
{
	if (!BIT(data, 5))
		m_qs1000->set_irq(CLEAR_LINE);

	membank("qs1000:data")->set_entry(data & 7);
}


static ADDRESS_MAP_START( common_map, AS_PROGRAM, 16, vamphalf_state )
	AM_RANGE(0x00000000, 0x001fffff) AM_RAM AM_SHARE("wram")
	AM_RANGE(0x40000000, 0x4003ffff) AM_RAM AM_SHARE("tiles")
	AM_RANGE(0x80000000, 0x8000ffff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xfff00000, 0xffffffff) AM_ROM AM_REGION("user1",0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( common_32bit_map, AS_PROGRAM, 32, vamphalf_state )
	AM_RANGE(0x00000000, 0x001fffff) AM_RAM AM_SHARE("wram32")
	AM_RANGE(0x40000000, 0x4003ffff) AM_RAM AM_SHARE("tiles32")
	AM_RANGE(0x80000000, 0x8000ffff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xfff00000, 0xffffffff) AM_ROM AM_REGION("user1",0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( yorijori_32bit_map, AS_PROGRAM, 32, vamphalf_state )
	AM_RANGE(0x00000000, 0x001fffff) AM_RAM AM_SHARE("wram32")
	AM_RANGE(0x40000000, 0x4003ffff) AM_RAM AM_SHARE("tiles32")
	AM_RANGE(0x80000000, 0x8000ffff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xffe00000, 0xffffffff) AM_ROM AM_REGION("user1",0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( vamphalf_io, AS_IO, 16, vamphalf_state )
	AM_RANGE(0x0c0, 0x0c1) AM_NOP // return 0, when oki chip is read / written
	AM_RANGE(0x0c2, 0x0c3) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x140, 0x143) AM_DEVWRITE8("ymsnd", ym2151_device, register_w, 0x00ff)
	AM_RANGE(0x146, 0x147) AM_DEVREADWRITE8("ymsnd", ym2151_device, status_r, data_w, 0x00ff)
	AM_RANGE(0x1c0, 0x1c3) AM_READ(eeprom_r)
	AM_RANGE(0x240, 0x243) AM_WRITE(flipscreen_w)
	AM_RANGE(0x600, 0x603) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x604, 0x607) AM_READ_PORT("P1_P2")
	AM_RANGE(0x608, 0x60b) AM_WRITE(eeprom_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( misncrft_io, AS_IO, 16, vamphalf_state )
	AM_RANGE(0x100, 0x103) AM_WRITE(flipscreen_w)
	AM_RANGE(0x200, 0x203) AM_READ_PORT("P1_P2")
	AM_RANGE(0x240, 0x243) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x3c0, 0x3c3) AM_WRITE(eeprom_w)
	AM_RANGE(0x400, 0x403) AM_WRITE(misncrft_snd_w)
	AM_RANGE(0x580, 0x583) AM_READ(eeprom_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( coolmini_io, AS_IO, 16, vamphalf_state )
//  AM_RANGE(0x002, 0x003) AM_WRITE(mrkicker_oki_bank_w) // not coolmini?, installed on init
	AM_RANGE(0x200, 0x203) AM_WRITE(flipscreen_w)
	AM_RANGE(0x300, 0x303) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x304, 0x307) AM_READ_PORT("P1_P2")
	AM_RANGE(0x308, 0x30b) AM_WRITE(eeprom_w)
	AM_RANGE(0x4c0, 0x4c1) AM_NOP // return 0, when oki chip is read / written
	AM_RANGE(0x4c2, 0x4c3) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x540, 0x543) AM_DEVWRITE8("ymsnd", ym2151_device, register_w, 0x00ff)
	AM_RANGE(0x544, 0x547) AM_DEVREADWRITE8("ymsnd", ym2151_device, status_r, data_w, 0x00ff)
	AM_RANGE(0x7c0, 0x7c3) AM_READ(eeprom_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( suplup_io, AS_IO, 16, vamphalf_state )
	AM_RANGE(0x020, 0x023) AM_WRITE(eeprom_w)
	AM_RANGE(0x040, 0x043) AM_READ_PORT("P1_P2")
	AM_RANGE(0x060, 0x063) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x080, 0x081) AM_NOP // return 0, when oki chip is read / written
	AM_RANGE(0x082, 0x083) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x0c0, 0x0c3) AM_DEVWRITE8("ymsnd", ym2151_device, register_w, 0x00ff)
	AM_RANGE(0x0c4, 0x0c7) AM_DEVREADWRITE8("ymsnd", ym2151_device, status_r, data_w, 0x00ff)
	AM_RANGE(0x100, 0x103) AM_READ(eeprom_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( wyvernwg_io, AS_IO, 32, vamphalf_state )
	AM_RANGE(0x1800, 0x1803) AM_READWRITE(wyvernwg_prot_r, wyvernwg_prot_w)
	AM_RANGE(0x2000, 0x2003) AM_WRITE(flipscreen32_w)
	AM_RANGE(0x2800, 0x2803) AM_READ_PORT("P1_P2")
	AM_RANGE(0x3000, 0x3003) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x5400, 0x5403) AM_WRITE(wyvernwg_snd_w)
	AM_RANGE(0x7000, 0x7003) AM_WRITE(eeprom32_w)
	AM_RANGE(0x7c00, 0x7c03) AM_READ(eeprom32_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( finalgdr_io, AS_IO, 32, vamphalf_state )
	AM_RANGE(0x2400, 0x2403) AM_READ(finalgdr_prot_r)
	AM_RANGE(0x2800, 0x2803) AM_WRITE(finalgdr_backupram_bank_w)
	AM_RANGE(0x2c00, 0x2dff) AM_READWRITE(finalgdr_backupram_r, finalgdr_backupram_w)
	AM_RANGE(0x3000, 0x3007) AM_DEVREADWRITE8("ymsnd", ym2151_device, read, write, 0x0000ff00)
	AM_RANGE(0x3800, 0x3803) AM_READ_PORT("P1_P2")
	AM_RANGE(0x3400, 0x3403) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x0000ff00)
	AM_RANGE(0x3c00, 0x3c03) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x4400, 0x4403) AM_READ(eeprom32_r)
	AM_RANGE(0x6000, 0x6003) AM_READNOP //?
	AM_RANGE(0x6000, 0x6003) AM_WRITE(finalgdr_eeprom_w)
	AM_RANGE(0x6040, 0x6043) AM_WRITE(finalgdr_prot_w)
	//AM_RANGE(0x6080, 0x6083) AM_WRITE(flipscreen32_w) //?
	AM_RANGE(0x6060, 0x6063) AM_WRITE(finalgdr_prize_w)
	AM_RANGE(0x60a0, 0x60a3) AM_WRITE(finalgdr_oki_bank_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mrkickera_io, AS_IO, 32, vamphalf_state )
	AM_RANGE(0x2400, 0x2403) AM_READ(eeprom32_r)
	AM_RANGE(0x4000, 0x4003) AM_READNOP //?
	AM_RANGE(0x4000, 0x4003) AM_WRITE(finalgdr_eeprom_w)
	AM_RANGE(0x4040, 0x4043) AM_WRITE(finalgdr_prot_w)
	AM_RANGE(0x4084, 0x4087) AM_WRITENOP //?
	AM_RANGE(0x40a0, 0x40a3) AM_WRITE(finalgdr_oki_bank_w)
	AM_RANGE(0x6400, 0x6403) AM_READ(finalgdr_prot_r)
	AM_RANGE(0x7000, 0x7007) AM_DEVREADWRITE8("ymsnd", ym2151_device, read, write, 0x0000ff00)
	AM_RANGE(0x7400, 0x7403) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x0000ff00)
	AM_RANGE(0x7800, 0x7803) AM_READ_PORT("P1_P2")
	AM_RANGE(0x7c00, 0x7c03) AM_READ_PORT("SYSTEM")
ADDRESS_MAP_END



static ADDRESS_MAP_START( jmpbreak_io, AS_IO, 16, vamphalf_state )
	AM_RANGE(0x0c0, 0x0c3) AM_NOP // ?
	AM_RANGE(0x100, 0x103) AM_WRITENOP // ?
	AM_RANGE(0x240, 0x243) AM_READ_PORT("P1_P2")
	AM_RANGE(0x280, 0x283) AM_WRITE(eeprom_w)
	AM_RANGE(0x2c0, 0x2c3) AM_READ(eeprom_r)
	AM_RANGE(0x440, 0x441) AM_NOP // return 0, when oki chip is read / written
	AM_RANGE(0x442, 0x443) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x540, 0x543) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x680, 0x683) AM_DEVWRITE8("ymsnd", ym2151_device, register_w, 0x00ff)
	AM_RANGE(0x684, 0x687) AM_DEVREADWRITE8("ymsnd", ym2151_device, status_r, data_w, 0x00ff)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mrdig_io, AS_IO, 16, vamphalf_state )
	AM_RANGE(0x500, 0x503) AM_READ_PORT("P1_P2")
	AM_RANGE(0x3c0, 0x3c3) AM_WRITE(eeprom_w)
	AM_RANGE(0x180, 0x183) AM_READ(eeprom_r)
	AM_RANGE(0x080, 0x081) AM_NOP // return 0, when oki chip is read / written
	AM_RANGE(0x082, 0x083) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x280, 0x283) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x0c0, 0x0c3) AM_DEVWRITE8("ymsnd", ym2151_device, register_w, 0x00ff)
	AM_RANGE(0x0c4, 0x0c7) AM_DEVREADWRITE8("ymsnd", ym2151_device, status_r, data_w, 0x00ff)
ADDRESS_MAP_END

static ADDRESS_MAP_START( aoh_map, AS_PROGRAM, 32, vamphalf_state )
	AM_RANGE(0x00000000, 0x003fffff) AM_RAM AM_SHARE("wram32")
	AM_RANGE(0x40000000, 0x4003ffff) AM_RAM AM_SHARE("tiles32")
	AM_RANGE(0x80000000, 0x8000ffff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x80210000, 0x80210003) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x80220000, 0x80220003) AM_READ_PORT("P1_P2")
	AM_RANGE(0xffc00000, 0xffffffff) AM_ROM AM_REGION("user1",0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( aoh_io, AS_IO, 32, vamphalf_state )
	AM_RANGE(0x0480, 0x0483) AM_WRITE(eeprom32_w)
	AM_RANGE(0x0620, 0x0623) AM_DEVREADWRITE8("oki_2", okim6295_device, read, write, 0x0000ff00)
	AM_RANGE(0x0660, 0x0663) AM_DEVREADWRITE8("oki_1", okim6295_device, read, write, 0x0000ff00)
	AM_RANGE(0x0640, 0x0647) AM_DEVREADWRITE8("ymsnd", ym2151_device, read, write, 0x0000ff00)
	AM_RANGE(0x0680, 0x0683) AM_WRITE(aoh_oki_bank_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( boonggab_io, AS_IO, 16, vamphalf_state )
	AM_RANGE(0x0c0, 0x0c3) AM_READ(eeprom_r)
	AM_RANGE(0x200, 0x203) AM_NOP // seems unused
	AM_RANGE(0x300, 0x303) AM_WRITE(flipscreen_w)
	AM_RANGE(0x400, 0x403) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x404, 0x407) AM_READ_PORT("P1_P2")
	AM_RANGE(0x408, 0x40b) AM_WRITE(eeprom_w)
	AM_RANGE(0x410, 0x413) AM_WRITE(boonggab_prize_w)
	AM_RANGE(0x414, 0x41b) AM_WRITE(boonggab_lamps_w)
	AM_RANGE(0x600, 0x603) AM_WRITE(boonggab_oki_bank_w)
	AM_RANGE(0x700, 0x701) AM_NOP // return 0, when oki chip is read / written
	AM_RANGE(0x702, 0x703) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x740, 0x743) AM_DEVWRITE8("ymsnd", ym2151_device, register_w, 0x00ff)
	AM_RANGE(0x744, 0x747) AM_DEVREADWRITE8("ymsnd", ym2151_device, status_r, data_w, 0x00ff)
ADDRESS_MAP_END

static ADDRESS_MAP_START( yorijori_io, AS_IO, 32, vamphalf_state )
ADDRESS_MAP_END

/*
Sprite list:

Offset+0
-------- xxxxxxxx Y offs
-------x -------- Don't draw the sprite
x------- -------- Flip X
-x------ -------- Flip Y

Offset+1
xxxxxxxx xxxxxxxx Sprite number

Offset+2
-------- -xxxxxxx Color
or
-xxxxxxx -------- Color

Offset+3
-------x xxxxxxxx X offs
*/

void vamphalf_state::video_start()
{
	save_item(NAME(m_flipscreen));
}

void vamphalf_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap)
{
	gfx_element *gfx = m_gfxdecode->gfx(0);
	UINT32 cnt;
	int block, offs;
	int code,color,x,y,fx,fy;
	rectangle clip;

	clip.min_x = screen.visible_area().min_x;
	clip.max_x = screen.visible_area().max_x;

	for (block=0; block<0x8000; block+=0x800)
	{
		if(m_flipscreen)
		{
			clip.min_y = 256 - (16-(block/0x800))*16;
			clip.max_y = 256 - ((16-(block/0x800))*16)+15;
		}
		else
		{
			clip.min_y = (16-(block/0x800))*16;
			clip.max_y = ((16-(block/0x800))*16)+15;
		}

		if (clip.min_y < screen.visible_area().min_y)
			clip.min_y = screen.visible_area().min_y;

		if (clip.max_y > screen.visible_area().max_y)
			clip.max_y = screen.visible_area().max_y;

		for (cnt=0; cnt<0x800; cnt+=8)
		{
			offs = (block + cnt) / 2;

			// 16bit version
			if(m_tiles != NULL)
			{
				if(m_tiles[offs] & 0x0100) continue;

				code  = m_tiles[offs+1];
				color = (m_tiles[offs+2] >> m_palshift) & 0x7f;

				// boonggab
				if(m_has_extra_gfx)
				{
					code  |= ((m_tiles[offs+2] & 0x100) << 8);
				}

				x = m_tiles[offs+3] & 0x01ff;
				y = 256 - (m_tiles[offs] & 0x00ff);

				fx = m_tiles[offs] & 0x8000;
				fy = m_tiles[offs] & 0x4000;
			}
			// 32bit version
			else
			{
				offs /= 2;

				if(m_tiles32[offs] & 0x01000000) continue;

				code  = m_tiles32[offs] & 0xffff;
				color = ((m_tiles32[offs+1] >> m_palshift) & 0x7f0000) >> 16;

				x = m_tiles32[offs+1] & 0x01ff;
				y = 256 - ((m_tiles32[offs] & 0x00ff0000) >> 16);

				fx = m_tiles32[offs] & 0x80000000;
				fy = m_tiles32[offs] & 0x40000000;
			}

			if(m_flipscreen)
			{
				fx = !fx;
				fy = !fy;

				x = 366 - x;
				y = 256 - y;
			}

			gfx->transpen(bitmap,clip,code,color,fx,fy,x,y,0);
		}
	}
}

void vamphalf_state::draw_sprites_aoh(screen_device &screen, bitmap_ind16 &bitmap)
{
	gfx_element *gfx = m_gfxdecode->gfx(0);
	UINT32 cnt;
	int block, offs;
	int code,color,x,y,fx,fy;
	rectangle clip;

	clip.min_x = screen.visible_area().min_x;
	clip.max_x = screen.visible_area().max_x;

	for (block=0; block<0x8000; block+=0x800)
	{
		if(m_flipscreen)
		{
			clip.min_y = 256 - (16-(block/0x800))*16;
			clip.max_y = 256 - ((16-(block/0x800))*16)+15;
		}
		else
		{
			clip.min_y = (16-(block/0x800))*16;
			clip.max_y = ((16-(block/0x800))*16)+15;
		}

		if (clip.min_y < screen.visible_area().min_y)
			clip.min_y = screen.visible_area().min_y;

		if (clip.max_y > screen.visible_area().max_y)
			clip.max_y = screen.visible_area().max_y;


		for (cnt=0; cnt<0x800; cnt+=8)
		{
			offs = (block + cnt) / 2;
			{
				offs /= 2;
				code  = (m_tiles32[offs] & 0xffff) | ((m_tiles32[offs] & 0x3000000) >> 8);
				color = ((m_tiles32[offs+1] >> m_palshift) & 0x7f0000) >> 16;

				x = m_tiles32[offs+1] & 0x01ff;
				y = 256 - ((m_tiles32[offs] & 0x00ff0000) >> 16);

				fx = m_tiles32[offs] & 0x4000000;
				fy = 0; // not used ? or it's m_tiles32[offs] & 0x8000000?
			}

			if(m_flipscreen)
			{
				fx = !fx;
				fy = !fy;

				x = 366 - x;
				y = 256 - y;
			}

			gfx->transpen(bitmap,clip,code,color,fx,fy,x,y,0);
		}
	}
}


void vamphalf_state::handle_flipped_visible_area( screen_device &screen )
{
	// are there actually registers to handle this?
	if(!m_flipscreen)
	{
		rectangle visarea;
		visarea.set(31, 350, 16, 251);
		screen.configure(512, 256, visarea, HZ_TO_ATTOSECONDS(60));
	}
	else
	{
		rectangle visarea;
		visarea.set(31, 350, 20, 255);
		screen.configure(512, 256, visarea, HZ_TO_ATTOSECONDS(60));
	}
}


UINT32 vamphalf_state::screen_update_common(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	handle_flipped_visible_area(screen);
	bitmap.fill(0, cliprect);
	draw_sprites(screen, bitmap);
	return 0;
}

UINT32 vamphalf_state::screen_update_aoh(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
//  handle_flipped_visible_area(screen); // not on this?
	bitmap.fill(0, cliprect);
	draw_sprites_aoh(screen, bitmap);
	return 0;
}

CUSTOM_INPUT_MEMBER(vamphalf_state::boonggab_photo_sensors_r)
{
	static const UINT16 photo_sensors_table[8] = { 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00 };
	UINT8 res = ioport("PHOTO_SENSORS")->read();

	switch(res)
	{
		case 0x01:
			return photo_sensors_table[1]; // 5 - 7 points

		case 0x02:
			return photo_sensors_table[2]; // 8 - 10 points

		case 0x04:
			return photo_sensors_table[3]; // 11 - 13 points

		case 0x08:
			return photo_sensors_table[4]; // 14 - 16 points

		case 0x10:
			return photo_sensors_table[5]; // 17 - 19 points

		case 0x20:
			return photo_sensors_table[6]; // 20 - 22 points

		case 0x40:
			return photo_sensors_table[7]; // 23 - 25 points
	}

	return photo_sensors_table[0];
}


static INPUT_PORTS_START( common )
	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_SERVICE_NO_TOGGLE( 0x0010, IP_ACTIVE_LOW )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( finalgdr )
	PORT_START("P1_P2")
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x00800000, IP_ACTIVE_LOW )
INPUT_PORTS_END

static INPUT_PORTS_START( aoh )
	PORT_START("P1_P2")
	PORT_BIT( 0x000000001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x000000002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x000000004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x000000008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x000000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x000000020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x000000040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x0000ff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0xff000000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000010, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read) // eeprom bit
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0000ff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x00100000, IP_ACTIVE_LOW )
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( boonggab )
	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_SPECIAL ) // sensor 1
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_SPECIAL ) // sensor 2
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_SPECIAL ) // sensor 3
	PORT_BIT( 0x3800, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, vamphalf_state,boonggab_photo_sensors_r, NULL) // photo sensors 1, 2 and 3
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_SERVICE_NO_TOGGLE( 0x0010, IP_ACTIVE_LOW )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("PHOTO_SENSORS")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_PLAYER(1)
INPUT_PORTS_END

static const gfx_layout sprites_layout =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 0,8,16,24, 32,40,48,56, 64,72,80,88 ,96,104,112,120 },
	{ 0*128, 1*128, 2*128, 3*128, 4*128, 5*128, 6*128, 7*128, 8*128,9*128,10*128,11*128,12*128,13*128,14*128,15*128 },
	16*128,
};

static GFXDECODE_START( vamphalf )
	GFXDECODE_ENTRY( "gfx1", 0, sprites_layout, 0, 0x80 )
GFXDECODE_END


static MACHINE_CONFIG_START( common, vamphalf_state )
	MCFG_CPU_ADD("maincpu", E116T, 50000000)    /* 50 MHz */
	MCFG_CPU_PROGRAM_MAP(common_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", vamphalf_state,  irq1_line_hold)

	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")
	// various games require fast timing to save settings, probably because our Hyperstone core timings are incorrect
	MCFG_EEPROM_ERASE_TIME(attotime::from_usec(1))
	MCFG_EEPROM_WRITE_TIME(attotime::from_usec(1))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(31, 350, 16, 251)
	MCFG_SCREEN_UPDATE_DRIVER(vamphalf_state, screen_update_common)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 0x8000)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", vamphalf)
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( sound_ym_oki )
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_YM2151_ADD("ymsnd", XTAL_28MHz/8) /* 3.5MHz */
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	MCFG_OKIM6295_ADD("oki", XTAL_28MHz/16 , OKIM6295_PIN7_HIGH) /* 1.75MHz */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( sound_suplup )
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_YM2151_ADD("ymsnd", XTAL_14_31818MHz/4) /* 3.579545 MHz */
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	MCFG_OKIM6295_ADD("oki", XTAL_14_31818MHz/8, OKIM6295_PIN7_HIGH) /* 1.7897725 MHz */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( sound_qs1000 )
	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("qs1000", QS1000, XTAL_24MHz)
	MCFG_QS1000_EXTERNAL_ROM(true)
	MCFG_QS1000_IN_P1_CB(READ8(vamphalf_state, qs1000_p1_r))
	MCFG_QS1000_OUT_P3_CB(WRITE8(vamphalf_state, qs1000_p3_w))
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( vamphalf, common )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(vamphalf_io)

	MCFG_FRAGMENT_ADD(sound_ym_oki)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( misncrft, common )
	MCFG_CPU_REPLACE("maincpu", GMS30C2116, XTAL_50MHz)   /* 50 MHz */
	MCFG_CPU_PROGRAM_MAP(common_map)
	MCFG_CPU_IO_MAP(misncrft_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", vamphalf_state,  irq1_line_hold)

	MCFG_FRAGMENT_ADD(sound_qs1000)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( coolmini, common )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(coolmini_io)

	MCFG_FRAGMENT_ADD(sound_ym_oki)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( suplup, common )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(suplup_io)

	MCFG_FRAGMENT_ADD(sound_suplup)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( jmpbreak, common )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(jmpbreak_io)

	MCFG_FRAGMENT_ADD(sound_ym_oki)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( mrdig, common )
	MCFG_CPU_REPLACE("maincpu", GMS30C2116, XTAL_50MHz)   /* 50 MHz */
	MCFG_CPU_PROGRAM_MAP(common_map)
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(mrdig_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", vamphalf_state,  irq1_line_hold)

	MCFG_FRAGMENT_ADD(sound_ym_oki)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( wyvernwg, common )
	MCFG_CPU_REPLACE("maincpu", E132T, XTAL_50MHz)    /* 50 MHz */
	MCFG_CPU_PROGRAM_MAP(common_32bit_map)
	MCFG_CPU_IO_MAP(wyvernwg_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", vamphalf_state,  irq1_line_hold)

	MCFG_FRAGMENT_ADD(sound_qs1000)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( finalgdr, common )
	MCFG_CPU_REPLACE("maincpu", E132T, XTAL_50MHz)    /* 50 MHz */
	MCFG_CPU_PROGRAM_MAP(common_32bit_map)
	MCFG_CPU_IO_MAP(finalgdr_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", vamphalf_state,  irq1_line_hold)

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_FRAGMENT_ADD(sound_ym_oki)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( mrkickera, common )
	MCFG_CPU_REPLACE("maincpu", E132T, XTAL_50MHz)    /* 50 MHz */
	MCFG_CPU_PROGRAM_MAP(common_32bit_map)
	MCFG_CPU_IO_MAP(mrkickera_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", vamphalf_state,  irq1_line_hold)

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_FRAGMENT_ADD(sound_ym_oki)
MACHINE_CONFIG_END



static MACHINE_CONFIG_START( aoh, vamphalf_state )
	MCFG_CPU_ADD("maincpu", E132XN, XTAL_20MHz*4) /* 4x internal multiplier */
	MCFG_CPU_PROGRAM_MAP(aoh_map)
	MCFG_CPU_IO_MAP(aoh_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", vamphalf_state,  irq1_line_hold)

	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(59.185)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 512)
	MCFG_SCREEN_VISIBLE_AREA(64, 511-64, 16, 255-16)
	MCFG_SCREEN_UPDATE_DRIVER(vamphalf_state, screen_update_aoh)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 0x8000)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", vamphalf)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_YM2151_ADD("ymsnd", XTAL_3_579545MHz)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	MCFG_OKIM6295_ADD("oki_1", XTAL_32MHz/8, OKIM6295_PIN7_HIGH) /* 4MHz */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)

	MCFG_OKIM6295_ADD("oki_2", XTAL_32MHz/32, OKIM6295_PIN7_HIGH) /* 1MHz */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( boonggab, common )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(boonggab_io)

	MCFG_FRAGMENT_ADD(sound_ym_oki)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( yorijori, common )
	MCFG_CPU_REPLACE("maincpu", E132T, XTAL_50MHz)    /* 50 MHz */
	MCFG_CPU_PROGRAM_MAP(yorijori_32bit_map)
	MCFG_CPU_IO_MAP(yorijori_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", vamphalf_state,  irq1_line_hold)

	MCFG_FRAGMENT_ADD(sound_qs1000)
MACHINE_CONFIG_END

/*

Vamp 1/2 (Semi Vamp)
Danbi, 1999


PCB Layout
----------
             KA12    VROM1.

             BS901   AD-65    ROML01.   ROMU01.
                              ROML00.   ROMU00.
                 62256
                 62256

T2316162A  E1-16T  PROM1.          QL2003-XPL84C

                 62256
                 62256       62256
                             62256
    93C46.IC3                62256
                             62256
    50.000MHz  QL2003-XPL84C
B1 B2 B3                     28.000MHz



Notes
-----
B1 B2 B3:      Push buttons for SERV, RESET, TEST
T2316162A:     Main program RAM
E1-16T:        Hyperstone E1-16T CPU
QL2003-XPL84C: QuickLogic PLCC84 PLD
AD-65:         Compatible to OKI M6295
KA12:          Compatible to Y3012
BS901          Compatible to YM2151
PROM1:         Main program
VROM1:         OKI samples
ROML* / U*:    Graphics, device is MX29F1610ML (surface mounted SOP44 MASK ROM)

*/

ROM_START( vamphalf )
	ROM_REGION16_BE( 0x100000, "user1", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	/* 0 - 0x80000 empty */
	ROM_LOAD( "prg.rom1", 0x80000, 0x80000, CRC(9b1fc6c5) SHA1(acf10a50d2119ac893b6cbd494911982a9352350) )

	ROM_REGION( 0x800000, "gfx1", 0 ) /* 16x16x8 Sprites */
	ROM_LOAD32_WORD( "eur.roml00", 0x000000, 0x200000, CRC(bdee9a46) SHA1(7e240b07377201afbe0cd0911ccee4ad52a74079) )
	ROM_LOAD32_WORD( "eur.romu00", 0x000002, 0x200000, CRC(fa79e8ea) SHA1(feaba99f0a863bc5d27ad91d206168684976b4c2) )
	ROM_LOAD32_WORD( "eur.roml01", 0x400000, 0x200000, CRC(a7995b06) SHA1(8b789b6a00bc177c3329ee4a31722fc65376b975) )
	ROM_LOAD32_WORD( "eur.romu01", 0x400002, 0x200000, CRC(e269f5fe) SHA1(70f1308f11e147dd20f8bd45b91aefc9fd653da6) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Oki Samples */
	ROM_LOAD( "snd.vrom1", 0x00000, 0x40000, CRC(ee9e371e) SHA1(3ead5333121a77d76e4e40a0e0bf0dbc75f261eb) )
ROM_END


ROM_START( vamphalfk )
	ROM_REGION16_BE( 0x100000, "user1", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	/* 0 - 0x80000 empty */
	ROM_LOAD( "prom1", 0x80000, 0x80000, CRC(f05e8e96) SHA1(c860e65c811cbda2dc70300437430fb4239d3e2d) )

	ROM_REGION( 0x800000, "gfx1", 0 ) /* 16x16x8 Sprites */
	ROM_LOAD32_WORD( "roml00", 0x000000, 0x200000, CRC(cc075484) SHA1(6496d94740457cbfdac3d918dce2e52957341616) )
	ROM_LOAD32_WORD( "romu00", 0x000002, 0x200000, CRC(711c8e20) SHA1(1ef7f500d6f5790f5ae4a8b58f96ee9343ef8d92) )
	ROM_LOAD32_WORD( "roml01", 0x400000, 0x200000, CRC(626c9925) SHA1(c90c72372d145165a8d3588def12e15544c6223b) )
	ROM_LOAD32_WORD( "romu01", 0x400002, 0x200000, CRC(d5be3363) SHA1(dbdd0586909064e015f190087f338f37bbf205d2) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Oki Samples */
	ROM_LOAD( "snd.vrom1", 0x00000, 0x40000, CRC(ee9e371e) SHA1(3ead5333121a77d76e4e40a0e0bf0dbc75f261eb) )
ROM_END

/*

Super Lup Lup Puzzle / Lup Lup Puzzle
Omega System, 1999

PCB Layout
----------

F-E1-16-001
|----------------------------------------------|
|       M6295       VROM1    N341256           |
|  YM3012                                      |
|       YM2151    |---------|N341256           |
|                 |Quicklogi|                  |
|                 |c        |N341256           |
|J                |QL2003-  |                  |
|A        N341256 |XPL84C   |N341256           |
|M                |---------|                  |
|M        N341256 |---------|N341256           |
|A                |Quicklogi|                  |
|         N341256 |c        |N341256           |
|                 |QL2003-  |                  |
|         N341256 |XPL84C   |N341256           |
|                 |---------|    ROML00  ROMU00|
|93C46            GM71C18163 N341256           |
|PAL          E1-16T             ROML01  ROMU01|
|TEST  ROM1                                    |
|SERV                                          |
|RESET ROM2   50MHz                 14.31818MHz|
|----------------------------------------------|
Notes:
      E1-16T clock : 50.000MHz
      M6295 clock  : 1.7897725MHz (14.31818/8). Sample Rate = 1789772.5 / 132
      YM2151 clock : 3.579545MHz (14.31818/4). Chip stamped 'KA51' on one PCB, BS901 on another
      VSync        : 60Hz
      N341256      : NKK N341256SJ-15 32K x8 SRAM (SOJ28)
      GM71C18163   : LG Semi GM71C18163 1M x16 EDO DRAM (SOJ44)

      ROMs:
           ROML00/01, ROMU00/01 - Macronix MX29F1610MC-12 SOP44 16MBit FlashROM
           VROM1                - Macronix MX27C2000 2MBit DIP32 EPROM
           ROM1/2               - ST M27C4001 4MBit DIP32 EPROM
*/

ROM_START( suplup ) /* version 4.0 / 990518 - also has 'Puzzle Bang Bang' title but it can't be selected */
	ROM_REGION16_BE( 0x100000, "user1", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	ROM_LOAD( "suplup-rom1.bin", 0x00000, 0x80000, CRC(61fb2dbe) SHA1(21cb8f571b2479de6779b877b656d1ffe5b3516f) )
	ROM_LOAD( "suplup-rom2.bin", 0x80000, 0x80000, CRC(0c176c57) SHA1(f103a1afc528c01cbc18639273ab797fb9afacb1) )

	ROM_REGION( 0x800000, "gfx1", 0 ) /* 16x16x8 Sprites */
	ROM_LOAD32_WORD( "suplup-roml00.bin", 0x000000, 0x200000, CRC(7848e183) SHA1(1db8f0ea8f73f42824423d382b37b4d75fa3e54c) )
	ROM_LOAD32_WORD( "suplup-romu00.bin", 0x000002, 0x200000, CRC(13e3ab7f) SHA1(d5b6b15ca5aef2e2788d2b81e0418062f42bf2f2) )
	ROM_LOAD32_WORD( "suplup-roml01.bin", 0x400000, 0x200000, CRC(15769f55) SHA1(2c13e8da2682ccc7878218aaebe3c3c67d163fd2) )
	ROM_LOAD32_WORD( "suplup-romu01.bin", 0x400002, 0x200000, CRC(6687bc6f) SHA1(cf842dfb2bcdfda0acc0859985bdba91d4a80434) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Oki Samples */
	ROM_LOAD( "vrom1.bin", 0x00000, 0x40000, CRC(34a56987) SHA1(4d8983648a7f0acf43ff4c9c8aa6c8640ee2bbfe) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) /* Default EEPROM */
	ROM_LOAD( "eeprom-suplup.bin", 0x0000, 0x0080, CRC(e60c9883) SHA1(662dd8fb85eb97a8a4d53886198b269a5f6a6268) )
ROM_END

ROM_START( luplup ) /* version 3.0 / 990128 */
	ROM_REGION16_BE( 0x100000, "user1", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	ROM_LOAD( "luplup-rom1.v30", 0x00000, 0x80000, CRC(9ea67f87) SHA1(73d16c056a8d64743181069a01559a43fee529a3) )
	ROM_LOAD( "luplup-rom2.v30", 0x80000, 0x80000, CRC(99840155) SHA1(e208f8731c06b634e84fb73e04f6cdbb8b504b94) )

	ROM_REGION( 0x800000, "gfx1", 0 ) /* 16x16x8 Sprites */
	ROM_LOAD32_WORD( "luplup-roml00",   0x000000, 0x200000, CRC(08b2aa75) SHA1(7577b3ab79c54980307a83186dd1500f044c1bc8) )
	ROM_LOAD32_WORD( "luplup-romu00",   0x000002, 0x200000, CRC(b57f4ca5) SHA1(b968c44a0ceb3274e066fa1d057fb6b017bb3fd3) )
	ROM_LOAD32_WORD( "luplup30-roml01", 0x400000, 0x200000, CRC(40e85f94) SHA1(531e67eb4eedf47b0dded52ba2f4942b12cbbe2f) ) /* This one changed between v2.9 & v3.0 */
	ROM_LOAD32_WORD( "luplup30-romu01", 0x400002, 0x200000, CRC(f2645b78) SHA1(b54c3047346c0f40dba0ba23b0d607cc53384edb) ) /* This one changed between v2.9 & v3.0 */

	ROM_REGION( 0x40000, "oki", 0 ) /* Oki Samples */
	ROM_LOAD( "vrom1.bin", 0x00000, 0x40000, CRC(34a56987) SHA1(4d8983648a7f0acf43ff4c9c8aa6c8640ee2bbfe) )

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "gal22v10b.gal1", 0x0000, 0x02e5, NO_DUMP ) /* GAL is read protected */
ROM_END


ROM_START( luplup29 ) /* version 2.9 / 990108 */
	ROM_REGION16_BE( 0x100000, "user1", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	ROM_LOAD( "luplup-rom1.v29", 0x00000, 0x80000, CRC(36a8b8c1) SHA1(fed3eb2d83adc1b071a12ce5d49d4cab0ca20cc7) )
	ROM_LOAD( "luplup-rom2.v29", 0x80000, 0x80000, CRC(50dac70f) SHA1(0e313114a988cb633a89508fda17eb09023827a2) )

	ROM_REGION( 0x800000, "gfx1", 0 ) /* 16x16x8 Sprites */
	ROM_LOAD32_WORD( "luplup-roml00", 0x000000, 0x200000, CRC(08b2aa75) SHA1(7577b3ab79c54980307a83186dd1500f044c1bc8) )
	ROM_LOAD32_WORD( "luplup-romu00", 0x000002, 0x200000, CRC(b57f4ca5) SHA1(b968c44a0ceb3274e066fa1d057fb6b017bb3fd3) )
	ROM_LOAD32_WORD( "luplup-roml01", 0x400000, 0x200000, CRC(41c7ca8c) SHA1(55704f9d54f31bbaa044cd9d10ac2d9cb5e8fb70) )
	ROM_LOAD32_WORD( "luplup-romu01", 0x400002, 0x200000, CRC(16746158) SHA1(a5036a7aaa717fde89d62b7ff7a3fded8b7f5cda) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Oki Samples */
	ROM_LOAD( "vrom1.bin", 0x00000, 0x40000, CRC(34a56987) SHA1(4d8983648a7f0acf43ff4c9c8aa6c8640ee2bbfe) )
ROM_END


ROM_START( puzlbang ) /* version 2.9 / 990108 - Korea only, cannot select title, language and limited selection of background choices, EI: censored  */
	ROM_REGION16_BE( 0x100000, "user1", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	ROM_LOAD( "pbb-rom1.v29", 0x00000, 0x80000, CRC(eb829586) SHA1(1f8a6af7c51c715724f5a242f4e22f7f6fb1f0ee) )
	ROM_LOAD( "pbb-rom2.v29", 0x80000, 0x80000, CRC(fb84c793) SHA1(a2d27caecdae457d12b48d88d19ce417f69507c6) )

	ROM_REGION( 0x800000, "gfx1", 0 ) /* 16x16x8 Sprites */
	ROM_LOAD32_WORD( "luplup-roml00", 0x000000, 0x200000, CRC(08b2aa75) SHA1(7577b3ab79c54980307a83186dd1500f044c1bc8) )
	ROM_LOAD32_WORD( "luplup-romu00", 0x000002, 0x200000, CRC(b57f4ca5) SHA1(b968c44a0ceb3274e066fa1d057fb6b017bb3fd3) )
	ROM_LOAD32_WORD( "luplup-roml01", 0x400000, 0x200000, CRC(41c7ca8c) SHA1(55704f9d54f31bbaa044cd9d10ac2d9cb5e8fb70) )
	ROM_LOAD32_WORD( "luplup-romu01", 0x400002, 0x200000, CRC(16746158) SHA1(a5036a7aaa717fde89d62b7ff7a3fded8b7f5cda) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Oki Samples */
	ROM_LOAD( "vrom1.bin", 0x00000, 0x40000, CRC(34a56987) SHA1(4d8983648a7f0acf43ff4c9c8aa6c8640ee2bbfe) )
ROM_END


ROM_START( puzlbanga ) /* version 2.8 / 990106 - Korea only, cannot select title, language or change background selection, EI: censored */
	ROM_REGION16_BE( 0x100000, "user1", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	ROM_LOAD( "pbb-rom1.v28", 0x00000, 0x80000, CRC(fd21c5ff) SHA1(bc6314bbb2495c140788025153c893d5fd00bdc1) )
	ROM_LOAD( "pbb-rom2.v28", 0x80000, 0x80000, CRC(490ecaeb) SHA1(2b0f25e3d681ddf95b3c65754900c046b5b50b09) )

	ROM_REGION( 0x800000, "gfx1", 0 ) /* 16x16x8 Sprites */
	ROM_LOAD32_WORD( "luplup-roml00", 0x000000, 0x200000, CRC(08b2aa75) SHA1(7577b3ab79c54980307a83186dd1500f044c1bc8) )
	ROM_LOAD32_WORD( "luplup-romu00", 0x000002, 0x200000, CRC(b57f4ca5) SHA1(b968c44a0ceb3274e066fa1d057fb6b017bb3fd3) )
	ROM_LOAD32_WORD( "luplup-roml01", 0x400000, 0x200000, CRC(41c7ca8c) SHA1(55704f9d54f31bbaa044cd9d10ac2d9cb5e8fb70) )
	ROM_LOAD32_WORD( "luplup-romu01", 0x400002, 0x200000, CRC(16746158) SHA1(a5036a7aaa717fde89d62b7ff7a3fded8b7f5cda) )
	ROM_REGION( 0x40000, "oki", 0 ) /* Oki Samples */
	ROM_LOAD( "vrom1.bin", 0x00000, 0x40000, CRC(34a56987) SHA1(4d8983648a7f0acf43ff4c9c8aa6c8640ee2bbfe) )
ROM_END

/*

Jumping Break
F2 System, 1999

sequel to "Die Break" from the Bestri 3 game collection (see crospang.c)

F-E1-16-002
+----------------------------------------------+
|     VR1                   M6295  VROM1 28MHz |
|                 YM3012                       |
|                 YM2151            MEM2       |
|                                   MEM3       |
|               CRAM1               MEM5       |
|               CRAM2               MEM7       |
|J                                             |
|A              MEM1U  +----------++----------+|
|M                     |          ||          ||
|M              MEM1L  |Quicklogic||Quicklogic||
|A                     | QL2003-  || QL2003-  ||
|                      | XPL84C   || XPL84C   ||
|                      |          ||          ||
|                      +----------++----------+|
|                                              |
| 93C46          DRAM1      ROM1 ROML00  ROMU00|
|P1 P2   50MHz   E1-16T     ROM2 ROML01  ROMU01|
|                                              |
+----------------------------------------------+

Notes:
CPU: Hyperstone E1-16T @ 50.000MHz

     DRAM1 - LG Semi GM71C18163 1M x16 EDO DRAM (SOJ44)
MEMx/CRAMx - NKK N341256SJ-15 32K x8 SRAM (SOJ28)

Oki M6295 rebaged as AD-65
YM3012/YM2151 rebaged as KA12/BS901

 P1 - Setup push button
 P2 - Reset push button
VR1 - Volume adjust pot

ROMs:
    ROML00/01, ROMU00/01 - Macronix MX29F1610MC-12 SOP44 16MBit FlashROM
    VROM1                - AMD 27C020 2MBit DIP32 EPROM
    ROM1/2               - TMS 27C040 4MBit DIP32 EPROM

*/

ROM_START( jmpbreak )
	ROM_REGION16_BE( 0x100000, "user1", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	ROM_LOAD( "rom1.bin", 0x00000, 0x80000, CRC(7e237f7d) SHA1(042e672be34644311eefc7b998bcdf6a9ea2c28a) )
	ROM_LOAD( "rom2.bin", 0x80000, 0x80000, CRC(c722f7be) SHA1(d8b3c6b5fd0942147e0a61169c3eb6334a3b5a40) )

	ROM_REGION( 0x800000, "gfx1", 0 ) /* 16x16x8 Sprites */
	ROM_LOAD32_WORD( "roml00.bin", 0x000000, 0x200000, CRC(4b99190a) SHA1(30af068f7d9f9f349db5696c19ab53ac33304271) )
	ROM_LOAD32_WORD( "romu00.bin", 0x000002, 0x200000, CRC(e93762f8) SHA1(cc589b59e3ab7aa7092e96a1ff8a9de8a499b257) )
	ROM_LOAD32_WORD( "roml01.bin", 0x400000, 0x200000, CRC(6796a104) SHA1(3f7352cd37f78c1b01f7df45344ee7800db110f9) )
	ROM_LOAD32_WORD( "romu01.bin", 0x400002, 0x200000, CRC(0cc907c8) SHA1(86029eca0870f3b7dd4f1ee8093ccb09077cc00b) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Oki Samples */
	ROM_LOAD( "vrom1.bin", 0x00000, 0x40000, CRC(1b6e3671) SHA1(bd601460387b56c989785ae03d5bb3c6cdb30a50) )
ROM_END

/*

Mr. Dig
SUN, 2000

Rip-off of Mr. Driller series

F-E1-16-002
+----------------------------------------------+
|     VR1                   M6295  VROM1 28MHz |
|                 YM3012                       |
|                 YM2151            MEM2       |
|                                   MEM3       |
|               CRAM1               MEM5       |
|               CRAM2               MEM7       |
|J                                             |
|A              MEM1U  +----------++----------+|
|M                     |          ||          ||
|M              MEM1L  |Quicklogic||Quicklogic||
|A                     | QL2003-  || QL2003-  ||
|                      | XPL84C   || XPL84C   ||
|                      |          ||          ||
|                      +----------++----------+|
|                                              |
| 93C46          DRAM1      ROM1 ROML00  ROMH00|
|P1 P2   50MHz  GMS30C2116  ROM2   *       *   |
|                                              |
+----------------------------------------------+

Notes:
CPU: HYUNDAI GMS30C2116 (Hyperstone E1-16T compatible) @ 50.000MHz

     DRAM1 - LG Semi GM71C18163 1M x16 EDO DRAM (SOJ44)
MEMx/CRAMx - NKK N341256SJ-15 32K x8 SRAM (SOJ28)

Oki M6295 rebaged as AD-65
YM3012/YM2151 rebaged as KA12/KB2001

 P1 - Setup push button
 P2 - Reset push button
VR1 - Volume adjust pot

ROMs:
    ROML00 & ROMH00 - Macronix MX29F1610MC-12 SOP44 16MBit FlashROM
  * ROML01 & ROMH01 - Unpopulated space for MX29F1610MC-12 SOP44 16MBit FlashROM
    VROM1           - Atmel AT27C020 2MBit DIP32 EPROM
    ROM1/2          - MX 27C4000 4MBit DIP32 EPROM

*/

ROM_START( mrdig )
	ROM_REGION16_BE( 0x100000, "user1", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	ROM_LOAD( "rom1.bin", 0x00000, 0x80000, CRC(5b960320) SHA1(adf5499a39987041fc93e409bdb5fd07dacec4f9) )
	ROM_LOAD( "rom2.bin", 0x80000, 0x80000, CRC(75d48b64) SHA1(c9c492fb9cabafcf0bc05f44bf80ee6df3c21a1b) )

	ROM_REGION( 0x800000, "gfx1", 0 ) /* 16x16x8 Sprites */
	ROM_LOAD32_WORD( "roml00.bin", 0x000000, 0x200000, CRC(f6b161ea) SHA1(c417a4c877ffa2fdf5857ecc9c78ffc0c09dc516) )
	ROM_LOAD32_WORD( "romh00.bin", 0x000002, 0x200000, CRC(5477efed) SHA1(e4991ee1b41d512eaa508351b6a78261dfde5a3d) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Oki Samples */
	ROM_LOAD( "vrom1.bin", 0x00000, 0x40000, CRC(5fd9e1c6) SHA1(fef82ef816af69f31d12fc4634d06d825e8b7416) )
ROM_END

/*

Cool Minigame Collection
SemiCom, 1999

PCB Layout
----------

F-E1-16-008
|-------------------------------------------------------|
|UPC1241            YM3012   VROM1                      |
|      LM324  LM324 YM2151                              |
|               MCM6206       M6295   ROML00    ROMU00  |
|                                                       |
|               MCM6206               ROML01    ROMU01  |
|                                                       |
|J              MCM6206               ROML02    ROMU02  |
|A                                                      |
|M              MCM6206               ROML03    ROMU03  |
|M                                                      |
|A              MCM6206                                 |
|                                                       |
|               MCM6206       QL2003    QL2003          |
|                                                28MHz  |
|               MCM6206                                 |
|                                                       |
|               MCM6206  E1-16T   GM71C1816     ROM1    |
|                                                       |
|              93C46                            ROM2    |
|RESET  TEST          50MHz              PAL            |
|-------------------------------------------------------|

*/

ROM_START( coolmini )
	ROM_REGION16_BE( 0x100000, "user1", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	ROM_LOAD( "cm-rom1.040", 0x00000, 0x80000, CRC(9688fa98) SHA1(d5ebeb1407980072f689c3b3a5161263c7082e9a) )
	ROM_LOAD( "cm-rom2.040", 0x80000, 0x80000, CRC(9d588fef) SHA1(7b6b0ba074c7fa0aecda2b55f411557b015522b6) )

	ROM_REGION( 0x1000000, "gfx1", 0 )  /* 16x16x8 Sprites */
	ROM_LOAD32_WORD( "roml00", 0x000000, 0x200000, CRC(4b141f31) SHA1(cf4885789b0df67d00f9f3659c445248c4e72446) )
	ROM_LOAD32_WORD( "romu00", 0x000002, 0x200000, CRC(9b2fb12a) SHA1(8dce367c4c2cab6e84f586bd8dfea3ea0b6d7225) )
	ROM_LOAD32_WORD( "roml01", 0x400000, 0x200000, CRC(1e3a04bb) SHA1(9eb84b6a0172a8868f440065c30b4519e0c3fe33) )
	ROM_LOAD32_WORD( "romu01", 0x400002, 0x200000, CRC(06dd1a6c) SHA1(8c707d388848bc5826fbfc48c3035fdaf5018515) )
	ROM_LOAD32_WORD( "roml02", 0x800000, 0x200000, CRC(1e8c12cb) SHA1(f57489e81eb1e476939148cfc8d03f3df03b2a84) )
	ROM_LOAD32_WORD( "romu02", 0x800002, 0x200000, CRC(4551d4fc) SHA1(4ec102120ab99e324d9574bfce93837d8334da06) )
	ROM_LOAD32_WORD( "roml03", 0xc00000, 0x200000, CRC(231650bf) SHA1(065f742a37d5476ec6f72f0bd8ba2cfbe626b872) )
	ROM_LOAD32_WORD( "romu03", 0xc00002, 0x200000, CRC(273d5654) SHA1(0ae3d1c4c4862a8642dbebd7c955b29df29c4938) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Oki Samples */
	ROM_LOAD( "cm-vrom1.020", 0x00000, 0x40000, CRC(fcc28081) SHA1(44031df0ee28ca49df12bcb73c83299fac205e21) )
ROM_END

/*

Date Quiz Go Go Episode 2
SemiCom, 2000

PCB Layout
----------

F-E1-16-010
+-----------------------------------------------+
|     VR1          YM3012  VROM1                |
|                  YM2151  M6295   ROML03 ROMU03|
|               CRAM2              ROML02 ROMU02|
|               CRAM1              ROML01 ROMU01|
|               MEM1L              ROML00 ROMU00|
|J              MEM1U                           |
|A              MEM2  +----------++----------+  |
|M                    |          ||          |  |
|M              MEM3  |Quicklogic||Quicklogic| 2|
|A                    | QL2003-  || QL2003-  | 8|
|               MEM6  | XPL84C   || XPL84C   | M|
|                     |          ||          | H|
|               MEM7  +----------++----------+ z|
|                      GAL                      |
|    93C46                       ROM1           |
|P1 P2   50MHz E1-16T   DRAM1    ROM2           |
+-----------------------------------------------+

Notes:
CPU - Hyperstone E1-16T @ 50.000MHz

DRAM1 - LG Semi GM71C18163 1M x16 EDO DRAM (SOJ44)
CRAMx - W24M257AK-15 32K x8 SRAM (SOJ28)
MEMx  - UM61256FK-15 32K x8 SRAM (SOJ28)

Oki M6295 rebaged as AD-65
YM3012/YM2151 rebaged as BS902/KA51

 P1 - Reset push button
 P2 - Setup push button
VR1 - Volume adjust pot

ROMs:
    ROML00 & ROMH00 - Macronix MX29F1610MC-12 SOP44 16MBit FlashROM
    ROML01 & ROMH01 - Macronix MX29F1610MC-12 SOP44 16MBit FlashROM
    ROML02 & ROMH02 - Macronix MX29F1610MC-12 SOP44 16MBit FlashROM
    ROML03 & ROMH03 - Unpopulated space for MX29F1610MC-12 SOP44 16MBit FlashROM
    VROM1           - 27C020 2MBit DIP32 EPROM
    ROM1            - 27C040 4MBit DIP32 EPROM
    ROM2            - 27C040 4MBit DIP32 EPROM

*/

ROM_START( dquizgo2 )
	ROM_REGION16_BE( 0x100000, "user1", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	ROM_LOAD( "rom1",         0x00000, 0x080000, CRC(81eef038) SHA1(9c925d1ef261ea85069925ccd1a5aeb939f55d5a) )
	ROM_LOAD( "rom2",         0x80000, 0x080000, CRC(e8789d8a) SHA1(1ee26c26cc7024c5df9d0da630b326021ece9f41) )

	ROM_REGION( 0xc00000, "gfx1", 0 ) /* 16x16x8 Sprites */
	ROM_LOAD32_WORD( "roml00", 0x000000, 0x200000, CRC(de811dd7) SHA1(bf31e165440ed2e3cdddd2174521b15afd8b2e69) )
	ROM_LOAD32_WORD( "romu00", 0x000002, 0x200000, CRC(2bdbfc6b) SHA1(8e755574e3c9692bd8f82c7351fe3623a31ec136) )
	ROM_LOAD32_WORD( "roml01", 0x400000, 0x200000, CRC(f574a2a3) SHA1(c6a8aca75bd3a4e4109db5095f3a3edb9b1e6657) )
	ROM_LOAD32_WORD( "romu01", 0x400002, 0x200000, CRC(d05cf02f) SHA1(624316d4ee42c6257bc64747e4260a0d3950f9cd) )
	ROM_LOAD32_WORD( "roml02", 0x800000, 0x200000, CRC(43ca2cff) SHA1(02ad7cce42d917dbefdba2e4e8886fc883b1dc60) )
	ROM_LOAD32_WORD( "romu02", 0x800002, 0x200000, CRC(b8218222) SHA1(1e1aa60e0de9c02b841896512a1163dda280c845) )
	/* roml03 empty */
	/* romu03 empty */

	ROM_REGION( 0x40000, "oki", 0 ) /* Oki Samples */
	ROM_LOAD( "vrom1", 0x00000, 0x40000, CRC(24d5b55f) SHA1(cb4d3a22440831e37df0a7fe5433bea708d60f31) )
ROM_END

/*

Diet Family
SemiCom, 2001

PCB Layout
----------

F-E1-16-010
+-----------------------------------------------+
|     VR1          YM3012  VROM1                |
|                  YM2151  M6295   ROML03 ROMU03|
|               CRAM2              ROML02 ROMU02|
|               CRAM1              ROML01 ROMU01|
|               MEM1L              ROML00 ROMU00|
|J              MEM1U                           |
|A              MEM2  +----------++----------+  |
|M                    |          ||          |  |
|M              MEM3  |Quicklogic||Quicklogic| 2|
|A                    | QL2003-  || QL2003-  | 8|
|               MEM6  | XPL84C   || XPL84C   | M|
|                     |          ||          | H|
|               MEM7  +----------++----------+ z|
|                      GAL                      |
|    93C46                       ROM1           |
|P1 P2   50MHz E1-16T   DRAM1    ROM2           |
+-----------------------------------------------+

Notes:
CPU - Hyperstone E1-16T @ 50.000MHz

DRAM1 - LG Semi GM71C18163 1M x16 EDO DRAM (SOJ44)
CRAMx - W24M257AK-15 32K x8 SRAM (SOJ28)
MEMx  - UM61256FK-15 32K x8 SRAM (SOJ28)

Oki M6295 rebaged as AD-65
YM3012/YM2151 rebaged as BS902/KA51

 P1 - Reset push button
 P2 - Setup push button
VR1 - Volume adjust pot

ROMs:
    ROML00 & ROMH00 - Macronix MX29F1610MC-12 SOP44 16MBit FlashROM
    ROML01 & ROMH01 - Macronix MX29F1610MC-12 SOP44 16MBit FlashROM
    ROML02 & ROMH02 - Macronix MX29F1610MC-12 SOP44 16MBit FlashROM
    ROML03 & ROMH03 - Unpopulated space for MX29F1610MC-12 SOP44 16MBit FlashROM
    VROM1           - 27C040 2MBit DIP32 EPROM
    ROM1            - 27C040 4MBit DIP32 EPROM
    ROM2            - 27C040 4MBit DIP32 EPROM

*/

ROM_START( dtfamily )
	ROM_REGION16_BE( 0x100000, "user1", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	ROM_LOAD( "rom1",         0x00000, 0x080000, CRC(738636d2) SHA1(ba7906df99764ee7e1f505c319d364c64c605ff0) )
	ROM_LOAD( "rom2",         0x80000, 0x080000, CRC(0953f5e4) SHA1(ee8b3c4f9c9301c9815747eab5435e006ec84ca1) )

	ROM_REGION( 0xc00000, "gfx1", 0 ) /* 16x16x8 Sprites */
	ROM_LOAD32_WORD( "roml00", 0x000000, 0x200000, CRC(7e2a7520) SHA1(0ff157fe34ff31cd8636af821fe14c12242d757f) )
	ROM_LOAD32_WORD( "romu00", 0x000002, 0x200000, CRC(c170755f) SHA1(019d24979071f0ab2b3c93a5ec9327e6a0b2afa2) )
	ROM_LOAD32_WORD( "roml01", 0x400000, 0x200000, CRC(3d487ffe) SHA1(c5608423d608922c0e1ac8bdfaa0de062b2c9821) )
	ROM_LOAD32_WORD( "romu01", 0x400002, 0x200000, CRC(716efedb) SHA1(fb468d93817a49173698872c49a289c257f77a92) )
	ROM_LOAD32_WORD( "roml02", 0x800000, 0x200000, CRC(c3dd3c96) SHA1(2707f690b5850685f782fc04a7dbd1d91f443223) )
	ROM_LOAD32_WORD( "romu02", 0x800002, 0x200000, CRC(80830961) SHA1(b318e9e3a4d1d7dca61d7d4c9ee01f605e2b2f4a) )
	/* roml03 empty */
	/* romu03 empty */

	ROM_REGION( 0x80000, "oki", 0 ) /* Oki Samples */
	ROM_LOAD( "vrom1", 0x00000, 0x80000, CRC(4aacaef3) SHA1(c079170dc0ba0b91b1780cd175dc38151d640ff0) )
ROM_END

/*

Toy Land Adventure
SemiCom, 2001

PCB Layout
----------

F-E1-16-010
+-----------------------------------------------+
|     VR1          YM3012  VROM1                |
|                  YM2151  M6295   ROML03 ROMU03|
|               CRAM2              ROML02 ROMU02|
|               CRAM1              ROML01 ROMU01|
|               MEM1L              ROML00 ROMU00|
|J              MEM1U                           |
|A              MEM2  +----------++----------+  |
|M                    |          ||          |  |
|M              MEM3  |Quicklogic||Quicklogic| 2|
|A                    | QL2003-  || QL2003-  | 8|
|               MEM6  | XPL84C   || XPL84C   | M|
|                     |          ||          | H|
|               MEM7  +----------++----------+ z|
|                      GAL                      |
|    93C46                       ROM1*          |
|P1 P2   50MHz E1-16T   DRAM1    ROM2           |
+-----------------------------------------------+

Notes:
CPU - Hyperstone E1-16T @ 50.000MHz

DRAM1 - LG Semi GM71C18163 1M x16 EDO DRAM (SOJ44)
CRAMx - W24M257AK-15 32K x8 SRAM (SOJ28)
MEMx  - UM61256FK-15 32K x8 SRAM (SOJ28)

Oki M6295 rebaged as AD-65
YM3012/YM2151 rebaged as BS902/KA51

 P1 - Reset push button
 P2 - Setup push button
VR1 - Volume adjust pot

ROMs:
    ROML00 & ROMH00 - Macronix MX29F1610MC-12 SOP44 16MBit FlashROM
    ROML01 & ROMH01 - Macronix MX29F1610MC-12 SOP44 16MBit FlashROM
    ROML02 & ROMH02 - Unpopulated space for MX29F1610MC-12 SOP44 16MBit FlashROM
    ROML03 & ROMH03 - Unpopulated space for MX29F1610MC-12 SOP44 16MBit FlashROM
    VROM1           - MX 27C2000 2MBit DIP32 EPROM
  * ROM1            - Unpopulated space for DIP32 EPROM (up to 4MBit)
    ROM2            - TMS 27C040 4MBit DIP32 EPROM

*/

ROM_START( toyland )
	ROM_REGION16_BE( 0x100000, "user1", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	/* ROM1 empty */
	ROM_LOAD( "rom2.bin",         0x80000, 0x080000, CRC(e3455002) SHA1(5ad7884f82fb125d70829accec02f238e7d9593c) )

	ROM_REGION( 0xc00000, "gfx1", 0 ) /* 16x16x8 Sprites */
	ROM_LOAD32_WORD( "roml00.bin", 0x000000, 0x200000, CRC(06f5673d) SHA1(23769015fc9a37d36b0fe4924964650aeca77573) )
	ROM_LOAD32_WORD( "romu00.bin", 0x000002, 0x200000, CRC(8c3db0e4) SHA1(6101ec550ae165338333fb04e0762edee65ca253) )
	ROM_LOAD32_WORD( "roml01.bin", 0x400000, 0x200000, CRC(076a84e1) SHA1(f58cb4cd874e1f3f266a5ccbf8ffb5e0111034d3) )
	ROM_LOAD32_WORD( "romu01.bin", 0x400002, 0x200000, CRC(1bc33d01) SHA1(a2a3e6b473cefe463dbd60bda98cb5a4df2bc81b) )
	/* roml02 empty */
	/* romu02 empty */
	/* roml03 empty */
	/* romu03 empty */

	ROM_REGION( 0x40000, "oki", 0 ) /* Oki Samples */
	ROM_LOAD( "vrom1.bin", 0x00000, 0x40000, CRC(d7e6fc5d) SHA1(ab5bca4035299214d98b222320276fbcaedb0898) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) /* Default EEPROM */
	ROM_LOAD( "epr1.ic3", 0x0000, 0x0080, CRC(812f3d87) SHA1(744919ff4b44eaa3c4dcc75a1cc2f231ccbb4a3e) )

ROM_END

/*

Wivern Wings (c) 2001 SemiCom / Wyvern Wings (c) 2001 SemiCom, Game Vision License

   CPU: Hyperstone E1-32T
 Video: 2 QuickLogic QL12x16B-XPL84 FPGA
 Sound: AdMOS QDSP1000 with QDSP QS1001A sample rom
   OSC: 50MHz, 28MHz & 24MHz
EEPROM: 93C46

F-E1-32-010-D
+------------------------------------------------------------------+
|    VOL    +-------+  +---------+                                 |
+-+         | QDSP  |  |  U15A   |      +---------+   +---------+  |
  |         |QS1001A|  |         |      | ROMH00  |   | ROML00  |  |
+-+         +-------+  +---------+      |         |   |         |  |
|           +-------+                   +---------+   +---------+  |
|           |QDSP   |   +----------+    +---------+   +---------+  |
|           |QS1000 |   |    U7    |    | ROMH01  |   | ROML01  |  |
|J   24MHz  +-------+   +----------+    |         |   |         |  |
|A                                      +---------+   +---------+  |
|M   50MHz           +-----+            +---------+   +---------+  |
|M                   |DRAM2|            | ROMH02  |   | ROML02  |  |
|A     +----------+  +-----+    +-----+ |         |   |         |  |
|      |          |  +-----+    |93C46| +---------+   +---------+  |
|C     |HyperStone|  |DRAM1|    +-----+ +---------+   +---------+  |
|O     |  E1-32T  |  +-----+            | ROMH03  |   | ROML03  |  |
|N     |          |              28MHz  |         |   |         |  |
|N     +----------+                     +---------+   +---------+  |
|E                                                                 |
|C           +----------+           +------------+ +------------+  |
|T           |   GAL1   |           | QuickLogic | | QuickLogic |  |
|O           +----------+           | 0048 BH    | | 0048 BH    |  |
|R           +----------+           | QL12X16B   | | QL12X16B   |  |
|            |   ROM2   |           | -XPL84C    | | -XPL84C    |  |
|            +----------+           +------------+ +------------+  |
|            +----------+            +----+                        |
|            |   ROM1   |            |MEM3|                        |
+-++--+      +----------+            +----+                        |
  ||S1|    +-----+                   |MEM2|                        |
+-++--+    |CRAM2|                   +----+                        |
|  +--+    +-----+                   |MEM7|                        |
|  |S2|    |CRAM1|                   +----+                        |
|  +--+    +-----+                   |MEM6|                        |
+------------------------------------+----+------------------------+

S1 is the setup button
S2 is the reset button

ROMH & ROML are all MX 29F1610MC-16 flash roms
u15A is a MX 29F1610MC-16 flash rom
u7 is a ST 27c1001
ROM1 & ROM2 are both ST 27c4000D

*/

ROM_START( wivernwg )
	ROM_REGION32_BE( 0x100000, "user1", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	ROM_LOAD( "rom1", 0x000000, 0x080000, CRC(83eb9a36) SHA1(d9c3b2facf42c137abc2923bbaeae300964ca4a0) ) /* ST 27C4000D with no labels */
	ROM_LOAD( "rom2", 0x080000, 0x080000, CRC(5d657055) SHA1(21baa81b80f28aec4a6be9eaf69709958bf2a129) )

	ROM_REGION( 0x080000, "qs1000:cpu", 0 ) /* QDSP (8052) Code */
	ROM_LOAD( "u7",  0x00000, 0x20000, CRC(00a3f705) SHA1(f0a6bafd16bea53d4c05c8cc108983cbd41e5757) ) /* ST 27C1001 with no labels */
	ROM_RELOAD(      0x20000, 0x20000 )
	ROM_RELOAD(      0x40000, 0x20000 )
	ROM_RELOAD(      0x60000, 0x20000 )

	ROM_REGION( 0x1000000, "gfx1", 0 )  /* gfx data */
	ROM_LOAD32_WORD( "roml00", 0x000000, 0x200000, CRC(fb3541b6) SHA1(4f569ac7bde92c5febf005ab73f76552421ec223) ) /* MX 29F1610MC-16 flash roms with no labels */
	ROM_LOAD32_WORD( "romh00", 0x000002, 0x200000, CRC(516aca48) SHA1(42cf5678eb4c0ee7da2ab0bd66e4e34b2735c75a) )
	ROM_LOAD32_WORD( "roml01", 0x400000, 0x200000, CRC(1c764f95) SHA1(ba6ac1376e837b491bc0269f2a1d10577a3d40cb) )
	ROM_LOAD32_WORD( "romh01", 0x400002, 0x200000, CRC(fee42c63) SHA1(a27b5cbca0defa9be85fee91dde1273f445d3372) )
	ROM_LOAD32_WORD( "roml02", 0x800000, 0x200000, CRC(fc846707) SHA1(deaee15ab71927f644dcf576959e2ceaa55bfd44) )
	ROM_LOAD32_WORD( "romh02", 0x800002, 0x200000, CRC(86141c7d) SHA1(22a82cc7d44d655b03867503a83e81f7c82d6c91) )
	ROM_LOAD32_WORD( "l03",    0xc00000, 0x200000, CRC(85aa8db8) SHA1(8ad067f92ff161683ac962ffc5391504145a3d4a) )
	ROM_LOAD32_WORD( "h03",    0xc00002, 0x200000, CRC(ade8af9f) SHA1(05cdc1b38dec9d8a86302f2de794391fd3e376a5) )

	ROM_REGION( 0x1000000, "qs1000", 0 ) /* Music data / QDSP samples (SFX) */
	ROM_LOAD( "romsnd.u15a", 0x000000, 0x200000, CRC(fc89eedc) SHA1(2ce28bdb773cfa5b5660e4c0a9ef454cb658f2da) ) /* MX 29F1610MC-16 flash rom with no label */
	ROM_LOAD( "qs1001a",     0x200000, 0x080000, CRC(d13c6407) SHA1(57b14f97c7d4f9b5d9745d3571a0b7115fbe3176) )
ROM_END

ROM_START( wyvernwg )
	ROM_REGION32_BE( 0x100000, "user1", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	ROM_LOAD( "rom1.bin", 0x000000, 0x080000, CRC(66bf3a5c) SHA1(037d5e7a6ef6f5b4ac08a9c811498c668a9d2522) ) /* ST 27c4000D with no labels */
	ROM_LOAD( "rom2.bin", 0x080000, 0x080000, CRC(fd9b5911) SHA1(a01e8c6e5a9009024af385268ba3ba90e1ebec50) )

	ROM_REGION( 0x080000, "qs1000:cpu", 0 ) /* QDSP (8052) Code */
	ROM_LOAD( "u7",  0x00000, 0x20000, CRC(00a3f705) SHA1(f0a6bafd16bea53d4c05c8cc108983cbd41e5757) ) /* ST 27C1001 with no labels */
	ROM_RELOAD(      0x20000, 0x20000 )
	ROM_RELOAD(      0x40000, 0x20000 )
	ROM_RELOAD(      0x60000, 0x20000 )

	ROM_REGION( 0x1000000, "gfx1", 0 )  /* gfx data */
	ROM_LOAD32_WORD( "roml00", 0x000000, 0x200000, CRC(fb3541b6) SHA1(4f569ac7bde92c5febf005ab73f76552421ec223) ) /* MX 29F1610MC-16 flash roms with no labels */
	ROM_LOAD32_WORD( "romh00", 0x000002, 0x200000, CRC(516aca48) SHA1(42cf5678eb4c0ee7da2ab0bd66e4e34b2735c75a) )
	ROM_LOAD32_WORD( "roml01", 0x400000, 0x200000, CRC(1c764f95) SHA1(ba6ac1376e837b491bc0269f2a1d10577a3d40cb) )
	ROM_LOAD32_WORD( "romh01", 0x400002, 0x200000, CRC(fee42c63) SHA1(a27b5cbca0defa9be85fee91dde1273f445d3372) )
	ROM_LOAD32_WORD( "roml02", 0x800000, 0x200000, CRC(fc846707) SHA1(deaee15ab71927f644dcf576959e2ceaa55bfd44) )
	ROM_LOAD32_WORD( "romh02", 0x800002, 0x200000, CRC(86141c7d) SHA1(22a82cc7d44d655b03867503a83e81f7c82d6c91) )
	ROM_LOAD32_WORD( "roml03", 0xc00000, 0x200000, CRC(b10bf37c) SHA1(6af835b1e2573f0bb2c17057e016a7aecc8fcde8) )
	ROM_LOAD32_WORD( "romh03", 0xc00002, 0x200000, CRC(e01c2a92) SHA1(f53c2db92d62f595d473b1835c46d426f0dbe6b3) )

	ROM_REGION( 0x1000000, "qs1000", 0 ) /* Music data / QDSP samples (SFX) */
	ROM_LOAD( "romsnd.u15a", 0x000000, 0x200000, CRC(fc89eedc) SHA1(2ce28bdb773cfa5b5660e4c0a9ef454cb658f2da) ) /* MX 29F1610MC-16 flash rom with no label */
	ROM_LOAD( "qs1001a",     0x200000, 0x080000, CRC(d13c6407) SHA1(57b14f97c7d4f9b5d9745d3571a0b7115fbe3176) )
ROM_END

ROM_START( wyvernwga )
	ROM_REGION32_BE( 0x100000, "user1", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	ROM_LOAD( "rom1.rom", 0x000000, 0x080000, CRC(586881fd) SHA1(d335bbd91def8fa4935eb2375c9b00471a1f40eb) ) /* ST 27c4000D with no labels */
	ROM_LOAD( "rom2.rom", 0x080000, 0x080000, CRC(938049ec) SHA1(cc10944c99ceb388dd4aafc93377c40540861d14) )

	ROM_REGION( 0x080000, "qs1000:cpu", 0 ) /* QDSP (8052) Code */
	ROM_LOAD( "u7",  0x00000, 0x20000, CRC(00a3f705) SHA1(f0a6bafd16bea53d4c05c8cc108983cbd41e5757) ) /* ST 27C1001 with no labels */
	ROM_RELOAD(      0x20000, 0x20000 )
	ROM_RELOAD(      0x40000, 0x20000 )
	ROM_RELOAD(      0x60000, 0x20000 )

	ROM_REGION( 0x1000000, "gfx1", 0 )  /* gfx data */
	ROM_LOAD32_WORD( "roml00", 0x000000, 0x200000, CRC(fb3541b6) SHA1(4f569ac7bde92c5febf005ab73f76552421ec223) ) /* MX 29F1610MC-16 flash roms with no labels */
	ROM_LOAD32_WORD( "romh00", 0x000002, 0x200000, CRC(516aca48) SHA1(42cf5678eb4c0ee7da2ab0bd66e4e34b2735c75a) )
	ROM_LOAD32_WORD( "roml01", 0x400000, 0x200000, CRC(1c764f95) SHA1(ba6ac1376e837b491bc0269f2a1d10577a3d40cb) )
	ROM_LOAD32_WORD( "romh01", 0x400002, 0x200000, CRC(fee42c63) SHA1(a27b5cbca0defa9be85fee91dde1273f445d3372) )
	ROM_LOAD32_WORD( "roml02", 0x800000, 0x200000, CRC(fc846707) SHA1(deaee15ab71927f644dcf576959e2ceaa55bfd44) )
	ROM_LOAD32_WORD( "romh02", 0x800002, 0x200000, CRC(86141c7d) SHA1(22a82cc7d44d655b03867503a83e81f7c82d6c91) )
	ROM_LOAD32_WORD( "roml03", 0xc00000, 0x200000, CRC(b10bf37c) SHA1(6af835b1e2573f0bb2c17057e016a7aecc8fcde8) )
	ROM_LOAD32_WORD( "romh03", 0xc00002, 0x200000, CRC(e01c2a92) SHA1(f53c2db92d62f595d473b1835c46d426f0dbe6b3) )

	ROM_REGION( 0x1000000, "qs1000", 0 ) /* Music data / QDSP samples (SFX) */
	ROM_LOAD( "romsnd.u15a", 0x000000, 0x200000, CRC(fc89eedc) SHA1(2ce28bdb773cfa5b5660e4c0a9ef454cb658f2da) ) /* MX 29F1610MC-16 flash rom with no label */
	ROM_LOAD( "qs1001a",     0x200000, 0x080000, CRC(d13c6407) SHA1(57b14f97c7d4f9b5d9745d3571a0b7115fbe3176) )
ROM_END

/*

Mission Craft
Sun, 2000

PCB Layout
----------

SUN2000
|---------------------------------------------|
|       |------|  SND-ROM1     ROMH00  ROMH01 |
|       |QDSP  |                              |
|       |QS1001|                              |
|DA1311A|------|  SND-ROM2                    |
|       /------\                              |
|       |QDSP  |               ROML00  ROML01 |
|       |QS1000|                              |
|  24MHz\------/                              |
|                                 |---------| |
|                                 | ACTEL   | |
|J               62256            |A40MX04-F| |
|A  *  PRG-ROM2  62256            |PL84     | |
|M   PAL                          |         | |
|M                    62256 62256 |---------| |
|A                    62256 62256             |
|             |-------|           |---------| |
|             |GMS    |           | ACTEL   | |
|  93C46      |30C2116|           |A40MX04-F| |
|             |       | 62256     |PL84     | |
|  HY5118164C |-------| 62256     |         | |
|                                 |---------| |
|SW2                                          |
|SW1                                          |
|   50MHz                              28MHz  |
|---------------------------------------------|
Notes:
      GMS30C2116 - based on Hyperstone technology, clock running at 50.000MHz
      QS1001A    - Wavetable audio chip, 1M ROM, manufactured by AdMOS (Now LG Semi.), SOP32
      QS1000     - Wavetable audio chip manufactured by AdMOS (Now LG Semi.), QFP100
                   provides Creative Waveblaster functionality and General Midi functions
      SW1        - Used to enter test mode
      SW2        - PCB Reset
      *          - Empty socket for additional program ROM

*/

ROM_START( misncrft )
	ROM_REGION16_BE( 0x100000, "user1", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	/* 0 - 0x80000 empty */
	ROM_LOAD( "prg-rom2.bin", 0x80000, 0x80000, CRC(04d22da6) SHA1(1c5be430000a31f21204fb756fadf2523a546b8b) ) /* Version 2.7 */

	ROM_REGION( 0x080000, "qs1000:cpu", 0 ) /* QDSP (8052) Code */
	ROM_LOAD( "snd-rom2.us1", 0x00000, 0x20000, CRC(8821e5b9) SHA1(4b8df97bc61b48aa16ed411614fcd7ed939cac33) )
	ROM_RELOAD(      0x20000, 0x20000 )
	ROM_RELOAD(      0x40000, 0x20000 )
	ROM_RELOAD(      0x60000, 0x20000 )

	ROM_REGION( 0x800000, "gfx1", 0 )
	ROM_LOAD32_WORD( "roml00", 0x000000, 0x200000, CRC(748c5ae5) SHA1(28005f655920e18c82eccf05c0c449dac16ee36e) )
	ROM_LOAD32_WORD( "romh00", 0x000002, 0x200000, CRC(f34ae697) SHA1(2282e3ef2d100f3eea0167b25b66b35a64ddb0f8) )
	ROM_LOAD32_WORD( "roml01", 0x400000, 0x200000, CRC(e37ece7b) SHA1(744361bb73905bc0184e6938be640d3eda4b758d) )
	ROM_LOAD32_WORD( "romh01", 0x400002, 0x200000, CRC(71fe4bc3) SHA1(08110b02707e835bf428d343d5112b153441e255) )

	ROM_REGION( 0x1000000, "qs1000", 0 )
	ROM_LOAD( "snd-rom1.u15", 0x000000, 0x80000, CRC(fb381da9) SHA1(2b1a5447ed856ab92e44d000f27a04d981e3ac52) )
	ROM_LOAD( "qs1001a.u17",  0x200000, 0x80000, CRC(d13c6407) SHA1(57b14f97c7d4f9b5d9745d3571a0b7115fbe3176) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46-eeprom-misncrft", 0x0000, 0x0080, CRC(83c813eb) SHA1(fe09ea1b4ad959c11fd904e55f7072dc12235491) )
ROM_END

ROM_START( misncrfta )
	ROM_REGION16_BE( 0x100000, "user1", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	/* 0 - 0x80000 empty */
	ROM_LOAD( "prg-rom2.bin", 0x80000, 0x80000, CRC(059ae8c1) SHA1(2c72fcf560166cb17cd8ad665beae302832d551c) ) /* Version 2.4 */

	ROM_REGION( 0x080000, "qs1000:cpu", 0 ) /* QDSP (8052) Code */
	ROM_LOAD( "snd-rom2.us1", 0x00000, 0x20000, CRC(8821e5b9) SHA1(4b8df97bc61b48aa16ed411614fcd7ed939cac33) )
	ROM_RELOAD(      0x20000, 0x20000 )
	ROM_RELOAD(      0x40000, 0x20000 )
	ROM_RELOAD(      0x60000, 0x20000 )

	ROM_REGION( 0x800000, "gfx1", 0 )
	ROM_LOAD32_WORD( "roml00", 0x000000, 0x200000, CRC(748c5ae5) SHA1(28005f655920e18c82eccf05c0c449dac16ee36e) )
	ROM_LOAD32_WORD( "romh00", 0x000002, 0x200000, CRC(f34ae697) SHA1(2282e3ef2d100f3eea0167b25b66b35a64ddb0f8) )
	ROM_LOAD32_WORD( "roml01", 0x400000, 0x200000, CRC(e37ece7b) SHA1(744361bb73905bc0184e6938be640d3eda4b758d) )
	ROM_LOAD32_WORD( "romh01", 0x400002, 0x200000, CRC(71fe4bc3) SHA1(08110b02707e835bf428d343d5112b153441e255) )

	ROM_REGION( 0x1000000, "qs1000", 0 )
	ROM_LOAD( "snd-rom1.u15", 0x000000, 0x80000, CRC(fb381da9) SHA1(2b1a5447ed856ab92e44d000f27a04d981e3ac52) )
	ROM_LOAD( "qs1001a.u17",  0x200000, 0x80000, CRC(d13c6407) SHA1(57b14f97c7d4f9b5d9745d3571a0b7115fbe3176) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46-eeprom-misncrfta", 0x0000, 0x0080, CRC(9ad27077) SHA1(7f0e98eff9cf6e1b60c19fc1016b888e50b087e0) )
ROM_END

/*

Yori Jori Kuk Kuk


PCB Layout
----------

GOLDEN BELL-002
+----------------------------------------------+
|                  CON6* CON7*                 |
|VR1  24MHz         16C550*    7.3728MHz*      |
|      QS1000 QS1001A SND2              27MHz  |
|                           MEM1L +----------+ |
|   SND5                          |QuickLogic| |
|                           MEM1U | 0152 BA  | |
|J                                | QL12X16B | |
|A                                |  XPL84C  | |
|M                           MEM2 +----------+ |
|M        CRAM2                   +----------+ |
|A        CRAM1              MEM3 |QuickLogic| |
|                                 | 0152 BA  | |
|      DRAM1                 MEM6 | QL12X16B | |
|      DRAM2  E1-32T              |  XPL84C  | |
|                            MEM7 +----------+ |
|P2 P1                93C46                    |
|              PRG1    ROML00 ROML01 L02* L03* |
|          GAL1                                |
|CON2 CON3 CON1* 50MHz ROMH00 ROMH01 H02* H03* |
+----------------------------------------------+

* Denotes unpopulated component

Notes:
CPU - Hyperstone E1-32T @ 50.000MHz

OSC - 50MHz, 27MHz, 24MHz & 7.3728MHz (unpopulated)

QDSP QS1000 @ 24MHz (silkscreened as SND1)
     QS1001A Sample rom (silkscreened as SND3)
     SND2 Additional sound samples
     SND5 8052 CPU code for QS1000?

EEPROM - Atmel 93C46 at U6

DRAM1 - Hynix GM71C18163CJ5 1M x16 EDO DRAM (SOJ44)
CRAMx - M61C256J-15 32K x8 SRAM (SOJ28)
MEMx  - M61C256J-15 32K x8 SRAM (SOJ28)

 P1 - Reset push button
 P2 - Setup push button
VR1 - Volume adjust pot

16C550 - Asynchronous Comm Element with Autoflow Conrol (all components related to the 16C500 are unpopulated)
         7.3728MHz OSC connected to XIN & XOUT of 16C550
         CON6 & CON7 connected to 16C550

CON1 - 20 pin connector (unpopulated)
CON2 - 7 pin connector silkscreened GIFT
CON3 - 6 pin connector silkscreened HOPPER
CON6 - 4 pin connector silkscreened IN (unpopulated)
CON7 - 4 pin connector silkscreened OUT (unpopulated)

ROMs:
    PRG1            - ST M27C160 16MBit DIP42 EPROM
    SND2            - ST M27C160 16MBit DIP42 EPROM
    SND5            - ST M27C1001 1MBit DIP32 EPROM
    ROML00 & ROMH00 - Macronix MX29F1610MC-12 SOP44 16MBit FlashROM
    ROML01 & ROMH01 - Macronix MX29F1610MC-12 SOP44 16MBit FlashROM
    ROML02 & ROMH02 - Unpopulated
    ROML03 & ROMH03 - Unpopulated

*/

ROM_START( yorijori )
	ROM_REGION32_BE( 0x200000, "user1", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	ROM_LOAD( "prg1", 0x000000, 0x200000, CRC(0e04eb40) SHA1(0cec9dc91aaf9cf7c459c7baac200cf0fcfddc18) )

	ROM_REGION( 0x080000, "qs1000:cpu", 0 ) /* QDSP (8052) Code */
	ROM_LOAD( "snd5", 0x00000, 0x20000, CRC(79067367) SHA1(a8f0c02dd616ff8c5fb49dea1a116fea2aced19c) )
	ROM_RELOAD(      0x20000, 0x20000 )
	ROM_RELOAD(      0x40000, 0x20000 )
	ROM_RELOAD(      0x60000, 0x20000 )

	ROM_REGION( 0x800000, "gfx1", 0 )
	ROM_LOAD32_WORD( "roml00", 0x000000, 0x200000, CRC(9299ce36) SHA1(cd8a9e2619da93e2015704230e8189a6ae52de69) )
	ROM_LOAD32_WORD( "romh00", 0x000002, 0x200000, CRC(16584ff2) SHA1(69dce8c33b246b4327b330233116c1b72a8b7e84) )
	ROM_LOAD32_WORD( "roml01", 0x400000, 0x200000, CRC(b5d1892f) SHA1(20afcd00a506ec0fd1c4fffb2d9c853c8dc61e2e) )
	ROM_LOAD32_WORD( "romh01", 0x400002, 0x200000, CRC(fe0485ef) SHA1(bd1a26aa386803df8e8e137ea5d5a2cdd6ad1197) )

	ROM_REGION( 0x1000000, "qs1000", 0 )
	ROM_LOAD( "snd2", 0x000000, 0x200000, CRC(8d9a8795) SHA1(482acb3beafc9baa43284c54ac36086c57098465) )
	ROM_LOAD( "qs1001a.snd3",  0x200000, 0x80000, CRC(d13c6407) SHA1(57b14f97c7d4f9b5d9745d3571a0b7115fbe3176) )
ROM_END

/*

Final Godori (c) SemiCom

SEMICOM-003a

+---------------------------------------------+
|                     +------+                |
|            YM3012   |  U7  |                |
| VR1                 +------+                |
|            YM2151   M6295                   |
|                                             |
|            +-----+      MEM1l  +----------+ |
|            |CRAM2|             |QuickLogic| |
|            +-----+             | QL12X16B | |
|            +-----+             | XPL84C   | |
|  +-------+ |CRAM2|      MEM1U  |          | |
|J | DRAM1 | +-----+             +----------+ |
|A +-------+ +----------+ MEM3                |
|M +-------+ |          |        +----------+ |
|M | DRAM2 | |HyperStone| MEM7   |QuickLogic| |
|A +-------+ |  E1-32T  |        | QL12X16B | |
|            |          | MEM6   | XPL84C   | |
|     PAL    +----------+        |          | |
|                         MEM2   +----------+ |
|SW1 SW2       61L256S                        |
|        ROM0*  +--------+ +--------+  28MHz  |
|        ROM1   | ROML00 | | ROMH00 |  +-----+|
|               +--------+ +--------+  |93C46||
|   50MHz         ROML01*    ROMH01*   +-----+|
|                                             |
+---------------------------------------------+

ROM1 & U7 are 27C040
ROML00 & ROMH00 are MX 29F1610MC flashroms
ROM0, ROML01 & ROMH01 are unpopulated
YM2151, YM3012 & M6295 badged as BS901, BS902 & U6295
CRAM are MCM6206BAEJ15
DRAM are KM416C1204AJ-6
MEM are MCM6206BAEJ15
61L256S - 32K x 8 bit High Speed CMOS SRAM (game's so called "Backup Data")

SW1 is the reset button
SW2 is the setup button
VR1 is the volume adjust pot

*/

ROM_START( finalgdr ) /* version 2.20.5915, Korea only */
	ROM_REGION32_BE( 0x100000, "user1", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	/* rom0 empty */
	ROM_LOAD( "rom1", 0x080000, 0x080000, CRC(45815931) SHA1(80ba7a366994e40a1f520ea18fad82e6b068b279) )

	ROM_REGION( 0x800000, "gfx1", 0 )  /* gfx data */
	ROM_LOAD32_WORD( "roml00", 0x000000, 0x200000, CRC(8334459d) SHA1(70ad560dada8aa8ce192e5307bd805744b82fcfe) )
	ROM_LOAD32_WORD( "romh00", 0x000002, 0x200000, CRC(f28578a5) SHA1(a5c7b17aff101f1f4f52657d0567a6c9d12a178d) )
	/* roml01 empty */
	/* romh01 empty */

	ROM_REGION( 0x080000, "user2", 0 ) /* Oki Samples */
	ROM_LOAD( "u7", 0x000000, 0x080000, CRC(080f61f8) SHA1(df3764b1b07f9fc38685e3706b0f834f62088727) )

	/* $00000-$20000 stays the same in all sound banks, */
	/* the second half of the bank is what gets switched */
	ROM_REGION( 0x100000, "oki", 0 ) /* Samples */
	ROM_COPY( "user2", 0x000000, 0x000000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x020000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x040000, 0x020000)
	ROM_COPY( "user2", 0x020000, 0x060000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x080000, 0x020000)
	ROM_COPY( "user2", 0x040000, 0x0a0000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x0c0000, 0x020000)
	ROM_COPY( "user2", 0x060000, 0x0e0000, 0x020000)
ROM_END

/*

Mr. Kicker (c) SemiCom

SEMICOM-003b
+---------------------------------------------+
|                     +------+                |
|            YM3012   |  U7  |                |
| VR1                 +------+                |
|            YM2151   M6295                   |
|                                             |
|            +-----+      MEM1l  +----------+ |
|            |CRAM2|             |QuickLogic| |
|            +-----+             | QL12X16B | |
|            +-----+             | XPL84C   | |
|  +-------+ |CRAM2|      MEM1U  |          | |
|J | DRAM1 | +-----+             +----------+ |
|A +-------+ +----------+ MEM3                |
|M +-------+ |          |        +----------+ |
|M | DRAM2 | |HyperStone| MEM7   |QuickLogic| |
|A +-------+ |  E1-32T  |        | QL12X16B | |
|            |          | MEM6   | XPL84C   | |
|     PAL    +----------+        |          | |
|                         MEM2   +----------+ |
|SW1 SW2       61L256S                        |
|        ROM0*  +--------+ +--------+  28MHz  |
|        ROM1   | ROML00 | | ROMH00 |  +-----+|
|               +--------+ +--------+  |93C46||
|   50MHz         ROML01*    ROMH01*   +-----+|
|                                             |
+---------------------------------------------+

ROM1 & U7 are 27C040
ROML00 & ROMH00 are MX 29F1610MC flashroms
ROM0, ROML01 & ROMH01 are unpopulated
YM2151, YM3012 & M6295 badged as U6651, U6612 & AD-65
CRAM are MCM6206BAEJ15
DRAM are KM416C1204AJ-6
MEM are MCM6206BAEJ15
61L256S - 32K x 8 bit High Speed CMOS SRAM (game's so called "Backup Data")

SW1 is the reset button
SW2 is the setup button
VR1 is the volume adjust pot


F-E1-16-010
+-----------------------------------------------+
|     VR1          YM3012  VROM1                |
|                  YM2151  M6295   ROML03 ROMU03|
|               CRAM2              ROML02 ROMU02|
|               CRAM1              ROML01 ROMU01|
|               MEM1L              ROML00 ROMU00|
|J              MEM1U                           |
|A              MEM2  +----------++----------+  |
|M                    |          ||          |  |
|M              MEM3  |Quicklogic||Quicklogic| 2|
|A                    | QL2003-  || QL2003-  | 8|
|               MEM6  | XPL84C   || XPL84C   | M|
|                     |          ||          | H|
|               MEM7  +----------++----------+ z|
|                      GAL                      |
|    93C46                       ROM1*          |
|P1 P2   50MHz E1-16T   DRAM1    ROM2           |
+-----------------------------------------------+

Notes:
CPU - Hyperstone E1-16T @ 50.000MHz

DRAM1 - LG Semi GM71C18163 1M x16 EDO DRAM (SOJ44)
CRAMx - W24M257AK-15 32K x8 SRAM (SOJ28)
MEMx  - UM61256FK-15 32K x8 SRAM (SOJ28)

Oki M6295 rebaged as AD-65
YM3012/YM2151 rebaged as BS902/KA51

 P1 - Reset push button
 P2 - Setup push button
VR1 - Volume adjust pot

ROMs:
    ROML00 & ROMH00 - Macronix MX29F1610MC-12 SOP44 16MBit FlashROM
    ROML01 & ROMH01 - Unpopulated space for MX29F1610MC-12 SOP44 16MBit FlashROM
    ROML02 & ROMH02 - Unpopulated space for MX29F1610MC-12 SOP44 16MBit FlashROM
    ROML03 & ROMH03 - Unpopulated space for MX29F1610MC-12 SOP44 16MBit FlashROM
    VROM1           - MX 27C2000 2MBit DIP32 EPROM
  * ROM1            - Unpopulated space for DIP32 EPROM (up to 4MBit)
    ROM2            - 27C040 4MBit DIP32 EPROM

*/

ROM_START( mrkickera )
	ROM_REGION32_BE( 0x100000, "user1", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	/* rom0 empty */
	ROM_LOAD( "2-semicom.rom1", 0x080000, 0x080000, CRC(d3da29ca) SHA1(b843c650096a1c6d50f99e354ec0c93eb4406c5b) ) /* SEMICOM-003b PCB */

	ROM_REGION( 0x800000, "gfx1", 0 )  /* gfx data */
	ROM_LOAD32_WORD( "roml00", 0x000000, 0x200000, CRC(c677aac3) SHA1(356073a29260e8e6c29dd12b2113b30140c6108c) )
	ROM_LOAD32_WORD( "romh00", 0x000002, 0x200000, CRC(b6337d4a) SHA1(2f46e2933af7fd0f71083900d5e6e4f602ab4c66) )
	/* roml01 empty */
	/* romh01 empty */

	ROM_REGION( 0x080000, "user2", 0 ) /* Oki Samples */
	ROM_LOAD( "at27c040.u7", 0x000000, 0x080000, CRC(e8141fcd) SHA1(256fd1987030e0a1df0a66a228c1fea996cda686) ) /* Mask ROM */

	/* $00000-$20000 stays the same in all sound banks, */
	/* the second half of the bank is what gets switched */
	ROM_REGION( 0x100000, "oki", 0 ) /* Samples */
	ROM_COPY( "user2", 0x000000, 0x000000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x020000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x040000, 0x020000)
	ROM_COPY( "user2", 0x020000, 0x060000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x080000, 0x020000)
	ROM_COPY( "user2", 0x040000, 0x0a0000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x0c0000, 0x020000)
	ROM_COPY( "user2", 0x060000, 0x0e0000, 0x020000)

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) /* Default EEPROM (it doesn't boot without and the game code crashes) (game also refuses to boot if program attempts to rewrite it, CPU bug or protection?) */
	ROM_LOAD( "eeprom-mrkicker.bin", 0x0000, 0x0080, CRC(87afb8f7) SHA1(444203b793c1d7929fc5916f18b510198719cd38) )
ROM_END

ROM_START( mrkicker )
	ROM_REGION16_BE( 0x100000, "user1", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	/* rom1 empty */
	ROM_LOAD( "3-semicom.rom2", 0x080000, 0x080000, CRC(3f7fa08b) SHA1(dbffd44d8387e6ed1a4b5ec85ccf64d69a108d88) ) /* F-E1-16-010 PCB */

	ROM_REGION( 0x800000, "gfx1", 0 )  /* gfx data */
	ROM_LOAD32_WORD( "roml00", 0x000000, 0x200000, CRC(c677aac3) SHA1(356073a29260e8e6c29dd12b2113b30140c6108c) )
	ROM_LOAD32_WORD( "romh00", 0x000002, 0x200000, CRC(b6337d4a) SHA1(2f46e2933af7fd0f71083900d5e6e4f602ab4c66) )
	/* roml01 empty */
	/* romh01 empty */
	/* roml02 empty */
	/* romh02 empty */
	/* roml03 empty */
	/* romh03 empty */

	ROM_REGION( 0x080000, "user2", 0 ) /* Oki Samples */
	ROM_LOAD( "11-semicom.vrom1", 0x000000, 0x080000, CRC(e8141fcd) SHA1(256fd1987030e0a1df0a66a228c1fea996cda686) ) /* same data as above */

	/* $00000-$20000 stays the same in all sound banks, */
	/* the second half of the bank is what gets switched */
	ROM_REGION( 0x100000, "oki", 0 ) /* Samples */
	ROM_COPY( "user2", 0x000000, 0x000000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x020000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x040000, 0x020000)
	ROM_COPY( "user2", 0x020000, 0x060000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x080000, 0x020000)
	ROM_COPY( "user2", 0x040000, 0x0a0000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x0c0000, 0x020000)
	ROM_COPY( "user2", 0x060000, 0x0e0000, 0x020000)
ROM_END

/*
Age Of Heroes - Silkroad 2
Unico, 2001

PCB Layout
----------

|----------------------------------------|
|UPC1241H  BA4558     32MHz         G05  |
|TL084  YM3012  ROM3   |--------|   G06  |
|VOL ULN2003  M6295(1) |A40MX04 |   G07  |
|       YM2151  ROM4   |PL84    |   G08  |
|     93C46   M6295(2) |        |        |
|J  3.579545MHz 62256  |--------|        |
|A     HY18CV8  62256               G09  |
|M     GAL22V10           EPM7128   G10  |
|M      |--------|20MHz             G11  |
|A      |        |                  G12  |
|       |E1-32XN |                       |
|RESET  |        | 62256            62256|
|TEST   |--------| 62256            62256|
|ROM1    HY5118164                  62256|
|ROM2    HY5118164                  62256|
|----------------------------------------|
Notes:
      E1-32XN  - Hyperstone CPU, clock input 20.000MHz (QFP160)
      A40MX04  - Actel A40MX04-F FPGA (PLCC84)
      EPM7128  - Altera MAX EPM7128TC100 CPLD (TQFP100)
      YM2151   - clock 3.579545MHz (DIP24)
      M6295(1) - clock 4.000MHz [32/8] pin 7 HIGH (QFP44)
      M6295(2) - clock 1.000MHz [32/32] pin 7 HIGH (QFP44)
      YM3012   - clock 1.7897725MHz [3.579545/2] (DIP16)
      TL084    - Texas Instruments TL084 Quad JFET-Input General-Purpose Operational Amplifier (DIP8)
      BA4558   - Rohm BA4558 Dual Operational Amplifier (DIP8)
      93C46    - 128 bytes x8 EEPROM (DIP8)
      HY5118164- Hynix 1M x16 EDO DRAM (SOJ42)
      62256    - 32k x8 SRAM (DIP28)
      ROM1/2   - Main program ROMs, type MX29F1610MC-12 (SOP44)
      ROM3/4   - M6295 Sound Data ROMs, ROM3 = 27C020, ROM4 = 27C040 (both DIP32)
      G05-G12  - GFX Data ROMs, type Intel E28F640J3A120 64M x8 FlashROM (TSOP56)
      VSync    - 59.185Hz   \
      HSync    - 15.625kHz / via EL4583 & TTi PFM1300
*/

ROM_START( aoh )
	ROM_REGION32_BE( 0x400000, "user1", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	ROM_LOAD16_WORD_SWAP( "rom1", 0x000000, 0x200000, CRC(2e55ff55) SHA1(b2b7605b87ee609dfbc7c21dfae0ef8d847019f0) )
	ROM_LOAD16_WORD_SWAP( "rom2", 0x200000, 0x200000, CRC(50f8a409) SHA1(a8171b7cf59dd01de1e512ab21607b4f330f40b8) )

	ROM_REGION( 0x4000000, "gfx1", 0 ) /* 16x16x8 Sprites */
	ROM_LOAD32_WORD( "g05", 0x0000002, 0x800000, CRC(64c8f493) SHA1(d487a74c813abbd0a612f8346eed8a7c3ff3e84e) )
	ROM_LOAD32_WORD( "g09", 0x0000000, 0x800000, CRC(c359febb) SHA1(7955385748e24dd076bc4f954b193a53c0a729c5) )
	ROM_LOAD32_WORD( "g06", 0x1000002, 0x800000, CRC(ffbc9fe5) SHA1(5e0e5cfdf6af23db0733c9fedee9c5f9ccde1109) )
	ROM_LOAD32_WORD( "g10", 0x1000000, 0x800000, CRC(08217573) SHA1(10cecdfc3a1ef835a62325b023d3bca8d0aea67d) )
	ROM_LOAD32_WORD( "g07", 0x2000002, 0x800000, CRC(5cb3c86a) SHA1(2e89f467c1a220f2510977677215e040295c3dd0) )
	ROM_LOAD32_WORD( "g11", 0x2000000, 0x800000, CRC(5f0461b8) SHA1(a0ac37d9a357e69367b8fee68bc358bfea5ecca0) )
	ROM_LOAD32_WORD( "g08", 0x3000002, 0x800000, CRC(1fd08aa0) SHA1(376a91220cd6e63418b04d590b232bb1079a40c7) )
	ROM_LOAD32_WORD( "g12", 0x3000000, 0x800000, CRC(e437b35f) SHA1(411d2926d619fba057476864f0e580f608830522) )

	ROM_REGION( 0x40000, "oki_1", 0 ) /* Oki Samples */
	ROM_LOAD( "rom3", 0x00000, 0x40000, CRC(db8cb455) SHA1(6723b4018208d554bd1bf1e0640b72d2f4f47302) )

	ROM_REGION( 0x80000, "user2", 0 ) /* Oki Samples */
	ROM_LOAD( "rom4", 0x00000, 0x80000, CRC(bba47755) SHA1(e6eeb5f64eaa88a74536119b731a76921e79f8ff) )

	/* $00000-$20000 stays the same in all sound banks, */
	/* the second half of the bank is what gets switched */
	ROM_REGION( 0x100000, "oki_2", 0 ) /* Samples */
	ROM_COPY( "user2", 0x000000, 0x000000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x020000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x040000, 0x020000)
	ROM_COPY( "user2", 0x020000, 0x060000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x080000, 0x020000)
	ROM_COPY( "user2", 0x040000, 0x0a0000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x0c0000, 0x020000)
	ROM_COPY( "user2", 0x060000, 0x0e0000, 0x020000)
ROM_END

/*

Boong-Ga Boong-Ga (Spank'em!)
Taff System, 2001

*/

ROM_START( boonggab )
	ROM_REGION16_BE( 0x100000, "user1", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	/* rom2 empty */
	/* rom3 empty */
	ROM_LOAD( "2.rom0",       0x80000, 0x80000, CRC(3395541b) SHA1(4e822a52d6070bde232285e7ad8fbe74594bbf28) )
	ROM_LOAD( "1.rom1",       0x00000, 0x80000, CRC(50522da1) SHA1(28f92fc818513d7a4934b9f8e5d39243d720cc80) )

	ROM_REGION( 0x2000000, "gfx1", ROMREGION_ERASE00 )  /* 16x16x8 Sprites */
	ROM_LOAD32_WORD( "boong-ga.roml00", 0x0000000, 0x200000, CRC(18be5f92) SHA1(abccc578e5e9652a7829165b485776671938b9d9) )
	ROM_LOAD32_WORD( "boong-ga.romu00", 0x0000002, 0x200000, CRC(0158ba9e) SHA1(b6cb699f0779b26d578043c42a0ce14a59fd8ac5) )
	ROM_LOAD32_WORD( "boong-ga.roml05", 0x0400000, 0x200000, CRC(76d60553) SHA1(13a47aed2e7213be98e55a938887a3c2fb314fbe) )
	ROM_LOAD32_WORD( "boong-ga.romu05", 0x0400002, 0x200000, CRC(35ee8fb5) SHA1(79bd1775087bfaf7624978cec4e912553ca1f027) )
	ROM_LOAD32_WORD( "boong-ga.roml01", 0x0800000, 0x200000, CRC(636e9d5d) SHA1(d478ec905d6e56e4c46889430d8c32de98e9dc14) )
	ROM_LOAD32_WORD( "boong-ga.romu01", 0x0800002, 0x200000, CRC(b8dcf6b7) SHA1(8ea590f92832e6b6a4c27fb1f2aa18bb000f41e0) )
	ROM_LOAD32_WORD( "boong-ga.roml06", 0x0c00000, 0x200000, CRC(8dc521b7) SHA1(37021bb05a582b80a4883bddf677c1d41e6777d2) )
	ROM_LOAD32_WORD( "boong-ga.romu06", 0x0c00002, 0x200000, CRC(f6b83270) SHA1(7971fdb99987ac701c76958a626b0cb75ba31451) )
	ROM_LOAD32_WORD( "boong-ga.roml02", 0x1000000, 0x200000, CRC(d0661c69) SHA1(94f95df19b448565642db8c5aafb2532c0febc37) )
	ROM_LOAD32_WORD( "boong-ga.romu02", 0x1000002, 0x200000, CRC(eac01eb8) SHA1(c730078d8422d566378d6a4b0deb42d2814f0dab) )
	ROM_LOAD32_WORD( "boong-ga.roml07", 0x1400000, 0x200000, CRC(3301813a) SHA1(61997f07ca516eb77c9d9478b42950fd6fc42ac5) )
	ROM_LOAD32_WORD( "boong-ga.romu07", 0x1400002, 0x200000, CRC(3f1c3682) SHA1(969491b0d3be054ddc199db2ced38c76c8f561ee) )
	ROM_LOAD32_WORD( "boong-ga.roml03", 0x1800000, 0x200000, CRC(4d4260b3) SHA1(11a5d0b472b783094d44a5c931ee1cbe816b2a05) )
	ROM_LOAD32_WORD( "boong-ga.romu03", 0x1800002, 0x200000, CRC(4ba00032) SHA1(de9e0640e80204f4906576b20eeaa17f03694b3f) )
	/* roml08 empty */
	/* romu08 empty */
	/* roml04 empty */
	/* romu04 empty */
	/* roml09 empty */
	/* romu09 empty */

	ROM_REGION( 0x100000, "user2", 0 ) /* Oki Samples */
	ROM_LOAD( "3.vrom1",      0x00000, 0x80000, CRC(0696bfcb) SHA1(bba61f3cae23271215bbbf8214ce3b73459d5da5) )
	ROM_LOAD( "4.vrom2",      0x80000, 0x80000, CRC(305c2b16) SHA1(fa199c4cd4ebb952d934e3863fca8740eeba9294) )

	/* $00000-$20000 stays the same in all sound banks, */
	/* the second half of the bank is what gets switched */
	ROM_REGION( 0x200000, "oki", 0 ) /* Samples */
	ROM_COPY( "user2", 0x000000, 0x000000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x020000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x040000, 0x020000)
	ROM_COPY( "user2", 0x020000, 0x060000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x080000, 0x020000)
	ROM_COPY( "user2", 0x040000, 0x0a0000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x0c0000, 0x020000)
	ROM_COPY( "user2", 0x060000, 0x0e0000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x100000, 0x020000)
	ROM_COPY( "user2", 0x080000, 0x120000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x140000, 0x020000)
	ROM_COPY( "user2", 0x0a0000, 0x160000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x180000, 0x020000)
	ROM_COPY( "user2", 0x0c0000, 0x1a0000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x1c0000, 0x020000)
	ROM_COPY( "user2", 0x0e0000, 0x1e0000, 0x020000)
ROM_END

static int irq_active(address_space &space)
{
	UINT32 FCR = space.device().state().state_int(27);
	if( !(FCR&(1<<29)) ) // int 2 (irq 4)
		return 1;
	else
		return 0;
}

READ16_MEMBER(vamphalf_state::vamphalf_speedup_r)
{
	if(space.device().safe_pc() == 0x82de)
	{
		if(irq_active(space))
			space.device().execute().spin_until_interrupt();
		else
			space.device().execute().eat_cycles(50);
	}

	return m_wram[(0x4a840/2)+offset];
}

READ16_MEMBER(vamphalf_state::vamphafk_speedup_r)
{
	if(space.device().safe_pc() == 0x82de)
	{
		if(irq_active(space))
			space.device().execute().spin_until_interrupt();
		else
			space.device().execute().eat_cycles(50);
	}

	return m_wram[(0x4a6d0/2)+offset];
}

READ16_MEMBER(vamphalf_state::misncrft_speedup_r)
{
	if(space.device().safe_pc() == 0xff4c)
	{
		if(irq_active(space))
			space.device().execute().spin_until_interrupt();
		else
			space.device().execute().eat_cycles(50);
	}

	return m_wram[(0x74270/2)+offset];
}

READ16_MEMBER(vamphalf_state::misncrfta_speedup_r)
{
	if(space.device().safe_pc() == 0xecc8)
	{
		if(irq_active(space))
			space.device().execute().spin_until_interrupt();
		else
			space.device().execute().eat_cycles(50);
	}

	return m_wram[(0x72eb4/2)+offset];
}


READ16_MEMBER(vamphalf_state::coolmini_speedup_r)
{
	if(space.device().safe_pc() == 0x75f7a)
	{
		if(irq_active(space))
			space.device().execute().spin_until_interrupt();
		else
			space.device().execute().eat_cycles(50);
	}

	return m_wram[(0xd2e80/2)+offset];
}

READ16_MEMBER(vamphalf_state::suplup_speedup_r)
{
	if(space.device().safe_pc() == 0xaf18a )
	{
		if(irq_active(space))
			space.device().execute().spin_until_interrupt();
		else
			space.device().execute().eat_cycles(50);
	}

	return m_wram[(0x11605c/2)+offset];
}

READ16_MEMBER(vamphalf_state::luplup_speedup_r)
{
	if(space.device().safe_pc() == 0xaefac )
	{
		if(irq_active(space))
			space.device().execute().spin_until_interrupt();
		else
			space.device().execute().eat_cycles(50);
	}

	return m_wram[(0x115e84/2)+offset];
}

READ16_MEMBER(vamphalf_state::luplup29_speedup_r)
{
	if(space.device().safe_pc() == 0xae6c0 )
	{
		if(irq_active(space))
			space.device().execute().spin_until_interrupt();
		else
			space.device().execute().eat_cycles(50);
	}

	return m_wram[(0x113f08/2)+offset];
}

READ16_MEMBER(vamphalf_state::puzlbang_speedup_r)
{
	if(space.device().safe_pc() == 0xae6d2 )
	{
		if(irq_active(space))
			space.device().execute().spin_until_interrupt();
		else
			space.device().execute().eat_cycles(50);
	}

	return m_wram[(0x113f14/2)+offset];
}

READ16_MEMBER(vamphalf_state::puzlbanga_speedup_r)
{
	if(space.device().safe_pc() == 0xae6d2 )
	{
		if(irq_active(space))
			space.device().execute().spin_until_interrupt();
		else
			space.device().execute().eat_cycles(50);
	}

	return m_wram[(0x113ecc/2)+offset];
}

READ32_MEMBER(vamphalf_state::wivernwg_speedup_r)
{
	if(space.device().safe_pc() == 0x10758 )
	{
		if(irq_active(space))
			space.device().execute().spin_until_interrupt();
		else
			space.device().execute().eat_cycles(50);
	}

	return m_wram32[0x00b4ccc/4];
}

READ32_MEMBER(vamphalf_state::wyvernwg_speedup_r)
{
	if(space.device().safe_pc() == 0x10758 )
	{
		if(irq_active(space))
			space.device().execute().spin_until_interrupt();
		else
			space.device().execute().eat_cycles(50);
	}

	return m_wram32[0x00b56fc/4];
}

READ32_MEMBER(vamphalf_state::wyvernwga_speedup_r)
{
	if(space.device().safe_pc() == 0x10758 )
	{
		if(irq_active(space))
			space.device().execute().spin_until_interrupt();
		else
			space.device().execute().eat_cycles(50);
	}

	return m_wram32[0x00b74f8/4];
}

READ32_MEMBER(vamphalf_state::finalgdr_speedup_r)
{
	if(space.device().safe_pc() == 0x1c212)
	{
		if(irq_active(space))
			space.device().execute().spin_until_interrupt();
		else
			space.device().execute().eat_cycles(50);
	}

	return m_wram32[0x005e874/4];
}

READ32_MEMBER(vamphalf_state::mrkickera_speedup_r)
{
	UINT32 pc = space.device().safe_pc();
	if(pc == 0x469de || pc == 0x46a36)
	{
//      if(irq_active(space))
//          space.device().execute().spin_until_interrupt();
//      else
			space.device().execute().eat_cycles(50);
	}

	return m_wram32[0x00701a4/4];
}

READ16_MEMBER(vamphalf_state::mrkicker_speedup_r)
{
	UINT32 pc = space.device().safe_pc();
	if(pc == 0x41ec6)
	{
		space.device().execute().eat_cycles(50);
	}

	return m_wram[0x00063fc0/2];
}


READ16_MEMBER(vamphalf_state::dquizgo2_speedup_r)
{
	if(space.device().safe_pc() == 0xaa622)
	{
		if(irq_active(space))
			space.device().execute().spin_until_interrupt();
		else
			space.device().execute().eat_cycles(50);
	}

	return m_wram[(0xcde70/2)+offset];
}

READ32_MEMBER(vamphalf_state::aoh_speedup_r)
{
	if(space.device().safe_pc() == 0xb994 )
	{
		space.device().execute().eat_cycles(500);
	}
	else if (space.device().safe_pc() == 0xba40 )
	{
		space.device().execute().eat_cycles(500);
	}


	return m_wram32[0x28a09c/4];
}

READ16_MEMBER(vamphalf_state::jmpbreak_speedup_r)
{
	if(space.device().safe_pc() == 0x983c)
	{
		if(irq_active(space))
			space.device().execute().spin_until_interrupt();
		else
			space.device().execute().eat_cycles(50);
	}

	return m_wram[(0x00906fc / 2)+offset];
}

READ16_MEMBER(vamphalf_state::mrdig_speedup_r)
{
	if(space.device().safe_pc() == 0x1710)
	{
		if(irq_active(space))
			space.device().execute().spin_until_interrupt();
		else
			space.device().execute().eat_cycles(50);
	}

	return m_wram[(0x00a99c / 2)+offset];
}

READ16_MEMBER(vamphalf_state::dtfamily_speedup_r)
{
	if(space.device().safe_pc() == 0x12fa6 )
		space.device().execute().spin_until_interrupt();

	return m_wram[0xcc2a8 / 2];

}

READ16_MEMBER(vamphalf_state::toyland_speedup_r)
{
	if (space.device().safe_pc() == 0x130c2)
		space.device().execute().spin_until_interrupt();

	return m_wram[0x780d8 / 2];

}

READ16_MEMBER(vamphalf_state::boonggab_speedup_r)
{
	if(space.device().safe_pc() == 0x13198)
	{
		if(irq_active(space))
			space.device().execute().spin_until_interrupt();
	}

	return m_wram[(0xf1b7c / 2)+offset];
}

DRIVER_INIT_MEMBER(vamphalf_state,vamphalf)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x0004a840, 0x0004a843, read16_delegate(FUNC(vamphalf_state::vamphalf_speedup_r), this));

	m_palshift = 0;
	m_flip_bit = 0x80;
}

DRIVER_INIT_MEMBER(vamphalf_state,vamphafk)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x0004a6d0, 0x0004a6d3, read16_delegate(FUNC(vamphalf_state::vamphafk_speedup_r), this));

	m_palshift = 0;
	m_flip_bit = 0x80;
}

DRIVER_INIT_MEMBER(vamphalf_state,misncrft)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x00074270, 0x00074273, read16_delegate(FUNC(vamphalf_state::misncrft_speedup_r), this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x00072eb4, 0x00072eb7, read16_delegate(FUNC(vamphalf_state::misncrfta_speedup_r), this));
	m_palshift = 0;
	m_flip_bit = 1;

	// Configure the QS1000 ROM banking. Care must be taken not to overlap the 256b internal RAM
	machine().device("qs1000:cpu")->memory().space(AS_IO).install_read_bank(0x0100, 0xffff, "data");
	membank("qs1000:data")->configure_entries(0, 16, memregion("qs1000:cpu")->base()+0x100, 0x8000-0x100);

	save_item(NAME(m_qs1000_data));
}

DRIVER_INIT_MEMBER(vamphalf_state,coolmini)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x000d2e80, 0x000d2e83, read16_delegate(FUNC(vamphalf_state::coolmini_speedup_r), this));

	m_palshift = 0;
	m_flip_bit = 1;
}

DRIVER_INIT_MEMBER(vamphalf_state,mrkicker)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x00063fc0, 0x00063fc1, read16_delegate(FUNC(vamphalf_state::mrkicker_speedup_r), this));
	m_maincpu->space(AS_IO).install_write_handler(0x002, 0x003, write16_delegate(FUNC(vamphalf_state::mrkicker_oki_bank_w), this));

	m_palshift = 0;
	m_flip_bit = 1;
}

DRIVER_INIT_MEMBER(vamphalf_state,suplup)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x0011605c, 0x0011605f, read16_delegate(FUNC(vamphalf_state::suplup_speedup_r), this));

	m_palshift = 8;
	/* no flipscreen */
}

DRIVER_INIT_MEMBER(vamphalf_state,luplup)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x00115e84, 0x00115e87, read16_delegate(FUNC(vamphalf_state::luplup_speedup_r), this));

	m_palshift = 8;
	/* no flipscreen */
}

DRIVER_INIT_MEMBER(vamphalf_state,luplup29)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x00113f08, 0x00113f0b, read16_delegate(FUNC(vamphalf_state::luplup29_speedup_r), this));

	m_palshift = 8;
	/* no flipscreen */
}

DRIVER_INIT_MEMBER(vamphalf_state,puzlbang)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x00113f14, 0x00113f17, read16_delegate(FUNC(vamphalf_state::puzlbang_speedup_r), this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x00113ecc, 0x00113ecf, read16_delegate(FUNC(vamphalf_state::puzlbanga_speedup_r), this));

	m_palshift = 8;
	/* no flipscreen */
}

DRIVER_INIT_MEMBER(vamphalf_state,wyvernwg)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x00b4ccc, 0x00b4ccf, read32_delegate(FUNC(vamphalf_state::wivernwg_speedup_r), this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x00b56fc, 0x00b56ff, read32_delegate(FUNC(vamphalf_state::wyvernwg_speedup_r), this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x00b74f8, 0x00b74fb, read32_delegate(FUNC(vamphalf_state::wyvernwga_speedup_r), this));
	m_palshift = 0;
	m_flip_bit = 1;

	m_semicom_prot_idx = 8;
	m_semicom_prot_data[0] = 2;
	m_semicom_prot_data[1] = 1;

	// Configure the QS1000 ROM banking. Care must be taken not to overlap the 256b internal RAM
	machine().device("qs1000:cpu")->memory().space(AS_IO).install_read_bank(0x0100, 0xffff, "data");
	membank("qs1000:data")->configure_entries(0, 16, memregion("qs1000:cpu")->base()+0x100, 0x8000-0x100);

	save_item(NAME(m_qs1000_data));
	save_item(NAME(m_semicom_prot_idx));
	save_item(NAME(m_semicom_prot_which));
}

DRIVER_INIT_MEMBER(vamphalf_state,yorijori)
{
	// seems close to Final Godori in terms of port mappings, possibly a SemiCom game?

	m_palshift = 0;
	m_flip_bit = 1;

	m_semicom_prot_idx = 8;
	m_semicom_prot_data[0] = 2;
	m_semicom_prot_data[1] = 1;

//  UINT8 *romx = (UINT8 *)memregion("user1")->base();
	// prevent code dying after a trap 33 by patching it out, why?
//  romx[BYTE4_XOR_BE(0x8ff0)] = 3;
//  romx[BYTE4_XOR_BE(0x8ff1)] = 0;

	// Configure the QS1000 ROM banking. Care must be taken not to overlap the 256b internal RAM
	machine().device("qs1000:cpu")->memory().space(AS_IO).install_read_bank(0x0100, 0xffff, "data");
	membank("qs1000:data")->configure_entries(0, 16, memregion("qs1000:cpu")->base()+0x100, 0x8000-0x100);

	save_item(NAME(m_qs1000_data));
}

DRIVER_INIT_MEMBER(vamphalf_state,finalgdr)
{
	m_finalgdr_backupram_bank = 1;
	m_finalgdr_backupram = auto_alloc_array(machine(), UINT8, 0x80*0x100);
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x005e874, 0x005e877, read32_delegate(FUNC(vamphalf_state::finalgdr_speedup_r), this));
	machine().device<nvram_device>("nvram")->set_base(m_finalgdr_backupram, 0x80*0x100);

	m_palshift = 0;
	m_flip_bit = 1; //?

	m_semicom_prot_idx = 8;
	m_semicom_prot_data[0] = 2;
	m_semicom_prot_data[1] = 3;

	save_item(NAME(m_finalgdr_backupram_bank));
	save_pointer(NAME(m_finalgdr_backupram), 0x80*0x100);
	save_item(NAME(m_semicom_prot_idx));
	save_item(NAME(m_semicom_prot_which));
}

DRIVER_INIT_MEMBER(vamphalf_state,mrkickera)
{
	// backup ram isn't used
	m_finalgdr_backupram_bank = 1;
	m_finalgdr_backupram = auto_alloc_array(machine(), UINT8, 0x80*0x100);
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x00701a4, 0x00701a7, read32_delegate(FUNC(vamphalf_state::mrkickera_speedup_r), this));
	machine().device<nvram_device>("nvram")->set_base(m_finalgdr_backupram, 0x80*0x100);

	m_palshift = 0;
	m_flip_bit = 1; //?

	m_semicom_prot_idx = 8;
	m_semicom_prot_data[0] = 2;
	m_semicom_prot_data[1] = 3;

	save_item(NAME(m_semicom_prot_idx));
	save_item(NAME(m_semicom_prot_which));
}

DRIVER_INIT_MEMBER(vamphalf_state,dquizgo2)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x00cde70, 0x00cde73, read16_delegate(FUNC(vamphalf_state::dquizgo2_speedup_r), this));

	m_palshift = 0;
	m_flip_bit = 1;
}

DRIVER_INIT_MEMBER(vamphalf_state,dtfamily)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xcc2a8, 0xcc2a9, read16_delegate(FUNC(vamphalf_state::dtfamily_speedup_r), this));

	m_palshift = 0;
	m_flip_bit = 1;
}


DRIVER_INIT_MEMBER(vamphalf_state,toyland)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x780d8, 0x780d9, read16_delegate(FUNC(vamphalf_state::toyland_speedup_r), this));

	m_palshift = 0;
	m_flip_bit = 1;
}

DRIVER_INIT_MEMBER(vamphalf_state,aoh)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x028a09c, 0x028a09f, read32_delegate(FUNC(vamphalf_state::aoh_speedup_r), this));

	m_palshift = 0;
	/* no flipscreen */
}

DRIVER_INIT_MEMBER(vamphalf_state,jmpbreak)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x00906fc, 0x00906ff, read16_delegate(FUNC(vamphalf_state::jmpbreak_speedup_r), this));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xe0000000, 0xe0000003, write16_delegate(FUNC(vamphalf_state::jmpbreak_flipscreen_w), this));

	m_palshift = 0;
}

DRIVER_INIT_MEMBER(vamphalf_state,mrdig)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x00a99c, 0x00a99f, read16_delegate(FUNC(vamphalf_state::mrdig_speedup_r), this));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xe0000000, 0xe0000003, write16_delegate(FUNC(vamphalf_state::jmpbreak_flipscreen_w), this));

	m_palshift = 0;
}


DRIVER_INIT_MEMBER(vamphalf_state,boonggab)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x000f1b7c, 0x000f1b7f, read16_delegate(FUNC(vamphalf_state::boonggab_speedup_r), this));

	m_palshift = 0;
	m_has_extra_gfx = 1;
	m_flip_bit = 1;
}

GAME( 1999, coolmini,  0,        coolmini, common,   vamphalf_state, coolmini, ROT0,   "SemiCom",           "Cool Minigame Collection", GAME_SUPPORTS_SAVE )
GAME( 1999, jmpbreak,  0,        jmpbreak, common,   vamphalf_state, jmpbreak, ROT0,   "F2 System",         "Jumping Break" , GAME_SUPPORTS_SAVE )
GAME( 1999, suplup,    0,        suplup,   common,   vamphalf_state, suplup,   ROT0,   "Omega System",      "Super Lup Lup Puzzle / Zhuan Zhuan Puzzle (version 4.0 / 990518)" , GAME_SUPPORTS_SAVE )
GAME( 1999, luplup,    suplup,   suplup,   common,   vamphalf_state, luplup,   ROT0,   "Omega System",      "Lup Lup Puzzle / Zhuan Zhuan Puzzle (version 3.0 / 990128)", GAME_SUPPORTS_SAVE )
GAME( 1999, luplup29,  suplup,   suplup,   common,   vamphalf_state, luplup29, ROT0,   "Omega System",      "Lup Lup Puzzle / Zhuan Zhuan Puzzle (version 2.9 / 990108)", GAME_SUPPORTS_SAVE )
GAME( 1999, puzlbang,  suplup,   suplup,   common,   vamphalf_state, puzlbang, ROT0,   "Omega System",      "Puzzle Bang Bang (Korea, version 2.9 / 990108)", GAME_SUPPORTS_SAVE )
GAME( 1999, puzlbanga, suplup,   suplup,   common,   vamphalf_state, puzlbang, ROT0,   "Omega System",      "Puzzle Bang Bang (Korea, version 2.8 / 990106)", GAME_SUPPORTS_SAVE )
GAME( 1999, vamphalf,  0,        vamphalf, common,   vamphalf_state, vamphalf, ROT0,   "Danbi / F2 System", "Vamf x1/2 (Europe)", GAME_SUPPORTS_SAVE )
GAME( 1999, vamphalfk, vamphalf, vamphalf, common,   vamphalf_state, vamphafk, ROT0,   "Danbi / F2 System", "Vamp x1/2 (Korea)", GAME_SUPPORTS_SAVE )
GAME( 2000, dquizgo2,  0,        coolmini, common,   vamphalf_state, dquizgo2, ROT0,   "SemiCom",           "Date Quiz Go Go Episode 2" , GAME_SUPPORTS_SAVE )
GAME( 2000, misncrft,  0,        misncrft, common,   vamphalf_state, misncrft, ROT90,  "Sun",               "Mission Craft (version 2.7)", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
GAME( 2000, misncrfta, misncrft, misncrft, common,   vamphalf_state, misncrft, ROT90,  "Sun",               "Mission Craft (version 2.4)", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
GAME( 2000, mrdig,     0,        mrdig,    common,   vamphalf_state, mrdig,    ROT0,   "Sun",               "Mr. Dig", GAME_SUPPORTS_SAVE )
GAME( 2001, dtfamily,  0,        coolmini, common,   vamphalf_state, dtfamily, ROT0,   "SemiCom",           "Diet Family", GAME_SUPPORTS_SAVE )
GAME( 2001, finalgdr,  0,        finalgdr, finalgdr, vamphalf_state, finalgdr, ROT0,   "SemiCom",           "Final Godori (Korea, version 2.20.5915)", GAME_SUPPORTS_SAVE )
GAME( 2001, mrkicker,  0,        coolmini, common,   vamphalf_state, mrkicker,ROT0,    "SemiCom",           "Mr. Kicker (F-E1-16-010 PCB)", GAME_SUPPORTS_SAVE )
GAME( 2001, mrkickera, mrkicker, mrkickera,finalgdr, vamphalf_state, mrkickera, ROT0,  "SemiCom",           "Mr. Kicker (SEMICOM-003b PCB)", GAME_SUPPORTS_SAVE | GAME_NOT_WORKING ) // if you allow eeprom saving works then this set corrupts the eeprom and then won't boot
GAME( 2001, toyland,   0,        coolmini, common,   vamphalf_state, toyland,  ROT0,   "SemiCom",           "Toy Land Adventure", GAME_SUPPORTS_SAVE )
GAME( 2001, wivernwg,  0,        wyvernwg, common,   vamphalf_state, wyvernwg, ROT270, "SemiCom",           "Wivern Wings", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
GAME( 2001, wyvernwg,  wivernwg, wyvernwg, common,   vamphalf_state, wyvernwg, ROT270, "SemiCom (Game Vision license)", "Wyvern Wings (set 1)", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
GAME( 2001, wyvernwga, wivernwg, wyvernwg, common,   vamphalf_state, wyvernwg, ROT270, "SemiCom (Game Vision license)", "Wyvern Wings (set 2)", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
GAME( 2001, aoh,       0,        aoh,      aoh,      vamphalf_state, aoh,      ROT0,   "Unico",             "Age Of Heroes - Silkroad 2 (v0.63 - 2001/02/07)", GAME_SUPPORTS_SAVE )
GAME( 2001, boonggab,  0,        boonggab, boonggab, vamphalf_state, boonggab, ROT270, "Taff System",       "Boong-Ga Boong-Ga (Spank'em!)", GAME_SUPPORTS_SAVE )
GAME( 199?, yorijori,  0,        yorijori, common,   vamphalf_state, yorijori, ROT0,   "Golden Bell Entertainment",         "Yori Jori Kuk Kuk", GAME_NOT_WORKING | GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
