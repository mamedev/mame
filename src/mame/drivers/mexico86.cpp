// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/***************************************************************************

Kick & Run - (c) 1987 Taito

Ernesto Corvi
ernesto@imagina.com

Notes:
- master/slave 4 players mode is not emulated at all.
  To set it up, enable the Master/Slave Mode and set the two boards IDs with
  different settings. Do NOT enable the Single board 4p mode, I don't think
  that the main board is supposed to be still connected to the sub board.

- Single board 4 players mode actually works but I'm not sure how the reset /
  halt line is truly connected on the sub cpu.
  To set it up, just enable the single board 4p mode and keep the master/slave
  mode to off and the board ID to master.

- kicknrun does a PS4 STOP ERROR shortly after boot, but works afterwards.
  PS4 is the MC6801U4 mcu.

- Kiki Kaikai suffers from random lock-ups. It happens when the sound
  CPU misses CTS from YM2203. The processor will loop infinitely and the main
  CPU will in turn wait forever. It's difficult to meet the required level
  of synchronization. This is kludged by filtering the 2203's busy signal.

- KiKi KaiKai uses a custom MC6801U4 MCU which isn't dumped. The bootleg Knight Boy
  replaces it with a 68705. The bootleg is NOT 100% equivalent to the original
  (a situation similar to Bubble Bobble): collision detection is imperfect, the
  player can't be killed by some enemies.
  I think the bootleggers put the custom mcu in a test rig, examined its bus
  activity and replicated the behaviour inaccurately because they coudln't
  figure it all out. Indeed, the 68705 code reads all the memory locations
  related to the missing collision detection, but does nothing with them.

- Kick and Run is a rom swap for Kiki KaiKai as the pal chips are all A85-0x
  A85 is the Taito rom code for Kiki KaiKai.  Even the MCU is socketed!

Note MCU labeling:

Bubble Bobble   KiKi KaiKai      Kick and Run
-------------   -------------    -------------
TAITO A78-01    TAITO A85-01     TAITO A87-01
JPH1011P        JPH1020P         JPH1021P
185             185              185
PS4  J8635      PS4  J8541       PS4  J8648

***************************************************************************/

#include "emu.h"
#include "includes/mexico86.h"

#include "cpu/z80/z80.h"
#include "screen.h"
#include "speaker.h"


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

READ8_MEMBER(mexico86_state::kiki_ym2203_r)
{
	u8 result = m_ymsnd->read(offset);

	if (offset == 0)
		result &= 0x7f;

	return result;
}


/*************************************
 *
 *  Address maps
 *
 *************************************/

void mexico86_state::mexico86_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr("bank1");                /* banked roms */
	map(0xc000, 0xe7ff).ram().share("mainram");         /* shared with sound cpu */
	map(0xe800, 0xe8ff).ram().share("protection_ram");  /* shared with mcu */
	map(0xe900, 0xefff).ram();
	map(0xf000, 0xf000).w(FUNC(mexico86_state::mexico86_bankswitch_w));    /* program and gfx ROM banks */
	map(0xf008, 0xf008).w(FUNC(mexico86_state::mexico86_f008_w));          /* cpu reset lines + other unknown stuff */
	map(0xf010, 0xf010).portr("IN3");
	map(0xf018, 0xf018).nopw();                        /* watchdog? */
	map(0xf800, 0xffff).ram().share("subram");          /* communication ram - to connect 4 players's subboard */
}

void mexico86_state::mexico86_sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xa7ff).ram().share("mainram");  /* shared with main */
	map(0xa800, 0xbfff).ram();
	map(0xc000, 0xc001).r(FUNC(mexico86_state::kiki_ym2203_r)).w(m_ymsnd, FUNC(ym2203_device::write));
}

WRITE8_MEMBER(mexico86_state::mexico86_sub_output_w)
{
	/*--x- ---- coin lockout 2*/
	/*---x ---- coin lockout 1*/
	/*---- -x-- coin counter*/
	/*---- --x- <unknown, always high, irq ack?>*/
}

