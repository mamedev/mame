// license:BSD-3-Clause
// copyright-holders: Luca Elia, David Haywood

/***************************************************************************

                          -= ESD 16 Bit Games =-

                    driver by   Luca Elia (l.elia@tin.it)
                    additions by David Haywood

The hardware is basically a copy of Tumble Pop bootleg hardware but with
higher bitplane graphics for the backgrounds

Main  CPU   :   M68000
Video Chips :   2 x ACTEL A40MX04 (84 Pin Square Socketed) or
                ESD CRTC99 (QFP240) & ACTEL A40MX04

Sound CPU   :   Z80
Sound Chips :   M6295 (AD-65)  +  YM3812 (U6612)  +  YM3014 (U6614)

---------------------------------------------------------------------------
Year + Game            PCB             Notes
---------------------------------------------------------------------------
98  Multi Champ        ESD 11-09-98   (also a year 1999 revision)
99  Multi Champ Deluxe ESD 08-26-1999 (also a year 2000 revision)
99  Head Panic         ESD 05-28-99   (All English version, copyright 1999)
99  Head Panic         ESD 06-10-1999 (All English version, copyright 1999)
00  Head Panic         ESD 08-26-1999 (All English version, copyright 2000)
00  Head Panic         ESD 08-26-1999 (with Fuuki, Story in Japanese)
00  Deluxe 5           ESD            (no date is marked on PCB)
00  Tang Tang          ESD            (no date is marked on PCB)
01  SWAT Police        ESD            (no date is marked on PCB)
01  Jumping Pop        ESD 11-09-98   (version by Emag Soft)
01  Jumping Pop        ESD 12-04-00   (ESD branded version)
---------------------------------------------------------------------------

Other ESD games:

3 Cushion Billiards (c) 2000 - Undumped
Tang Tang           (c) 2000 - Undumped ESD 05-28-99 version
Fire Hawk           (c) 2001 - see nmk16.cpp driver

---------------------------------------------------------------------------

 Jumping Pop
 -----------

 Jumping Pop is a complete rip-off of Tumble Pop, not even the levels have
 been changed, it simply has different hardware and new 8bpp backgrounds!
 Looks like "Emag" might have been the original programmers (bootleggers)
 of Jumping Pop and ESD picked it up later. Check out the names on the high
 score table, they spell out emag soft. hhmmmm... Also it doesn't look like
 emag "cleaned" the tiles for the title screen, but started clean and ESD
 added their text into the tiles later.

ToDo:
 Verify if Multi Champ, on ESD 11-09-98, uses the same timing / clocks as
 Jumping Pop also on ESD 11-09-98 or the later games:

    Jumping Pop                   Later Games
 -------------------------------------------------------
  68000 = 16MHz                  68000 = 16MHz
    Z80 = 3.5MHz (14MHz/4)         Z80 = 4MHz (16MHz/4)
 YM3812 = 3.5MHz (14MHz/4)      YM3812 = 4MHz (16MHz/4)
  M6295 = 875KHz (14MHz/16)      M6295 = 1Mhz (16MHz/16)

***************************************************************************/

#include "emu.h"

#include "decospr.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/eepromser.h"
#include "machine/gen_latch.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class esd16_state : public driver_device
{
public:
	esd16_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_vram(*this, "vram_%u", 0U),
		m_scroll(*this, "scroll_%u", 0U),
		m_spriteram(*this, "spriteram"),
		m_layersize(*this, "head_layersize"),
		m_platform_x(*this, "platform_x"),
		m_platform_y(*this, "platform_y"),
		m_audiobank(*this, "audiobank"),
		m_io_eepromout(*this, "EEPROMOUT"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_sprgen(*this, "spritegen"),
		m_eeprom(*this, "eeprom"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void esd16_nosound(machine_config &config) ATTR_COLD;
	void esd16(machine_config &config) ATTR_COLD;
	void jumppop(machine_config &config) ATTR_COLD;
	void tangtang(machine_config &config) ATTR_COLD;
	void mchampdx(machine_config &config) ATTR_COLD;
	void hedpanio(machine_config &config) ATTR_COLD;
	void hedpanic(machine_config &config) ATTR_COLD;
	void fantstry(machine_config &config) ATTR_COLD;
	void fantstrya(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_shared_ptr_array<u16, 2> m_vram;
	required_shared_ptr_array<u16, 2> m_scroll;
	required_shared_ptr<u16> m_spriteram;
	required_shared_ptr<u16> m_layersize;
	required_shared_ptr<u16> m_platform_x;
	required_shared_ptr<u16> m_platform_y;

	optional_memory_bank m_audiobank;
	optional_ioport m_io_eepromout;

	tilemap_t *m_tilemap_16x16[2]{};
	tilemap_t *m_tilemap[2]{};
	u32 m_tilemap_color[2]{};

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	optional_device<decospr_device> m_sprgen;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;
	optional_device<generic_latch_8_device> m_soundlatch;

	void sound_command_w(u8 data);
	void platform_w(u16 data);
	u8 eeprom_r();
	void eeprom_w(u8 data);
	void sound_rombank_w(u8 data);
	template<unsigned Layer> void vram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void tilemap0_color_w(u16 data);
	void tilemap0_color_jumppop_w(u16 data);
	template<unsigned Layer> TILE_GET_INFO_MEMBER(get_tile_info);
	template<unsigned Layer> TILE_GET_INFO_MEMBER(get_tile_info_16x16);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECOSPR_PRIORITY_CB_MEMBER(pri_callback);
	void fantstrya_map(address_map &map) ATTR_COLD;
	void hedpanic_map(address_map &map) ATTR_COLD;
	void jumppop_map(address_map &map) ATTR_COLD;
	void mchampdx_map(address_map &map) ATTR_COLD;
	void multchmp_map(address_map &map) ATTR_COLD;
	void sound_io_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void tangtang_map(address_map &map) ATTR_COLD;

	void io_area_dsw(address_map &map, u32 base) ATTR_COLD;
	void io_area_eeprom(address_map &map, u32 base) ATTR_COLD;
	void palette_area(address_map &map, u32 base) ATTR_COLD;
	void sprite_area(address_map &map, u32 base) ATTR_COLD;
	void vid_attr_area(address_map &map, u32 base) ATTR_COLD;
	void vram_area(address_map &map, u32 base) ATTR_COLD;
};


/***************************************************************************

Note:   if MAME_DEBUG is defined, pressing Z with:

        Q / W           Shows Layer 0 / 1
        A               Shows Sprites

        Keys can be used together!


    [ 2 Scrolling Layers ]

        Tile Size:              8 x 8 x 8
        Color Codes:            1 per Layer (banked for Layer 0)
        Layer Size (tiles) :    128 x 64
        Layer Size (pixels):    1024 x 512

    [ 256 Sprites ]

        Sprites are made of 16 x 16 x 5 tiles. Size can vary from 1 to
        8 tiles vertically, while their width is always 1 tile.

    [ Priorities ]

        The game only uses this scheme:

        Back -> Front:  Layer 0, Layer 1, Sprites

***************************************************************************/


/***************************************************************************

                                    Tilemaps

    Offset:     Bits:                   Value:

        0.w                             Code

    Color code:  layer 0 (backmost) can bank at every 256 colors,
                 layer 1 uses the first 256.

***************************************************************************/

template<unsigned Layer>
TILE_GET_INFO_MEMBER(esd16_state::get_tile_info)
{
	const u16 code = m_vram[Layer][tile_index];
	tileinfo.set(0,
			code,
			m_tilemap_color[Layer],
			0);
}

template<unsigned Layer>
TILE_GET_INFO_MEMBER(esd16_state::get_tile_info_16x16)
{
	const u16 code = m_vram[Layer][tile_index];
	tileinfo.set(1,
			code,
			m_tilemap_color[Layer],
			0);
}


void esd16_state::tilemap0_color_w(u16 data)
{
	m_tilemap_color[0] = data & 0x03;
	m_tilemap[0]->mark_all_dirty();
	m_tilemap_16x16[0]->mark_all_dirty();

	bool const flip = BIT(data, 7);
	flip_screen_set(flip);
	m_sprgen->set_flip_screen(flip);
}

void esd16_state::tilemap0_color_jumppop_w(u16 data)
{
	// TODO
	m_tilemap_color[0] = 2;
	m_tilemap_color[1] = 1;

	bool const flip = BIT(data, 7);
	flip_screen_set(flip);
	m_sprgen->set_flip_screen(flip);
}


/***************************************************************************


                            Video Hardware Init


***************************************************************************/


void esd16_state::video_start()
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(esd16_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 8, 8, 0x80, 0x40);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(esd16_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 8, 8, 0x80, 0x40);

	// swatpolc changes tilemap 0 to 16x16 at various times
	m_tilemap_16x16[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(esd16_state::get_tile_info_16x16<0>)), TILEMAP_SCAN_ROWS, 16,16, 0x40, 0x40);

	// hedpanic changes tilemap 1 to 16x16 at various times
	m_tilemap_16x16[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(esd16_state::get_tile_info_16x16<1>)), TILEMAP_SCAN_ROWS, 16,16, 0x40, 0x40);

	m_tilemap[0]->set_scrolldx(-0x60 + 2, -0x60);
	m_tilemap[1]->set_scrolldx(-0x60, -0x60 + 2);
	m_tilemap_16x16[0]->set_scrolldx(-0x60 + 2, -0x60);
	m_tilemap_16x16[1]->set_scrolldx(-0x60, -0x60 + 2);

	m_tilemap[1]->set_transparent_pen(0x00);
	m_tilemap_16x16[1]->set_transparent_pen(0x00);
}


/***************************************************************************


                                Screen Drawing


***************************************************************************/

u32 esd16_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int layers_ctrl = -1;

	screen.priority().fill(0, cliprect);

#ifdef MAME_DEBUG
if (machine().input().code_pressed(KEYCODE_Z))
{
	int msk = 0;
	if (machine().input().code_pressed(KEYCODE_Q))  msk |= 1;
	if (machine().input().code_pressed(KEYCODE_W))  msk |= 2;
	if (machine().input().code_pressed(KEYCODE_A))  msk |= 4;
	if (msk != 0) layers_ctrl &= msk;
}
#endif

	if (layers_ctrl & 1)
	{
		if (m_layersize[0] & 0x0001)
		{
			m_tilemap_16x16[0]->set_scrollx(0, m_scroll[0][0]);
			m_tilemap_16x16[0]->set_scrolly(0, m_scroll[0][1]);
			m_tilemap_16x16[0]->draw(screen, bitmap, cliprect, 0, 0);
		}
		else
		{
			m_tilemap[0]->set_scrollx(0, m_scroll[0][0]);
			m_tilemap[0]->set_scrolly(0, m_scroll[0][1]);
			m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
		}
	}
	else
	{
		bitmap.fill(0, cliprect);
	}


	if (layers_ctrl & 2)
	{
		if (m_layersize[0] & 0x0002)
		{
			m_tilemap_16x16[1]->set_scrollx(0, m_scroll[1][0]);
			m_tilemap_16x16[1]->set_scrolly(0, m_scroll[1][1]);
			m_tilemap_16x16[1]->draw(screen, bitmap, cliprect, 0, 1);
		}
		else
		{
			m_tilemap[1]->set_scrollx(0, m_scroll[1][0]);
			m_tilemap[1]->set_scrolly(0, m_scroll[1][1]);
			m_tilemap[1]->draw(screen, bitmap, cliprect, 0, 1);
		}

	}

	if (layers_ctrl & 4) m_sprgen->draw_sprites(bitmap, cliprect, m_spriteram, 0x400);

//  popmessage("%04x %04x %04x %04x %04x",head_unknown1[0],head_layersize[0],head_unknown3[0],head_unknown4[0],head_unknown5[0]);
	return 0;
}


/***************************************************************************


                            Memory Maps - Main CPU


***************************************************************************/

void esd16_state::sound_command_w(u8 data)
{
	if (m_soundlatch)
	{
		m_soundlatch->write(data & 0xff);
		m_maincpu->spin_until_time(attotime::from_usec(50));  // Allow the other CPU to reply
	}
	else
	{
		logerror("%s: sound_command_w %02x without soundlatch\n", machine().describe_context(), data);
	}
}

template<unsigned Layer>
void esd16_state::vram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_vram[Layer][offset]);
	m_tilemap[Layer]->mark_tile_dirty(offset);
	m_tilemap_16x16[Layer]->mark_tile_dirty(offset);
}

void esd16_state::platform_w(u16 data)
{
	int const offsets = m_platform_x[0] + 0x40 * m_platform_y[0];

	m_vram[1][offsets] = data;
	m_tilemap[1]->mark_tile_dirty(offsets);
	m_tilemap_16x16[1]->mark_tile_dirty(offsets);
}


u8 esd16_state::eeprom_r()
{
	return ((m_eeprom->do_read() & 0x01) << 7);
}

void esd16_state::eeprom_w(u8 data)
{
	m_io_eepromout->write(data, 0xff);
}


void esd16_state::io_area_dsw(address_map &map, u32 base)
{
	map(base + 0x0, base + 0x1).nopw(); // IRQ Ack
	map(base + 0x2, base + 0x3).portr("P1_P2");
	map(base + 0x4, base + 0x5).portr("SYSTEM");
	map(base + 0x6, base + 0x7).portr("DSW");
	map(base + 0x8, base + 0x9).w(FUNC(esd16_state::tilemap0_color_w));
	map(base + 0xa, base + 0xb).nopw(); // Unknown
	map(base + 0xd, base + 0xd).w(FUNC(esd16_state::sound_command_w));
	map(base + 0xe, base + 0xf).nopw(); // n/c
}

void esd16_state::io_area_eeprom(address_map &map, u32 base)
{
	map(base + 0x0, base + 0x1).nopw(); // IRQ Ack
	map(base + 0x2, base + 0x3).portr("P1_P2");
	map(base + 0x4, base + 0x5).portr("SYSTEM");
	map(base + 0x6, base + 0x6).r(FUNC(esd16_state::eeprom_r));
	map(base + 0x8, base + 0x9).w(FUNC(esd16_state::tilemap0_color_w));
	map(base + 0xa, base + 0xb).nopw(); // Unknown
	map(base + 0xd, base + 0xd).w(FUNC(esd16_state::sound_command_w));
	map(base + 0xe, base + 0xf).w(FUNC(esd16_state::eeprom_w));
}

void esd16_state::vid_attr_area(address_map &map, u32 base)
{
	map(base + 0x0, base + 0x3).writeonly().share(m_scroll[0]);
	map(base + 0x4, base + 0x7).writeonly().share(m_scroll[1]);
	map(base + 0x8, base + 0x9).writeonly().share(m_platform_x);
	map(base + 0xa, base + 0xb).writeonly().share(m_platform_y);
	map(base + 0xc, base + 0xd).nopw();
	map(base + 0xe, base + 0xf).writeonly().share(m_layersize);
}

