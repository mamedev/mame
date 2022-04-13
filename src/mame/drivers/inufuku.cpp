// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
/******************************************************************************

    Video Hardware for Video System Games.

    Quiz & Variety Sukusuku Inufuku (Japan)
    (c)1998 Video System Co.,Ltd.

    3 On 3 Dunk Madness (US, prototype?)
    (c)1996 Video System Co.,Ltd.

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 2003/08/09 -

    based on other Video System drivers

    Known games not dumped ran on this hardware:
    - 3 on 3 Hyper Slams '96 (Korea) https://youtu.be/3gGB4kYYgi0

******************************************************************************/
/******************************************************************************

Quiz & Variety Sukusuku Inufuku
(c)1998 Video System

VSBB-31-1

CPU  : MC68HC000P-16
Sound: TMPZ84C000AP-8 YM2610 YM3016
OSC  : 32.0000MHz 14.31818MHz

ROMs:
U107.BIN     - Sound Program (27C1001)

U146.BIN     - Main Programs (27C240)
U147.BIN     |
LHMN5L28.148 / (32M Mask)

Others:
93C46 (EEPROM)
UMAG1 (ALTERA MAX EPM7128ELC84-10 BG9625)
PLD00?? (ALTERA EPM7032LC44-15 BA9631)
002 (PALCE16V8-10PC)
003 (PALCE16V8-15PC)

Custom Chips:
VS920A
VS920E
VS9210
VS9108 (Fujitsu CG10103)
(blank pattern for VS9210 and VS9108)

VSBB31-ROM

ROMs:
LHMN5KU6.U53 - 32M SOP Mask ROMs
LHMN5KU8.U40 |
LHMN5KU7.U8  |
LHMN5KUB.U34 |
LHMN5KUA.U36 |
LHMN5KU9.U38 /

******************************************************************************/
/******************************************************************************

TODO:

- User must initialize NVRAM at first boot in test mode (factory settings).

- Sometimes, sounds are not played (especially SFX), but this is a bug of real machine.

- Sound Code 0x08 remains unknown.

- Priority of tests and sprites seems to be correct, but I may have mistaken.

******************************************************************************/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/eepromser.h"
#include "machine/gen_latch.h"
#include "sound/ymopn.h"
#include "video/bufsprite.h"
#include "video/vsystem_spr.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class inufuku_state : public driver_device
{
public:
	inufuku_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_bg_videoram(*this, "bg_videoram")
		, m_bg_rasterram(*this, "bg_rasterram")
		, m_tx_videoram(*this, "tx_videoram")
		, m_sprtileram(*this, "sprtileram")
		, m_audiobank(*this, "audiobank")
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_spr(*this, "vsystem_spr")
		, m_soundlatch(*this, "soundlatch")
		, m_sprattrram(*this, "sprattrram")
		{ }

	void inufuku(machine_config &config);
	void _3on3dunk(machine_config &config);

	DECLARE_READ_LINE_MEMBER(soundflag_r);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	// memory pointers
	required_shared_ptr<u16> m_bg_videoram;
	required_shared_ptr<u16> m_bg_rasterram;
	required_shared_ptr<u16> m_tx_videoram;
	required_shared_ptr<u16> m_sprtileram;

	required_memory_bank m_audiobank;

	// video-related
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_tx_tilemap;
	int       m_bg_scrollx = 0;
	int       m_bg_scrolly = 0;
	int       m_tx_scrollx = 0;
	int       m_tx_scrolly = 0;
	bool      m_bg_raster = false;
	u8        m_bg_palettebank = 0;
	u8        m_tx_palettebank = 0;
	u32       tile_callback( u32 code );

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<vsystem_spr_device> m_spr;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<buffered_spriteram16_device> m_sprattrram;

	void soundrombank_w(u8 data);
	void palettereg_w(offs_t offset, u16 data);
	void scrollreg_w(offs_t offset, u16 data);
	void bg_videoram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void tx_videoram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map);
	void sound_io_map(address_map &map);
	void sound_map(address_map &map);
};


/******************************************************************************

    Memory handlers

******************************************************************************/