void mexico86_state::mexico86_sub_cpu_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x47ff).ram(); /* sub cpu ram */
	map(0x8000, 0x87ff).ram().share("subram");  /* shared with main */
	map(0xc000, 0xc000).portr("IN4");
	map(0xc001, 0xc001).portr("IN5");
	map(0xc002, 0xc002).portr("IN6");
	map(0xc003, 0xc003).portr("IN7");
	map(0xc004, 0xc004).w(FUNC(mexico86_state::mexico86_sub_output_w));
}

/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( mexico86 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE )    /* service 2 */

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW0")
	/* When Bit 1 is On, the machine waits a signal from another one */
	/* Seems like if you can join two cabinets, one as master */
	/* and the other as slave, probably to play four players. */
	PORT_DIPNAME( 0x01, 0x01, "Master/Slave Mode" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:2") // Screen ?
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x04, IP_ACTIVE_LOW ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:4")// this should be Demo Sounds, but doesn't work?
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x08, "Playing Time" ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, "40 Seconds" )
	PORT_DIPSETTING(    0x0c, "One Minute" )
	PORT_DIPSETTING(    0x08, "One Minute and 20 Sec." )
	PORT_DIPSETTING(    0x04, "One Minute and 40 Sec." )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	/* The following dip seems to be related with the first one */
	PORT_DIPNAME( 0x20, 0x20, "Board ID" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, "Master" )
	PORT_DIPSETTING(    0x00, "Slave" )
	PORT_DIPNAME( 0x40, 0x40, "Number of Matches" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x40, "6" )
	PORT_DIPNAME( 0x80, 0x80, "Single board 4 Players Mode" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) //p3 service

	PORT_START("IN5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) //p4 service

	PORT_START("IN6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH,IPT_COIN3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH,IPT_COIN4 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( kikikai )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) // Ofuda in service mode
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) // Oharai
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x04, IP_ACTIVE_LOW ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

#if 0 // old coinage settings
	PORT_DIPNAME( 0x30, 0x30, "Coin 1" )
	PORT_DIPSETTING(    0x30, "A:1C/1C B:1C/1C" )
	PORT_DIPSETTING(    0x20, "A:1C/2C B:2C/1C" )
	PORT_DIPSETTING(    0x10, "A:2C/1C B:3C/1C" )
	PORT_DIPSETTING(    0x00, "A:2C/3C B:4C/1C" )
	PORT_DIPNAME( 0xc0, 0xc0, "Coin 2" )
	PORT_DIPSETTING(    0xc0, "A:1C/1C B:1C/2C" )
	PORT_DIPSETTING(    0x80, "A:1C/2C B:1C/3C" )
	PORT_DIPSETTING(    0x40, "A:2C/1C B:1C/4C" )
	PORT_DIPSETTING(    0x00, "A:2C/3C B:1C/6C" )
#endif

	// coinage copied from Japanese manual but type B doesn't work
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )  PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )  PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, "50000 100000" )
	PORT_DIPSETTING(    0x0c, "70000 150000" )
	PORT_DIPSETTING(    0x08, "70000 200000" )
	PORT_DIPSETTING(    0x04, "100000 300000" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )  PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, "A" )
	PORT_DIPSETTING(    0x00, "B" )
	PORT_DIPNAME( 0x80, 0x00, "Number Match" )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,
	4*2048,
	4,
	{ 0x20000*8, 0x20000*8+4, 0, 4 },
	{ 3, 2, 1, 0, 8+3, 8+2, 8+1, 8+0 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static GFXDECODE_START( gfx_mexico86 )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 16 )
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void mexico86_state::machine_start()
{
	u8 *const ROM = memregion("maincpu")->base();

	membank("bank1")->configure_entries(0, 6, &ROM[0x08000], 0x4000);

	save_item(NAME(m_port_a_out));
	save_item(NAME(m_port_b_out));
	save_item(NAME(m_address));
	save_item(NAME(m_latch));

	save_item(NAME(m_mcu_running));
	save_item(NAME(m_mcu_initialised));
	save_item(NAME(m_coin_last));
	save_item(NAME(m_coin_fract));

	save_item(NAME(m_charbank));

	m_port_a_out = 0xff;
	m_port_b_out = 0xff;
}

