// license:BSD-3-Clause
// copyright-holders:Dan Boris
/* Hit Me driver by the EMUL8, led by Dan Boris */

/*

    Hit Me  (c) Ramtek  1976
---------------------------------------

    Memory map

    0000-07ff r    Rom
    0c00-0eff w    Video Ram
    1000-13ff r/w  Scratch Ram


*/

#include "emu.h"
#include "hitme.h"

#include "cpu/i8085/i8085.h"
#include "sound/discrete.h"
#include "emupal.h"
#include "speaker.h"

#include "barricad.lh"

#define MASTER_CLOCK (XTAL(8'945'000)) /* confirmed on schematic */


/*************************************
 *
 *  Video RAM access
 *
 *************************************/

TILE_GET_INFO_MEMBER(hitme_state::get_hitme_tile_info)
{
	/* the code is the low 6 bits */
	uint8_t code = m_videoram[tile_index] & 0x3f;
	tileinfo.set(0, code, 0, 0);
}


void hitme_state::hitme_vidram_w(offs_t offset, uint8_t data)
{
	/* mark this tile dirty */
	m_videoram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}



/*************************************
 *
 *  Video start/update
 *
 *************************************/

void hitme_state::video_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(hitme_state::get_hitme_tile_info)), TILEMAP_SCAN_ROWS, 8, 10, 40, 19);
}


VIDEO_START_MEMBER(hitme_state,barricad)
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(hitme_state::get_hitme_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 24);
}


uint32_t hitme_state::screen_update_hitme(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* the card width resistor comes from an input port, scaled to the range 0-25 kOhms */
	double width_resist = ioport("WIDTH")->read() * 25000 / 100;
	/* this triggers a oneshot for the following length of time */
	double width_duration = 0.45 * 1000e-12 * width_resist;
	/* the dot clock runs at the standard horizontal frequency * 320+16 clocks per scanline */
	double dot_freq = 15750 * 336;
	/* the number of pixels is the duration times the frequency */
	int width_pixels = width_duration * dot_freq;
	offs_t offs = 0;

	/* start by drawing the tilemap */
	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	/* now loop over and invert anything */
	for (int y = 0; y < 19; y++)
	{
		int dy = bitmap.rowpixels();
		int inv = 0;
		for (int x = 0; x < 40; x++, offs++)
		{
			/* if the high bit is set, reset the oneshot */
			if (m_videoram[y * 40 + x] & 0x80)
				inv = width_pixels;

			/* invert pixels until we run out */
			for (int xx = 0; xx < 8 && inv; xx++, inv--)
			{
				uint16_t *const dest = &bitmap.pix(y * 10, x * 8 + xx);
				dest[0 * dy] ^= 1;
				dest[1 * dy] ^= 1;
				dest[2 * dy] ^= 1;
				dest[3 * dy] ^= 1;
				dest[4 * dy] ^= 1;
				dest[5 * dy] ^= 1;
				dest[6 * dy] ^= 1;
				dest[7 * dy] ^= 1;
				dest[8 * dy] ^= 1;
				dest[9 * dy] ^= 1;
			}
		}
	}
	return 0;
}


uint32_t hitme_state::screen_update_barricad(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}



/*************************************
 *
 *  Input ports
 *
 *************************************/

uint8_t hitme_state::read_port_and_t0( int port )
{
	static const char *const portnames[] = { "IN0", "IN1", "IN2", "IN3" };

	uint8_t val = ioport(portnames[port])->read();
	if (machine().time() > m_timeout_time)
		val ^= 0x80;
	return val;
}


uint8_t hitme_state::read_port_and_t0_and_hblank( int port )
{
	uint8_t val = read_port_and_t0(port);
	if (m_screen->hpos() < (m_screen->width() * 9 / 10))
		val ^= 0x04;
	return val;
}


uint8_t hitme_state::hitme_port_0_r()
{
	return read_port_and_t0_and_hblank(0);
}


uint8_t hitme_state::hitme_port_1_r()
{
	return read_port_and_t0(1);
}


uint8_t hitme_state::hitme_port_2_r()
{
	return read_port_and_t0_and_hblank(2);
}