void inufuku_state::palettereg_w(offs_t offset, u16 data)
{
	switch (offset)
	{
		case 0x02:
			m_bg_palettebank = (data & 0xf000) >> 12;
			m_bg_tilemap->mark_all_dirty();
			/*
			    if (data & ~0xf000)
			        logerror("%s: palettereg_w %02X: %04x\n", machine().describe_context(), offset << 1, data);
			*/
			break;
		case 0x03:
			m_tx_palettebank = (data & 0xf000) >> 12;
			m_tx_tilemap->mark_all_dirty();
			/*
			    if (data & ~0xf000)
			        logerror("%s: palettereg_w %02X: %04x\n", machine().describe_context(), offset << 1, data);
			*/
			break;
		default:
			//logerror("%s: palettereg_w %02X: %04x\n", machine().describe_context(), offset << 1, data);
			break;
	}
}


void inufuku_state::scrollreg_w(offs_t offset, u16 data)
{
	switch (offset)
	{
		case 0x00:  m_bg_scrollx = data + 1; break;
		case 0x01:  m_bg_scrolly = data + 0; break;
		case 0x02:  m_tx_scrollx = data - 3; break;
		case 0x03:  m_tx_scrolly = data + 1; break;
		case 0x04:
			m_bg_raster = BIT(~data, 9);
			/*
			    if (data & ~0x0200)
			        logerror("%s: palettereg_w %02X: %04x\n", machine().describe_context(), offset << 1, data);
			*/
			break;
		default:
			//logerror("%s: scrollreg_w %02X: %04x\n", machine().describe_context(), offset << 1, data);
			break;
	}
}


/******************************************************************************

    Tilemap callbacks

******************************************************************************/

TILE_GET_INFO_MEMBER(inufuku_state::get_bg_tile_info)
{
	tileinfo.set(0,
			m_bg_videoram[tile_index],
			m_bg_palettebank,
			0);
}


TILE_GET_INFO_MEMBER(inufuku_state::get_tx_tile_info)
{
	tileinfo.set(1,
			m_tx_videoram[tile_index],
			m_tx_palettebank,
			0);
}


void inufuku_state::bg_videoram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_bg_videoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset);
}


void inufuku_state::tx_videoram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_tx_videoram[offset]);
	m_tx_tilemap->mark_tile_dirty(offset);
}


u32 inufuku_state::tile_callback( u32 code )
{
	return ((m_sprtileram[code * 2] & 0x0007) << 16) + m_sprtileram[(code * 2) + 1];
}


/******************************************************************************

    Start the video hardware emulation

******************************************************************************/

void inufuku_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(inufuku_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(inufuku_state::get_tx_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);

	m_bg_tilemap->set_transparent_pen(255);
	m_tx_tilemap->set_transparent_pen(255);
}


/******************************************************************************

    Display refresh

******************************************************************************/

u32 inufuku_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);
	screen.priority().fill(0);

	if (m_bg_raster)
	{
		m_bg_tilemap->set_scroll_rows(512);
		for (int i = cliprect.min_y; i <= cliprect.max_y; i++)
			m_bg_tilemap->set_scrollx((m_bg_scrolly + i) & 0x1ff, m_bg_scrollx + m_bg_rasterram[i]);
	}
	else
	{
		m_bg_tilemap->set_scroll_rows(1);
		m_bg_tilemap->set_scrollx(0, m_bg_scrollx);
	}
	m_bg_tilemap->set_scrolly(0, m_bg_scrolly);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	m_tx_tilemap->set_scrollx(0, m_tx_scrollx);
	m_tx_tilemap->set_scrolly(0, m_tx_scrolly);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 4);

	m_spr->draw_sprites( m_sprattrram->buffer(), m_sprattrram->bytes(), screen, bitmap, cliprect );
	return 0;
}


/******************************************************************************

    Sound CPU interface

******************************************************************************/

void inufuku_state::soundrombank_w(u8 data)
{
	m_audiobank->set_entry(data & 0x03);
}


/******************************************************************************

    Input/Output port interface

******************************************************************************/

READ_LINE_MEMBER(inufuku_state::soundflag_r)
{
	return m_soundlatch->pending_r() ? 0 : 1;
}


/******************************************************************************

    Main CPU memory handlers

******************************************************************************/

void inufuku_state::main_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom().region("maincpu", 0x000000);                                  // main rom