void esd16_state::palette_area(address_map &map, u32 base)
{
	map(base + 0x000, base + 0xfff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
}

void esd16_state::sprite_area(address_map &map, u32 base)
{
	map(base + 0x000, base + 0x7ff).writeonly().share(m_spriteram).mirror(0x000800);
}

void esd16_state::vram_area(address_map &map, u32 base)
{
	map(base + 0x00000, base + 0x03fff).w(FUNC(esd16_state::vram_w<0>)).share(m_vram[0]).mirror(0x4000);
	map(base + 0x20000, base + 0x23fff).w(FUNC(esd16_state::vram_w<1>)).share(m_vram[1]).mirror(0x4000);
}

/*** Memory Maps ***/

void esd16_state::multchmp_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x10ffff).ram();

	palette_area(map, 0x200000);
	sprite_area(map, 0x300000);
	vram_area(map, 0x400000);
	vid_attr_area(map, 0x500000);
	io_area_dsw(map, 0x600000);

	map(0x700008, 0x70000b).nopr(); // unused protection?
}

void esd16_state::fantstrya_map(address_map &map)
{
	multchmp_map(map);

	map(0x100000, 0x10ffff).unmaprw();
	map(0x900000, 0x90ffff).ram();
}

void esd16_state::jumppop_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x120000, 0x123fff).ram();
	map(0x1a0000, 0x1a7fff).ram();

	palette_area(map, 0x140000);
	sprite_area(map, 0x160000);
	io_area_dsw(map, 0x180000);
	vram_area(map, 0x300000);
	vid_attr_area(map, 0x380000);

	map(0x180008, 0x180009).w(FUNC(esd16_state::tilemap0_color_jumppop_w)); // todo
}

void esd16_state::hedpanic_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x10ffff).ram();

	palette_area(map, 0x800000);
	sprite_area(map, 0x900000);
	vram_area(map, 0xa00000);
	vid_attr_area(map, 0xb00000);
	io_area_eeprom(map, 0xc00000);

	map(0xd00008, 0xd00009).w(FUNC(esd16_state::platform_w)); // protection
}

// Multi Champ Deluxe, like Head Panic but different addresses

void esd16_state::mchampdx_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x200000, 0x20ffff).ram();

	vram_area(map, 0x300000);
	palette_area(map, 0x400000);
	io_area_eeprom(map, 0x500000);
	sprite_area(map, 0x600000);
	vid_attr_area(map, 0x700000);

	map(0xd00008, 0xd00009).w(FUNC(esd16_state::platform_w));                      // not used in mchampdx?
}

// Tang Tang & Deluxe 5 - like the others but again with different addresses

void esd16_state::tangtang_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x700000, 0x70ffff).ram();

	palette_area(map, 0x100000);
	sprite_area(map, 0x200000);
	vram_area(map, 0x300000);
	vid_attr_area(map, 0x400000);
	io_area_eeprom(map, 0x500000);
	map(0x600008, 0x600009).w(FUNC(esd16_state::platform_w));
}


/***************************************************************************


                            Memory Maps - Sound CPU


***************************************************************************/

void esd16_state::sound_rombank_w(u8 data)
{
	m_audiobank->set_entry(data & 0xf);
}

void esd16_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();                         // ROM
	map(0x8000, 0xbfff).bankr(m_audiobank);            // Banked ROM
	map(0xf800, 0xffff).ram();                         // RAM
}

void esd16_state::sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).w("ymsnd", FUNC(ym3812_device::write));          // YM3812
	map(0x02, 0x02).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));   // M6295
	map(0x03, 0x03).r(m_soundlatch, FUNC(generic_latch_8_device::read));             // From Main CPU
	map(0x04, 0x04).nopw();                        // ? $00, $30
	map(0x05, 0x05).w(FUNC(esd16_state::sound_rombank_w));                // ROM Bank
	map(0x06, 0x06).noprw();                         // ? At the start / ? 1 (End of NMI routine)
}


/***************************************************************************


                                Input Ports


***************************************************************************/


static INPUT_PORTS_START( jumppop )
	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_SERVICE_DIPLOC( 0x0001, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x001c, 0x001c, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:3,4,5")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x001c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0014, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x00e0, 0x00e0, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:6,7,8")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x00e0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x00a0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0400, 0x0400, "Picture Viewer" )        PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, "BG Modesty" )               PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, "More" )
	PORT_DIPSETTING(      0x0000, "Less" )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(      0x8000, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0xc000, "3" )
	PORT_DIPSETTING(      0x4000, "4" )
INPUT_PORTS_END

static INPUT_PORTS_START( multchmp )
	PORT_START("P1_P2")
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_BIT(  0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT(  0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT(  0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT(  0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT(  0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT(  0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT(  0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // Resets the test mode
	PORT_BIT(  0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_COIN1   )
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_COIN2   )
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_START1  )
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_START2  )
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_SERVICE_DIPLOC(  0x0001, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPNAME( 0x0002, 0x0002, "Coinage Type" )          PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0002, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )    PORT_CONDITION("DSW", 0x0002, EQUALS, 0x0000)
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )    PORT_CONDITION("DSW", 0x0002, EQUALS, 0x0000)
	PORT_DIPSETTING(      0x0020, DEF_STR( 2C_1C ) )    PORT_CONDITION("DSW", 0x0002, EQUALS, 0x0000)
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )    PORT_CONDITION("DSW", 0x0002, EQUALS, 0x0002)
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )    PORT_CONDITION("DSW", 0x0002, EQUALS, 0x0002)
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_2C ) )    PORT_CONDITION("DSW", 0x0002, EQUALS, 0x0002)
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x0040, DEF_STR( 2C_1C ) )    PORT_CONDITION("DSW", 0x0002, EQUALS, 0x0002)
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )    PORT_CONDITION("DSW", 0x0002, EQUALS, 0x0002)
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_2C ) )    PORT_CONDITION("DSW", 0x0002, EQUALS, 0x0002)
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_3C ) )    PORT_CONDITION("DSW", 0x0002, EQUALS, 0x0000)
	PORT_DIPSETTING(      0x0040, DEF_STR( 1C_4C ) )    PORT_CONDITION("DSW", 0x0002, EQUALS, 0x0000)
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )    PORT_CONDITION("DSW", 0x0002, EQUALS, 0x0000)

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0c00, "3" )
	PORT_DIPSETTING(      0x0800, "4" )
	PORT_DIPSETTING(      0x0400, "5" )
	PORT_DIPNAME( 0x1000, 0x1000, "Selectable Games" )      PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x1000, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Free_Play ) )        PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x4000, 0x4000, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x8000, "SW2:8" )
INPUT_PORTS_END


static INPUT_PORTS_START( fantstry )
	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x000e, 0x000e, "Game Level" )
	PORT_DIPSETTING(      0x000c, "1" )
	PORT_DIPSETTING(      0x000a, "2" )
	PORT_DIPSETTING(      0x000e, "3" )
	PORT_DIPSETTING(      0x0008, "4" )
	PORT_DIPSETTING(      0x0006, "5" )
	PORT_DIPSETTING(      0x0004, "6" )
	PORT_DIPSETTING(      0x0002, "7" )
	PORT_DIPSETTING(      0x0000, "8" )
	PORT_DIPNAME( 0x0010, 0x0000, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )

	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Language ) )
	PORT_DIPSETTING(      0x0100, "Korean" )
	PORT_DIPSETTING(      0x0000, "English" )
	PORT_DIPNAME( 0x1e00, 0x1e00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 4C_2C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_4C ) )
	PORT_DIPSETTING(      0x1a00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x1200, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0a00, DEF_STR( 3C_3C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x1c00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x1400, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(      0x1e00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x1600, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 2C_4C ) )
	PORT_DIPSETTING(      0x0e00, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_4C ) )

	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_SERVICE( 0x8000, IP_ACTIVE_LOW )
INPUT_PORTS_END


static INPUT_PORTS_START( hedpanic )
	PORT_START("P1_P2")
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_BIT(  0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT(  0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT(  0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT(  0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT(  0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT(  0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT(  0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_COIN1   )
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_COIN2   )
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_START1  )
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_START2  )
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_SERVICE1  )
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x0040, IP_ACTIVE_LOW)
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::cs_write))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::clk_write))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::di_write))
INPUT_PORTS_END


static INPUT_PORTS_START( swatpolc )
	PORT_START("P1_P2")
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_BIT(  0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT(  0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT(  0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT(  0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT(  0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT(  0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT(  0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT(  0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_COIN1   )
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_COIN2   )
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_START1  )
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_START2  )
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_SERVICE1  )
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x0040, IP_ACTIVE_LOW)
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::cs_write))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::clk_write))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::di_write))
INPUT_PORTS_END

/***************************************************************************


                            Graphics Layouts


***************************************************************************/



static const gfx_layout jumppop_sprite_16x16x4 =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2)+0, 8, 0 },
	{ STEP8(8*2*16,1), STEP8(0,1) },
	{ STEP16(0,8*2) },
	64*8
};

static const gfx_layout hedpanic_sprite_16x16x5 =
{
	16,16,
	RGN_FRAC(1,3),
	5,
	{ RGN_FRAC(2,3), RGN_FRAC(0,3), RGN_FRAC(0,3)+8, RGN_FRAC(1,3),RGN_FRAC(1,3)+8 },
	{ STEP8(8*2*16,1), STEP8(0,1) },
	{ STEP16(0,8*2) },
	16*32,
};

static const gfx_layout hedpanic_layout_16x16x8 =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ STEP8(0,1) },
	{ STEP8(0,8), STEP8(8*8*8,8) },
	{ STEP8(0,8*8), STEP8(8*8*8*2,8*8) },
	256*8,
};


static GFXDECODE_START( gfx_esd16 )
	GFXDECODE_ENTRY( "bgs", 0, gfx_8x8x8_raw,           0x000, 2 )      // [1] Layers
	GFXDECODE_ENTRY( "bgs", 0, hedpanic_layout_16x16x8, 0x000, 2 )      // [1] Layers
GFXDECODE_END

static GFXDECODE_START( gfx_esd16_spr )
	GFXDECODE_ENTRY( "spr", 0, hedpanic_sprite_16x16x5, 0x200, 8 )      // [0] Sprites
GFXDECODE_END

static GFXDECODE_START( gfx_jumppop )
	GFXDECODE_ENTRY( "bgs", 0, gfx_8x8x8_raw,           0x000, 4 )      // Characters 8x8
	GFXDECODE_ENTRY( "bgs", 0, hedpanic_layout_16x16x8, 0x000, 4 )      // Tiles 16x16
GFXDECODE_END

static GFXDECODE_START( gfx_jumppop_spr )
	GFXDECODE_ENTRY( "spr", 0, jumppop_sprite_16x16x4,  0x000, 0x40 )   // Sprites 16x16 - has 4bpp sprites, unlike the others
GFXDECODE_END


/***************************************************************************


                                Machine Drivers


***************************************************************************/

void esd16_state::machine_start()
{
	if (m_audiobank)
	{
		u8 *const audiorom = memregion("audiocpu")->base();
		m_audiobank->configure_entries(0, 16, &audiorom[0x0000], 0x4000);
	}

	save_item(NAME(m_tilemap_color));
}

void esd16_state::machine_reset()
{
	m_tilemap_color[0] = 0;
	m_tilemap_color[1] = 0;
}

DECOSPR_PRIORITY_CB_MEMBER(esd16_state::pri_callback)
{
	if (pri & 0x8000)
		return 0xfffe; // under "tilemap 1"
	else
		return 0; // above everything
}

void esd16_state::esd16_nosound(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, XTAL(16'000'000));  // 16MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &esd16_state::multchmp_map);
	m_maincpu->set_vblank_int("screen", FUNC(esd16_state::irq6_line_hold));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(0x140, 0x100);
	screen.set_visarea(0, 0x140-1, 0+8, 0x100-8-1);
	screen.set_screen_update(FUNC(esd16_state::screen_update));
	screen.set_palette("palette");

	DECO_SPRITE(config, m_sprgen, 0, "palette", gfx_esd16_spr);
	m_sprgen->set_is_bootleg(true);
	m_sprgen->set_pri_callback(FUNC(esd16_state::pri_callback));
	m_sprgen->set_flipallx(1);

	GFXDECODE(config, m_gfxdecode, "palette", gfx_esd16);
	PALETTE(config, "palette").set_format(palette_device::xRGB_555, 0x1000 / 2);
}

void esd16_state::fantstry(machine_config& config)
{
	esd16_nosound(config);

	// PIC16F84A-04/P for sound CPU

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, "okisfx", XTAL(16'000'000) / 8, okim6295_device::PIN7_LOW);
	OKIM6295(config, "okimusic", XTAL(16'000'000) / 8, okim6295_device::PIN7_LOW);
}

void esd16_state::fantstrya(machine_config& config)
{
	fantstry(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &esd16_state::fantstrya_map);

	config.device_remove("okisfx");
}

void esd16_state::esd16(machine_config& config)
{
	esd16_nosound(config);

	Z80(config, m_audiocpu, XTAL(16'000'000) / 4); // 4MHz
	m_audiocpu->set_addrmap(AS_PROGRAM, &esd16_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &esd16_state::sound_io_map);
	m_audiocpu->set_periodic_int(FUNC(esd16_state::nmi_line_pulse), attotime::from_hz(32 * 60));    // IRQ By Main CPU

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch).data_pending_callback().set_inputline(m_audiocpu, 0);

	YM3812(config, "ymsnd", XTAL(16'000'000) / 4).add_route(ALL_OUTPUTS, "mono", 0.30);   // 4MHz

	OKIM6295(config, "oki", XTAL(16'000'000) / 16, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.60); // 1MHz
}

void esd16_state::jumppop(machine_config &config)
{
	esd16(config);

	// basic machine hardware

	m_maincpu->set_addrmap(AS_PROGRAM, &esd16_state::jumppop_map);

	m_audiocpu->set_clock(XTAL(14'000'000) / 4); // 3.5MHz - Verified

	m_sprgen->set_info(gfx_jumppop_spr);
	m_gfxdecode->set_info(gfx_jumppop);

	subdevice<ym3812_device>("ymsnd")->set_clock(XTAL(14'000'000) / 4); // 3.5MHz - Verified

	subdevice<okim6295_device>("oki")->set_clock(XTAL(14'000'000) / 16); // 875kHz - Verified
}

// The ESD 05-28-99 PCB adds an EEPROM

void esd16_state::hedpanio(machine_config &config)
{
	esd16(config);
	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &esd16_state::hedpanic_map);

	EEPROM_93C46_16BIT(config, "eeprom");
}