void mexico86_state::machine_reset()
{
	/*TODO: check the PCB and see how the halt / reset lines are connected. */
	if (m_subcpu != nullptr)
		m_subcpu->set_input_line(INPUT_LINE_RESET, (ioport("DSW1")->read() & 0x80) ? ASSERT_LINE : CLEAR_LINE);

	m_address = 0;
	m_latch = 0;

	m_mcu_running = 0;
	m_mcu_initialised = 0;
	m_coin_last[0] = false;
	m_coin_last[1] = false;
	m_coin_fract = 0;

	m_charbank = 0;
}

void mexico86_state::mexico86(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 24000000/4); /* 6 MHz, Uses clock divided 24MHz OSC */
	m_maincpu->set_addrmap(AS_PROGRAM, &mexico86_state::mexico86_map);

	Z80(config, m_audiocpu, 24000000/4); /* 6 MHz, Uses clock divided 24MHz OSC */
	m_audiocpu->set_addrmap(AS_PROGRAM, &mexico86_state::mexico86_sound_map);
	m_audiocpu->set_vblank_int("screen", FUNC(mexico86_state::irq0_line_hold));

	M68705P3(config, m_mcu, 4000000); /* xtal is 4MHz, divided by 4 internally */
	m_mcu->portc_r().set_ioport("IN0");
	m_mcu->porta_w().set(FUNC(mexico86_state::mexico86_68705_port_a_w));
	m_mcu->portb_w().set(FUNC(mexico86_state::mexico86_68705_port_b_w));
	m_mcu->set_vblank_int("screen", FUNC(mexico86_state::mexico86_m68705_interrupt));

	Z80(config, m_subcpu, 8000000/2); /* 4 MHz, Uses 8Mhz OSC */
	m_subcpu->set_addrmap(AS_PROGRAM, &mexico86_state::mexico86_sub_cpu_map);
	m_subcpu->set_vblank_int("screen", FUNC(mexico86_state::irq0_line_hold));

	/* 100 CPU slices per frame - high value to ensure proper synchronization of the CPUs */
	config.set_maximum_quantum(attotime::from_hz(6000));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));  /* frames per second, vblank duration */
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(mexico86_state::screen_update_mexico86));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_mexico86);
	PALETTE(config, m_palette, palette_device::RGB_444_PROMS, "proms", 256);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	YM2203(config, m_ymsnd, 3000000);
	m_ymsnd->port_a_read_callback().set_ioport("DSW0");
	m_ymsnd->port_b_read_callback().set_ioport("DSW1");
	m_ymsnd->add_route(0, "mono", 0.30);
	m_ymsnd->add_route(1, "mono", 0.30);
	m_ymsnd->add_route(2, "mono", 0.30);
	m_ymsnd->add_route(3, "mono", 1.00);
}

void mexico86_state::knightb(machine_config &config)
{
	mexico86(config);
	config.device_remove("sub");
	m_screen->set_screen_update(FUNC(mexico86_state::screen_update_kikikai));
}

