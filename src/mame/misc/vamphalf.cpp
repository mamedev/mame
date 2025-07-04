// license:BSD-3-Clause
// copyright-holders:Angelo Salese, David Haywood, Pierpaolo Prazzoli, Tomasz Slanina
/********************************************************************

 Driver for common Hyperstone based games

 by bits from Angelo Salese, David Haywood,
    Pierpaolo Prazzoli and Tomasz Slanina

 Games Supported:


    Minigame Cool Collection        (c) 1999 SemiCom
    Jumping Break                   (c) 1999 F2 System
    Poosho Poosho                   (c) 1999 F2 System
    New Cross Pang                  (c) 1999 F2 System
    Solitaire                       (c) 1999 F2 System          (version 2.5)
    World Adventure                 (c) 1999 F2 System + Logic
    Lup Lup Puzzle                  (c) 1999 Omega System       (version 3.0, 2.9 and 1.05)
    Puzzle Bang Bang                (c) 1999 Omega System       (version 2.8 and 2.9)
    Super Lup Lup Puzzle            (c) 1999 Omega System       (version 4.0)
    Vamf 1/2                        (c) 1999 Danbi & F2 System  (Europe version 1.1.0908 and 1.0.0903)
    Vamp 1/2                        (c) 1999 Danbi & F2 System  (Korea version 1.1.0908)
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

 Mr Kicker: Doesn't boot without a valid default EEPROM, but no longer seems to fail
            after you get a high score (since EEPROM rewrite).

 Boong-Ga Boong-Ga: the test mode is usable with a standard input configuration like the "common" one

 The Semicom boards (at least) have a strange visible area, with the display output cutting off 4 lines
 before you'd expect.  It has been confirmed on real hardware that these lines are simply never output,
 no amount of stretching the screen renders them as visible.  The games are also programmed around this
 assumption in many places, with visible sprite clipping issues at screen edges otherwise.  This does
 result in some graphics also being cut off slightly, but that is correct.

 Undumped Semicom games on similar hardware:
   Red Wyvern - A semi-sequel or update?
   Choice III: Joker's Dream (c) 2001 (likely SEMICOM-003 hardware)

TODO:
- boonggab: simulate photo sensors with a "stroke strength"
- boonggab: what are sensors bit used for? are they used in the japanese version?
- wyvernsg: fails a protection check after ~1 hour of play?
- are CRTC parameters software-configurable?

*********************************************************************/

#include "emu.h"

#include "cpu/e132xs/e132xs.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/eepromser.h"
#include "machine/gen_latch.h"
#include "machine/nvram.h"
#include "sound/okim6295.h"
#include "sound/qs1000.h"
#include "sound/ymopm.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class vamphalf_state : public driver_device
{
public:
	vamphalf_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_wram(*this,"wram")
		, m_wram32(*this,"wram32")
		, m_qs1000_bank(*this, "qs1000_bank")
		, m_okibank(*this,"okibank")
		, m_palette(*this, "palette")
		, m_soundlatch(*this, "soundlatch")
		, m_eeprom(*this, "eeprom")
		, m_gfxdecode(*this, "gfxdecode")
		, m_tiles(*this,"tiles", 0x40000, ENDIANNESS_BIG)
		, m_okiregion(*this, "oki%u", 1)
		, m_photosensors(*this, "PHOTO_SENSORS")
		, m_has_extra_gfx(false)
	{
	}

	void common(machine_config &config);
	void sound_ym_oki(machine_config &config);
	void sound_ym_banked_oki(machine_config &config);
	void sound_suplup(machine_config &config);
	void sound_qs1000(machine_config &config);
	void mrdig(machine_config &config);
	void suplup(machine_config &config);
	void vamphalf(machine_config &config);
	void boonggab(machine_config &config);
	void jmpbreak(machine_config &config);
	void newxpang(machine_config &config);
	void worldadv(machine_config &config);
	void aoh(machine_config &config);
	void coolmini(machine_config &config);
	void mrkicker(machine_config &config);
	void solitaire(machine_config &config);

	void init_vamphalf();
	void init_vamphalfr1();
	void init_vamphafk();
	void init_coolmini();
	void init_coolminii();
	void init_mrdig();
	void init_jmpbreak();
	void init_jmpbreaka();
	void init_poosho();
	void init_newxpang();
	void init_newxpanga();
	void init_worldadv();
	void init_dtfamily();
	void init_dquizgo2();
	void init_suplup();
	void init_luplup();
	void init_luplup29();
	void init_luplup10();
	void init_puzlbang();
	void init_toyland();
	void init_aoh();
	void init_boonggab();
	void init_mrkicker();
	void init_solitaire();

	ioport_value boonggab_photo_sensors_r();

	u16 eeprom_r();
	void eeprom_w(u16 data);
	void flipscreen_w(u16 data);
	u16 vram_r(offs_t offset) { return m_tiles[offset]; }
	void vram_w(offs_t offset, u16 data, u16 mem_mask = ~0) { COMBINE_DATA(&m_tiles[offset]); }

	void banked_oki(int chip);

	void common_map(address_map &map) ATTR_COLD;
	void common_32bit_map(address_map &map) ATTR_COLD;

	static constexpr u16 HTOTAL = 448;
	static constexpr u16 HBEND = 31;
	static constexpr u16 HBSTART = 351;
	static constexpr u16 VTOTAL = 264;
	static constexpr u16 VBEND = 16;
	static constexpr u16 VBSTART = 252;

protected:
	virtual void video_start() override ATTR_COLD;

	u16 m_flip_bit;
	u8 m_palshift;

	required_device<cpu_device> m_maincpu;
	optional_shared_ptr<u16> m_wram;
	optional_shared_ptr<u32> m_wram32;

	memory_bank_creator m_qs1000_bank;

	u16 m_semicom_prot_data[2];
	int m_semicom_prot_idx;
	int m_semicom_prot_which;

	bool irq_active();

	optional_memory_bank m_okibank;
	required_device<palette_device> m_palette;
	optional_device<generic_latch_8_device> m_soundlatch;
	required_device<eeprom_serial_93cxx_device> m_eeprom;

	u32 finalgdr_prot_r();
	void finalgdr_prot_w(u32 data);

	template <u32 Pc, u32 Wram_offs> u16 speedup_16_r();
	template <u32 Pc, u32 Wram_offs> u32 speedup_32_r();

private:
	required_device<gfxdecode_device> m_gfxdecode;

	memory_share_creator<u16> m_tiles;

	optional_memory_region_array<2> m_okiregion;

	optional_ioport m_photosensors;

	// driver init configuration
	bool m_has_extra_gfx;
	bool m_flipscreen;

	void jmpbreak_flipscreen_w(u16 data);
	void boonggab_prize_w(u16 data);
	void boonggab_lamps_w(offs_t offset, u16 data);

	u32 aoh_speedup_r();

	void boonggab_oki_bank_w(u16 data);
	void mrkicker_oki_bank_w(u16 data);
	void qs1000_p3_w(u8 data);

	u32 screen_update_common(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_aoh(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites_aoh(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void handle_flipped_visible_area(screen_device &screen);

	void vamphalf_io(address_map &map) ATTR_COLD;
	void coolmini_io(address_map &map) ATTR_COLD;
	void mrkicker_io(address_map &map) ATTR_COLD;
	void suplup_io(address_map &map) ATTR_COLD;
	void jmpbreak_io(address_map &map) ATTR_COLD;
	void worldadv_io(address_map &map) ATTR_COLD;
	void solitaire_io(address_map &map) ATTR_COLD;
	void mrdig_io(address_map &map) ATTR_COLD;
	void aoh_map(address_map &map) ATTR_COLD;
	void aoh_io(address_map &map) ATTR_COLD;
	void boonggab_io(address_map &map) ATTR_COLD;

	void banked_oki_map(address_map &map) ATTR_COLD;
};

class vamphalf_qdsp_state : public vamphalf_state
{
public:
	vamphalf_qdsp_state(const machine_config &mconfig, device_type type, const char *tag)
		: vamphalf_state(mconfig, type, tag)
		, m_qdsp_cpu(*this, "qs1000:cpu")
	{
	}

	void misncrft(machine_config &config);
	void wyvernwg(machine_config &config);
	void yorijori(machine_config &config);

	void init_misncrft();
	void init_wyvernwg();
	void init_yorijori();

private:
	required_device<i8052_device> m_qdsp_cpu;

	u32 wyvernwg_prot_r();
	void wyvernwg_prot_w(u32 data);

	void yorijori_eeprom_w(u32 data);

	void misncrft_io(address_map &map) ATTR_COLD;
	void wyvernwg_io(address_map &map) ATTR_COLD;
	void yorijori_32bit_map(address_map &map) ATTR_COLD;
	void yorijori_io(address_map &map) ATTR_COLD;
};

class vamphalf_nvram_state : public vamphalf_state
{
public:
	vamphalf_nvram_state(const machine_config &mconfig, device_type type, const char *tag)
		: vamphalf_state(mconfig, type, tag)
		, m_nvram(*this, "nvram")
	{
	}

	void finalgdr(machine_config &config);
	void mrkickera(machine_config &config);

	void init_mrkickera();
	void init_finalgdr();

private:

	void finalgdr_io(address_map &map) ATTR_COLD;
	void mrkickera_io(address_map &map) ATTR_COLD;

	required_device<nvram_device> m_nvram;

	u16 m_finalgdr_backupram_bank = 0;
	std::unique_ptr<u8[]> m_finalgdr_backupram;
	void finalgdr_backupram_bank_w(u32 data);
	u32 finalgdr_backupram_r(offs_t offset);
	void finalgdr_backupram_w(offs_t offset, u32 data);

	void finalgdr_prize_w(u32 data);
	void finalgdr_oki_bank_w(u32 data);

	void finalgdr_eeprom_w(u32 data);
};

u16 vamphalf_state::eeprom_r()
{
	return m_eeprom->do_read();
}

void vamphalf_state::eeprom_w(u16 data)
{
	m_eeprom->di_write(data & 0x01);
	m_eeprom->cs_write((data & 0x04) ? ASSERT_LINE : CLEAR_LINE );
	m_eeprom->clk_write((data & 0x02) ? ASSERT_LINE : CLEAR_LINE );

	// data & 8?
}

void vamphalf_nvram_state::finalgdr_eeprom_w(u32 data)
{
	m_eeprom->di_write((data & 0x4000) >> 14);
	m_eeprom->cs_write((data & 0x1000) ? ASSERT_LINE : CLEAR_LINE );
	m_eeprom->clk_write((data & 0x2000) ? ASSERT_LINE : CLEAR_LINE );
}

void vamphalf_qdsp_state::yorijori_eeprom_w(u32 data)
{
	m_eeprom->di_write((data & 0x1000) >> 12);
	m_eeprom->cs_write((data & 0x4000) ? ASSERT_LINE : CLEAR_LINE );
	m_eeprom->clk_write((data & 0x2000) ? ASSERT_LINE : CLEAR_LINE );
}

void vamphalf_state::flipscreen_w(u16 data)
{
	m_flipscreen = data & m_flip_bit;
}

void vamphalf_state::jmpbreak_flipscreen_w(u16 data)
{
	m_flipscreen = data & 0x8000;
}


u32 vamphalf_qdsp_state::wyvernwg_prot_r()
{
	if (!machine().side_effects_disabled())
		m_semicom_prot_idx--;
	return (m_semicom_prot_data[m_semicom_prot_which] & (1 << m_semicom_prot_idx)) >> m_semicom_prot_idx;
}

void vamphalf_qdsp_state::wyvernwg_prot_w(u32 data)
{
	m_semicom_prot_which = data & 1;
	m_semicom_prot_idx = 8;
}

u32 vamphalf_state::finalgdr_prot_r()
{
	if (!machine().side_effects_disabled())
		m_semicom_prot_idx--;
	return (m_semicom_prot_data[m_semicom_prot_which] & (1 << m_semicom_prot_idx)) ? 0x8000 : 0;
}

void vamphalf_state::finalgdr_prot_w(u32 data)
{
	/*
	41C6
	967E
	446B
	F94B
	*/

	if (data == 0x41c6 || data == 0x446b)
		m_semicom_prot_which = 0;
	else
		m_semicom_prot_which = 1;

	m_semicom_prot_idx = 8;
}

void vamphalf_nvram_state::finalgdr_oki_bank_w(u32 data)
{
	m_okibank->set_entry((data & 0x300) >> 8);
}

void vamphalf_nvram_state::finalgdr_backupram_bank_w(u32 data)
{
	m_finalgdr_backupram_bank = (data & 0xff000000) >> 24;
}

u32 vamphalf_nvram_state::finalgdr_backupram_r(offs_t offset)
{
	return m_finalgdr_backupram[offset + m_finalgdr_backupram_bank * 0x80] << 24;
}

void vamphalf_nvram_state::finalgdr_backupram_w(offs_t offset, u32 data)
{
	m_finalgdr_backupram[offset + m_finalgdr_backupram_bank * 0x80] = data >> 24;
}

void vamphalf_nvram_state::finalgdr_prize_w(u32 data)
{
	if (data & 0x1000000)
	{
		// prize 1
	}

	if (data & 0x2000000)
	{
		// prize 2
	}

	if (data & 0x4000000)
	{
		// prize 3
	}
}

void vamphalf_state::boonggab_oki_bank_w(u16 data)
{
	m_okibank->set_entry(data & 0x7);
}


void vamphalf_state::mrkicker_oki_bank_w(u16 data)
{
	m_okibank->set_entry(data & 0x3);
}

void vamphalf_state::boonggab_prize_w(u16 data)
{
	// data & 0x01 == motor 1 on
	// data & 0x02 == motor 2 on
	// data & 0x04 == motor 3 on
	// data & 0x08 == prize power 1 on
	// data & 0x10 == prize lamp 1 off
	// data & 0x20 == prize lamp 2 off
	// data & 0x40 == prize lamp 3 off
}

void vamphalf_state::boonggab_lamps_w(offs_t offset, u16 data)
{
	if (offset == 0)
	{
		// data & 0x0001 == lamp  7 on (why is data & 0x8000 set too?)
		// data & 0x0002 == lamp  8 on
		// data & 0x0004 == lamp  9 on
		// data & 0x0008 == lamp 10 on
		// data & 0x0010 == lamp 11 on
		// data & 0x0020 == lamp 12 on
		// data & 0x0040 == lamp 13 on
	}
	else if (offset == 1)
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


void vamphalf_state::qs1000_p3_w(u8 data)
{
	if (!BIT(data, 5))
		m_soundlatch->acknowledge_w();

	m_qs1000_bank->set_entry(data & 7);
}


void vamphalf_state::common_map(address_map &map)
{
	map(0x00000000, 0x001fffff).ram().share("wram");
	map(0x40000000, 0x4003ffff).rw(FUNC(vamphalf_state::vram_r), FUNC(vamphalf_state::vram_w));
	map(0x80000000, 0x8000ffff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xfff00000, 0xffffffff).rom().region("maincpu", 0);
}

void vamphalf_state::common_32bit_map(address_map &map)
{
	map(0x00000000, 0x001fffff).ram().share("wram32");
	map(0x40000000, 0x4003ffff).rw(FUNC(vamphalf_state::vram_r), FUNC(vamphalf_state::vram_w));
	map(0x80000000, 0x8000ffff).ram().w(m_palette, FUNC(palette_device::write32)).share("palette");
	map(0xfff00000, 0xffffffff).rom().region("maincpu", 0);
}

void vamphalf_qdsp_state::yorijori_32bit_map(address_map &map)
{
	map(0x00000000, 0x001fffff).ram().share("wram32");
	map(0x40000000, 0x4003ffff).rw(FUNC(vamphalf_state::vram_r), FUNC(vamphalf_state::vram_w));
	map(0x80000000, 0x8000ffff).ram().w(m_palette, FUNC(palette_device::write32)).share("palette");
	map(0xffe00000, 0xffffffff).rom().region("maincpu", 0);
}

void vamphalf_state::vamphalf_io(address_map &map)
{
	map(0x030, 0x030).umask16(0x00ff).rw("oki1", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x050, 0x050).umask16(0x00ff).w("ymsnd", FUNC(ym2151_device::address_w));
	map(0x051, 0x051).umask16(0x00ff).rw("ymsnd", FUNC(ym2151_device::status_r), FUNC(ym2151_device::data_w));
	map(0x070, 0x070).r(FUNC(vamphalf_state::eeprom_r));
	map(0x090, 0x090).w(FUNC(vamphalf_state::flipscreen_w));
	map(0x180, 0x180).portr("SYSTEM");
	map(0x181, 0x181).portr("P1_P2");
	map(0x182, 0x182).w(FUNC(vamphalf_state::eeprom_w));
}

void vamphalf_qdsp_state::misncrft_io(address_map &map)
{
	map(0x040, 0x040).w(FUNC(vamphalf_state::flipscreen_w));
	map(0x080, 0x080).portr("P1_P2");
	map(0x090, 0x090).portr("SYSTEM");
	map(0x0f0, 0x0f0).w(FUNC(vamphalf_state::eeprom_w));
	map(0x100, 0x100).umask16(0x00ff).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x160, 0x161).r(FUNC(vamphalf_state::eeprom_r));
}

void vamphalf_state::coolmini_io(address_map &map)
{
	map(0x080, 0x080).w(FUNC(vamphalf_state::flipscreen_w));
	map(0x0c0, 0x0c0).portr("SYSTEM");
	map(0x0c1, 0x0c1).portr("P1_P2");
	map(0x0c2, 0x0c2).w(FUNC(vamphalf_state::eeprom_w));
	map(0x130, 0x130).umask16(0x00ff).rw("oki1", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x150, 0x150).umask16(0x00ff).w("ymsnd", FUNC(ym2151_device::address_w));
	map(0x151, 0x151).umask16(0x00ff).rw("ymsnd", FUNC(ym2151_device::status_r), FUNC(ym2151_device::data_w));
	map(0x1f0, 0x1f0).r(FUNC(vamphalf_state::eeprom_r));
}

void vamphalf_state::mrkicker_io(address_map &map)
{
	coolmini_io(map);

	map(0x000, 0x000).w(FUNC(vamphalf_state::mrkicker_oki_bank_w));
}

void vamphalf_state::suplup_io(address_map &map)
{
	map(0x008, 0x008).w(FUNC(vamphalf_state::eeprom_w));
	map(0x010, 0x010).portr("P1_P2");
	map(0x018, 0x018).portr("SYSTEM");
	map(0x020, 0x020).umask16(0x00ff).rw("oki1", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x030, 0x030).umask16(0x00ff).w("ymsnd", FUNC(ym2151_device::address_w));
	map(0x031, 0x031).umask16(0x00ff).rw("ymsnd", FUNC(ym2151_device::status_r), FUNC(ym2151_device::data_w));
	map(0x040, 0x040).r(FUNC(vamphalf_state::eeprom_r));
}

void vamphalf_qdsp_state::wyvernwg_io(address_map &map)
{
	map(0x0600, 0x0600).rw(FUNC(vamphalf_qdsp_state::wyvernwg_prot_r), FUNC(vamphalf_qdsp_state::wyvernwg_prot_w));
	map(0x0800, 0x0800).umask32(0x0000ffff).w(FUNC(vamphalf_state::flipscreen_w));
	map(0x0a00, 0x0a00).portr("P1_P2");
	map(0x0c00, 0x0c00).portr("SYSTEM");
	map(0x1500, 0x1500).umask32(0x000000ff).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x1c00, 0x1c00).umask32(0x0000ffff).w(FUNC(vamphalf_state::eeprom_w));
	map(0x1f00, 0x1f00).umask32(0x0000ffff).r(FUNC(vamphalf_state::eeprom_r));
}

void vamphalf_nvram_state::finalgdr_io(address_map &map)
{
	map(0x0900, 0x0900).r(FUNC(vamphalf_nvram_state::finalgdr_prot_r));
	map(0x0a00, 0x0a00).w(FUNC(vamphalf_nvram_state::finalgdr_backupram_bank_w));
	map(0x0b00, 0x0b7f).rw(FUNC(vamphalf_nvram_state::finalgdr_backupram_r), FUNC(vamphalf_nvram_state::finalgdr_backupram_w));
	map(0x0c00, 0x0c01).umask32(0x0000ff00).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x0d00, 0x0d00).umask32(0x0000ff00).rw("oki1", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x0e00, 0x0e00).portr("P1_P2");
	map(0x0f00, 0x0f00).portr("SYSTEM");
	map(0x1100, 0x1100).umask32(0x0000ffff).r(FUNC(vamphalf_state::eeprom_r));
	map(0x1800, 0x1800).nopr(); //?
	map(0x1800, 0x1800).w(FUNC(vamphalf_nvram_state::finalgdr_eeprom_w));
	map(0x1810, 0x1810).w(FUNC(vamphalf_nvram_state::finalgdr_prot_w));
	map(0x1818, 0x1818).w(FUNC(vamphalf_nvram_state::finalgdr_prize_w));
	//map(0x1820, 0x1820).w(FUNC(vamphalf_nvram_state::flipscreen32_w)); //?
	map(0x1828, 0x1828).w(FUNC(vamphalf_nvram_state::finalgdr_oki_bank_w));
}

void vamphalf_nvram_state::mrkickera_io(address_map &map)
{
	map(0x0900, 0x0900).umask32(0x0000ffff).r(FUNC(vamphalf_state::eeprom_r));
	map(0x1000, 0x1000).nopr(); //?
	map(0x1000, 0x1000).w(FUNC(vamphalf_nvram_state::finalgdr_eeprom_w));
	map(0x1010, 0x1010).w(FUNC(vamphalf_nvram_state::finalgdr_prot_w));
	map(0x1021, 0x1021).nopw(); //?
	map(0x1028, 0x1028).w(FUNC(vamphalf_nvram_state::finalgdr_oki_bank_w));
	map(0x1900, 0x1900).r(FUNC(vamphalf_nvram_state::finalgdr_prot_r));
	map(0x1c00, 0x1c01).umask32(0x0000ff00).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x1d00, 0x1d00).umask32(0x0000ff00).rw("oki1", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x1e00, 0x1e00).portr("P1_P2");
	map(0x1f00, 0x1f00).portr("SYSTEM");
}