//  map(0x100000, 0x100007).nopw();                                                             // ?

	map(0x180000, 0x180001).portr("P1");
	map(0x180002, 0x180003).portr("P2");
	map(0x180004, 0x180005).portr("SYSTEM");
	map(0x180006, 0x180007).portr("P4");
	map(0x180008, 0x180009).portr("EXTRA");
	map(0x18000a, 0x18000b).portr("P3");

	map(0x200000, 0x200001).portw("EEPROMOUT");
	map(0x280001, 0x280001).w(m_soundlatch, FUNC(generic_latch_8_device::write));               // sound command

	map(0x300000, 0x301fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette"); // palette ram
	map(0x380000, 0x3801ff).writeonly().share(m_bg_rasterram);                                  // bg raster ram
	map(0x400000, 0x401fff).ram().w(FUNC(inufuku_state::bg_videoram_w)).share(m_bg_videoram);   // bg ram
	map(0x402000, 0x403fff).ram().w(FUNC(inufuku_state::tx_videoram_w)).share(m_tx_videoram);   // text ram
	map(0x404000, 0x40ffff).ram(); // ?? mirror (3on3dunk)
	map(0x580000, 0x581fff).ram().share("sprattrram");                                          // sprite table + sprite attribute
	map(0x600000, 0x61ffff).ram().share(m_sprtileram);                                          // cell table

	map(0x780000, 0x780013).w(FUNC(inufuku_state::palettereg_w));                               // bg & text palettebank register
	map(0x7a0000, 0x7a0023).w(FUNC(inufuku_state::scrollreg_w));                                // bg & text scroll register
//  map(0x7e0000, 0x7e0001).nopw();                                                             // ?

	map(0x800000, 0xbfffff).rom().region("maincpu", 0x100000);                                  // data rom
	map(0xfd0000, 0xfdffff).ram();                                                              // work ram
}


/******************************************************************************

    Sound CPU memory handlers

******************************************************************************/

void inufuku_state::sound_map(address_map &map)
{
	map(0x0000, 0x77ff).rom();
	map(0x7800, 0x7fff).ram();
	map(0x8000, 0xffff).bankr(m_audiobank);
}


void inufuku_state::sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(inufuku_state::soundrombank_w));
	map(0x04, 0x04).rw(m_soundlatch, FUNC(generic_latch_8_device::read), FUNC(generic_latch_8_device::acknowledge_w));
	map(0x08, 0x0b).rw("ymsnd", FUNC(ym2610_device::read), FUNC(ym2610_device::write));
}


/******************************************************************************

    Port definitions

******************************************************************************/

static INPUT_PORTS_START( inufuku )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_SERVICE_NO_TOGGLE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(4)

	PORT_START("EXTRA")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START4 )
	PORT_DIPNAME( 0x0010, 0x0010, "3P/4P" )
	PORT_DIPSETTING(    0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(inufuku_state, soundflag_r)    // pending sound command
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN ) // 3on3dunk cares about something in here, possibly a vblank flag

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, di_write)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, clk_write)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, cs_write)

	PORT_START("P3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(3)
INPUT_PORTS_END


/******************************************************************************

    Graphics definitions

******************************************************************************/

static GFXDECODE_START( gfx_inufuku )
	GFXDECODE_ENTRY( "bgtile",  0, gfx_8x8x8_raw,          0, 4096/256 )  // bg
	GFXDECODE_ENTRY( "txtile",  0, gfx_8x8x8_raw,          0, 4096/256 )  // text
	GFXDECODE_ENTRY( "sprtile", 0, gfx_16x16x4_packed_msb, 0, 4096/16  )  // sprite
GFXDECODE_END


static GFXDECODE_START( gfx_3on3dunk )
	GFXDECODE_ENTRY( "bgtile",  0, gfx_8x8x8_raw,          0, 4096/256 )  // bg
	GFXDECODE_ENTRY( "txtile",  0, gfx_8x8x8_raw,          0, 4096/256 )  // text
	GFXDECODE_ENTRY( "sprtile", 0, gfx_16x16x4_packed_lsb, 0, 4096/16  )  // sprite
GFXDECODE_END


/******************************************************************************

    Machine driver

******************************************************************************/

void inufuku_state::machine_start()
{
	m_audiobank->configure_entries(0, 4, memregion("audiocpu")->base(), 0x8000);
	m_audiobank->set_entry(0);

	save_item(NAME(m_bg_scrollx));
	save_item(NAME(m_bg_scrolly));
	save_item(NAME(m_tx_scrollx));
	save_item(NAME(m_tx_scrolly));
	save_item(NAME(m_bg_raster));
	save_item(NAME(m_bg_palettebank));
	save_item(NAME(m_tx_palettebank));
}


void inufuku_state::machine_reset()
{
	m_bg_scrollx = 0;
	m_bg_scrolly = 0;
	m_tx_scrollx = 0;
	m_tx_scrolly = 0;
	m_bg_raster = false;
	m_bg_palettebank = 0;
	m_tx_palettebank = 0;
}


void inufuku_state::inufuku(machine_config &config)
{
	// basic machine hardware */
	M68000(config, m_maincpu, XTAL(32'000'000)/2); // 16.00 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &inufuku_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(inufuku_state::irq1_line_hold));

	Z80(config, m_audiocpu, XTAL(32'000'000)/4);       // 8.00 MHz
	m_audiocpu->set_addrmap(AS_PROGRAM, &inufuku_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &inufuku_state::sound_io_map); // IRQs are triggered by the YM2610

	EEPROM_93C46_16BIT(config, "eeprom");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2300));
	screen.set_size(2048, 256);
	screen.set_visarea(0, 319, 0, 223);
	screen.set_screen_update(FUNC(inufuku_state::screen_update));
	screen.screen_vblank().set(m_sprattrram, FUNC(buffered_spriteram16_device::vblank_copy_rising));
	screen.set_palette(m_palette);

	VSYSTEM_SPR(config, m_spr, 0);
	m_spr->set_offsets(0, 1); // reference videos confirm at least the +1 against tilemaps in 3on3dunk (the highscore header text and black box are meant to be 1 pixel misaligned, although there is currently a priority bug there too)
	m_spr->set_pdraw(true);
	m_spr->set_tile_indirect_cb(FUNC(inufuku_state::tile_callback));
	m_spr->set_gfx_region(2);
	m_spr->set_gfxdecode_tag(m_gfxdecode);

	BUFFERED_SPRITERAM16(config, m_sprattrram);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_inufuku);
	PALETTE(config, m_palette).set_format(palette_device::xGBR_555, 4096);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);
	m_soundlatch->set_separate_acknowledge(true);

	ym2610_device &ymsnd(YM2610(config, "ymsnd", XTAL(32'000'000)/4));
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "mono", 0.50);
	ymsnd.add_route(1, "mono", 0.75);
	ymsnd.add_route(2, "mono", 0.75);
}


