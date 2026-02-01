// license:GPL-2.0+
// copyright-holders:Jarek Burczynski
/*******************************************************************************

Ace by Allied Leisure

Driver by Jarek Burczynski
2002.09.19

It actually has two 2-way joysticks per player, MAME combines them into one.
The Bullet Range and Game Time settings are not DIP switches, PCBs were seen
with thumbwheels (the PCB diagram below), or jumpers.

TODO:
- discrete sound is unemulated
- What are the unknown inputs in IN2 for? Extra coin sensors? The cabinet does
  not have a start button.
- Is the border correct? There's one photo online with gaps in the border. Does
  it mean planes can fly through at one point, or was it just a PCB malfunction?
  The flyer shows alternating white/black border objects, but PCB references don't

================================================================================

Allied Leisure 1976
"MAJOR MFG. INC. SUNNYVALE, CA" in PCB etch

18MHz
                                                          5MHz

8080


2101
2101


A5               3106          3106         3106
A4
A3                                                      3622.K4
A2                                   2101
A1                   2101            2101

                                                         [ RANGE ] [ TIME ]
                                                        (two 0-9 thumbwheel switches)

5x2101 - SRAM 256x4
3x3106 - SRAM 256x1
1x3622 - ROM 512x4

*******************************************************************************/

#include "emu.h"

#include "cpu/i8085/i8085.h"

#include "emupal.h"
#include "screen.h"

#include "ace.lh"


namespace {

class ace_state : public driver_device
{
public:
	ace_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_scoreram(*this, "scoreram"),
		m_characterram(*this, "characterram"),
		m_objpos(*this, "objpos"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_in(*this, "IN%u", 0U)
	{ }

	void ace(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint8_t> m_scoreram;
	required_shared_ptr<uint8_t> m_characterram;
	required_shared_ptr<uint8_t> m_objpos;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_ioport_array<3> m_in;

	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void ace_characterram_w(offs_t offset, uint8_t data);
	void ace_scoreram_w(offs_t offset, uint8_t data);

	template<int N> uint8_t input_r(offs_t offset) { return BIT(m_in[N]->read(), offset); }

	void main_map(address_map &map) ATTR_COLD;
};


/*************************************
 *
 *  Video hardware
 *
 *************************************/

void ace_state::video_start()
{
	m_gfxdecode->gfx(1)->set_source(m_characterram);
	m_gfxdecode->gfx(2)->set_source(m_characterram);
	m_gfxdecode->gfx(3)->set_source(m_characterram);
	m_gfxdecode->gfx(4)->set_source(m_scoreram);
}

void ace_state::palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(0x80,0x80,0x80));
	palette.set_pen_color(1, rgb_t(0xff,0xff,0xff));
	palette.set_pen_color(2, rgb_t(0x80,0x80,0x80));
	palette.set_pen_color(3, rgb_t(0x00,0x00,0x00));
}

uint32_t ace_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	// border
	for (int x = 0; x < 16; x++)
		m_gfxdecode->gfx(0)->transpen(bitmap, cliprect, 0, 0, 0, 0, x * 16, 0, 0);

	for (int y = 1; y < 15; y++)
	{
		m_gfxdecode->gfx(0)->transpen(bitmap, cliprect, 0, 0, 0, 0, 0, y * 16, 0);
		m_gfxdecode->gfx(0)->transpen(bitmap, cliprect, 0, 0, 0, 0, 240, y * 16, 0);
	}

	// score panel (2*size)
	for (int x = 0; x < 8; x++)
		m_gfxdecode->gfx(4)->zoom_transpen(bitmap, cliprect, x, 0, 0, 0, x * 32, 256 - 32, 0x20000, 0x20000, 0);

	// objects
	m_gfxdecode->gfx(1)->transpen(bitmap, cliprect, 0, 0, 0, 0, m_objpos[0] - 16, m_objpos[1], 0); // p2
	m_gfxdecode->gfx(2)->transpen(bitmap, cliprect, 0, 1, 0, 0, m_objpos[2] - 16, m_objpos[3], 0); // p1
	m_gfxdecode->gfx(3)->transpen(bitmap, cliprect, 0, 0, 0, 0, m_objpos[4] - 16, m_objpos[5], 0); // bullet

