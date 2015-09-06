// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller
/***************************************************************************
  Lazer Command memory map (preliminary)

  driver by Juergen Buchmueller

  0000-0bff ROM

  1c00-1c1f RAM     CPU scratch pad is first 32 bytes of video RAM(not displayed)

  1c20-1eff RAM     video buffer
            xxxx    D0 - D5 character select
                    D6      horz line below character (row #9)
                    D7      vert line right of character (bit #0)

  1f00-1f03         R/W hardware

            1f00 W     audio channels
                    D4 gun fire
                    D5 explosion
                    D6 tank engine
                    D7 running man
            1f00 R     player 1 joystick
                    D0 up
                    D1 down
                    D2 right
                    D3 left

            1f01 W  D0 - D7 marker y position
            1f01 R     player 2 joystick
                    D0 up
                    D1 down
                    D2 right
                    D3 left

            1f02 W  D0 - D7 marker x position
            1f02 R     player 1 + 2 buttons
                    D0 button 1 player 2
                    D1 button 1 player 1
                    D2 button 2 player 2
                    D3 button 2 player 1

            1f03 W      attract mode
                    D0 toggle on attract mode
                       (attract mode switched off by coin detected)
                    D4 clear coin detected toggle
            1f03 R      coinage, coin detected and start buttons
                    D0 coin 1/2 (DIP switch 4)
                    D1 start 'expert'
                    D2 start 'novice'
                    D3 coin detected

  1f04-1f07         Read only hardware

            1f04 R     vertical scan counter
                    D0 60 Hz
                    D1 120 Hz
                    D2 240 Hz
                    D3 480 Hz

            1f05 R     vertical scan counter
                    D0 7.860 KHz
                    D1 3.960 KHz
                    D2 1.980 KHz
                    D3 960 Hz

            1f06 R  D0 - D7 readback of marker x position

            1f07 R  D0 - D7 readback of marker y position

  I/O ports:

  'data'         R          game time
                    D0 - D1 60,90,120,180 seconds (DIP switch 1 - 2)

***************************************************************************/

/***************************************************************************
  Meadows Lanes memory map (preliminary)

  0000-0bff ROM

  0c00-0c1f RAM     CPU scratch pad is first 32 bytes of video RAM(not displayed)

  0c20-0eff RAM     video buffer
            xxxx    D0 - D5 character select
                    D6      horz line below character (row #9)
                    D7      vert line right of character (bit #0)

  1000-17ff ROM

  1f00-1f03         R/W hardware

            1f00 W     audio control bits
                    D0 - D3 not used
                    D4 bowl and hit
                    D5 hit
                    D6 - D7 not used
            1f00 R     bowl ball
                    D0 fast
                    D1 slow
                       joystick
                    D2 right
                    D3 left

            1f01 W  D0 - D7 marker y position
            1f01 R     hook control
                    D0 left
                    D1 right
                    D2 - D3 not used

            1f02 W  D0 - D7 marker x position
            1f02 R  D0 - D3 not used

            1f03 W      attract mode
                    D0 toggle on attract mode
                       (attract mode switched off by coin detected)
                    D4 clear coin detected toggle
                    D5 can be jumpered to control inverse video
                    D6 - D7 not used
            1f03 R      coinage, coin detected and start buttons
                    D0 coin 1/2 (DIP switch 4)
                    D1 start
                    D2 not used
                    D3 coin detected

  1f04-1f07         Read only hardware

            1f04 R     vertical scan counter
                    D0 60 Hz
                    D1 120 Hz
                    D2 240 Hz
                    D3 480 Hz

            1f05 R     vertical scan counter
                    D0 7.860 KHz
                    D1 3.960 KHz
                    D2 1.980 KHz
                    D3 960 Hz

            1f06 R  D0 - D7 readback of marker x position

            1f07 R  D0 - D7 readback of marker y position

  I/O ports:

  'data'         R          game time
                    D0 time on     (DIP switch 1)
                    D1 3,5 seconds (DIP switch 2)

***************************************************************************/