void inufuku_state::_3on3dunk(machine_config &config)
{
	inufuku(config);
	m_gfxdecode->set_info(gfx_3on3dunk);
}


/******************************************************************************

    ROM definitions

******************************************************************************/

ROM_START( inufuku )
	ROM_REGION( 0x0500000, "maincpu", 0 )   // main cpu + data
	ROM_LOAD16_WORD_SWAP( "u147.bin",     0x0000000, 0x080000, CRC(ab72398c) SHA1(f5dc266ffa936ea6528b46a34113f5e2f8141d71) )
	ROM_LOAD16_WORD_SWAP( "u146.bin",     0x0080000, 0x080000, CRC(e05e9bd4) SHA1(af0fdf31c2bdf851bf15c9de725dcbbb58464d54) )
	ROM_LOAD16_WORD_SWAP( "lhmn5l28.148", 0x0100000, 0x400000, CRC(802d17e7) SHA1(43b26efea65fd051c094d19784cb977ced39a1a0) )

	ROM_REGION( 0x0020000, "audiocpu", 0 )  // sound cpu
	ROM_LOAD( "u107.bin", 0x0000000, 0x020000, CRC(1744ef90) SHA1(e019f4ca83e21aa25710cc0ca40ffe765c7486c9) )

	ROM_REGION( 0x0400000, "bgtile", 0 )  // bg
	ROM_LOAD( "lhmn5ku8.u40", 0x0000000, 0x400000, CRC(8cbca80a) SHA1(063e9be97f5a1f021f8326f2994b51f9af5e1eaf) )

	ROM_REGION( 0x0400000, "txtile", 0 )  // text
	ROM_LOAD( "lhmn5ku7.u8",  0x0000000, 0x400000, CRC(a6c0f07f) SHA1(971803d1933d8296767d8766ea9f04dcd6ab065c) )

	ROM_REGION( 0x0c00000, "sprtile", 0 )  // sprite
	ROM_LOAD( "lhmn5kub.u34", 0x0000000, 0x400000, CRC(7753a7b6) SHA1(a2e8747ce83ea5a57e2fe62f2452de355d7f48b6) )
	ROM_LOAD( "lhmn5kua.u36", 0x0400000, 0x400000, CRC(1ac4402a) SHA1(c15acc6fce4fe0b54e92d14c31a1bd78acf2c8fc) )
	ROM_LOAD( "lhmn5ku9.u38", 0x0800000, 0x400000, CRC(e4e9b1b6) SHA1(4d4ad85fbe6a442d4f8cafad748bcae4af6245b7) )

	ROM_REGION( 0x0400000, "ymsnd:adpcma", 0 ) // adpcm data
	ROM_LOAD( "lhmn5ku6.u53", 0x0000000, 0x400000, CRC(b320c5c9) SHA1(7c99da2d85597a3c008ed61a3aa5f47ad36186ec) )