uint8_t hitme_state::hitme_port_3_r()
{
	return read_port_and_t0(3);
}



/*************************************
 *
 *  Output ports
 *
 *************************************/

void hitme_state::output_port_0_w(uint8_t data)
{
	/*
	    Note: We compute the timeout time on a write here. Unfortunately, the situation is
	    kind of weird, because the discrete sound system is also affected by this timeout.
	    In fact, it is very important that our timing calculation timeout AFTER the sound
	    system's equivalent computation, or else we will hang notes.
	*/
	uint8_t raw_game_speed = ioport("R3")->read();
	double resistance = raw_game_speed * 25000 / 100;
	attotime duration = attotime(0, ATTOSECONDS_PER_SECOND * 0.45 * 6.8e-6 * resistance * (data + 1));
	m_timeout_time = machine().time() + duration;

	m_discrete->write(HITME_DOWNCOUNT_VAL, data);
	m_discrete->write(HITME_OUT0, 1);
}


void hitme_state::output_port_1_w(uint8_t data)
{
	m_discrete->write(HITME_ENABLE_VAL, data);
	m_discrete->write(HITME_OUT1, 1);
}



/*************************************
 *
 *  Memory maps
 *
 *************************************/

/*
    Note: the 8080 puts I/O port addresses out on the upper 8 address bits and asserts
    IORQ. Most systems decode IORQ, but hitme doesn't, which means that all the I/O
    port accesses can also be made via memory mapped accesses with the port number in the
    upper 8 bits.
*/

void hitme_state::hitme_map(address_map &map)
{
	map.global_mask(0x1fff);
	map(0x0000, 0x09ff).rom();
	map(0x0c00, 0x0eff).ram().w(FUNC(hitme_state::hitme_vidram_w)).share("videoram");
	map(0x1000, 0x10ff).mirror(0x300).ram();
	map(0x1400, 0x14ff).r(FUNC(hitme_state::hitme_port_0_r));
	map(0x1500, 0x15ff).r(FUNC(hitme_state::hitme_port_1_r));
	map(0x1600, 0x16ff).r(FUNC(hitme_state::hitme_port_2_r));
	map(0x1700, 0x17ff).r(FUNC(hitme_state::hitme_port_3_r));
	map(0x1800, 0x18ff).portr("IN4");
	map(0x1900, 0x19ff).portr("IN5");
	map(0x1d00, 0x1dff).w(FUNC(hitme_state::output_port_0_w));
	map(0x1e00, 0x1fff).w(FUNC(hitme_state::output_port_1_w));
}


void hitme_state::hitme_portmap(address_map &map)
{
	map(0x14, 0x14).r(FUNC(hitme_state::hitme_port_0_r));
	map(0x15, 0x15).r(FUNC(hitme_state::hitme_port_1_r));
	map(0x16, 0x16).r(FUNC(hitme_state::hitme_port_2_r));
	map(0x17, 0x17).r(FUNC(hitme_state::hitme_port_3_r));
	map(0x18, 0x18).portr("IN4");
	map(0x19, 0x19).portr("IN5");
	map(0x1d, 0x1d).w(FUNC(hitme_state::output_port_0_w));
	map(0x1e, 0x1f).w(FUNC(hitme_state::output_port_1_w));
}



/*************************************
 *
 *  Graphics layouts
 *
 *************************************/

/*
    Note: the hitme video generator adds two blank lines to the beginning of each
    row. In order to simulate this, we decode an extra two lines at the top of each
    character.
*/

static const gfx_layout hitme_charlayout =
{
	8,10,
	RGN_FRAC(1,2),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0x200*8, 0x200*8, 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( gfx_hitme )
	GFXDECODE_ENTRY( "gfx1", 0, hitme_charlayout, 0, 2  )
GFXDECODE_END


static GFXDECODE_START( gfx_barricad )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x1,   0, 1  )
GFXDECODE_END




/*************************************
 *
 *  Machine drivers
 *
 *************************************/

void hitme_state::machine_start()
{
	save_item(NAME(m_timeout_time));
}

void hitme_state::machine_reset()
{
	m_timeout_time = attotime::zero;
}