void vamphalf_state::jmpbreak_io(address_map &map)
{
	map(0x030, 0x030).noprw(); // ?
	map(0x040, 0x040).nopw(); // ?
	map(0x090, 0x090).portr("P1_P2");
	map(0x0a0, 0x0a0).w(FUNC(vamphalf_state::eeprom_w));
	map(0x0b0, 0x0b0).r(FUNC(vamphalf_state::eeprom_r));
	map(0x110, 0x110).umask16(0x00ff).rw("oki1", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x150, 0x150).portr("SYSTEM");
	map(0x1a0, 0x1a0).umask16(0x00ff).w("ymsnd", FUNC(ym2151_device::address_w));
	map(0x1a1, 0x1a1).umask16(0x00ff).rw("ymsnd", FUNC(ym2151_device::status_r), FUNC(ym2151_device::data_w));
}

void vamphalf_state::worldadv_io(address_map &map)
{
	map(0x060, 0x060).w(FUNC(vamphalf_state::eeprom_w));
	map(0x0a0, 0x0a0).portr("P1_P2");
	map(0x0d0, 0x0d0).portr("SYSTEM");
	map(0x190, 0x190).umask16(0x00ff).rw("oki1", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x1c0, 0x1c0).umask16(0x00ff).w("ymsnd", FUNC(ym2151_device::address_w));
	map(0x1c1, 0x1c1).umask16(0x00ff).rw("ymsnd", FUNC(ym2151_device::status_r), FUNC(ym2151_device::data_w));
	map(0x1e0, 0x1e0).r(FUNC(vamphalf_state::eeprom_r));
}

void vamphalf_state::solitaire_io(address_map &map)
{
	map(0x000, 0x000).r(FUNC(vamphalf_state::eeprom_r));
	map(0x030, 0x030).portr("P1_P2");
	map(0x050, 0x050).umask16(0x00ff).rw("oki1", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x110, 0x110).portr("SYSTEM");
	//map(0x141, 0x142) // lamps
	map(0x160, 0x160).umask16(0x00ff).w("ymsnd", FUNC(ym2151_device::address_w));
	map(0x161, 0x161).umask16(0x00ff).rw("ymsnd", FUNC(ym2151_device::status_r), FUNC(ym2151_device::data_w));
	map(0x1a0, 0x1a0).w(FUNC(vamphalf_state::eeprom_w));
}

void vamphalf_state::mrdig_io(address_map &map)
{
	map(0x020, 0x020).umask16(0x00ff).rw("oki1", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x030, 0x030).umask16(0x00ff).w("ymsnd", FUNC(ym2151_device::address_w));
	map(0x031, 0x031).umask16(0x00ff).rw("ymsnd", FUNC(ym2151_device::status_r), FUNC(ym2151_device::data_w));
	map(0x060, 0x060).r(FUNC(vamphalf_state::eeprom_r));
	map(0x0a0, 0x0a0).portr("SYSTEM");
	map(0x0f0, 0x0f0).w(FUNC(vamphalf_state::eeprom_w));
	map(0x140, 0x140).portr("P1_P2");
}

void vamphalf_state::aoh_map(address_map &map)
{
	map(0x00000000, 0x003fffff).ram().share("wram32");
	map(0x40000000, 0x4003ffff).rw(FUNC(vamphalf_state::vram_r), FUNC(vamphalf_state::vram_w));
	map(0x80000000, 0x8000ffff).ram().w(m_palette, FUNC(palette_device::write32)).share("palette");
	map(0x80210000, 0x80210003).portr("SYSTEM");
	map(0x80220000, 0x80220003).portr("P1_P2");
	map(0xffc00000, 0xffffffff).rom().region("maincpu", 0);
}

void vamphalf_state::aoh_io(address_map &map)
{
	map(0x0120, 0x0120).umask32(0x0000ffff).w(FUNC(vamphalf_state::eeprom_w));
	map(0x0188, 0x0188).umask32(0x0000ff00).rw("oki2", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x0190, 0x0191).umask32(0x0000ff00).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x0198, 0x0198).umask32(0x0000ff00).rw("oki1", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x01a0, 0x01a0).umask32(0x0000ffff).w(FUNC(vamphalf_state::mrkicker_oki_bank_w));
}

void vamphalf_state::boonggab_io(address_map &map)
{
	map(0x030, 0x030).r(FUNC(vamphalf_state::eeprom_r));
	map(0x080, 0x080).noprw(); // seems unused
	map(0x0c0, 0x0c0).w(FUNC(vamphalf_state::flipscreen_w));
	map(0x100, 0x100).portr("SYSTEM");
	map(0x101, 0x101).portr("P1_P2");
	map(0x102, 0x102).w(FUNC(vamphalf_state::eeprom_w));
	map(0x104, 0x104).w(FUNC(vamphalf_state::boonggab_prize_w));
	map(0x105, 0x106).w(FUNC(vamphalf_state::boonggab_lamps_w));
	map(0x180, 0x180).w(FUNC(vamphalf_state::boonggab_oki_bank_w));
	map(0x1c0, 0x1c0).umask16(0x00ff).rw("oki1", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x1d0, 0x1d0).umask16(0x00ff).w("ymsnd", FUNC(ym2151_device::address_w));
	map(0x1d1, 0x1d1).umask16(0x00ff).rw("ymsnd", FUNC(ym2151_device::status_r), FUNC(ym2151_device::data_w));
}

void vamphalf_qdsp_state::yorijori_io(address_map &map)
{
	map(0x0900, 0x0900).r(FUNC(vamphalf_qdsp_state::finalgdr_prot_r));
	map(0x0d00, 0x0d00).portr("P1_P2");
	map(0x0e00, 0x0e00).umask32(0x0000ff00).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x0f00, 0x0f00).portr("SYSTEM");
	map(0x1100, 0x1100).umask32(0x0000ffff).r(FUNC(vamphalf_state::eeprom_r));
	map(0x1800, 0x1800).nopr(); //?
	map(0x1800, 0x1800).w(FUNC(vamphalf_qdsp_state::yorijori_eeprom_w));
	map(0x1810, 0x1810).w(FUNC(vamphalf_qdsp_state::finalgdr_prot_w));
}

void vamphalf_state::banked_oki_map(address_map &map)
{
	map(0x00000, 0x1ffff).rom();
	map(0x20000, 0x3ffff).bankr("okibank");
}

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

	m_flipscreen = 0;
}

void vamphalf_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	gfx_element *gfx = m_gfxdecode->gfx(0);
	rectangle clip = cliprect;
	int block;

	for (int y = (cliprect.min_y & ~15); y <= (cliprect.max_y | 15); y += 16)
	{
		clip.min_y = y;
		clip.max_y = y + 15;
		if (m_flipscreen)
		{
			block = (y / 16) * 0x800;
		}
		else
		{
			block = (16 - (y / 16)) * 0x800;
		}

		if (clip.min_y < cliprect.min_y)
			clip.min_y = cliprect.min_y;

		if (clip.max_y > cliprect.max_y)
			clip.max_y = cliprect.max_y;

		for (u32 cnt = 0; cnt < 0x800; cnt += 8)
		{
			const int offs = (block + cnt) / 2;

			if (m_tiles[offs] & 0x0100) continue;

			u32 code = m_tiles[offs+1];
			const u32 color = (m_tiles[offs+2] >> m_palshift) & 0x7f;

			// boonggab
			if (m_has_extra_gfx)
			{
				code |= ((m_tiles[offs+2] & 0x100) << 8);
			}

			int x = m_tiles[offs+3] & 0x01ff;
			int y = 256 - (m_tiles[offs] & 0x00ff);

			int fx = m_tiles[offs] & 0x8000;
			int fy = m_tiles[offs] & 0x4000;

			if (m_flipscreen)
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

/*
Sprite list:

Offset+0
-------- xxxxxxxx Y offs
------xx -------- Sprite number hi bits
-----x-- -------- Flip X
----x--- -------- Flip Y?

Offset+1
xxxxxxxx xxxxxxxx Sprite number

Offset+2
-------- -xxxxxxx Color
or
-xxxxxxx -------- Color

Offset+3
-------x xxxxxxxx X offs
*/

void vamphalf_state::draw_sprites_aoh(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	gfx_element *gfx = m_gfxdecode->gfx(0);
	rectangle clip = cliprect;
	int block;

	for (int y = (cliprect.min_y & ~15); y <= (cliprect.max_y | 15); y += 16)
	{
		clip.min_y = y;
		clip.max_y = y + 15;
		if (m_flipscreen)
		{
			block = (y / 16) * 0x800;
		}
		else
		{
			block = (16 - (y / 16)) * 0x800;
		}

		if (clip.min_y < cliprect.min_y)
			clip.min_y = cliprect.min_y;

		if (clip.max_y > cliprect.max_y)
			clip.max_y = cliprect.max_y;

		for (u32 cnt = 0; cnt < 0x800; cnt += 8)
		{
			const int offs = (block + cnt) / 2;
			const u32 code  = (m_tiles[offs+1] & 0xffff) | ((m_tiles[offs] & 0x300) << 8);
			const u32 color = (m_tiles[offs+2] >> m_palshift) & 0x7f;

			int x = m_tiles[offs+3] & 0x01ff;
			int y = 256 - (m_tiles[offs] & 0x00ff);

			int fx = m_tiles[offs] & 0x400;
			int fy = 0; // not used ? or it's m_tiles[offs] & 0x800?

			if (m_flipscreen)
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


void vamphalf_state::handle_flipped_visible_area(screen_device &screen)
{
	// are there actually registers to handle this?
	if (!m_flipscreen)
	{
		rectangle visarea;
		visarea.set(31, 350, 16, 251);
		screen.configure(screen.width(), screen.height(), visarea, screen.refresh_attoseconds());
	}
	else
	{
		rectangle visarea;
		visarea.set(31, 350, 20, 255);
		screen.configure(screen.width(), screen.height(), visarea, screen.refresh_attoseconds());
	}
}


u32 vamphalf_state::screen_update_common(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	handle_flipped_visible_area(screen);
	bitmap.fill(0, cliprect);
	draw_sprites(screen, bitmap, cliprect);
	return 0;
}

u32 vamphalf_state::screen_update_aoh(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
//  handle_flipped_visible_area(screen); // not on this?
	bitmap.fill(0, cliprect);
	draw_sprites_aoh(screen, bitmap, cliprect);
	return 0;
}

ioport_value vamphalf_state::boonggab_photo_sensors_r()
{
	static const u16 photo_sensors_table[8] = { 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00 };
	u8 res = m_photosensors->read();

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
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
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
	PORT_BIT( 0x00000010, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read)) // EEPROM bit
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
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_CUSTOM ) // sensor 1
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_CUSTOM ) // sensor 2
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_CUSTOM ) // sensor 3
	PORT_BIT( 0x3800, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(vamphalf_state::boonggab_photo_sensors_r)) // photo sensors 1, 2 and 3
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

static INPUT_PORTS_START( yorijori )
// TODO: test mode shows the two start buttons always stuck high, but maybe leftover from other games given button1 also acts as start?
// Test mode also shows button 5 and 6 for both players but where are they? Also leftover?
	PORT_START("P1_P2")
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) // Also seems to act as start button
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) // Also seems to act as start button
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)

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

static INPUT_PORTS_START( solitaire )
	PORT_START("P1_P2") // when you have no more moves, hold down “Turn Up Card” & “Register” and you get a count down to end that round / game
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_Z) PORT_NAME("Column 1 / 2 Credit Start") // L1 Button in test mode
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_X) PORT_NAME("Column 2 / 3 Credit Start") // L2 Button in test mode
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_C) PORT_NAME("Column 3 / 4 Credit Start") // L3 Button in test mode
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_V) PORT_NAME("Column 4") // L4 Button in test mode
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_B) PORT_NAME("Column 5") // L5 Button in test mode
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_CODE(KEYCODE_N) PORT_NAME("Column 6") // L6 Button in test mode
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_CODE(KEYCODE_M) PORT_NAME("Column 7") // L7 Button in test mode
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect in test mode
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_CODE(KEYCODE_A) PORT_NAME("Turn Up Card") // D1 Button in test mode
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_CODE(KEYCODE_S) PORT_NAME("Select Turned Up Card") // D2 Button in test mode
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_CODE(KEYCODE_D) PORT_NAME("Register") // R1 Button in test mode
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_CODE(KEYCODE_F) PORT_NAME("Gift") // Gift Button in test mode
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect in test mode
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect in test mode
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect in test mode
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect in test mode

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

static GFXDECODE_START( gfx_vamphalf )
	GFXDECODE_ENTRY( "gfx", 0, gfx_16x16x8_raw, 0, 0x80 )
GFXDECODE_END

void vamphalf_state::common(machine_config &config)
{
	E116(config, m_maincpu, 50_MHz_XTAL);    // E1-16T (TQFP), 50 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &vamphalf_state::common_map);
	m_maincpu->set_vblank_int("screen", FUNC(vamphalf_state::irq1_line_hold));

	// various games require fast timing to save settings, probably because our Hyperstone core timings are incorrect
	EEPROM_93C46_16BIT(config, m_eeprom);
	m_eeprom->erase_time(attotime::from_usec(1));
	m_eeprom->write_time(attotime::from_usec(1));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(28_MHz_XTAL / 4, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART);
	screen.set_screen_update(FUNC(vamphalf_state::screen_update_common));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 0x8000);
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_vamphalf);
}

void vamphalf_state::sound_ym_oki(machine_config &config)
{
	SPEAKER(config, "speaker", 2).front();

	YM2151(config, "ymsnd", 28_MHz_XTAL / 8).add_route(0, "speaker", 1.0, 0).add_route(1, "speaker", 1.0, 1); // 3.5MHz

	okim6295_device &oki1(OKIM6295(config, "oki1", 28_MHz_XTAL / 16 , okim6295_device::PIN7_HIGH)); // 1.75MHz
	oki1.add_route(ALL_OUTPUTS, "speaker", 1.0, 0);
	oki1.add_route(ALL_OUTPUTS, "speaker", 1.0, 1);
}

