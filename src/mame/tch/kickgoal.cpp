// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Driver Info

Kick Goal (c)1995 TCH
Action Hollywood (c)1995 TCH

driver by David Haywood


todo:
  PIC from Action Hollywood still needs deprotecting + dumping
  Both games have problems with the Eeprom (settings are not saved)


Main board: PRO-3/B
+-------------------------------------------+
|       IC13   PIC16C57                16MHz|
|VOL    M6295                   +----+ 65764|
|                               |1020|      |
|                               +----+ 65764|
|J    65764 65764                           |
|A    65764 65764               +----+ IC36 |
|M                              |1020| IC35 |
|M                              +----+ IC34 |
|A    62256 62256               +----+ IC33 |
|  SW   IC5 IC6                 |1020| IC32 |
|12MHz MC68000P12               +----+ IC31 |
|MACH1111                62256         IC30 |
|     93C46N             62256         IC29 |
+-------------------------------------------+

   CPU: MC68000P12
        PIC16C57-XT/P
 Sound: OKI M6295 (rebadged as U6295)
   OSC: 16.000MHz, 12.000MHz
   RAM: UM62256D-70LL - 32Kx8 SRAM (x4)
        HM3-65764E-5 - 8Kx8 SRAM (x6)
EEPROM: 93C46N
 Other: TI TPC1020BFN-084C (x3)
        AMD MACH111-15CJ
        VOL - volume pot
        SW - push button service switch

Measured clocks:
    68000 - 12MHz
 PIC16C57 - 4MHz
    M6295 - 1MHz
*/

/* Notes

68k interrupts
lev 1 : 0x64 : 0000 0000 - x
lev 2 : 0x68 : 0000 0000 - x
lev 3 : 0x6c : 0000 0000 - x
lev 4 : 0x70 : 0000 0000 - x
lev 5 : 0x74 : 0000 0000 - x
lev 6 : 0x78 : 0000 0510 - vblank?
lev 7 : 0x7c : 0000 0000 - x

*/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "cpu/pic16c5x/pic16c5x.h"
#include "machine/eepromser.h"
#include "machine/gen_latch.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class kickgoal_state : public driver_device
{
public:
	kickgoal_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_fgram(*this, "fgram"),
		m_bgram(*this, "bgram"),
		m_bg2ram(*this, "bg2ram"),
		m_scrram(*this, "scrram"),
		m_spriteram(*this, "spriteram"),
		m_eeprom(*this, "eeprom") ,
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_oki(*this, "oki"),
		m_okibank(*this, "okibank"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void kickgoal(machine_config &config) ATTR_COLD;
	void actionhw(machine_config &config) ATTR_COLD;

	void init_kickgoal() ATTR_COLD;
	void init_actionhw() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void fgram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void bgram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void bg2ram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void actionhw_snd_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	void soundio_port_a_w(u8 data);
	u8 soundio_port_b_r();
	void soundio_port_b_w(u8 data);
	u8 soundio_port_c_r();
	void soundio_port_c_w(u8 data);
	void to_pic_w(u16 data);

	TILE_GET_INFO_MEMBER(get_kickgoal_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg2_tile_info);
	TILE_GET_INFO_MEMBER(get_actionhw_fg_tile_info);
	TILEMAP_MAPPER_MEMBER(tilemap_scan_8x8);
	TILEMAP_MAPPER_MEMBER(tilemap_scan_16x16);
	TILEMAP_MAPPER_MEMBER(tilemap_scan_32x32);
	DECLARE_VIDEO_START(kickgoal);
	DECLARE_VIDEO_START(actionhw);

	INTERRUPT_GEN_MEMBER(kickgoal_interrupt);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;
	void oki_map(address_map &map) ATTR_COLD;

	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);

	/* video-related */
	tilemap_t *m_fgtm = nullptr;
	tilemap_t *m_bgtm = nullptr;
	tilemap_t *m_bg2tm = nullptr;

	/* misc */
	int m_snd_new = 0;
	int m_snd_sam[4]{};

	u8 m_pic_portc = 0;
	u8 m_pic_portb = 0;
	int m_sound_command_sent = 0;

	int m_fg_base = 0;

	int m_bg_base = 0;
	int m_bg_mask = 0;

	int m_bg2_base = 0;
	int m_bg2_mask = 0;
	int m_bg2_region = 0;

	int m_sprbase = 0;

	/* memory pointers */
	required_shared_ptr<u16> m_fgram;
	required_shared_ptr<u16> m_bgram;
	required_shared_ptr<u16> m_bg2ram;
	required_shared_ptr<u16> m_scrram;
	required_shared_ptr<u16> m_spriteram;

	/* devices */
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<cpu_device> m_maincpu;
	required_device<pic16c57_device> m_audiocpu;
	required_device<okim6295_device> m_oki;
	required_memory_bank m_okibank;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
};