void mexico86_state::kikikai(machine_config &config)
{
	knightb(config);

	// IRQs should be triggered by the MCU, but we don't have it
	m_maincpu->set_vblank_int("screen", FUNC(mexico86_state::kikikai_interrupt));

	config.device_remove("mcu");   // we don't have code for the MC6801U4

	/* video hardware */
	m_screen->set_screen_update(FUNC(mexico86_state::screen_update_kikikai));
}


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( kikikai )
	ROM_REGION( 0x28000, "maincpu", 0 )
	ROM_LOAD( "a85-17.h16", 0x00000, 0x08000, CRC(c141d5ab) SHA1(fe3622ba283e514416c43a44f83f922a958b27cd) ) /* 1st half, main code        */
	ROM_CONTINUE(           0x18000, 0x08000 )             /* 2nd half, banked at 0x8000 */
	ROM_LOAD( "a85-16.h18", 0x08000, 0x10000, CRC(4094d750) SHA1(05e0ad177a3eb144b203784ecb6242a0fc5c4d4d) ) /* banked at 0x8000           */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a85-11.f6", 0x0000, 0x8000, CRC(cc3539db) SHA1(4239a40fdee65cba613e4b4ec54cf7899480e366) )

	ROM_REGION( 0x0800, "cpu2", 0 )    /* 2k for the microcontroller (MC6801U4 type MCU) */
	/* MCU labeled TAITO A85 01,  JPH1020P, 185, PS4 */
	ROM_LOAD( "a85-01.g8",    0x0000, 0x0800, NO_DUMP )

	ROM_REGION( 0x40000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "a85-15.a1", 0x00000, 0x10000, CRC(aebc8c32) SHA1(77347cf5780f084a77123eb636cd0bad672a39e8) )
	ROM_LOAD( "a85-14.a3", 0x10000, 0x10000, CRC(a9df0453) SHA1(a5e9cd6266ab3ae46cd1b35a4603e13a2ca023fb) )
	ROM_LOAD( "a85-13.a4", 0x20000, 0x10000, CRC(3eeaf878) SHA1(f8ae8938a8358d1222e9fdf7bc0094ac13faf404) )
	ROM_LOAD( "a85-12.a6", 0x30000, 0x10000, CRC(91e58067) SHA1(c7eb9bf650039254fb7664758938b1012eacc597) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "a85-08.g15", 0x0000, 0x0100, CRC(d15f61a8) SHA1(945c8aa26c85269c10373218bef13e04e25eb1e4) ) /* all proms are 63S141AN or compatible type */
	ROM_LOAD( "a85-10.g12", 0x0100, 0x0100, CRC(8fc3fa86) SHA1(d4d86f8e147bbf2a370de428ac20a28b0f146782) )
	ROM_LOAD( "a85-09.g14", 0x0200, 0x0100, CRC(b931c94d) SHA1(fb554084f34c602d1ff7806fb945a06cf14332af) )
ROM_END

ROM_START( knightb )
	ROM_REGION( 0x28000, "maincpu", 0 )
	ROM_LOAD( "a85-17.h16", 0x00000, 0x08000, CRC(c141d5ab) SHA1(fe3622ba283e514416c43a44f83f922a958b27cd) ) /* 1st half, main code        */
	ROM_CONTINUE(           0x18000, 0x08000 )             /* 2nd half, banked at 0x8000 */
	ROM_LOAD( "a85-16.h18", 0x08000, 0x10000, CRC(4094d750) SHA1(05e0ad177a3eb144b203784ecb6242a0fc5c4d4d) ) /* banked at 0x8000           */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a85-11.f6", 0x0000, 0x8000, CRC(cc3539db) SHA1(4239a40fdee65cba613e4b4ec54cf7899480e366) )

	ROM_REGION( 0x0800, "mcu", 0 )    /* 2k for the microcontroller */
	ROM_LOAD( "knightb.uc", 0x0000, 0x0800, CRC(3cc2bbe4) SHA1(af018a1e0655b66fd859617a3bd0c01a4967c0e6) )

	ROM_REGION( 0x40000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "knightb.d",  0x00000, 0x10000, CRC(53ecdb3f) SHA1(f8b4822926f3712a426c014759b1cf382a7ad9d1) )
	ROM_LOAD( "a85-14.a3",  0x10000, 0x10000, CRC(a9df0453) SHA1(a5e9cd6266ab3ae46cd1b35a4603e13a2ca023fb) )
	ROM_LOAD( "knightb.b",  0x20000, 0x10000, CRC(63ad7df3) SHA1(8ce149b63032bcdd596a3fa52baba2f2c154e84e) )
	ROM_LOAD( "a85-12.a6", 0x30000, 0x10000, CRC(91e58067) SHA1(c7eb9bf650039254fb7664758938b1012eacc597) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "a85-08.g15", 0x0000, 0x0100, CRC(d15f61a8) SHA1(945c8aa26c85269c10373218bef13e04e25eb1e4) ) /* all proms are 63S141AN or compatible type */
	ROM_LOAD( "a85-10.g12", 0x0100, 0x0100, CRC(8fc3fa86) SHA1(d4d86f8e147bbf2a370de428ac20a28b0f146782) )
	ROM_LOAD( "a85-09.g14", 0x0200, 0x0100, CRC(b931c94d) SHA1(fb554084f34c602d1ff7806fb945a06cf14332af) )
