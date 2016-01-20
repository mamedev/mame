// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  Atari Quiz Show
  released under their Kee Games label, 04/1976

  S2650 CPU, 512 bytes RAM, B&W tilemapped video. It uses a tape player to
  stream questions, totaling 1000, divided into 4 categories.

TODO:
- preserve tape and hook it up, the game is not playable without it
- is timing accurate?

***************************************************************************/

#include "emu.h"
#include "cpu/s2650/s2650.h"
#include "sound/dac.h"

#include "quizshow.lh"


#define MASTER_CLOCK    XTAL_12_096MHz
#define PIXEL_CLOCK     (MASTER_CLOCK/2)

#define HTOTAL          ((32+8+4+1) * 8)
#define HBEND           (0)
#define HBSTART         (256)

#define VTOTAL          (256+8+4)
#define VBEND           (0)
#define VBSTART         (240)


class quizshow_state : public driver_device
{
public:
	quizshow_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_dac(*this, "dac"),
		m_main_ram(*this, "main_ram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<dac_device> m_dac;
	required_shared_ptr<UINT8> m_main_ram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;

	tilemap_t *m_tilemap;
	UINT32 m_clocks;
	int m_blink_state;
	int m_category_enable;
	int m_tape_head_pos;

	DECLARE_WRITE8_MEMBER(quizshow_lamps1_w);
	DECLARE_WRITE8_MEMBER(quizshow_lamps2_w);
	DECLARE_WRITE8_MEMBER(quizshow_lamps3_w);
	DECLARE_WRITE8_MEMBER(quizshow_tape_control_w);
	DECLARE_WRITE8_MEMBER(quizshow_audio_w);
	DECLARE_WRITE8_MEMBER(quizshow_video_disable_w);
	DECLARE_READ8_MEMBER(quizshow_timing_r);
	DECLARE_READ8_MEMBER(quizshow_tape_signal_r);
	DECLARE_WRITE8_MEMBER(quizshow_main_ram_w);
	DECLARE_CUSTOM_INPUT_MEMBER(quizshow_tape_headpos_r);
	DECLARE_INPUT_CHANGED_MEMBER(quizshow_category_select);
	DECLARE_DRIVER_INIT(quizshow);
	TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(quizshow);
	UINT32 screen_update_quizshow(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(quizshow_clock_timer_cb);
};


/***************************************************************************

  Video

***************************************************************************/

PALETTE_INIT_MEMBER(quizshow_state, quizshow)
{
	palette.set_indirect_color(0, rgb_t::black);
	palette.set_indirect_color(1, rgb_t::white);

	// normal, blink/off, invert, blink+invert
	const int lut_pal[16] = {
		0, 0, 1, 0,
		0, 0, 0, 0,
		1, 0, 0, 0,
		1, 0, 1, 0
	};

	for (int i = 0; i < 16 ; i++)
		palette.set_pen_indirect(i, lut_pal[i]);
}

TILE_GET_INFO_MEMBER(quizshow_state::get_tile_info)
{
	UINT8 code = m_main_ram[tile_index];

	// d6: blink, d7: invert
	UINT8 color = (code & (m_blink_state | 0x80)) >> 6;

	SET_TILE_INFO_MEMBER(0, code & 0x3f, color, 0);
}

void quizshow_state::video_start()
{
	m_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(quizshow_state::get_tile_info),this), TILEMAP_SCAN_ROWS, 8, 16, 32, 16);
}

UINT32 quizshow_state::screen_update_quizshow(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
	return 0;
}


/***************************************************************************

  I/O

***************************************************************************/

WRITE8_MEMBER(quizshow_state::quizshow_lamps1_w)
{
	// d0-d3: P1 answer button lamps
	for (int i = 0; i < 4; i++)
		output().set_lamp_value(i, data >> i & 1);

	// d4-d7: N/C
}

WRITE8_MEMBER(quizshow_state::quizshow_lamps2_w)
{
	// d0-d3: P2 answer button lamps
	for (int i = 0; i < 4; i++)
		output().set_lamp_value(i + 4, data >> i & 1);

	// d4-d7: N/C
}