void vamphalf_state::sound_ym_banked_oki(machine_config &config)
{
	sound_ym_oki(config);
	subdevice<okim6295_device>("oki1")->set_addrmap(0, &vamphalf_state::banked_oki_map);
}

void vamphalf_state::sound_suplup(machine_config &config)
{
	SPEAKER(config, "speaker", 2).front();

	YM2151(config, "ymsnd", 14.318181_MHz_XTAL / 4).add_route(0, "speaker", 1.0, 0).add_route(1, "speaker", 1.0, 1); // 3.579545 MHz

	okim6295_device &oki1(OKIM6295(config, "oki1", 14.318181_MHz_XTAL / 8, okim6295_device::PIN7_HIGH)); // 1.75MHz
	oki1.add_route(ALL_OUTPUTS, "speaker", 1.0, 0);
	oki1.add_route(ALL_OUTPUTS, "speaker", 1.0, 1);
}

void vamphalf_state::sound_qs1000(machine_config &config)
{
	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set("qs1000", FUNC(qs1000_device::set_irq));
	m_soundlatch->set_separate_acknowledge(true);

	qs1000_device &qs1000(QS1000(config, "qs1000", 24_MHz_XTAL));
	qs1000.set_external_rom(true);
	qs1000.p1_in().set("soundlatch", FUNC(generic_latch_8_device::read));
	qs1000.p3_out().set(FUNC(vamphalf_state::qs1000_p3_w));
	qs1000.add_route(0, "speaker", 1.0, 0);
	qs1000.add_route(1, "speaker", 1.0, 1);
}

void vamphalf_state::vamphalf(machine_config &config)
{
	common(config);

	m_maincpu->set_addrmap(AS_IO, &vamphalf_state::vamphalf_io);

	sound_ym_oki(config);
}

void vamphalf_qdsp_state::misncrft(machine_config &config)
{
	common(config);

	GMS30C2116(config.replace(), m_maincpu, 50_MHz_XTAL); // 50 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &vamphalf_qdsp_state::common_map);
	m_maincpu->set_addrmap(AS_IO, &vamphalf_qdsp_state::misncrft_io);
	m_maincpu->set_vblank_int("screen", FUNC(vamphalf_state::irq1_line_hold));

	sound_qs1000(config);
}

void vamphalf_state::coolmini(machine_config &config)
{
	common(config);

	m_maincpu->set_addrmap(AS_IO, &vamphalf_state::coolmini_io);

	sound_ym_oki(config);
}

void vamphalf_state::mrkicker(machine_config &config)
{
	common(config);

	m_maincpu->set_addrmap(AS_IO, &vamphalf_state::mrkicker_io);

	sound_ym_banked_oki(config);
}

void vamphalf_state::suplup(machine_config &config)
{
	common(config);

	m_maincpu->set_addrmap(AS_IO, &vamphalf_state::suplup_io);

	// 14.31818MHz instead 28MHz
	subdevice<screen_device>("screen")->set_raw(14.318181_MHz_XTAL / 2, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART);
	sound_suplup(config);
}

void vamphalf_state::jmpbreak(machine_config &config)
{
	common(config);

	m_maincpu->set_addrmap(AS_IO, &vamphalf_state::jmpbreak_io);

	sound_ym_oki(config);
}

void vamphalf_state::solitaire(machine_config &config)
{
	common(config);

	m_maincpu->set_addrmap(AS_IO, &vamphalf_state::solitaire_io);

	sound_ym_oki(config);
}

void vamphalf_state::newxpang(machine_config &config)
{
	common(config);

	m_maincpu->set_addrmap(AS_IO, &vamphalf_state::mrdig_io);

	sound_ym_oki(config);
}

void vamphalf_state::worldadv(machine_config &config)
{
	common(config);

	m_maincpu->set_addrmap(AS_IO, &vamphalf_state::worldadv_io);

	sound_ym_oki(config);
}

void vamphalf_state::mrdig(machine_config &config)
{
	common(config);

	GMS30C2116(config.replace(), m_maincpu, 50_MHz_XTAL);   // 50 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &vamphalf_state::common_map);
	m_maincpu->set_addrmap(AS_IO, &vamphalf_state::mrdig_io);
	m_maincpu->set_vblank_int("screen", FUNC(vamphalf_state::irq1_line_hold));

	sound_ym_oki(config);
}

void vamphalf_qdsp_state::wyvernwg(machine_config &config)
{
	common(config);

	E132(config.replace(), m_maincpu, 50_MHz_XTAL);    // E1-32T (TQFP), 50 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &vamphalf_qdsp_state::common_32bit_map);
	m_maincpu->set_addrmap(AS_IO, &vamphalf_qdsp_state::wyvernwg_io);
	m_maincpu->set_vblank_int("screen", FUNC(vamphalf_state::irq1_line_hold));

	sound_qs1000(config);
}

void vamphalf_nvram_state::finalgdr(machine_config &config)
{
	common(config);

	E132(config.replace(), m_maincpu, 50_MHz_XTAL);    // E1-32T (TQFP), 50 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &vamphalf_nvram_state::common_32bit_map);
	m_maincpu->set_addrmap(AS_IO, &vamphalf_nvram_state::finalgdr_io);
	m_maincpu->set_vblank_int("screen", FUNC(vamphalf_state::irq1_line_hold));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	sound_ym_banked_oki(config);
}

void vamphalf_nvram_state::mrkickera(machine_config &config)
{
	common(config);

	E132(config.replace(), m_maincpu, 50_MHz_XTAL);    // E1-32T (TQFP), 50 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &vamphalf_nvram_state::common_32bit_map);
	m_maincpu->set_addrmap(AS_IO, &vamphalf_nvram_state::mrkickera_io);
	m_maincpu->set_vblank_int("screen", FUNC(vamphalf_state::irq1_line_hold));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	sound_ym_banked_oki(config);
}

void vamphalf_state::aoh(machine_config &config)
{
	E132X(config, m_maincpu, 20_MHz_XTAL * 4); // E1-32XN (PQFP), 4x internal multiplier
	m_maincpu->set_addrmap(AS_PROGRAM, &vamphalf_state::aoh_map);
	m_maincpu->set_addrmap(AS_IO, &vamphalf_state::aoh_io);
	m_maincpu->set_vblank_int("screen", FUNC(vamphalf_state::irq1_line_hold));

	EEPROM_93C46_16BIT(config, m_eeprom);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(32_MHz_XTAL / 4, 512, 64, 448, 264, 16, 240);
	screen.set_screen_update(FUNC(vamphalf_state::screen_update_aoh));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 0x8000);
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_vamphalf);

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	YM2151(config, "ymsnd", 3.579545_MHz_XTAL).add_route(0, "speaker", 1.0, 0).add_route(1, "speaker", 1.0, 1);

	okim6295_device &oki1(OKIM6295(config, "oki1", 32_MHz_XTAL / 8, okim6295_device::PIN7_HIGH)); // 4MHz
	oki1.add_route(ALL_OUTPUTS, "speaker", 1.0, 0);
	oki1.add_route(ALL_OUTPUTS, "speaker", 1.0, 1);

	okim6295_device &oki2(OKIM6295(config, "oki2", 32_MHz_XTAL / 32, okim6295_device::PIN7_HIGH)); // 1MHz
	oki2.set_addrmap(0, &vamphalf_state::banked_oki_map);
	oki2.add_route(ALL_OUTPUTS, "speaker", 1.0, 0);
	oki2.add_route(ALL_OUTPUTS, "speaker", 1.0, 1);
}

void vamphalf_state::boonggab(machine_config &config)
{
	common(config);

	m_maincpu->set_addrmap(AS_IO, &vamphalf_state::boonggab_io);

	sound_ym_banked_oki(config);
}

void vamphalf_qdsp_state::yorijori(machine_config &config)
{
	common(config);

	E132(config.replace(), m_maincpu, 50_MHz_XTAL);   // E1-32T (TQFP), 50 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &vamphalf_qdsp_state::yorijori_32bit_map);
	m_maincpu->set_addrmap(AS_IO, &vamphalf_qdsp_state::yorijori_io);
	m_maincpu->set_vblank_int("screen", FUNC(vamphalf_state::irq2_line_hold));

	// 27MHz instead 28MHz
	subdevice<screen_device>("screen")->set_raw(27_MHz_XTAL / 4, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART);
	sound_qs1000(config);
}

/*

Vamp x1/2 (Danbi, 1999)
Hardware info by Guru

DANBI (no PCB number)
|----------------------------------------------|
|uPC1241 VOL      KA12   VROM1                 |
|       LM324 LM324                            |
|      7805       BS901  AD-65   ROML01 ROMU01 |
|                                ROML00 ROMU00 |
|                     52258                    |
|                     52258        |--------|  |
|             |------|             |        |  |
|J T2316162   |E1-16T| ROM1        | QL2003 |  |
|A            |------|             |        |  |
|M                    52258        |--------|  |
|M                    52258                    |
|A  PALCE22V10H   |--------|                   |
|     93C46       |        |      52258        |
|T518A            | QL2003 |      52258        |
|  RESET    50MHz |        |      52258        |
|SERV SETUP EMUIO1|--------|28MHz 52258        |
|----------------------------------------------|
Notes:
      E1-16T - Hyperstone E1-16T 32-bit RISC/DSP Microprocessor. Clock 50.000MHz
       AD-65 - OKI M6295 rebadged as AD-65. Clock 1.750MHz [28/16]. Pin 7 HIGH
       BS109 - Yamaha YM2151 8-Channel 4-Operator OPM (FM Operator Type-M) sound chip rebadged as BS109. Clock 3.500MHz [28/8]
        KA12 - Yamaha YM3012 DAC. Clock 1.750MHz [28/8/2]. Source = YM2151 pin 23
       LM324 - Texas Instruments LM324 Quad Operational Amplifier
     uPC1241 - NEC uPC1241 7W AF Audio Power Amplifier
       52258 - Sharp LH52258 32kBx8-bit SRAM or HMC HM2H256 32kBx8-bit SRAM in SOJ28 package
    T2316162 - Taiwan Memory Technology T2316162 1MBx16-bit EDO Page-Mode DRAM
               or Samsung KM416C1200 1MBx16-bit EDO Page-Mode DRAM in SOJ42 package
       T518A - Mitsumi Electric PST518A System Reset IC with low voltage detection reset 4.2V
      QL2003 - QuickLogic QL2003-XPL84C pASIC-family FPGA in PLCC84 package
        SERV - Push button tact switch for service credit
       SETUP - Push button tact switch for test mode
       RESET - Push button tact switch for manual reset
      EMUIO1 - Not-populated 20 pin header. Possible for extra controls.
 PALCE22V10H - AMD PALCE22V10H Programmable Array Logic (PAL)
       93C46 - ST Microelectronics ST93C46 128x8 / 64x16 Serial EEPROM
               ORG pin is tied high on the PCB so the x16 mode is selected
        ROM1 - Texas Instruments TMS27C040 512kBx8-bit EPROM (main program)
       VROM1 - AMIC A278308 256kBx8-bit OTP EPROM, compatible with 27C020 etc (OKI samples)
      ROML00 \
      ROML01 |
      ROMU00 | Macronix MX29F1610MC-12 SOP44 16Mbit FlashROM (sprites)
      ROMU01 /
       HSync - 15.6249kHz
       VSync - 59.1855Hz

***************************************************************************

Ealier DANBI PCB:
+-----------------------------------------------+
|     VR1          KA3002    VROM1              |
|                                               |
|                  KA51   U6295  ELC     EVI    |
|                                ROML00* ROMU00*|
|                    62256                      |
|                    62256                      |
|J        +----------+              +---------+ |
|A  DRAM1 |GMS30C2116|  ROM1        |  Actel  | |
|M        +----------+              |A40MX04-F| |
|M                   62256          |  PL84   | |
|A          GAL1     62256          +---------+ |
|       93C46                                   |
|                 +---------+    62256          |
|                 |  Actel  |    62256          |
|                 |A40MX04-F|    62256          |
|       50.000MHz |  PL84   |    62256          |
|B1 B2 B3         +---------+ 28.000MHz         |
+-----------------------------------------------+

     CPU: HYUNDAI GMS30C2116
Graphics: Actel A40MX04-F PL84
   Sound: Oki M6295 rebadged as U6295
          YM3012/YM2151 rebadged as KA3002/KA51
    ROMs: ROML01, ROMU01 - SOP44 32MBit mask ROM for ELC & EVI
          ROML00, ROMU00 - unpopulated
   DRAM1: LG Semi GM71C18163 1M x16 EDO DRAM (SOJ44)
   VROM1: Macronix MX27C2000 2MBit DIP32 EPROM
    ROM1: ST M27C4001 4MBit DIP32 EPROM

    RAMs: MEMx/CRAMx - HMC HM2H256AJ-15 32K x8 SRAM (SOJ28)
    GAL1: PALCE22V10H

B1 B2 B3: Push buttons for SERV, RESET, TEST
     VR1: Volume adjust pot
*/