ROM_END

ROM_START( kicknrun )
	ROM_REGION( 0x28000, "maincpu", 0 )
	ROM_LOAD( "a87-08.h16", 0x00000, 0x08000, CRC(715e1b04) SHA1(60b7259758ec73f1cc945556e9c2b25766b745a8) ) /* 1st half, main code        */
	ROM_CONTINUE(           0x18000, 0x08000 )             /* 2nd half, banked at 0x8000 */
	ROM_LOAD( "a87-07.h18", 0x08000, 0x10000, CRC(6cb6ebfe) SHA1(fca61fc2ad8fadc1e15b9ff84c7469b68d16e885) ) /* banked at 0x8000           */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a87-06.f6", 0x0000, 0x8000, CRC(1625b587) SHA1(7336384e13c114915de5e439df5731ce3fc2054a) )

	ROM_REGION( 0x0800, "mcu", 0 )    /* 2k for the microcontroller (MC6801U4 type MCU) */
	/* MCU labeled TAITO A78 01,  JPH1021P, 185, PS4 */
	ROM_LOAD( "a87-01.g8", 0x0000, 0x0800, BAD_DUMP CRC(8e821fa0) SHA1(331f5da31d8767674e2b5bf0e7f5b5ad2535e044)  )  /* manually crafted from the Mexico '86 one */

	ROM_REGION( 0x10000, "sub", 0 )    /* 64k for the cpu on the sub board */
	ROM_LOAD( "a87-09-1",  0x0000, 0x4000, CRC(6a2ad32f) SHA1(42d4b97b25d219902ad215793f1d2c006ffe94dc) )

	ROM_REGION( 0x40000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "a87-05.a1", 0x08000, 0x08000, CRC(4eee3a8a) SHA1(2f0e4c2fb6cba48d0e2b95927fc14f0038557371) )
	ROM_CONTINUE(          0x00000, 0x08000 )
	ROM_LOAD( "a87-04.a3", 0x10000, 0x08000, CRC(8b438d20) SHA1(12e615f34b7e732157f893b97c9b7e99e9ef7d62) )
	ROM_RELOAD(            0x18000, 0x08000 )
	ROM_LOAD( "a87-03.a4", 0x28000, 0x08000, CRC(f42e8a88) SHA1(db2702141981ba368bdc665443a8a0662266e6d9) )
	ROM_CONTINUE(          0x20000, 0x08000 )
	ROM_LOAD( "a87-02.a6", 0x30000, 0x08000, CRC(64f1a85f) SHA1(04fb9824450812b08f7e6fc57e0af828be9bd575) )
	ROM_RELOAD(            0x38000, 0x08000 )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "a87-10.g15", 0x0000, 0x0100, CRC(be6eb1f0) SHA1(f4d00e9b12bf116bf84edb2ff6caab158094b668) ) /* all proms are 63S141AN or compatible type */
	ROM_LOAD( "a87-12.g12", 0x0100, 0x0100, CRC(3e953444) SHA1(e9c84ca9390fd7c73738a8b681a02e87fbd51bb4) )
	ROM_LOAD( "a87-11.g14", 0x0200, 0x0100, CRC(14f6c28d) SHA1(8c60974e4607906a3f77260bdd0704af60d596fc) )
ROM_END