ROM_END


ROM_START( 3on3dunk )
	ROM_REGION( 0x0500000, "maincpu", 0 )   // main cpu + data
	ROM_LOAD16_WORD_SWAP( "prog0_2_4_usa.u147", 0x0000000, 0x080000, CRC(957924ab) SHA1(6fe8ca711d11239310d58188e9d6d28cd27bc5af) )
	ROM_LOAD16_WORD_SWAP( "prog1_2_4_usa.u146", 0x0080000, 0x080000, CRC(2479e236) SHA1(729e6c85d34d6925c8d6557b138e2bed43e1de6a) )
	ROM_LOAD16_WORD_SWAP( "lh535l5y.u148",      0x0100000, 0x400000, CRC(aa33e02a) SHA1(86381ecf18fba9065cbc02112751c435bbf8b8b4) )

	ROM_REGION( 0x0020000, "audiocpu", 0 )  // sound cpu
	ROM_LOAD( "sound_prog_97_1_13.u107", 0x0000000, 0x020000, CRC(d9d42805) SHA1(ab5cb7c141d9c9ed5121ba4dbc1d0fa187bd9f68) )

	ROM_REGION( 0x0400000, "bgtile", 0 )  // bg
	ROM_LOAD( "lh525kwy.u40", 0x0000000, 0x400000, CRC(aaa426d1) SHA1(2f9a2981f336caf3188baec9a34f61452dee2203) )

	ROM_REGION( 0x0400000, "txtile", 0 )  // text
	ROM_LOAD( "lh537nn4.u8",  0x0000000, 0x200000, CRC(2b7be1d8) SHA1(aac274a8f4028db7429478601a1761e61ab4f9a2) )

	ROM_REGION( 0x2000000, "sprtile", 0 )  // sprite
	ROM_LOAD( "lh535kwz.u34", 0x0000000, 0x400000, CRC(7372ce78) SHA1(ed2a861986357fad7ef983750cd906c3d722b862) )
	ROM_LOAD( "lh535kv0.u36", 0x0400000, 0x400000, CRC(247e5741) SHA1(8d71d964791fb4b86e390bcdf7744f616d6357b1) )
	ROM_LOAD( "lh535kv2.u38", 0x0800000, 0x400000, CRC(76449b1e) SHA1(b63d50c6f0dc91dc94dbcdda9842598529c1c26e) )
	ROM_LOAD( "lh537nn5.u20", 0x0c00000, 0x200000, CRC(f457cd3b) SHA1(cc13f5dc44e4675c1074a365b10f34e684817d81) )
	//                        0x0e00000, 0x200000 empty
	ROM_LOAD( "lh536pnm.u32", 0x1000000, 0x400000, CRC(bc39e449) SHA1(5aea90b66ee03c70797ddc42dbcb064d83ce8cc7) )

	ROM_REGION( 0x0400000, "ymsnd:adpcma", 0 ) // ADPCM data
	ROM_LOAD( "lh5388r1.u53", 0x0000000, 0x100000, CRC(765d892f) SHA1(9b078c879d0437d1669bf4301fd52a768aa4d293) )

	ROM_REGION( 0x0400000, "ymsnd:adpcmb", 0 ) // speech
	ROM_LOAD( "lh536pkl.u51", 0x0000000, 0x300000, CRC(e4919abf) SHA1(d6af4b9c6ff62f92216c9927027d3b2376416bae) )
ROM_END

} // Anonymous namespace


/******************************************************************************

    Game drivers

******************************************************************************/

GAME( 1998, inufuku,  0, inufuku,   inufuku, inufuku_state, empty_init, ROT0, "Video System Co.", "Quiz & Variety Sukusuku Inufuku (Japan)",         MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1996, 3on3dunk, 0, _3on3dunk, inufuku, inufuku_state, empty_init, ROT0, "Video System Co.", "3 On 3 Dunk Madness (US, prototype? 1997/02/04)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS ) // tilemap priority is wrong in places (basketball before explosion in attract, highscores)