WRITE8_MEMBER(quizshow_state::quizshow_lamps3_w)
{
	// d0-d1: start button lamps
	output().set_lamp_value(8, data >> 0 & 1);
	output().set_lamp_value(9, data >> 1 & 1);

	// d2-d3: unused? (chip is shared with quizshow_tape_control_w)
	// d4-d7: N/C
}

WRITE8_MEMBER(quizshow_state::quizshow_tape_control_w)
{
	// d2: enable user category select (changes tape head position)
	output().set_lamp_value(10, data >> 2 & 1);
	m_category_enable = (data & 0xc) == 0xc;

	// d3: tape motor
	// TODO

	// d0-d1: unused? (chip is shared with quizshow_lamps3_w)
	// d4-d7: N/C
}

WRITE8_MEMBER(quizshow_state::quizshow_audio_w)
{
	// d1: audio out
	m_dac->write_signed8((data & 2) ? 0x7f : 0);

	// d0, d2-d7: N/C
}

WRITE8_MEMBER(quizshow_state::quizshow_video_disable_w)
{
	// d0: video disable (looked glitchy when I implemented it, maybe there's more to it)
	// d1-d7: N/C
}

READ8_MEMBER(quizshow_state::quizshow_timing_r)
{
	UINT8 ret = 0x80;

	// d0-d3: 1R-8R (16-line counter)
	ret |= m_clocks >> 1 & 0xf;

	// d4: 8VAC?, use 8V instead
	ret |= m_clocks << 4 & 0x10;

	// d5-d6: 4F-8F
	ret |= m_clocks >> 2 & 0x60;

	// d7: display busy/idle, during in-between tilerows(?) and blanking
	if (m_screen->vpos() >= VBSTART || (m_screen->vpos() + 4) & 8)
		ret &= 0x7f;

	return ret;
}

READ8_MEMBER(quizshow_state::quizshow_tape_signal_r)
{
	// TODO (for now, hold INS to fastforward and it'll show garbage questions where D is always(?) the right answer)
	return machine().rand() & 0x80;
}

WRITE8_MEMBER(quizshow_state::quizshow_main_ram_w)
{
	m_main_ram[offset]=data;
	m_tilemap->mark_tile_dirty(offset);
}


