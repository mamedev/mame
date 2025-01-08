// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

Egg Hunt

hardware seems close to mitchell.cpp
--
palette format is different
has a sound cpu
ports shuffled around a bit



the following string is at the start of the roms

INVI IMAGE COPYWRITE 1995 OCT SEOUL IN KOREA
TEL 02-569-5830
PROGRMED BY JANG-K.Y
GRAPHIC DESIGN  KIM-Y.H & LEE-H.M

(and in the sound program)
SOUND DIRECTOR  LEE-S.O

the screen says VH-K October 1995, are VH-K another company involved?

---------------------

Egghunt by Invi Image

PCB marked "Invi Image Co. 1995 IZ80B-1"
The pcb has poor quality and resembles a bootleg.

2x Z80
1x AD65 (oki 6295 probably)
1x OSc 12mhz
1x OSC 30mhz
1x FPGA
2x Dipswitch

Note: rom 3 has the 16th pin overlapped and soldered to 15th....it seems it's a manufacturer choice since pin 16 on the socket is not connected with any trace on the pcb
I dumped it with this configuration. In case I'll redump it desoldering pin 16 from 15

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/okim6295.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class egghunt_state : public driver_device
{
public:
	egghunt_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_audiocpu(*this, "audiocpu"),
		m_atram(*this, "atram"),
		m_maincpu(*this, "maincpu"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void egghunt(machine_config &config);

private:
	/* video-related */
	tilemap_t   *m_bg_tilemap = nullptr;
	uint8_t     m_vidram_bank = 0;

	/* misc */
	uint8_t     m_okibanking = 0;
	uint8_t     m_gfx_banking = 0;

	/* devices */
	required_device<cpu_device> m_audiocpu;

	/* memory */
	required_shared_ptr<uint8_t> m_atram;
	uint8_t     m_bgram[0x1000]{};
	uint8_t     m_spram[0x1000]{};
	uint8_t egghunt_bgram_r(offs_t offset);
	void egghunt_bgram_w(offs_t offset, uint8_t data);
	void egghunt_atram_w(offs_t offset, uint8_t data);
	void egghunt_gfx_banking_w(uint8_t data);
	void egghunt_vidram_bank_w(uint8_t data);
	void egghunt_soundlatch_w(uint8_t data);
	uint8_t egghunt_okibanking_r();
	void egghunt_okibanking_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	uint32_t screen_update_egghunt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap,const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	void egghunt_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


void egghunt_state::draw_sprites( bitmap_ind16 &bitmap,const rectangle &cliprect )
{
	int flipscreen = 0;
	int offs, sx, sy;

	for (offs = 0x1000 - 0x40; offs >= 0; offs -= 0x20)
	{
		int code = m_spram[offs];
		int attr = m_spram[offs + 1];
		int color = attr & 0x0f;
		sx = m_spram[offs + 3] + ((attr & 0x10) << 4);
		sy = ((m_spram[offs + 2] + 8) & 0xff) - 8;
		code += (attr & 0xe0) << 3;

		if (attr & 0xe0)
		{
			switch(m_gfx_banking & 0x30)
			{
	//          case 0x00:
	//          case 0x10: code += 0; break;
				case 0x20: code += 0x400; break;
				case 0x30: code += 0x800; break;
			}
		}

		if (flipscreen)
		{
			sx = 496 - sx;
			sy = 240 - sy;
		}
		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
					code,
					color,
					flipscreen,flipscreen,
					sx,sy,15);
	}
}

TILE_GET_INFO_MEMBER(egghunt_state::get_bg_tile_info)
{
	int code = ((m_bgram[tile_index * 2 + 1] << 8) | m_bgram[tile_index * 2]) & 0x3fff;
	int colour = m_atram[tile_index] & 0x3f;

	if(code & 0x2000)
	{
		if((m_gfx_banking & 3) == 2)
			code += 0x2000;
		else if((m_gfx_banking & 3) == 3)
			code += 0x4000;
//      else if((m_gfx_banking & 3) == 1)
//          code += 0;
	}

	tileinfo.set(0, code, colour, 0);
}

uint8_t egghunt_state::egghunt_bgram_r(offs_t offset)
{
	if (m_vidram_bank)
	{
		return m_spram[offset];
	}
	else
	{
		return m_bgram[offset];
	}
}

void egghunt_state::egghunt_bgram_w(offs_t offset, uint8_t data)
{
	if (m_vidram_bank)
	{
		m_spram[offset] = data;
	}
	else
	{
		m_bgram[offset] = data;
		m_bg_tilemap->mark_tile_dirty(offset / 2);
	}
}

void egghunt_state::egghunt_atram_w(offs_t offset, uint8_t data)
{
	m_atram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


void egghunt_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(egghunt_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	save_item(NAME(m_bgram));
	save_item(NAME(m_spram));
}

uint32_t egghunt_state::screen_update_egghunt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}

void egghunt_state::egghunt_gfx_banking_w(uint8_t data)
{
	// data & 0x03 is used for tile banking
	// data & 0x30 is used for sprites banking
	m_gfx_banking = data & 0x33;

	m_bg_tilemap->mark_all_dirty();
}

void egghunt_state::egghunt_vidram_bank_w(uint8_t data)
{
	m_vidram_bank = data & 1;
}

void egghunt_state::egghunt_soundlatch_w(uint8_t data)
{
	m_soundlatch->write(data);
	m_audiocpu->set_input_line(0, HOLD_LINE);
}

uint8_t egghunt_state::egghunt_okibanking_r()
{
	return m_okibanking;
}

void egghunt_state::egghunt_okibanking_w(uint8_t data)
{
	m_okibanking = data;
	m_oki->set_rom_bank((data >> 4) & 1);
}

void egghunt_state::egghunt_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xc000, 0xc7ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0xc800, 0xcfff).ram().w(FUNC(egghunt_state::egghunt_atram_w)).share("atram");
	map(0xd000, 0xdfff).rw(FUNC(egghunt_state::egghunt_bgram_r), FUNC(egghunt_state::egghunt_bgram_w));
	map(0xe000, 0xffff).ram();
}