void hitme_state::hitme(machine_config &config)
{
	/* basic machine hardware */
	I8080(config, m_maincpu, MASTER_CLOCK/16);
	m_maincpu->set_addrmap(AS_PROGRAM, &hitme_state::hitme_map);
	m_maincpu->set_addrmap(AS_IO, &hitme_state::hitme_portmap);


	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(40*8, 19*10);
	m_screen->set_visarea(0*8, 40*8-1, 0*8, 19*10-1);
	m_screen->set_screen_update(FUNC(hitme_state::screen_update_hitme));
	m_screen->set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_hitme);
	PALETTE(config, "palette", palette_device::MONOCHROME);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	DISCRETE(config, m_discrete, hitme_discrete).add_route(ALL_OUTPUTS, "mono", 1.0);
}



/*
    Note: The Barricade rom is using a resolution of 32x24 which suggests slightly
    different hardware from HitMe (40x19) however the screenshot on the arcade
    flyer is using a 40x19 resolution. So is this a different version of
    Barricade or is the resolution set by a dip switch?
*/

void hitme_state::barricad(machine_config &config)
{
	hitme(config);

	/* video hardware */
	m_screen->set_size(32*8, 24*8);
	m_screen->set_visarea(0*8, 32*8-1, 0*8, 24*8-1);
	m_screen->set_screen_update(FUNC(hitme_state::screen_update_barricad));

	m_gfxdecode->set_info(gfx_barricad);

	MCFG_VIDEO_START_OVERRIDE(hitme_state,barricad)
}



/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( hitme )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )                 /* Start button */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )                 /* Always high */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_CUSTOM )                /* Hblank */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )                 /* Always high */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) /* P1 Stand button */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) /* P1 Hit button */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) /* P1 Bet button */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM )                /* Time out counter (*TO) */

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )                 /* Always high */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )                 /* Aux 2 dipswitch - Unused */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )                 /* Always high */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) /* P2 Stand button */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) /* P2 Hit button */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) /* P2 Bet button */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM )                /* Time out counter (*TO) */

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x00, "Extra Hand On Natural" )         /* Aux 1 dipswitch */
	PORT_DIPSETTING(    0x00, DEF_STR ( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR ( On )  )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )                 /* Always high */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_CUSTOM )                /* Hblank */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )                 /* Always high */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3) /* P3 Stand button */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3) /* P3 Hit button */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3) /* P3 Bet button */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM )                /* Time out counter (*TO) */

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )                /* Time out counter (TOC1) */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )                 /* Always high */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )                 /* Aux 2 dipswitch - Unused */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )                 /* Always high */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4) /* P4 Stand button */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4) /* P4 Hit button */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4) /* P4 Bet button */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM )                /* Time out counter (*TO) */

	PORT_START("IN4")
	PORT_DIPNAME( 0x07, 0x07, "Number of Chips" )
	PORT_DIPSETTING(    0x00, "5 Chips" )
	PORT_DIPSETTING(    0x01, "10 Chips" )
	PORT_DIPSETTING(    0x02, "15 Chips" )
	PORT_DIPSETTING(    0x03, "20 Chips" )
	PORT_DIPSETTING(    0x04, "25 Chips" )
	PORT_DIPSETTING(    0x05, "30 Chips" )
	PORT_DIPSETTING(    0x06, "35 Chips" )
	PORT_DIPSETTING(    0x07, "40 Chips" )
	PORT_BIT( 0xf8, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN5")
	PORT_DIPNAME( 0x07, 0x00, "Number of Hands" )
	PORT_DIPSETTING(    0x00, "5 Hands" )
	PORT_DIPSETTING(    0x01, "10 Hands" )
	PORT_DIPSETTING(    0x02, "15 Hands" )
	PORT_DIPSETTING(    0x03, "20 Hands" )
	PORT_DIPSETTING(    0x04, "25 Hands" )
	PORT_DIPSETTING(    0x05, "30 Hands" )
	PORT_DIPSETTING(    0x06, "35 Hands" )
	PORT_DIPSETTING(    0x07, "40 Hands" )
	PORT_BIT( 0xf8, IP_ACTIVE_HIGH, IPT_UNUSED )

	/* this is actually a variable resistor */
	PORT_START("R3")
	PORT_ADJUSTER(30, "Game Speed")

	/* this is actually a variable resistor */
	PORT_START("WIDTH")
	PORT_ADJUSTER(50, "Card Width")
INPUT_PORTS_END

static INPUT_PORTS_START( super21 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_CUSTOM )                /* Hblank */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )                /* Always high */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4) /* P4 Stand button */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4) /* P4 Hit button */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4) /* P4 Ante button */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM )                /* Time out counter (*TO) */

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )                /* Aux 2 dipswitch? */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3) /* P3 Stand button */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3) /* P3 Hit button */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3) /* P3 Ante button */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM )                /* Time out counter (*TO) */

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )                /* Aux 1 dipswitch? */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_CUSTOM )                /* Hblank */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) /* P2 Stand button */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) /* P2 Hit button */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) /* P2 Ante button */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM )                /* Time out counter (*TO) */

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )                /* Time out counter (TOC1) */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )                /* Aux 2 dipswitch? */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) /* P1 Stand button */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) /* P1 Hit button */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) /* P1 Ante button */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM )                /* Time out counter (*TO) */

	PORT_START("IN4")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN5") // bit 2 is chip/bet related, but also causes gfx glitches when set
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	/* this is actually a variable resistor */
	PORT_START("R3")
	PORT_ADJUSTER(30, "Game Speed")

	/* this is actually a variable resistor */
	PORT_START("WIDTH")
	PORT_ADJUSTER(50, "Card Width")