ROM_START( vamphalf )
	ROM_REGION16_BE( 0x100000, "maincpu", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	/* 0 - 0x80000 empty */
	ROM_LOAD( "prg.rom1", 0x80000, 0x80000, CRC(9b1fc6c5) SHA1(acf10a50d2119ac893b6cbd494911982a9352350) ) /* at 0x16554: Europe Version 1.1.0908 */

	ROM_REGION( 0x800000, "gfx", 0 ) /* 16x16x8 Sprites */
	ROM_LOAD32_WORD( "eur.roml00", 0x000000, 0x200000, CRC(bdee9a46) SHA1(7e240b07377201afbe0cd0911ccee4ad52a74079) )
	ROM_LOAD32_WORD( "eur.romu00", 0x000002, 0x200000, CRC(fa79e8ea) SHA1(feaba99f0a863bc5d27ad91d206168684976b4c2) )
	ROM_LOAD32_WORD( "eur.roml01", 0x400000, 0x200000, CRC(a7995b06) SHA1(8b789b6a00bc177c3329ee4a31722fc65376b975) )
	ROM_LOAD32_WORD( "eur.romu01", 0x400002, 0x200000, CRC(e269f5fe) SHA1(70f1308f11e147dd20f8bd45b91aefc9fd653da6) )

	ROM_REGION( 0x40000, "oki1", 0 ) /* Oki Samples */
	ROM_LOAD( "snd.vrom1", 0x00000, 0x40000, CRC(ee9e371e) SHA1(3ead5333121a77d76e4e40a0e0bf0dbc75f261eb) )
ROM_END

ROM_START( vamphalfr1 )
	ROM_REGION16_BE( 0x100000, "maincpu", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	/* 0 - 0x80000 empty */
	ROM_LOAD( "ws1-01201.rom1", 0x80000, 0x80000, CRC(afa75c19) SHA1(5dac104d1b3c026b6fce4d1f9126c048ebb557ef) ) /* at 0x162B8: Europe Version 1.0.0903 */

	ROM_REGION( 0x800000, "gfx", 0 ) /* 16x16x8 Sprites */
	ROM_LOAD32_WORD( "elc.roml01", 0x000000, 0x400000, CRC(19df4056) SHA1(8b05769d8e245f8b25bf92013b98c9d7e5ab4548) ) /* only 2 ROMs, though twice as big as other sets */
	ROM_LOAD32_WORD( "evi.romu01", 0x000002, 0x400000, CRC(f9803923) SHA1(adc1d4fa2c6283bc24829f924b58fbd9d1bacdd2) )

	ROM_REGION( 0x40000, "oki1", 0 ) /* Oki Samples */
	ROM_LOAD( "ws1-01202.vrom1", 0x00000, 0x40000, CRC(ee9e371e) SHA1(3ead5333121a77d76e4e40a0e0bf0dbc75f261eb) ) /* same data as other sets */
ROM_END


ROM_START( vamphalfk )
	ROM_REGION16_BE( 0x100000, "maincpu", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	/* 0 - 0x80000 empty */
	ROM_LOAD( "prom1", 0x80000, 0x80000, CRC(f05e8e96) SHA1(c860e65c811cbda2dc70300437430fb4239d3e2d) ) /* at 0x1653C: Korean Version 1.1.0908 */

	ROM_REGION( 0x800000, "gfx", 0 ) /* 16x16x8 Sprites */
	ROM_LOAD32_WORD( "roml00", 0x000000, 0x200000, CRC(cc075484) SHA1(6496d94740457cbfdac3d918dce2e52957341616) )
	ROM_LOAD32_WORD( "romu00", 0x000002, 0x200000, CRC(711c8e20) SHA1(1ef7f500d6f5790f5ae4a8b58f96ee9343ef8d92) )
	ROM_LOAD32_WORD( "roml01", 0x400000, 0x200000, CRC(626c9925) SHA1(c90c72372d145165a8d3588def12e15544c6223b) )
	ROM_LOAD32_WORD( "romu01", 0x400002, 0x200000, CRC(d5be3363) SHA1(dbdd0586909064e015f190087f338f37bbf205d2) )

	ROM_REGION( 0x40000, "oki1", 0 ) /* Oki Samples */
	ROM_LOAD( "snd.vrom1", 0x00000, 0x40000, CRC(ee9e371e) SHA1(3ead5333121a77d76e4e40a0e0bf0dbc75f261eb) )
ROM_END

/*

Super Lup Lup Puzzle / Lup Lup Puzzle
Omega System, 1999

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
	ROM_REGION16_BE( 0x100000, "maincpu", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	ROM_LOAD( "suplup-rom1.bin", 0x00000, 0x80000, CRC(61fb2dbe) SHA1(21cb8f571b2479de6779b877b656d1ffe5b3516f) )
	ROM_LOAD( "suplup-rom2.bin", 0x80000, 0x80000, CRC(0c176c57) SHA1(f103a1afc528c01cbc18639273ab797fb9afacb1) )

	ROM_REGION( 0x800000, "gfx", 0 ) /* 16x16x8 Sprites */
	ROM_LOAD32_WORD( "suplup-roml00.bin", 0x000000, 0x200000, CRC(7848e183) SHA1(1db8f0ea8f73f42824423d382b37b4d75fa3e54c) )
	ROM_LOAD32_WORD( "suplup-romu00.bin", 0x000002, 0x200000, CRC(13e3ab7f) SHA1(d5b6b15ca5aef2e2788d2b81e0418062f42bf2f2) )
	ROM_LOAD32_WORD( "suplup-roml01.bin", 0x400000, 0x200000, CRC(15769f55) SHA1(2c13e8da2682ccc7878218aaebe3c3c67d163fd2) )
	ROM_LOAD32_WORD( "suplup-romu01.bin", 0x400002, 0x200000, CRC(6687bc6f) SHA1(cf842dfb2bcdfda0acc0859985bdba91d4a80434) )

	ROM_REGION( 0x40000, "oki1", 0 ) /* Oki Samples */
	ROM_LOAD( "vrom1.bin", 0x00000, 0x40000, CRC(34a56987) SHA1(4d8983648a7f0acf43ff4c9c8aa6c8640ee2bbfe) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) /* Default EEPROM */
	ROM_LOAD( "eeprom-suplup.bin", 0x0000, 0x0080, CRC(e60c9883) SHA1(662dd8fb85eb97a8a4d53886198b269a5f6a6268) )
ROM_END

ROM_START( luplup ) /* version 3.0 / 990128 */
	ROM_REGION16_BE( 0x100000, "maincpu", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	ROM_LOAD( "luplup-rom1.v30", 0x00000, 0x80000, CRC(9ea67f87) SHA1(73d16c056a8d64743181069a01559a43fee529a3) )
	ROM_LOAD( "luplup-rom2.v30", 0x80000, 0x80000, CRC(99840155) SHA1(e208f8731c06b634e84fb73e04f6cdbb8b504b94) )

	ROM_REGION( 0x800000, "gfx", 0 ) /* 16x16x8 Sprites */
	ROM_LOAD32_WORD( "luplup-roml00",   0x000000, 0x200000, CRC(08b2aa75) SHA1(7577b3ab79c54980307a83186dd1500f044c1bc8) )
	ROM_LOAD32_WORD( "luplup-romu00",   0x000002, 0x200000, CRC(b57f4ca5) SHA1(b968c44a0ceb3274e066fa1d057fb6b017bb3fd3) )
	ROM_LOAD32_WORD( "luplup30-roml01", 0x400000, 0x200000, CRC(40e85f94) SHA1(531e67eb4eedf47b0dded52ba2f4942b12cbbe2f) ) /* This one changed between v2.9 & v3.0 */
	ROM_LOAD32_WORD( "luplup30-romu01", 0x400002, 0x200000, CRC(f2645b78) SHA1(b54c3047346c0f40dba0ba23b0d607cc53384edb) ) /* This one changed between v2.9 & v3.0 */

	ROM_REGION( 0x40000, "oki1", 0 ) /* Oki Samples */
	ROM_LOAD( "vrom1.bin", 0x00000, 0x40000, CRC(34a56987) SHA1(4d8983648a7f0acf43ff4c9c8aa6c8640ee2bbfe) )

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "gal22v10b.gal1", 0x0000, 0x02e5, NO_DUMP ) /* GAL is read protected */
ROM_END

ROM_START( luplup29 ) /* version 2.9 / 990108 */
	ROM_REGION16_BE( 0x100000, "maincpu", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	ROM_LOAD( "luplup-rom1.v29", 0x00000, 0x80000, CRC(36a8b8c1) SHA1(fed3eb2d83adc1b071a12ce5d49d4cab0ca20cc7) )
	ROM_LOAD( "luplup-rom2.v29", 0x80000, 0x80000, CRC(50dac70f) SHA1(0e313114a988cb633a89508fda17eb09023827a2) )

	ROM_REGION( 0x800000, "gfx", 0 ) /* 16x16x8 Sprites */
	ROM_LOAD32_WORD( "luplup-roml00", 0x000000, 0x200000, CRC(08b2aa75) SHA1(7577b3ab79c54980307a83186dd1500f044c1bc8) )
	ROM_LOAD32_WORD( "luplup-romu00", 0x000002, 0x200000, CRC(b57f4ca5) SHA1(b968c44a0ceb3274e066fa1d057fb6b017bb3fd3) )
	ROM_LOAD32_WORD( "luplup-roml01", 0x400000, 0x200000, CRC(41c7ca8c) SHA1(55704f9d54f31bbaa044cd9d10ac2d9cb5e8fb70) )
	ROM_LOAD32_WORD( "luplup-romu01", 0x400002, 0x200000, CRC(16746158) SHA1(a5036a7aaa717fde89d62b7ff7a3fded8b7f5cda) )

	ROM_REGION( 0x40000, "oki1", 0 ) /* Oki Samples */
	ROM_LOAD( "vrom1.bin", 0x00000, 0x40000, CRC(34a56987) SHA1(4d8983648a7f0acf43ff4c9c8aa6c8640ee2bbfe) )
ROM_END

ROM_START( luplup10 ) /* version 1.05 / 981214 */
	ROM_REGION16_BE( 0x100000, "maincpu", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	ROM_LOAD( "p0_rom1.rom1", 0x00000, 0x80000, CRC(a2684e3c) SHA1(9178ab6e7695cfb5bcdac3f3b8f3ea2a86372018) )
	ROM_LOAD( "p1_rom2.rom2", 0x80000, 0x80000, CRC(1043ce44) SHA1(13a23f35ff2335d837f682761f774a70e298e77a) )

	ROM_REGION( 0x800000, "gfx", 0 ) /* 16x16x8 Sprites */
	ROM_LOAD32_WORD( "roml00.roml00", 0x000000, 0x200000, CRC(e2eeb61e) SHA1(5261cf29cd7e10d86c0dd4bc640ad9c3db99cec3) )
	ROM_LOAD32_WORD( "romu00.romu00", 0x000002, 0x200000, CRC(9ee855b9) SHA1(a51b268a640b667d88a8ceab562607a811602fff) )
	ROM_LOAD32_WORD( "roml01.roml01", 0x400000, 0x200000, CRC(7182864c) SHA1(48789b20d9b8f41d7c9f5690f4f44bc6f15b8cfe) )
	ROM_LOAD32_WORD( "romu01.romu01", 0x400002, 0x200000, CRC(44f76640) SHA1(6a49ed4d5584ecd0496b9ce19aefd5f4e0126da7) )

	ROM_REGION( 0x40000, "oki1", 0 ) /* Oki Samples */
	ROM_LOAD( "vrom1.bin", 0x00000, 0x40000, CRC(34a56987) SHA1(4d8983648a7f0acf43ff4c9c8aa6c8640ee2bbfe) )

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "gal22v10b.gal1",  0x0000, 0x02e5, CRC(776c5137) SHA1(f6ced83ef803549cc61c14c276f914f267e91ce6) ) /* GAL22V10B at GAL1 */
ROM_END

ROM_START( puzlbang ) /* version 2.9 / 990108 - Korea only, cannot select title, language and limited selection of background choices, EI: censored  */
	ROM_REGION16_BE( 0x100000, "maincpu", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	ROM_LOAD( "pbb-rom1.v29", 0x00000, 0x80000, CRC(eb829586) SHA1(1f8a6af7c51c715724f5a242f4e22f7f6fb1f0ee) )
	ROM_LOAD( "pbb-rom2.v29", 0x80000, 0x80000, CRC(fb84c793) SHA1(a2d27caecdae457d12b48d88d19ce417f69507c6) )

	ROM_REGION( 0x800000, "gfx", 0 ) /* 16x16x8 Sprites */
	ROM_LOAD32_WORD( "luplup-roml00", 0x000000, 0x200000, CRC(08b2aa75) SHA1(7577b3ab79c54980307a83186dd1500f044c1bc8) )
	ROM_LOAD32_WORD( "luplup-romu00", 0x000002, 0x200000, CRC(b57f4ca5) SHA1(b968c44a0ceb3274e066fa1d057fb6b017bb3fd3) )
	ROM_LOAD32_WORD( "luplup-roml01", 0x400000, 0x200000, CRC(41c7ca8c) SHA1(55704f9d54f31bbaa044cd9d10ac2d9cb5e8fb70) )
	ROM_LOAD32_WORD( "luplup-romu01", 0x400002, 0x200000, CRC(16746158) SHA1(a5036a7aaa717fde89d62b7ff7a3fded8b7f5cda) )

	ROM_REGION( 0x40000, "oki1", 0 ) /* Oki Samples */
	ROM_LOAD( "vrom1.bin", 0x00000, 0x40000, CRC(34a56987) SHA1(4d8983648a7f0acf43ff4c9c8aa6c8640ee2bbfe) )
ROM_END

ROM_START( puzlbanga ) /* version 2.8 / 990106 - Korea only, cannot select title, language or change background selection, EI: censored */
	ROM_REGION16_BE( 0x100000, "maincpu", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	ROM_LOAD( "pbb-rom1.v28", 0x00000, 0x80000, CRC(fd21c5ff) SHA1(bc6314bbb2495c140788025153c893d5fd00bdc1) )
	ROM_LOAD( "pbb-rom2.v28", 0x80000, 0x80000, CRC(490ecaeb) SHA1(2b0f25e3d681ddf95b3c65754900c046b5b50b09) )

	ROM_REGION( 0x800000, "gfx", 0 ) /* 16x16x8 Sprites */
	ROM_LOAD32_WORD( "luplup-roml00", 0x000000, 0x200000, CRC(08b2aa75) SHA1(7577b3ab79c54980307a83186dd1500f044c1bc8) )
	ROM_LOAD32_WORD( "luplup-romu00", 0x000002, 0x200000, CRC(b57f4ca5) SHA1(b968c44a0ceb3274e066fa1d057fb6b017bb3fd3) )
	ROM_LOAD32_WORD( "luplup-roml01", 0x400000, 0x200000, CRC(41c7ca8c) SHA1(55704f9d54f31bbaa044cd9d10ac2d9cb5e8fb70) )
	ROM_LOAD32_WORD( "luplup-romu01", 0x400002, 0x200000, CRC(16746158) SHA1(a5036a7aaa717fde89d62b7ff7a3fded8b7f5cda) )
	ROM_REGION( 0x40000, "oki1", 0 ) /* Oki Samples */
	ROM_LOAD( "vrom1.bin", 0x00000, 0x40000, CRC(34a56987) SHA1(4d8983648a7f0acf43ff4c9c8aa6c8640ee2bbfe) )
ROM_END

/*

Jumping Break & Poosho Poosho
F2 System, 1999

sequel to "Die Break" from the Bestri 3 game collection (see crospang.cpp)

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
|             GAL1                             |
| 93C46          DRAM1      ROM1 ROML00  ROMU00|
|P1 P2   50MHz   E1-16T     ROM2 ROML01  ROMU01|
|                                              |
+----------------------------------------------+

Notes:
CPU: Hyperstone E1-16T @ 50.000MHz

     DRAM1 - LG Semi GM71C18163 1M x16 EDO DRAM (SOJ44)
MEMx/CRAMx - NKK N341256SJ-15 32K x8 SRAM (SOJ28)
      GAL1 - PALCE22V10H

Oki M6295 rebadged as AD-65
YM3012/YM2151 rebadged as KA12/BS901

 P1 - Setup push button
 P2 - Reset push button
VR1 - Volume adjust pot

ROMs:
    ROML00/01, ROMU00/01 - Macronix MX29F1610MC-12 SOP44 16MBit FlashROM
    VROM1                - AMD 27C020 2MBit DIP32 EPROM
    ROM1/2               - TMS 27C040 4MBit DIP32 EPROM

Measured Clocks:
  E1-16T  @ 50MHz
  YM2151  @ 3.5MHz (28MHz/8)
  M6295   @ 1.75MH (28MHz/16), Pin7 High
   H-Sync @ 15.625KHz
   V-Sync @ 59.189Hz

*/

ROM_START( jmpbreak ) /* Released February 1999 */
	ROM_REGION16_BE( 0x100000, "maincpu", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	ROM_LOAD( "rom1.bin", 0x00000, 0x80000, CRC(7e237f7d) SHA1(042e672be34644311eefc7b998bcdf6a9ea2c28a) )
	ROM_LOAD( "rom2.bin", 0x80000, 0x80000, CRC(c722f7be) SHA1(d8b3c6b5fd0942147e0a61169c3eb6334a3b5a40) )

	ROM_REGION( 0x800000, "gfx", 0 ) /* 16x16x8 Sprites */
	ROM_LOAD32_WORD( "roml00.bin", 0x000000, 0x200000, CRC(4b99190a) SHA1(30af068f7d9f9f349db5696c19ab53ac33304271) )
	ROM_LOAD32_WORD( "romu00.bin", 0x000002, 0x200000, CRC(e93762f8) SHA1(cc589b59e3ab7aa7092e96a1ff8a9de8a499b257) )
	ROM_LOAD32_WORD( "roml01.bin", 0x400000, 0x200000, CRC(6796a104) SHA1(3f7352cd37f78c1b01f7df45344ee7800db110f9) )
	ROM_LOAD32_WORD( "romu01.bin", 0x400002, 0x200000, CRC(0cc907c8) SHA1(86029eca0870f3b7dd4f1ee8093ccb09077cc00b) )

	ROM_REGION( 0x40000, "oki1", 0 ) /* Oki Samples */
	ROM_LOAD( "vrom1.bin", 0x00000, 0x40000, CRC(1b6e3671) SHA1(bd601460387b56c989785ae03d5bb3c6cdb30a50) )
ROM_END

ROM_START( jmpbreaka ) // PCB has a New Impeuropex sticker, so sold in the Italian market. There also an hand-written IMP 28.04.99
	ROM_REGION16_BE( 0x100000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "2.rom1", 0x00000, 0x80000, CRC(553af133) SHA1(e2ae803e8f58426417093cc4b3784dee858f41ef) )
	ROM_LOAD( "3.rom2", 0x80000, 0x80000, CRC(bd0a5eed) SHA1(9aaf83e4dcd4d02fb9b1c3156264c013a6873972) )

	ROM_REGION( 0x800000, "gfx", 0 ) // these were not dumped for this set
	ROM_LOAD32_WORD( "roml00.bin", 0x000000, 0x200000, BAD_DUMP CRC(4b99190a) SHA1(30af068f7d9f9f349db5696c19ab53ac33304271) )
	ROM_LOAD32_WORD( "romu00.bin", 0x000002, 0x200000, BAD_DUMP CRC(e93762f8) SHA1(cc589b59e3ab7aa7092e96a1ff8a9de8a499b257) )
	ROM_LOAD32_WORD( "roml01.bin", 0x400000, 0x200000, BAD_DUMP CRC(6796a104) SHA1(3f7352cd37f78c1b01f7df45344ee7800db110f9) )
	ROM_LOAD32_WORD( "romu01.bin", 0x400002, 0x200000, BAD_DUMP CRC(0cc907c8) SHA1(86029eca0870f3b7dd4f1ee8093ccb09077cc00b) )

	ROM_REGION( 0x40000, "oki1", 0 )
	ROM_LOAD( "1.vrom1", 0x00000, 0x40000, CRC(1b6e3671) SHA1(bd601460387b56c989785ae03d5bb3c6cdb30a50) )

	ROM_REGION( 0x2dd, "plds", 0 )
	ROM_LOAD( "palce22v10h.gal1", 0x000, 0x2dd, CRC(0ff86470) SHA1(0cc2bd2958c71d0bb58081a8f88327b09e92e2ea) )
ROM_END

ROM_START( poosho ) /* Released November 1999 - Updated sequel to Jumping Break for Korean market */
	ROM_REGION16_BE( 0x100000, "maincpu", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	ROM_LOAD( "rom1.bin", 0x00000, 0x80000, CRC(2072c120) SHA1(cf066cd277840fdbb7a854a052a80b2fbb582278) )
	ROM_LOAD( "rom2.bin", 0x80000, 0x80000, CRC(80e70d7a) SHA1(cdafce4bfe7370978414a12aaf482e07a1c89ff8) )

	ROM_REGION( 0x800000, "gfx", 0 ) /* 16x16x8 Sprites */
	ROM_LOAD32_WORD( "roml00.bin", 0x000000, 0x200000, CRC(9efb0673) SHA1(3aeae96e591a415c27942dce90fc64c11287097d) )
	ROM_LOAD32_WORD( "romu00.bin", 0x000002, 0x200000, CRC(fe1d6a02) SHA1(4d451cfc6457f56a98bcec7998713757dbefa2b5) )
	ROM_LOAD32_WORD( "roml01.bin", 0x400000, 0x200000, CRC(05e81ca0) SHA1(22c6b78e3a0f27195142221bd179a4ecac819684) )
	ROM_LOAD32_WORD( "romu01.bin", 0x400002, 0x200000, CRC(fd2d02c7) SHA1(cc4fb765c6083e36a49f32f0d4e77792eb354f44) )

	ROM_REGION( 0x40000, "oki1", 0 ) /* Oki Samples */
	ROM_LOAD( "vrom1.bin", 0x00000, 0x40000, CRC(1b6e3671) SHA1(bd601460387b56c989785ae03d5bb3c6cdb30a50) )

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "gal1.bin",  0x0000, 0x02e5, CRC(90352c93) SHA1(cb72e52313dcd9fc0c8b794a1745d54af76a6129) )
ROM_END

/*

New Cross Pang
F2 System, 1999

sequel to "Cross Pang" (see crospang.cpp)

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
|             GAL1                             |
| 93C46          DRAM1     ROM1* ROML00  ROMU00|
|P1 P2   50MHz   E1-16T    ROM2  ROML01  ROMU01|
|                                              |
+----------------------------------------------+

Notes:
CPU: Hyperstone E1-16T @ 50.000MHz

     DRAM1 - LG Semi GM71C18163 1M x16 EDO DRAM (SOJ44)
MEMx/CRAMx - NKK N341256SJ-15 32K x8 SRAM (SOJ28)
      GAL1 - PALCE22V10H

Oki M6295 rebadged as AD-65
YM3012/YM2151 rebadged as KA12/KA51

 P1 - Setup push button
 P2 - Reset push button
VR1 - Volume adjust pot

ROMs:
    ROML00/01, ROMU00/01 - Macronix MX29F1610MC-12 SOP44 16MBit FlashROM
    VROM1                - TMS 27C020 2MBit DIP32 EPROM
  * ROM1                 - Unpopulated space for DIP32 EPROM (up to 4MBit)
    ROM2                 - ST M27C4001 4MBit DIP32 EPROM

Measured Clocks:
  E1-16T  @ 50MHz
  YM2151  @ 3.5MHz (28MHz/8)
  M6295   @ 1.75MH (28MHz/16), Pin7 High
   H-Sync @ 15.625KHz
   V-Sync @ 59.189Hz

*/

ROM_START( newxpang ) /* Released January 1999 */
	ROM_REGION16_BE( 0x100000, "maincpu", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	/* ROM1 empty */
	ROM_LOAD( "rom2.bin", 0x80000, 0x80000, CRC(6d69c799) SHA1(e8c9b8c00056c4d019b44918a2e03e18cf68b833) )

	ROM_REGION( 0x800000, "gfx", 0 ) /* 16x16x8 Sprites */
	ROM_LOAD32_WORD( "roml00.bin", 0x000000, 0x200000, CRC(4f8253d3) SHA1(0a4d5db879da6412326bff3edc3007402883fb02) )
	ROM_LOAD32_WORD( "romu00.bin", 0x000002, 0x200000, CRC(0ac8f8e4) SHA1(af89b1bb422faa42f5a0980a999803150e7d9f39) )
	ROM_LOAD32_WORD( "roml01.bin", 0x400000, 0x200000, CRC(66e6e05e) SHA1(032fa6155590bea879ce09ce8d08101c9eed8b7b) )
	ROM_LOAD32_WORD( "romu01.bin", 0x400002, 0x200000, CRC(73907b33) SHA1(63320131f9c1c07ab537c98cf5f31a077fb70799) )

	ROM_REGION( 0x40000, "oki1", 0 ) /* Oki Samples */
	ROM_LOAD( "vrom1.bin", 0x00000, 0x40000, CRC(0f339d68) SHA1(9dc128aa35d37c84c2caee839f69bd0d090bae8f) )
ROM_END

ROM_START( newxpanga ) // F-E1-16-002, too, but uses jmpbreak I/O map
	ROM_REGION16_BE( 0x100000, "maincpu", ROMREGION_ERASE00 ) // Hyperstone CPU Code
	// ROM1 empty
	ROM_LOAD( "rom2.bin", 0x80000, 0x80000, CRC(325c2c4f) SHA1(8019032cb714d85f182bb15650f9dad4fe89d8f0) ) // sldh

	ROM_REGION( 0x800000, "gfx", 0 ) // 16x16x8 sprites, not dumped for this set, seem to work fine
	ROM_LOAD32_WORD( "roml00.bin", 0x000000, 0x200000, BAD_DUMP CRC(4f8253d3) SHA1(0a4d5db879da6412326bff3edc3007402883fb02) )
	ROM_LOAD32_WORD( "romu00.bin", 0x000002, 0x200000, BAD_DUMP CRC(0ac8f8e4) SHA1(af89b1bb422faa42f5a0980a999803150e7d9f39) )
	ROM_LOAD32_WORD( "roml01.bin", 0x400000, 0x200000, BAD_DUMP CRC(66e6e05e) SHA1(032fa6155590bea879ce09ce8d08101c9eed8b7b) )
	ROM_LOAD32_WORD( "romu01.bin", 0x400002, 0x200000, BAD_DUMP CRC(73907b33) SHA1(63320131f9c1c07ab537c98cf5f31a077fb70799) )

	ROM_REGION( 0x40000, "oki1", 0 )
	ROM_LOAD( "vrom1.bin", 0x00000, 0x40000, CRC(0f339d68) SHA1(9dc128aa35d37c84c2caee839f69bd0d090bae8f) )
ROM_END

/*

World Adventure
F2 System, 1999

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
|             GAL1                             |
| 93C46          DRAM1     ROM1  ROML00  ROMU00|
|P1 P2   50MHz   E1-16T    ROM2  ROML01  ROMU01|
|                                              |
+----------------------------------------------+

Notes:
CPU: Hyperstone E1-16T @ 50.000MHz

     DRAM1 - LG Semi GM71C18163 1M x16 EDO DRAM (SOJ44)
MEMx/CRAMx - NKK N341256SJ-15 32K x8 SRAM (SOJ28)
      GAL1 - PALCE22V10H

Oki M6295 rebadged as AD-65
YM3012/YM2151

 P1 - Setup push button
 P2 - Reset push button
VR1 - Volume adjust pot

ROMs:
    ROML00/01, ROMU00/01 - Macronix MX29F1610MC-12 SOP44 16MBit FlashROM
    VROM1                - AMIC A278308 2MBit DIP32 EPROM
    ROM1/2               - ST M27C4001 4MBit DIP32 EPROM

Measured Clocks:
  E1-16T  @ 50MHz
  YM2151  @ 3.5MHz (28MHz/8)
  M6295   @ 1.75MH (28MHz/16), Pin7 High
   H-Sync @ 15.625KHz
   V-Sync @ 59.189Hz

*/

ROM_START( worldadv ) /* Developed April 1999 */
	ROM_REGION16_BE( 0x100000, "maincpu", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	ROM_LOAD( "rom1.bin", 0x00000, 0x80000, CRC(1855c235) SHA1(b4f7488365474248be8473c61bd2545e59132e44) )
	ROM_LOAD( "rom2.bin", 0x80000, 0x80000, CRC(671ddbb0) SHA1(07f856ae33105440e08e4ae353952db4df65ad9f) )

	ROM_REGION( 0x800000, "gfx", 0 ) /* 16x16x8 Sprites */
	ROM_LOAD32_WORD( "roml00.bin", 0x000000, 0x200000, CRC(fe422890) SHA1(98c52f924345718a3b86d49b42b8c6fbba596da7) )
	ROM_LOAD32_WORD( "romu00.bin", 0x000002, 0x200000, CRC(dd1066f5) SHA1(bf10217404eebbddc8bc639e86ca77f935e0b148) )
	ROM_LOAD32_WORD( "roml01.bin", 0x400000, 0x200000, CRC(9ab76649) SHA1(ba4ae12638e1b25e77e7b7d20e6518bf9ce6bd1b) )
	ROM_LOAD32_WORD( "romu01.bin", 0x400002, 0x200000, CRC(62132228) SHA1(7588fa90424ce4e557d1f43d3944cb89e007d63b) )

	ROM_REGION( 0x40000, "oki1", 0 ) /* Oki Samples */
	ROM_LOAD( "vrom1.bin", 0x00000, 0x40000, CRC(c87cce3b) SHA1(0b189fee8fb87c8fb06a67ae9d901732b89fbf38) )
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

Oki M6295 rebadged as AD-65
YM3012/YM2151 rebadged as KA12/KB2001

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
	ROM_REGION16_BE( 0x100000, "maincpu", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	ROM_LOAD( "rom1.bin", 0x00000, 0x80000, CRC(5b960320) SHA1(adf5499a39987041fc93e409bdb5fd07dacec4f9) )
	ROM_LOAD( "rom2.bin", 0x80000, 0x80000, CRC(75d48b64) SHA1(c9c492fb9cabafcf0bc05f44bf80ee6df3c21a1b) )

	ROM_REGION( 0x800000, "gfx", 0 ) /* 16x16x8 Sprites */
	ROM_LOAD32_WORD( "roml00.bin", 0x000000, 0x200000, CRC(f6b161ea) SHA1(c417a4c877ffa2fdf5857ecc9c78ffc0c09dc516) )
	ROM_LOAD32_WORD( "romh00.bin", 0x000002, 0x200000, CRC(5477efed) SHA1(e4991ee1b41d512eaa508351b6a78261dfde5a3d) )

	ROM_REGION( 0x40000, "oki1", 0 ) /* Oki Samples */
	ROM_LOAD( "vrom1.bin", 0x00000, 0x40000, CRC(5fd9e1c6) SHA1(fef82ef816af69f31d12fc4634d06d825e8b7416) )
ROM_END

/*

Solitaire
F2 System, 1999

F-E1-16-004
+-----------------------------------------------+
| RESET                  KA12    VROM1  ROMU01  |
| TEST                   KA51    AD-65  ROMU00  |
|                CRAM2        28.000MHz ROML01  |
|                CRAM1                  ROML00  |
|                                               |
|J                                  +----------+|
|A      DRAM1  E1-16T ROM2          |          ||
|M              50.000MHz           |Quicklogic||
|M          PAL                     | QL2003-  ||
|A              93C46               | XPL84C   ||
|                             MEM1U +----------+|
|                +----------+ MEM1L             |
|         MOTOR3 |          | MEM3              |
|         MOTOR2 |Quicklogic| MEM2              |
|         MOTOR1 | QL2003-  | MEM7              |
|  M01 L01 LIGHT | XPL84C   | MEM6              |
+----------------+----------+-------------------+

   CPU: Hyperstone E1-16T
 Video: 2 x QuickLogic QL2003-XPL84C FPGA
 Sound: AD-65 (OKI 6295), KA51 (YM2151) & KA12 (YM3012)
   OSC: 50MHz & 28MHz
EEPROM: 93C46

MOTOR1 - MOTOR3 are 4 pin headers
LIGHT is a 4 pin header
AAM01 & AAL01 are 10 pin headers
RESET & TEST are push buttons

RAM:
     DRAM1 - LG Semi GM71C18163 1M x16 EDO DRAM (SOJ44)
MEMx/CRAMx - NKK N341256SJ-15 32K x8 SRAM (SOJ28)

ROMs:
    ROMU00/L00 & ROMU01/L01 - Macronix MX29F1610MC-12 SOP44 16MBit FlashROM
    ROM2  - AMD AM27C040 4MBit DIP32 EPROM
    VROM1 - AMD AM27C020 2MBit DIP32 EPROM

*/

ROM_START( solitaire ) // Version 2.5
	ROM_REGION32_BE( 0x100000, "maincpu", ROMREGION_ERASE00 ) // Hyperstone CPU Code
	// 0 - 0x80000 empty
	ROM_LOAD( "rom2.bin",               0x080000, 0x080000, CRC(304e4338) SHA1(6b2817d7505c943ca7cdfa9176c9504e30936235) ) // 27c040

	ROM_REGION32_BE( 0x800000, "gfx", 0 )  // gfx data, all mx29f1610mc
	ROM_LOAD32_WORD_SWAP( "romu00.bin", 0x000000, 0x200000, CRC(7fee63ac) SHA1(ef22145da9ce3100c8736e9a77e59da4f984aaba) )
	ROM_LOAD32_WORD_SWAP( "roml00.bin", 0x000002, 0x200000, CRC(0d973625) SHA1(b482a97732a6117d9c1c7507118e111ac4f7f3f1) )
	ROM_LOAD32_WORD_SWAP( "romu01.bin", 0x400000, 0x200000, CRC(f3f3f3e5) SHA1(9a0d91351903b70049fbbc76a9ccff1a382ecbfd) )
	ROM_LOAD32_WORD_SWAP( "roml01.bin", 0x400002, 0x200000, CRC(5bba95b8) SHA1(6d884a694cbbad6768e606afd5b234a07a3b5b50) )

	ROM_REGION( 0x80000, "oki1", 0 ) // Oki Samples
	ROM_LOAD( "vrom1.bin",              0x000000, 0x040000, CRC(bbbf4ac8) SHA1(b37f945143a9ed7a372a953ef93dbea01c4fcce4) ) // 27c020

	ROM_REGION( 0x2dd, "plds", 0 )
	ROM_LOAD( "palce22v10.gal1",        0x000000, 0x0002dd, NO_DUMP ) // Protected
ROM_END

/*

Cool Minigame Collection
SemiCom, 1999

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

Also known to be found on the F-E1-16-010 PCB

Notes:
CPU: Hyperstone E1-16T @ 50.000MHz

Oki M6295 rebadged as AD-65
YM3012/YM2151 rebadged as BS902/KA51

ROMs:
    ROML00 & ROMH00 - Macronix MX29F1610MC-12 SOP44 16MBit FlashROM
    ROML01 & ROMH01 - Macronix MX29F1610MC-12 SOP44 16MBit FlashROM
    ROML02 & ROMH02 - Macronix MX29F1610MC-12 SOP44 16MBit FlashROM
    ROML03 & ROMH03 - Macronix MX29F1610MC-12 SOP44 16MBit FlashROM
    VROM1           - MX 27C2000 2MBit DIP32 EPROM
    ROM1            - MX 27C4000 4MBit DIP32 EPROM
    ROM2            - MX 27C4000 4MBit DIP32 EPROM

Measured Clocks:
   H-Sync @ 15.625KHz
   V-Sync @ 59.000Hz
*/

ROM_START( coolmini )
	ROM_REGION16_BE( 0x100000, "maincpu", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	ROM_LOAD( "cm-rom1", 0x00000, 0x80000, CRC(9688fa98) SHA1(d5ebeb1407980072f689c3b3a5161263c7082e9a) )
	ROM_LOAD( "cm-rom2", 0x80000, 0x80000, CRC(9d588fef) SHA1(7b6b0ba074c7fa0aecda2b55f411557b015522b6) )

	ROM_REGION( 0x1000000, "gfx", 0 )  /* 16x16x8 Sprites */
	ROM_LOAD32_WORD( "roml00", 0x000000, 0x200000, CRC(4b141f31) SHA1(cf4885789b0df67d00f9f3659c445248c4e72446) )
	ROM_LOAD32_WORD( "romu00", 0x000002, 0x200000, CRC(9b2fb12a) SHA1(8dce367c4c2cab6e84f586bd8dfea3ea0b6d7225) )
	ROM_LOAD32_WORD( "roml01", 0x400000, 0x200000, CRC(1e3a04bb) SHA1(9eb84b6a0172a8868f440065c30b4519e0c3fe33) )
	ROM_LOAD32_WORD( "romu01", 0x400002, 0x200000, CRC(06dd1a6c) SHA1(8c707d388848bc5826fbfc48c3035fdaf5018515) )
	ROM_LOAD32_WORD( "roml02", 0x800000, 0x200000, CRC(1e8c12cb) SHA1(f57489e81eb1e476939148cfc8d03f3df03b2a84) )
	ROM_LOAD32_WORD( "romu02", 0x800002, 0x200000, CRC(4551d4fc) SHA1(4ec102120ab99e324d9574bfce93837d8334da06) )
	ROM_LOAD32_WORD( "roml03", 0xc00000, 0x200000, CRC(231650bf) SHA1(065f742a37d5476ec6f72f0bd8ba2cfbe626b872) )
	ROM_LOAD32_WORD( "romu03", 0xc00002, 0x200000, CRC(273d5654) SHA1(0ae3d1c4c4862a8642dbebd7c955b29df29c4938) )

	ROM_REGION( 0x40000, "oki1", 0 ) /* Oki Samples */
	ROM_LOAD( "cm-vrom1", 0x00000, 0x40000, CRC(fcc28081) SHA1(44031df0ee28ca49df12bcb73c83299fac205e21) )
ROM_END

ROM_START( coolminii )
	ROM_REGION16_BE( 0x100000, "maincpu", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	ROM_LOAD( "cm-rom1.040", 0x00000, 0x80000, CRC(aa94bb86) SHA1(f1d75bf54b75f234cc872779c5b1ff6679778841) )
	ROM_LOAD( "cm-rom2.040", 0x80000, 0x80000, CRC(be7d02c8) SHA1(4897f3c890dd66f94d7a29f7a73c59857e4af218) )

	ROM_REGION( 0x1000000, "gfx", 0 )  /* 16x16x8 Sprites */
	ROM_LOAD32_WORD( "roml00",     0x000000, 0x200000, CRC(4b141f31) SHA1(cf4885789b0df67d00f9f3659c445248c4e72446) )
	ROM_LOAD32_WORD( "romu00",     0x000002, 0x200000, CRC(9b2fb12a) SHA1(8dce367c4c2cab6e84f586bd8dfea3ea0b6d7225) )
	ROM_LOAD32_WORD( "roml01",     0x400000, 0x200000, CRC(1e3a04bb) SHA1(9eb84b6a0172a8868f440065c30b4519e0c3fe33) )
	ROM_LOAD32_WORD( "romu01",     0x400002, 0x200000, CRC(06dd1a6c) SHA1(8c707d388848bc5826fbfc48c3035fdaf5018515) )
	ROM_LOAD32_WORD( "roml02",     0x800000, 0x200000, CRC(1e8c12cb) SHA1(f57489e81eb1e476939148cfc8d03f3df03b2a84) )
	ROM_LOAD32_WORD( "romu02",     0x800002, 0x200000, CRC(4551d4fc) SHA1(4ec102120ab99e324d9574bfce93837d8334da06) )
	ROM_LOAD32_WORD( "roml03.l03", 0xc00000, 0x200000, CRC(30a7fe2f) SHA1(f2c56728fcbe656bf22239763884518b01b3697c) ) /* only these two changed for the Italian version */
	ROM_LOAD32_WORD( "romu03.u03", 0xc00002, 0x200000, CRC(eb7c943d) SHA1(2a3207dea482a71d7cce017c429a2915ae99fdb1) ) /* only these two changed for the Italian version */

	ROM_REGION( 0x40000, "oki1", 0 ) /* Oki Samples */
	ROM_LOAD( "cm-vrom1", 0x00000, 0x40000, CRC(fcc28081) SHA1(44031df0ee28ca49df12bcb73c83299fac205e21) )
ROM_END

/*

Date Quiz Go Go Episode 2
SemiCom, 2000

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

Oki M6295 rebadged as AD-65
YM3012/YM2151 rebadged as BS902/KA51

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
	ROM_REGION16_BE( 0x100000, "maincpu", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	ROM_LOAD( "rom1",         0x00000, 0x080000, CRC(81eef038) SHA1(9c925d1ef261ea85069925ccd1a5aeb939f55d5a) )
	ROM_LOAD( "rom2",         0x80000, 0x080000, CRC(e8789d8a) SHA1(1ee26c26cc7024c5df9d0da630b326021ece9f41) )

	ROM_REGION( 0xc00000, "gfx", 0 ) /* 16x16x8 Sprites */
	ROM_LOAD32_WORD( "roml00", 0x000000, 0x200000, CRC(de811dd7) SHA1(bf31e165440ed2e3cdddd2174521b15afd8b2e69) )
	ROM_LOAD32_WORD( "romu00", 0x000002, 0x200000, CRC(2bdbfc6b) SHA1(8e755574e3c9692bd8f82c7351fe3623a31ec136) )
	ROM_LOAD32_WORD( "roml01", 0x400000, 0x200000, CRC(f574a2a3) SHA1(c6a8aca75bd3a4e4109db5095f3a3edb9b1e6657) )
	ROM_LOAD32_WORD( "romu01", 0x400002, 0x200000, CRC(d05cf02f) SHA1(624316d4ee42c6257bc64747e4260a0d3950f9cd) )
	ROM_LOAD32_WORD( "roml02", 0x800000, 0x200000, CRC(43ca2cff) SHA1(02ad7cce42d917dbefdba2e4e8886fc883b1dc60) )
	ROM_LOAD32_WORD( "romu02", 0x800002, 0x200000, CRC(b8218222) SHA1(1e1aa60e0de9c02b841896512a1163dda280c845) )
	/* roml03 empty */
	/* romu03 empty */

	ROM_REGION( 0x40000, "oki1", 0 ) /* Oki Samples */
	ROM_LOAD( "vrom1", 0x00000, 0x40000, CRC(24d5b55f) SHA1(cb4d3a22440831e37df0a7fe5433bea708d60f31) )
ROM_END

/*

Diet Family
SemiCom, 2001

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

Oki M6295 rebadged as AD-65
YM3012/YM2151 rebadged as BS902/KA51

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
	ROM_REGION16_BE( 0x100000, "maincpu", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	ROM_LOAD( "rom1",         0x00000, 0x080000, CRC(738636d2) SHA1(ba7906df99764ee7e1f505c319d364c64c605ff0) )
	ROM_LOAD( "rom2",         0x80000, 0x080000, CRC(0953f5e4) SHA1(ee8b3c4f9c9301c9815747eab5435e006ec84ca1) )

	ROM_REGION( 0xc00000, "gfx", 0 ) /* 16x16x8 Sprites */
	ROM_LOAD32_WORD( "roml00", 0x000000, 0x200000, CRC(7e2a7520) SHA1(0ff157fe34ff31cd8636af821fe14c12242d757f) )
	ROM_LOAD32_WORD( "romu00", 0x000002, 0x200000, CRC(c170755f) SHA1(019d24979071f0ab2b3c93a5ec9327e6a0b2afa2) )
	ROM_LOAD32_WORD( "roml01", 0x400000, 0x200000, CRC(3d487ffe) SHA1(c5608423d608922c0e1ac8bdfaa0de062b2c9821) )
	ROM_LOAD32_WORD( "romu01", 0x400002, 0x200000, CRC(716efedb) SHA1(fb468d93817a49173698872c49a289c257f77a92) )
	ROM_LOAD32_WORD( "roml02", 0x800000, 0x200000, CRC(c3dd3c96) SHA1(2707f690b5850685f782fc04a7dbd1d91f443223) )
	ROM_LOAD32_WORD( "romu02", 0x800002, 0x200000, CRC(80830961) SHA1(b318e9e3a4d1d7dca61d7d4c9ee01f605e2b2f4a) )
	/* roml03 empty */
	/* romu03 empty */

	/* $00000-$20000 stays the same in all sound banks, */
	/* the second half of the bank is what gets switched */
	ROM_REGION( 0x80000, "oki1", 0 ) /* Oki Samples */
	ROM_LOAD( "vrom1", 0x00000, 0x80000, CRC(4aacaef3) SHA1(c079170dc0ba0b91b1780cd175dc38151d640ff0) )
ROM_END

/*

Toy Land Adventure
SemiCom, 2001

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

Oki M6295 rebadged as AD-65
YM3012/YM2151 rebadged as BS902/KA51

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
	ROM_REGION16_BE( 0x100000, "maincpu", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	/* ROM1 empty */
	ROM_LOAD( "rom2.bin",         0x80000, 0x080000, CRC(e3455002) SHA1(5ad7884f82fb125d70829accec02f238e7d9593c) )

	ROM_REGION( 0xc00000, "gfx", 0 ) /* 16x16x8 Sprites */
	ROM_LOAD32_WORD( "roml00.bin", 0x000000, 0x200000, CRC(06f5673d) SHA1(23769015fc9a37d36b0fe4924964650aeca77573) )
	ROM_LOAD32_WORD( "romu00.bin", 0x000002, 0x200000, CRC(8c3db0e4) SHA1(6101ec550ae165338333fb04e0762edee65ca253) )
	ROM_LOAD32_WORD( "roml01.bin", 0x400000, 0x200000, CRC(076a84e1) SHA1(f58cb4cd874e1f3f266a5ccbf8ffb5e0111034d3) )
	ROM_LOAD32_WORD( "romu01.bin", 0x400002, 0x200000, CRC(1bc33d01) SHA1(a2a3e6b473cefe463dbd60bda98cb5a4df2bc81b) )
	/* roml02 empty */
	/* romu02 empty */
	/* roml03 empty */
	/* romu03 empty */

	ROM_REGION( 0x40000, "oki1", 0 ) /* Oki Samples */
	ROM_LOAD( "vrom1.bin", 0x00000, 0x40000, CRC(d7e6fc5d) SHA1(ab5bca4035299214d98b222320276fbcaedb0898) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) /* Default EEPROM */
	ROM_LOAD( "epr1.ic3", 0x0000, 0x0080, CRC(812f3d87) SHA1(744919ff4b44eaa3c4dcc75a1cc2f231ccbb4a3e) )

ROM_END

/*

Wivern Wings (c) 2001 SemiCom / Wyvern Wings (c) 2001 SemiCom, Game Vision License

   CPU: Hyperstone E1-32T
 Video: 2 QuickLogic QL12x16B-XPL84 FPGA
 Sound: AdMOS QDSP1000 with QDSP QS1001A sample ROM
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

ROMH & ROML are all MX 29F1610MC-16 flash ROMs
u15A is a MX 29F1610MC-16 flash ROM
u7 is a ST 27c1001
ROM1 & ROM2 are both ST 27C4000D

*/

ROM_START( wivernwg )
	ROM_REGION32_BE( 0x100000, "maincpu", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	ROM_LOAD( "rom1", 0x000000, 0x080000, CRC(83eb9a36) SHA1(d9c3b2facf42c137abc2923bbaeae300964ca4a0) ) /* ST 27C4000D with no labels */
	ROM_LOAD( "rom2", 0x080000, 0x080000, CRC(5d657055) SHA1(21baa81b80f28aec4a6be9eaf69709958bf2a129) )

	ROM_REGION( 0x080000, "qs1000:cpu", 0 ) /* QDSP (8052) Code */
	ROM_LOAD( "u7",  0x00000, 0x20000, CRC(00a3f705) SHA1(f0a6bafd16bea53d4c05c8cc108983cbd41e5757) ) /* ST 27C1001 with no labels */
	ROM_RELOAD(      0x20000, 0x20000 )
	ROM_RELOAD(      0x40000, 0x20000 )
	ROM_RELOAD(      0x60000, 0x20000 )

	ROM_REGION( 0x1000000, "gfx", 0 )  /* gfx data */
	ROM_LOAD32_WORD( "roml00", 0x000000, 0x200000, CRC(fb3541b6) SHA1(4f569ac7bde92c5febf005ab73f76552421ec223) ) /* MX 29F1610MC-16 flash ROMs with no labels */
	ROM_LOAD32_WORD( "romh00", 0x000002, 0x200000, CRC(516aca48) SHA1(42cf5678eb4c0ee7da2ab0bd66e4e34b2735c75a) )
	ROM_LOAD32_WORD( "roml01", 0x400000, 0x200000, CRC(1c764f95) SHA1(ba6ac1376e837b491bc0269f2a1d10577a3d40cb) )
	ROM_LOAD32_WORD( "romh01", 0x400002, 0x200000, CRC(fee42c63) SHA1(a27b5cbca0defa9be85fee91dde1273f445d3372) )
	ROM_LOAD32_WORD( "roml02", 0x800000, 0x200000, CRC(fc846707) SHA1(deaee15ab71927f644dcf576959e2ceaa55bfd44) )
	ROM_LOAD32_WORD( "romh02", 0x800002, 0x200000, CRC(86141c7d) SHA1(22a82cc7d44d655b03867503a83e81f7c82d6c91) )
	ROM_LOAD32_WORD( "l03",    0xc00000, 0x200000, CRC(85aa8db8) SHA1(8ad067f92ff161683ac962ffc5391504145a3d4a) )
	ROM_LOAD32_WORD( "h03",    0xc00002, 0x200000, CRC(ade8af9f) SHA1(05cdc1b38dec9d8a86302f2de794391fd3e376a5) )

	ROM_REGION( 0x1000000, "qs1000", 0 ) /* Music data / QDSP samples (SFX) */
	ROM_LOAD( "romsnd.u15a", 0x000000, 0x200000, CRC(fc89eedc) SHA1(2ce28bdb773cfa5b5660e4c0a9ef454cb658f2da) ) /* MX 29F1610MC-16 flash ROM with no label */
	ROM_LOAD( "qs1001a",     0x200000, 0x080000, CRC(d13c6407) SHA1(57b14f97c7d4f9b5d9745d3571a0b7115fbe3176) )
ROM_END

ROM_START( wyvernwg )
	ROM_REGION32_BE( 0x100000, "maincpu", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	ROM_LOAD( "rom1.bin", 0x000000, 0x080000, CRC(66bf3a5c) SHA1(037d5e7a6ef6f5b4ac08a9c811498c668a9d2522) ) /* ST 27c4000D with no labels */
	ROM_LOAD( "rom2.bin", 0x080000, 0x080000, CRC(fd9b5911) SHA1(a01e8c6e5a9009024af385268ba3ba90e1ebec50) )

	ROM_REGION( 0x080000, "qs1000:cpu", 0 ) /* QDSP (8052) Code */
	ROM_LOAD( "u7",  0x00000, 0x20000, CRC(00a3f705) SHA1(f0a6bafd16bea53d4c05c8cc108983cbd41e5757) ) /* ST 27C1001 with no labels */
	ROM_RELOAD(      0x20000, 0x20000 )
	ROM_RELOAD(      0x40000, 0x20000 )
	ROM_RELOAD(      0x60000, 0x20000 )

	ROM_REGION( 0x1000000, "gfx", 0 )  /* gfx data */
	ROM_LOAD32_WORD( "roml00", 0x000000, 0x200000, CRC(fb3541b6) SHA1(4f569ac7bde92c5febf005ab73f76552421ec223) ) /* MX 29F1610MC-16 flash ROMs with no labels */
	ROM_LOAD32_WORD( "romh00", 0x000002, 0x200000, CRC(516aca48) SHA1(42cf5678eb4c0ee7da2ab0bd66e4e34b2735c75a) )
	ROM_LOAD32_WORD( "roml01", 0x400000, 0x200000, CRC(1c764f95) SHA1(ba6ac1376e837b491bc0269f2a1d10577a3d40cb) )
	ROM_LOAD32_WORD( "romh01", 0x400002, 0x200000, CRC(fee42c63) SHA1(a27b5cbca0defa9be85fee91dde1273f445d3372) )
	ROM_LOAD32_WORD( "roml02", 0x800000, 0x200000, CRC(fc846707) SHA1(deaee15ab71927f644dcf576959e2ceaa55bfd44) )
	ROM_LOAD32_WORD( "romh02", 0x800002, 0x200000, CRC(86141c7d) SHA1(22a82cc7d44d655b03867503a83e81f7c82d6c91) )
	ROM_LOAD32_WORD( "roml03", 0xc00000, 0x200000, CRC(b10bf37c) SHA1(6af835b1e2573f0bb2c17057e016a7aecc8fcde8) )
	ROM_LOAD32_WORD( "romh03", 0xc00002, 0x200000, CRC(e01c2a92) SHA1(f53c2db92d62f595d473b1835c46d426f0dbe6b3) )

	ROM_REGION( 0x1000000, "qs1000", 0 ) /* Music data / QDSP samples (SFX) */
	ROM_LOAD( "romsnd.u15a", 0x000000, 0x200000, CRC(fc89eedc) SHA1(2ce28bdb773cfa5b5660e4c0a9ef454cb658f2da) ) /* MX 29F1610MC-16 flash ROM with no label */
	ROM_LOAD( "qs1001a",     0x200000, 0x080000, CRC(d13c6407) SHA1(57b14f97c7d4f9b5d9745d3571a0b7115fbe3176) )
ROM_END

ROM_START( wyvernwga )
	ROM_REGION32_BE( 0x100000, "maincpu", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	ROM_LOAD( "rom1.rom", 0x000000, 0x080000, CRC(586881fd) SHA1(d335bbd91def8fa4935eb2375c9b00471a1f40eb) ) /* ST 27c4000D with no labels */
	ROM_LOAD( "rom2.rom", 0x080000, 0x080000, CRC(938049ec) SHA1(cc10944c99ceb388dd4aafc93377c40540861d14) )

	ROM_REGION( 0x080000, "qs1000:cpu", 0 ) /* QDSP (8052) Code */
	ROM_LOAD( "u7",  0x00000, 0x20000, CRC(00a3f705) SHA1(f0a6bafd16bea53d4c05c8cc108983cbd41e5757) ) /* ST 27C1001 with no labels */
	ROM_RELOAD(      0x20000, 0x20000 )
	ROM_RELOAD(      0x40000, 0x20000 )
	ROM_RELOAD(      0x60000, 0x20000 )

	ROM_REGION( 0x1000000, "gfx", 0 )  /* gfx data */
	ROM_LOAD32_WORD( "roml00", 0x000000, 0x200000, CRC(fb3541b6) SHA1(4f569ac7bde92c5febf005ab73f76552421ec223) ) /* MX 29F1610MC-16 flash ROMs with no labels */
	ROM_LOAD32_WORD( "romh00", 0x000002, 0x200000, CRC(516aca48) SHA1(42cf5678eb4c0ee7da2ab0bd66e4e34b2735c75a) )
	ROM_LOAD32_WORD( "roml01", 0x400000, 0x200000, CRC(1c764f95) SHA1(ba6ac1376e837b491bc0269f2a1d10577a3d40cb) )
	ROM_LOAD32_WORD( "romh01", 0x400002, 0x200000, CRC(fee42c63) SHA1(a27b5cbca0defa9be85fee91dde1273f445d3372) )
	ROM_LOAD32_WORD( "roml02", 0x800000, 0x200000, CRC(fc846707) SHA1(deaee15ab71927f644dcf576959e2ceaa55bfd44) )
	ROM_LOAD32_WORD( "romh02", 0x800002, 0x200000, CRC(86141c7d) SHA1(22a82cc7d44d655b03867503a83e81f7c82d6c91) )
	ROM_LOAD32_WORD( "roml03", 0xc00000, 0x200000, CRC(b10bf37c) SHA1(6af835b1e2573f0bb2c17057e016a7aecc8fcde8) )
	ROM_LOAD32_WORD( "romh03", 0xc00002, 0x200000, CRC(e01c2a92) SHA1(f53c2db92d62f595d473b1835c46d426f0dbe6b3) )

	ROM_REGION( 0x1000000, "qs1000", 0 ) /* Music data / QDSP samples (SFX) */
	ROM_LOAD( "romsnd.u15a", 0x000000, 0x200000, CRC(fc89eedc) SHA1(2ce28bdb773cfa5b5660e4c0a9ef454cb658f2da) ) /* MX 29F1610MC-16 flash ROM with no label */
	ROM_LOAD( "qs1001a",     0x200000, 0x080000, CRC(d13c6407) SHA1(57b14f97c7d4f9b5d9745d3571a0b7115fbe3176) )
ROM_END

/*

Mission Craft
Sun, 2000

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

ROM_START( misncrft ) /* Version 2.7 */
	ROM_REGION16_BE( 0x100000, "maincpu", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	/* 0 - 0x80000 empty */
	ROM_LOAD( "prg-rom2.bin", 0x80000, 0x80000, CRC(04d22da6) SHA1(1c5be430000a31f21204fb756fadf2523a546b8b) )

	ROM_REGION( 0x080000, "qs1000:cpu", 0 ) /* QDSP (8052) Code */
	ROM_LOAD( "snd-rom2.us1", 0x00000, 0x20000, CRC(8821e5b9) SHA1(4b8df97bc61b48aa16ed411614fcd7ed939cac33) )
	ROM_RELOAD(      0x20000, 0x20000 )
	ROM_RELOAD(      0x40000, 0x20000 )
	ROM_RELOAD(      0x60000, 0x20000 )

	ROM_REGION( 0x800000, "gfx", 0 )
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

ROM_START( misncrfta ) /* Version 2.4 */
	ROM_REGION16_BE( 0x100000, "maincpu", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	/* 0 - 0x80000 empty */
	ROM_LOAD( "prg-rom2.bin", 0x80000, 0x80000, CRC(059ae8c1) SHA1(2c72fcf560166cb17cd8ad665beae302832d551c) ) // sldh

	ROM_REGION( 0x080000, "qs1000:cpu", 0 ) /* QDSP (8052) Code */
	ROM_LOAD( "snd-rom2.us1", 0x00000, 0x20000, CRC(8821e5b9) SHA1(4b8df97bc61b48aa16ed411614fcd7ed939cac33) )
	ROM_RELOAD(      0x20000, 0x20000 )
	ROM_RELOAD(      0x40000, 0x20000 )
	ROM_RELOAD(      0x60000, 0x20000 )

	ROM_REGION( 0x800000, "gfx", 0 )
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
     QS1001A Sample ROM (silkscreened as SND3)
     SND2 Additional sound samples
     SND5 8052 CPU code for QS1000

EEPROM - Atmel 93C46 at U6

DRAM1 - Hynix GM71C18163CJ5 1M x16 EDO DRAM (SOJ44)
CRAMx - M61C256J-15 32K x8 SRAM (SOJ28)
MEMx  - M61C256J-15 32K x8 SRAM (SOJ28)

 P1 - Reset push button
 P2 - Setup push button
VR1 - Volume adjust pot

16C550 - Asynchronous Comm Element with Autoflow Control (all components related to the 16C500 are unpopulated)
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
	ROM_REGION32_BE( 0x200000, "maincpu", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	ROM_LOAD( "prg1", 0x000000, 0x200000, CRC(0e04eb40) SHA1(0cec9dc91aaf9cf7c459c7baac200cf0fcfddc18) )

	ROM_REGION( 0x080000, "qs1000:cpu", 0 ) /* QDSP (8052) Code */
	ROM_LOAD( "snd5", 0x00000, 0x20000, CRC(79067367) SHA1(a8f0c02dd616ff8c5fb49dea1a116fea2aced19c) )
	ROM_RELOAD(       0x20000, 0x20000 )
	ROM_RELOAD(       0x40000, 0x20000 )
	ROM_RELOAD(       0x60000, 0x20000 )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD32_WORD( "roml00", 0x000000, 0x200000, CRC(9299ce36) SHA1(cd8a9e2619da93e2015704230e8189a6ae52de69) )
	ROM_LOAD32_WORD( "romh00", 0x000002, 0x200000, CRC(16584ff2) SHA1(69dce8c33b246b4327b330233116c1b72a8b7e84) )
	ROM_LOAD32_WORD( "roml01", 0x400000, 0x200000, CRC(b5d1892f) SHA1(20afcd00a506ec0fd1c4fffb2d9c853c8dc61e2e) )
	ROM_LOAD32_WORD( "romh01", 0x400002, 0x200000, CRC(fe0485ef) SHA1(bd1a26aa386803df8e8e137ea5d5a2cdd6ad1197) )

	ROM_REGION( 0x1000000, "qs1000", 0 )
	ROM_LOAD( "snd2",         0x000000, 0x200000, CRC(8d9a8795) SHA1(482acb3beafc9baa43284c54ac36086c57098465) )
	ROM_LOAD( "qs1001a.snd3", 0x200000, 0x080000, CRC(d13c6407) SHA1(57b14f97c7d4f9b5d9745d3571a0b7115fbe3176) )
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
ROML00 & ROMH00 are MX 29F1610MC flash ROMs
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
	ROM_REGION32_BE( 0x100000, "maincpu", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	/* rom0 empty */
	ROM_LOAD( "rom1", 0x080000, 0x080000, CRC(45815931) SHA1(80ba7a366994e40a1f520ea18fad82e6b068b279) )

	ROM_REGION( 0x800000, "gfx", 0 )  /* gfx data */
	ROM_LOAD32_WORD( "roml00", 0x000000, 0x200000, CRC(8334459d) SHA1(70ad560dada8aa8ce192e5307bd805744b82fcfe) )
	ROM_LOAD32_WORD( "romh00", 0x000002, 0x200000, CRC(f28578a5) SHA1(a5c7b17aff101f1f4f52657d0567a6c9d12a178d) )
	/* roml01 empty */
	/* romh01 empty */

	/* $00000-$20000 stays the same in all sound banks, */
	/* the second half of the bank is what gets switched */
	ROM_REGION( 0x80000, "oki1", 0 ) /* Oki Samples */
	ROM_LOAD( "u7", 0x000000, 0x080000, CRC(080f61f8) SHA1(df3764b1b07f9fc38685e3706b0f834f62088727) )
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
ROML00 & ROMH00 are MX 29F1610MC flash ROMs
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

Oki M6295 rebadged as AD-65
YM3012/YM2151 rebadged as BS902/KA51

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
	ROM_REGION32_BE( 0x100000, "maincpu", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	/* rom0 empty */
	ROM_LOAD( "2-semicom.rom1", 0x080000, 0x080000, CRC(d3da29ca) SHA1(b843c650096a1c6d50f99e354ec0c93eb4406c5b) ) /* SEMICOM-003b PCB */

	ROM_REGION( 0x800000, "gfx", 0 )  /* gfx data */
	ROM_LOAD32_WORD( "roml00", 0x000000, 0x200000, CRC(c677aac3) SHA1(356073a29260e8e6c29dd12b2113b30140c6108c) )
	ROM_LOAD32_WORD( "romh00", 0x000002, 0x200000, CRC(b6337d4a) SHA1(2f46e2933af7fd0f71083900d5e6e4f602ab4c66) )
	/* roml01 empty */
	/* romh01 empty */

	/* $00000-$20000 stays the same in all sound banks, */
	/* the second half of the bank is what gets switched */
	ROM_REGION( 0x80000, "oki1", 0 ) /* Oki Samples */
	ROM_LOAD( "at27c040.u7", 0x000000, 0x080000, CRC(e8141fcd) SHA1(256fd1987030e0a1df0a66a228c1fea996cda686) ) /* Mask ROM */

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) /* Default EEPROM (it doesn't boot without and the game code crashes) (game also refuses to boot if program attempts to rewrite it, CPU bug or protection?) */
	ROM_LOAD( "eeprom-mrkicker.bin", 0x0000, 0x0080, CRC(87afb8f7) SHA1(444203b793c1d7929fc5916f18b510198719cd38) )
ROM_END

ROM_START( mrkicker )
	ROM_REGION16_BE( 0x100000, "maincpu", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	/* rom1 empty */
	ROM_LOAD( "3-semicom.rom2", 0x080000, 0x080000, CRC(3f7fa08b) SHA1(dbffd44d8387e6ed1a4b5ec85ccf64d69a108d88) ) /* F-E1-16-010 PCB */

	ROM_REGION( 0x800000, "gfx", 0 )  /* gfx data */
	ROM_LOAD32_WORD( "roml00", 0x000000, 0x200000, CRC(c677aac3) SHA1(356073a29260e8e6c29dd12b2113b30140c6108c) )
	ROM_LOAD32_WORD( "romh00", 0x000002, 0x200000, CRC(b6337d4a) SHA1(2f46e2933af7fd0f71083900d5e6e4f602ab4c66) )
	/* roml01 empty */
	/* romh01 empty */
	/* roml02 empty */
	/* romh02 empty */
	/* roml03 empty */
	/* romh03 empty */

	/* $00000-$20000 stays the same in all sound banks, */
	/* the second half of the bank is what gets switched */
	ROM_REGION( 0x80000, "oki1", 0 ) /* Oki Samples */
	ROM_LOAD( "11-semicom.vrom1", 0x000000, 0x080000, CRC(e8141fcd) SHA1(256fd1987030e0a1df0a66a228c1fea996cda686) ) /* same data as above */
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
	ROM_REGION32_BE( 0x400000, "maincpu", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	ROM_LOAD16_WORD_SWAP( "rom1", 0x000000, 0x200000, CRC(2e55ff55) SHA1(b2b7605b87ee609dfbc7c21dfae0ef8d847019f0) )
	ROM_LOAD16_WORD_SWAP( "rom2", 0x200000, 0x200000, CRC(50f8a409) SHA1(a8171b7cf59dd01de1e512ab21607b4f330f40b8) )

	ROM_REGION( 0x4000000, "gfx", 0 ) /* 16x16x8 Sprites */
	ROM_LOAD32_WORD( "g05", 0x0000002, 0x800000, CRC(64c8f493) SHA1(d487a74c813abbd0a612f8346eed8a7c3ff3e84e) )
	ROM_LOAD32_WORD( "g09", 0x0000000, 0x800000, CRC(c359febb) SHA1(7955385748e24dd076bc4f954b193a53c0a729c5) )
	ROM_LOAD32_WORD( "g06", 0x1000002, 0x800000, CRC(ffbc9fe5) SHA1(5e0e5cfdf6af23db0733c9fedee9c5f9ccde1109) )
	ROM_LOAD32_WORD( "g10", 0x1000000, 0x800000, CRC(08217573) SHA1(10cecdfc3a1ef835a62325b023d3bca8d0aea67d) )
	ROM_LOAD32_WORD( "g07", 0x2000002, 0x800000, CRC(5cb3c86a) SHA1(2e89f467c1a220f2510977677215e040295c3dd0) )
	ROM_LOAD32_WORD( "g11", 0x2000000, 0x800000, CRC(5f0461b8) SHA1(a0ac37d9a357e69367b8fee68bc358bfea5ecca0) )
	ROM_LOAD32_WORD( "g08", 0x3000002, 0x800000, CRC(1fd08aa0) SHA1(376a91220cd6e63418b04d590b232bb1079a40c7) )
	ROM_LOAD32_WORD( "g12", 0x3000000, 0x800000, CRC(e437b35f) SHA1(411d2926d619fba057476864f0e580f608830522) )

	ROM_REGION( 0x40000, "oki1", 0 ) /* Oki Samples */
	ROM_LOAD( "rom3", 0x00000, 0x40000, CRC(db8cb455) SHA1(6723b4018208d554bd1bf1e0640b72d2f4f47302) )

	/* $00000-$20000 stays the same in all sound banks, */
	/* the second half of the bank is what gets switched */
	ROM_REGION( 0x80000, "oki2", 0 ) /* Oki Samples */
	ROM_LOAD( "rom4", 0x00000, 0x80000, CRC(bba47755) SHA1(e6eeb5f64eaa88a74536119b731a76921e79f8ff) )
ROM_END

/*

Boong-Ga Boong-Ga (Spank'em!)
Taff System, 2001

TAFF SYSTEM
+-----------------------------------------------+
|     VR1               VROM2                   |
|               YM3012  VROM1  L04*L09* U04*U09*|
|               YM2151  M6295  L03 L08* U03 U08*|
|               CRAM1          L02 L07  U02 U07 |
|               CRAM2          L01 L06  U01 U06 |
|               MEM1L          L00 L05  U00 U05 |
|J              MEM1U                           |
|A              MEM3  +----------++----------+  |
|M                    |          ||          |  |
|M              MEM2  |Quicklogic||Quicklogic|  |
|A                    | QL2003-  || QL2003-  |  |
|               MEM7  | XPL84C   || XPL84C   |  |
|                     |          ||          |  |
|               MEM6  +----------++----------+  |
|           M3     93C46        GAL1     280MHz |
|           M2                                  |
|CN2        M1  P1   DRAM1  E1-16T  ROM0  ROM2* |
|CN3  AL00 AL01 P2          50MHz   ROM1  ROM3* |
+-----------------------------------------------+

NOTE: All L0x & H0x are silkscreened on the PCB as ROML0x & ROMH0x

Notes:
CPU - Hyperstone E1-16T @ 50.000MHz

DRAM1 - LG Semi GM71C18163 1M x16 EDO DRAM (SOJ44)
CRAMx - W24M257AK-15 32K x8 SRAM (SOJ28)
MEMx  - UM61256FK-15 32K x8 SRAM (SOJ28)
GAL1  - GAL22V10B

Oki M6295 rebadged as AD-65
YM3012/YM2151

 P1 - Reset push button
 P2 - Setup push button
VR1 - Volume adjust pot
 M1 - 4-Pin header silkscreened MOTOR1
 M2 - 4-Pin header silkscreened MOTOR2
 M3 - 4-Pin header silkscreened MOTOR3
CN2 - 4-Pin Header
CN3 - 2-Pin Header
AL00 - 10-Pin Header
AL01 - 10-Pin Header

ROMs:
    ROML00/ROMH00 & ROML05/ROMH05 - Macronix MX29F1610MC-12 SOP44 16MBit FlashROM
    ROML01/ROMH01 & ROML06/ROMH06 - Macronix MX29F1610MC-12 SOP44 16MBit FlashROM
    ROML02/ROMH02 & ROML07/ROMH07 - Macronix MX29F1610MC-12 SOP44 16MBit FlashROM
    ROML03/ROMH03                 - Macronix MX29F1610MC-12 SOP44 16MBit FlashROM
  * ROML08/ROMH08                 - Unpopulated space for MX29F1610MC-12 SOP44 16MBit FlashROM
  * ROML04/ROMH04 & ROML09/ROMH09 - Unpopulated space for MX29F1610MC-12 SOP44 16MBit FlashROM
    VROM1/VROM2                   - ST M27C4001 4MBit DIP32 EPROM
    ROM0/ROM1                     - ST M27C4001 4MBit DIP32 EPROM
  * ROM2/ROM3                     - Unpopulated space for DIP32 EPROM (up to 4MBit)

*/

ROM_START( boonggab )
	ROM_REGION16_BE( 0x100000, "maincpu", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	/* rom2 empty */
	/* rom3 empty */
	ROM_LOAD( "2.rom0",       0x80000, 0x80000, CRC(3395541b) SHA1(4e822a52d6070bde232285e7ad8fbe74594bbf28) )
	ROM_LOAD( "1.rom1",       0x00000, 0x80000, CRC(50522da1) SHA1(28f92fc818513d7a4934b9f8e5d39243d720cc80) )

	ROM_REGION( 0x2000000, "gfx", ROMREGION_ERASE00 )  /* 16x16x8 Sprites */
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

	/* $00000-$20000 stays the same in all sound banks, */
	/* the second half of the bank is what gets switched */
	ROM_REGION( 0x100000, "oki1", 0 ) /* Oki Samples */
	ROM_LOAD( "3.vrom1",      0x00000, 0x80000, CRC(0696bfcb) SHA1(bba61f3cae23271215bbbf8214ce3b73459d5da5) )
	ROM_LOAD( "4.vrom2",      0x80000, 0x80000, CRC(305c2b16) SHA1(fa199c4cd4ebb952d934e3863fca8740eeba9294) )
ROM_END

bool vamphalf_state::irq_active()
{
	const u32 FCR = m_maincpu->state_int(E132XS_FCR);
	if (!(FCR & (1 << 29))) // int 2 (irq 4)
		return true;
	else
		return false;
}

void vamphalf_state::banked_oki(int chip)
{
	assert((m_okiregion[chip].found()) && (m_okibank.found()));
	u8 *ROM = m_okiregion[chip]->base();
	const u32 size = m_okiregion[chip]->bytes();
	if (size > 0x40000)
		m_okibank->configure_entries(0, size / 0x20000, &ROM[0], 0x20000);
	else
		m_okibank->set_base(&ROM[0x20000]);
}

template <u32 Pc, u32 Wram_offs>
u16 vamphalf_state::speedup_16_r()
{
	if (m_maincpu->pc() == Pc)
	{
		if (irq_active())
			m_maincpu->spin_until_interrupt();
		else
			m_maincpu->eat_cycles(50);
	}

	return m_wram[Wram_offs / 2];
}

template <u32 Pc, u32 Wram_offs>
u32 vamphalf_state::speedup_32_r()
{
	if (m_maincpu->pc() == Pc)
	{
		if (irq_active())
			m_maincpu->spin_until_interrupt();
		else
			m_maincpu->eat_cycles(50);
	}

	return m_wram32[Wram_offs / 4];
}

u32 vamphalf_state::aoh_speedup_r()
{
	if (m_maincpu->pc() == 0xb994 || m_maincpu->pc() == 0xba40)
	{
		m_maincpu->eat_cycles(500);
	}

	return m_wram32[0x28a09c / 4];
}

void vamphalf_state::init_vamphalf()
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x0004a7b8, 0x0004a7b9, emu::rw_delegate(*this, NAME((&vamphalf_state::speedup_16_r<0x82ec, 0x4a7b8>))));

	m_palshift = 0;
	m_flip_bit = 0x80;
}

void vamphalf_state::init_vamphalfr1()
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x0004a468, 0x0004a469, emu::rw_delegate(*this, NAME((&vamphalf_state::speedup_16_r<0x82ec, 0x4a468>))));

	m_palshift = 0;
	m_flip_bit = 0x80;
}

void vamphalf_state::init_vamphafk()
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x0004a648, 0x0004a649, emu::rw_delegate(*this, NAME((&vamphalf_state::speedup_16_r<0x82ec, 0x4a648>))));

	m_palshift = 0;
	m_flip_bit = 0x80;
}

void vamphalf_qdsp_state::init_misncrft()
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x000741e8, 0x000741e9, emu::rw_delegate(*this, NAME((&vamphalf_qdsp_state::speedup_16_r<0xff5a, 0x741e8>))));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x00072e2c, 0x00072e2d, emu::rw_delegate(*this, NAME((&vamphalf_qdsp_state::speedup_16_r<0xecd6, 0x72e2c>))));
	m_palshift = 0;
	m_flip_bit = 1;

	// Configure the QS1000 ROM banking. Care must be taken not to overlap the 256b internal RAM
	m_qdsp_cpu->space(AS_IO).install_read_bank(0x0100, 0xffff, m_qs1000_bank);
	m_qs1000_bank->configure_entries(0, 16, memregion("qs1000:cpu")->base() + 0x100, 0x8000-0x100);
}

void vamphalf_state::init_coolmini()
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x000d2df8, 0x000d2df9, emu::rw_delegate(*this, NAME((&vamphalf_state::speedup_16_r<0x75f88, 0xd2df8>))));

	m_palshift = 0;
	m_flip_bit = 1;
}

void vamphalf_state::init_coolminii()
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x000d30a8, 0x000d30a9, emu::rw_delegate(*this, NAME((&vamphalf_state::speedup_16_r<0x76024, 0xd30a8>))));

	m_palshift = 0;
	m_flip_bit = 1;
}

void vamphalf_state::init_mrkicker()
{
	banked_oki(0);
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x00063fc0, 0x00063fc1, emu::rw_delegate(*this, NAME((&vamphalf_state::speedup_16_r<0x41ec6, 0x63fc0>))));

	m_palshift = 0;
	m_flip_bit = 1;
}

void vamphalf_state::init_suplup()
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x0011605c, 0x0011605d, emu::rw_delegate(*this, NAME((&vamphalf_state::speedup_16_r<0xaf184, 0x11605c>))));

	m_palshift = 8;
	/* no flipscreen */
}

void vamphalf_state::init_luplup()
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x00115e84, 0x00115e85, emu::rw_delegate(*this, NAME((&vamphalf_state::speedup_16_r<0xaefac, 0x115e84>))));

	m_palshift = 8;
	/* no flipscreen */
}

void vamphalf_state::init_luplup29()
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x00113f08, 0x00113f09, emu::rw_delegate(*this, NAME((&vamphalf_state::speedup_16_r<0xae6c0, 0x113f08>))));

	m_palshift = 8;
	/* no flipscreen */
}

void vamphalf_state::init_luplup10()
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x00113b78, 0x00113b79, emu::rw_delegate(*this, NAME((&vamphalf_state::speedup_16_r<0xb1128, 0x113b78>))));

	m_palshift = 8;
	/* no flipscreen */
}

