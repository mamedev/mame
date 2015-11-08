// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/***************************************************************************

    Atari Sprint 2 hardware

    driver by Mike Balfour

    Games supported:
        * Sprint 1
        * Sprint 2
        * Dominos

    All three games run on the same PCB but with minor modifications (some
    chips removed, some wire-wrap connections added).

    If you have any questions about how this driver works, don't hesitate to
    ask.  - Mike Balfour (mab22@po.cwru.edu)

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "includes/sprint2.h"
#include "sound/discrete.h"

#define MACHINE_IS_SPRINT1   (m_game == 1)
#define MACHINE_IS_SPRINT2   (m_game == 2)
#define MACHINE_IS_DOMINOS   (m_game == 3)

DRIVER_INIT_MEMBER(sprint2_state,sprint1)
{
	m_game = 1;
}
DRIVER_INIT_MEMBER(sprint2_state,sprint2)
{
	m_game = 2;
}
DRIVER_INIT_MEMBER(sprint2_state,dominos)
{
	m_game = 3;
}

DRIVER_INIT_MEMBER(sprint2_state,dominos4)
{
	m_game = 3;
	m_maincpu->space(AS_PROGRAM).install_read_port(0x0880, 0x0880, "SELFTTEST");
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x0c50, 0x0c5f, write8_delegate(FUNC(sprint2_state::dominos4_lamp3_w),this));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x0c60, 0x0c6f, write8_delegate(FUNC(sprint2_state::dominos4_lamp4_w),this));
}

int sprint2_state::service_mode()
{
	UINT8 v = ioport("INB")->read();

	if (MACHINE_IS_SPRINT1)
	{
		return (v & 0x10) == 0;
	}
	if (MACHINE_IS_SPRINT2)
	{
		return (v & 0x04) == 0;
	}
	if (MACHINE_IS_DOMINOS)
	{
		return (v & 0x40) == 0;
	}

	return 0;
}


INTERRUPT_GEN_MEMBER(sprint2_state::sprint2)
{
	/* handle steering wheels */

	if (MACHINE_IS_SPRINT1 || MACHINE_IS_SPRINT2)
	{
		int i;

		for (i = 0; i < 2; i++)
		{
			signed char delta = ioport(i ? "DIAL_P2" : "DIAL_P1")->read() - m_dial[i];

			if (delta < 0)
			{
				m_steering[i] = 0x00;
			}
			if (delta > 0)
			{
				m_steering[i] = 0x40;
			}

			m_dial[i] += delta;

			switch (ioport(i ? "GEAR_P2" : "GEAR_P1")->read() & 15)
			{
			case 1: m_gear[i] = 1; break;
			case 2: m_gear[i] = 2; break;
			case 4: m_gear[i] = 3; break;
			case 8: m_gear[i] = 4; break;
			}
			output_set_value("P1gear", m_gear[0]);
			output_set_value("P2gear", m_gear[1]);
		}
	}

	address_space &space = m_maincpu->space(AS_PROGRAM);
	m_discrete->write(space, SPRINT2_MOTORSND1_DATA, m_video_ram[0x394] & 15); // also DOMINOS_FREQ_DATA
	m_discrete->write(space, SPRINT2_MOTORSND2_DATA, m_video_ram[0x395] & 15);
	m_discrete->write(space, SPRINT2_CRASHSND_DATA, m_video_ram[0x396] & 15);  // also DOMINOS_AMP_DATA

	/* interrupts and watchdog are disabled during service mode */

	machine().watchdog_enable(!service_mode());

	if (!service_mode())
		device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}


READ8_MEMBER(sprint2_state::sprint2_wram_r)
{
	return m_video_ram[0x380 + offset % 0x80];
}


READ8_MEMBER(sprint2_state::sprint2_dip_r)
{
	return (ioport("DSW")->read() << (2 * ((offset & 3) ^ 3))) & 0xc0;
}