// The ESD 08-26-1999 PCBs take that further and modify the sprite offsets

void esd16_state::hedpanic(machine_config &config)
{
	hedpanio(config);
	m_sprgen->set_offsets(-0x18, -0x100);
}

// ESD 08-26-1999 PCBs with different memory maps

void esd16_state::mchampdx(machine_config &config)
{
	hedpanic(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &esd16_state::mchampdx_map);
}

void esd16_state::tangtang(machine_config &config)
{
	hedpanic(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &esd16_state::tangtang_map);
}


/***************************************************************************


                                ROMs Loading


***************************************************************************/

/***************************************************************************

                                Multi Champ
PCB Layout
----------

ESD 11-09-98
+-----------------------------------------+
|       YM3812   6116    su10         fu27|
|VOL    YM3014   su06    M6295        fu32|
|           PAL  Z80                  fu26|
|        6116                         fu30|
|J       6116            Actel        fu28|
|A DSWA DSWB    76C256   A40MX04      fu31|
|M              76C256                fu29|
|M    cu03 76C256   6116 Actel    PAL fu33|
|A    cu02 76C256   6116 A40MX04  PAL ju07|
|      68000              6116        ju03|
|       PAL               6116 PAL    ju04|
|       PAL               6116 PAL    ju05|
|16MHz 14MHz              6116 PAL    ju06|
+-----------------------------------------+

(C) ESD 1998, 1999
PCB No. ESD 11-09-98
CPU: MC68HC000FN16 (68000, 68 pin PLCC socketed)
SND: Z80 (Z0840006PSC), YM3812/YM3014 & OKI M6295 (rebadged as U6612/U6614 & AD-65)
OSC: 16.000MHz, 14.000MHz
RAM: 4 x 62256, 9 x 6116
DIPS: 2 x 8 position
Dip info is in Japanese! I will scan and make it available on my site for translation.

Other Chips: 2 x Actel A40MX04-F FPGA (PLCC84)
8 PAL's (not dumped)

ROMS:

MULTCHMP.U02  \   Main Program     MX27C2000
MULTCHMP.U03  /                    MX27C2000
MULTCHMP.U06   -- Sound Program    27C010
MULTCHMP.U10   -- ADPCM Samples    27C010
MULTCHMP.U27 -\                    27C4001
MULTCHMP.U28   \                   27C4001
MULTCHMP.U29    |                  27C4001
MULTCHMP.U30    + Backgrounds      27C4001
MULTCHMP.U31    |                  27C4001
MULTCHMP.U32    |                  27C4001
MULTCHMP.U33   /                   27C4001
MULTCHMP.U34 -/                    27C4001
MULTCHMP.U35 -\                    MX27C2000
MULTCHMP.U36   \                   MX27C2000
MULTCHMP.U37    +- Sprites         MX27C2000
MULTCHMP.U38   /                   MX27C2000
MULTCHMP.U39 -/                    MX27C2000

***************************************************************************/

ROM_START( multchmp25 )
	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "esd2.cu02", 0x000000, 0x040000,  CRC(2d1b098a) SHA1(c2f3991f02c611c258219da2c61cad22c9a21f7d) )
	ROM_LOAD16_BYTE( "esd1.cu03", 0x000001, 0x040000,  CRC(10974063) SHA1(854b38b4d4cb529e9928aae4212c86a220615e04) )

	ROM_REGION( 0x40000, "audiocpu", 0 )        // Z80 Code
	ROM_LOAD( "esd3.su06", 0x00000, 0x20000, CRC(7c178bd7) SHA1(8754d3c70d9b2bf369a5ce0cce4cc0696ed22750) )

	ROM_REGION( 0x180000, "spr", 0 )    // Sprites, 16x16x5
	ROM_LOAD16_BYTE( "esd17.ju06", 0x000000, 0x040000, CRC(a69d4399) SHA1(06ae6c07cc6b7313e2e2aa3b994f7532d6994e1b) )
	ROM_LOAD16_BYTE( "esd16.ju05", 0x000001, 0x040000, CRC(e670a6da) SHA1(47cbe45b6d5d0ca70d0c6787d589dde5d14fdba4) )
	ROM_LOAD16_BYTE( "esd15.ju04", 0x080000, 0x040000, CRC(88b7a97c) SHA1(0a57ec8f6a44c8e3aa3ef35499a415d6a2b7eb16) )
	ROM_LOAD16_BYTE( "esd14.ju03", 0x080001, 0x040000, CRC(a6122225) SHA1(cbcf2b31c4c011daba21f0ae5fd3be63c9a87c00) )
	ROM_LOAD16_BYTE( "esd13.ju07", 0x100000, 0x040000, CRC(22071594) SHA1(c79102b250780d1da8c290d065d61fbbfa193366) )

	ROM_REGION( 0x400000, "bgs", 0 )    // Layers, 16x16x8
	ROM_LOAD32_BYTE( "esd9.fu28",  0x000000, 0x080000, CRC(6652c04a) SHA1(178e1d42847506d869ef79db2f7e10df05e9ef76) )
	ROM_LOAD32_BYTE( "esd7.fu26",  0x000001, 0x080000, CRC(a783a003) SHA1(1ff61a049485c5b599c458a8bf7f48027d14f8e0) )
	ROM_LOAD32_BYTE( "esd11.fu29", 0x000002, 0x080000, CRC(9bafd8ee) SHA1(db18be05431d4b6d4207e19fa4ed8701621aaa19) )
	ROM_LOAD32_BYTE( "esd5.fu27",  0x000003, 0x080000, CRC(299f32c2) SHA1(274752444f6ddba16eeefc02c3e78525c079b3d8) )
	ROM_LOAD32_BYTE( "esd10.fu31", 0x200000, 0x080000, CRC(d815974b) SHA1(3e528a5df79fa7dc0f38b0ee7f2f3a0ebc97a369) )
	ROM_LOAD32_BYTE( "esd8.fu30",  0x200001, 0x080000, CRC(22861af2) SHA1(1e74e85517cb8fd5fb4bda6e9d9d54046e31f653) )
	ROM_LOAD32_BYTE( "esd12.fu33", 0x200002, 0x080000, CRC(c6b86001) SHA1(11a63b56df30ab7b85ce4568d2a24e96a125735a) )
	ROM_LOAD32_BYTE( "esd6.fu32",  0x200003, 0x080000, CRC(e2689bb2) SHA1(1da9b1f7335d5c2d1c2f8353fccf91c0109d2e9d) )

	ROM_REGION( 0x40000, "oki", 0 ) // Samples
	ROM_LOAD( "esd4.su10", 0x00000, 0x20000, CRC(6e741fcd) SHA1(742e0952916c00f67dd9f8d01e721a9a538d2fc4) )
ROM_END

ROM_START( multchmpk )
	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "multchmp.u02", 0x000000, 0x040000, CRC(7da8c0df) SHA1(763a3240554a02d8a9a0b13b6bfcd384825a6c57) )
	ROM_LOAD16_BYTE( "multchmp.u03", 0x000001, 0x040000, CRC(5dc62799) SHA1(ff7882985efc20309c3f901a622f1beffa0c47be) )

	ROM_REGION( 0x40000, "audiocpu", 0 )        // Z80 Code
	ROM_LOAD( "esd3.su06", 0x00000, 0x20000, CRC(7c178bd7) SHA1(8754d3c70d9b2bf369a5ce0cce4cc0696ed22750) )

	ROM_REGION( 0x180000, "spr", 0 )    // Sprites, 16x16x5
	ROM_LOAD16_BYTE( "multchmp.u39", 0x000000, 0x040000, CRC(51f01067) SHA1(d5ebbc7d358b63724d2f24da8b2ce4a202be37a5) )
	ROM_LOAD16_BYTE( "multchmp.u38", 0x000001, 0x040000, CRC(88e252e8) SHA1(07d898379798c6be42b636762b0af61b9111a480) )
	ROM_LOAD16_BYTE( "multchmp.u37", 0x080000, 0x040000, CRC(b1ae7f08) SHA1(37dd9d4cef8b9e1d09d7b46a9794fb2b777c9a01) )
	ROM_LOAD16_BYTE( "multchmp.u36", 0x080001, 0x040000, CRC(d8f06fa8) SHA1(f76912f93f99578529612a7f01d82ac7229a8e41) )
	ROM_LOAD16_BYTE( "multchmp.u35", 0x100000, 0x040000, CRC(9d1590a6) SHA1(35f634dbf0df06ec62359c7bae43c7f5d14b0ab2) )

	ROM_REGION( 0x400000, "bgs", 0 )    // Layers, 16x16x8
	ROM_LOAD32_BYTE( "multchmp.u31", 0x000000, 0x080000, CRC(b1e4e9e3) SHA1(1a7393e9073b028b4170393b3788ad8cb86c0c78) )
	ROM_LOAD32_BYTE( "multchmp.u29", 0x000001, 0x080000, CRC(01bd1399) SHA1(b717ccffe0af92a42a0879736d34d3ad71840233) )
	ROM_LOAD32_BYTE( "multchmp.u33", 0x000002, 0x080000, CRC(e4c0ec96) SHA1(74152108e4d05f4aff9d38919f212fcb8c87cef3) )
	ROM_LOAD32_BYTE( "multchmp.u27", 0x000003, 0x080000, CRC(dc42704e) SHA1(58a04a47ffc6d6ae0e4d49e466b1c58b37ad741a) )
	ROM_LOAD32_BYTE( "multchmp.u32", 0x200000, 0x080000, CRC(f05cb5b4) SHA1(1b33e60942238e39d61ae59e9317b99e83595ab1) )
	ROM_LOAD32_BYTE( "multchmp.u30", 0x200001, 0x080000, CRC(c6b4cc18) SHA1(d9097b85584272cfe4989a40d622ef1feeee6775) )
	ROM_LOAD32_BYTE( "multchmp.u34", 0x200002, 0x080000, CRC(bffaaccc) SHA1(d9ab248e2c7c639666e3717cfc5d8c8468a1bde2) )
	ROM_LOAD32_BYTE( "multchmp.u28", 0x200003, 0x080000, CRC(449991fa) SHA1(fd93e420a04cb8bea5421aa9cbe079bd3e7d4924) )

	ROM_REGION( 0x40000, "oki", 0 ) // Samples
	ROM_LOAD( "esd4.su10", 0x00000, 0x20000, CRC(6e741fcd) SHA1(742e0952916c00f67dd9f8d01e721a9a538d2fc4) )
ROM_END

ROM_START( multchmpa ) // Also found on a ESD 10-10-98 PCB which looks identical to the ESD 11-09-98 PCB
	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "esd2.cu02", 0x000000, 0x040000, CRC(bfd39198) SHA1(11c0cb7a865daa1be9301ddfa5f5d2014e8f9908) )
	ROM_LOAD16_BYTE( "esd1.cu03", 0x000001, 0x040000, CRC(cd769077) SHA1(741cca679393dab031691834874c96fee791241e) )

	ROM_REGION( 0x40000, "audiocpu", 0 )        // Z80 Code
	ROM_LOAD( "esd3.su01", 0x00000, 0x20000, CRC(7c178bd7) SHA1(8754d3c70d9b2bf369a5ce0cce4cc0696ed22750) )

	ROM_REGION( 0x180000, "spr", 0 )    // Sprites, 16x16x5
	ROM_LOAD16_BYTE( "esd17.ju06", 0x000000, 0x040000, CRC(51f01067) SHA1(d5ebbc7d358b63724d2f24da8b2ce4a202be37a5) )
	ROM_LOAD16_BYTE( "esd16.ju05", 0x000001, 0x040000, CRC(88e252e8) SHA1(07d898379798c6be42b636762b0af61b9111a480) )
	ROM_LOAD16_BYTE( "esd15.ju04", 0x080000, 0x040000, CRC(b1ae7f08) SHA1(37dd9d4cef8b9e1d09d7b46a9794fb2b777c9a01) )
	ROM_LOAD16_BYTE( "esd14.ju03", 0x080001, 0x040000, CRC(d8f06fa8) SHA1(f76912f93f99578529612a7f01d82ac7229a8e41) )
	ROM_LOAD16_BYTE( "esd13.ju07", 0x100000, 0x040000, CRC(9d1590a6) SHA1(35f634dbf0df06ec62359c7bae43c7f5d14b0ab2) )

	ROM_REGION( 0x400000, "bgs", 0 )    // Layers, 16x16x8
	ROM_LOAD32_BYTE( "esd9.fu28",  0x000000, 0x080000, CRC(a3cfe895) SHA1(a8dc0d5d9e64d4c5112177b8f20b5bdb86ca73af) )
	ROM_LOAD32_BYTE( "esd7.fu26",  0x000001, 0x080000, CRC(042d59ff) SHA1(8e45a4757e07d8aaf50b151d8849c1a27424e64b) )
	ROM_LOAD32_BYTE( "esd11.fu29", 0x000002, 0x080000, CRC(d3c1855e) SHA1(bb547d4a45a745e9ae4a6727087cdf325105de90) )
	ROM_LOAD32_BYTE( "esd5.fu27",  0x000003, 0x080000, CRC(ed5b4e58) SHA1(82c3ee9e2525c0b370a29d5560c21ec6380d1a43) )
	ROM_LOAD32_BYTE( "esd10.fu31", 0x200000, 0x080000, CRC(396d77b6) SHA1(f22449a7f9f50e172e36db4f399c14e527409884) )
	ROM_LOAD32_BYTE( "esd8.fu30",  0x200001, 0x080000, CRC(fa8cd2d3) SHA1(ddc1b98867e6d2eee458bf35a933e7cdc59f4c7e) )
	ROM_LOAD32_BYTE( "esd12.fu33", 0x200002, 0x080000, CRC(a68848a8) SHA1(915239a961d76af6a1a567eb89b1569f158e714e) )
	ROM_LOAD32_BYTE( "esd6.fu32",  0x200003, 0x080000, CRC(97fde7b1) SHA1(b3610f6fcc1367ff079dc01121c86bc1e1f4c7a2) )

	ROM_REGION( 0x40000, "oki", 0 ) // Samples
	ROM_LOAD( "esd4.su08", 0x00000, 0x20000, CRC(6e741fcd) SHA1(742e0952916c00f67dd9f8d01e721a9a538d2fc4) )
ROM_END