void vamphalf_state::init_puzlbang()
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x00113f14, 0x00113f15, emu::rw_delegate(*this, NAME((&vamphalf_state::speedup_16_r<0xae6cc, 0x113f14>))));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x00113ecc, 0x00113ecd, emu::rw_delegate(*this, NAME((&vamphalf_state::speedup_16_r<0xae6cc, 0x113ecc>))));

	m_palshift = 8;
	/* no flipscreen */
}

void vamphalf_qdsp_state::init_wyvernwg()
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x00b4cc4, 0x00b4cc7, emu::rw_delegate(*this, NAME((&vamphalf_qdsp_state::speedup_32_r<0x10766, 0xb4cc4>))));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x00b56f4, 0x00b56f7, emu::rw_delegate(*this, NAME((&vamphalf_qdsp_state::speedup_32_r<0x10766, 0xb56f4>))));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x00b74f0, 0x00b74f3, emu::rw_delegate(*this, NAME((&vamphalf_qdsp_state::speedup_32_r<0x10766, 0xb74f0>))));
	m_palshift = 0;
	m_flip_bit = 1;

	m_semicom_prot_idx = 8;
	m_semicom_prot_data[0] = 2;
	m_semicom_prot_data[1] = 1;

	// Configure the QS1000 ROM banking. Care must be taken not to overlap the 256b internal RAM
	m_qdsp_cpu->space(AS_IO).install_read_bank(0x0100, 0xffff, m_qs1000_bank);
	m_qs1000_bank->configure_entries(0, 16, memregion("qs1000:cpu")->base() + 0x100, 0x8000-0x100);

	save_item(NAME(m_semicom_prot_idx));
	save_item(NAME(m_semicom_prot_which));
}