READ8_MEMBER(sprint2_state::sprint2_input_A_r)
{
	UINT8 val = ioport("INA")->read();

	if (m_game == 2)// (MACHINE_IS_SPRINT2)
	{
		if (m_gear[0] == 1) val &= ~0x01;
		if (m_gear[1] == 1) val &= ~0x02;
		if (m_gear[0] == 2) val &= ~0x04;
		if (m_gear[1] == 2) val &= ~0x08;
		if (m_gear[0] == 3) val &= ~0x10;
		if (m_gear[1] == 3) val &= ~0x20;
	}

	return (val << (offset ^ 7)) & 0x80;
}


READ8_MEMBER(sprint2_state::sprint2_input_B_r)
{
	UINT8 val = ioport("INB")->read();

	if (m_game == 1) // (MACHINE_IS_SPRINT1)
	{
		if (m_gear[0] == 1) val &= ~0x01;
		if (m_gear[0] == 2) val &= ~0x02;
		if (m_gear[0] == 3) val &= ~0x04;
	}

	return (val << (offset ^ 7)) & 0x80;
}


READ8_MEMBER(sprint2_state::sprint2_sync_r)
{
	UINT8 val = 0;

	if (m_attract != 0)
		val |= 0x10;

	if (m_screen->vpos() == 261)
		val |= 0x20; /* VRESET */

	if (m_screen->vpos() >= 224)
		val |= 0x40; /* VBLANK */

	if (m_screen->vpos() >= 131)
		val |= 0x80; /* 60 Hz? */

	return val;
}


READ8_MEMBER(sprint2_state::sprint2_steering1_r)
{
	return m_steering[0];
}
READ8_MEMBER(sprint2_state::sprint2_steering2_r)
{
	return m_steering[1];
}


WRITE8_MEMBER(sprint2_state::sprint2_steering_reset1_w)
{
	m_steering[0] |= 0x80;
}
WRITE8_MEMBER(sprint2_state::sprint2_steering_reset2_w)
{
	m_steering[1] |= 0x80;
}


WRITE8_MEMBER(sprint2_state::sprint2_wram_w)
{
	m_video_ram[0x380 + offset % 0x80] = data;
}


WRITE8_MEMBER(sprint2_state::sprint2_attract_w)
{
	m_attract = offset & 1;

	// also DOMINOS_ATTRACT_EN
	m_discrete->write(space, SPRINT2_ATTRACT_EN, m_attract);
}


WRITE8_MEMBER(sprint2_state::sprint2_noise_reset_w)
{
	m_discrete->write(space, SPRINT2_NOISE_RESET, 0);
}


WRITE8_MEMBER(sprint2_state::sprint2_skid1_w)
{
	// also DOMINOS_TUMBLE_EN
	m_discrete->write(space, SPRINT2_SKIDSND1_EN, offset & 1);
}

WRITE8_MEMBER(sprint2_state::sprint2_skid2_w)
{
	m_discrete->write(space, SPRINT2_SKIDSND2_EN, offset & 1);
}


WRITE8_MEMBER(sprint2_state::sprint2_lamp1_w)
{
	set_led_status(machine(), 0, offset & 1);
}
WRITE8_MEMBER(sprint2_state::sprint2_lamp2_w)
{
	set_led_status(machine(), 1, offset & 1);
}

WRITE8_MEMBER(sprint2_state::dominos4_lamp3_w)
{
	set_led_status(machine(), 2, offset & 1);
}
WRITE8_MEMBER(sprint2_state::dominos4_lamp4_w)
{
	set_led_status(machine(), 3, offset & 1);
}