void egghunt_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("DSW1").w(FUNC(egghunt_state::egghunt_vidram_bank_w));
	map(0x01, 0x01).portr("SYSTEM").w(FUNC(egghunt_state::egghunt_gfx_banking_w));
	map(0x02, 0x02).portr("P1");
	map(0x03, 0x03).portr("P2").w(FUNC(egghunt_state::egghunt_soundlatch_w));
	map(0x04, 0x04).portr("DSW2");
	map(0x06, 0x06).portr("UNK").nopw();
	map(0x07, 0x07).nopw();
}

void egghunt_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xe000, 0xe000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0xe001, 0xe001).rw(FUNC(egghunt_state::egghunt_okibanking_r), FUNC(egghunt_state::egghunt_okibanking_w));
	map(0xe004, 0xe004).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xf000, 0xffff).ram();
}


static INPUT_PORTS_START( egghunt )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Debug Mode" ) // Run all the animations
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, "Credits per Player" )
	PORT_DIPSETTING(    0x60, "1" )
//  PORT_DIPSETTING(    0x20, "2" ) /* One of these maybe 2 to start 1 to continue */
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )


	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Censor Pictures" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )

	PORT_START("UNK")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END



static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(2,4),RGN_FRAC(3,4),RGN_FRAC(0,4),RGN_FRAC(1,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout tiles16x16_layout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(2,4),RGN_FRAC(3,4),RGN_FRAC(0,4),RGN_FRAC(1,4) },
	{  4, 5, 6, 7,0, 1, 2, 3, 16*8+4,16*8+5,16*8+6,16*8+7,16*8+0,16*8+1,16*8+2,16*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8 },
	16*16
};

static GFXDECODE_START( gfx_egghunt )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 64 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles16x16_layout, 0, 64 )
GFXDECODE_END


void egghunt_state::machine_start()
{
	save_item(NAME(m_gfx_banking));
	save_item(NAME(m_okibanking));
	save_item(NAME(m_vidram_bank));
}

void egghunt_state::machine_reset()
{
	m_gfx_banking = 0;
	m_okibanking = 0;
	m_vidram_bank = 0;
}

void egghunt_state::egghunt(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 12000000/2);      /* 6 MHz ?*/
	m_maincpu->set_addrmap(AS_PROGRAM, &egghunt_state::egghunt_map);
	m_maincpu->set_addrmap(AS_IO, &egghunt_state::io_map);
	m_maincpu->set_vblank_int("screen", FUNC(egghunt_state::irq0_line_hold)); // or 2 like mitchell.cpp?

	Z80(config, m_audiocpu, 12000000/2);         /* 6 MHz ?*/
	m_audiocpu->set_addrmap(AS_PROGRAM, &egghunt_state::sound_map);


	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(8*8, 56*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(egghunt_state::screen_update_egghunt));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_egghunt);

	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 0x400);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	OKIM6295(config, m_oki, 1056000, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0); // clock frequency & pin 7 not verified
}

} // anonymous namespace


ROM_START( egghunt )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "prg.bin", 0x00000, 0x20000, CRC(eb647145) SHA1(792951b76b5fac01e72ae34a2fe2108e373c5b62) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "rom2.bin", 0x00000, 0x10000, CRC(88a71bc3) SHA1(cf5acccfda9fda0d55af91a415a54391d0d0b7a2) )

	ROM_REGION( 0x100000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "rom3.bin", 0x00000, 0x40000, CRC(9d51ac49) SHA1(b0785d746fb2872a04386016ffdee80e6174dfc0) )
	ROM_LOAD( "rom4.bin", 0x40000, 0x40000, CRC(41c63041) SHA1(24e9a21d448c144db2356329cf87dc99598c96dc) )
	ROM_LOAD( "rom5.bin", 0x80000, 0x40000, CRC(6f96cb97) SHA1(7dde7d2aec6b5f9929b98d06c07bb07bf7bd59dd) )
	ROM_LOAD( "rom6.bin", 0xc0000, 0x40000, CRC(b5a41d4b) SHA1(1b4cf9c944e3eb7dc2d26d8a73bf5efb7b53253a) )

	ROM_REGION( 0x80000, "gfx2", ROMREGION_INVERT )
	ROM_LOAD( "rom7.bin", 0x00000, 0x20000, CRC(1b43fb57) SHA1(f06e186bf514f2ad655df23636eab72e6fafd815) )
	ROM_LOAD( "rom8.bin", 0x20000, 0x20000, CRC(f8122d0d) SHA1(78551c689b9e4eeed5e1ae97d8c7a907a388a9ff) )
	ROM_LOAD( "rom9.bin", 0x40000, 0x20000, CRC(dbfa0ffe) SHA1(2aa759c0bd3945473a6d8fa48226ce6c6c94d740) )
	ROM_LOAD( "rom10.bin",0x60000, 0x20000, CRC(14f5fc74) SHA1(769bccf9c1b42c35c3aee3866ed015de7c83b710) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "rom1.bin", 0x00000, 0x80000, CRC(f03589bc) SHA1(4d9c8422ac3c4c3ecba3bcf0ed47b8c7d5903f8c) )
ROM_END

GAME( 1995, egghunt, 0, egghunt, egghunt, egghunt_state, empty_init, ROT0, "Invi Image", "Egg Hunt", MACHINE_SUPPORTS_SAVE )