ROM_START( kicknrunu )
	ROM_REGION( 0x28000, "maincpu", 0 )
	ROM_LOAD( "a87-23.h16", 0x00000, 0x08000, CRC(37182560) SHA1(8db393131f50af88b2e7489d6aae65bad0a5a65b) ) /* 1st half, main code        */
	ROM_CONTINUE(           0x18000, 0x08000 )             /* 2nd half, banked at 0x8000 */
	ROM_LOAD( "a87-22.h18", 0x08000, 0x10000, CRC(3b5a8354) SHA1(e0db4cb0657989d5a21f9a8d4e8f842adba636ad) ) /* banked at 0x8000           */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a87-06.f6", 0x0000, 0x8000, CRC(1625b587) SHA1(7336384e13c114915de5e439df5731ce3fc2054a) )

	ROM_REGION( 0x0800, "mcu", 0 )    /* 2k for the microcontroller (MC6801U4 type MCU) */
	/* MCU labeled TAITO A78 01,  JPH1021P, 185, PS4 */
	ROM_LOAD( "a87-01.g8", 0x0000, 0x0800, BAD_DUMP CRC(8e821fa0) SHA1(331f5da31d8767674e2b5bf0e7f5b5ad2535e044)  )  /* manually crafted from the Mexico '86 one */

	ROM_REGION( 0x10000, "sub", 0 )    /* 64k for the cpu on the sub board */
	ROM_LOAD( "a87-09-1",  0x0000, 0x4000, CRC(6a2ad32f) SHA1(42d4b97b25d219902ad215793f1d2c006ffe94dc) )

	ROM_REGION( 0x40000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "a87-05.a1", 0x08000, 0x08000, CRC(4eee3a8a) SHA1(2f0e4c2fb6cba48d0e2b95927fc14f0038557371) )
	ROM_CONTINUE(          0x00000, 0x08000 )
	ROM_LOAD( "a87-04.a3", 0x10000, 0x08000, CRC(8b438d20) SHA1(12e615f34b7e732157f893b97c9b7e99e9ef7d62) )
	ROM_RELOAD(            0x18000, 0x08000 )
	ROM_LOAD( "a87-03.a4", 0x28000, 0x08000, CRC(f42e8a88) SHA1(db2702141981ba368bdc665443a8a0662266e6d9) )
	ROM_CONTINUE(          0x20000, 0x08000 )
	ROM_LOAD( "a87-02.a6", 0x30000, 0x08000, CRC(64f1a85f) SHA1(04fb9824450812b08f7e6fc57e0af828be9bd575) )
	ROM_RELOAD(            0x38000, 0x08000 )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "a87-10.g15", 0x0000, 0x0100, CRC(be6eb1f0) SHA1(f4d00e9b12bf116bf84edb2ff6caab158094b668) ) /* all proms are 63S141AN or compatible type */
	ROM_LOAD( "a87-12.g12", 0x0100, 0x0100, CRC(3e953444) SHA1(e9c84ca9390fd7c73738a8b681a02e87fbd51bb4) )
	ROM_LOAD( "a87-11.g14", 0x0200, 0x0100, CRC(14f6c28d) SHA1(8c60974e4607906a3f77260bdd0704af60d596fc) )
ROM_END

