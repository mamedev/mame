// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/***************************************************************************

    Atari Super Breakout hardware

    driver by Mike Balfour

    Games supported:
        * Super Breakout
        * Super Breakout (Canyon and Vertical Breakout, prototype)
        * Super Breakout (Cocktail, prototype)

    Known issues:
        * none at this time

****************************************************************************

    To do:

    * Merge with Sprint 1

****************************************************************************

    Stephh's notes (based on the games M6502 code and some tests) :

      - Each time the game is reset, it is set to "Cavity".
        I can't remember if it's the correct behaviour or not,
        but the VBLANK interruption is not called in "demo mode".
      - You can only select the game after 1st coin is inserted
        and before you press BUTTON1 to launch the first ball,
        then the VBLANK interruption is no more called.
        This means that player 2 plays the same game as player 1.

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "sound/dac.h"

#include "sbrkout.lh"


class sbrkout_state : public driver_device
{
public:
	sbrkout_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_dac(*this, "dac"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	required_shared_ptr<UINT8> m_videoram;
	emu_timer *m_scanline_timer;
	emu_timer *m_pot_timer;
	tilemap_t *m_bg_tilemap;
	UINT8 m_sync2_value;
	UINT8 m_pot_mask[2];
	UINT8 m_pot_trigger[2];
	DECLARE_WRITE8_MEMBER(irq_ack_w);
	DECLARE_READ8_MEMBER(switches_r);
	DECLARE_READ8_MEMBER(sbrkoutct_switches_r);
	DECLARE_WRITE8_MEMBER(pot_mask1_w);
	DECLARE_WRITE8_MEMBER(pot_mask2_w);
	DECLARE_WRITE8_MEMBER(start_1_led_w);
	DECLARE_WRITE8_MEMBER(start_2_led_w);
	DECLARE_WRITE8_MEMBER(serve_led_w);
	DECLARE_WRITE8_MEMBER(serve_2_led_w);
	DECLARE_WRITE8_MEMBER(coincount_w);
	DECLARE_READ8_MEMBER(sync_r);
	DECLARE_READ8_MEMBER(sync2_r);
	DECLARE_WRITE8_MEMBER(sbrkout_videoram_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_sbrkout(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(scanline_callback);
	TIMER_CALLBACK_MEMBER(pot_trigger_callback);
	void update_nmi_state();
	required_device<cpu_device> m_maincpu;
	required_device<dac_device> m_dac;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
};




/*************************************
 *
 *  Constants
 *
 *************************************/

#define MAIN_CLOCK      XTAL_12_096MHz
#define TIME_4V         attotime::from_hz(MAIN_CLOCK/2/256/2/4)



/*************************************
 *
 *  Prototypes
 *
 *************************************/






/*************************************
 *
 *  Machine setup
 *
 *************************************/

void sbrkout_state::machine_start()
{
	UINT8 *videoram = m_videoram;
	membank("bank1")->set_base(&videoram[0x380]);
	m_scanline_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(sbrkout_state::scanline_callback),this));
	m_pot_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(sbrkout_state::pot_trigger_callback),this));

	save_item(NAME(m_sync2_value));
	save_item(NAME(m_pot_mask));
	save_item(NAME(m_pot_trigger));
}


void sbrkout_state::machine_reset()
{
	m_scanline_timer->adjust(m_screen->time_until_pos(0));
}



/*************************************
 *
 *  Interrupts
 *
 *************************************/

TIMER_CALLBACK_MEMBER(sbrkout_state::scanline_callback)
{
	UINT8 *videoram = m_videoram;
	int scanline = param;

	/* force a partial update before anything happens */
	m_screen->update_partial(scanline);

	/* if this is a rising edge of 16V, assert the CPU interrupt */
	if (scanline % 32 == 16)
		m_maincpu->set_input_line(0, ASSERT_LINE);

	/* update the DAC state */
	m_dac->write_unsigned8((videoram[0x380 + 0x11] & (scanline >> 2)) ? 255 : 0);

	/* on the VBLANK, read the pot and schedule an interrupt time for it */
	if (scanline == m_screen->visible_area().max_y + 1)
	{
		UINT8 potvalue = ioport("PADDLE")->read();
		m_pot_timer->adjust(m_screen->time_until_pos(56 + (potvalue / 2), (potvalue % 2) * 128));
	}

	/* call us back in 4 scanlines */
	scanline += 4;
	if (scanline >= m_screen->height())
		scanline = 0;
	m_scanline_timer->adjust(m_screen->time_until_pos(scanline), scanline);
}