void vamphalf_qdsp_state::init_yorijori()
{
	// it's close to Final Godori in terms of port mappings, possibly a SemiCom game?

	m_palshift = 0;
	m_flip_bit = 1;

	m_semicom_prot_idx = 8;
	m_semicom_prot_data[0] = 2;
	m_semicom_prot_data[1] = 3;

	u8 *romx = (u8 *)memregion("maincpu")->base();
	// prevent code dying after a trap 33 by patching it out, why?
	romx[BYTE4_XOR_BE(0x8ff0)] = 3;
	romx[BYTE4_XOR_BE(0x8ff1)] = 0;

	// Configure the QS1000 ROM banking. Care must be taken not to overlap the 256b internal RAM
	m_qdsp_cpu->space(AS_IO).install_read_bank(0x0100, 0xffff, m_qs1000_bank);
	m_qs1000_bank->configure_entries(0, 16, memregion("qs1000:cpu")->base() + 0x100, 0x8000-0x100);
}

void vamphalf_nvram_state::init_finalgdr()
{
	banked_oki(0);
	m_finalgdr_backupram_bank = 1;
	m_finalgdr_backupram = std::make_unique<u8[]>(0x80*0x100);
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x005e870, 0x005e873, emu::rw_delegate(*this, NAME((&vamphalf_nvram_state::speedup_32_r<0x1c20c, 0x5e870>))));
	m_nvram->set_base(m_finalgdr_backupram.get(), 0x80*0x100);

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