ROM_START( mexico86 )
	ROM_REGION( 0x28000, "maincpu", 0 )
	ROM_LOAD( "2_g.bin",    0x00000, 0x08000, CRC(2bbfe0fb) SHA1(8f047e001ea8e49d28f73e546c82812af1c2533c) ) /* 1st half, main code        */
	ROM_CONTINUE(           0x18000, 0x08000 )             /* 2nd half, banked at 0x8000 */
	ROM_LOAD( "1_f.bin",    0x08000, 0x10000, CRC(0b93e68e) SHA1(c6fbcce83103e3e71a7a1ef9f18a10622ed6b951) ) /* banked at 0x8000           */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a87-06.f6", 0x0000, 0x8000, CRC(1625b587) SHA1(7336384e13c114915de5e439df5731ce3fc2054a) )

	ROM_REGION( 0x0800, "mcu", 0 )    /* 2k for the microcontroller */
	ROM_LOAD( "68_h.bin",   0x0000, 0x0800, CRC(ff92f816) SHA1(0015c3f2ed014052b3fa376409e3a7cca36fac72) )

	ROM_REGION( 0x10000, "sub", 0 )    /* 64k for the cpu on the sub board */
	ROM_LOAD( "a87-09-1",  0x0000, 0x4000, CRC(6a2ad32f) SHA1(42d4b97b25d219902ad215793f1d2c006ffe94dc) )

	ROM_REGION( 0x40000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "4_d.bin",    0x08000, 0x08000, CRC(57cfdbca) SHA1(89c305c380c3de14a956ee4bc85d3a0d343b638e) )
	ROM_CONTINUE(           0x00000, 0x08000 )
	ROM_LOAD( "5_c.bin",    0x10000, 0x08000, CRC(e42fa143) SHA1(02d7e0e01af1cecc3952f6355987118098d346c3) )
	ROM_RELOAD(             0x18000, 0x08000 )
	ROM_LOAD( "6_b.bin",    0x28000, 0x08000, CRC(a4607989) SHA1(6832147603a146c34cc1809e839c8e034d0dacc5) )
	ROM_CONTINUE(           0x20000, 0x08000 )
	ROM_LOAD( "7_a.bin",    0x30000, 0x08000, CRC(245036b1) SHA1(108d9959de869b4fdf766abeade1486acec13bf2) )
	ROM_RELOAD(             0x38000, 0x08000 )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "a87-10.g15", 0x0000, 0x0100, CRC(be6eb1f0) SHA1(f4d00e9b12bf116bf84edb2ff6caab158094b668) ) /* all proms are 63S141AN or compatible type */
	ROM_LOAD( "a87-12.g12", 0x0100, 0x0100, CRC(3e953444) SHA1(e9c84ca9390fd7c73738a8b681a02e87fbd51bb4) )
	ROM_LOAD( "a87-11.g14", 0x0200, 0x0100, CRC(14f6c28d) SHA1(8c60974e4607906a3f77260bdd0704af60d596fc) )
ROM_END