// this runs on a slightly newer PCB: ESD 05-17-99
// there is a string about EEPROM write error but there isn't any on PCB
ROM_START( multchmp )
	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "esd7.cu02", 0x000000, 0x040000, CRC(e25cbbfe) SHA1(fa142489a2171fb94b62d2635a1da5944ddde0cb) )
	ROM_LOAD16_BYTE( "esd6.cu03", 0x000001, 0x040000, CRC(e7f837f9) SHA1(f31bf6c99b4f9be7bf9bb7ba04cf354ff070ad7f) )

	ROM_REGION( 0x40000, "audiocpu", 0 )        // Z80 Code
	ROM_LOAD( "esd3.su06", 0x00000, 0x20000, CRC(7c178bd7) SHA1(8754d3c70d9b2bf369a5ce0cce4cc0696ed22750) )

	ROM_REGION( 0x180000, "spr", 0 )    // Sprites, 16x16x5
	ROM_LOAD16_BYTE( "esd17.ju06", 0x000000, 0x040000, CRC(a69d4399) SHA1(06ae6c07cc6b7313e2e2aa3b994f7532d6994e1b) )
	ROM_LOAD16_BYTE( "esd16.ju05", 0x000001, 0x040000, CRC(e670a6da) SHA1(47cbe45b6d5d0ca70d0c6787d589dde5d14fdba4) )
	ROM_LOAD16_BYTE( "esd15.ju04", 0x080000, 0x040000, CRC(88b7a97c) SHA1(0a57ec8f6a44c8e3aa3ef35499a415d6a2b7eb16) )
	ROM_LOAD16_BYTE( "esd14.ju03", 0x080001, 0x040000, CRC(a6122225) SHA1(cbcf2b31c4c011daba21f0ae5fd3be63c9a87c00) )
	ROM_LOAD16_BYTE( "esd13.ju07", 0x100000, 0x040000, CRC(22071594) SHA1(c79102b250780d1da8c290d065d61fbbfa193366) )

	ROM_REGION( 0x400000, "bgs", 0 )    // Layers, 16x16x8
	ROM_LOAD( "mx29f1610ml.fu27_fu32", 0x000000, 0x200000, NO_DUMP ) // These 2 ROMs are soldered and weren't dumped. Using the ones from the other sets for now.
	ROM_LOAD( "mx29f1610ml.fu28_fu31", 0x000000, 0x200000, NO_DUMP )
	ROM_LOAD32_BYTE( "esd9.fu28",      0x000000, 0x080000, BAD_DUMP CRC(6652c04a) SHA1(178e1d42847506d869ef79db2f7e10df05e9ef76) )
	ROM_LOAD32_BYTE( "esd7.fu26",      0x000001, 0x080000, BAD_DUMP CRC(a783a003) SHA1(1ff61a049485c5b599c458a8bf7f48027d14f8e0) )
	ROM_LOAD32_BYTE( "esd11.fu29",     0x000002, 0x080000, BAD_DUMP CRC(9bafd8ee) SHA1(db18be05431d4b6d4207e19fa4ed8701621aaa19) )
	ROM_LOAD32_BYTE( "esd5.fu27",      0x000003, 0x080000, BAD_DUMP CRC(299f32c2) SHA1(274752444f6ddba16eeefc02c3e78525c079b3d8) )
	ROM_LOAD32_BYTE( "esd10.fu31",     0x200000, 0x080000, BAD_DUMP CRC(d815974b) SHA1(3e528a5df79fa7dc0f38b0ee7f2f3a0ebc97a369) )
	ROM_LOAD32_BYTE( "esd8.fu30",      0x200001, 0x080000, BAD_DUMP CRC(22861af2) SHA1(1e74e85517cb8fd5fb4bda6e9d9d54046e31f653) )
	ROM_LOAD32_BYTE( "esd12.fu33",     0x200002, 0x080000, BAD_DUMP CRC(c6b86001) SHA1(11a63b56df30ab7b85ce4568d2a24e96a125735a) )
	ROM_LOAD32_BYTE( "esd6.fu32",      0x200003, 0x080000, BAD_DUMP CRC(e2689bb2) SHA1(1da9b1f7335d5c2d1c2f8353fccf91c0109d2e9d) )

	ROM_REGION( 0x40000, "oki", 0 ) // Samples
	ROM_LOAD( "esd4.su10", 0x00000, 0x20000, CRC(6e741fcd) SHA1(742e0952916c00f67dd9f8d01e721a9a538d2fc4) )

	ROM_REGION( 0x1800, "plds", ROMREGION_ERASE00 ) // all read-protected
	ROM_LOAD( "palce22v10h-25pc-4.cu04", 0x0000, 0x02dd, NO_DUMP )
	ROM_LOAD( "palce22v10h-25pc-4.cu05", 0x0300, 0x02dd, NO_DUMP )
	ROM_LOAD( "palce22v10h-25pc-4.du05", 0x0600, 0x02dd, NO_DUMP )
	ROM_LOAD( "palce22v10h-25pc-4.fu01", 0x0900, 0x02dd, NO_DUMP )
	ROM_LOAD( "palce16v8h-25.du03",      0x0c00, 0x0117, NO_DUMP )
	ROM_LOAD( "palce16v8h-25.ru27",      0x0e00, 0x0117, NO_DUMP )
	ROM_LOAD( "palce16v8h-25.fu01",      0x1000, 0x0117, NO_DUMP )
	ROM_LOAD( "palce16v8h-25.su07",      0x1200, 0x0117, NO_DUMP )
	ROM_LOAD( "a40mx04-f-pl84.bu01",     0x1400, 0x0200, NO_DUMP ) // size wrong
	ROM_LOAD( "a40mx04-f-pl84.du16",     0x1600, 0x0200, NO_DUMP ) // size wrong
ROM_END

/*

Multi Champ Deluxe
------------------

PCB Layout
----------

ESD 08-26-1999
|-----------------------------------------|
|  3014  3812 6116   6295   ESD4.SU10   * |
|VOL      ESD3.SU06  Z80          ROM.JU01|
|             PAL                       * |
|                            6116         |
|       6116            PAL  6116 ROM.JU02|
|       6116           |-------|        * |
|J                PAL  | ESD   |        * |
|A                PAL  |CRTC99 |ESD5.JU07 |
|M     PAL             |       |        * |
|M     PAL             |-------|          |
|A    68000    ESD1.CU03                  |
|              ESD2.CU02  |-------|       |
|                         |ACTEL  | 6116  |
|   93C46                 |A40MX04| 6116  |
|              MCM6206    |       | 6116  |
|              MCM6206    |-------| 6116  |
|SW1 16MHz PAL MCM6206                    |
|SW2 14MHz PAL MCM6206  ROM.FU35 ROM.FU34 |
|-----------------------------------------|

Notes:
      68000 clock 16.000MHz
        Z80 clock 4.000MHz
      M6295 clock 1.000MHz. Sample rate 1000000/132
     YM3812 clock 4.000MHz
      HSync   - 15.625kHz
      VSync   - 60Hz
      MCM6206 - 32k x8 SRAM (SOJ28)
      6116    - 2k x8 SRAM (SOP28)
      A40MX04 - Actel A40MX04-F FPGA (PLCC84)
      CRTC99  - ESD CRTC99 Graphics Controller (QFP240)

      * : Board has positions for 6x standard 32 pin EPROMs but only position ESD5 is populated
          with an EPROM. In between the unpopulated positions are 2x smt pads. These are populated
          with 2x 16M SOP44 smt Mask ROMs.

Note: Some versions of this PCB used larger EPROMs with the data repeated:
      ESD 3 @ SU06 as a 27C040 with data repeated 2x (CRC32 0x2C0C8813)

*/


ROM_START( mchampdx )
	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "ver0106_esd2.cu02", 0x000000, 0x040000, CRC(ea98b3fd) SHA1(107ee8adea246141fd6fa9209541ce0a7ed1e24c) )
	ROM_LOAD16_BYTE( "ver0106_esd1.cu03", 0x000001, 0x040000, CRC(c6e4546b) SHA1(af9a8edffe94d035f92b36b1cd145c2a5ee66f48) )

	ROM_REGION( 0x40000, "audiocpu", 0 )        // Z80 Code
	ROM_LOAD( "esd3.su06", 0x00000, 0x40000, CRC(1b22568c) SHA1(5458e1a798357a6785f8ea1fe9da37768cd4761d) )

	// this has additional copyright sprites in the flash ROMs for the (c)2000 message..
	ROM_REGION( 0x600000, "spr", 0 )    // Sprites, 16x16x5
	ROM_LOAD( "ver0106_ju02.bin",         0x000000, 0x200000, CRC(b27a4977) SHA1(b7f94bb04d0046538b3938335e6b0cce330ad79c) )
	ROM_LOAD( "ver0106_ju01.bin",         0x200000, 0x200000, CRC(55841d90) SHA1(52ba3ee9393dcddf28e2d20a50151bc739faaaa4) )
	// expand this to take up 0x200000 bytes too so we can decode it
	ROM_LOAD16_BYTE( "ver0106_esd5.ju07", 0x400000, 0x040000, CRC(7a3ac887) SHA1(3c759f9bed396bbaf6bd7298a8bd2bd76df3aa6f) )
	ROM_FILL(                             0x500000, 0x100000, 0x00 )

	ROM_REGION( 0x400000, "bgs", 0 )    // Layers, 16x16x8
	ROM_LOAD32_WORD( "rom.fu35", 0x000000, 0x200000, CRC(ba46f3dc) SHA1(4ac7695bdf4237654481f7f74f8650d70a51e691) )
	ROM_LOAD32_WORD( "rom.fu34", 0x000002, 0x200000, CRC(2895cf09) SHA1(88756fcd589af1986c3881d4080f086afc11b498) )

	ROM_REGION( 0x40000, "oki", 0 ) // Samples
	ROM_LOAD( "esd4.su10", 0x00000, 0x40000, CRC(2fbe94ab) SHA1(1bc4a33ec93a80fb598722d2b50bdf3ccaaa984a) )

	ROM_REGION16_BE( 0x80, "eeprom", ROMREGION_ERASE00 ) // factory default settings because game doesn't init them properly otherwise
	ROM_LOAD16_WORD_SWAP( "eeprom", 0x0000, 0x0080, CRC(646b2f53) SHA1(f6673f68084b63a69c612a03c58f57435d5a9496) )
ROM_END

ROM_START( mchampdxa )
	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "esd2.cu02", 0x000000, 0x040000, CRC(4cca802c) SHA1(5e6e81febbb56b7c4630b530e546e7ab59c6c6c1) )
	ROM_LOAD16_BYTE( "esd1.cu03", 0x000001, 0x040000, CRC(0af1cd0a) SHA1(d2befcb596d83d523317d17b4c1c71f99de0d33e) )

	ROM_REGION( 0x40000, "audiocpu", 0 )        // Z80 Code
	ROM_LOAD( "esd3.su06", 0x00000, 0x40000, CRC(1b22568c) SHA1(5458e1a798357a6785f8ea1fe9da37768cd4761d) )

	ROM_REGION( 0x600000, "spr", 0 )    // Sprites, 16x16x5
	ROM_LOAD( "rom.ju02",         0x000000, 0x200000, CRC(7e87e332) SHA1(f90aa00a64a940846d99053c7aa023e3fd5d070b) )
	ROM_LOAD( "rom.ju01",         0x200000, 0x200000, CRC(1a749fc2) SHA1(feff4b26ee28244b4d092798a176e33e09d5df2c) )
	// expand this to take up 0x200000 bytes too so we can decode it
	ROM_LOAD16_BYTE( "esd5.ju07", 0x400000, 0x080000, CRC(6cc871cc) SHA1(710b9695c864e4234686993b88d24590d60e1cb9) )
	ROM_FILL(                     0x500000, 0x100000, 0x00 )

	ROM_REGION( 0x400000, "bgs", 0 )    // Layers, 16x16x8
	ROM_LOAD32_WORD( "rom.fu35", 0x000000, 0x200000, CRC(ba46f3dc) SHA1(4ac7695bdf4237654481f7f74f8650d70a51e691) )
	ROM_LOAD32_WORD( "rom.fu34", 0x000002, 0x200000, CRC(2895cf09) SHA1(88756fcd589af1986c3881d4080f086afc11b498) )

	ROM_REGION( 0x40000, "oki", 0 ) // Samples
	ROM_LOAD( "esd4.su10", 0x00000, 0x40000, CRC(2fbe94ab) SHA1(1bc4a33ec93a80fb598722d2b50bdf3ccaaa984a) )

	ROM_REGION16_BE( 0x80, "eeprom", ROMREGION_ERASE00 ) // factory default settings because game doesn't init them properly otherwise
	ROM_LOAD16_WORD_SWAP( "eeprom", 0x0000, 0x0080, CRC(646b2f53) SHA1(f6673f68084b63a69c612a03c58f57435d5a9496) )
ROM_END

ROM_START( mchampdxb )
	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "ver1114_esd2.cu02", 0x000000, 0x040000, CRC(d17b2616) SHA1(2c50c2bf928036678b92b8862d191552e46d9faa) )
	ROM_LOAD16_BYTE( "ver1114_esd1.cu03", 0x000001, 0x040000, CRC(11ff2e94) SHA1(30044bedfff514ae0a855cffa756e5c315fe2124) )

	ROM_REGION( 0x40000, "audiocpu", 0 )        // Z80 Code
	ROM_LOAD( "ver1114_esd3.su06", 0x00000, 0x40000, CRC(b87a1e85) SHA1(2fcdd7e8b301e3d20e6500a03dc293403b23b471) )

	ROM_REGION( 0x600000, "spr", 0 )    // Sprites, 16x16x5
	ROM_LOAD( "ver1114_ju02",             0x000000, 0x200000, CRC(2f9ccff8) SHA1(176240cd247cc5d3efd58fe0630726a8633be2a4) )
	ROM_LOAD( "ver1114_ju01",             0x200000, 0x200000, CRC(0048e687) SHA1(5cc0a35b5f5f8d69b2dc3728ad6d0d505d9e16c5) )  // SMT Flash MX chips
	// expand this to take up 0x200000 bytes too so we can decode it
	ROM_LOAD16_BYTE( "ver1114_esd5.ju07", 0x400000, 0x040000,  CRC(8175939f) SHA1(cd0132ae0d2e35dc656434989b1f0f255ad562ab) )
	ROM_FILL(                             0x500000, 0x100000, 0x00 )

	ROM_REGION( 0x400000, "bgs", 0 )    // Layers, 16x16x8
	ROM_LOAD32_WORD( "ver1114_fu35", 0x000000, 0x200000, CRC(c515c704) SHA1(c1657534314e66a25c38f70a12f14d2225ab89cc) ) // SMT Flash MX chips
	ROM_LOAD32_WORD( "ver1114_fu34", 0x000002, 0x200000, CRC(39d448bb) SHA1(07cd6e30a25d1c0caeef0f95f23df0ca6a2c7a26) )

	ROM_REGION( 0x40000, "oki", 0 ) // Samples
	ROM_LOAD( "esd4.su10", 0x00000, 0x40000, CRC(2fbe94ab) SHA1(1bc4a33ec93a80fb598722d2b50bdf3ccaaa984a) )

	ROM_REGION16_BE( 0x80, "eeprom", ROMREGION_ERASE00 ) // factory default settings because game doesn't init them properly otherwise
	ROM_LOAD16_WORD_SWAP( "eeprom1114", 0x0000, 0x0080, CRC(427d90d2) SHA1(39983f9b22b1e9221f7f745f7e84ddcf44d03a08) )