WRITE8_MEMBER(sbrkout_state::irq_ack_w)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
}



/*************************************
 *
 *  Inputs
 *
 *************************************/

READ8_MEMBER(sbrkout_state::switches_r)
{
	UINT8 result = 0xff;

	/* DIP switches are selected by ADR0+ADR1 if ADR3 == 0 */
	if ((offset & 0x0b) == 0x00)
		result &= (ioport("DIPS")->read() << 6) | 0x3f;
	if ((offset & 0x0b) == 0x01)
		result &= (ioport("DIPS")->read() << 4) | 0x3f;
	if ((offset & 0x0b) == 0x02)
		result &= (ioport("DIPS")->read() << 0) | 0x3f;
	if ((offset & 0x0b) == 0x03)
		result &= (ioport("DIPS")->read() << 2) | 0x3f;

	/* other switches are selected by ADR0+ADR1+ADR2 if ADR4 == 0 */
	if ((offset & 0x17) == 0x00)
		result &= (ioport("SELECT")->read() << 7) | 0x7f;
	if ((offset & 0x17) == 0x04)
		result &= ((m_pot_trigger[0] & ~m_pot_mask[0]) << 7) | 0x7f;
	if ((offset & 0x17) == 0x05)
		result &= ((m_pot_trigger[1] & ~m_pot_mask[1]) << 7) | 0x7f;
	if ((offset & 0x17) == 0x06)
		result &= ioport("SERVE")->read();
	if ((offset & 0x17) == 0x07)
		result &= (ioport("SELECT")->read() << 6) | 0x7f;

	return result;
}

READ8_MEMBER(sbrkout_state::sbrkoutct_switches_r)
{
	UINT8 result = 0xff;

	switch( offset )
	{
		case 0x28: result = ioport("SELECT")->read(); break;
		case 0x2e: result = ioport("SERVE")->read(); break;
		case 0x2f: result = ioport("SERVE2")->read(); break;
		case 0x30: result = (ioport("DIPS")->read() & 0x03) << 6; break;
		case 0x31: result = (ioport("DIPS")->read() & 0x0c) << 4; break;
		case 0x32: result = ioport("DIPS")->read() & 0xc0; break;
		case 0x33: result = (ioport("DIPS")->read() & 0x30) << 2; break;
		default: logerror("Unknown port read %x\n", offset); break;
	}
	return result;
}

void sbrkout_state::update_nmi_state()
{
	if ((m_pot_trigger[0] & ~m_pot_mask[0]) | (m_pot_trigger[1] & ~m_pot_mask[1]))
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	else
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}


TIMER_CALLBACK_MEMBER(sbrkout_state::pot_trigger_callback)
{
	m_pot_trigger[param] = 1;
	update_nmi_state();
}


WRITE8_MEMBER(sbrkout_state::pot_mask1_w)
{
	m_pot_mask[0] = ~offset & 1;
	m_pot_trigger[0] = 0;
	update_nmi_state();
}


WRITE8_MEMBER(sbrkout_state::pot_mask2_w)
{
	m_pot_mask[1] = ~offset & 1;
	m_pot_trigger[1] = 0;
	update_nmi_state();
}



/*************************************
 *
 *  Lamps and other outputs
 *
 *************************************/

/*
    The LEDs are turned on and off by two consecutive memory addresses.  The
    first address turns them off, the second address turns them on.  This is
    reversed for the Serve LED, which has a NOT on the signal.
*/

WRITE8_MEMBER(sbrkout_state::start_1_led_w)
{
	output().set_led_value(0, offset & 1);
}


WRITE8_MEMBER(sbrkout_state::start_2_led_w)
{
	output().set_led_value(1, offset & 1);
}


WRITE8_MEMBER(sbrkout_state::serve_led_w)
{
	output().set_led_value(0, ~offset & 1);
}

WRITE8_MEMBER(sbrkout_state::serve_2_led_w)
{
	output().set_led_value(1, ~offset & 1);
}