void vamphalf_nvram_state::init_mrkickera()
{
	banked_oki(0);
	// backup ram isn't used
	m_finalgdr_backupram_bank = 1;
	m_finalgdr_backupram = std::make_unique<u8[]>(0x80*0x100);
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x00701a0, 0x00701a3, emu::rw_delegate(*this, NAME((&vamphalf_nvram_state::speedup_32_r<0x46a30, 0x701a0>))));
	m_nvram->set_base(m_finalgdr_backupram.get(), 0x80*0x100);

	m_palshift = 0;
	m_flip_bit = 1; //?

	m_semicom_prot_idx = 8;
	m_semicom_prot_data[0] = 2;
	m_semicom_prot_data[1] = 3;

	save_item(NAME(m_semicom_prot_idx));
	save_item(NAME(m_semicom_prot_which));
}

void vamphalf_state::init_dquizgo2()
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x00cdde8, 0x00cdde9, emu::rw_delegate(*this, NAME((&vamphalf_state::speedup_16_r<0xaa630, 0xcdde8>))));

	m_palshift = 0;
	m_flip_bit = 1;
}

void vamphalf_state::init_dtfamily()
{
	banked_oki(0);
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xcc2a8, 0xcc2a9, emu::rw_delegate(*this, NAME((&vamphalf_state::speedup_16_r<0x12fa6, 0xcc2a8>))));

	m_palshift = 0;
	m_flip_bit = 1;
}