static ADDRESS_MAP_START( quizshow_mem_map, AS_PROGRAM, 8, quizshow_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x0bff) AM_ROM
	AM_RANGE(0x1802, 0x1802) AM_WRITE(quizshow_audio_w)
	AM_RANGE(0x1804, 0x1804) AM_WRITE(quizshow_lamps1_w)
	AM_RANGE(0x1808, 0x1808) AM_WRITE(quizshow_lamps2_w)
	AM_RANGE(0x1810, 0x1810) AM_WRITE(quizshow_lamps3_w)
	AM_RANGE(0x1820, 0x1820) AM_WRITE(quizshow_tape_control_w)
	AM_RANGE(0x1840, 0x1840) AM_WRITE(quizshow_video_disable_w)
	AM_RANGE(0x1881, 0x1881) AM_READ_PORT("IN0")
	AM_RANGE(0x1882, 0x1882) AM_READ_PORT("IN1")
	AM_RANGE(0x1884, 0x1884) AM_READ_PORT("IN2")
	AM_RANGE(0x1888, 0x1888) AM_READ_PORT("IN3")
	AM_RANGE(0x1900, 0x1900) AM_READ(quizshow_timing_r)
	AM_RANGE(0x1e00, 0x1fff) AM_RAM_WRITE(quizshow_main_ram_w) AM_SHARE("main_ram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( quizshow_io_map, AS_IO, 8, quizshow_state )
	ADDRESS_MAP_UNMAP_HIGH
//  AM_RANGE(S2650_CTRL_PORT, S2650_CTRL_PORT) AM_NOP // unused
//  AM_RANGE(S2650_DATA_PORT, S2650_DATA_PORT) AM_NOP // unused
	AM_RANGE(S2650_SENSE_PORT, S2650_SENSE_PORT) AM_READ(quizshow_tape_signal_r)
ADDRESS_MAP_END


/***************************************************************************

  Inputs

***************************************************************************/

CUSTOM_INPUT_MEMBER(quizshow_state::quizshow_tape_headpos_r)
{
	return 1 << m_tape_head_pos;
}

INPUT_CHANGED_MEMBER(quizshow_state::quizshow_category_select)
{
	if (newval)
	{
		if (m_category_enable)
			m_tape_head_pos = (m_tape_head_pos + 1) & 3;
	}
}

static INPUT_PORTS_START( quizshow )
	PORT_START("IN0") // ADR strobe 0
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, quizshow_state,quizshow_tape_headpos_r, NULL)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN2 )

	PORT_START("IN1") // ADR strobe 1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Answer A")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Answer B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P1 Answer C")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P1 Answer D")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Answer A") PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P2 Answer B") PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P2 Answer C") PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P2 Answer D") PORT_PLAYER(2)

	PORT_START("IN2") // ADR strobe 2
	PORT_DIPNAME( 0x0f, 0x05, "Game Duration" )         PORT_DIPLOCATION("SW3:4,3,2,1")
	PORT_DIPSETTING( 0x00, "50 sec. / 5 questions" )
	PORT_DIPSETTING( 0x01, "60 sec. / 6 questions" )
	PORT_DIPSETTING( 0x02, "70 sec. / 7 questions" )
	PORT_DIPSETTING( 0x03, "80 sec. / 8 questions" )
	PORT_DIPSETTING( 0x04, "90 sec. / 9 questions" )
	PORT_DIPSETTING( 0x05, "100 sec. / 10 questions" )
	PORT_DIPSETTING( 0x06, "110 sec. / 11 questions" )
	PORT_DIPSETTING( 0x07, "120 sec. / 12 questions" )
	PORT_DIPSETTING( 0x08, "130 sec. / 13 questions" )
	PORT_DIPSETTING( 0x09, "140 sec. / 14 questions" )
	PORT_DIPSETTING( 0x0a, "150 sec. / 15 questions" ) // not listed in manual
	PORT_DIPSETTING( 0x0b, "160 sec. / 16 questions" ) // "
	PORT_DIPSETTING( 0x0c, "170 sec. / 17 questions" ) // "
	PORT_DIPSETTING( 0x0d, "180 sec. / 18 questions" ) // "
	PORT_DIPSETTING( 0x0e, "190 sec. / 19 questions" ) // "
	PORT_DIPSETTING( 0x0f, "200 sec. / 20 questions" ) // "

	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING( 0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING( 0x10, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x20, 0x00, "Duration Mode" )         PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, "Question Count" )
	PORT_DIPSETTING(    0x20, "Timed" )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) // N/C

	PORT_START("IN3") // ADR strobe 3
	PORT_DIPNAME( 0x0f, 0x05, "Bonus Questions" )       PORT_DIPLOCATION("SW2:4,3,2,1")
	PORT_DIPSETTING( 0x00, "0" )
	PORT_DIPSETTING( 0x01, "1" )
	PORT_DIPSETTING( 0x02, "2" )
	PORT_DIPSETTING( 0x03, "3" )
	PORT_DIPSETTING( 0x04, "4" )
	PORT_DIPSETTING( 0x05, "5" )
	PORT_DIPSETTING( 0x06, "6" )
	PORT_DIPSETTING( 0x07, "7" )
	PORT_DIPSETTING( 0x08, "8" )
	PORT_DIPSETTING( 0x09, "9" )
	PORT_DIPSETTING( 0x0a, "10" ) // not listed in manual
	PORT_DIPSETTING( 0x0b, "11" ) // "
	PORT_DIPSETTING( 0x0c, "12" ) // "
	PORT_DIPSETTING( 0x0d, "13" ) // "
	PORT_DIPSETTING( 0x0e, "14" ) // "
	PORT_DIPSETTING( 0x0f, "15" ) // "
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("CAT")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Category Select") PORT_CHANGED_MEMBER(DEVICE_SELF, quizshow_state,quizshow_category_select, NULL)

INPUT_PORTS_END


/***************************************************************************

  Machine Config

***************************************************************************/

static const gfx_layout tile_layout =
{
	8, 16,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(0,2), RGN_FRAC(1,2) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{
		0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		8*8, 9*8,10*8,11*8,12*8,13*8,14*8,15*8,
	},
	8*16
};

static GFXDECODE_START( quizshow )
	GFXDECODE_ENTRY( "gfx1", 0, tile_layout, 0, 4 )
GFXDECODE_END