void kickgoal_state::fgram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_fgram[offset]);
	m_fgtm->mark_tile_dirty(offset / 2);
}

void kickgoal_state::bgram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_bgram[offset]);
	m_bgtm->mark_tile_dirty(offset / 2);
}

void kickgoal_state::bg2ram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_bg2ram[offset]);
	m_bg2tm->mark_tile_dirty(offset / 2);
}

/* FG */
TILE_GET_INFO_MEMBER(kickgoal_state::get_kickgoal_fg_tile_info)
{
	u16 const tileno = m_fgram[tile_index * 2] & 0x0fff;
	u16 const color  = m_fgram[tile_index * 2 + 1] & 0x000f;

	tileinfo.set(BIT(tile_index, 5) ? 3 : 0, tileno + m_fg_base, color + 0x00, 0); // similar 8x8 gfx behavior as CPS1
}

TILE_GET_INFO_MEMBER(kickgoal_state::get_actionhw_fg_tile_info)
{
	u16 const tileno = m_fgram[tile_index * 2] & 0x0fff;
	u16 const color  = m_fgram[tile_index * 2 + 1] & 0x000f;

	tileinfo.set(0, tileno + m_fg_base, color + 0x00, 0);
}

/* BG */
TILE_GET_INFO_MEMBER(kickgoal_state::get_bg_tile_info)
{
	u16 const tileno = m_bgram[tile_index * 2] & m_bg_mask;
	u16 const color  = m_bgram[tile_index * 2 + 1] & 0x000f;
	bool const flipx = m_bgram[tile_index * 2 + 1] & 0x0020;
	bool const flipy = m_bgram[tile_index * 2 + 1] & 0x0040;

	tileinfo.set(1, tileno + m_bg_base, color + 0x10, (flipx ? TILE_FLIPX : 0) | (flipy ? TILE_FLIPY : 0));
}

/* BG 2 */
TILE_GET_INFO_MEMBER(kickgoal_state::get_bg2_tile_info)
{
	u16 const tileno = m_bg2ram[tile_index * 2] & m_bg2_mask;
	u16 const color  = m_bg2ram[tile_index * 2 + 1] & 0x000f;
	bool const flipx = m_bg2ram[tile_index * 2 + 1] & 0x0020;
	bool const flipy = m_bg2ram[tile_index * 2 + 1] & 0x0040;

	tileinfo.set(m_bg2_region, tileno + m_bg2_base, color + 0x20, (flipx ? TILE_FLIPX : 0) | (flipy ? TILE_FLIPY : 0));
}


TILEMAP_MAPPER_MEMBER(kickgoal_state::tilemap_scan_8x8)
{
	/* logical (col,row) -> memory offset */
	return (row & 0x1f) | ((col & 0x3f) << 5) | ((row & 0x20) << 6);
}

TILEMAP_MAPPER_MEMBER(kickgoal_state::tilemap_scan_16x16)
{
	/* logical (col,row) -> memory offset */
	return (row & 0xf) | ((col & 0x3f) << 4) | ((row & 0x30) << 6);
}