ROM_END


/***************************************************************************

PCB Layout (Head Panic)
----------

ESD 05-28-99
+-----------------------------------------+
|            3812 3014 W24165 6295 ESD4   |
|VOL                 ESD3                 |
|                   PAL Z80               |
|                                         |
|                       +-------+    SM4  |
|      W24165    62256  | ESD   |         |
|J     W24165    62256  |CRTC99 |    SM3  |
|A               62256  |       |         |
|M               62256  +-------+    PAL  |
|M   68000 ESD1                      PAL  |
|A     PAL ESD2 W24165  +-------+   ESD5  |
|      PAL      W24165  |ACTEL  |         |
|                       |A40MX04|         |
|               W24165  |       |   SM2   |
|               W24165  +-------+         |
|S1 93C46       W24165   PAL        SM1   |
|S2 16MHz 14MHZ W24165   PAL PAL          |
+-----------------------------------------+

Notes:
      68000 (MC68HC000FN16)
      Z80 (Z84C00006FEC)
      OKI6295 label AD65 (sound)
      YM3812 label U6612 (sound)
      YM3014 label U6614 (sound)
      A40MX04 - Actel A40MX04-F FPGA (PLCC84)
      CRTC99  - ESD CRTC99 Graphics Controller (QFP240)

      ESD1, ESD2 are 27C2001
      ESD3 is a 27C2000
      ESD4 is a 27C010 Mask ROM
      ESD5 is a 27C040
      SM1, SM2, SM3 & SM4 are MX29F1610MC 16M SOP44 smt flash ROM

1x connector JAMMA
1x trimmer (volume)
2x pushbutton


ESD 08-26-1999 (ESD 06-10-1999 nearly identical)
|-----------------------------------------|
|  3014  3812 6116   6295   ESD4          |
|VOL        ESD3    Z80                 * |
|             PAL                   ESD6  |
|                            6116       * |
|       6116            PAL  6116   ESD7  |
|       6116           |-------|        * |
|J                PAL  | ESD   |        * |
|A                PAL  |CRTC99 |    ESD5  |
|M     PAL             |       |        * |
|M     PAL             |-------|          |
|A            ESD1                        |
|      68000  ESD2        |-------|       |
|                         |ACTEL  | 6116  |
|                         |A40MX04| 6116  |
|   93C46      MCM6206    |       | 6116  |
|              MCM6206    |-------| 6116  |
|SW1 16MHz PAL MCM6206                    |
|SW2 14MHz PAL MCM6206    ESD8 %  ESD9    |
|-----------------------------------------|

Notes:
      HSync: 15.625kHz
      VSync: 60Hz
      MCM6206 is 32kx8 SRAM
      6116 is 8kx8 SRAM

      * : Board has positions for 6x standard 32 pin EPROMs but only position ESD5 is populated
          with an EPROM. In between the unpopulated positions are 2x smt pads. These are populated
          with 2x 16M SOP44 smt Mask ROMs.
      % : ROMs ESD8 and ESD9 are also 16M SOP44 smt Mask ROMs, though these are dedicated smt
          locations (i.e. no option for EPROMs at this location)

Note: Some versions of this PCB used larger EPROMs with the data repeated:
      ESD 3 @ SU06 as a 27C040 with data repeated 2x (CRC32 0xC668D443)
      ESD 4 @ SU10 as a 27C040 with data repeated 4x (CRC32 0x5692FE92)

***************************************************************************/


ROM_START( hedpanic ) // Story line & game instructions in English
	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "esd2.cu03", 0x000000, 0x040000, CRC(7c7be3bb) SHA1(d43ad7a967e1ef79ee0cf50d3842cc9174fbef3a) )
	ROM_LOAD16_BYTE( "esd1.cu02", 0x000001, 0x040000, CRC(42405e9d) SHA1(0fa088b8bd921e42cedcc4083dfe41bc9888dfd1) )

	ROM_REGION( 0x40000, "audiocpu", 0 )        // Z80 Code
	ROM_LOAD( "esd3.su06", 0x00000, 0x40000, CRC(a88d4424) SHA1(eefb5ac79632931a36f360713c482cd079891f91) ) // AT27C020 mask ROM

	ROM_REGION( 0x600000, "spr", 0 )    // Sprites, 16x16x5
	ROM_LOAD( "esd7.ju02",        0x000000, 0x200000, CRC(055d525f) SHA1(85ad474691f96e47311a1904015d1c92d3b2d607) )
	ROM_LOAD( "esd6.ju01",        0x200000, 0x200000, CRC(5858372c) SHA1(dc96112587df681d53cf7449bd39477919978325) )
	// expand this to take up 0x200000 bytes too so we can decode it
	ROM_LOAD16_BYTE( "esd5.ju07", 0x400000, 0x080000, CRC(bd785921) SHA1(c8bcb38d5aa6f5a27f0dedf7efd1d6737d59b4ca) )
	ROM_FILL(                     0x500000, 0x100000, 0x00 )

	ROM_REGION( 0x400000, "bgs", 0 )    // Layers, 16x16x8
	ROM_LOAD32_WORD( "esd8.fu35", 0x000000, 0x200000, CRC(23aceb4f) SHA1(35d9ebc33b9e1515e47750cfcdfc0bf8bf44b71d) )
	ROM_LOAD32_WORD( "esd9.fu34", 0x000002, 0x200000, CRC(76b46cd2) SHA1(679cbf50ae5935e8848868081ecef4ec66424f6c) )

	ROM_REGION( 0x40000, "oki", 0 ) // Samples
	ROM_LOAD( "esd4.su10", 0x000000, 0x020000, CRC(3c11c590) SHA1(cb33845c3dc0501fff8055c2d66f412881089df1) ) // AT27010 mask ROM

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "hedpanic.nv", 0x0000, 0x0080, CRC(e91f4038) SHA1(f492de71170900f87912a272ab4f4a3a37ba31fe) )
ROM_END


ROM_START( hedpanicf ) // Story line in Japanese, game instructions in English
	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "esd2", 0x000000, 0x040000, CRC(8cccc691) SHA1(d6a5dd6c21a67638b9023182f77780282b9b04e5) ) // CU03
	ROM_LOAD16_BYTE( "esd1", 0x000001, 0x040000, CRC(d8574925) SHA1(bd4990778b90a49aa6b10f8cf6709ce2424f546a) ) // CU02

	ROM_REGION( 0x40000, "audiocpu", 0 )        // Z80 Code
	ROM_LOAD( "esd3.su06", 0x00000, 0x40000, CRC(a88d4424) SHA1(eefb5ac79632931a36f360713c482cd079891f91) ) // AT27C020 mask ROM

	ROM_REGION( 0x600000, "spr", 0 )    // Sprites, 16x16x5
	ROM_LOAD( "esd7.ju02",        0x000000, 0x200000, CRC(055d525f) SHA1(85ad474691f96e47311a1904015d1c92d3b2d607) )
	ROM_LOAD( "esd6.ju01",        0x200000, 0x200000, CRC(5858372c) SHA1(dc96112587df681d53cf7449bd39477919978325) )
	// expand this to take up 0x200000 bytes too so we can decode it
	ROM_LOAD16_BYTE( "esd5.ju07", 0x400000, 0x080000, CRC(bd785921) SHA1(c8bcb38d5aa6f5a27f0dedf7efd1d6737d59b4ca) )
	ROM_FILL(                     0x500000, 0x100000, 0x00 )

	ROM_REGION( 0x400000, "bgs", 0 )    // Layers, 16x16x8
	ROM_LOAD32_WORD( "esd8.fu35", 0x000000, 0x200000, CRC(23aceb4f) SHA1(35d9ebc33b9e1515e47750cfcdfc0bf8bf44b71d) )
	ROM_LOAD32_WORD( "esd9.fu34", 0x000002, 0x200000, CRC(76b46cd2) SHA1(679cbf50ae5935e8848868081ecef4ec66424f6c) )

	ROM_REGION( 0x40000, "oki", 0 ) // Samples
	ROM_LOAD( "esd4.su10", 0x000000, 0x020000, CRC(3c11c590) SHA1(cb33845c3dc0501fff8055c2d66f412881089df1) ) // AT27010 mask ROM

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "hedpanic.nv", 0x0000, 0x0080, CRC(e91f4038) SHA1(f492de71170900f87912a272ab4f4a3a37ba31fe) )
ROM_END


ROM_START( hedpanica ) // Story line & game instructions in English, copyright year is 1999 - ESD 06-10-1999 PCB
	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "esd12.cu03", 0x000000, 0x040000, CRC(deb7e0a0) SHA1(ef3a00e9bfdffd7c89326ad97a261f9a7b9863ae) ) // CU03
	ROM_LOAD16_BYTE( "esd11.cu02", 0x000001, 0x040000, CRC(e1418f23) SHA1(39f14172d9b1a0d47edfe2456362fddc22f60066) ) // CU02

	ROM_REGION( 0x40000, "audiocpu", 0 )        // Z80 Code
	ROM_LOAD( "esd3.su06", 0x00000, 0x40000, CRC(a88d4424) SHA1(eefb5ac79632931a36f360713c482cd079891f91) ) // AT27C020 mask ROM

	ROM_REGION( 0x600000, "spr", 0 )    // Sprites, 16x16x5
	ROM_LOAD( "ju06",            0x000000, 0x200000, CRC(9f6f6193) SHA1(c7c7ae6898ab7177eefb0e525d827666e2af9f7e) )
	ROM_LOAD( "ju04",            0x200000, 0x200000, CRC(4f3503d7) SHA1(4bed795c7328e0ebfa97688918eb8a908c29deb8) )
	// expand this to take up 0x200000 bytes too so we can decode it
	ROM_LOAD16_BYTE( "esd5.bin", 0x400000, 0x080000, CRC(6968265a) SHA1(84b4f2d8b3bf6ea4117fa8281c76b58df778261d) ) // JU07
	ROM_FILL(                    0x500000, 0x100000, 0x00 )

	ROM_REGION( 0x400000, "bgs", 0 )    // Layers, 16x16x8
	ROM_LOAD32_WORD( "fu35", 0x000000, 0x200000, CRC(9b5a45c5) SHA1(fbd8bc6ccc068d2cc7fe4f575fa0847f53e786ab) )
	ROM_LOAD32_WORD( "fu34", 0x000002, 0x200000, CRC(8f2099cc) SHA1(40795ae5fb8de613c2d5b6147992c153695bf698) )

	ROM_REGION( 0x80000, "oki", 0 ) // Samples
	ROM_LOAD( "esd4.bin", 0x000000, 0x080000, CRC(5692fe92) SHA1(4423039cb437ab36d198b212ef394bf1704be404) ) // SU10

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "hedpanic.nv", 0x0000, 0x0080, CRC(e91f4038) SHA1(f492de71170900f87912a272ab4f4a3a37ba31fe) )
ROM_END


ROM_START( hedpanico ) // Story line & game instructions in English, copyright year is 1999 - ESD 05-28-99 PCB which uses older style sprites
	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "esd2.rom", 0x000000, 0x040000, CRC(70b08424) SHA1(2ba4fb3b749e31db4239a9173b8509366400152f) ) // CU03
	ROM_LOAD16_BYTE( "esd1.rom", 0x000001, 0x040000, CRC(4e0682c5) SHA1(f4117f31b6426d7bf126a6c62c489b9347885b42) ) // CU02

	ROM_REGION( 0x40000, "audiocpu", 0 )        // Z80 Code
	ROM_LOAD( "esd3.su06", 0x00000, 0x40000, CRC(a88d4424) SHA1(eefb5ac79632931a36f360713c482cd079891f91) ) // AT27C020 mask ROM

	ROM_REGION( 0x600000, "spr", 0 )    // Sprites, 16x16x5
	ROM_LOAD( "sm1.ju01",        0x000000, 0x200000, CRC(8083813f) SHA1(9492e7e844e45d59f0506f69d40c338b27bd3ce3) )
	ROM_LOAD( "sm2.ju02",        0x200000, 0x200000, CRC(7a9610e4) SHA1(21ae3ec3fbddfc66416c109b091bd885d5ba0558) )
	// expand this to take up 0x200000 bytes too so we can decode it
	ROM_LOAD16_BYTE( "esd5.rom", 0x400000, 0x080000, CRC(82c5727f) SHA1(017f1d0c94475c51d17f12e24895f47a273a2dbb) ) // JU07
	ROM_FILL(                    0x500000, 0x100000, 0x00 )

	ROM_REGION( 0x400000, "bgs", 0 )    // Layers, 16x16x8
	ROM_LOAD32_WORD( "sm3.fu35", 0x000000, 0x200000, CRC(94dd4cfc) SHA1(a3f9c49611f0bc9d26166dafb44e2c5ebbb31127) )
	ROM_LOAD32_WORD( "sm4.fu34", 0x000002, 0x200000, CRC(6da0fb9e) SHA1(c4e7487953f45c5f6ce2ebe558b4c325f6ec54eb) )

	ROM_REGION( 0x40000, "oki", 0 ) // Samples
	ROM_LOAD( "esd4.rom", 0x000000, 0x020000, CRC(d7ca6806) SHA1(8ad668bfb5b7561cc0f3e36dfc3c936b136a4274) ) // SU10

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "hedpanic.nv", 0x0000, 0x0080, CRC(e91f4038) SHA1(f492de71170900f87912a272ab4f4a3a37ba31fe) )
ROM_END