ROM_START( mexico86a )
	ROM_REGION( 0x28000, "maincpu", 0 )
	ROM_LOAD( "2.bin",    0x00000, 0x08000, CRC(397c93ad) SHA1(6b28d284cafb86f3efd13033984caa1a221a8a14) ) /* 1st half, main code        */
	ROM_CONTINUE(           0x18000, 0x08000 )             /* 2nd half, banked at 0x8000 */
	ROM_LOAD( "1.bin",    0x08000, 0x10000, CRC(0b93e68e) SHA1(c6fbcce83103e3e71a7a1ef9f18a10622ed6b951) ) /* banked at 0x8000           */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "3x.bin", 0x0000, 0x8000, CRC(abbbf6c4) SHA1(27456d8607e0a246f0c2ad1bc57ee7e4ec37b278) ) // 0x1FEF is 0x2f instead of 0x0f, causes checksum failure, bad?
	ROM_LOAD( "3.bin", 0x0000, 0x8000, CRC(1625b587) SHA1(7336384e13c114915de5e439df5731ce3fc2054a) )

	ROM_REGION( 0x0800, "mcu", 0 )    /* 2k for the microcontroller */
	ROM_LOAD( "68_h.bin",   0x0000, 0x0800, CRC(ff92f816) SHA1(0015c3f2ed014052b3fa376409e3a7cca36fac72) ) // missing in this set, not dumped or never present??

	ROM_REGION( 0x10000, "sub", 0 )    /* 64k for the cpu on the sub board */
	ROM_LOAD( "8.bin",  0x0000, 0x4000, CRC(6a2ad32f) SHA1(42d4b97b25d219902ad215793f1d2c006ffe94dc) )

	ROM_REGION( 0x40000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "4.bin",    0x08000, 0x08000, CRC(57cfdbca) SHA1(89c305c380c3de14a956ee4bc85d3a0d343b638e) )
	ROM_CONTINUE(           0x00000, 0x08000 )
	ROM_LOAD( "5.bin",    0x10000, 0x08000, CRC(e42fa143) SHA1(02d7e0e01af1cecc3952f6355987118098d346c3) )
	ROM_RELOAD(             0x18000, 0x08000 )
	ROM_LOAD( "6.bin",    0x28000, 0x08000, CRC(a4607989) SHA1(6832147603a146c34cc1809e839c8e034d0dacc5) )
	ROM_CONTINUE(           0x20000, 0x08000 )
	ROM_LOAD( "7.bin",    0x30000, 0x08000, CRC(245036b1) SHA1(108d9959de869b4fdf766abeade1486acec13bf2) )
	ROM_RELOAD(             0x38000, 0x08000 )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "n82s129n.1.bin", 0x0000, 0x0100, CRC(be6eb1f0) SHA1(f4d00e9b12bf116bf84edb2ff6caab158094b668) ) /* all proms are 63S141AN or compatible type */
	ROM_LOAD( "n82s129n.3.bin", 0x0100, 0x0100, CRC(3e953444) SHA1(e9c84ca9390fd7c73738a8b681a02e87fbd51bb4) )
	ROM_LOAD( "n82s129n.2.bin", 0x0200, 0x0100, CRC(14f6c28d) SHA1(8c60974e4607906a3f77260bdd0704af60d596fc) )

	ROM_REGION( 0x0300, "plds", 0 )
	ROM_LOAD( "ampal16l8pc.1.bin", 0x0000, 0x0104, CRC(634f3a5b) SHA1(af895a10563e0011c9e6546de2bb61fb8c93bcf4) )
	ROM_LOAD( "ampal16l8pc.3.bin", 0x0000, 0x0104, CRC(f9ce900a) SHA1(c2b4626a4663a3dedd70e04833b9968e58ae372b) )
	ROM_LOAD( "ampal16l8pc.4.bin", 0x0000, 0x0104, CRC(39120b6f) SHA1(d44291f72566f2ad61ab6e612c2c6212076ef541) )
	ROM_LOAD( "ampal16l8pc.5.bin", 0x0000, 0x0104, CRC(1d27f7b9) SHA1(7fe3cb474c599acd7b5fe74bef8f2bae582f2ce9) )
	ROM_LOAD( "ampal16l8pc.6.bin", 0x0000, 0x0104, CRC(9f941c8e) SHA1(34728a572132c23bd2887452ec7ad38504d392d7) )
	ROM_LOAD( "ampal16l8pc.7.bin", 0x0000, 0x0104, CRC(9f941c8e) SHA1(34728a572132c23bd2887452ec7ad38504d392d7) )

	ROM_LOAD( "ampal16r4pc.2.bin", 0x0000, 0x0104, CRC(213a71d1) SHA1(a83b1c089fae72b8216533d0733491c3dc3630af) )
ROM_END




/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1986, kikikai,  0,        kikikai,  kikikai,  mexico86_state, empty_init, ROT90, "Taito Corporation",  "KiKi KaiKai",                                 MACHINE_SUPPORTS_SAVE )
GAME( 1986, knightb,  kikikai,  knightb,  kikikai,  mexico86_state, empty_init, ROT90, "bootleg",            "Knight Boy",                                  MACHINE_SUPPORTS_SAVE )
GAME( 1986, kicknrun, 0,        mexico86, mexico86, mexico86_state, empty_init, ROT0,  "Taito Corporation",  "Kick and Run (World)",                        MACHINE_SUPPORTS_SAVE )
GAME( 1986, kicknrunu,kicknrun, mexico86, mexico86, mexico86_state, empty_init, ROT0,  "Taito America Corp", "Kick and Run (US)",                           MACHINE_SUPPORTS_SAVE )
GAME( 1986, mexico86, kicknrun, mexico86, mexico86, mexico86_state, empty_init, ROT0,  "bootleg",            "Mexico 86 (bootleg of Kick and Run) (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, mexico86a,kicknrun, mexico86, mexico86, mexico86_state, empty_init, ROT0,  "bootleg",            "Mexico 86 (bootleg of Kick and Run) (set 2)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