static ADDRESS_MAP_START( sprint2_map, AS_PROGRAM, 8, sprint2_state )
	ADDRESS_MAP_GLOBAL_MASK(0x3fff)
	AM_RANGE(0x0000, 0x03ff) AM_READWRITE(sprint2_wram_r,sprint2_wram_w)
	AM_RANGE(0x0400, 0x07ff) AM_RAM_WRITE(sprint2_video_ram_w) AM_SHARE("video_ram")
	AM_RANGE(0x0818, 0x081f) AM_READ(sprint2_input_A_r)
	AM_RANGE(0x0828, 0x082f) AM_READ(sprint2_input_B_r)
	AM_RANGE(0x0830, 0x0837) AM_READ(sprint2_dip_r)
	AM_RANGE(0x0840, 0x087f) AM_READ_PORT("COIN")
	AM_RANGE(0x0880, 0x08bf) AM_READ(sprint2_steering1_r)
	AM_RANGE(0x08c0, 0x08ff) AM_READ(sprint2_steering2_r)
	AM_RANGE(0x0c00, 0x0fff) AM_READ(sprint2_sync_r)
	AM_RANGE(0x0c00, 0x0c0f) AM_WRITE(sprint2_attract_w)
	AM_RANGE(0x0c10, 0x0c1f) AM_WRITE(sprint2_skid1_w)
	AM_RANGE(0x0c20, 0x0c2f) AM_WRITE(sprint2_skid2_w)
	AM_RANGE(0x0c30, 0x0c3f) AM_WRITE(sprint2_lamp1_w)
	AM_RANGE(0x0c40, 0x0c4f) AM_WRITE(sprint2_lamp2_w)
	AM_RANGE(0x0c60, 0x0c6f) AM_WRITENOP /* SPARE */
	AM_RANGE(0x0c80, 0x0cff) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x0d00, 0x0d7f) AM_WRITE(sprint2_collision_reset1_w)
	AM_RANGE(0x0d80, 0x0dff) AM_WRITE(sprint2_collision_reset2_w)
	AM_RANGE(0x0e00, 0x0e7f) AM_WRITE(sprint2_steering_reset1_w)
	AM_RANGE(0x0e80, 0x0eff) AM_WRITE(sprint2_steering_reset2_w)
	AM_RANGE(0x0f00, 0x0f7f) AM_WRITE(sprint2_noise_reset_w)
	AM_RANGE(0x1000, 0x13ff) AM_READ(sprint2_collision1_r)
	AM_RANGE(0x1400, 0x17ff) AM_READ(sprint2_collision2_r)
	AM_RANGE(0x1800, 0x1800) AM_READNOP  /* debugger ROM location? */
	AM_RANGE(0x2000, 0x3fff) AM_ROM
ADDRESS_MAP_END