ROM_START( hedpanic830 ) // Story line & game instructions in English
	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "esd2.cu03", 0x000000, 0x040000, CRC(955be87f) SHA1(835ec1c9a6d77d095bac7427e428ec25861d91dd) )
	ROM_LOAD16_BYTE( "esd1.cu02", 0x000001, 0x040000, CRC(b56a4fe8) SHA1(dc64c81c1a04af8b248d3e46ddee73eca23c74fd) )

	ROM_REGION( 0x40000, "audiocpu", 0 )  // couldn't read properly, but looking at the various reads it probably matches the other sets
	ROM_LOAD( "esd3.su06", 0x00000, 0x40000, BAD_DUMP CRC(a88d4424) SHA1(eefb5ac79632931a36f360713c482cd079891f91) )

	ROM_REGION( 0x600000, "spr", 0 )    // Sprites, 16x16x5
	ROM_LOAD( "esd7.ju02",        0x000000, 0x200000, CRC(5554ba0f) SHA1(6d02fa6d0712aa820231a24ee7a8396cc575e788) )
	ROM_LOAD( "esd6.ju01",        0x200000, 0x200000, CRC(ffa6eb26) SHA1(c2e3371925604db0f46991cbfd39d763518ed89c) )
	// expand this to take up 0x200000 bytes too so we can decode it
	ROM_LOAD16_BYTE( "esd5.ju07", 0x400000, 0x080000, CRC(95269b3c) SHA1(df106d9e8d4c1484ff86170adcb1932d4d02f265) )
	ROM_FILL(                     0x500000, 0x100000, 0x00 )

	ROM_REGION( 0x400000, "bgs", 0 )    // Layers, 16x16x8
	ROM_LOAD32_WORD( "esd8.fu35", 0x000000, 0x200000, CRC(23aceb4f) SHA1(35d9ebc33b9e1515e47750cfcdfc0bf8bf44b71d) )
	ROM_LOAD32_WORD( "esd9.fu34", 0x000002, 0x200000, CRC(76b46cd2) SHA1(679cbf50ae5935e8848868081ecef4ec66424f6c) )

	ROM_REGION( 0x40000, "oki", 0 ) // Samples
	ROM_LOAD( "esd4.su10", 0x000000, 0x040000, CRC(5ba89bf1) SHA1(f3c5b588dc3fbd0dba3120aa60f76c08db4c6e08) ) // 1ST AND 2ND HALF IDENTICAL

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "hedpanic.nv", 0x0000, 0x0080, CRC(e91f4038) SHA1(f492de71170900f87912a272ab4f4a3a37ba31fe) )
ROM_END



/*

Deluxe 5 (c) 2000 ESD
PCB Layout
----------

ESD made in Korea
|-----------------------------------------|
|     3014 3812 6116  6295   ESD4.SU10    |
|VOL      ESD3.SU06  Z80              JU03|
|             PAL                         |
|                           6116      JU04|
|       6116           PAL  6116          |
|       6116     PAL  |-------|       JU05|
|J               PAL  | ESD   |           |
|A                    |CRTC99 |       JU06|
|M     PAL            | 0016  |           |
|M     PAL            |-------|       JU07|
|A    68000   ESD1.CU03                   |
|             ESD2.CU02   |-------|       |
|                         |ACTEL  |       |
|                         |A40MX04|   6116|
|              MCM6206    |  0008 |   6116|
|              MCM6206    |-------|       |
|SW1 16MHz PAL MCM6206                6116|
|SW2 14MHz PAL MCM6206   FU35   FU34  6116|
|-----------------------------------------|

Notes:
      68000 (MC68HC000FN16)
      Z80 (Z84C00006FEC-Z80CPU)
      OKI6295 label AD65 (sound)
      YM3812 label U6612 (sound)
      YM3014 label U6614 (sound)
      MCM6206 - 32k x8 SRAM (SOJ28)
      6116    - 2k x8 SRAM (SOP28)
      A40MX04 - Actel A40MX04-F FPGA (PLCC84)
      CRTC99  - ESD CRTC99 Graphics Controller (QFP240)
      ESD1-2  - M27C2001
      JU03-8  - AM27C020
      FU34,FU35  -  MX29F1610MC

1x connector JAMMA
1x trimmer (volume)
2x pushbutton

      * : Board has positions for 6x standard 32 pin EPROMs but only 5 positions are populated with an EPROM.

*/

ROM_START( deluxe5 ) // Deluxe 5
	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "esd2.cu02", 0x000000, 0x040000,  CRC(d077dc13) SHA1(d83feadb29674d56a5f019641f402798c7ba8d61) ) // M27C2001 EPROM
	ROM_LOAD16_BYTE( "esd1.cu03", 0x000001, 0x040000,  CRC(15d6644f) SHA1(cfb8168167389855f906658511d1dc7460e13100) ) // M27C2001 EPROM

	ROM_REGION( 0x40000, "audiocpu", 0 )        // Z80 Code
	ROM_LOAD( "esd3.su06", 0x00000, 0x40000, CRC(31de379a) SHA1(a0c9a9cec7207cc4ba33abb68bef62d7eb8e75e9) ) // AM27C020 mask ROM

	ROM_REGION( 0x180000, "spr", 0 )    // Sprites, 16x16x5
	ROM_LOAD16_BYTE( "am27c020.ju06", 0x000000, 0x040000, CRC(8b853bce) SHA1(fa6e654fc965d88bb426b76cdce3417f357b25f3) ) // AM27C020 mask ROMs with no label
	ROM_LOAD16_BYTE( "am27c020.ju05", 0x000001, 0x040000, CRC(bbe81779) SHA1(750387fb4aaa04b7f4f1d3985896f5e11219e3ea) )
	ROM_LOAD16_BYTE( "am27c020.ju04", 0x080000, 0x040000, CRC(40fa2c2f) SHA1(b9d9bfdc9343f00bad9749c76472f064c509cfce) )
	ROM_LOAD16_BYTE( "am27c020.ju03", 0x080001, 0x040000, CRC(aa130fd3) SHA1(46a55d8ca59a52e610600fdba76d9729528d2871) )
	ROM_LOAD16_BYTE( "am27c020.ju07", 0x100000, 0x040000, CRC(d414c3af) SHA1(9299b07a8c7a3e30a1bb6028204a049a7cb510f7) )

	ROM_REGION( 0x400000, "bgs", 0 )    // Layers, 16x16x8
	ROM_LOAD32_WORD( "fu35", 0x000000, 0x200000, CRC(ae10242a) SHA1(f3d18c0cb7951b5f7ee47aa2856b7554088328ed) ) // No labels on the flash ROMs
	ROM_LOAD32_WORD( "fu34", 0x000002, 0x200000, CRC(248b8c05) SHA1(fe7bcc05ae0dd0a27c6ba4beb4ac155a8f3d7f7e) ) // No labels on the flash ROMs

	ROM_REGION( 0x40000, "oki", 0 ) // Samples
	ROM_LOAD( "esd4.su10", 0x00000, 0x20000, CRC(23f2b7d9) SHA1(328c951d14674760df68486841c933bad0d59fe3) ) // AT27C010 mask ROM

	ROM_REGION16_BE( 0x80, "eeprom", ROMREGION_ERASE00 ) // factory default settings because game doesn't init them properly otherwise
	ROM_LOAD16_WORD_SWAP( "eeprom", 0x0000, 0x0080, CRC(4539a8a0) SHA1(b882110b489e61ac5421fbe3551d9ee323b5d86b) )
ROM_END

ROM_START( deluxe5a ) // Deluxe 5
	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "esd2.cu02", 0x000000, 0x040000,  CRC(c67bf757) SHA1(c90d486088d4aedbc9dd307cf1a8d5febf6fdba0) ) // sldh
	ROM_LOAD16_BYTE( "esd1.cu03", 0x000001, 0x040000,  CRC(24f4d7b9) SHA1(bb0eabdd72a475149d6df768d9d29b545f061e54) ) // sldh

	ROM_REGION( 0x40000, "audiocpu", 0 )        // Z80 Code
	ROM_LOAD( "esd3.su06", 0x00000, 0x40000, CRC(31de379a) SHA1(a0c9a9cec7207cc4ba33abb68bef62d7eb8e75e9) ) // AM27C020 mask ROM

	ROM_REGION( 0x180000, "spr", 0 )    // Sprites, 16x16x5
	ROM_LOAD16_BYTE( "am27c020.ju06", 0x000000, 0x040000, CRC(8b853bce) SHA1(fa6e654fc965d88bb426b76cdce3417f357b25f3) ) // AM27C020 mask ROMs with no label
	ROM_LOAD16_BYTE( "am27c020.ju05", 0x000001, 0x040000, CRC(bbe81779) SHA1(750387fb4aaa04b7f4f1d3985896f5e11219e3ea) )
	ROM_LOAD16_BYTE( "am27c020.ju04", 0x080000, 0x040000, CRC(40fa2c2f) SHA1(b9d9bfdc9343f00bad9749c76472f064c509cfce) )
	ROM_LOAD16_BYTE( "am27c020.ju03", 0x080001, 0x040000, CRC(aa130fd3) SHA1(46a55d8ca59a52e610600fdba76d9729528d2871) )
	ROM_LOAD16_BYTE( "am27c020.ju07", 0x100000, 0x040000, CRC(d414c3af) SHA1(9299b07a8c7a3e30a1bb6028204a049a7cb510f7) )

	ROM_REGION( 0x400000, "bgs", 0 )    // Layers, 16x16x8
	ROM_LOAD32_WORD( "fu35", 0x000000, 0x200000, CRC(ae10242a) SHA1(f3d18c0cb7951b5f7ee47aa2856b7554088328ed) ) // No labels on the flash ROMs
	ROM_LOAD32_WORD( "fu34", 0x000002, 0x200000, CRC(248b8c05) SHA1(fe7bcc05ae0dd0a27c6ba4beb4ac155a8f3d7f7e) ) // No labels on the flash ROMs

	ROM_REGION( 0x40000, "oki", 0 ) // Samples
	ROM_LOAD( "esd4.su10", 0x00000, 0x20000, CRC(23f2b7d9) SHA1(328c951d14674760df68486841c933bad0d59fe3) ) // AT27C010 mask ROM

	ROM_REGION16_BE( 0x80, "eeprom", ROMREGION_ERASE00 ) // factory default settings because game doesn't init them properly otherwise
	ROM_LOAD16_WORD_SWAP( "eeprom", 0x0000, 0x0080, CRC(4539a8a0) SHA1(b882110b489e61ac5421fbe3551d9ee323b5d86b) )
ROM_END

ROM_START( deluxe5b ) // Deluxe 5
	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "esd2.cu02", 0x000000, 0x040000,  CRC(72a67495) SHA1(4fd5871621a6d1d4ea7a23c84f5796ee99caf857) ) // sldh
	ROM_LOAD16_BYTE( "esd1.cu03", 0x000001, 0x040000,  CRC(7cc119c8) SHA1(4d2d37e815ab3211ff88c2e6584b4eaee1cd202d) ) // sldh

	ROM_REGION( 0x40000, "audiocpu", 0 )        // Z80 Code
	ROM_LOAD( "esd3.su06", 0x00000, 0x40000, CRC(31de379a) SHA1(a0c9a9cec7207cc4ba33abb68bef62d7eb8e75e9) ) // AM27C020 mask ROM

	ROM_REGION( 0x180000, "spr", 0 )    // Sprites, 16x16x5
	ROM_LOAD16_BYTE( "am27c020.ju06", 0x000000, 0x040000, CRC(8b853bce) SHA1(fa6e654fc965d88bb426b76cdce3417f357b25f3) ) // AM27C020 mask ROMs with no label
	ROM_LOAD16_BYTE( "am27c020.ju05", 0x000001, 0x040000, CRC(bbe81779) SHA1(750387fb4aaa04b7f4f1d3985896f5e11219e3ea) )
	ROM_LOAD16_BYTE( "am27c020.ju04", 0x080000, 0x040000, CRC(40fa2c2f) SHA1(b9d9bfdc9343f00bad9749c76472f064c509cfce) )
	ROM_LOAD16_BYTE( "am27c020.ju03", 0x080001, 0x040000, CRC(aa130fd3) SHA1(46a55d8ca59a52e610600fdba76d9729528d2871) )
	ROM_LOAD16_BYTE( "am27c020.ju07", 0x100000, 0x040000, CRC(d414c3af) SHA1(9299b07a8c7a3e30a1bb6028204a049a7cb510f7) )

	ROM_REGION( 0x400000, "bgs", 0 )    // Layers, 16x16x8
	ROM_LOAD32_WORD( "fu35", 0x000000, 0x200000, CRC(ae10242a) SHA1(f3d18c0cb7951b5f7ee47aa2856b7554088328ed) ) // No labels on the flash ROMs
	ROM_LOAD32_WORD( "fu34", 0x000002, 0x200000, CRC(248b8c05) SHA1(fe7bcc05ae0dd0a27c6ba4beb4ac155a8f3d7f7e) ) // No labels on the flash ROMs

	ROM_REGION( 0x40000, "oki", 0 ) // Samples
	ROM_LOAD( "esd4.su10", 0x00000, 0x20000, CRC(23f2b7d9) SHA1(328c951d14674760df68486841c933bad0d59fe3) ) // AT27C010 mask ROM

	ROM_REGION16_BE( 0x80, "eeprom", ROMREGION_ERASE00 ) // factory default settings because game doesn't init them properly otherwise
	ROM_LOAD16_WORD_SWAP( "eeprom", 0x0000, 0x0080, CRC(4539a8a0) SHA1(b882110b489e61ac5421fbe3551d9ee323b5d86b) )
ROM_END


ROM_START( deluxe4u ) // Deluxe 4 U - Removes Blackjack game, but otherwise same as Deluxe 5
	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "2.cu02", 0x000000, 0x040000,  CRC(db213e1f) SHA1(bf9c49635f79b92a761715138528200106aa86ae) )
	ROM_LOAD16_BYTE( "1.cu03", 0x000001, 0x040000,  CRC(fbf14d74) SHA1(5ff5bf4ff55609452d5b8a49d8658f878541ce60) )

	ROM_REGION( 0x40000, "audiocpu", 0 )        // Z80 Code
	ROM_LOAD( "esd3.su06", 0x00000, 0x40000, CRC(31de379a) SHA1(a0c9a9cec7207cc4ba33abb68bef62d7eb8e75e9) ) // AM27C020 mask ROM

	ROM_REGION( 0x180000, "spr", 0 )    // Sprites, 16x16x5
	ROM_LOAD16_BYTE( "am27c020.ju06", 0x000000, 0x040000, CRC(8b853bce) SHA1(fa6e654fc965d88bb426b76cdce3417f357b25f3) ) // AM27C020 mask ROMs with no label
	ROM_LOAD16_BYTE( "am27c020.ju05", 0x000001, 0x040000, CRC(bbe81779) SHA1(750387fb4aaa04b7f4f1d3985896f5e11219e3ea) )
	ROM_LOAD16_BYTE( "am27c020.ju04", 0x080000, 0x040000, CRC(40fa2c2f) SHA1(b9d9bfdc9343f00bad9749c76472f064c509cfce) )
	ROM_LOAD16_BYTE( "am27c020.ju03", 0x080001, 0x040000, CRC(aa130fd3) SHA1(46a55d8ca59a52e610600fdba76d9729528d2871) )
	ROM_LOAD16_BYTE( "am27c020.ju07", 0x100000, 0x040000, CRC(d414c3af) SHA1(9299b07a8c7a3e30a1bb6028204a049a7cb510f7) )

	ROM_REGION( 0x400000, "bgs", 0 )    // Layers, 16x16x8
	ROM_LOAD32_WORD( "fu35", 0x000000, 0x200000, CRC(6df14570) SHA1(fa4fc64c984d6a94fe61ec809ec515e840388704) ) // Specific to Deluxe 4 U - No labels on the flash ROMs
	ROM_LOAD32_WORD( "fu34", 0x000002, 0x200000, CRC(93175d6d) SHA1(691832134f43e17bb767dff080b2736288961414) ) // Specific to Deluxe 4 U - No labels on the flash ROMs

	ROM_REGION( 0x40000, "oki", 0 ) // Samples
	ROM_LOAD( "esd4.su10", 0x00000, 0x20000, CRC(23f2b7d9) SHA1(328c951d14674760df68486841c933bad0d59fe3) ) // AT27C010 mask ROM

	ROM_REGION16_BE( 0x80, "eeprom", ROMREGION_ERASE00 ) // factory default settings because game doesn't init them properly otherwise
	ROM_LOAD16_WORD_SWAP( "eeprom", 0x0000, 0x0080, CRC(4539a8a0) SHA1(b882110b489e61ac5421fbe3551d9ee323b5d86b) )