TILEMAP_MAPPER_MEMBER(kickgoal_state::tilemap_scan_32x32)
{
	/* logical (col,row) -> memory offset */
	return (row & 0x7) | ((col & 0x3f) << 3) | ((row & 0x38) << 6);
}


void kickgoal_state::draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	for (int offs = 0; offs < m_spriteram.bytes() / 2; offs += 4)
	{
		int xpos         = m_spriteram[offs + 3];
		int ypos         = m_spriteram[offs + 0] & 0x00ff;
		u16 const tileno = m_spriteram[offs + 2] & 0x3fff;
		bool const flipx = m_spriteram[offs + 1] & 0x0020;
		u16 const color  = m_spriteram[offs + 1] & 0x000f;

		if (m_spriteram[offs + 0] & 0x0100) break;

		ypos = 0x110 - ypos;

		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
				tileno + m_sprbase,
				0x30 + color,
				flipx,0,
				xpos-16+4,ypos-32,15);
	}
}


VIDEO_START_MEMBER(kickgoal_state,kickgoal)
{
	m_sprbase = 0x0000;

	m_fg_base = 0x7000;
	m_bg_base = 0x1000;
	m_bg_mask = 0x0fff;

	m_bg2_region = 2; // 32x32 tile source
	m_bg2_base = 0x2000 / 4;
	m_bg2_mask = (0x2000/4) - 1;

	m_fgtm = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(kickgoal_state::get_kickgoal_fg_tile_info)), tilemap_mapper_delegate(*this, FUNC(kickgoal_state::tilemap_scan_8x8)), 8, 8, 64, 64);
	m_bgtm = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(kickgoal_state::get_bg_tile_info)), tilemap_mapper_delegate(*this, FUNC(kickgoal_state::tilemap_scan_16x16)), 16, 16, 64, 64);
	m_bg2tm = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(kickgoal_state::get_bg2_tile_info)), tilemap_mapper_delegate(*this, FUNC(kickgoal_state::tilemap_scan_32x32)), 32, 32, 64, 64);

	m_fgtm->set_transparent_pen(15);
	m_bgtm->set_transparent_pen(15);
}

VIDEO_START_MEMBER(kickgoal_state,actionhw)
{
	m_sprbase = 0x4000;
	m_fg_base = 0x7000 * 2;

	m_bg_base = 0x0000;
	m_bg_mask = 0x1fff;

	m_bg2_region = 1; // 16x16 tile source
	m_bg2_base = 0x2000;
	m_bg2_mask = 0x2000 - 1;

	m_fgtm = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(kickgoal_state::get_actionhw_fg_tile_info)), tilemap_mapper_delegate(*this, FUNC(kickgoal_state::tilemap_scan_8x8)), 8, 8, 64, 64);
	m_bgtm = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(kickgoal_state::get_bg_tile_info)), tilemap_mapper_delegate(*this, FUNC(kickgoal_state::tilemap_scan_16x16)), 16, 16, 64, 64);
	m_bg2tm = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(kickgoal_state::get_bg2_tile_info)), tilemap_mapper_delegate(*this, FUNC(kickgoal_state::tilemap_scan_16x16)), 16, 16, 64, 64);

	m_fgtm->set_transparent_pen(15);
	m_bgtm->set_transparent_pen(15);
}


u32 kickgoal_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* set scroll */
	m_fgtm->set_scrollx(0, m_scrram[0]);
	m_fgtm->set_scrolly(0, m_scrram[1]);
	m_bgtm->set_scrollx(0, m_scrram[2]);
	m_bgtm->set_scrolly(0, m_scrram[3]);
	m_bg2tm->set_scrollx(0, m_scrram[4]);
	m_bg2tm->set_scrolly(0, m_scrram[5]);

	/* draw */
	m_bg2tm->draw(screen, bitmap, cliprect, 0, 0);
	m_bgtm->draw(screen, bitmap, cliprect, 0, 0);

	draw_sprites(bitmap, cliprect);

	m_fgtm->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}