WRITE8_MEMBER(sbrkout_state::coincount_w)
{
	machine().bookkeeping().coin_counter_w(0, offset & 1);
}



/*************************************
 *
 *  Video timing
 *
 *************************************/

READ8_MEMBER(sbrkout_state::sync_r)
{
	int hpos = m_screen->hpos();
	m_sync2_value = (hpos >= 128 && hpos <= m_screen->visible_area().max_x);
	return m_screen->vpos();
}


READ8_MEMBER(sbrkout_state::sync2_r)
{
	return (m_sync2_value << 7) | 0x7f;
}



/*************************************
 *
 *  Background tilemap
 *
 *************************************/

TILE_GET_INFO_MEMBER(sbrkout_state::get_bg_tile_info)
{
	UINT8 *videoram = m_videoram;
	int code = (videoram[tile_index] & 0x80) ? videoram[tile_index] : 0;
	SET_TILE_INFO_MEMBER(0, code, 0, 0);
}


void sbrkout_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(sbrkout_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}


WRITE8_MEMBER(sbrkout_state::sbrkout_videoram_w)
{
	UINT8 *videoram = m_videoram;
	videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}



/*************************************
 *
 *  Video update
 *
 *************************************/

UINT32 sbrkout_state::screen_update_sbrkout(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 *videoram = m_videoram;
	int ball;

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	for (ball = 2; ball >= 0; ball--)
	{
		int code = ((videoram[0x380 + 0x18 + ball * 2 + 1] & 0x80) >> 7);
		int sx = 31 * 8 - videoram[0x380 + 0x10 + ball * 2];
		int sy = 30 * 8 - videoram[0x380 + 0x18 + ball * 2];

		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect, code, 0, 0, 0, sx, sy, 0);
	}
	return 0;
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

/* full memory map derived from schematics */
static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, sbrkout_state )
	ADDRESS_MAP_GLOBAL_MASK(0x3fff)
	AM_RANGE(0x0000, 0x007f) AM_MIRROR(0x380) AM_RAMBANK("bank1")
	AM_RANGE(0x0400, 0x07ff) AM_RAM_WRITE(sbrkout_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x0800, 0x083f) AM_READ(switches_r)
	AM_RANGE(0x0840, 0x0840) AM_MIRROR(0x003f) AM_READ_PORT("COIN")
	AM_RANGE(0x0880, 0x0880) AM_MIRROR(0x003f) AM_READ_PORT("START")
	AM_RANGE(0x08c0, 0x08c0) AM_MIRROR(0x003f) AM_READ_PORT("SERVICE")
	AM_RANGE(0x0c00, 0x0c00) AM_MIRROR(0x03ff) AM_READ(sync_r)
	AM_RANGE(0x0c10, 0x0c11) AM_MIRROR(0x000e) AM_WRITE(serve_led_w)
	AM_RANGE(0x0c30, 0x0c31) AM_MIRROR(0x000e) AM_WRITE(start_1_led_w)
	AM_RANGE(0x0c40, 0x0c41) AM_MIRROR(0x000e) AM_WRITE(start_2_led_w)
	AM_RANGE(0x0c50, 0x0c51) AM_MIRROR(0x000e) AM_WRITE(pot_mask1_w)
	AM_RANGE(0x0c60, 0x0c61) AM_MIRROR(0x000e) AM_WRITE(pot_mask2_w)
	AM_RANGE(0x0c70, 0x0c71) AM_MIRROR(0x000e) AM_WRITE(coincount_w)
	AM_RANGE(0x0c80, 0x0c80) AM_MIRROR(0x007f) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x0e00, 0x0e00) AM_MIRROR(0x007f) AM_WRITE(irq_ack_w)
	AM_RANGE(0x1000, 0x1000) AM_MIRROR(0x03ff) AM_READ(sync2_r)
	AM_RANGE(0x2800, 0x3fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sbrkoutct_main_map, AS_PROGRAM, 8, sbrkout_state )
	ADDRESS_MAP_GLOBAL_MASK(0x3fff)
	AM_RANGE(0x0000, 0x007f) AM_MIRROR(0x380) AM_RAMBANK("bank1")
	AM_RANGE(0x0400, 0x07ff) AM_RAM_WRITE(sbrkout_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x0800, 0x083f) AM_READ(sbrkoutct_switches_r)
	AM_RANGE(0x0840, 0x0840) AM_MIRROR(0x003f) AM_READ_PORT("COIN")
	AM_RANGE(0x0880, 0x0880) AM_MIRROR(0x003f) AM_READ_PORT("START")
	AM_RANGE(0x08c0, 0x08c0) AM_MIRROR(0x003f) AM_READ_PORT("SERVICE")
	AM_RANGE(0x0c00, 0x0c00) AM_MIRROR(0x03ff) AM_READ(sync_r)
	AM_RANGE(0x0c10, 0x0c11) AM_MIRROR(0x000e) AM_WRITE(serve_led_w)
	AM_RANGE(0x0c20, 0x0c21) AM_MIRROR(0x000e) AM_WRITE(serve_2_led_w)
	AM_RANGE(0x0c30, 0x0c31) AM_MIRROR(0x000e) AM_WRITE(start_1_led_w)
	AM_RANGE(0x0c40, 0x0c41) AM_MIRROR(0x000e) AM_WRITE(start_2_led_w)
	AM_RANGE(0x0c50, 0x0c51) AM_MIRROR(0x000e) AM_WRITE(pot_mask1_w)
	AM_RANGE(0x0c60, 0x0c61) AM_MIRROR(0x000e) AM_WRITE(pot_mask2_w)
	AM_RANGE(0x0c70, 0x0c71) AM_MIRROR(0x000e) AM_WRITE(coincount_w)
	AM_RANGE(0x0c80, 0x0c80) AM_MIRROR(0x007f) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x0e00, 0x0e00) AM_MIRROR(0x007f) AM_WRITE(irq_ack_w)
	AM_RANGE(0x1000, 0x1000) AM_MIRROR(0x03ff) AM_READ(sync2_r)
	AM_RANGE(0x2800, 0x3fff) AM_ROM