/***************************************************************************
  Bigfoot Bonkers memory map (preliminary)

  driver by Juergen Buchmueller

  0000-07ff ROM

  1c00-1c1f RAM     CPU scratch pad is first 32 bytes of video RAM(not displayed)

  1c20-1eff RAM     video buffer
            xxxx    D0 - D5 character select
                    D6      horz line below character (row #9)
                    D7      vert line right of character (bit #0)

  1f00-1f03         R/W hardware

            1f00 W     audio channels
                    D4 unused
                    D5 tone 1
                    D6 tone 2
                    D7 unused
            1f00 R     player 1 joystick
                    D0 up
                    D1 down
                    D2 right
                    D3 left

            1f01 W  D0 - D7 unused
            1f01 R     player 2 joystick
                    D0 up
                    D1 down
                    D2 right
                    D3 left

            1f02 W  D0 - D7 unused
            1f02 R     player 1 + 2 buttons
                    D0 unused
                    D1 unused
                    D2 unused
                    D3 unused

            1f03 W      attract mode
                    D0 toggle on attract mode
                       (attract mode switched off by coin detected)
                    D4 clear coin detected toggle
            1f03 R      coinage, coin detected and start buttons
                    D0 coin 1/2 (DIP switch 4)
                    D1 start
                    D2 start
                    D3 coin detected

  1f04-1f07         Read only hardware

            1f04 R     vertical scan counter
                    D0 60 Hz
                    D1 120 Hz
                    D2 240 Hz
                    D3 480 Hz

            1f05 R     vertical scan counter
                    D0 7.860 KHz
                    D1 3.960 KHz
                    D2 1.980 KHz
                    D3 960 Hz

            1f06 R  D0 - D7 unused

            1f07 R  D0 - D7 unused

***************************************************************************/

/***************************************************************************

  TODO
  - how does video invert exactly work? the hardware can control it,
    for example in bbonk death sequence
  - bbonk video on youtube shows a 1-pixel gap horizontally between tiles,
    maybe the characters are 9*10 or it differs per game
  - improve sound emulation

***************************************************************************/

#include "emu.h"
#include "includes/lazercmd.h"

// color overlays, bbonk does not have an overlay
#include "lazercmd.lh"
#include "medlanes.lh"

#define MASTER_CLOCK XTAL_8MHz

/*************************************************************
 * Interrupt for the cpu
 * Fake something toggling the sense input line of the S2650
 * The rate should be at about 1 Hz
 *************************************************************/