TIMER_DEVICE_CALLBACK_MEMBER(quizshow_state::quizshow_clock_timer_cb)
{
	m_clocks++;

	// blink is on 4F and 8F
	int blink_old = m_blink_state;
	m_blink_state = (m_clocks >> 2 & m_clocks >> 1) & 0x40;
	if (m_blink_state != blink_old)
		m_tilemap->mark_all_dirty();
}

void quizshow_state::machine_reset()
{
	m_category_enable = 0;
	m_tape_head_pos = 0;
}

static MACHINE_CONFIG_START( quizshow, quizshow_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", S2650, MASTER_CLOCK / 16) // divider guessed
	MCFG_CPU_PROGRAM_MAP(quizshow_mem_map)
	MCFG_CPU_IO_MAP(quizshow_io_map)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("clock_timer", quizshow_state, quizshow_clock_timer_cb, attotime::from_hz(PIXEL_CLOCK / (HTOTAL * 8))) // 8V

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART)

	MCFG_SCREEN_UPDATE_DRIVER(quizshow_state, screen_update_quizshow)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", quizshow)
	MCFG_PALETTE_ADD("palette", 8*2)
	MCFG_PALETTE_INDIRECT_ENTRIES(2)
	MCFG_PALETTE_INIT_OWNER(quizshow_state, quizshow)

	/* sound hardware (discrete) */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END


/***************************************************************************

  Game drivers

***************************************************************************/

ROM_START( quizshow )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "005464-01.a1", 0x00000, 0x0200, CRC(c9da809a) SHA1(0d16e552398069a4389c34cc9fb6dcc89eb05b9b) )
	ROM_LOAD( "005464-02.c1", 0x00200, 0x0200, CRC(42237134) SHA1(2932d4820f6c9a383cb5a4e504e043e2d479d474) )
	ROM_LOAD( "005464-03.d1", 0x00400, 0x0200, CRC(0c58fee9) SHA1(c7b081bc4f274a29eb758c8758877b15c9e54d79) )
	ROM_LOAD( "005464-04.f1", 0x00600, 0x0200, CRC(4c6cffd4) SHA1(c291d0fa140faa78b807af72c677d53c620b3103) )
	ROM_LOAD( "005464-05.h1", 0x00800, 0x0200, CRC(b8d61b96) SHA1(eb437a5deaf2fc2a9acebbc506321f3151b4eafa) )
	ROM_LOAD( "005464-06.k1", 0x00a00, 0x0200, CRC(200023b2) SHA1(271d0b2b2f985a6c7b7146869ed00990a52dd653) )

	ROM_REGION( 0x0800, "gfx1", ROMREGION_ERASEFF )

	ROM_REGION( 0x0200, "user1", 0 ) // gfx1
	ROM_LOAD_NIB_HIGH( "005466-01.m2", 0x0000, 0x0200, BAD_DUMP CRC(03017820) SHA1(fd118aa706bdc6976e527ed63388fad01e66270e) ) // from Atari's source archive, may have some bad bits
	ROM_LOAD_NIB_LOW ( "005466-02.n2", 0x0000, 0x0200, CRC(cd554367) SHA1(04da83eb6e2f86f88a3495072b98fbdaca485ae8) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "005465-01.f2", 0x0000, 0x0200, CRC(0fe46552) SHA1(d79b1ff0abfaba1ef2d564d1166c3696e0a1a3f1) ) // memory timing
ROM_END


DRIVER_INIT_MEMBER(quizshow_state,quizshow)
{
	UINT8 *gfxdata = memregion("user1")->base();
	UINT8 *dest = memregion("gfx1")->base();

	int tile, line;

	// convert gfx data to 8*16(actually 8*12), and 2bpp for masking inverted colors
	for (tile = 0; tile < 0x40; tile++)
	{
		for (line = 2; line < 14; line ++)
		{
			dest[tile << 4 | line] = 0;
			dest[tile << 4 | line | 0x400] = 0;

			if (line >= 4 && line < 12)
				dest[tile << 4 | line] = gfxdata[(tile ^ 0x3f) << 3 | (line - 4)];
		}
	}
}


GAMEL( 1976, quizshow, 0, quizshow, quizshow, quizshow_state, quizshow, ROT0, "Atari (Kee Games)", "Quiz Show", MACHINE_NOT_WORKING, layout_quizshow )