ADDRESS_MAP_END


/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( sbrkout )
	PORT_START("DIPS")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Language ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x01, DEF_STR( German ) )
	PORT_DIPSETTING(    0x02, DEF_STR( French ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Spanish ) )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x70, 0x00, "Extended Play" )
	/* Progressive */
	PORT_DIPSETTING(    0x10, "200" )   PORT_CONDITION("SELECT",0x03,EQUALS,0x00)
	PORT_DIPSETTING(    0x20, "400" )   PORT_CONDITION("SELECT",0x03,EQUALS,0x00)
	PORT_DIPSETTING(    0x30, "600" )   PORT_CONDITION("SELECT",0x03,EQUALS,0x00)
	PORT_DIPSETTING(    0x40, "900" )   PORT_CONDITION("SELECT",0x03,EQUALS,0x00)
	PORT_DIPSETTING(    0x50, "1200" )  PORT_CONDITION("SELECT",0x03,EQUALS,0x00)
	PORT_DIPSETTING(    0x60, "1600" )  PORT_CONDITION("SELECT",0x03,EQUALS,0x00)
	PORT_DIPSETTING(    0x70, "2000" )  PORT_CONDITION("SELECT",0x03,EQUALS,0x00)
	/* Double */
	PORT_DIPSETTING(    0x10, "200" )   PORT_CONDITION("SELECT",0x03,EQUALS,0x02)
	PORT_DIPSETTING(    0x20, "400" )   PORT_CONDITION("SELECT",0x03,EQUALS,0x02)
	PORT_DIPSETTING(    0x30, "600" )   PORT_CONDITION("SELECT",0x03,EQUALS,0x02)
	PORT_DIPSETTING(    0x40, "800" )   PORT_CONDITION("SELECT",0x03,EQUALS,0x02)
	PORT_DIPSETTING(    0x50, "1000" )  PORT_CONDITION("SELECT",0x03,EQUALS,0x02)
	PORT_DIPSETTING(    0x60, "1200" )  PORT_CONDITION("SELECT",0x03,EQUALS,0x02)
	PORT_DIPSETTING(    0x70, "1500" )  PORT_CONDITION("SELECT",0x03,EQUALS,0x02)
	/* Cavity */
	PORT_DIPSETTING(    0x10, "200" )   PORT_CONDITION("SELECT",0x03,EQUALS,0x01)
	PORT_DIPSETTING(    0x20, "300" )   PORT_CONDITION("SELECT",0x03,EQUALS,0x01)
	PORT_DIPSETTING(    0x30, "400" )   PORT_CONDITION("SELECT",0x03,EQUALS,0x01)
	PORT_DIPSETTING(    0x40, "700" )   PORT_CONDITION("SELECT",0x03,EQUALS,0x01)
	PORT_DIPSETTING(    0x50, "900" )   PORT_CONDITION("SELECT",0x03,EQUALS,0x01)
	PORT_DIPSETTING(    0x60, "1100" )  PORT_CONDITION("SELECT",0x03,EQUALS,0x01)
	PORT_DIPSETTING(    0x70, "1400" )  PORT_CONDITION("SELECT",0x03,EQUALS,0x01)
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x80, "3" )
	PORT_DIPSETTING(    0x00, "5" )

	PORT_START("COIN")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN2 )

	PORT_START("START")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SERVICE")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_TILT )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("SERVE")
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 )

	PORT_START("PADDLE")
	PORT_BIT( 0xff, 0x00, IPT_PADDLE ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_CENTERDELTA(0)

	PORT_START("SELECT")        /* IN6 - fake port, used to set the game select dial */
	PORT_CONFNAME( 0x03, 0x00, "Game Select" )
	PORT_CONFSETTING(    0x00, "Progressive" )
	PORT_CONFSETTING(    0x02, "Double" )
	PORT_CONFSETTING(    0x01, "Cavity" )
INPUT_PORTS_END

static INPUT_PORTS_START( sbrkoutc )
	PORT_INCLUDE(sbrkout)

	PORT_MODIFY("SELECT")        /* IN6 - fake port, used to set the game select dial */
	PORT_CONFNAME( 0x03, 0x00, "Game Select" )
	PORT_CONFSETTING(    0x00, "Canyon" )
	PORT_CONFSETTING(    0x02, "Vertical" )
INPUT_PORTS_END

static INPUT_PORTS_START( sbrkoutct )
	PORT_INCLUDE(sbrkout)

	PORT_START("SERVE2")
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_MODIFY("SELECT")
	PORT_CONFNAME(0x80, 0x00, "Game Select" )
	PORT_CONFSETTING( 0x00, DEF_STR( Off ) )
	PORT_CONFSETTING( 0x80, DEF_STR( On ) )
INPUT_PORTS_END

/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,
	64,
	1,
	{ 0 },
	{ 4, 5, 6, 7, 0x200*8 + 4, 0x200*8 + 5, 0x200*8 + 6, 0x200*8 + 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


static const gfx_layout balllayout =
{
	3,3,
	2,
	1,
	{ 0 },
	{ 0, 1, 2 },
	{ 0*8, 1*8, 2*8 },
	3*8
};


static GFXDECODE_START( sbrkout )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, balllayout, 0, 1 )
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_CONFIG_START( sbrkout, sbrkout_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502,MAIN_CLOCK/16)       /* 375 KHz? Should be 750KHz? */
	MCFG_CPU_PROGRAM_MAP(main_map)

	MCFG_WATCHDOG_VBLANK_INIT(8)

	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", sbrkout)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(MAIN_CLOCK/2, 384, 0, 256, 262, 0, 224)
	MCFG_SCREEN_UPDATE_DRIVER(sbrkout_state, screen_update_sbrkout)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED(sbrkoutct, sbrkout)
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(sbrkoutct_main_map)
MACHINE_CONFIG_END

/*************************************
 *
 *  ROM definitions
 *
 *************************************/

// see page 3-21 of schematics which explain the 2 versions.
// the rev 01 and rev 02 are not mentioned in the tm-118 manual despite it being a first printing
// hence they are probably prototypes. neither is dumped.

ROM_START( sbrkout ) // rev 04
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "033453.c1",    0x2800, 0x0800, CRC(a35d00e3) SHA1(53617ed1d362e82d6f45abd66056bffe23300e3b) )
	ROM_LOAD( "033454.d1",    0x3000, 0x0800, CRC(d42ea79a) SHA1(66c9b29226cde36d1ac6d1e81f34ebb5c79eded4) )
	ROM_LOAD( "033455.e1",    0x3800, 0x0800, CRC(e0a6871c) SHA1(1bdfa73d7b8d91e1c68b7847fc310cac314ee02d) )

	ROM_REGION( 0x0400, "gfx1", 0 )
	ROM_LOAD( "033280.p4",    0x0000, 0x0200, CRC(5a69ce85) SHA1(ad9078d12495c350738bdb0b1e1b6120d9e01f60) )
	ROM_LOAD( "033281.r4",    0x0200, 0x0200, CRC(066bd624) SHA1(cfb86c7013a70b8375126b23a4e66df5f3b9186b) )

	ROM_REGION( 0x0020, "gfx2", 0 )
	ROM_LOAD( "033282.k6",    0x0000, 0x0020, CRC(6228736b) SHA1(bc176261dba11521df19d545ce604f8cc294287a) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "006400.m2",    0x0000, 0x0100, CRC(b8094b4c) SHA1(82dc6799a19984f3b204ee3aeeb007e55afc8be3) )    /* sync (not used) */
	ROM_LOAD( "006401.e2",    0x0100, 0x0020, CRC(857df8db) SHA1(06313d5bde03220b2bc313d18e50e4bb1d0cfbbb) )    /* memory mapper */
