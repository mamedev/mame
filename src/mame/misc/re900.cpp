// license:BSD-3-Clause
// copyright-holders:Grull Osgo, Roberto Fresca
/***********************************************************************************

    re900.cpp

    Ruleta RE-900 - Entretenimientos GEMINIS & GENATRON (C) 1993

    Driver by Grull Osgo.
    Additional work by Roberto Fresca.

    Games running on this hardware:

    * Ruleta RE-900,    1993, Entretenimientos GEMINIS.
    * Buena Suerte '94  1994, Entretenimientos GEMINIS.


************************************************************************************

    Hardware Info
    =============

    * Main board:

    1x AMD P80C31BH                 Main CPU.
    1x AY-3-8910                    Sound & I/O System.
    1x TMS-9129-NL                  Video System.

    1x 27C512 EPROM                 Program ROM.
    1x MS6264L-10PC (8Kx8) SRAM     Data Memory (Battery Backed RAM).
    2x TMS4416-15NL (64Kx4) DRAM    Video Memory.

    1x 11.0592 MHz Crystal          CPU clock.
    1x 10.738635 MHz. Crystal       Video System clock.
    1x MAX691CPE                    Power supervisor system, Data RAM Protect and Watchdog.

    1x 3.6 Ni-Cd Battery            Data Memory.
    1x LM380N-8                     1 Channel Audio Amplifier.


************************************************************************************

    Game Info
    =========

    * RE-900 Electronic Roulette.- 6 Players - W/ Random Bonus Multiplier (up to 10x)

    How to play...

    This Roulette allows up to 6 players. To start the machine, turn the Operator Key
    (the Operator Key light will turn green). Whilst this key is turned ON, you
    can insert credits, play, and payout. Once the key is turned OFF (red light), you
    can play, but credits can't be entered/taken.

    You can select the player number through key "L" (the respective player light will
    lite on). Key-In for all 6 players are keys 1-2-3-4-5-6 and Key-Out are Q-W-E-R-T-Y
    respectively. Up-Down-Left-Right to place cursor, and left CTRL to place a bet.
    After a short time without activity, the roulette starts to play, simulating the ball
    with an array of leds...

    We made a full artwork that allows you to play this game with bells and whistles.


    * Buena Suerte ?94 Video Poker Game w/ Double Up feature - 1 Player.

    This game is a reprogrammed version of the Buena Suerte! poker game, to run on this
    GEMINIS RE900 hardware.

    Graphics are worse than original BS, but sounds are improved through the AY-8910.


***********************************************************************************/

#include "emu.h"

#include "cpu/mcs51/mcs51.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "video/tms9928a.h"

#include "speaker.h"

#include "re900.lh"


namespace {

#define MAIN_CLOCK      XTAL(11'059'200)
#define VDP_CLOCK       XTAL(10'730'000)
#define TMS_CLOCK       VDP_CLOCK / 24


class re900_state : public driver_device
{
public:
	re900_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_lamps(*this, "lamp%u", 0U)
	{ }

	void re900(machine_config &config);
	void bs94(machine_config &config);

	void init_re900();

protected:
	virtual void machine_start() override { m_lamps.resolve(); }

private:
	// common
	uint8_t rom_r(offs_t offset);
	void cpu_port_0_w(uint8_t data);
	void watchdog_reset_w(uint8_t data);

	// re900 specific
	uint8_t re_psg_portA_r();
	uint8_t re_psg_portB_r();
	void re_mux_port_A_w(uint8_t data);
	void re_mux_port_B_w(uint8_t data);