TIMER_DEVICE_CALLBACK_MEMBER(lazercmd_state::lazercmd_timer)
{
	int scanline = param;

	if((scanline % 2) == 1)
		return;

	if (++m_timer_count >= 64 * 128)
	{
		m_timer_count = 0;
		m_sense_state ^= 1;
		m_maincpu->write_sense(m_sense_state ? ASSERT_LINE : CLEAR_LINE);
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(lazercmd_state::bbonk_timer)
{
	int scanline = param;

	if((scanline % 2) == 1)
		return;

	if (++m_timer_count >= 64 * 128)
		m_timer_count = 0;
}

/*************************************************************
 *
 * IO port read/write
 *
 *************************************************************/

/* triggered by WRTC,r opcode */
WRITE8_MEMBER(lazercmd_state::lazercmd_ctrl_port_w)
{
}

/* triggered by REDC,r opcode */
READ8_MEMBER(lazercmd_state::lazercmd_ctrl_port_r)
{
	UINT8 data = 0;
	return data;
}

/* triggered by WRTD,r opcode */
WRITE8_MEMBER(lazercmd_state::lazercmd_data_port_w)
{
}

/* triggered by REDD,r opcode */
READ8_MEMBER(lazercmd_state::lazercmd_data_port_r)
{
	UINT8 data = ioport("DSW")->read() & 0x0f;
	return data;
}

WRITE8_MEMBER(lazercmd_state::lazercmd_hardware_w)
{
	switch (offset)
	{
		case 0: /* audio channels */
			m_dac_data = (data & 0x80) ^ ((data & 0x40) << 1) ^ ((data & 0x20) << 2) ^ ((data & 0x10) << 3);
			if (m_dac_data)
				m_dac->write_unsigned8(0xff);
			else
				m_dac->write_unsigned8(0);
			break;
		case 1: /* marker Y position */
			m_marker_y = data;
			break;
		case 2: /* marker X position */
			m_marker_x = data;
			break;
		case 3: /* D4 clears coin detected and D0 toggles on attract mode */
			m_attract = data;
			break;
	}
}

WRITE8_MEMBER(lazercmd_state::medlanes_hardware_w)
{
	switch (offset)
	{
		case 0: /* audio control */
			/* bits 4 and 5 are used to control a sound board */
			/* these could be used to control sound samples */
			/* at the moment they are routed through the dac */
			m_dac_data = ((data & 0x20) << 2) ^ ((data & 0x10) << 3);
			if (m_dac_data)
				m_dac->write_unsigned8(0xff);
			else
				m_dac->write_unsigned8(0);
			break;
		case 1: /* marker Y position */
			m_marker_y = data;
			break;
		case 2: /* marker X position */
			m_marker_x = data;
			break;
		case 3: /* D4 clears coin detected and D0 toggles on attract mode */
			m_attract = data;
			break;
	}
}

WRITE8_MEMBER(lazercmd_state::bbonk_hardware_w)
{
	switch (offset)
	{
		case 0: /* audio control */
			/* bits 4 and 5 are used to control a sound board */
			/* these could be used to control sound samples */
			/* at the moment they are routed through the dac */
			m_dac_data = ((data & 0x20) << 2) ^ ((data & 0x10) << 3);
			if (m_dac_data)
				m_dac->write_unsigned8(0xff);
			else
				m_dac->write_unsigned8(0);
			break;
		case 3: /* D5 inverts video?, D4 clears coin detected and D0 toggles on attract mode */
			m_attract = data;
			break;
	}
}

READ8_MEMBER(lazercmd_state::lazercmd_hardware_r)
{
	UINT8 data = 0;

	switch (offset)
	{
		case 0: /* player 1 joysticks */
			data = ioport("IN0")->read();
			break;
		case 1: /* player 2 joysticks */
			data = ioport("IN1")->read();
			break;
		case 2: /* player 1 + 2 buttons */
			data = ioport("IN3")->read();
			break;
		case 3: /* coin slot + start buttons */
			data = ioport("IN2")->read();
			break;
		case 4: /* vertical scan counter */
			data = ((m_timer_count & 0x10) >> 1) | ((m_timer_count & 0x20) >> 3) | ((m_timer_count & 0x40) >> 5) | ((m_timer_count & 0x80) >> 7);
			break;
		case 5: /* vertical scan counter */
			data = m_timer_count & 0x0f;
			break;
		case 6: /* 1f02 readback */
			data = m_marker_x;
			break;
		case 7: /* 1f01 readback */
			data = m_marker_y;
			break;
	}
	return data;
}


/*************************************************************
 *
 * Memory maps
 *
 *************************************************************/

static ADDRESS_MAP_START( lazercmd_map, AS_PROGRAM, 8, lazercmd_state )
	AM_RANGE(0x0000, 0x0bff) AM_ROM
	AM_RANGE(0x1c00, 0x1c1f) AM_RAM
	AM_RANGE(0x1c20, 0x1eff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x1f00, 0x1f03) AM_WRITE(lazercmd_hardware_w)
	AM_RANGE(0x1f00, 0x1f07) AM_READ(lazercmd_hardware_r)
ADDRESS_MAP_END


static ADDRESS_MAP_START( medlanes_map, AS_PROGRAM, 8, lazercmd_state )
	AM_RANGE(0x0000, 0x0bff) AM_ROM
	AM_RANGE(0x1000, 0x17ff) AM_ROM
	AM_RANGE(0x1c00, 0x1c1f) AM_RAM
	AM_RANGE(0x1c20, 0x1eff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x1f00, 0x1f03) AM_WRITE(medlanes_hardware_w)
	AM_RANGE(0x1f00, 0x1f07) AM_READ(lazercmd_hardware_r)
ADDRESS_MAP_END


static ADDRESS_MAP_START( bbonk_map, AS_PROGRAM, 8, lazercmd_state )
	AM_RANGE(0x0000, 0x0bff) AM_ROM
	AM_RANGE(0x1c00, 0x1c1f) AM_RAM
	AM_RANGE(0x1c20, 0x1eff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x1f00, 0x1f03) AM_WRITE(bbonk_hardware_w)
	AM_RANGE(0x1f00, 0x1f07) AM_READ(lazercmd_hardware_r)
ADDRESS_MAP_END


static ADDRESS_MAP_START( lazercmd_portmap, AS_IO, 8, lazercmd_state )
	AM_RANGE(S2650_CTRL_PORT, S2650_CTRL_PORT) AM_READWRITE(lazercmd_ctrl_port_r, lazercmd_ctrl_port_w)
	AM_RANGE(S2650_DATA_PORT, S2650_DATA_PORT) AM_READWRITE(lazercmd_data_port_r, lazercmd_data_port_w)
ADDRESS_MAP_END



static INPUT_PORTS_START( lazercmd )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Game_Time ) ) PORT_DIPLOCATION("SW1:!1,!2")
	PORT_DIPSETTING(    0x00, "60 seconds" )
	PORT_DIPSETTING(    0x01, "90 seconds" )
	PORT_DIPSETTING(    0x02, "120 seconds" )
	PORT_DIPSETTING(    0x03, "180 seconds" )
	PORT_BIT( 0x98, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x20, 0x20, "Video Invert" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Marker Size" ) // jumper
	PORT_DIPSETTING(    0x00, "Small" )
	PORT_DIPSETTING(    0x40, "Large" )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 ) PORT_NAME("Start Expert")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start Novice")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( medlanes )
	PORT_START("IN0")
	// note: real cpanel has 2 buttons for left/right, no joystick. 6 buttons total (excluding start)
	// layout is: move left, move right  -  throw fast, throw slow  -  hook right, hook left
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Throw Fast")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Throw Slow")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Hook Left")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Hook Right")
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x03, "Throw Time" ) PORT_DIPLOCATION("SW1:!1,!2")
	PORT_DIPSETTING(    0x01, "3 seconds" )
	PORT_DIPSETTING(    0x03, "5 seconds" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x02, DEF_STR( Off ) ) // dupe
	PORT_BIT( 0x9C, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x20, 0x00, "Video Invert" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Marker Size" ) // jumper
	PORT_DIPSETTING(    0x00, "Small" )
	PORT_DIPSETTING(    0x40, "Large" )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0xf4, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( bbonk )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x03, "Games to win" ) PORT_DIPLOCATION("SW1:!1,!2")
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_BIT( 0x9C, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x20, 0x00, "Video Invert" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED ) // marker is not used in this game

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN3")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, "Partial" )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8, 10,                  /* 8*10 characters */
	4*64,                   /* 4 * 64 characters */
	1,                      /* 1 bit per pixel */
	{ 0 },                  /* no bitplanes */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8 },
	10*8                    /* every char takes 10 bytes */
};