	return 0;
}


void ace_state::ace_characterram_w(offs_t offset, uint8_t data)
{
	if (m_characterram[offset] != data)
	{
		if (data & ~0x07)
			logerror("write to %04x data = %02x\n", 0x8000 + offset, data);

		m_characterram[offset] = data;
		m_gfxdecode->gfx(1)->mark_dirty(0);
		m_gfxdecode->gfx(2)->mark_dirty(0);
		m_gfxdecode->gfx(3)->mark_dirty(0);
	}
}

void ace_state::ace_scoreram_w(offs_t offset, uint8_t data)
{
	m_scoreram[offset] = data;
	m_gfxdecode->gfx(4)->mark_dirty(offset / 32);
}



/*************************************
 *
 *  Address map
 *
 *************************************/

void ace_state::main_map(address_map &map)
{
	map(0x0000, 0x09ff).rom();

	map(0x2000, 0x20ff).ram().w(FUNC(ace_state::ace_scoreram_w)).share(m_scoreram); // 2x2101
	map(0x8000, 0x80ff).ram().w(FUNC(ace_state::ace_characterram_w)).share(m_characterram); // 3x3101 (3 bits)
	map(0x8300, 0x83ff).ram(); // 2x2101

	map(0xc000, 0xc005).writeonly().share(m_objpos);

	map(0xc008, 0xc00c).r(FUNC(ace_state::input_r<0>));
	map(0xc00d, 0xc011).r(FUNC(ace_state::input_r<1>));
	map(0xc012, 0xc017).r(FUNC(ace_state::input_r<2>));
	map(0xc013, 0xc013).unmapr();

	map(0xc018, 0xc018).portr("RANGE");
	map(0xc019, 0xc019).portr("TIME");

	map(0xc020, 0xc026).unmapr(); // sound triggers
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( ace )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_2WAY PORT_PLAYER(2) PORT_NAME("P2 Thrust")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_2WAY PORT_PLAYER(2) PORT_NAME("P2 Slowdown")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Fire")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_2WAY PORT_PLAYER(1) PORT_NAME("P1 Thrust")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_2WAY PORT_PLAYER(1) PORT_NAME("P1 Slowdown")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Fire")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // must be 0 after insert coin
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED ) // never read
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) // must be 1 after insert coin
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // read when plane is destroyed

	PORT_START("RANGE")
	PORT_CONFNAME( 0x0f, 0x05, "Bullet Range" )
	PORT_CONFSETTING(    0x00, "0" )
	PORT_CONFSETTING(    0x01, "1" )
	PORT_CONFSETTING(    0x02, "2" )
	PORT_CONFSETTING(    0x03, "3" )
	PORT_CONFSETTING(    0x04, "4" )
	PORT_CONFSETTING(    0x05, "5" )
	PORT_CONFSETTING(    0x06, "6" )
	PORT_CONFSETTING(    0x07, "7" )
	PORT_CONFSETTING(    0x08, "8" )
	PORT_CONFSETTING(    0x09, "9" )

	PORT_START("TIME")
	PORT_CONFNAME( 0x0f, 0x05, "Game Time" )
	PORT_CONFSETTING(    0x00, "0" )
	PORT_CONFSETTING(    0x01, "1" )
	PORT_CONFSETTING(    0x02, "2" )
	PORT_CONFSETTING(    0x03, "3" )
	PORT_CONFSETTING(    0x04, "4" )
	PORT_CONFSETTING(    0x05, "5" )
	PORT_CONFSETTING(    0x06, "6" )
	PORT_CONFSETTING(    0x07, "7" )
	PORT_CONFSETTING(    0x08, "8" )
	PORT_CONFSETTING(    0x09, "9" )
INPUT_PORTS_END



/*************************************
 *
 *  GFX layouts
 *
 *************************************/

static const gfx_layout charlayoutrom =
{
	16,16,  /* 16*16 chars */
	8,  /* 8 characters */
	1,      /* 1 bit per pixel */
	{ 4 },  /* character rom is 512x4 bits (3622 type)*/
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3, 16+0, 16+1, 16+2, 16+3, 24+0, 24+1, 24+2, 24+3 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32, 8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	64*8    /* every char takes 64 consecutive bytes */
};