	void mem_io(address_map &map) ATTR_COLD;
	void mem_prg(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;

	// re900 specific
	uint8_t m_psg_pa = 0;
	uint8_t m_psg_pb = 0;
	uint8_t m_mux_data = 0;
	uint8_t m_ledant = 0;
	uint8_t m_player = 0;
	uint8_t m_stat_a = 0;
	output_finder<84> m_lamps;
};


/****************
* Read Handlers *
****************/

uint8_t re900_state::re_psg_portA_r()
{
	if ((ioport("IN0")->read() & 0x01) == 0)
	{
		m_lamps[0] = 1;     // Operator Key ON
	}

	else
	{
		m_lamps[0] = 0;     // Operator Key OFF
	}

	return ioport("IN0")->read();
}

uint8_t re900_state::re_psg_portB_r()
{
	uint8_t retval = 0xff;
	logerror("llamada a re_psg_portB_r\n");
	/* This is a hack to select the active player due to Keyboard size restrictions  */

	m_lamps[m_player] = 1;

	if (ioport("IN_S")->read())
	{
		if (!m_stat_a)
		{
			m_lamps[1] = 0;
			m_lamps[2] = 0;
			m_lamps[3] = 0;
			m_lamps[4] = 0;
			m_lamps[5] = 0;
			m_lamps[6] = 0;
			m_player++;

			if (m_player == 7)
			{
				m_player = 1;
			}

			m_lamps[m_player] = 1; /* It shows active player via layout buttons   */
			m_stat_a = 1;
		}
	}

	else
	{
		m_stat_a = 0;
	}
	/* End of Select Player Hack */

	/* "INA": Unified port to share the player Keys among all players - Key In & Key Out have their own buttons on keyboard. */
	switch( m_mux_data )
	{
		case 0x01: retval = (ioport("IN6")->read() | 0x80 ) - (( m_player == 6 ) ? (ioport("INA")->read() | 0x80 ) ^ 0xff: 0x00 ); break; /* Player 6 */
		case 0x02: retval = (ioport("IN5")->read() | 0x80 ) - (( m_player == 5 ) ? (ioport("INA")->read() | 0x80 ) ^ 0xff: 0x00 ); break; /* Player 5 */
		case 0x04: retval = (ioport("IN4")->read() | 0x80 ) - (( m_player == 4 ) ? (ioport("INA")->read() | 0x80 ) ^ 0xff: 0x00 ); break; /* Player 4 */
		case 0x08: retval = (ioport("IN3")->read() | 0x80 ) - (( m_player == 3 ) ? (ioport("INA")->read() | 0x80 ) ^ 0xff: 0x00 ); break; /* Player 3 */
		case 0x10: retval = (ioport("IN2")->read() | 0x80 ) - (( m_player == 2 ) ? (ioport("INA")->read() | 0x80 ) ^ 0xff: 0x00 ); break; /* Player 2 */
		case 0x20: retval = (ioport("IN1")->read() | 0x80 ) - (( m_player == 1 ) ? (ioport("INA")->read() | 0x80 ) ^ 0xff: 0x00 ); break; /* Player 1 */
	}

	return retval;
}


/***********************
*    Write Handlers    *
***********************/

void re900_state::re_mux_port_A_w(uint8_t data)
{
	m_psg_pa = data;
	m_mux_data = ((data >> 2) & 0x3f) ^ 0x3f;
}

void re900_state::re_mux_port_B_w(uint8_t data)
{
	uint8_t led;
	m_psg_pb = data;
	led = (m_psg_pa >> 2) & 0x3f;

	if (data == 0x7f)
	{
		m_lamps[20 + led] = 1;

		if (led != m_ledant)
		{
			m_lamps[20 + m_ledant] = 0;
			m_ledant = led;
		}
	}
}

void re900_state::cpu_port_0_w(uint8_t data)
{
//  m_lamps[7] = 1 ^ ( (data >> 4) & 1); /* Cont. Sal */
//  m_lamps[8] = 1 ^ ( (data >> 5) & 1); /* Cont. Ent */
}

void re900_state::watchdog_reset_w(uint8_t data)
{
	//watchdog_reset_w(space,0,0); /* To do! */
}


/*******************************
*    Memory Map Information    *
*******************************/

void re900_state::mem_prg(address_map &map)
{
	map(0x0000, 0xffff).rom().region("maincpu", 0);
}

void re900_state::mem_io(address_map &map)
{
	map(0x0000, 0xbfff).rom().region("maincpu", 0);
	map(0xc000, 0xdfff).ram().share("nvram");
	map(0xe000, 0xefff).w(FUNC(re900_state::watchdog_reset_w));
	map(0xe000, 0xe001).w("tms9128", FUNC(tms9928a_device::write));
	map(0xe800, 0xe801).w("ay_re900", FUNC(ay8910_device::address_data_w));
	map(0xe802, 0xe802).r("ay_re900", FUNC(ay8910_device::data_r));
}


/************************
*      Input ports      *
************************/

static INPUT_PORTS_START( re900 )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Operator Key") PORT_TOGGLE PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Auditor Key")  PORT_TOGGLE PORT_CODE(KEYCODE_9)