void vamphalf_state::init_toyland()
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x780d8, 0x780d9, emu::rw_delegate(*this, NAME((&vamphalf_state::speedup_16_r<0x130c2, 0x780d8>))));

	m_palshift = 0;
	m_flip_bit = 1;
}

void vamphalf_state::init_aoh()
{
	banked_oki(1);
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x028a09c, 0x028a09f, emu::rw_delegate(*this, FUNC(vamphalf_state::aoh_speedup_r)));

	m_palshift = 0;
	/* no flipscreen */
}

void vamphalf_state::init_jmpbreak()
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x00906f4, 0x00906f5, emu::rw_delegate(*this, NAME((&vamphalf_state::speedup_16_r<0x984a, 0x906f4>))));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xe0000000, 0xe0000003, emu::rw_delegate(*this, FUNC(vamphalf_state::jmpbreak_flipscreen_w)));

	m_palshift = 0;
}

void vamphalf_state::init_jmpbreaka()
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x00e1dfc, 0x00e1dfd, emu::rw_delegate(*this, NAME((&vamphalf_state::speedup_16_r<0x909ac, 0xe1dfc>))));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xe0000000, 0xe0000003, emu::rw_delegate(*this, FUNC(vamphalf_state::jmpbreak_flipscreen_w)));

	m_palshift = 0;
}

void vamphalf_state::init_mrdig()
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x0e0768, 0x0e0769, emu::rw_delegate(*this, NAME((&vamphalf_state::speedup_16_r<0xae38, 0xe0768>))));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xe0000000, 0xe0000003, emu::rw_delegate(*this, FUNC(vamphalf_state::jmpbreak_flipscreen_w)));

	m_palshift = 0;
}

void vamphalf_state::init_poosho()
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x0c8b58, 0x0c8b59, emu::rw_delegate(*this, NAME((&vamphalf_state::speedup_16_r<0xa8c78, 0xc8b58>))));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xe0000000, 0xe0000003, emu::rw_delegate(*this, FUNC(vamphalf_state::jmpbreak_flipscreen_w)));

	m_palshift = 0;
}

void vamphalf_state::init_newxpang()
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x061218, 0x061219, emu::rw_delegate(*this, NAME((&vamphalf_state::speedup_16_r<0x8b8e, 0x61218>))));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xe0000000, 0xe0000003, emu::rw_delegate(*this, FUNC(vamphalf_state::jmpbreak_flipscreen_w)));

	m_palshift = 0;
}

void vamphalf_state::init_newxpanga()
{
	// m_maincpu->space(AS_PROGRAM).install_read_handler(0x061218, 0x061219, emu::rw_delegate(*this, NAME((&vamphalf_state::speedup_16_r<, >)))); // TODO
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xe0000000, 0xe0000003, emu::rw_delegate(*this, FUNC(vamphalf_state::jmpbreak_flipscreen_w)));

	m_palshift = 0;
}

void vamphalf_state::init_worldadv()
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x0c5e78, 0x0c5e79, emu::rw_delegate(*this, NAME((&vamphalf_state::speedup_16_r<0x93ae, 0xc5e78>))));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xe0000000, 0xe0000003, emu::rw_delegate(*this, FUNC(vamphalf_state::jmpbreak_flipscreen_w)));

	m_palshift = 0;
}

void vamphalf_state::init_solitaire()
{

	// TODO: speedup
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x05d1c0, 0x05d1c1, emu::rw_delegate(*this, NAME((&vamphalf_state::speedup_16_r<0x8810, 0x5d1c0>))));

	m_palshift = 0;
}

void vamphalf_state::init_boonggab()
{
	banked_oki(0);
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x000f1b74, 0x000f1b75, emu::rw_delegate(*this, NAME((&vamphalf_state::speedup_16_r<0x131a6, 0xf1b74>))));

	m_palshift = 0;
	m_has_extra_gfx = true;
	m_flip_bit = 1;
}

} // anonymous namespace


GAME( 1999, coolmini,   0,        coolmini,  common,    vamphalf_state,      init_coolmini,  ROT0,   "SemiCom",                       "Cool Minigame Collection", MACHINE_SUPPORTS_SAVE )
GAME( 1999, coolminii,  coolmini, coolmini,  common,    vamphalf_state,      init_coolminii, ROT0,   "SemiCom",                       "Cool Minigame Collection (Italy)", MACHINE_SUPPORTS_SAVE )

GAME( 1999, jmpbreak,   0,        jmpbreak,  common,    vamphalf_state,      init_jmpbreak,  ROT0,   "F2 System",                     "Jumping Break (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, jmpbreaka,  jmpbreak, jmpbreak,  common,    vamphalf_state,      init_jmpbreaka, ROT0,   "F2 System",                     "Jumping Break (set 2)", MACHINE_SUPPORTS_SAVE )

GAME( 1999, poosho,     0,        jmpbreak,  common,    vamphalf_state,      init_poosho,    ROT0,   "F2 System",                     "Poosho Poosho", MACHINE_SUPPORTS_SAVE )

GAME( 1999, newxpang,   0,        newxpang,  common,    vamphalf_state,      init_newxpang,  ROT0,   "F2 System",                     "New Cross Pang (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, newxpanga,  newxpang, jmpbreak,  common,    vamphalf_state,      init_newxpanga, ROT0,   "F2 System",                     "New Cross Pang (set 2)", MACHINE_SUPPORTS_SAVE ) // TODO: speed up for this set

GAME( 1999, worldadv,   0,        worldadv,  common,    vamphalf_state,      init_worldadv,  ROT0,   "Logic / F2 System",             "World Adventure", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING ) // game starts to stall for several seconds at a time after it's been running for a certain amount of time

GAME( 1999, solitaire,  0,        solitaire, solitaire, vamphalf_state,      init_solitaire, ROT0,   "F2 System",                     "Solitaire (version 2.5)", MACHINE_SUPPORTS_SAVE )

GAME( 1999, suplup,     0,        suplup,    common,    vamphalf_state,      init_suplup,    ROT0,   "Omega System",                  "Super Lup Lup Puzzle / Zhuan Zhuan Puzzle (version 4.0 / 990518)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, luplup,     suplup,   suplup,    common,    vamphalf_state,      init_luplup,    ROT0,   "Omega System",                  "Lup Lup Puzzle / Zhuan Zhuan Puzzle (version 3.0 / 990128)",       MACHINE_SUPPORTS_SAVE )
GAME( 1999, luplup29,   suplup,   suplup,    common,    vamphalf_state,      init_luplup29,  ROT0,   "Omega System",                  "Lup Lup Puzzle / Zhuan Zhuan Puzzle (version 2.9 / 990108)",       MACHINE_SUPPORTS_SAVE )
GAME( 1999, luplup10,   suplup,   suplup,    common,    vamphalf_state,      init_luplup10,  ROT0,   "Omega System (Adko license)",   "Lup Lup Puzzle / Zhuan Zhuan Puzzle (version 1.05 / 981214)",      MACHINE_SUPPORTS_SAVE )
GAME( 1999, puzlbang,   suplup,   suplup,    common,    vamphalf_state,      init_puzlbang,  ROT0,   "Omega System",                  "Puzzle Bang Bang (Korea, version 2.9 / 990108)",                   MACHINE_SUPPORTS_SAVE )
GAME( 1999, puzlbanga,  suplup,   suplup,    common,    vamphalf_state,      init_puzlbang,  ROT0,   "Omega System",                  "Puzzle Bang Bang (Korea, version 2.8 / 990106)",                   MACHINE_SUPPORTS_SAVE )

GAME( 1999, vamphalf,   0,        vamphalf,  common,    vamphalf_state,      init_vamphalf,  ROT0,   "Danbi / F2 System",             "Vamf x1/2 (Europe, version 1.1.0908)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, vamphalfr1, vamphalf, vamphalf,  common,    vamphalf_state,      init_vamphalfr1,ROT0,   "Danbi / F2 System",             "Vamf x1/2 (Europe, version 1.0.0903)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, vamphalfk,  vamphalf, vamphalf,  common,    vamphalf_state,      init_vamphafk,  ROT0,   "Danbi / F2 System",             "Vamp x1/2 (Korea, version 1.1.0908)",  MACHINE_SUPPORTS_SAVE )

GAME( 2000, dquizgo2,   0,        coolmini,  common,    vamphalf_state,      init_dquizgo2,  ROT0,   "SemiCom",                       "Date Quiz Go Go Episode 2", MACHINE_SUPPORTS_SAVE )

GAME( 2000, misncrft,   0,        misncrft,  common,    vamphalf_qdsp_state, init_misncrft,  ROT90,  "Sun",                           "Mission Craft (version 2.7)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING ) // game starts to stall for several seconds at a time after it's been running for a certain amount of time (you can usually complete 1 loop)
GAME( 2000, misncrfta,  misncrft, misncrft,  common,    vamphalf_qdsp_state, init_misncrft,  ROT90,  "Sun",                           "Mission Craft (version 2.4)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )

GAME( 2000, mrdig,      0,        mrdig,     common,    vamphalf_state,      init_mrdig,     ROT0,   "Sun",                           "Mr. Dig", MACHINE_SUPPORTS_SAVE )

GAME( 2001, dtfamily,   0,        mrkicker,  common,    vamphalf_state,      init_dtfamily,  ROT0,   "SemiCom",                       "Diet Family", MACHINE_SUPPORTS_SAVE )

GAME( 2001, finalgdr,   0,        finalgdr,  finalgdr,  vamphalf_nvram_state,init_finalgdr,  ROT0,   "SemiCom",                       "Final Godori (Korea, version 2.20.5915)", MACHINE_SUPPORTS_SAVE )

GAME( 2001, mrkicker,   0,        mrkicker,  common,    vamphalf_state,      init_mrkicker,  ROT0,   "SemiCom",                       "Mr. Kicker (F-E1-16-010 PCB)",  MACHINE_SUPPORTS_SAVE )
GAME( 2001, mrkickera,  mrkicker, mrkickera, finalgdr,  vamphalf_nvram_state,init_mrkickera, ROT0,   "SemiCom",                       "Mr. Kicker (SEMICOM-003b PCB)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING ) // if you allow EEPROM saving, then this set corrupts the EEPROM and then won't boot

GAME( 2001, toyland,    0,        coolmini,  common,    vamphalf_state,      init_toyland,   ROT0,   "SemiCom",                       "Toy Land Adventure", MACHINE_SUPPORTS_SAVE )

GAME( 2001, wivernwg,   0,        wyvernwg,  common,    vamphalf_qdsp_state, init_wyvernwg,  ROT270, "SemiCom",                       "Wivern Wings",         MACHINE_SUPPORTS_SAVE | MACHINE_UNEMULATED_PROTECTION ) // gives a protection error after a certain number of plays / coins?
GAME( 2001, wyvernwg,   wivernwg, wyvernwg,  common,    vamphalf_qdsp_state, init_wyvernwg,  ROT270, "SemiCom (Game Vision license)", "Wyvern Wings (set 1)", MACHINE_SUPPORTS_SAVE | MACHINE_UNEMULATED_PROTECTION )
GAME( 2001, wyvernwga,  wivernwg, wyvernwg,  common,    vamphalf_qdsp_state, init_wyvernwg,  ROT270, "SemiCom (Game Vision license)", "Wyvern Wings (set 2)", MACHINE_SUPPORTS_SAVE | MACHINE_UNEMULATED_PROTECTION )

GAME( 2001, aoh,        0,        aoh,       aoh,       vamphalf_state,      init_aoh,       ROT0,   "Unico",                         "Age Of Heroes - Silkroad 2 (v0.63 - 2001/02/07)", MACHINE_SUPPORTS_SAVE )

GAME( 2001, boonggab,   0,        boonggab,  boonggab,  vamphalf_state,      init_boonggab,  ROT270, "Taff System",                   "Boong-Ga Boong-Ga (Spank'em!)", MACHINE_SUPPORTS_SAVE )

GAME( 2002, yorijori,   0,        yorijori,  yorijori,  vamphalf_qdsp_state, init_yorijori,  ROT0,   "Golden Bell Entertainment",     "Yori Jori Kuk Kuk", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // ROM patch needed to boot