static const gfx_layout charlayout0 =
{
	16,16,  /* 16*16 chars */
	1,  /* 1 characters */
	1,      /* 1 bit per pixel */
	{ 7 },  /* bit 0 in character ram */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	{ 0*128, 1*128, 2*128, 3*128, 4*128, 5*128, 6*128, 7*128, 8*128, 9*128, 10*128, 11*128, 12*128, 13*128, 14*128, 15*128 },
	256*8   /* every char takes 256 consecutive bytes */
};

static const gfx_layout charlayout1 =
{
	16,16,  /* 16*16 chars */
	1,  /* 1 characters */
	1,      /* 1 bit per pixel */
	{ 6 },  /* bit 1 in character ram */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	{ 0*128, 1*128, 2*128, 3*128, 4*128, 5*128, 6*128, 7*128, 8*128, 9*128, 10*128, 11*128, 12*128, 13*128, 14*128, 15*128 },
	256*8   /* every char takes 256 consecutive bytes */
};

static const gfx_layout charlayout2 =
{
	16,16,  /* 16*16 chars */
	1,  /* 1 characters */
	1,      /* 1 bit per pixel */
	{ 5 },  /* bit 2 in character ram */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	{ 0*128, 1*128, 2*128, 3*128, 4*128, 5*128, 6*128, 7*128, 8*128, 9*128, 10*128, 11*128, 12*128, 13*128, 14*128, 15*128 },
	256*8   /* every char takes 256 consecutive bytes */
};

static const gfx_layout scorelayout =
{
	16,16,  /* 16*16 chars */
	8,  /* 8 characters */
	1,      /* 1 bit per pixel */
	{ 0 },  /*  */
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16, 8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	32*8    /* every char takes 32 consecutive bytes */
};

static GFXDECODE_START( gfx_ace )
	GFXDECODE_ENTRY( "gfx1", 0, charlayoutrom,  0, 2 )
	GFXDECODE_RAM( nullptr, 0x8000, charlayout0, 0, 2 )
	GFXDECODE_RAM( nullptr, 0x8000, charlayout1, 0, 2 )
	GFXDECODE_RAM( nullptr, 0x8000, charlayout2, 0, 2 )
	GFXDECODE_RAM( nullptr, 0x8000, scorelayout, 0, 2 )
GFXDECODE_END



/*************************************
 *
 *  Machine config
 *
 *************************************/

void ace_state::ace(machine_config &config)
{
	/* basic machine hardware */
	I8080(config, m_maincpu, 18_MHz_XTAL / 9); // 2 MHz?
	m_maincpu->set_addrmap(AS_PROGRAM, &ace_state::main_map);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 0*8, 32*8-1);
	screen.set_screen_update(FUNC(ace_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_ace);
	PALETTE(config, m_palette, FUNC(ace_state::palette), 4);

	// sound hardware
	// TODO
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( ace )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ace.a1", 0x0000, 0x0200, CRC(16811834) SHA1(5502812dd161908eea3fa8851d7e5c1e22b0f8ff) )
	ROM_LOAD( "ace.a2", 0x0200, 0x0200, CRC(f9eae80e) SHA1(8865b86c7b5d57c76312c16f8a614bf35ffaf532) )
	ROM_LOAD( "ace.a3", 0x0400, 0x0200, CRC(c5c63b8c) SHA1(2079dd12ff0c4aafec19aeb9baa70fc9b6788356) )
	ROM_LOAD( "ace.a4", 0x0600, 0x0200, CRC(ea4503aa) SHA1(fea610124b9f7ea18d29b4e4599253ba1ee067e1) )
	ROM_LOAD( "ace.a5", 0x0800, 0x0200, CRC(623c58e7) SHA1(a92418bc323a1ae76eae8e094e4d6ebd1e8da14e) )

	ROM_REGION( 0x0200, "gfx1", 0 )
	ROM_LOAD( "ace.k4", 0x0000, 0x0200, CRC(daa05ec6) SHA1(8b71ffb802293dc93f6b492ff128a704e676a5fd) )
ROM_END

} // anonymous namespace



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAMEL(1976, ace, 0, ace, ace, ace_state, empty_init, ROT0, "Allied Leisure", "Ace", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND, layout_ace )