/*

****************************************************************
Action Hollywood

01-19 Samples
21-26 Melodies Bank 0
41-48 Melodies Bank 1
61-63 Melodies Bank 2

*/

void kickgoal_state::actionhw_snd_w(offs_t offset, u16 data, u16 mem_mask)
{
	logerror("%s: Writing %04x to Sound CPU - mask %04x\n",machine().describe_context(),data,mem_mask);

	if (!ACCESSING_BITS_0_7)
		data >>= 8;

	switch (data)
	{
		case 0xfc:  m_okibank->set_entry(0); break;
		case 0xfd:  m_okibank->set_entry(2); break;
		case 0xfe:  m_okibank->set_entry(1); break;
		case 0xff:  m_okibank->set_entry(3); break;
		case 0x78:
				m_oki->write(data);
				m_snd_sam[0] = 00; m_snd_sam[1]= 00; m_snd_sam[2] = 00; m_snd_sam[3] = 00;
				break;
		default:
				if (m_snd_new) /* Play new sample */
				{
					if ((data & 0x80) && (m_snd_sam[3] != m_snd_new))
					{
						logerror("About to play sample %02x at vol %02x\n", m_snd_new, data);
						if ((m_oki->read() & 0x08) != 0x08)
						{
							logerror("Playing sample %02x at vol %02x\n", m_snd_new, data);
							m_oki->write(m_snd_new);
							m_oki->write(data);
						}
						m_snd_new = 00;
					}
					if ((data & 0x40) && (m_snd_sam[2] != m_snd_new))
					{
						logerror("About to play sample %02x at vol %02x\n", m_snd_new, data);
						if ((m_oki->read() & 0x04) != 0x04)
						{
							logerror("Playing sample %02x at vol %02x\n", m_snd_new, data);
							m_oki->write(m_snd_new);
							m_oki->write(data);
						}
						m_snd_new = 00;
					}
					if ((data & 0x20) && (m_snd_sam[1] != m_snd_new))
					{
						logerror("About to play sample %02x at vol %02x\n", m_snd_new, data);
						if ((m_oki->read() & 0x02) != 0x02)
						{
							logerror("Playing sample %02x at vol %02x\n", m_snd_new, data);
							m_oki->write(m_snd_new);
							m_oki->write(data);
						}
						m_snd_new = 00;
					}
					if ((data & 0x10) && (m_snd_sam[0] != m_snd_new))
					{
						logerror("About to play sample %02x at vol %02x\n", m_snd_new, data);
						if ((m_oki->read() & 0x01) != 0x01)
						{
							logerror("Playing sample %02x at vol %02x\n", m_snd_new, data);
							m_oki->write(m_snd_new);
							m_oki->write(data);
						}
						m_snd_new = 00;
					}
					break;
				}
				else if (data > 0x80) /* New sample command */
				{
					logerror("Next sample %02x\n", data);
					m_snd_new = data;
					break;
				}
				else /* Turn a channel off */
				{
					logerror("Turning channel %02x off\n", data);
					m_oki->write(data);
					if (data & 0x40) m_snd_sam[3] = 00;
					if (data & 0x20) m_snd_sam[2] = 00;
					if (data & 0x10) m_snd_sam[1] = 00;
					if (data & 0x08) m_snd_sam[0] = 00;
					m_snd_new = 00;
					break;
				}
	}
}


static const u16 kickgoal_default_eeprom_type1[64] = {
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
};


/* Memory Maps *****************************************************************/