ROM_END


ROM_START( sbrkout3 ) // rev 03; main cpu roms are on 1024x4bit (82s137 or equiv) proms, otherwise seems identical to rev 04
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD_NIB_HIGH( "33442-01.kl0",    0x2C00, 0x0400, CRC(fb5cb68a) SHA1(301e8e47f6a82d6c2290a890bcd5c53d61d58ff7) )
	ROM_LOAD_NIB_LOW( "33448-01.kl1",    0x2C00, 0x0400, CRC(b1d2b269) SHA1(46de71f1f9695f03465fd9b2289b5c5ffe19b3a2) )
	ROM_LOAD_NIB_HIGH( "33443-01.l0",    0x3000, 0x0400, CRC(1e7d059f) SHA1(e1831febfd26cf2560351d45f37763a7498c029e) )
	ROM_LOAD_NIB_LOW( "33449-01.l1",    0x3000, 0x0400, CRC(f936918d) SHA1(9d62fe75d39f95085a4380059c4980f3affe1bbf) )
	ROM_LOAD_NIB_HIGH( "33444-01.m0",    0x3400, 0x0400, CRC(5b7e0e3b) SHA1(4dbd62b23249fbb05e1fffe50b89a5e280a2dde9) )
	ROM_LOAD_NIB_LOW( "33450-01.m1",    0x3400, 0x0400, CRC(430cf9e8) SHA1(8e6075f12dbe0b973500d4e38e0090e40ee47260) )
	ROM_LOAD_NIB_HIGH( "33445-01.n0",    0x3800, 0x0400, CRC(cdf19919) SHA1(13623bde69e7f352beaef33524f69d74c540e1cc) )
	ROM_LOAD_NIB_LOW( "33451-01.n1",    0x3800, 0x0400, CRC(19f7c50d) SHA1(91ba9ef7ab4b200a55ae7b7979f4a01e617dd9ad) )
	ROM_LOAD_NIB_HIGH( "33446-01.p0",    0x3C00, 0x0400, CRC(9553663c) SHA1(6c28b3a11b7ff0aa224bf262c664a62166dc9cdf) )
	ROM_LOAD_NIB_LOW( "33452-01.p1",    0x3C00, 0x0400, CRC(6dc0439a) SHA1(9cc0b735935a610519eb1b53ed303223e69af0b7) )

	ROM_REGION( 0x0400, "gfx1", 0 )
	ROM_LOAD( "033280.p4",    0x0000, 0x0200, CRC(5a69ce85) SHA1(ad9078d12495c350738bdb0b1e1b6120d9e01f60) )
	ROM_LOAD( "033281.r4",    0x0200, 0x0200, CRC(066bd624) SHA1(cfb86c7013a70b8375126b23a4e66df5f3b9186b) )

	ROM_REGION( 0x0020, "gfx2", 0 )
	ROM_LOAD( "033282.k6",    0x0000, 0x0020, CRC(6228736b) SHA1(bc176261dba11521df19d545ce604f8cc294287a) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "006400.m2",    0x0000, 0x0100, CRC(b8094b4c) SHA1(82dc6799a19984f3b204ee3aeeb007e55afc8be3) )    /* sync (not used) */
	ROM_LOAD( "006401.e2",    0x0100, 0x0020, CRC(857df8db) SHA1(06313d5bde03220b2bc313d18e50e4bb1d0cfbbb) )    /* memory mapper */