ROM_END


/* Tang Tang

Tang Tang (ESD)
------------------------

PCB Layout
----------

ESD made in Korea
|-----------------------------------------|
| U6614 U6612 6116   AD65   ESD4.SU10     |
|VOL      ESD3.SU06  Z80          ROM.JU04|
|             PAL                         |
|                            6116 ROM.JU05|
|       6116            PAL  6116         |
|       6116           |-------|  ROM.JU06|
|J                PAL  | ESD   |          |
|A                PAL  |CRTC99 |  ROM.JU07|
|M     PAL             | 0016  |          |
|M     PAL             |-------|  ROM.JU08|
|A    68000    ESD1.CU03                  |
|              ESD2.CU02  |-------|       |
|                         |ACTEL  | 6116  |
|                         |A40MX04| 6116  |
|              MCM6206    |  0008 | 6116  |
|              MCM6206    |-------| 6116  |
|SW1 16MHz PAL MCM6206                6116|
|SW2 14MHz PAL MCM6206   FU35   FU34  6116|
|-----------------------------------------|

Notes:
      68000 (MC68HC000FN16-2E60R-QQJU9508)
      Z80 (Z84C00006FEC-Z80CPU-9618Z3)
      OKI6295 label AD65 (sound)
      YM3812 label U6612 (sound)
      YM3014 label U6614 (sound)
      MCM6206 - 32k x8 SRAM (SOJ28)
      6116    - 2k x8 SRAM (SOP28)
      A40MX04 - Actel A40MX04-F FPGA (PLCC84)
      CRTC99  - ESD CRTC99 Graphics Controller (QFP240)
      ESD1-2  - 27C2001
      ESD3-4  - 27C2000
      JU04-8  - MX27C2000PC
      FU34,FU35  -  MX29F1610MC
1x connector JAMMA
1x trimmer (volume)
2x pushbutton

      * : Board has positions for 6x standard 32 pin EPROMs but only 5 positions are populated with an EPROM.
      * : Tang Tang also known to be found on a ESD 05-28-99 PCB (see Head Panic above for PCB layout)

*/

ROM_START( tangtang )
	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "esd2.cu02", 0x000000, 0x040000,  CRC(b6dd6e3d) SHA1(44d2663827c45267eb154c873f3bd2e9e2bf3d3f) )
	ROM_LOAD16_BYTE( "esd1.cu03", 0x000001, 0x040000,  CRC(b6c0f2f4) SHA1(68ad76e7e380c728dda200a852729e034d9c9f4c) )

	ROM_REGION( 0x40000, "audiocpu", 0 )        // Z80 Code
	ROM_LOAD( "esd3.su06", 0x00000, 0x40000, CRC(d48ecc5c) SHA1(5015dd775980542eb29a08bffe1a09ea87d56272) )

	ROM_REGION( 0x180000, "spr", 0 )    // Sprites, 16x16x5
	ROM_LOAD16_BYTE( "xju07.bin", 0x000000, 0x040000, CRC(556acac3) SHA1(10e919e63b434da80fb261db1d8967cb11e95e00) )
	ROM_LOAD16_BYTE( "xju06.bin", 0x000001, 0x040000, CRC(01f59ff7) SHA1(a62a2d5c2d107f67fecfc08fdb5d801ee39c3875) )
	ROM_LOAD16_BYTE( "xju05.bin", 0x080000, 0x040000, CRC(679302cf) SHA1(911c2f7e0e809ee28e4f2364788fd51d2bcef24e) )
	ROM_LOAD16_BYTE( "xju04.bin", 0x080001, 0x040000, CRC(f999b9d7) SHA1(9e4d0e68cdc429c7563b8ad51c072d68ffed09dc) )
	ROM_LOAD16_BYTE( "xju08.bin", 0x100000, 0x040000, CRC(ecc2d8c7) SHA1(1aabdf7204fcdff8d46cb50de8b097e3775dddf3) )

	ROM_REGION( 0x400000, "bgs", 0 )    // Layers, 16x16x8
	ROM_LOAD32_WORD( "fu35.bin", 0x000000, 0x200000, CRC(84f3f833) SHA1(f84e41d93dc47a58ada800b921a7e5902b7631cd) )
	ROM_LOAD32_WORD( "fu34.bin", 0x000002, 0x200000, CRC(bf91f543) SHA1(7c149fed8b8044850cd6b798622a91c45336cd47) )

	ROM_REGION( 0x40000, "oki", 0 ) // Samples
	ROM_LOAD( "esd4.su10", 0x00000, 0x20000, CRC(f2dfb02d) SHA1(04001488697aad3e5b2d15c9f5a81dc2b7d0952c) )

	ROM_REGION16_BE( 0x80, "eeprom", ROMREGION_ERASE00 ) // default settings because game doesn't init them properly otherwise
	ROM_LOAD16_WORD_SWAP( "eeprom", 0x0000, 0x0080, CRC(00514989) SHA1(86cdca86ed48962e3bda13f9e6f8e573e176dec0) )
ROM_END

/*

SWAT Police (c) 2001 ESD
PCB Layout
----------

ESD made in Korea
|-----------------------------------------|
|     3014 3812 6116  6295   AT27C020     |
|VOL      ESD3.SU06  Z80         ESD1.JU03|
|             PAL                         |
|                           6116 ESD2.JU04|
|       6116           PAL  6116          |
|       6116     PAL  |-------|  ESD3.JU05|
|J               PAL  | ESD   |           |
|A                    |CRTC99 |  ESD4.JU06|
|M     PAL            | 0016  |           |
|M     PAL            |-------|  ESD5.JU07|
|A    68000   ESD.CU03                    |
|             ESD.CU02    |-------|       |
|                         |ACTEL  |       |
|                         |A40MX04|   6116|
|              MCM6206    |  0008 |   6116|
|              MCM6206    |-------|       |
|SW1 16MHz PAL MCM6206                6116|
|SW2 14MHz PAL MCM6206   FU35   FU34  6116|
|-----------------------------------------|

Notes:
      68000 (MC68HC000FN16)
      Z80 (Z84C00006FEC-Z80CPU)
      OKI6295 label AD65 (sound)
      YM3812 label U6612 (sound)
      YM3014 label U6614 (sound)
      MCM6206 - 32k x8 SRAM (SOJ28)
      6116    - 2k x8 SRAM (SOP28)
      A40MX04 - Actel A40MX04-F FPGA (PLCC84)
      CRTC99  - ESD CRTC99 Graphics Controller (QFP240)
      CU02-3  - 27C2001
      JU03-8  - 27C040
      FU34,FU35  -  MX29F1610MC

1x connector JAMMA
1x trimmer (volume)
2x pushbutton

      * : Board has positions for 6x standard 32 pin EPROMs but only 5 positions are populated with an EPROM.

*/

ROM_START( swatpolc ) // SWAT Police
	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "esd.cu02", 0x000000, 0x040000,  CRC(29e0c126) SHA1(7c0356eed4ffdc056b7ec5c1ac07f1c9cc6aeffa) ) // ESD labels but not numbered
	ROM_LOAD16_BYTE( "esd.cu03", 0x000001, 0x040000,  CRC(1070208b) SHA1(1e058774c5aee1de15ffcd26d530b23592286db1) ) // ESD labels but not numbered

	ROM_REGION( 0x40000, "audiocpu", 0 )        // Z80 Code
	ROM_LOAD( "esd3.su06", 0x00000, 0x40000, CRC(80e97dbe) SHA1(d6fae689cd3737777f36c980b9a7d9e42b06a467) ) // 2 ROMs on PCB with an ESD3 label

	ROM_REGION( 0x300000, "spr", 0 )    // Sprites, 16x16x5
	ROM_LOAD16_BYTE( "esd4.ju06", 0x000000, 0x080000, CRC(bde1b130) SHA1(e45a2257f8c4d107dfb7401b5ae1b79951052bc6) )
	ROM_LOAD16_BYTE( "esd3.ju05", 0x000001, 0x080000, CRC(e8d9c092) SHA1(80e1f1d4dad48c7be3d4b72c4a82d5388fd493c7) )
	ROM_LOAD16_BYTE( "esd2.ju04", 0x100000, 0x080000, CRC(9c1752f2) SHA1(2e8c377137258498564749413b49e156180e806a) )
	ROM_LOAD16_BYTE( "esd1.ju03", 0x100001, 0x080000, CRC(17fcc5e7) SHA1(ad57d2b0c0062f6f8c7732df57e4d12ca47c1bb8) )
	ROM_LOAD16_BYTE( "esd5.ju07", 0x200000, 0x080000, CRC(d2c27f03) SHA1(7cbdf7f7ff17df16ca81823f69e82ae1cf96b714) )

	ROM_REGION( 0x400000, "bgs", 0 )    // Layers, 16x16x8
	ROM_LOAD32_WORD( "fu35", 0x000000, 0x200000, CRC(c55897c5) SHA1(f6e0ef1c2fcfe6a511fe787a3abeff4da16d1b54) ) // No labels on the flash ROMs
	ROM_LOAD32_WORD( "fu34", 0x000002, 0x200000, CRC(7117a6a2) SHA1(17c0ab02698cffa0582ed2d2b7dbb7fed8cd9393) ) // No labels on the flash ROMs

	ROM_REGION( 0x40000, "oki", 0 ) // Samples
	ROM_LOAD( "at27c020.su10", 0x00000, 0x40000, CRC(c43efec2) SHA1(4ef328d8703b81328de09ecc4328763aba06e883) ) // AT27C020 mask ROM with no label
ROM_END


/*

Jumping Pop
ESD, 2001

PCB Layout
----------

ESD 12-04-00
|------------------------------------------------------|
| TDA1519A                62256         PAL            |
| SAMPLES.BIN YM3014      62256         BG0.BIN        |
|             YM3812    |---------|     BG1.BIN        |
|6295   Z80   6116      |         |                    |
|          Z80_PRG.BIN  |A40MX04  |PAL                 |
|                       |         |                    |
|J                      |         |                    |
|A PAL                  |---------|                    |
|M                           6116                      |
|M                           6116                      |
|A     14MHz                 6116                      |
|      16MHz                 6116|---------|           |
|      68K_PRG.BIN        PAL    |         |           |
|                         PAL    |A40MX04  |           |
|              |-----|    PAL    |         |  SP0.BIN  |
|      62256   |68000|           |         |  SP1.BIN  |
|DIP1  62256   |     |           |---------|           |
|      PAL     |-----|           6116  6116            |
|DIP2  PAL                       6116  6116            |
|------------------------------------------------------|
Notes:
      68000   - Motorola MC68EC000FU10, running at 16.000MHz (QFP64)
      YM3812  - Yamaha YM3812, running at 3.500MHz [14 / 4] (DIP24)
      YM3014  - Yamaha YM3014 16bit Serial DAC (DIP8)
      Z80     - Zilog Z84C0006FEC, running at 3.500MHz [14 / 4] (QFP44)
      6295    - Oki M6295, running at 875kHz [14 / 16], samples rate 6.628787879kHz [875000 /132] (QFP44)
      A40MX04 - Actel A40MX04-F FPGA (x2, PLCC84)
      TDA1519A- Philips TDA1519A Dual 6W Power Amplifier
      DIP1/2  - 8 Position Dip Switch
      62256   - 8K x8 SRAM (x4, DIP28)
      6116    - 2K x8 SRAM (x9, DIP24)
      VSync   - 60Hz

      ROMs -
              Filename      Type                                      Use
              ---------------------------------------------------------------------------
              68K_PRG.BIN   Hitachi HN27C4096 256K x16 EPROM          68000 Program
              Z80_PRG.BIN   Atmel AT27C020 256K x8 OTP mask ROM       Z80 Program
              SAMPLES.BIN   Atmel AT27C020 256K x8 OTP mask ROM       Oki M6295 Samples
              BG0/1.BIN     Macronix 29F8100MC 1M x8 SOP44 Flash ROM  Background Graphics
              SP0/1.BIN     Macronix 29F8100MC 1M x8 SOP44 Flash ROM  Sprite Graphics

              Note there are no IC locations on the PCB, so the extension of the ROMs is just 'BIN'

-------------------------------------------

Jumping Pop (c) 2001 Emag Soft

PCB Layout
----------

ESD 11-09-98
+-----------------------------------------+
|       YM3812   6116    su10         fu27|
|VOL    YM3014   su06    M6295        fu32|
|           PAL  Z80                  fu26|
|        6116                         fu30|
|J       6116            Actel        fu28|
|A DSWA DSWB    76C256   A40MX04      fu31|
|M              76C256                fu29|
|M    cu03 76C256   6116 Actel    PAL fu33|
|A    cu02 76C256   6116 A40MX04  PAL ju07|
|      68000              6116        ju03|
|       PAL               6116 PAL    ju04|
|       PAL               6116 PAL    ju05|
|16MHz 14MHz              6116 PAL    ju06|
+-----------------------------------------+

PCB No. ESD 11-09-98
  CPU: MC68HC000FN16 (68000, 68 pin PLCC socketed)
  SND: Z80 (Z0840006PSC), U6614/U6612 (YM3014/YM3812), AD-65 (OKI M6295)
  OSC: 16.000MHz, 14.000MHz
  RAM: 4 x 62256, 9 x 6116
 DIPS: 2 x 8 position
Other: 2 x Actel A40MX04-F FPGA (PLCC84)

JU07 and FU30 through FU32 unpopulated

*/

