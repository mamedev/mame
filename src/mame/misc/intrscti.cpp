// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

Intersecti

No names on the pcb and the only ascii text is in the sound rom "Intersecti". Has a daughterboard with "Serai-Padova-1" on it

Having disassembled the 1911 code roms, I'd guess that there is a rom in the epoxy block.

I've not had a chance to wire up the board yet, but it might be possible to write a trojan to display the rom(s) at $8000


*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "emupal.h"
#include "screen.h"


namespace {

class intrscti_state : public driver_device
{
public:
	intrscti_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_subcpu(*this,"subcpu"),
		m_vram(*this, "vram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void intrscti(machine_config &config);

	void init_intrscti();

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_shared_ptr<uint8_t> m_vram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	virtual void video_start() override ATTR_COLD;
	uint32_t screen_update_intrscti(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void intrscti_io_map(address_map &map) ATTR_COLD;
	void intrscti_map(address_map &map) ATTR_COLD;
	void intrscti_sub_io_map(address_map &map) ATTR_COLD;
	void intrscti_sub_map(address_map &map) ATTR_COLD;
};

void intrscti_state::video_start()
{
}

uint32_t intrscti_state::screen_update_intrscti(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int y,x;
	int count;

	bitmap.fill(m_palette->black_pen(), cliprect);

	count = 0;
	for (y=0;y<32;y++)
	{
		for (x=0;x<32;x++)
		{
			int dat;
			dat = m_vram[count];
			m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,dat/*+0x100*/,0,0,0,x*8,y*8,0);
			count++;
		}
	}

	count = 0x400;
	for (y=0;y<32;y++)
	{
		for (x=0;x<32;x++)
		{
			int dat;
			dat = m_vram[count];
			m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,dat+0x100,0,0,0,x*8,y*8,0);
			count++;
		}
	}

	return 0;
}


void intrscti_state::intrscti_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x6000, 0x67ff).ram();
	map(0x7000, 0x77ff).ram().share("vram");
	map(0x8000, 0x8fff).rom();
}

void intrscti_state::intrscti_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("IN0");
	map(0x01, 0x01).portr("IN1");
}


void intrscti_state::intrscti_sub_map(address_map &map)
{
	map(0x0000, 0x07ff).rom();
	map(0x2000, 0x23ff).ram();
//  map(0x0000, 0xffff).nopw();
}

void intrscti_state::intrscti_sub_io_map(address_map &map)
{
	map.global_mask(0xff);
//  map(0x00, 0xff).noprw();
}


static INPUT_PORTS_START( intrscti )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x01, "DSWA" )
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
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "DSWA" )
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
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3),RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( gfx_intrscti )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 16 )
GFXDECODE_END


void intrscti_state::intrscti(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 4000000);        /* ? MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &intrscti_state::intrscti_map);
	m_maincpu->set_addrmap(AS_IO, &intrscti_state::intrscti_io_map);
	m_maincpu->set_vblank_int("screen", FUNC(intrscti_state::irq0_line_hold));

	Z80(config, m_subcpu, 4000000);         /* ? MHz */
	m_subcpu->set_addrmap(AS_PROGRAM, &intrscti_state::intrscti_sub_map);
	m_subcpu->set_addrmap(AS_IO, &intrscti_state::intrscti_sub_io_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(256, 256);
	screen.set_visarea(0, 256-1, 0, 256-1);
	screen.set_screen_update(FUNC(intrscti_state::screen_update_intrscti));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_intrscti);
	PALETTE(config, m_palette).set_entries(0x100);
}


ROM_START( intrscti )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1911_1.7g", 0x0000, 0x1000, CRC(8940e6ee) SHA1(50da11a6fab8f31c72c08dbf374268fff18a74e3) )
	ROM_LOAD( "1911_2.8g", 0x1000, 0x1000, CRC(a461031e) SHA1(338c8cd79b98c666edd204150dea65ce4b9ec288) )
	ROM_LOAD( "epoxy_block", 0x8000,0x1000, NO_DUMP )

	ROM_REGION( 0x10000, "subcpu", 0 )
	ROM_LOAD( "ok.13b", 0x00000, 0x800, CRC(cbfa3eba) SHA1(b5a81a4535e7883a3ff8fb4021ddd7dbfaf3c7ae) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_LOAD( "r.5c", 0x0000, 0x1000, CRC(092cc8f2) SHA1(f0c717128e0ac6adc032616a8cafaec88aa0fb90) )
	ROM_LOAD( "g.5b", 0x1000, 0x1000, CRC(2d7cf465) SHA1(70fcb5818f2dfe499bb52501403449660837557d) )
	ROM_LOAD( "b.5a", 0x2000, 0x1000, CRC(8951fb7e) SHA1(c423bf0536e3a09453814172e31b47f9c3c3324c) )
ROM_END

void intrscti_state::init_intrscti()
{
	uint8_t *cpu = memregion( "maincpu" )->base();
	for (int i = 0; i < 0x1000; i++)
		cpu[i+0x8000]=0xc9; // ret

	/*
	0x8208 -> string copy (hl = pointer to videoram, de = pointer to epoxy block ROM)

	*/

	/* one of the protection sub-routines does this */
	for (int i = 0; i < 0x400; i++)
	{
		m_vram[i+0x000] = 0x0e;
		m_vram[i+0x400] = 0xff;
	}
}

} // anonymous namespace


GAME( 19??, intrscti, 0, intrscti, intrscti, intrscti_state, init_intrscti, ROT0, "<unknown>", "Intersecti", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_NO_SOUND )