void kickgoal_state::program_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();

	map(0x800000, 0x800fff).nopw(); // during startup

	map(0x800000, 0x800001).portr("P1_P2");
	map(0x800002, 0x800003).portr("SYSTEM");
	map(0x800004, 0x800005).w(FUNC(kickgoal_state::to_pic_w));

	map(0x880000, 0x89ffff).nopw(); // during startup

	map(0x900000, 0x90ffff).nopw(); // during startup
	map(0x900001, 0x900001).lw8(NAME([this] (u8 data) { m_eeprom->cs_write(BIT(data, 0)); }));
	map(0x900003, 0x900003).lw8(NAME([this] (u8 data) { m_eeprom->clk_write(BIT(data, 0)); }));
	map(0x900005, 0x900005).lw8(NAME([this] (u8 data) { m_eeprom->di_write(BIT(data, 0)); }));
	map(0x900007, 0x900007).lr8(NAME([this] () { return m_eeprom->do_read(); }));

	map(0xa00000, 0xa03fff).ram().w(FUNC(kickgoal_state::fgram_w)).share("fgram"); /* FG Layer */
	map(0xa04000, 0xa07fff).ram().w(FUNC(kickgoal_state::bgram_w)).share("bgram"); /* Higher BG Layer */
	map(0xa08000, 0xa0bfff).ram().w(FUNC(kickgoal_state::bg2ram_w)).share("bg2ram"); /* Lower BG Layer */
	map(0xa0c000, 0xa0ffff).ram(); // more tilemap?
	map(0xa10000, 0xa1000f).writeonly().share("scrram"); /* Scroll Registers */
	map(0xb00000, 0xb007ff).writeonly().share("spriteram"); /* Sprites */
	map(0xc00000, 0xc007ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette"); /* Palette */ // actionhw reads this
	map(0xff0000, 0xffffff).ram();
}


/* INPUT ports ***************************************************************/

static INPUT_PORTS_START( kickgoal )
	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x0800, IP_ACTIVE_LOW )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

/* GFX Decodes ***************************************************************/

static const gfx_layout layout_8x8 =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ STEP8(0,1) },
	{ STEP8(0,8*2) },
	16*8
};


static const gfx_layout layout_32x32 =
{
	32,32,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ STEP32(0,1) },
	{ STEP32(0,32) },
	32*32,
};

static GFXDECODE_START( gfx_kickgoal )
	GFXDECODE_ENTRY( "gfx1", 0, layout_8x8,          0x000, 0x40 ) // FG GFX for even column like CPS1
	GFXDECODE_ENTRY( "gfx1", 0, gfx_16x16x4_planar,  0x000, 0x40 )
	GFXDECODE_ENTRY( "gfx1", 0, layout_32x32,        0x000, 0x40 )
	GFXDECODE_ENTRY( "gfx1", 1, layout_8x8,          0x000, 0x40 ) // FG GFX for odd column like CPS1
GFXDECODE_END

static GFXDECODE_START( gfx_actionhw )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x4_planar,   0x000, 0x40 )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_16x16x4_planar, 0x000, 0x40 )
GFXDECODE_END

/* MACHINE drivers ***********************************************************/

void kickgoal_state::machine_start()
{
	save_item(NAME(m_snd_sam));
	save_item(NAME(m_snd_new));
	save_item(NAME(m_pic_portc));
	save_item(NAME(m_pic_portb));
	save_item(NAME(m_sound_command_sent));

	m_okibank->configure_entries(0, 4, memregion("oki")->base(), 0x20000);
	m_okibank->set_entry(1);
}

void kickgoal_state::machine_reset()
{
	m_snd_new = 0;
	m_snd_sam[0] = 0;
	m_snd_sam[1] = 0;
	m_snd_sam[2] = 0;
	m_snd_sam[3] = 0;

	m_pic_portc = 0x00;
	m_pic_portb = 0x00;
	m_sound_command_sent = 0x00;
}


void kickgoal_state::oki_map(address_map &map)
{
	map(0x00000, 0x1ffff).rom();
	map(0x20000, 0x3ffff).bankr("okibank");
}


void kickgoal_state::soundio_port_a_w(u8 data)
{
	// only time this ever gets a different value is the high score name entry, these banks are correct based on sample positions
	switch (data)
	{
	case 0x02: m_okibank->set_entry(1); break;
	case 0x01: m_okibank->set_entry(3); break;
	default: m_okibank->set_entry(2); break; // not used
	}
}