INPUT_PORTS_END

static INPUT_PORTS_START( barricad )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )                         /* Start button */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )                         /* Always high */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_CUSTOM )                        /* Hblank */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT  ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP  ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM )                        /* Time out counter (*TO) */

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )                         /* Always high */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )                         /* Aux 2 dipswitch - Unused */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT  ) PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP  ) PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM )                        /* Time out counter (*TO) */

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )                        /* ??? */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )                         /* Always high */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_CUSTOM )                        /* Hblank */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT  ) PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP  ) PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM )                        /* Time out counter (*TO) */

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )                        /* Time out counter (TOC1) */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )                        /* Always high */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )                        /* Aux 2 dipswitch - Unused */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT  ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP  ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM )                        /* Time out counter (*TO) */

	/* On the flyer it says that barricade has both user adjustable points per
	    game, and speed. From experimenting it looks like points per game is the
	    same dipswitch as hitme's chips, and speed is hitme's hands. The flyer
	  says 1-7 points per games, but it really can go to 8. */

	PORT_START("IN4")
	PORT_DIPNAME( 0x07, 0x07, "Points Per Game" )
	PORT_DIPSETTING(    0x00, "1 Point" )
	PORT_DIPSETTING(    0x01, "2 Points" )
	PORT_DIPSETTING(    0x02, "3 Points" )
	PORT_DIPSETTING(    0x03, "4 Points" )
	PORT_DIPSETTING(    0x04, "5 Points" )
	PORT_DIPSETTING(    0x05, "6 Points" )
	PORT_DIPSETTING(    0x06, "7 Points" )
	PORT_DIPSETTING(    0x07, "8 Points" )

	/* These are like lives, you lose a point if you crash. The last person with
	    points wins the game. */

	PORT_START("IN5")
	PORT_DIPNAME( 0x07, 0x00, "Game Speed" )
	PORT_DIPSETTING(    0x00, "Fast Fast" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x05, "3" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x07, "Slow Slow" )

	/* this is actually a variable resistor */
	PORT_START("R3")
	PORT_ADJUSTER(30, "Tone")
INPUT_PORTS_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

/*
Hit me by Ramtek

Etched in copper on board   HIT ME by RAMTEK Made in U.S.A
                ASSY 550596 D               D was as sticker
                SER 957                 957 was a sticker

Etched in copper on back    FAB 550595-C

.b7         stamped     15347 7625
                    HM2-0


.c7 IM5605      handwritten HM-2-2

.d7         stamped     15349 7625
                    HM1-4

.e7         stamped     15350 7625
                    HM1-6

.f7         stamped     15351 7625
                    HM2-8

.h7 IM560?      handwritten HM0-CG  hard to read

All chips we read as 82s141 - guessed because of 24 pin pinout and 512x8 rom according to mame
*/