ROM_START( jumppop )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_WORD_SWAP ("68k_prg.bin", 0x00000, 0x80000, CRC(123536b9) SHA1(3597dec81e98d7bdf4ea9053983e62f127defcb7) )

	ROM_REGION( 0x80000, "audiocpu", 0 ) // Z80 code
	ROM_LOAD( "z80_prg.bin", 0x00000, 0x40000, CRC(a88d4424) SHA1(eefb5ac79632931a36f360713c482cd079891f91) )

	ROM_REGION( 0x200000, "spr", 0 ) // 2nd half of these is just unused garbage data from the 'bgs' region
	ROM_LOAD( "sp0.bin", 0x000000, 0x100000, CRC(7c5d0633) SHA1(1fba60073d1d5d4dbd217fde181fa73a9d92bdc6) )
	ROM_LOAD( "sp1.bin", 0x100000, 0x100000, CRC(7eae782e) SHA1(a33c544ad9516ec409c209968e72f63e7cdb934b) )

	ROM_REGION( 0x200000, "bgs", 0 )
	ROM_LOAD32_WORD( "bg1.bin", 0x000000, 0x100000, CRC(5b37f943) SHA1(fe73b839f29d4c32823418711b22f85a5f583ec2) )
	ROM_LOAD32_WORD( "bg0.bin", 0x000002, 0x100000, CRC(35a1363d) SHA1(66c550b0bdea7c8b079f186f5e044f731d31bc58) )

	ROM_REGION( 0x80000, "oki", 0 ) // Oki samples
	ROM_LOAD( "samples.bin", 0x00000, 0x40000, CRC(066f30a7) SHA1(6bdd0210001c597819f7132ffa1dc1b1d55b4e0a) )
ROM_END

// This set displays an a '(c)2001 Emag Soft' copyright and doesn't have the ESD copyright embedded into the 'bgs' tiles
ROM_START( jumppope ) // Running on an original ESD 11-09-98 PCB with original ESD labeled ROMs
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "esd2.cu02", 0x000000, 0x040000, CRC(302dd093) SHA1(fd52dc2342652fd6e6f24942d00a0c2bff83e4ed) ) // 68k_prg.bin  [odd]      99.980164%
	ROM_LOAD16_BYTE( "esd1.cu03", 0x000001, 0x040000, CRC(883392ba) SHA1(7241fd35b0431bbb6e83e4f0eb9026bafbcf1d7f) ) // 68k_prg.bin  [even]     99.979782%

	ROM_REGION( 0x80000, "audiocpu", 0 ) // Z80 code
	ROM_LOAD( "at27c020.su06", 0x00000, 0x40000, CRC(a88d4424) SHA1(eefb5ac79632931a36f360713c482cd079891f91) ) // z80_prg.bin             IDENTICAL

	ROM_REGION( 0x200000, "spr", 0 )
	ROM_LOAD16_BYTE( "esd7.ju03",  0x000000, 0x040000, CRC(9c2970e0) SHA1(000b0f43d4d5434ba6c4834107ade8ebcd509ff8) ) // sp0.bin      [even 1/2] IDENTICAL
	ROM_LOAD16_BYTE( "esd8.ju04",  0x000001, 0x040000, CRC(33bf99b0) SHA1(023ce2948b8130bf8464b3fc6f5543c6f3b1865c) ) // sp0.bin      [odd 1/2]  IDENTICAL
	ROM_LOAD16_BYTE( "esd9.ju05",  0x100000, 0x040000, CRC(671d21fd) SHA1(c9dfe163bd9e46855db7af8daf436b1248df1ed0) ) // sp1.bin      [even 1/2] IDENTICAL
	ROM_LOAD16_BYTE( "esd10.ju06", 0x100001, 0x040000, CRC(85a3cc73) SHA1(ec90f4d2e4244dffbada306a732e50263173203e) ) // sp1.bin      [odd 1/2]  IDENTICAL

	ROM_REGION( 0x200000, "bgs", 0 )
	ROM_LOAD32_BYTE( "esd5.fu28", 0x000000, 0x080000, CRC(0d47f821) SHA1(fc1ef080eb05990909e25d5db59918f1f4e90a67) ) // [even 1/2] 99.769974%, [even 2/2] 99.267578%
	ROM_LOAD32_BYTE( "esd6.fu29", 0x000002, 0x080000, CRC(c01af40d) SHA1(fce0244027d4d4eb5cff1809cf8f404bfe016455) ) // [even 1/2] 99.778366%, [even 2/2] 99.267578%
	ROM_LOAD32_BYTE( "esd4.fu26", 0x000001, 0x080000, CRC(97b409be) SHA1(3a4344ca8ffb0aee046e3c0bab2d7c3f7c0eb204) ) // [odd 1/2]  99.763107%, [odd 2/2]  99.267578%
	ROM_LOAD32_BYTE( "esd3.fu27", 0x000003, 0x080000, CRC(3358a693) SHA1(2e368e5c26755bbe6d04838015fd4ca5e43ccfb5) ) // [odd 1/2]  99.784470%, [odd 2/2]  99.267578%

	ROM_REGION( 0x80000, "oki", 0 ) // Oki samples
	ROM_LOAD( "at27c020.su10", 0x00000, 0x40000, CRC(066f30a7) SHA1(6bdd0210001c597819f7132ffa1dc1b1d55b4e0a) ) // samples.bin             IDENTICAL
ROM_END

// Fantasy Story - not an ESD PCB
ROM_START( fantstry )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "system_rom", 0x00000, 0x80000, CRC(3d7f19ce) SHA1(eb163489adda25a0ece1a21292bfe5818b52cddc) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // PIC16F84A-04/P Code
	ROM_LOAD( "pic16f84a", 0x00000, 0x10000, NO_DUMP )

	ROM_REGION( 0x040000, "okisfx", 0 )
	ROM_LOAD( "voice_rom", 0x00000, 0x040000, CRC(3353e8df) SHA1(22998100e0afa14927ab219dfdbf21c1fd7e27c6) ) // single bank

	ROM_REGION( 0x200000, "okimusic", 0 )
	ROM_LOAD( "sound_rom", 0x00000, 0x200000, CRC(46e3c7fd) SHA1(56ae5c67f05d01bf2679e2c3fe864a38d3bb7f9d) ) // multiple banks, one sample in each

	ROM_REGION( 0x400000, "bgs", 0 )
	ROM_LOAD32_WORD( "graphics_rom_4", 0x000000, 0x200000, CRC(82d5104a) SHA1(23067ca2698eb1ec39a77af75beb79431c054fa9) )
	ROM_LOAD32_WORD( "graphics_rom_5", 0x000002, 0x200000, CRC(9178c370) SHA1(2c0f29c62d36cdb29380a3e63945da1a80e8d170) )

	ROM_REGION( 0x300000, "spr", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "graphics_rom_3", 0x000000, 0x100000, CRC(1bd2b5fc) SHA1(21f32e9d2673c376c0781137db0f2d267b64bc0c) ) // 2bpp
	ROM_LOAD16_WORD_SWAP( "graphics_rom_2", 0x100000, 0x100000, CRC(2020add8) SHA1(a7d950c6485f6abdcf4fc609a85f60df2bafe34e) ) // 2bpp
	ROM_LOAD16_BYTE( "graphics_rom_1", 0x200000, 0x080000, CRC(c8caa752) SHA1(cecc7a18a0bb59554316ed24ed0dc01d58d6ae53) ) // 1bpp
ROM_END

// the following set has a worse sound section: only one OKI with only 0x80000 of ROM.
ROM_START( fantstrya ) // PCB marked: PNXND-MULTI 2002 01 01 - Only EPROMs dumped for now, marked the flash ROMs as BAD_DUMP but they should match
	ROM_REGION( 0x80000, "maincpu", 0 ) // only difference from the above set seems to be they moved the RAM base from 0x100000 to 0x900000
	ROM_LOAD16_WORD_SWAP( "system_rom", 0x00000, 0x80000, CRC(38471eed) SHA1(02f311ea5bdea41092e6754ed120e5e3e6994623) ) // 27c040, sldh

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "pic16f84a", 0x00000, 0x10000, NO_DUMP ) // did they change the program to take into account the one less OKI?

	ROM_REGION( 0x80000, "okimusic", 0 )
	ROM_LOAD( "sound_rom", 0x00000, 0x80000, CRC(44424914) SHA1(5bbe296d2c8de89b9271914164ef178239fa3e05) ) // 27c4000, sldh

	ROM_REGION( 0x400000, "bgs", 0 )
	ROM_LOAD32_WORD( "graphics_rom_4", 0x000000, 0x200000, BAD_DUMP CRC(82d5104a) SHA1(23067ca2698eb1ec39a77af75beb79431c054fa9) )
	ROM_LOAD32_WORD( "graphics_rom_5", 0x000002, 0x200000, BAD_DUMP CRC(9178c370) SHA1(2c0f29c62d36cdb29380a3e63945da1a80e8d170) )

	ROM_REGION( 0x300000, "spr", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "graphics_rom_3", 0x000000, 0x100000, BAD_DUMP CRC(1bd2b5fc) SHA1(21f32e9d2673c376c0781137db0f2d267b64bc0c) ) // 2bpp
	ROM_LOAD16_WORD_SWAP( "graphics_rom_2", 0x100000, 0x100000, BAD_DUMP CRC(2020add8) SHA1(a7d950c6485f6abdcf4fc609a85f60df2bafe34e) ) // 2bpp
	ROM_LOAD16_BYTE( "graphics_rom_1", 0x200000, 0x080000, CRC(c8caa752) SHA1(cecc7a18a0bb59554316ed24ed0dc01d58d6ae53) ) // 1bpp, 27c040
ROM_END

} // anonymous namespace


/***************************************************************************


                                Game Drivers


***************************************************************************/

// ESD 11-09-98
GAME( 1999, multchmp25,  multchmp, esd16,     multchmp, esd16_state, empty_init, ROT0, "ESD",         "Multi Champ (World, ver. 2.5)",              MACHINE_SUPPORTS_SAVE )
GAME( 1998, multchmpk,   multchmp, esd16,     multchmp, esd16_state, empty_init, ROT0, "ESD",         "Multi Champ (Korea, older)",                 MACHINE_SUPPORTS_SAVE )
GAME( 1998, multchmpa,   multchmp, esd16,     multchmp, esd16_state, empty_init, ROT0, "ESD",         "Multi Champ (World, older)",                 MACHINE_SUPPORTS_SAVE )

GAME( 2001, jumppop,     0,        jumppop,   jumppop,  esd16_state, empty_init, ROT0, "ESD",         "Jumping Pop (set 1)",                        MACHINE_SUPPORTS_SAVE )
GAME( 2001, jumppope,    jumppop,  jumppop,   jumppop,  esd16_state, empty_init, ROT0, "Emag Soft",   "Jumping Pop (set 2)",                        MACHINE_SUPPORTS_SAVE )

// ESD 05-17-99
GAME( 1999, multchmp,    0,        esd16,     multchmp, esd16_state, empty_init, ROT0, "ESD",         "Multi Champ (World, ver. 9.6)",              MACHINE_SUPPORTS_SAVE )

// ESD 05-28-99
GAME( 1999, hedpanico,   hedpanic, hedpanio,  hedpanic, esd16_state, empty_init, ROT0, "ESD",         "Head Panic (ver. 0615, 15/06/1999)",         MACHINE_SUPPORTS_SAVE )

// ESD 06-10-1999
GAME( 1999, hedpanica,   hedpanic, hedpanic,  hedpanic, esd16_state, empty_init, ROT0, "ESD",         "Head Panic (ver. 0702, 02/07/1999)",         MACHINE_SUPPORTS_SAVE )

// ESD 08-26-1999
GAME( 2000, mchampdx,    0,        mchampdx,  hedpanic, esd16_state, empty_init, ROT0, "ESD",         "Multi Champ Deluxe (ver. 0106, 06/01/2000)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, mchampdxa,   mchampdx, mchampdx,  hedpanic, esd16_state, empty_init, ROT0, "ESD",         "Multi Champ Deluxe (ver. 1126, 26/11/1999)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, mchampdxb,   mchampdx, mchampdx,  hedpanic, esd16_state, empty_init, ROT0, "ESD",         "Multi Champ Deluxe (ver. 1114, 14/11/1999)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, hedpanic830, hedpanic, hedpanic,  hedpanic, esd16_state, empty_init, ROT0, "ESD",         "Head Panic (ver. 8.30)",                     MACHINE_SUPPORTS_SAVE )
GAME( 2000, hedpanic,    0,        hedpanic,  hedpanic, esd16_state, empty_init, ROT0, "ESD",         "Head Panic (ver. 0117, 17/01/2000)",         MACHINE_SUPPORTS_SAVE )
GAME( 2000, hedpanicf,   hedpanic, hedpanic,  hedpanic, esd16_state, empty_init, ROT0, "ESD / Fuuki", "Head Panic (ver. 0315, 15/03/2000)",         MACHINE_SUPPORTS_SAVE )

// ESD - This PCB looks identical to the ESD 08-26-1999 PCB
GAME( 2000, deluxe5,     0,        tangtang,  hedpanic, esd16_state, empty_init, ROT0, "ESD",         "Deluxe 5 (ver. 0107, 07/01/2000, set 1)",    MACHINE_SUPPORTS_SAVE ) // all 4 sets report the same version number?
GAME( 2000, deluxe5a,    deluxe5,  tangtang,  hedpanic, esd16_state, empty_init, ROT0, "ESD",         "Deluxe 5 (ver. 0107, 07/01/2000, set 2)",    MACHINE_SUPPORTS_SAVE )
GAME( 2000, deluxe5b,    deluxe5,  tangtang,  hedpanic, esd16_state, empty_init, ROT0, "ESD",         "Deluxe 5 (ver. 0107, 07/01/2000, set 3)",    MACHINE_SUPPORTS_SAVE )
GAME( 2000, deluxe4u,    deluxe5,  tangtang,  hedpanic, esd16_state, empty_init, ROT0, "ESD",         "Deluxe 4 U (ver. 0107, 07/01/2000)",         MACHINE_SUPPORTS_SAVE )

GAME( 2000, tangtang,    0,        tangtang,  hedpanic, esd16_state, empty_init, ROT0, "ESD",         "Tang Tang (ver. 0526, 26/05/2000)",          MACHINE_SUPPORTS_SAVE )
GAME( 2001, swatpolc,    0,        hedpanic,  swatpolc, esd16_state, empty_init, ROT0, "ESD",         "SWAT Police",                                MACHINE_SUPPORTS_SAVE )

// Z Soft PCB, uses PIC instead of Z80
GAME( 2002, fantstry,    0,        fantstry,  fantstry, esd16_state, empty_init, ROT0, "Z Soft",      "Fantasy Story (set 1)",                      MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE ) // playable, just no sound
GAME( 2002, fantstrya,   fantstry, fantstrya, fantstry, esd16_state, empty_init, ROT0, "Z Soft",      "Fantasy Story (set 2)",                      MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE ) // same