static INPUT_PORTS_START( sprint2 )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, "Tracks on Demo" )
	PORT_DIPSETTING(    0x00, "Easy Track Only" )
	PORT_DIPSETTING(    0x01, "Cycle 12 Tracks" )
	PORT_DIPNAME( 0x02, 0x00, "Oil Slicks" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unused) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Extended Play" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, "Play Time" )
	PORT_DIPSETTING(    0xc0, "60 seconds" )
	PORT_DIPSETTING(    0x80, "90 seconds" )
	PORT_DIPSETTING(    0x40, "120 seconds" )
	PORT_DIPSETTING(    0x00, "150 seconds" )

	PORT_START("INA")   /* input A */
	PORT_BIT (0x01, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 1st gear */
	PORT_BIT (0x02, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 1st gear */
	PORT_BIT (0x04, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 2nd gear */
	PORT_BIT (0x08, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 2nd gear */
	PORT_BIT (0x10, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 3rd gear */
	PORT_BIT (0x20, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 3rd gear */

	PORT_START("INB")   /* input B */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Player 1 Gas") PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Player 2 Gas") PORT_PLAYER(2)
	PORT_SERVICE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT (0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT (0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT (0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Track Select") PORT_CODE(KEYCODE_SPACE)

	PORT_START("COIN")
	PORT_BIT ( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("GEAR_P1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Player 1 Gear 1") PORT_CODE(KEYCODE_Z) PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Player 1 Gear 2") PORT_CODE(KEYCODE_X) PORT_PLAYER(1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Player 1 Gear 3") PORT_CODE(KEYCODE_C) PORT_PLAYER(1)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Player 1 Gear 4") PORT_CODE(KEYCODE_V) PORT_PLAYER(1)

	PORT_START("GEAR_P2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Player 2 Gear 1") PORT_CODE(KEYCODE_Q) PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Player 2 Gear 2") PORT_CODE(KEYCODE_W) PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Player 2 Gear 3") PORT_CODE(KEYCODE_E) PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Player 2 Gear 4") PORT_CODE(KEYCODE_R) PORT_PLAYER(2)

	PORT_START("DIAL_P1")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("DIAL_P2")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("MOTOR1")
	PORT_ADJUSTER( 30, "Motor 1 RPM" )

	PORT_START("MOTOR2")
	PORT_ADJUSTER( 40, "Motor 2 RPM" )
INPUT_PORTS_END


static INPUT_PORTS_START( sprint1 )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, "Change Track" )
	PORT_DIPSETTING(    0x01, "Every Lap" )
	PORT_DIPSETTING(    0x00, "Every 2 Laps" )
	PORT_DIPNAME( 0x02, 0x00, "Oil Slicks" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unused) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Extended Play" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, "Play Time" )
	PORT_DIPSETTING(    0xc0, "60 seconds" )
	PORT_DIPSETTING(    0x80, "90 seconds" )
	PORT_DIPSETTING(    0x40, "120 seconds" )
	PORT_DIPSETTING(    0x00, "150 seconds" )

	PORT_START("INA")   /* input A */

	PORT_START("INB")   /* input B */
	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_UNUSED ) /* 1st gear */
	PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_UNUSED ) /* 2nd gear */
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_UNUSED ) /* 3rd gear */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Gas")
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT ( 0x20, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("COIN")
	PORT_BIT ( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("GEAR_P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Gear 1") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Gear 2") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Gear 3") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Gear 4") PORT_CODE(KEYCODE_V)

	PORT_START("GEAR_P2")

	PORT_START("DIAL_P1")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10)

	PORT_START("DIAL_P2")

	PORT_START("MOTOR")
	PORT_ADJUSTER( 30, "Motor RPM" )
INPUT_PORTS_END


static INPUT_PORTS_START( dominos )
	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x01, "Points to Win" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x0C, 0x08, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_2C ) )
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

	PORT_START("INA")   /* input A */
	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT ( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)

	PORT_START("INB")   /* input B */
	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT ( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT ( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )

	PORT_START("COIN")
	PORT_BIT ( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("R23")
	PORT_ADJUSTER( 50, "R23 - Tone Freq" )
INPUT_PORTS_END

static INPUT_PORTS_START( dominos4 )
	PORT_INCLUDE(dominos)

	PORT_MODIFY("INA")   /* input A */
	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT ( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT ( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT ( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)

	PORT_MODIFY("INB")   /* input A */
	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(3)
	PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(3)
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(3)
	PORT_BIT ( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(3)
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(4)
	PORT_BIT ( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(4)
	PORT_BIT ( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(4)
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(4)

	PORT_START("SELFTTEST")
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )
INPUT_PORTS_END

static const gfx_layout tile_layout =
{
	16, 8,
	64,
	1,
	{ 0 },
	{
		0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7
	},
	{
		0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38
	},
	0x40
};


static const gfx_layout car_layout =
{
	16, 8,
	32,
	1,
	{ 0 },
	{
		0x7, 0x6, 0x5, 0x4, 0x3, 0x2, 0x1, 0x0,
		0xf, 0xe, 0xd, 0xc, 0xb, 0xa, 0x9, 0x8
	},
	{
		0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70
	},
	0x80
};


static GFXDECODE_START( sprint2 )
	GFXDECODE_ENTRY( "gfx1", 0, tile_layout, 0, 2 )
	GFXDECODE_ENTRY( "gfx2", 0, car_layout, 4, 4 )
GFXDECODE_END


static MACHINE_CONFIG_START( sprint2, sprint2_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, XTAL_12_096MHz / 16)
	MCFG_CPU_PROGRAM_MAP(sprint2_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", sprint2_state,  sprint2)
	MCFG_WATCHDOG_VBLANK_INIT(8)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(512, 262)
	MCFG_SCREEN_VISIBLE_AREA(0, 511, 0, 223)
	MCFG_SCREEN_UPDATE_DRIVER(sprint2_state, screen_update_sprint2)
	MCFG_SCREEN_VBLANK_DRIVER(sprint2_state, screen_eof_sprint2)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", sprint2)
	MCFG_PALETTE_ADD("palette", 12)
	MCFG_PALETTE_INDIRECT_ENTRIES(4)
	MCFG_PALETTE_INIT_OWNER(sprint2_state, sprint2)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("discrete", DISCRETE, 0)
	MCFG_DISCRETE_INTF(sprint2)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( sprint1, sprint2 )

	/* sound hardware */
	MCFG_DEVICE_REMOVE("lspeaker")
	MCFG_DEVICE_REMOVE("rspeaker")
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_DEVICE_REMOVE("discrete")

	MCFG_SOUND_ADD("discrete", DISCRETE, 0)
	MCFG_DISCRETE_INTF(sprint1)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( dominos, sprint2 )

	/* sound hardware */
	MCFG_DEVICE_REMOVE("lspeaker")
	MCFG_DEVICE_REMOVE("rspeaker")
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_DEVICE_REMOVE("discrete")

	MCFG_SOUND_ADD("discrete", DISCRETE, 0)
	MCFG_DISCRETE_INTF(dominos)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

ROM_START( sprint1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6290-01.b1", 0x2000, 0x0800, CRC(41fc985e) SHA1(7178846480cbf8d15955ccd987d0b0e902ab9f90) )
	ROM_LOAD( "6291-01.c1", 0x2800, 0x0800, CRC(07f7a920) SHA1(845f65d2bd290eb295ca6bae2575f27aaa08c0dd) )
	ROM_LOAD( "6442-01.d1", 0x3000, 0x0800, CRC(e9ff0124) SHA1(42fe028e2e595573ccc0821de3bb6970364c585d) )
	ROM_LOAD( "6443-01.e1", 0x3800, 0x0800, CRC(d6bb00d0) SHA1(cdcd4bb7b32be7a11480d3312fcd8d536e2d0caf) )

	ROM_REGION( 0x0200, "gfx1", 0 ) /* tiles */
	ROM_LOAD_NIB_HIGH( "6396-01.p4", 0x0000, 0x0200, CRC(801b42dd) SHA1(1db58390d803f404253cbf36d562016441ca568d) )
	ROM_LOAD_NIB_LOW ( "6397-01.r4", 0x0000, 0x0200, CRC(135ba1aa) SHA1(0465259440f73e1a2c8d8101f29e99b4885420e4) )

	ROM_REGION( 0x0200, "gfx2", 0 ) /* cars */
	ROM_LOAD_NIB_HIGH( "6399-01.j6", 0x0000, 0x0200, CRC(63d685b2) SHA1(608746163e25dbc14cde43c17aecbb9a14fac875) )
	ROM_LOAD_NIB_LOW ( "6398-01.k6", 0x0000, 0x0200, CRC(c9e1017e) SHA1(e7279a13e4a812d2e0218be0bc5162f2e56c6b66) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "6400-01.m2", 0x0000, 0x0100, CRC(b8094b4c) SHA1(82dc6799a19984f3b204ee3aeeb007e55afc8be3) )  /* SYNC */
	ROM_LOAD( "6401-01.e2", 0x0100, 0x0020, CRC(857df8db) SHA1(06313d5bde03220b2bc313d18e50e4bb1d0cfbbb) )  /* address */
ROM_END


ROM_START( sprint2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6290-01.b1", 0x2000, 0x0800, CRC(41fc985e) SHA1(7178846480cbf8d15955ccd987d0b0e902ab9f90) )
	ROM_LOAD( "6291-01.c1", 0x2800, 0x0800, CRC(07f7a920) SHA1(845f65d2bd290eb295ca6bae2575f27aaa08c0dd) )
	ROM_LOAD( "6404.d1",    0x3000, 0x0800, CRC(d2878ff6) SHA1(b742a8896c1bf1cfacf48d06908920d88a2c9ea8) )
	ROM_LOAD( "6405.e1",    0x3800, 0x0800, CRC(6c991c80) SHA1(c30a5b340f05dd702c7a186eb62607a48fa19f72) )

	ROM_REGION( 0x0200, "gfx1", 0 ) /* tiles */
	ROM_LOAD_NIB_HIGH( "6396-01.p4", 0x0000, 0x0200, CRC(801b42dd) SHA1(1db58390d803f404253cbf36d562016441ca568d) )
	ROM_LOAD_NIB_LOW ( "6397-01.r4", 0x0000, 0x0200, CRC(135ba1aa) SHA1(0465259440f73e1a2c8d8101f29e99b4885420e4) )

	ROM_REGION( 0x0200, "gfx2", 0 ) /* cars */
	ROM_LOAD_NIB_HIGH( "6399-01.j6", 0x0000, 0x0200, CRC(63d685b2) SHA1(608746163e25dbc14cde43c17aecbb9a14fac875) )
	ROM_LOAD_NIB_LOW ( "6398-01.k6", 0x0000, 0x0200, CRC(c9e1017e) SHA1(e7279a13e4a812d2e0218be0bc5162f2e56c6b66) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "6400-01.m2", 0x0000, 0x0100, CRC(b8094b4c) SHA1(82dc6799a19984f3b204ee3aeeb007e55afc8be3) )  /* SYNC */
	ROM_LOAD( "6401-01.e2", 0x0100, 0x0020, CRC(857df8db) SHA1(06313d5bde03220b2bc313d18e50e4bb1d0cfbbb) )  /* address */
ROM_END


ROM_START( sprint2a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6290-01.b1", 0x2000, 0x0800, CRC(41fc985e) SHA1(7178846480cbf8d15955ccd987d0b0e902ab9f90) )
	ROM_LOAD( "6291-01.c1", 0x2800, 0x0800, CRC(07f7a920) SHA1(845f65d2bd290eb295ca6bae2575f27aaa08c0dd) )
	ROM_LOAD( "6404.d1",    0x3000, 0x0800, CRC(d2878ff6) SHA1(b742a8896c1bf1cfacf48d06908920d88a2c9ea8) )
	ROM_LOAD( "6405-02.e1", 0x3800, 0x0800, CRC(e80fd249) SHA1(7bcf7dfd72ca83fdd80593eaf392570da1f71298) )

	ROM_REGION( 0x0200, "gfx1", 0 ) /* tiles */
	ROM_LOAD_NIB_HIGH( "6396-01.p4", 0x0000, 0x0200, CRC(801b42dd) SHA1(1db58390d803f404253cbf36d562016441ca568d) )
	ROM_LOAD_NIB_LOW ( "6397-01.r4", 0x0000, 0x0200, CRC(135ba1aa) SHA1(0465259440f73e1a2c8d8101f29e99b4885420e4) )

	ROM_REGION( 0x0200, "gfx2", 0 ) /* cars */
	ROM_LOAD_NIB_HIGH( "6399-01.j6", 0x0000, 0x0200, CRC(63d685b2) SHA1(608746163e25dbc14cde43c17aecbb9a14fac875) )
	ROM_LOAD_NIB_LOW ( "6398-01.k6", 0x0000, 0x0200, CRC(c9e1017e) SHA1(e7279a13e4a812d2e0218be0bc5162f2e56c6b66) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "6400-01.m2", 0x0000, 0x0100, CRC(b8094b4c) SHA1(82dc6799a19984f3b204ee3aeeb007e55afc8be3) )  /* SYNC */
	ROM_LOAD( "6401-01.e2", 0x0100, 0x0020, CRC(857df8db) SHA1(06313d5bde03220b2bc313d18e50e4bb1d0cfbbb) )  /* address */
ROM_END


ROM_START( sprint2h )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6290-01.b1", 0x2000, 0x0800, CRC(41fc985e) SHA1(7178846480cbf8d15955ccd987d0b0e902ab9f90) )
	ROM_LOAD( "6291-01.c1", 0x2800, 0x0800, CRC(07f7a920) SHA1(845f65d2bd290eb295ca6bae2575f27aaa08c0dd) )
	ROM_LOAD( "6404.d1",    0x3000, 0x0800, CRC(d2878ff6) SHA1(b742a8896c1bf1cfacf48d06908920d88a2c9ea8) )
	ROM_LOAD( "6405-02.e1", 0x3800, 0x0800, CRC(6de291f1) SHA1(00c2826011d80ac0784649a7bc156a97c26565fd) ) // sldh

	ROM_REGION( 0x0200, "gfx1", 0 ) /* tiles */
	ROM_LOAD_NIB_HIGH( "6396-01.p4", 0x0000, 0x0200, CRC(801b42dd) SHA1(1db58390d803f404253cbf36d562016441ca568d) )
	ROM_LOAD_NIB_LOW ( "6397-01.r4", 0x0000, 0x0200, CRC(135ba1aa) SHA1(0465259440f73e1a2c8d8101f29e99b4885420e4) )

	ROM_REGION( 0x0200, "gfx2", 0 ) /* cars */
	ROM_LOAD_NIB_HIGH( "6399-01.j6", 0x0000, 0x0200, CRC(63d685b2) SHA1(608746163e25dbc14cde43c17aecbb9a14fac875) )
	ROM_LOAD_NIB_LOW ( "6398-01.k6", 0x0000, 0x0200, CRC(c9e1017e) SHA1(e7279a13e4a812d2e0218be0bc5162f2e56c6b66) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "6400-01.m2", 0x0000, 0x0100, CRC(b8094b4c) SHA1(82dc6799a19984f3b204ee3aeeb007e55afc8be3) )  /* SYNC */
	ROM_LOAD( "6401-01.e2", 0x0100, 0x0020, CRC(857df8db) SHA1(06313d5bde03220b2bc313d18e50e4bb1d0cfbbb) )  /* address */
ROM_END


ROM_START( dominos )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "7352-02.d1",   0x3000, 0x0800, CRC(738b4413) SHA1(3a90ab25bb5f65504692f97da43f03e21392dcd8) )
	ROM_LOAD( "7438-02.e1",   0x3800, 0x0800, CRC(c84e54e2) SHA1(383b388a1448a195f28352fc5e4ff1a2af80cc95) )

	ROM_REGION( 0x200, "gfx1", 0 ) /* tiles */
	ROM_LOAD_NIB_HIGH( "7439-01.p4",   0x0000, 0x0200, CRC(4f42fdd6) SHA1(f8ea4b582e26cad37b746174cdc9f1c7ae0819c3) )
	ROM_LOAD_NIB_LOW ( "7440-01.r4",   0x0000, 0x0200, CRC(957dd8df) SHA1(280457392f40cd66eae34d2fcdbd4d2142793402) )

	ROM_REGION( 0x200, "gfx2", 0 ) /* sprites, not used */
	ROM_FILL( 0x0000, 0x0200, 0 )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "6400-01.m2", 0x0000, 0x0100, CRC(b8094b4c) SHA1(82dc6799a19984f3b204ee3aeeb007e55afc8be3) )  /* SYNC */
	ROM_LOAD( "6401-01.e2", 0x0100, 0x0020, CRC(857df8db) SHA1(06313d5bde03220b2bc313d18e50e4bb1d0cfbbb) )  /* address */
ROM_END

ROM_START( dominos4 ) // built from original Atari source code
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD_NIB_HIGH( "007754-01.l1",   0x3000, 0x0400, CRC(03fae4a9) SHA1(a132bd8bc866e33cdf6b4881064c8d265c2b25f4) )
	ROM_LOAD_NIB_LOW ( "007755-01.l0",   0x3000, 0x0400, CRC(fa2d0c04) SHA1(fcf618c7089db46d55933d58ea04701af515ad49) )
	ROM_LOAD_NIB_HIGH( "007756-01.m1",   0x3400, 0x0400, CRC(d2acb1b5) SHA1(ad81eed9dd0a2d5ecfd42daf90825726e64063b3) )
	ROM_LOAD_NIB_LOW ( "007757-01.m0",   0x3400, 0x0400, CRC(69f2db90) SHA1(a064c840599c4e7cb65670e5480adeb310247f16) )
	ROM_LOAD_NIB_HIGH( "007758-01.n1",   0x3800, 0x0400, CRC(b49083b4) SHA1(41999e8d3fd6104c42f3a034045f9f9c75d8247a) )
	ROM_LOAD_NIB_LOW ( "007759-01.n0",   0x3800, 0x0400, CRC(542200c7) SHA1(111f06e942e247b00b9f90fae2986c3c8d9ec8c5) )
	ROM_LOAD_NIB_HIGH( "007760-01.p1",   0x3c00, 0x0400, CRC(7dc2a7a1) SHA1(9d02572cf689c6476b33226a5358dd1f72c4e61d) )
	ROM_LOAD_NIB_LOW ( "007761-01.p0",   0x3c00, 0x0400, CRC(04365e0d) SHA1(fefc3c04e55f1aa8c80b1e5e1e403af8698c3530) )

	ROM_REGION( 0x200, "gfx1", 0 ) /* tiles */
	ROM_LOAD_NIB_HIGH( "007764-01.p4",   0x0000, 0x0200, CRC(e4332dc0) SHA1(1f16c5b9f9fd7d478fd729cc79968f17746111f4) )
	ROM_LOAD_NIB_LOW ( "007765-01.r4",   0x0000, 0x0200, CRC(6e4e6c75) SHA1(0fc77fecaa73eac57baf778bc51387c75883aad4) )

	ROM_REGION( 0x200, "gfx2", 0 ) /* sprites, not used */
	ROM_FILL( 0x0000, 0x0200, 0 )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "6400-01.m2", 0x0000, 0x0100, CRC(b8094b4c) SHA1(82dc6799a19984f3b204ee3aeeb007e55afc8be3) )  /* SYNC */
	ROM_LOAD( "6401-01.e2", 0x0100, 0x0020, CRC(857df8db) SHA1(06313d5bde03220b2bc313d18e50e4bb1d0cfbbb) )  /* address */
ROM_END

GAME( 1978, sprint1,  0,       sprint1, sprint1, sprint2_state, sprint1, ROT0, "Atari (Kee Games)", "Sprint 1", 0 )
GAME( 1976, sprint2,  sprint1, sprint2, sprint2, sprint2_state, sprint2, ROT0, "Atari (Kee Games)", "Sprint 2 (set 1)", 0 )
GAME( 1976, sprint2a, sprint1, sprint2, sprint2, sprint2_state, sprint2, ROT0, "Atari (Kee Games)", "Sprint 2 (set 2)", 0 )
GAME( 1976, sprint2h, sprint1, sprint2, sprint2, sprint2_state, sprint2, ROT0, "hack", "Sprint 2 (color kit, Italy)", MACHINE_WRONG_COLORS ) // Italian hack, supposedly is color instead of b/w? how?
GAME( 1977, dominos,  0,       dominos, dominos, sprint2_state, dominos, ROT0, "Atari", "Dominos", 0 )
GAME( 1977, dominos4, dominos, dominos, dominos4,sprint2_state, dominos4,ROT0, "Atari", "Dominos 4 (Cocktail)", 0 )
