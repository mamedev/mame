// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

Royal Gum

Unknown CPU (either Z80 or Z180) - or at least rgum.u47 looks z80 related
rgum.u5 is for a 6502?

Big Black Box in the middle of the PCB (for encryption, or containing roms?)

The ppi at 3000-3003 seems to be a dual port communication thing with the z80.

*/

#include "emu.h"
#include "cpu/m6502/m65c02.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "sound/ay8910.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


class rgum_state : public driver_device
{
public:
	rgum_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_vram(*this, "vram"),
		m_cram(*this, "cram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")  { }

	void rgum(machine_config &config);

	DECLARE_CUSTOM_INPUT_MEMBER(rgum_heartbeat_r);

private:
	required_shared_ptr<uint8_t> m_vram;
	required_shared_ptr<uint8_t> m_cram;
	uint8_t m_hbeat;
	virtual void video_start() override;
	uint32_t screen_update_royalgum(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	void rgum_map(address_map &map);
};


void rgum_state::video_start()
{
}

uint32_t rgum_state::screen_update_royalgum(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int x,y,count;
	gfx_element *gfx = m_gfxdecode->gfx(0);

	count = 0;

	for(y=0;y<32;y++)
	{
		for(x=0;x<66;x++)
		{
			int tile = m_vram[count] | ((m_cram[count] & 0xf) <<8);

			gfx->opaque(bitmap,cliprect,tile,0,0,0,x*8,y*8);

			count++;
		}
	}

	return 0;
}

void rgum_state::rgum_map(address_map &map)
{
	map(0x0000, 0x07ff).ram(); //not all of it?

	map(0x0800, 0x0800).w("crtc", FUNC(mc6845_device::address_w));
	map(0x0801, 0x0801).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));

	map(0x2000, 0x2000).w("aysnd", FUNC(ay8910_device::data_w));
	map(0x2002, 0x2002).rw("aysnd", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_w));

	map(0x2801, 0x2801).nopr(); //read but value discarded?
	map(0x2803, 0x2803).nopr();

	map(0x3000, 0x3003).rw("ppi8255", FUNC(i8255_device::read), FUNC(i8255_device::write));

	map(0x4000, 0x47ff).ram().share("vram");
	map(0x5000, 0x57ff).ram().share("cram");

	map(0x8000, 0xffff).rom();
}


CUSTOM_INPUT_MEMBER(rgum_state::rgum_heartbeat_r)
{
	m_hbeat ^= 1;

	return m_hbeat;
}


static INPUT_PORTS_START( rgum )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x01, "IN0" )
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
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, rgum_state,rgum_heartbeat_r, nullptr)
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "IN1" )
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

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, "IN2" )
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
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNKNOWN ) //communication port with the z80?

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "DSW1" )
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

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "DSW2" )
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
	{ RGN_FRAC(0,3),RGN_FRAC(1,3),RGN_FRAC(2,3) },
	{ 0, 1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( gfx_rgum )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 16 )
GFXDECODE_END


void rgum_state::rgum(machine_config &config)
{
	/* basic machine hardware */
	M65C02(config, m_maincpu, 24000000/16);      /* ? MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &rgum_state::rgum_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(256, 256);
	screen.set_visarea(0, 256-1, 0, 256-1);
	screen.set_screen_update(FUNC(rgum_state::screen_update_royalgum));

	mc6845_device &crtc(MC6845(config, "crtc", 24000000/16));   /* unknown clock & type, hand tuned to get ~50 fps (?) */
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
	crtc.out_vsync_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	i8255_device &ppi(I8255A(config, "ppi8255"));
	ppi.in_pa_callback().set_ioport("IN0");
	ppi.in_pb_callback().set_ioport("IN1");
	ppi.in_pc_callback().set_ioport("IN2");

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_rgum);
	PALETTE(config, m_palette).set_entries(0x100);

	SPEAKER(config, "mono").front_center();

	AY8910(config, "aysnd", 24000000/16).add_route(ALL_OUTPUTS, "mono", 0.50); /* guessed to use the same xtal as the crtc */
}




ROM_START( rgum )
	ROM_REGION( 0x20000, "z80cpu", 0 )
	ROM_LOAD( "rgum.u47", 0x00000, 0x20000, CRC(fe410eb9) SHA1(25180ba336269279f251be5483c210a581d27197) ) // encrypted.. 2nd half empty

	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rgum.u5", 0x00000, 0x10000, CRC(9d2d1681) SHA1(1c1da0d970ea2cf58f7961417ab6986cc667da5c) ) // plaintext in here, but firt half is empty

	ROM_REGION( 0x10000, "unk", 0 )
	ROM_LOAD( "rgum.u6", 0x00000, 0x2000, CRC(15a34117) SHA1(c7e0aef4007abfaaa533feb026148ba03230b79f) ) // near the data rom, mostly empty

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "rgum.u16", 0x00000, 0x8000, CRC(2a2c8d78) SHA1(2ce335b900dccbc34ad8ae7ae02ec7c75ffcd559) ) // first half empty
	ROM_CONTINUE(0x00000,0x8000)
	ROM_LOAD( "rgum.u17", 0x08000,  0x8000, CRC(fae4e41a) SHA1(421aac2b567040c3a56e01aa70880c94450eaf76) ) // first half empty
	ROM_CONTINUE(0x08000,0x8000)
	ROM_LOAD( "rgum.u18", 0x10000, 0x8000, CRC(79b17da7) SHA1(31e1845261b0152df56135c212e55c4048b7496f) ) // first half empty
	ROM_CONTINUE(0x10000,0x8000)
ROM_END


GAME( 199?, rgum, 0, rgum, rgum, rgum_state, empty_init, ROT0, "<unknown>", "Royal Gum (Italy)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