ROM_END

ROM_START( sbrkoutc ) // built from original Atari source code
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "a33443.bin",   0x2800, 0x1800, CRC(bf418976) SHA1(d766e220a284a7b9caf876207e8191aff0497a03) )

	ROM_REGION( 0x0400, "gfx1", 0 )
	ROM_LOAD( "033280.p4",    0x0000, 0x0200, CRC(5a69ce85) SHA1(ad9078d12495c350738bdb0b1e1b6120d9e01f60) )
	ROM_LOAD( "033281.r4",    0x0200, 0x0200, CRC(066bd624) SHA1(cfb86c7013a70b8375126b23a4e66df5f3b9186b) )

	ROM_REGION( 0x0020, "gfx2", 0 )
	ROM_LOAD( "033282.k6",    0x0000, 0x0020, CRC(6228736b) SHA1(bc176261dba11521df19d545ce604f8cc294287a) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "006400.m2",    0x0000, 0x0100, CRC(b8094b4c) SHA1(82dc6799a19984f3b204ee3aeeb007e55afc8be3) )    /* sync (not used) */
	ROM_LOAD( "006401.e2",    0x0100, 0x0020, CRC(857df8db) SHA1(06313d5bde03220b2bc313d18e50e4bb1d0cfbbb) )    /* memory mapper */