static GFXDECODE_START( lazercmd )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 2 )
GFXDECODE_END

PALETTE_INIT_MEMBER(lazercmd_state, lazercmd)
{
	palette.set_pen_color(0, rgb_t(0xb0, 0xb0, 0xb0)); /* white */
	palette.set_pen_color(1, rgb_t(0x00, 0x00, 0x00)); /* black */

	palette.set_pen_color(2, rgb_t(0x00, 0x00, 0x00)); /* black */
	palette.set_pen_color(3, rgb_t(0xb0, 0xb0, 0xb0)); /* white */

	palette.set_pen_color(4, rgb_t(0xff, 0xff, 0xff)); /* bright white */
}


void lazercmd_state::machine_start()
{
	save_item(NAME(m_marker_x));
	save_item(NAME(m_marker_y));
	save_item(NAME(m_timer_count));
	save_item(NAME(m_sense_state));
	save_item(NAME(m_dac_data));
	save_item(NAME(m_attract));
}

void lazercmd_state::machine_reset()
{
	m_marker_x = 0;
	m_marker_y = 0;
	m_timer_count = 0;
	m_sense_state = 0;
	m_dac_data = 0;
	m_attract = 0;
}


static MACHINE_CONFIG_START( lazercmd, lazercmd_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", S2650, MASTER_CLOCK/12) /* 666 kHz? */
/*  Main Clock is 8MHz divided by 12
    but memory and IO access is only possible
    within the line and frame blanking period
    thus requiring an extra loading of approx 3-5 */
	MCFG_CPU_PROGRAM_MAP(lazercmd_map)
	MCFG_CPU_IO_MAP(lazercmd_portmap)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", lazercmd_state, lazercmd_timer, "screen", 0, 1)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(HORZ_RES * HORZ_CHR, VERT_RES * VERT_CHR + 16)
	MCFG_SCREEN_VISIBLE_AREA(0 * HORZ_CHR, HORZ_RES * HORZ_CHR - 1, 0 * VERT_CHR, (VERT_RES - 1) * VERT_CHR - 1)
	MCFG_SCREEN_UPDATE_DRIVER(lazercmd_state, screen_update_lazercmd)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", lazercmd)
	MCFG_PALETTE_ADD("palette", 5)
	MCFG_PALETTE_INIT_OWNER(lazercmd_state, lazercmd)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( medlanes, lazercmd_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", S2650, MASTER_CLOCK/12) /* 666 kHz */
/*  Main Clock is 8MHz divided by 12
    but memory and IO access is only possible
    within the line and frame blanking period
    thus requiring an extra loading of approx 3-5 */
	MCFG_CPU_PROGRAM_MAP(medlanes_map)
	MCFG_CPU_IO_MAP(lazercmd_portmap)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", lazercmd_state, lazercmd_timer, "screen", 0, 1)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(HORZ_RES * HORZ_CHR, VERT_RES * VERT_CHR)
	MCFG_SCREEN_VISIBLE_AREA(0 * HORZ_CHR, HORZ_RES * HORZ_CHR - 1, 0 * VERT_CHR, VERT_RES * VERT_CHR - 1)
	MCFG_SCREEN_UPDATE_DRIVER(lazercmd_state, screen_update_lazercmd)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", lazercmd)
	MCFG_PALETTE_ADD("palette", 5)
	MCFG_PALETTE_INIT_OWNER(lazercmd_state, lazercmd)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( bbonk, lazercmd_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", S2650, MASTER_CLOCK/12) /* 666 kHz */
/*  Main Clock is 8MHz divided by 12
    but memory and IO access is only possible
    within the line and frame blanking period
    thus requiring an extra loading of approx 3-5 */
	MCFG_CPU_PROGRAM_MAP(bbonk_map)
	MCFG_CPU_IO_MAP(lazercmd_portmap)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", lazercmd_state, bbonk_timer, "screen", 0, 1)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(HORZ_RES * HORZ_CHR, VERT_RES * VERT_CHR)
	MCFG_SCREEN_VISIBLE_AREA(0 * HORZ_CHR, HORZ_RES * HORZ_CHR - 1, 0 * VERT_CHR, (VERT_RES - 1) * VERT_CHR - 1)
	MCFG_SCREEN_UPDATE_DRIVER(lazercmd_state, screen_update_lazercmd)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", lazercmd)
	MCFG_PALETTE_ADD("palette", 5)
	MCFG_PALETTE_INIT_OWNER(lazercmd_state, lazercmd)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( lazercmd )
	ROM_REGION( 0x0c00, "maincpu", ROMREGION_INVERT ) /* 32K cpu, 4K for ROM/RAM */
	ROM_LOAD_NIB_HIGH( "lc.e5", 0x0000, 0x0400, CRC(56dc7a40) SHA1(1324d5d6a44d7314723a0b5745d89f8e27f49d25) )
	ROM_LOAD_NIB_LOW(  "lc.f5", 0x0000, 0x0400, CRC(fc5b38a4) SHA1(bff670d7b78c6b9324d2bf4b2d8a4f9dbfe82158) )
	ROM_LOAD_NIB_HIGH( "lc.e6", 0x0400, 0x0400, CRC(b1ef0aa2) SHA1(3edeaa4d4f4e18536066898284d430a1ac00512e) )
	ROM_LOAD_NIB_LOW(  "lc.f6", 0x0400, 0x0400, CRC(26eaee21) SHA1(9c0a4a22abb0b0466378f067ef52a45f86cc4369) )
	ROM_LOAD_NIB_HIGH( "lc.e7", 0x0800, 0x0400, CRC(8e6ffc97) SHA1(d5243ce88585db91573b6586d3d47d13b5b473c8) )
	ROM_LOAD_NIB_LOW(  "lc.f7", 0x0800, 0x0400, CRC(9ec3534d) SHA1(98f15c5828ad2743bf205f71b8e69abd4db78a58) )

	ROM_REGION( 0x0c00, "gfx1", 0 )
	ROM_LOAD( "lc.b8", 0x0a00, 0x0200, CRC(6d708edd) SHA1(85a45a292eb7bca288b06a118658bf754f828a92) )
ROM_END

ROM_START( medlanes )
	ROM_REGION( 0x1800, "maincpu", ROMREGION_INVERT ) /* 32K cpu, 4K for ROM/RAM */
	ROM_LOAD_NIB_HIGH( "medlanes.2a", 0x0000, 0x0400, CRC(9c77566a) SHA1(60e1820012b47da8b86d54f00b6f60d2d0123745) )
	ROM_LOAD_NIB_LOW(  "medlanes.3a", 0x0000, 0x0400, CRC(22bc56a6) SHA1(7444170c19274d9d889df61796e6f61af2361f3e) )
	ROM_LOAD_NIB_HIGH( "medlanes.2b", 0x0400, 0x0400, CRC(7841b1a9) SHA1(80621d30995dad42ae44c62494922ca8b75415cf) )
	ROM_LOAD_NIB_LOW(  "medlanes.3b", 0x0400, 0x0400, CRC(6616dbef) SHA1(9506177315883b7d87a9bfada712ddeea12fd446) )
	ROM_LOAD_NIB_HIGH( "medlanes.2c", 0x0800, 0x0400, CRC(a359b5b8) SHA1(dbc3c286951c50e3465132fc0d6054f06026425d) )
	ROM_LOAD_NIB_LOW(  "medlanes.3c", 0x0800, 0x0400, CRC(b3db0f3d) SHA1(57c28a54f7a1f17df3a24b61dd0cf37f9f6bc7d8) )
	ROM_LOAD_NIB_HIGH( "medlanes.1a", 0x1000, 0x0400, CRC(0d57c596) SHA1(f3ce4802fc777c57f75fe691c93b7062903bdf06) )
	ROM_LOAD_NIB_LOW(  "medlanes.4a", 0x1000, 0x0400, CRC(30d495e9) SHA1(4f2414bf60ef91093bedf5e9ae16833e9e135aa7) )
	ROM_LOAD_NIB_HIGH( "medlanes.1b", 0x1400, 0x0400, CRC(1d451630) SHA1(bf9de3096e98685355c906ab7e1dc2628dce79d6) )
	ROM_LOAD_NIB_LOW(  "medlanes.4b", 0x1400, 0x0400, CRC(a4abb5db) SHA1(a20da872b0f7d6b16b9551233af4269db9d1b55f) )

	ROM_REGION( 0x0c00, "gfx1", 0 )
	ROM_LOAD( "medlanes.8b", 0x0a00, 0x0200, CRC(44e5de8f) SHA1(fc797fa137f0c11a15caf9c0013aac668fd69a3c) )
ROM_END


ROM_START( bbonk )
	ROM_REGION( 0x0c00, "maincpu", ROMREGION_INVERT ) /* 32K cpu, 4K for ROM/RAM */
	ROM_LOAD_NIB_HIGH( "bbonk.e5", 0x0000, 0x0400, CRC(d032baa0) SHA1(09cba16f6a2b7d8a8c501db639bd5eeefb63dc0f) )
	ROM_LOAD_NIB_LOW(  "bbonk.f5", 0x0000, 0x0400, CRC(748e8c7f) SHA1(99e4e182ee41c246e31f656411a9f09d7b617f92) )
	ROM_LOAD_NIB_HIGH( "bbonk.e6", 0x0400, 0x0400, CRC(71df0e25) SHA1(c2f78490816add1296923861a89df15be9822fed) )
	ROM_LOAD_NIB_LOW(  "bbonk.f6", 0x0400, 0x0400, CRC(5ce183ed) SHA1(7c78dfa463a37605e8423104426af2f5906fae24) )

	ROM_REGION( 0x0c00, "gfx1", 0 )
	ROM_LOAD( "bbonk.b8", 0x0a00, 0x0200, CRC(5ac34260) SHA1(7c2b1e378d2b9fed27117f9adab1381507f5d554) )
ROM_END


DRIVER_INIT_MEMBER(lazercmd_state,lazercmd)
{
	int i, y;
	UINT8 *gfx = memregion("gfx1")->base();

/******************************************************************
 * To show the maze bit #6 and #7 of the video ram are used.
 * Bit #7: add a vertical line to the right of the character
 * Bit #6: add a horizontal line below the character
 * The video logic generates 10 lines per character row, but the
 * character generator only contains 8 rows, so we expand the
 * font to 8x10.
 ******************************************************************/
	for (i = 0; i < 0x40; i++)
	{
		UINT8 *d = &gfx[0 * 64 * 10 + i * VERT_CHR];
		UINT8 *s = &gfx[4 * 64 * 10 + i * VERT_FNT];

		for (y = 0; y < VERT_CHR; y++)
		{
			d[0 * 64 * 10] = (y < VERT_FNT) ? *s++ : 0xff;
			d[1 * 64 * 10] = (y == VERT_CHR - 1) ? 0 : *d;
			d[2 * 64 * 10] = *d & 0xfe;
			d[3 * 64 * 10] = (y == VERT_CHR - 1) ? 0 : *d & 0xfe;
			d++;
		}
	}
}



GAMEL( 1976, lazercmd, 0, lazercmd, lazercmd, lazercmd_state, lazercmd, ROT0, "Meadows Games, Inc.", "Lazer Command", MACHINE_SUPPORTS_SAVE, layout_lazercmd )
GAMEL( 1977, medlanes, 0, medlanes, medlanes, lazercmd_state, lazercmd, ROT0, "Meadows Games, Inc.", "Meadows Lanes", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_medlanes )
GAME ( 1976, bbonk,    0, bbonk,    bbonk,    lazercmd_state, lazercmd, ROT0, "Meadows Games, Inc.", "Bigfoot Bonkers", MACHINE_SUPPORTS_SAVE )