ROM_START( hitme )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_INVERT )
	ROM_LOAD( "hm2-0.b7", 0x0000, 0x0200, CRC(1b94caad) SHA1(30987e5cb0d55f3666dd63f04132a0e65988caea) )
	ROM_LOAD( "hm-2-2.c7",0x0200, 0x0200, CRC(fa7e8c33) SHA1(2d04635cee32d49cccd9a9a855b3a2be4295c2a5) )
	ROM_LOAD( "hm1-4.d7", 0x0400, 0x0200, CRC(10dd4581) SHA1(eaa7c9e75f79befc8abf0bd0913bbf15dd04230e) )
	ROM_LOAD( "hm1-6.e7", 0x0600, 0x0200, CRC(18e4c83c) SHA1(bce987da371b7946262d7dff65f61ff2fcb55bf6) )
	ROM_LOAD( "hm2-8.f7", 0x0800, 0x0200, CRC(f28983f8) SHA1(89167cf41ba71d90cd6133751158bb99bfc5e829) )

	ROM_REGION( 0x0400, "gfx1", ROMREGION_ERASE00 )
	ROM_LOAD( "hmcg.h7", 0x0000, 0x0200, CRC(818f5fbe) SHA1(e2b3349e51ba57d14f3388ba93891bc6274b7a14) )
ROM_END

ROM_START( hitme1 )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_INVERT )
	ROM_LOAD( "hm0.b7", 0x0000, 0x0200, CRC(6c48c50f) SHA1(42dc7c3461687e5be4393cc21d695bc84ae4f5dc) )
	ROM_LOAD( "hm2.c7", 0x0200, 0x0200, CRC(25d47ba4) SHA1(6f3bb4ca6918dc07f37d0c0c7fe5ec53aa7171a5) )
	ROM_LOAD( "hm4.d7", 0x0400, 0x0200, CRC(f8bfda8d) SHA1(48bbc106f8d80d6c1ad1a2c1575ce7d6452fbe9d) )
	ROM_LOAD( "hm6.e7", 0x0600, 0x0200, CRC(8aa87118) SHA1(aca395a4f6a1981cd89ca99e05935d72adcb69ca) )

	ROM_REGION( 0x0400, "gfx1", ROMREGION_ERASE00 )
	ROM_LOAD( "hmcg.h7", 0x0000, 0x0200, CRC(818f5fbe) SHA1(e2b3349e51ba57d14f3388ba93891bc6274b7a14) )
ROM_END

ROM_START( mirco21 )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_INVERT )
	ROM_LOAD( "mirco1.bin", 0x0000, 0x0200, CRC(aa796ad7) SHA1(2908bdb4ab17a2f5bc4da2f957906bf2b57afa50) )
	ROM_LOAD( "hm2.c7", 0x0200, 0x0200, CRC(25d47ba4) SHA1(6f3bb4ca6918dc07f37d0c0c7fe5ec53aa7171a5) )
	ROM_LOAD( "hm4.d7", 0x0400, 0x0200, CRC(f8bfda8d) SHA1(48bbc106f8d80d6c1ad1a2c1575ce7d6452fbe9d) )
	ROM_LOAD( "hm6.e7", 0x0600, 0x0200, CRC(8aa87118) SHA1(aca395a4f6a1981cd89ca99e05935d72adcb69ca) )

	ROM_REGION( 0x0400, "gfx1", ROMREGION_ERASE00 )
	ROM_LOAD( "hmcg.h7", 0x0000, 0x0200, CRC(818f5fbe) SHA1(e2b3349e51ba57d14f3388ba93891bc6274b7a14) )
ROM_END