ROM_END

ROM_START( sbrkoutct ) // built from original Atari source code
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "034555-01.c1",   0x2800, 0x0800, CRC(2da82521) SHA1(1f53e549676052647486cea6738c5c7a45133538) )
	ROM_LOAD( "034556-01.d11",  0x3000, 0x0800, CRC(5a6497ae) SHA1(96c2a136fb1e649e2db17bcb12bdc2a8d250a63e) )
	ROM_LOAD( "034557-01.ef1",  0x3800, 0x0800, CRC(b6b3b07b) SHA1(c4d2cdcca89c2944afd4a4ed0bb5003b3eca4c7e) )

	ROM_REGION( 0x0400, "gfx1", 0 )
	ROM_LOAD( "034559-01.r4",    0x0000, 0x0200, CRC(84368539) SHA1(50b2c3f443346e3a355492ed1f7ec0a8cc6364d4) )
	ROM_LOAD( "034558-01.p4",    0x0200, 0x0200, CRC(cc0f81f2) SHA1(a2180280991c9cf43f4e941d9ba4fe5654d1af65) )

	ROM_REGION( 0x0020, "gfx2", 0 )
	ROM_LOAD( "033282.k6",    0x0000, 0x0020, CRC(6228736b) SHA1(bc176261dba11521df19d545ce604f8cc294287a) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "006400.m2",    0x0000, 0x0100, CRC(b8094b4c) SHA1(82dc6799a19984f3b204ee3aeeb007e55afc8be3) )    /* sync (not used) */
	ROM_LOAD( "006401.e2",    0x0100, 0x0020, CRC(857df8db) SHA1(06313d5bde03220b2bc313d18e50e4bb1d0cfbbb) )    /* memory mapper */
ROM_END

/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAMEL( 1978, sbrkout,   0,       sbrkout,   sbrkout,   driver_device, 0, ROT270, "Atari", "Super Breakout (rev 04)", MACHINE_SUPPORTS_SAVE, layout_sbrkout )
GAMEL( 1978, sbrkout3,  sbrkout, sbrkout,   sbrkout,   driver_device, 0, ROT270, "Atari", "Super Breakout (rev 03)", MACHINE_SUPPORTS_SAVE, layout_sbrkout )
GAMEL( 1978, sbrkoutc,  sbrkout, sbrkout,   sbrkoutc,  driver_device, 0, ROT270, "Atari", "Super Breakout (Canyon and Vertical Breakout, prototype)", MACHINE_SUPPORTS_SAVE, layout_sbrkout )
GAMEL( 1978, sbrkoutct, sbrkout, sbrkoutct, sbrkoutct, driver_device, 0, ROT270, "Atari", "Super Breakout (Cocktail, prototype)", MACHINE_SUPPORTS_SAVE, layout_sbrkout )