	PORT_START("INA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Bet")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Left")         PORT_CODE(KEYCODE_LEFT)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Right")        PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Up")           PORT_CODE(KEYCODE_UP)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Down")         PORT_CODE(KEYCODE_DOWN)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("P1 Key-In")   PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("P1 Key-Out")  PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("P2 Key-In")   PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("P2 Key-Out")  PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("P3 Key-In")   PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("P3 Key-Out")  PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("P4 Key-In")   PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("P4 Key-Out")  PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("P5 Key-In")   PORT_CODE(KEYCODE_5)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("P5 Key-Out")  PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("P6 Key-In")   PORT_CODE(KEYCODE_6)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("P6 Key-Out")  PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN_S")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("Player Select")  PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( bs94 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP ) PORT_NAME("Double")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_NAME("Deal / Draw")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_CANCEL )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Operator") PORT_TOGGLE PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Card High")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_NAME("Card Low")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Auditor") PORT_CODE(KEYCODE_9)
INPUT_PORTS_END

/***************************
*      Machine Driver      *
***************************/

void re900_state::re900(machine_config &config)
{
	/* basic machine hardware */
	i8051_device &maincpu(I8051(config, m_maincpu, MAIN_CLOCK));
	maincpu.set_addrmap(AS_PROGRAM, &re900_state::mem_prg);
	maincpu.set_addrmap(AS_IO, &re900_state::mem_io);
	maincpu.port_out_cb<0>().set(FUNC(re900_state::cpu_port_0_w));

	/* video hardware */
	tms9128_device &vdp(TMS9128(config, "tms9128", VDP_CLOCK));   /* TMS9128NL on the board */
	vdp.set_screen("screen");
	vdp.set_vram_size(0x4000);
	//vdp.int_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* sound hardware   */
	SPEAKER(config, "mono").front_center();
	ay8910_device &ay_re900(AY8910(config, "ay_re900", TMS_CLOCK)); /* From TMS9128NL - Pin 37 (GROMCLK) */
	ay_re900.port_a_read_callback().set(FUNC(re900_state::re_psg_portA_r));
	ay_re900.port_b_read_callback().set(FUNC(re900_state::re_psg_portB_r));
	ay_re900.port_a_write_callback().set(FUNC(re900_state::re_mux_port_A_w));
	ay_re900.port_b_write_callback().set(FUNC(re900_state::re_mux_port_B_w));
	ay_re900.add_route(ALL_OUTPUTS, "mono", 0.5);
}

void re900_state::bs94(machine_config &config)
{
	re900(config);

	/* sound hardware   */
	auto &ay_re900(*subdevice<ay8910_device>("ay_re900"));
	ay_re900.port_a_read_callback().set_ioport("IN0");
	ay_re900.port_b_read_callback().set_ioport("IN1");
	ay_re900.port_a_write_callback().set_nop();
	ay_re900.port_b_write_callback().set_nop();
}


/*************************
*        Rom Load        *
*************************/

ROM_START( re900 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "re900.bin", 0x0000, 0x10000, CRC(967ae944) SHA1(104bab79fd50a8e38ae15058dbe47a59f1ec4b05) )
ROM_END

ROM_START( bs94 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bs94.bin",  0x0000, 0x10000, CRC(bbd484ce) SHA1(4128e488ca806842c3639e05c4c9cf4c0da2990d) )
ROM_END


/************************
*      Driver Init      *
************************/

void re900_state::init_re900()
{
	m_player = 1;
	m_stat_a = 1;
	m_psg_pa = m_psg_pb = m_mux_data = m_ledant = 0;

	save_item(NAME(m_psg_pa));
	save_item(NAME(m_psg_pb));
	save_item(NAME(m_mux_data));
	save_item(NAME(m_ledant));
	save_item(NAME(m_player));
	save_item(NAME(m_stat_a));
}

} // Anonymous namespace


/*************************
*      Game Drivers      *
*************************/

//     YEAR  NAME   PARENT  MACHINE  INPUT  CLASS        INIT        ROT    COMPANY                     FULLNAME            FLAGS                  LAYOUT
GAMEL( 1993, re900, 0,      re900,   re900, re900_state, init_re900, ROT90, "Entretenimientos GEMINIS", "Ruleta RE-900",    MACHINE_SUPPORTS_SAVE, layout_re900 )
GAME(  1994, bs94,  0,      bs94,    bs94,  re900_state, empty_init, ROT0,  "Entretenimientos GEMINIS", "Buena Suerte '94", MACHINE_SUPPORTS_SAVE )