u8 kickgoal_state::soundio_port_b_r()
{
	return m_pic_portb;
}

void kickgoal_state::soundio_port_b_w(u8 data)
{
	m_pic_portb = data;
}

u8 kickgoal_state::soundio_port_c_r()
{
	// 0x20 = sound command ready?
	return (m_pic_portc & ~0x20) | m_sound_command_sent;
}

void kickgoal_state::soundio_port_c_w(u8 data)
{
	if ((data & 0x10) != (m_pic_portc & 0x10))
	{
		if (!(data & 0x10))
		{
			m_pic_portb = m_soundlatch->read();
			m_sound_command_sent = 0x00;
		}
	}

	if ((data & 0x01) != (m_pic_portc & 0x01))
	{
		if (!(data & 0x01))
		{
			m_pic_portb = m_oki->read();
		}
	}

	if ((data & 0x02) != (m_pic_portc & 0x02))
	{
		if (!(data & 0x02))
		{
			m_oki->write(m_pic_portb);
		}
	}

	m_pic_portc = data;
}


void kickgoal_state::to_pic_w(u16 data)
{
	m_soundlatch->write(data);
	m_sound_command_sent = 0x20;
}


void kickgoal_state::kickgoal(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(12'000'000));   /* 12 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &kickgoal_state::program_map);
	m_maincpu->set_vblank_int("screen", FUNC(kickgoal_state::irq6_line_hold));

	PIC16C57(config, m_audiocpu, XTAL(12'000'000)/3);  /* 4MHz */
	m_audiocpu->write_a().set(FUNC(kickgoal_state::soundio_port_a_w));
	m_audiocpu->read_b().set(FUNC(kickgoal_state::soundio_port_b_r));
	m_audiocpu->write_b().set(FUNC(kickgoal_state::soundio_port_b_w));
	m_audiocpu->read_c().set(FUNC(kickgoal_state::soundio_port_c_r));
	m_audiocpu->write_c().set(FUNC(kickgoal_state::soundio_port_c_w));

	config.set_perfect_quantum(m_maincpu);

	EEPROM_93C46_16BIT(config, "eeprom").default_data(kickgoal_default_eeprom_type1, 128);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(9*8, 55*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(kickgoal_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_kickgoal);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_444, 1024);

	MCFG_VIDEO_START_OVERRIDE(kickgoal_state,kickgoal)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	OKIM6295(config, m_oki, XTAL(12'000'000)/12, okim6295_device::PIN7_LOW);  /* 1MHz */
	m_oki->set_addrmap(0, &kickgoal_state::oki_map);
	m_oki->add_route(ALL_OUTPUTS, "mono", 0.80);
}

void kickgoal_state::actionhw(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(12'000'000)); /* verified on pcb */
	m_maincpu->set_addrmap(AS_PROGRAM, &kickgoal_state::program_map);
	m_maincpu->set_vblank_int("screen", FUNC(kickgoal_state::irq6_line_hold));

	PIC16C57(config, m_audiocpu, XTAL(12'000'000)/3);    /* verified on pcb */
	m_audiocpu->set_disable(); /* Disabled since the internal rom isn't dumped */
	/* Program and Data Maps are internal to the MCU */

	EEPROM_93C46_16BIT(config, "eeprom").default_data(kickgoal_default_eeprom_type1, 128);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(10*8+2, 54*8-1+2, 0*8, 30*8-1);
	screen.set_screen_update(FUNC(kickgoal_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_actionhw);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_444, 1024);

	MCFG_VIDEO_START_OVERRIDE(kickgoal_state,actionhw)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	OKIM6295(config, m_oki, XTAL(12'000'000)/12, okim6295_device::PIN7_HIGH); /* verified on pcb */
	m_oki->set_addrmap(0, &kickgoal_state::oki_map);
	m_oki->add_route(ALL_OUTPUTS, "mono", 0.80);
}