ROM_START( super21 )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_INVERT )
	ROM_LOAD( "1.h1", 0x0000, 0x0200, CRC(cecf2224) SHA1(794d7b9f2533f7c00bbe1c7b3d37eb08c25a09bb) )
	ROM_LOAD( "3.h2", 0x0200, 0x0200, CRC(eb62ea5f) SHA1(e4b9fafe2cab5a31549504ce430eadc230e3da39) )
	ROM_LOAD( "2.j1", 0x0400, 0x0200, CRC(fea88bf2) SHA1(a18081cecd29c7929e589c5fda7ba1033ef8e7c3) )
	ROM_LOAD( "4.j2", 0x0600, 0x0200, CRC(63238ddb) SHA1(7197640e758d7ef93c659eeea098424d3aa50314) )
	ROM_LOAD( "5.k2", 0x0800, 0x0200, CRC(89983131) SHA1(c22c02a3bff0a61c5f341f1c2b3b37150ad2c1e3) )

	ROM_REGION( 0x0400, "gfx1", ROMREGION_ERASE00 )
	ROM_LOAD( "7.h6", 0x0000, 0x0200, CRC(818f5fbe) SHA1(e2b3349e51ba57d14f3388ba93891bc6274b7a14) )
ROM_END

ROM_START( barricad )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_INVERT )
	ROM_LOAD( "550806.7b",   0x0000, 0x0200, CRC(ea7f5da7) SHA1(c0ad37a0ffdb0500e8adc8fb9c4369e461307f84) )
	ROM_LOAD( "550807.7c",   0x0200, 0x0200, CRC(0afef174) SHA1(2a7be988262b855bc81a1b0036fa9f2481d4d53b) )
	ROM_LOAD( "550808.7d",   0x0400, 0x0200, CRC(6e02d260) SHA1(8a1640a1d56cbc34f74f07bc15e77db63635e8f5) )
	ROM_LOAD( "550809.7e",   0x0600, 0x0200, CRC(d834a63f) SHA1(ffb631cc4f51a670c7cd30df1c79bf51301d9e9a) )

	ROM_REGION( 0x0400, "gfx1", 0 )
	ROM_LOAD( "550805.7h",   0x0000, 0x0200, CRC(35197599) SHA1(3c49af89b1bc1d495e1d6265ff3feaf33c56facb) )
ROM_END

ROM_START( brickyrd )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_INVERT )
	ROM_LOAD( "550806.7b",   0x0000, 0x0200, CRC(ea7f5da7) SHA1(c0ad37a0ffdb0500e8adc8fb9c4369e461307f84) )
	ROM_LOAD( "barricad.7c", 0x0200, 0x0200, CRC(94e1d1c0) SHA1(f6e6f9a783867c3602ba8cff6a18c47c5df987a4) )
	ROM_LOAD( "550808.7d",   0x0400, 0x0200, CRC(6e02d260) SHA1(8a1640a1d56cbc34f74f07bc15e77db63635e8f5) )
	ROM_LOAD( "barricad.7e", 0x0600, 0x0200, CRC(2b1d914f) SHA1(f1a6631949a7c62f5de39d58821e1be36b98629e) )

	ROM_REGION( 0x0400, "gfx1", 0 )
	ROM_LOAD( "barricad.7h", 0x0000, 0x0200, CRC(c676fd22) SHA1(c37bf92f5a146a93bd977b2a05485addc00ab066) )
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1976, hitme,    0,        hitme,    hitme,    hitme_state, empty_init, ROT0, "Ramtek",      "Hit Me (set 1)",   MACHINE_SUPPORTS_SAVE )   // 05/1976
GAME( 1976, hitme1,   hitme,    hitme,    hitme,    hitme_state, empty_init, ROT0, "Ramtek",      "Hit Me (set 2)",   MACHINE_SUPPORTS_SAVE )
GAME( 1976, mirco21,  hitme,    hitme,    hitme,    hitme_state, empty_init, ROT0, "Mirco Games", "21 (Mirco)",       MACHINE_SUPPORTS_SAVE )   // 08/1976, licensed?
GAME( 1978, super21,  0,        hitme,    super21,  hitme_state, empty_init, ROT0, "Mirco Games", "Super Twenty One", MACHINE_SUPPORTS_SAVE )
GAMEL(1976, barricad, 0,        barricad, barricad, hitme_state, empty_init, ROT0, "Ramtek",      "Barricade",        MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_barricad )
GAMEL(1976, brickyrd, barricad, barricad, barricad, hitme_state, empty_init, ROT0, "Ramtek",      "Brickyard",        MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_barricad )