/* Rom Loading ***************************************************************/

ROM_START( kickgoal ) /* PRO-3/B pcb */
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "ic6",   0x000000, 0x40000, CRC(498ca792) SHA1(c638c3a1755870010c5961b58bcb02458ff4e238) )
	ROM_LOAD16_BYTE( "ic5",   0x000001, 0x40000, CRC(d528740a) SHA1(d56a71004aabc839b0833a6bf383e5ef9d4948fa) )

	ROM_REGION( 0x2000, "audiocpu", 0 ) /* sound */
	ROM_LOAD( "pic16c57",     0x0000, 0x1007, CRC(65dda03d) SHA1(ffe16cfc7dea6cb4cad6765b855a0039a4a7e120) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46_16bit.ic12",     0x00, 0x80, CRC(58f512ff) SHA1(67ffb7e2d817087d8158ee53974e46ec85a3e1ed) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "tch__4.tms27c040.ic33",   0x000000, 0x80000, CRC(5038f52a) SHA1(22ed0e2c8a99056e73cff912731626689996a276) )
	ROM_LOAD( "tch__5.tms27c040.ic34",   0x080000, 0x80000, CRC(06e7094f) SHA1(e41b893ef91d541d2623d76ce6c69ecf4218c16d) )
	ROM_LOAD( "tch__6.tms27c040.ic35",   0x100000, 0x80000, CRC(ea010563) SHA1(5e474db372550e9d33f933ab00881a9b29a712d1) )
	ROM_LOAD( "tch__7.tms27c040.ic36",   0x180000, 0x80000, CRC(b6a86860) SHA1(73ab43830d5e62154bc8953615cdb397c7a742aa) )

	ROM_REGION( 0x080000, "oki", 0 )    /* OKIM6295 samples */
	ROM_LOAD( "tch__3.tms27c040.ic13",        0x00000, 0x80000, CRC(51272b0b) SHA1(ba94385183a9d74bb1d5159d2908492bf500f31e) )
ROM_END

ROM_START( kickgoala ) /* PRO-3/B pcb */
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "tch__2.mc27c2001.ic6",  0x000000, 0x40000, CRC(3ce2743a) SHA1(7998c476c8e630487213dd23ef4fec94a95497ca) )
	ROM_LOAD16_BYTE( "tch__1.am27c020.ic5",   0x000001, 0x40000, CRC(d7d7f83c) SHA1(4ee66a379a0c7ecb15ee4923ac98ba28bfb1e4bd) )

	ROM_REGION( 0x2000, "audiocpu", 0 ) /* sound */
	ROM_LOAD( "pic16c57",     0x0000, 0x1007, CRC(65dda03d) SHA1(ffe16cfc7dea6cb4cad6765b855a0039a4a7e120) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46_16bit.ic12",     0x00, 0x80, CRC(58f512ff) SHA1(67ffb7e2d817087d8158ee53974e46ec85a3e1ed) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "tch__4.tms27c040.ic33",   0x000000, 0x80000, CRC(5038f52a) SHA1(22ed0e2c8a99056e73cff912731626689996a276) )
	ROM_LOAD( "tch__5.tms27c040.ic34",   0x080000, 0x80000, CRC(06e7094f) SHA1(e41b893ef91d541d2623d76ce6c69ecf4218c16d) )
	ROM_LOAD( "tch__6.tms27c040.ic35",   0x100000, 0x80000, CRC(ea010563) SHA1(5e474db372550e9d33f933ab00881a9b29a712d1) )
	ROM_LOAD( "tch__7.tms27c040.ic36",   0x180000, 0x80000, CRC(b6a86860) SHA1(73ab43830d5e62154bc8953615cdb397c7a742aa) )

	ROM_REGION( 0x080000, "oki", 0 )    /* OKIM6295 samples */
	ROM_LOAD( "tch__3.tms27c040.ic13",        0x00000, 0x80000, CRC(51272b0b) SHA1(ba94385183a9d74bb1d5159d2908492bf500f31e) )
ROM_END


ROM_START( actionhw ) /* PRO-3/B pcb */
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "2.ic6",  0x000000, 0x80000, CRC(2b71d58c) SHA1(3e58531fa56d41a3c7944e3beab4850907564a89) )
	ROM_LOAD16_BYTE( "1.ic5",  0x000001, 0x80000, CRC(136b9711) SHA1(553f9fdd99bb9ce2e1492d0755633075e59ba587) )

	ROM_REGION( 0x1000, "audiocpu", 0 ) /* sound? (missing) */
	/* Remove the m_audiocpu->set_disable(); flag in MACHINE_DRIVER when the rom is dumped */
	ROM_LOAD( "pic16c57",     0x0000, 0x0800, NO_DUMP )

	ROM_REGION( 0x400000, "gfx1", 0 )
	ROM_LOAD( "4.ic29",  0x000000, 0x80000, CRC(df076744) SHA1(4b2c8e21a201e1491e4ba3cda8d71b51e0943431) )
	ROM_LOAD( "5.ic33",  0x080000, 0x80000, CRC(8551fdd4) SHA1(f29bdfb75af7607534de171d7b3927419c00377c) )
	ROM_LOAD( "6.ic30",  0x100000, 0x80000, CRC(5cb005a5) SHA1(d3a5ab8f9a520bfaa53fdf6145142ccba416fbb8) )
	ROM_LOAD( "7.ic34",  0x180000, 0x80000, CRC(c2f7d284) SHA1(b3c3d6aa932c813affd667344ea5ddefa55f219b) )
	ROM_LOAD( "8.ic31",  0x200000, 0x80000, CRC(50dffa47) SHA1(33da3b2cabb7b0e480158d343e876563bd0f0930) )
	ROM_LOAD( "9.ic35",  0x280000, 0x80000, CRC(c1ea0370) SHA1(c836611e478d2bf9ae2a5d7e7665982c2b731189) )
	ROM_LOAD( "10.ic32", 0x300000, 0x80000, CRC(5ee5db3e) SHA1(c79f84548ce5311acac478c5180330bf56485863) )
	ROM_LOAD( "11.ic36", 0x380000, 0x80000, CRC(8d376b1e) SHA1(37f16b3237d9813a8d153ab5640252e7643f3b99) )

	ROM_REGION( 0x80000, "oki", 0 )    /* OKIM6295 samples */
	ROM_LOAD( "3.ic13",      0x00000, 0x80000, CRC(b8f6705d) SHA1(55116e14aba6dac7334e26f704b3e6b0b9f856c2) )
ROM_END

/* GAME drivers **************************************************************/

void kickgoal_state::init_kickgoal()
{
#if 0 /* we should find a real fix instead  */
	u16 *rom = (u16 *)memregion("maincpu")->base();

	/* fix "bug" that prevents game from writing to EEPROM */
	rom[0x12b0/2] = 0x0001;
#endif
}

void kickgoal_state::init_actionhw()
{
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x800004, 0x800005, write16s_delegate(*this, FUNC(kickgoal_state::actionhw_snd_w)));
}

} // anonymous namespace

GAME( 1995, kickgoal,  0,        kickgoal, kickgoal, kickgoal_state, init_kickgoal, ROT0, "TCH / Proyesel", "Kick Goal (set 1)",        MACHINE_SUPPORTS_SAVE )
GAME( 1995, kickgoala, kickgoal, kickgoal, kickgoal, kickgoal_state, init_kickgoal, ROT0, "TCH / Proyesel", "Kick Goal (set 2)",        MACHINE_SUPPORTS_SAVE )

GAME( 1995, actionhw,  0,        actionhw, kickgoal, kickgoal_state, init_actionhw, ROT0, "TCH / Proyesel", "Action Hollywood",         MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
