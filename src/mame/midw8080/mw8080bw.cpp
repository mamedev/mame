// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Tormod Tjaberg, Mirko Buffoni,Lee Taylor, Valerio Verrando, Zsolt Vasvari
// thanks-to:Michael Strutts, Marco Cassili
/***************************************************************************

    Midway 8080-based black and white hardware

    driver by Michael Strutts, Nicola Salmoria, Tormod Tjaberg, Mirko Buffoni,
    Lee Taylor, Valerio Verrando, Marco Cassili, Zsolt Vasvari and others

    Games supported:
        * Sea Wolf
        * Gun Fight
        * Tornado Baseball
        * (Datsun) 280-ZZZAP
        * Amazing Maze
        * Boot Hill
        * Checkmate
        * Desert Gun
        * Road Runner
        * Double Play
        * Laguna Racer
        * Guided Missile
        * M-4
        * Clowns
        * Space Walk
        * Extra Inning
        * Shuffleboard
        * Dog Patch
        * Space Encounters
        * Phantom II
        * Bowling Alley
        * Space Invaders
        * Blue Shark
        * Space Invaders II (Midway, cocktail version)
        * Space Invaders Deluxe (cocktail version)

    Other games on this basic hardware:
        * Gun Fight (cocktail version)
        * 4 Player Bowling Alley (cocktail version)

    Notes:
        * Most of these games do not actually use the MB14241 shifter IC,
          but instead implement equivalent functionality using a bunch of
          standard 74XX IC's.
        * The Amazing Maze Game" on title screen, but manual, flyer,
          cabinet side art all call it just "Amazing Maze"
        * Desert Gun was originally named Road Runner. The name was changed
          when Midway merged with Bally who had a game by the same title
        * Guided Missile: It is a misconception that that this is the same
          game as Taito's "Missile X". The latter does not run on a CPU,
          akin to Midway's Gun Fight vs Taito's Western Gun.
        * Space Invaders: Taito imported this licensed version because of
          short supply in Japan. The game is called "Space Invaders M"
          The M stands for Midway.
        * "Gun Fight" (Midway) is ported version of "Western Gun" (Taito)
        * in Japan, Taito released "Tornado Baseball" as "Ball Park",
          "Extra Inning" as "Ball Park II".
        * "Road Runner" (Midway): To access the test mode, adjust the
          "Extended Time At" dipswitch to select "Test Mode". While in
          the test mode, hold Fire until the gun alignment screen appears.

    Known issues/to-do's:
        * Space Encounters: verify strobe light frequency
        * Phantom II: cloud generator is implemented according to the schematics,
           but it doesn't look right.  Cloud color mixing to be verified as well
        * Dog Patch: verify all assumptions
        * Blue Shark - all sounds are suspicious.  Why is there no diver kill sound?
           Why does the shark sound so bad and appear rarely?
           Schematics need to be verified against real board.


****************************************************************************

    Memory map

****************************************************************************

    ========================================================================
    MAIN CPU memory address space
    ========================================================================

    Address (15-bits) Dir Data     Description
    ----------------- --- -------- -----------------------
    x0xxxxxxxxxxxxx   R   xxxxxxxx Program ROM (various amounts populated)
    -1xxxxxxxxxxxxx   R/W xxxxxxxx Video RAM (256x256x1 bit display)
                                   Portion in VBLANK region used as work RAM
    Legend: (x)   bit significant
            (-)   bit ignored
            (0/1) bit must be given value

    The I/O address space is used differently from game to game.


****************************************************************************

    Horizontal sync chain:

        The horizontal synch chain is clocked by the pixel clock, which
        is the master clock divided by four via the counter @ C7 and
        the D flip-flop at B5.

        A 4-bit binary counter @ D5 counts 1H,2H,4H and 8H. This counter
        cascades into another 4-bit binary counter @ E5, which counts
        16H,32H,64H and 128H. The carry-out of this counter enables the
        vertical sync chain. It also clocks a D flip-flop @ A5(1). The
        output of the flip-flop is HBLANK and it is also used to reset
        the two counters. When HBLANK is high, they are reset to 192,
        otherwise to 0, thus giving 320 total pixels.

        Clock = 19.968000/4MHz
        HBLANK ends at H = 0
        HBLANK begins at H = 256 (0x100)
        HSYNC begins at H = 272 (0x110)
        HSYNC ends at H = 288 (0x120)
        HTOTAL = 320 (0x140)

    Vertical sync chain:

        The vertical synch chain is also clocked by the clock, but it is
        only enabled counting in HBLANK, when the horizontal synch chain
        overflows.

        A 4-bit binary counter @ E6 counts 1V,2V,4V and 8V. This counter
        cascades into another 4-bit binary counter @ E7, which counts
        16V,32V,64V and 128V. The carry-out of this counter clocks a
        D flip-flop @ A5(2). The output of the flip-flop is VBLANK and
        it is also used to reset the two counters. When VBLANK is high,
        they are reset to 218, otherwise to 32, thus giving
        (256-218)+(256-32)=262 total pixels.

        Clock = 19.968000/4MHz
        VBLANK ends at V = 0
        VBLANK begins at V = 224 (0x0e0)
        VSYNC begins at V = 236 (0x0ec)
        VSYNC ends at V = 240 (0x0f0)
        VTOTAL = 262 (0x106)

    Interrupts:

        The CPU's INT line is asserted via a D flip-flop at E3.
        The flip-flop is clocked by the expression (!(64V | !128V) | VBLANK).
        According to this, the LO to HI transition happens when the vertical
        sync chain is 0x80 and 0xda and VBLANK is 0 and 1, respectively.
        These correspond to lines 96 and 224 as displayed.
        The interrupt vector is provided by the expression:
        0xc7 | (64V << 4) | (!64V << 3), giving 0xcf and 0xd7 for the vectors.
        The flip-flop, thus the INT line, is later cleared by the CPU via
        one of its memory access control signals.

****************************************************************************/

#include "emu.h"
#include "mw8080bw.h"

#include "cpu/i8085/i8085.h"
#include "machine/rescap.h"

#include <algorithm>

#include "280zzzap.lh"
#include "clowns.lh"
#include "gunfight.lh"
#include "invaders.lh"
#include "invad2ct.lh"
#include "lagunar.lh"
#include "maze.lh"
#include "phantom2.lh"
#include "seawolf.lh"
#include "spacwalk.lh"
#include "spcenctr.lh"



INPUT_CHANGED_MEMBER(mw8080bw_state::direct_coin_count)
{
	machine().bookkeeping().coin_counter_w(0, newval);
}



/*************************************
 *
 *  Interrupt generation
 *
 *************************************/

uint8_t mw8080bw_state::vpos_to_vysnc_chain_counter( int vpos )
{
	/* convert from a vertical position to the actual values on the vertical sync counters */
	uint8_t counter;
	int vblank = (vpos >= MW8080BW_VBSTART);

	if (vblank)
		counter = vpos - MW8080BW_VBSTART + MW8080BW_VCOUNTER_START_VBLANK;
	else
		counter = vpos + MW8080BW_VCOUNTER_START_NO_VBLANK;

	return counter;
}


int mw8080bw_state::vysnc_chain_counter_to_vpos( uint8_t counter, int vblank )
{
	/* convert from the vertical sync counters to an actual vertical position */
	int vpos;

	if (vblank)
		vpos = counter - MW8080BW_VCOUNTER_START_VBLANK + MW8080BW_VBSTART;
	else
		vpos = counter - MW8080BW_VCOUNTER_START_NO_VBLANK;

	return vpos;
}


TIMER_CALLBACK_MEMBER(mw8080bw_state::interrupt_trigger)
{
	int const vpos = m_screen->vpos();
	uint8_t const counter = vpos_to_vysnc_chain_counter(vpos);

	if (m_int_enable)
	{
		m_maincpu->set_input_line(0, ASSERT_LINE);
		m_interrupt_time = machine().time();
	}
	else
		m_maincpu->set_input_line(0, CLEAR_LINE);

	// set up for next interrupt
	uint8_t next_counter;
	int next_vblank;
	if (counter == MW8080BW_INT_TRIGGER_COUNT_1)
	{
		next_counter = MW8080BW_INT_TRIGGER_COUNT_2;
		next_vblank = MW8080BW_INT_TRIGGER_VBLANK_2;
	}
	else
	{
		next_counter = MW8080BW_INT_TRIGGER_COUNT_1;
		next_vblank = MW8080BW_INT_TRIGGER_VBLANK_1;
	}

	int const next_vpos = vysnc_chain_counter_to_vpos(next_counter, next_vblank);
	m_interrupt_timer->adjust(m_screen->time_until_pos(next_vpos));
}


void mw8080bw_state::int_enable_w(int state)
{
	m_int_enable = state;
}


IRQ_CALLBACK_MEMBER(mw8080bw_state::interrupt_vector)
{
	int vpos = m_screen->vpos();
	// MAME scheduling quirks cause this to happen more often than you might think, in fact far too often
	if (machine().time() < m_interrupt_time)
		vpos++;
	uint8_t counter = vpos_to_vysnc_chain_counter(vpos);
	uint8_t vector = 0xc7 | ((counter & 0x40) >> 2) | ((~counter & 0x40) >> 3);

	m_maincpu->set_input_line(0, CLEAR_LINE);
	return vector;
}



/*************************************
 *
 *  Machine setup
 *
 *************************************/

void mw8080bw_state::machine_start()
{
	m_interrupt_timer = timer_alloc(FUNC(mw8080bw_state::interrupt_trigger), this);
}


void mw8080bw_state::machine_reset()
{
	int vpos = vysnc_chain_counter_to_vpos(MW8080BW_INT_TRIGGER_COUNT_1, MW8080BW_INT_TRIGGER_VBLANK_1);
	m_interrupt_timer->adjust(m_screen->time_until_pos(vpos));

	m_interrupt_time = attotime::zero;
}



/*************************************
 *
 *  Special shifter circuit
 *
 *************************************/

u8 mw8080bw_state::mw8080bw_shift_result_rev_r()
{
	uint8_t ret = m_mb14241->shift_result_r();

	return bitswap<8>(ret,0,1,2,3,4,5,6,7);
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void mw8080bw_state::main_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x1fff).rom().nopw();
	map(0x2000, 0x3fff).mirror(0x4000).ram().share("main_ram");
	map(0x4000, 0x5fff).rom().nopw();
}



/*************************************
 *
 *  Root driver structure
 *
 *************************************/

void mw8080bw_state::mw8080bw_root(machine_config &config)
{
	/* basic machine hardware */
	i8080_cpu_device &maincpu(I8080(config, m_maincpu, MW8080BW_CPU_CLOCK));
	maincpu.set_addrmap(AS_PROGRAM, &mw8080bw_state::main_map);
	maincpu.set_irq_acknowledge_callback(FUNC(mw8080bw_state::interrupt_vector));
	maincpu.out_inte_func().set(FUNC(mw8080bw_state::int_enable_w));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(MW8080BW_PIXEL_CLOCK, MW8080BW_HTOTAL, MW8080BW_HBEND, MW8080BW_HPIXCOUNT, MW8080BW_VTOTAL, MW8080BW_VBEND, MW8080BW_VBSTART);
	m_screen->set_screen_update(FUNC(mw8080bw_state::screen_update_mw8080bw));
}



/*************************************
 *
 *  Sea Wolf (PCB #596)
 *
 *************************************/

void seawolf_state::machine_start()
{
	mw8080bw_state::machine_start();

	m_exp_lamps.resolve();
	m_torp_lamps.resolve();
	m_ready_lamp.resolve();
	m_reload_lamp.resolve();
}


void seawolf_state::explosion_lamp_w(u8 data)
{
	/*  D0-D3 are column drivers and D4-D7 are row drivers.
	    The following table shows values that light up individual lamps.

	    D7 D6 D5 D4 D3 D2 D1 D0   Function
	    --------------------------------------------------------------------------------------
	     0  0  0  1  1  0  0  0   Explosion Lamp 0
	     0  0  0  1  0  1  0  0   Explosion Lamp 1
	     0  0  0  1  0  0  1  0   Explosion Lamp 2
	     0  0  0  1  0  0  0  1   Explosion Lamp 3
	     0  0  1  0  1  0  0  0   Explosion Lamp 4
	     0  0  1  0  0  1  0  0   Explosion Lamp 5
	     0  0  1  0  0  0  1  0   Explosion Lamp 6
	     0  0  1  0  0  0  0  1   Explosion Lamp 7
	     0  1  0  0  1  0  0  0   Explosion Lamp 8
	     0  1  0  0  0  1  0  0   Explosion Lamp 9
	     0  1  0  0  0  0  1  0   Explosion Lamp A
	     0  1  0  0  0  0  0  1   Explosion Lamp B
	     1  0  0  0  1  0  0  0   Explosion Lamp C
	     1  0  0  0  0  1  0  0   Explosion Lamp D
	     1  0  0  0  0  0  1  0   Explosion Lamp E
	     1  0  0  0  0  0  0  1   Explosion Lamp F
	*/
	static constexpr u8 BITS_FOR_LAMPS[] =
	{
		0x18, 0x14, 0x12, 0x11,
		0x28, 0x24, 0x22, 0x21,
		0x48, 0x44, 0x42, 0x41,
		0x88, 0x84, 0x82, 0x81
	};

	for (int i = 0; i < 16; i++)
	{
		u8 const bits_for_lamp(BITS_FOR_LAMPS[i]);
		m_exp_lamps[i] = ((data & bits_for_lamp) == bits_for_lamp) ? 1 : 0;
	}
}


void seawolf_state::periscope_lamp_w(u8 data)
{
	/* the schematics and the connecting diagrams show the torpedo light
	   order differently, but this order is confirmed by the software */
	m_torp_lamps[3] = BIT(data, 0);
	m_torp_lamps[2] = BIT(data, 1);
	m_torp_lamps[1] = BIT(data, 2);
	m_torp_lamps[0] = BIT(data, 3);

	m_ready_lamp = BIT(data, 4);

	m_reload_lamp = BIT(data, 5);
}


ioport_value seawolf_state::erase_input_r()
{
	return m_erase_sw->read() & m_erase_dip->read();
}


void seawolf_state::io_map(address_map &map)
{
	map.global_mask(0x7);
	map(0x00, 0x00).mirror(0x04).r(FUNC(seawolf_state::mw8080bw_shift_result_rev_r));
	map(0x01, 0x01).mirror(0x04).portr("IN0");
	map(0x02, 0x02).mirror(0x04).portr("IN1");
	map(0x03, 0x03).mirror(0x04).r(m_mb14241, FUNC(mb14241_device::shift_result_r));

	map(0x01, 0x01).w(FUNC(seawolf_state::explosion_lamp_w));
	map(0x02, 0x02).w(FUNC(seawolf_state::periscope_lamp_w));
	map(0x03, 0x03).w(m_mb14241, FUNC(mb14241_device::shift_data_w));
	map(0x04, 0x04).w(m_mb14241, FUNC(mb14241_device::shift_count_w));
	map(0x05, 0x05).w("soundboard", FUNC(seawolf_audio_device::write));
}


// the 30 position encoder is verified
static const ioport_value seawolf_controller_table[30] =
{
	0x1e, 0x1c, 0x1d, 0x19, 0x18, 0x1a, 0x1b, 0x13,
	0x12, 0x10, 0x11, 0x15, 0x14, 0x16, 0x17, 0x07,
	0x06, 0x04, 0x05, 0x01, 0x00, 0x02, 0x03, 0x0b,
	0x0a, 0x08, 0x09, 0x0d, 0x0c, 0x0e
};


static INPUT_PORTS_START( seawolf )
	PORT_START("IN0")
	// the grey code is inverted by buffers
	// The wiring diagram shows the encoder has 32 positions.
	// But there is a hand written table on the game logic sheet showing only 30 positions.
	// The actual commutator pcb (encoder) has 30 positions and works like the table says.
	PORT_BIT( 0x1f, 0x0f, IPT_POSITIONAL ) PORT_POSITIONS(30) PORT_REMAP_TABLE(seawolf_controller_table) PORT_INVERT PORT_SENSITIVITY(20) PORT_KEYDELTA(8) PORT_CENTERDELTA(0) PORT_NAME("Periscope axis") PORT_CROSSHAIR(X, ((float)MW8080BW_HPIXCOUNT - 28) / MW8080BW_HPIXCOUNT, 16.0 / MW8080BW_HPIXCOUNT, 32.0 / MW8080BW_VBSTART)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_DIPNAME( 0xc0, 0x40, DEF_STR( Game_Time ) ) PORT_CONDITION("IN1", 0xe0, NOTEQUALS, 0xe0) PORT_DIPLOCATION("G4:1,2")
	PORT_DIPSETTING(    0x00, "60 seconds + 20 extended" ) PORT_CONDITION("IN1", 0xe0, NOTEQUALS, 0x00)
	PORT_DIPSETTING(    0x40, "70 seconds + 20 extended" ) PORT_CONDITION("IN1", 0xe0, NOTEQUALS, 0x00)
	PORT_DIPSETTING(    0x80, "80 seconds + 20 extended" ) PORT_CONDITION("IN1", 0xe0, NOTEQUALS, 0x00)
	PORT_DIPSETTING(    0xc0, "90 seconds + 20 extended" ) PORT_CONDITION("IN1", 0xe0, NOTEQUALS, 0x00)
	PORT_DIPSETTING(    0x00, "60 seconds" ) PORT_CONDITION("IN1", 0xe0, EQUALS, 0x00)
	PORT_DIPSETTING(    0x40, "70 seconds" ) PORT_CONDITION("IN1", 0xe0, EQUALS, 0x00)
	PORT_DIPSETTING(    0x80, "80 seconds" ) PORT_CONDITION("IN1", 0xe0, EQUALS, 0x00)
	PORT_DIPSETTING(    0xc0, "90 seconds" ) PORT_CONDITION("IN1", 0xe0, EQUALS, 0x00)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coinage ) ) PORT_CONDITION("IN1", 0xe0, NOTEQUALS, 0xe0) PORT_DIPLOCATION("G4:3,4")
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(seawolf_state::erase_input_r))
	PORT_DIPNAME( 0xe0, 0x60, "Extended Time At" ) PORT_DIPLOCATION("G4:6,7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPSETTING(    0x20, "2000" )
	PORT_DIPSETTING(    0x40, "3000" )
	PORT_DIPSETTING(    0x60, "4000" )
	PORT_DIPSETTING(    0x80, "5000" )
	PORT_DIPSETTING(    0xa0, "6000" )
	PORT_DIPSETTING(    0xc0, "7000" )
	PORT_DIPSETTING(    0xe0, "Test Mode" )

	// 2 fake ports for the 'Reset High Score' input, which has a DIP to enable it
	PORT_START(SEAWOLF_ERASE_SW_PORT_TAG)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_MEMORY_RESET ) PORT_NAME("Reset High Score")
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START(SEAWOLF_ERASE_DIP_PORT_TAG)
	PORT_DIPNAME( 0x01, 0x01, "Enable Reset High Score Button" ) PORT_DIPLOCATION("G4:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


void seawolf_state::seawolf(machine_config &config)
{
	mw8080bw_root(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_IO, &seawolf_state::io_map);

	// there is no watchdog

	// add shifter
	MB14241(config, m_mb14241);

	// audio hardware
	SEAWOLF_AUDIO(config, "soundboard");
}



/*************************************
 *
 *  Gun Fight (PCB #597)
 *
 *************************************/

void gunfight_state::io_w(offs_t offset, u8 data)
{
	if (offset & 0x01)
		m_soundboard->write(data);

	if (offset & 0x02)
		m_mb14241->shift_count_w(data);

	if (offset & 0x04)
		m_mb14241->shift_data_w(data);
}


void gunfight_state::io_map(address_map &map)
{
	map.global_mask(0x7);
	map(0x00, 0x00).mirror(0x04).portr("IN0");
	map(0x01, 0x01).mirror(0x04).portr("IN1");
	map(0x02, 0x02).mirror(0x04).portr("IN2");
	map(0x03, 0x03).mirror(0x04).r(m_mb14241, FUNC(mb14241_device::shift_result_r));

	map(0x00, 0x07).w(FUNC(gunfight_state::io_w)); // no decoder, just 3 AND gates
}


static const ioport_value gunfight_controller_table[7] =
{
	0x06, 0x02, 0x00, 0x04, 0x05, 0x01, 0x03
};


static INPUT_PORTS_START( gunfight )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x70, 0x30, IPT_POSITIONAL_V ) PORT_POSITIONS(7) PORT_REMAP_TABLE(gunfight_controller_table) PORT_INVERT PORT_SENSITIVITY(5) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_CODE_DEC(KEYCODE_N) PORT_CODE_INC(KEYCODE_H) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x70, 0x30, IPT_POSITIONAL_V ) PORT_POSITIONS(7) PORT_REMAP_TABLE(gunfight_controller_table) PORT_INVERT PORT_SENSITIVITY(5) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_CODE_DEC(KEYCODE_M) PORT_CODE_INC(KEYCODE_J) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_START("IN2")
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("C1:1,2,3,4")
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 4C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 4C_4C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 3C_3C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 2C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x30, 0x10, DEF_STR( Game_Time ) ) PORT_DIPLOCATION("C1:5,6")
	PORT_DIPSETTING(    0x00, "60 seconds" )
	PORT_DIPSETTING(    0x10, "70 seconds" )
	PORT_DIPSETTING(    0x20, "80 seconds" )
	PORT_DIPSETTING(    0x30, "90 seconds" )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START1 )
INPUT_PORTS_END


void gunfight_state::gunfight(machine_config &config)
{
	mw8080bw_root(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_IO, &gunfight_state::io_map);

	// there is no watchdog

	// add shifter
	MB14241(config, m_mb14241);

	// audio hardware
	GUNFIGHT_AUDIO(config, m_soundboard);
}



/*************************************
 *
 *  Tornado Baseball (PCB #605)
 *
 *  Notes:
 *  -----
 *
 *  In baseball, the Visitor team always hits first and the Home team pitches (throws the ball).
 *  This rule gives an advantage to the Home team because they get to score last in any baseball game.
 *  It is also the team that pitches that controls the player on the field, which, in this game,
 *  is limited to moving the 3 outfielders left and right.
 *
 *  There are 3 types of cabinets using the same software:
 *
 *  Old Upright: One of everything
 *
 *  New Upright: One fielding/pitching controls, but two (Left/Right) hitting buttons
 *
 *  Cocktail:    Two of everything, but the pitching/fielding controls are swapped
 *
 *  Interestingly, the "Whistle" sound effect is controlled by a different
 *  bit on the Old Upright cabinet than the other two types.
 *
 *************************************/

#define TORNBASE_L_HIT_PORT_TAG         ("LHIT")
#define TORNBASE_R_HIT_PORT_TAG         ("RHIT")
#define TORNBASE_L_PITCH_PORT_TAG       ("LPITCH")
#define TORNBASE_R_PITCH_PORT_TAG       ("RPITCH")
#define TORNBASE_SCORE_SW_PORT_TAG      ("SCORESW")
#define TORNBASE_SCORE_DIP_PORT_TAG     ("ERASEDIP")
#define TORNBASE_CAB_TYPE_PORT_TAG      ("CAB")


uint8_t mw8080bw_state::tornbase_get_cabinet_type()
{
	return ioport(TORNBASE_CAB_TYPE_PORT_TAG)->read();
}


ioport_value mw8080bw_state::tornbase_hit_left_input_r()
{
	return ioport(TORNBASE_L_HIT_PORT_TAG)->read();
}


ioport_value mw8080bw_state::tornbase_hit_right_input_r()
{
	uint32_t ret;

	switch (tornbase_get_cabinet_type())
	{
	case TORNBASE_CAB_TYPE_UPRIGHT_OLD:
		ret = ioport(TORNBASE_L_HIT_PORT_TAG)->read();
		break;

	case TORNBASE_CAB_TYPE_UPRIGHT_NEW:
	case TORNBASE_CAB_TYPE_COCKTAIL:
	default:
		ret = ioport(TORNBASE_R_HIT_PORT_TAG)->read();
		break;
	}

	return ret;
}


ioport_value mw8080bw_state::tornbase_pitch_left_input_r()
{
	uint32_t ret;

	switch (tornbase_get_cabinet_type())
	{
	case TORNBASE_CAB_TYPE_UPRIGHT_OLD:
	case TORNBASE_CAB_TYPE_UPRIGHT_NEW:
		ret = ioport(TORNBASE_L_PITCH_PORT_TAG)->read();
		break;

	case TORNBASE_CAB_TYPE_COCKTAIL:
	default:
		ret = ioport(TORNBASE_R_PITCH_PORT_TAG)->read();
		break;
	}

	return ret;
}


ioport_value mw8080bw_state::tornbase_pitch_right_input_r()
{
	return ioport(TORNBASE_L_PITCH_PORT_TAG)->read();
}


ioport_value mw8080bw_state::tornbase_score_input_r()
{
	return ioport(TORNBASE_SCORE_SW_PORT_TAG)->read() &
			ioport(TORNBASE_SCORE_DIP_PORT_TAG)->read();
}


void mw8080bw_state::tornbase_io_w(offs_t offset, uint8_t data)
{
	if (offset & 0x01)
		tornbase_audio_w(data);

	if (offset & 0x02)
		m_mb14241->shift_count_w(data);

	if (offset & 0x04)
		m_mb14241->shift_data_w(data);
}


void mw8080bw_state::tornbase_io_map(address_map &map)
{
	map.global_mask(0x7);
	map(0x00, 0x00).mirror(0x04).portr("IN0");
	map(0x01, 0x01).mirror(0x04).portr("IN1");
	map(0x02, 0x02).mirror(0x04).portr("IN2");
	map(0x03, 0x03).mirror(0x04).r(m_mb14241, FUNC(mb14241_device::shift_result_r));

	/* no decoder, just 3 AND gates */
	map(0x00, 0x07).w(FUNC(mw8080bw_state::tornbase_io_w));
}


static INPUT_PORTS_START( tornbase )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(mw8080bw_state::tornbase_hit_left_input_r))
	PORT_BIT( 0x7e, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(mw8080bw_state::tornbase_pitch_left_input_r))
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unused ) ) PORT_CONDITION("IN2", 0x80, EQUALS, 0x00) PORT_DIPLOCATION("B1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(mw8080bw_state::tornbase_hit_right_input_r))
	PORT_BIT( 0x7e, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(mw8080bw_state::tornbase_pitch_right_input_r))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)  /* not connected */

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )  /* schematics shows it as "START", but not used by the software */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(mw8080bw_state::tornbase_score_input_r))
	PORT_DIPNAME( 0x78, 0x40, DEF_STR( Coinage ) ) PORT_CONDITION("IN2", 0x80, EQUALS, 0x00) PORT_DIPLOCATION("B1:2,3,4,5")
	PORT_DIPSETTING(    0x18, "4 Coins/1 Inning" )
	PORT_DIPSETTING(    0x10, "3 Coins/1 Inning" )
	PORT_DIPSETTING(    0x38, "4 Coins/2 Innings" )
	PORT_DIPSETTING(    0x08, "2 Coins/1 Inning" )
	PORT_DIPSETTING(    0x30, "3 Coins/2 Innings" )
	PORT_DIPSETTING(    0x28, "2 Coins/2 Innings" )
	PORT_DIPSETTING(    0x00, "1 Coin/1 Inning" )
	PORT_DIPSETTING(    0x58, "4 Coins/4 Innings" )
	PORT_DIPSETTING(    0x50, "3 Coins/4 Innings" )
	PORT_DIPSETTING(    0x48, "2 Coins/4 Innings" )
	PORT_DIPSETTING(    0x20, "1 Coin/2 Innings" )
	PORT_DIPSETTING(    0x40, "1 Coin/4 Innings" )
	PORT_DIPSETTING(    0x78, "4 Coins/9 Innings" )
	PORT_DIPSETTING(    0x70, "3 Coins/9 Innings" )
	PORT_DIPSETTING(    0x68, "2 Coins/9 Innings" )
	PORT_DIPSETTING(    0x60, "1 Coin/9 Innings" )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_HIGH, "B1:6" )

	/* fake ports to handle the various input configurations based on cabinet type */
	PORT_START(TORNBASE_L_HIT_PORT_TAG)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Hit") PORT_PLAYER(1)
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START(TORNBASE_R_HIT_PORT_TAG)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Hit") PORT_PLAYER(2)
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START(TORNBASE_L_PITCH_PORT_TAG)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Move Outfield Left") PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P1 Move Outfield Right") PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_NAME("P1 Pitch Left") PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_NAME("P1 Pitch Right") PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_NAME("P1 Pitch Slow") PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_NAME("P1 Pitch Fast") PORT_PLAYER(1)
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START(TORNBASE_R_PITCH_PORT_TAG)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P2 Move Outfield Left") PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P2 Move Outfield Right") PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_NAME("P2 Pitch Left") PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_NAME("P2 Pitch Right") PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_NAME("P2 Pitch Slow") PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_NAME("P2 Pitch Fast") PORT_PLAYER(2)
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	/* 2 fakes port for the 'ERASE' input, which has a DIP to enable it.
	   This switch is not actually used by the software */
	PORT_START(TORNBASE_SCORE_SW_PORT_TAG)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("SCORE Input (Not Used)") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START(TORNBASE_SCORE_DIP_PORT_TAG)
	PORT_DIPNAME( 0x01, 0x01, "Enable SCORE Input" ) PORT_DIPLOCATION("B1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	/* fake port for cabinet type */
	PORT_START(TORNBASE_CAB_TYPE_PORT_TAG)
	PORT_CONFNAME( 0x03, TORNBASE_CAB_TYPE_UPRIGHT_NEW, DEF_STR( Cabinet ) )
	PORT_CONFSETTING( TORNBASE_CAB_TYPE_UPRIGHT_OLD, "Upright/w One Hit Button" )
	PORT_CONFSETTING( TORNBASE_CAB_TYPE_UPRIGHT_NEW, "Upright/w P1/P2 Hit Buttons" )
	PORT_CONFSETTING( TORNBASE_CAB_TYPE_COCKTAIL, DEF_STR( Cocktail ) )
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


void mw8080bw_state::tornbase(machine_config &config)
{
	mw8080bw_root(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_IO, &mw8080bw_state::tornbase_io_map);
	/* there is no watchdog */

	/* add shifter */
	MB14241(config, m_mb14241);

	/* audio hardware */
	tornbase_audio(config);
}



/*************************************
 *
 *  280 ZZZAP (PCB #610) / Laguna Racer (PCB #622)
 *
 *************************************/

void zzzap_state::io_map(address_map &map)
{
	map.global_mask(0x7);
	map(0x00, 0x00).mirror(0x04).portr("IN0");
	map(0x01, 0x01).mirror(0x04).portr("IN1");
	map(0x02, 0x02).mirror(0x04).portr("IN2");
	map(0x03, 0x03).mirror(0x04).r(m_mb14241, FUNC(mb14241_device::shift_result_r));

	map(0x02, 0x02).w("soundboard", FUNC(zzzap_common_audio_device::p1_w));
	map(0x03, 0x03).w(m_mb14241, FUNC(mb14241_device::shift_data_w));
	map(0x04, 0x04).w(m_mb14241, FUNC(mb14241_device::shift_count_w));
	map(0x05, 0x05).w("soundboard", FUNC(zzzap_common_audio_device::p2_w));
	map(0x07, 0x07).w(m_watchdog, FUNC(watchdog_timer_device::reset_w));
}


static INPUT_PORTS_START( zzzap )
	PORT_START("IN0")
	PORT_BIT( 0x0f, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(64) PORT_PLAYER(1)   /* accelerator */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_TOGGLE PORT_NAME("P1 Shift") PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )    /* not connected */
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_START1 )    /* start button, but never used? */

	PORT_START("IN1")   /* steering wheel */
	PORT_BIT( 0xff, 0x7f, IPT_PADDLE ) PORT_MINMAX(0x01,0xfe) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("IN2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) ) PORT_CONDITION("IN2", 0x0c, NOTEQUALS, 0x04) PORT_DIPLOCATION("E3:1,2")
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Game_Time ) ) PORT_DIPLOCATION("E3:3,4")
	PORT_DIPSETTING(    0x0c, "60 seconds + 30 extended" ) PORT_CONDITION("IN2", 0x30, NOTEQUALS, 0x20)
	PORT_DIPSETTING(    0x00, "80 seconds + 40 extended" ) PORT_CONDITION("IN2", 0x30, NOTEQUALS, 0x20)
	PORT_DIPSETTING(    0x08, "99 seconds + 50 extended" ) PORT_CONDITION("IN2", 0x30, NOTEQUALS, 0x20)
	PORT_DIPSETTING(    0x0c, "60 seconds" ) PORT_CONDITION("IN2", 0x30, EQUALS, 0x20)
	PORT_DIPSETTING(    0x00, "80 seconds" ) PORT_CONDITION("IN2", 0x30, EQUALS, 0x20)
	PORT_DIPSETTING(    0x08, "99 seconds" ) PORT_CONDITION("IN2", 0x30, EQUALS, 0x20)
	PORT_DIPSETTING(    0x04, "Test Mode" )
	PORT_DIPNAME( 0x30, 0x00, "Extended Time At" ) PORT_CONDITION("IN2", 0x0c, NOTEQUALS, 0x04) PORT_DIPLOCATION("E3:5,6")
	PORT_DIPSETTING(    0x10, "2.00" )
	PORT_DIPSETTING(    0x00, "2.50" )
	PORT_DIPSETTING(    0x20, DEF_STR( None ) )
	/* PORT_DIPSETTING( 0x30, DEF_STR( None ) ) */
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Language )) PORT_CONDITION("IN2", 0x0c, NOTEQUALS, 0x04) PORT_DIPLOCATION("E3:7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x40, DEF_STR( German ) )
	PORT_DIPSETTING(    0x80, DEF_STR( French ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Spanish ) )
INPUT_PORTS_END


static INPUT_PORTS_START( lagunar )
	PORT_INCLUDE( zzzap )

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("E3:1,2")
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Game_Time ) ) PORT_CONDITION("IN2", 0xc0, NOTEQUALS, 0x04) PORT_DIPLOCATION("E3:3,4")
	PORT_DIPSETTING(    0x00, "45 seconds + 22 extended" ) PORT_CONDITION("IN2", 0xc0, NOTEQUALS, 0xc0)
	PORT_DIPSETTING(    0x04, "60 seconds + 30 extended" ) PORT_CONDITION("IN2", 0xc0, NOTEQUALS, 0xc0)
	PORT_DIPSETTING(    0x08, "75 seconds + 37 extended" ) PORT_CONDITION("IN2", 0xc0, NOTEQUALS, 0xc0)
	PORT_DIPSETTING(    0x0c, "90 seconds + 45 extended" ) PORT_CONDITION("IN2", 0xc0, NOTEQUALS, 0xc0)
	PORT_DIPSETTING(    0x00, "45 seconds" ) PORT_CONDITION("IN2", 0xc0, EQUALS, 0xc0)
	PORT_DIPSETTING(    0x04, "60 seconds" ) PORT_CONDITION("IN2", 0xc0, EQUALS, 0xc0)
	PORT_DIPSETTING(    0x08, "75 seconds" ) PORT_CONDITION("IN2", 0xc0, EQUALS, 0xc0)
	PORT_DIPSETTING(    0x0c, "90 seconds" ) PORT_CONDITION("IN2", 0xc0, EQUALS, 0xc0)
	PORT_DIPNAME( 0x30, 0x20, "Extended Time At" ) PORT_CONDITION("IN2", 0xc0, NOTEQUALS, 0xc0) PORT_DIPLOCATION("E3:5,6")
	PORT_DIPSETTING(    0x00, "350" )
	PORT_DIPSETTING(    0x10, "400" )
	PORT_DIPSETTING(    0x20, "450" )
	PORT_DIPSETTING(    0x30, "500" )
	PORT_DIPNAME( 0xc0, 0x00, "Test Modes/Extended Time") PORT_DIPLOCATION("E3:7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, "RAM/ROM Test" )
	PORT_DIPSETTING(    0x80, "Input Test" )
	PORT_DIPSETTING(    0xc0, "No Extended Time" )
INPUT_PORTS_END


void zzzap_state::zzzap_common(machine_config &config)
{
	mw8080bw_root(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_IO, &zzzap_state::io_map);

	WATCHDOG_TIMER(config, m_watchdog).set_time(PERIOD_OF_555_MONOSTABLE(RES_M(1), CAP_U(1))); /* 1.1s */

	/* add shifter */
	MB14241(config, m_mb14241);

	/* audio hardware handled by specific machine */
}


void zzzap_state::zzzap(machine_config &config)
{
	zzzap_common(config);

	/* audio hardware */
	ZZZAP_AUDIO(config, "soundboard");
}


void zzzap_state::lagunar(machine_config &config)
{
	zzzap_common(config);

	/* audio hardware */
	LAGUNAR_AUDIO(config, "soundboard");
}



/*************************************
 *
 *  Amazing Maze (PCB #611)
 *
 *************************************/

/* schematic says 12.5 Hz, but R/C values shown give 8.5Hz */
#define MAZE_555_B1_PERIOD      PERIOD_OF_555_ASTABLE(RES_K(33) /* R200 */, RES_K(68) /* R201 */, CAP_U(1) /* C201 */)

void mw8080bw_state::maze_update_discrete()
{
	maze_write_discrete(m_maze_tone_timing_state);
}


TIMER_CALLBACK_MEMBER(mw8080bw_state::maze_tone_timing_timer_callback)
{
	m_maze_tone_timing_state = !m_maze_tone_timing_state;
	maze_write_discrete(m_maze_tone_timing_state);
}


MACHINE_START_MEMBER(mw8080bw_state,maze)
{
	mw8080bw_state::machine_start();

	/* create astable timer for IC B1 */
	m_maze_tone_timer = timer_alloc(FUNC(mw8080bw_state::maze_tone_timing_timer_callback), this);
	m_maze_tone_timer->adjust(MAZE_555_B1_PERIOD, 0, MAZE_555_B1_PERIOD);

	/* initialize state of Tone Timing FF, IC C1 */
	m_maze_tone_timing_state = 0;

	/* setup for save states */
	save_item(NAME(m_maze_tone_timing_state));
	machine().save().register_postload(save_prepost_delegate(FUNC(mw8080bw_state::maze_update_discrete), this));
}


void mw8080bw_state::maze_coin_counter_w(uint8_t data)
{
	/* the data is not used, just pulse the counter */
	machine().bookkeeping().coin_counter_w(0, 0);
	machine().bookkeeping().coin_counter_w(0, 1);
}


void mw8080bw_state::maze_io_w(offs_t offset, uint8_t data)
{
	if (offset & 0x01)  maze_coin_counter_w(data);

	if (offset & 0x02)  m_watchdog->watchdog_reset();
}


void mw8080bw_state::maze_io_map(address_map &map)
{
	map.global_mask(0x3);
	map(0x00, 0x00).portr("IN0");
	map(0x01, 0x01).portr("IN1");

	/* no decoder, just a couple of AND gates */
	map(0x00, 0x03).w(FUNC(mw8080bw_state::maze_io_w));
}


static INPUT_PORTS_START( maze )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )    /* labeled 'Not Used' */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1  )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Coinage ) ) PORT_CONDITION("IN1", 0x80, EQUALS, 0x00) PORT_DIPLOCATION("SW:1,2")
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x40, 0x40, "2 Player Game Time" ) PORT_CONDITION("IN1", 0x80, EQUALS, 0x00) PORT_DIPLOCATION("SW:3")
	PORT_DIPSETTING(    0x40, "4 minutes" )
	PORT_DIPSETTING(    0x00, "6 minutes" )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_HIGH, "SW:4" )
INPUT_PORTS_END


void mw8080bw_state::maze(machine_config &config)
{
	mw8080bw_root(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_IO, &mw8080bw_state::maze_io_map);

	MCFG_MACHINE_START_OVERRIDE(mw8080bw_state,maze)

	WATCHDOG_TIMER(config, m_watchdog).set_time(PERIOD_OF_555_MONOSTABLE(RES_K(270), CAP_U(10))); /* 2.97s */

	/* audio hardware */
	maze_audio(config);
}



/*************************************
 *
 *  Boot Hill (PCB #612)
 *
 *************************************/

void boothill_state::machine_start()
{
	mw8080bw_state::machine_start();

	m_rev_shift_res = 0U;

	save_item(NAME(m_rev_shift_res));
}

u8 boothill_state::reversible_shift_result_r()
{
	return m_rev_shift_res ? mw8080bw_shift_result_rev_r() : m_mb14241->shift_result_r();
}

void boothill_state::reversible_shift_count_w(u8 data)
{
	m_mb14241->shift_count_w(data);

	m_rev_shift_res = BIT(data, 3);
}


void boothill_state::boothill_io_map(address_map &map)
{
	map.global_mask(0x7);
	map(0x00, 0x00).mirror(0x04).portr("IN0");
	map(0x01, 0x01).mirror(0x04).portr("IN1");
	map(0x02, 0x02).mirror(0x04).portr("IN2");
	map(0x03, 0x03).mirror(0x04).r(FUNC(boothill_state::reversible_shift_result_r));

	map(0x01, 0x01).w(FUNC(boothill_state::reversible_shift_count_w));
	map(0x02, 0x02).w(m_mb14241, FUNC(mb14241_device::shift_data_w));
	map(0x03, 0x03).w("soundboard", FUNC(boothill_audio_device::write));
	map(0x04, 0x04).w(m_watchdog, FUNC(watchdog_timer_device::reset_w));
	map(0x05, 0x05).w("soundboard", FUNC(boothill_audio_device::tone_generator_lo_w));
	map(0x06, 0x06).w("soundboard", FUNC(boothill_audio_device::tone_generator_hi_w));
}


static const ioport_value boothill_controller_table[7] =
{
	0x00, 0x04, 0x06, 0x07, 0x03, 0x01, 0x05
};


static INPUT_PORTS_START( boothill )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x70, 0x30, IPT_POSITIONAL_V ) PORT_POSITIONS(7) PORT_REMAP_TABLE(boothill_controller_table) PORT_SENSITIVITY(5) PORT_KEYDELTA(10) PORT_CODE_DEC(KEYCODE_M) PORT_CODE_INC(KEYCODE_J) PORT_CENTERDELTA(0) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x70, 0x30, IPT_POSITIONAL_V ) PORT_POSITIONS(7) PORT_REMAP_TABLE(boothill_controller_table) PORT_SENSITIVITY(5) PORT_KEYDELTA(10) PORT_CODE_DEC(KEYCODE_N) PORT_CODE_INC(KEYCODE_H) PORT_CENTERDELTA(0) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)

	PORT_START("IN2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) ) PORT_CONDITION("IN2", 0x10, EQUALS, 0x00) PORT_DIPLOCATION("SW:1,2")
	PORT_DIPSETTING(    0x02, "2 Coins per Player" )
	PORT_DIPSETTING(    0x03, "2 Coins/1 or 2 Players" )
	PORT_DIPSETTING(    0x00, "1 Coin per Player" )
	PORT_DIPSETTING(    0x01, "1 Coin/1 or 2 Players" )
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Game_Time ) ) PORT_CONDITION("IN2", 0x10, EQUALS, 0x00) PORT_DIPLOCATION("SW:3,4")
	PORT_DIPSETTING(    0x00, "60 seconds" )
	PORT_DIPSETTING(    0x04, "70 seconds" )
	PORT_DIPSETTING(    0x08, "80 seconds" )
	PORT_DIPSETTING(    0x0c, "90 seconds" )
	PORT_SERVICE_DIPLOC (0x10, IP_ACTIVE_HIGH, "SW:5" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )
INPUT_PORTS_END


void boothill_state::boothill(machine_config &config)
{
	mw8080bw_root(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_IO, &boothill_state::boothill_io_map);

	WATCHDOG_TIMER(config, m_watchdog).set_time(PERIOD_OF_555_MONOSTABLE(RES_K(270), CAP_U(10))); // 2.97s

	// add shifter
	MB14241(config, m_mb14241);

	// audio hardware
	BOOTHILL_AUDIO(config, "soundboard");
}



/*************************************
 *
 *  Checkmate (PCB #615)
 *
 *************************************/

void mw8080bw_state::checkmat_io_w(offs_t offset, uint8_t data)
{
	if (offset & 0x01)  checkmat_audio_w(data);

	if (offset & 0x02)  m_watchdog->watchdog_reset();
}


void mw8080bw_state::checkmat_io_map(address_map &map)
{
	map.global_mask(0x3);
	map(0x00, 0x00).portr("IN0");
	map(0x01, 0x01).portr("IN1");
	map(0x02, 0x02).portr("IN2");
	map(0x03, 0x03).portr("IN3");

	/* no decoder, just a couple of AND gates */
	map(0x00, 0x03).w(FUNC(mw8080bw_state::checkmat_io_w));
}


static INPUT_PORTS_START( checkmat )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4)

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Coinage ) ) PORT_CONDITION("IN2", 0x80, EQUALS, 0x00) PORT_DIPLOCATION("A4:1")
	PORT_DIPSETTING(    0x00, "1 Coin/1 or 2 Players" )
	PORT_DIPSETTING(    0x01, "1 Coin/1 or 2 Players, 2 Coins/3 or 4 Players" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unused ) ) PORT_CONDITION("IN2", 0x80, EQUALS, 0x00) PORT_DIPLOCATION("A4:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x04, "Rounds" ) PORT_CONDITION("IN2", 0x80, EQUALS, 0x00) PORT_DIPLOCATION("A4:3,4")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unused ) ) PORT_CONDITION("IN2", 0x80, EQUALS, 0x00) PORT_DIPLOCATION("A4:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x00, DEF_STR( Language ) ) PORT_CONDITION("IN2", 0x80, EQUALS, 0x00) PORT_DIPLOCATION("A4:6,7")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x20, "Language 2" )
	PORT_DIPSETTING(    0x40, "Language 3" )
	PORT_DIPSETTING(    0x60, "Language 4" )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_HIGH, "A4:8" )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START4 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("R309")
	PORT_ADJUSTER( 50, "Boom Volume" )

	PORT_START("R411")
	PORT_ADJUSTER( 50, "Tone Volume" )
INPUT_PORTS_END


void mw8080bw_state::checkmat(machine_config &config)
{
	mw8080bw_root(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_IO, &mw8080bw_state::checkmat_io_map);

	WATCHDOG_TIMER(config, m_watchdog).set_time(PERIOD_OF_555_MONOSTABLE(RES_K(270), CAP_U(10))); /* 2.97s */

	/* audio hardware */
	checkmat_audio(config);
}



/*************************************
 *
 *  Desert Gun / Road Runner (PCB #618)
 *
 *************************************/

void desertgu_state::machine_start()
{
	mw8080bw_state::machine_start();

	m_controller_select = 0U;

	save_item(NAME(m_controller_select));
}


ioport_value desertgu_state::gun_input_r()
{
	return m_gun_port[m_controller_select]->read();
}


ioport_value desertgu_state::dip_sw_0_1_r()
{
	return m_dip_sw_0_1[m_controller_select]->read();
}


void desertgu_state::io_map(address_map &map)
{
	map.global_mask(0x7);
	map(0x00, 0x00).mirror(0x04).r(FUNC(desertgu_state::mw8080bw_shift_result_rev_r));
	map(0x01, 0x01).mirror(0x04).portr("IN0");
	map(0x02, 0x02).mirror(0x04).portr("IN1");
	map(0x03, 0x03).mirror(0x04).r(m_mb14241, FUNC(mb14241_device::shift_result_r));

	map(0x01, 0x01).w(m_mb14241, FUNC(mb14241_device::shift_count_w));
	map(0x02, 0x02).w(m_mb14241, FUNC(mb14241_device::shift_data_w));
	map(0x03, 0x03).w("soundboard", FUNC(desertgu_audio_device::p1_w));
	map(0x04, 0x04).w(m_watchdog, FUNC(watchdog_timer_device::reset_w));
	map(0x05, 0x05).w("soundboard", FUNC(desertgu_audio_device::tone_generator_lo_w));
	map(0x06, 0x06).w("soundboard", FUNC(desertgu_audio_device::tone_generator_hi_w));
	map(0x07, 0x07).w("soundboard", FUNC(desertgu_audio_device::p2_w));
}


static INPUT_PORTS_START( desertgu )
	PORT_START("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(desertgu_state::gun_input_r))

	PORT_START("IN1")
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(desertgu_state::dip_sw_0_1_r))
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Language ) ) PORT_CONDITION("IN1", 0x30, NOTEQUALS, 0x30) PORT_DIPLOCATION("C2:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x04, DEF_STR( German ) )
	PORT_DIPSETTING(    0x08, DEF_STR( French ) )
	PORT_DIPSETTING(    0x0c, "Danish" )
	PORT_DIPNAME( 0x30, 0x10, "Extended Time At" ) PORT_DIPLOCATION("C2:7,8")
	PORT_DIPSETTING(    0x00, "5000" )
	PORT_DIPSETTING(    0x10, "7000" )
	PORT_DIPSETTING(    0x20, "9000" )
	PORT_DIPSETTING(    0x30, "Test Mode" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	/* fake ports for reading the gun's X and Y axis */
	PORT_START(DESERTGU_GUN_X_PORT_TAG)
	PORT_BIT( 0xff, 0x4d, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX(0x10,0x8e) PORT_SENSITIVITY(70) PORT_KEYDELTA(10)

	PORT_START(DESERTGU_GUN_Y_PORT_TAG)
	PORT_BIT( 0xff, 0x48, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX(0x10,0x7f) PORT_SENSITIVITY(70) PORT_KEYDELTA(10)

	/* D0 and D1 in the DIP SW input port can reflect two sets of switches depending on the controller
	   select bit.  These two ports are fakes to handle this case */
	PORT_START(DESERTGU_DIP_SW_0_1_SET_1_TAG)
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) ) PORT_CONDITION("IN1", 0x30, NOTEQUALS, 0x30) PORT_DIPLOCATION("C2:1,2")
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START(DESERTGU_DIP_SW_0_1_SET_2_TAG)
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Game_Time ) ) PORT_CONDITION("IN1", 0x30, NOTEQUALS, 0x30) PORT_DIPLOCATION("C2:3,4")
	PORT_DIPSETTING(    0x00, "40 seconds + 30 extended" )
	PORT_DIPSETTING(    0x01, "50 seconds + 30 extended" )
	PORT_DIPSETTING(    0x02, "60 seconds + 30 extended" )
	PORT_DIPSETTING(    0x03, "70 seconds + 30 extended" )
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


void desertgu_state::desertgu(machine_config &config)
{
	mw8080bw_root(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_IO, &desertgu_state::io_map);

	WATCHDOG_TIMER(config, m_watchdog).set_time(255 * attotime::from_hz(MW8080BW_60HZ));

	// add shifter
	MB14241(config, m_mb14241);

	// audio hardware
	DESERTGU_AUDIO(config, "soundboard").ctrl_sel_out().set([this] (int state) { m_controller_select = state ? 1U : 0U; });
}



/*************************************
 *
 *  Double Play (PCB #619) / Extra Inning (PCB #642)
 *
 *  This game comes in an upright and a cocktail cabinet.
 *  The upright one had a shared joystick and a hitting button for
 *  each player, while in the cocktail version each player
 *  had their own set of controls.  The display is never flipped,
 *  as the two players sit diagonally across from each other.
 *
 *************************************/

#define DPLAY_CAB_TYPE_UPRIGHT      (0)
#define DPLAY_CAB_TYPE_COCKTAIL     (1)


ioport_value dplay_state::dplay_pitch_left_input_r()
{
	uint32_t ret;

	if (m_cab_type->read() == DPLAY_CAB_TYPE_UPRIGHT)
		return m_l_pitch->read();
	else
		return m_r_pitch->read();

	return ret;
}


ioport_value dplay_state::dplay_pitch_right_input_r()
{
	return m_l_pitch->read();
}


void dplay_state::io_map(address_map &map)
{
	map.global_mask(0x7);
	map(0x00, 0x00).mirror(0x04).portr("IN0");
	map(0x01, 0x01).mirror(0x04).portr("IN1");
	map(0x02, 0x02).mirror(0x04).portr("IN2");
	map(0x03, 0x03).mirror(0x04).r(m_mb14241, FUNC(mb14241_device::shift_result_r));

	map(0x01, 0x01).w(m_mb14241, FUNC(mb14241_device::shift_count_w));
	map(0x02, 0x02).w(m_mb14241, FUNC(mb14241_device::shift_data_w));
	map(0x03, 0x03).w("soundboard", FUNC(dplay_audio_device::write));
	map(0x04, 0x04).w(m_watchdog, FUNC(watchdog_timer_device::reset_w));
	map(0x05, 0x05).w("soundboard", FUNC(dplay_audio_device::tone_generator_lo_w));
	map(0x06, 0x06).w("soundboard", FUNC(dplay_audio_device::tone_generator_hi_w));
}


static INPUT_PORTS_START( dplay )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Hit") PORT_PLAYER(1)
	PORT_BIT( 0x7e, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(dplay_state::dplay_pitch_left_input_r))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P2 Hit") PORT_PLAYER(2)
	PORT_BIT( 0x7e, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(dplay_state::dplay_pitch_right_input_r))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN2")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage )) PORT_CONDITION("IN2", 0x40, EQUALS, 0x40) PORT_DIPLOCATION("C1:1,2,3")
	PORT_DIPSETTING(    0x05, "2 Coins/1 Inning/1 Player, 4 Coins/1 Inning/2 Players, 8 Coins/3 Innings/2 Players" )
	PORT_DIPSETTING(    0x04, "1 Coin/1 Inning/1 Player, 2 Coins/1 Inning/2 Players, 4 Coins/3 Innings/2 Players" )
	PORT_DIPSETTING(    0x02, "2 Coins per Inning" )
	PORT_DIPSETTING(    0x03, "2 Coins/1 Inning, 4 Coins/3 Innings" )
	PORT_DIPSETTING(    0x00, "1 Coin per Inning" )
	// PORT_DIPSETTING( 0x06, "1 Coin per Inning" )
	// PORT_DIPSETTING( 0x07, "1 Coin per Inning" )
	PORT_DIPSETTING(    0x01, "1 Coin/1 Inning, 2 Coins/3 Innings" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) ) PORT_CONDITION("IN2", 0x40, EQUALS, 0x40) PORT_DIPLOCATION("C1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unused ) ) PORT_CONDITION("IN2", 0x40, EQUALS, 0x40) PORT_DIPLOCATION("C1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unused ) ) PORT_CONDITION("IN2", 0x40, EQUALS, 0x40) PORT_DIPLOCATION("C1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x40, IP_ACTIVE_LOW, "C1:7" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	// fake ports to handle the various input configurations based on cabinet type
	PORT_START(DPLAY_L_PITCH_PORT_TAG)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Move Outfield Left") PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 Move Outfield Right") PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_NAME("P1 Pitch Left") PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_NAME("P1 Pitch Right") PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_NAME("P1 Pitch Slow") PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_NAME("P1 Pitch Fast") PORT_PLAYER(1)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START(DPLAY_R_PITCH_PORT_TAG)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P2 Move Outfield Left") PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P2 Move Outfield Right") PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_NAME("P2 Pitch Left") PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_NAME("P2 Pitch Right") PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_NAME("P2 Pitch Slow") PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_NAME("P2 Pitch Fast") PORT_PLAYER(2)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	// fake port for cabinet type
	PORT_START(DPLAY_CAB_TYPE_PORT_TAG)
	PORT_CONFNAME( 0x01, DPLAY_CAB_TYPE_UPRIGHT, DEF_STR( Cabinet ) )
	PORT_CONFSETTING( DPLAY_CAB_TYPE_UPRIGHT, DEF_STR( Upright ) )
	PORT_CONFSETTING( DPLAY_CAB_TYPE_COCKTAIL, DEF_STR( Cocktail ) )
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( einning )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Hit") PORT_PLAYER(1)
	PORT_BIT( 0x7e, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(dplay_state::dplay_pitch_left_input_r))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P2 Hit") PORT_PLAYER(2)
	PORT_BIT( 0x7e, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(dplay_state::dplay_pitch_right_input_r))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN2")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage )) PORT_CONDITION("IN2", 0x40, EQUALS, 0x40) PORT_DIPLOCATION("C1:1,2,3")
	PORT_DIPSETTING(    0x05, "2 Coins/1 Inning/1 Player, 4 Coins/1 Inning/2 Players, 8 Coins/3 Innings/2 Players" )
	PORT_DIPSETTING(    0x04, "1 Coin/1 Inning/1 Player, 2 Coins/1 Inning/2 Players, 4 Coins/3 Innings/2 Players" )
	PORT_DIPSETTING(    0x02, "2 Coins per Inning" )
	PORT_DIPSETTING(    0x03, "2 Coins/1 Inning, 4 Coins/3 Innings" )
	PORT_DIPSETTING(    0x00, "1 Coin per Inning" )
	// PORT_DIPSETTING( 0x06, "1 Coin per Inning" )
	// PORT_DIPSETTING( 0x07, "1 Coin per Inning" )
	PORT_DIPSETTING(    0x01, "1 Coin/1 Inning, 2 Coins/3 Innings" )
	PORT_DIPNAME( 0x08, 0x00, "Wall Knock Out Behavior" ) PORT_CONDITION("IN2", 0x40, EQUALS, 0x40) PORT_DIPLOCATION("C1:4")
	PORT_DIPSETTING(    0x00, "Individually" )
	PORT_DIPSETTING(    0x08, "In Pairs" )
	PORT_DIPNAME( 0x10, 0x00, "Double Score when Special Lit" ) PORT_CONDITION("IN2", 0x40, EQUALS, 0x40) PORT_DIPLOCATION("C1:5")
	PORT_DIPSETTING(    0x00, "Home Run Only" )
	PORT_DIPSETTING(    0x10, "Any Hit" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unused ) ) PORT_CONDITION("IN2", 0x40, EQUALS, 0x40) PORT_DIPLOCATION("C1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x40, IP_ACTIVE_LOW, "C1:7" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	// fake ports to handle the various input configurations based on cabinet type
	PORT_START(DPLAY_L_PITCH_PORT_TAG)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Move Outfield Left") PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 Move Outfield Right") PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_NAME("P1 Pitch Left") PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_NAME("P1 Pitch Right") PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_NAME("P1 Pitch Slow") PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_NAME("P1 Pitch Fast") PORT_PLAYER(1)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START(DPLAY_R_PITCH_PORT_TAG)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P2 Move Outfield Left") PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P2 Move Outfield Right") PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_NAME("P2 Pitch Left") PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_NAME("P2 Pitch Right") PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_NAME("P2 Pitch Slow") PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_NAME("P2 Pitch Fast") PORT_PLAYER(2)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	// fake port for cabinet type
	PORT_START(DPLAY_CAB_TYPE_PORT_TAG)
	PORT_CONFNAME( 0x01, DPLAY_CAB_TYPE_UPRIGHT, DEF_STR( Cabinet ) )
	PORT_CONFSETTING( DPLAY_CAB_TYPE_UPRIGHT, DEF_STR( Upright ) )
	PORT_CONFSETTING( DPLAY_CAB_TYPE_COCKTAIL, DEF_STR( Cocktail ) )
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


void dplay_state::dplay(machine_config &config)
{
	mw8080bw_root(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_IO, &dplay_state::io_map);

	WATCHDOG_TIMER(config, m_watchdog).set_time(255 * attotime::from_hz(MW8080BW_60HZ));

	// add shifter
	MB14241(config, m_mb14241);

	// audio hardware
	DPLAY_AUDIO(config, "soundboard");
}



/*************************************
 *
 *  Guided Missile (PCB #623)
 *
 *************************************/

void boothill_state::gmissile_io_map(address_map &map)
{
	map.global_mask(0x7);
	map(0x00, 0x00).mirror(0x04).portr("IN0");
	map(0x01, 0x01).mirror(0x04).portr("IN1");
	map(0x02, 0x02).mirror(0x04).portr("IN2");
	map(0x03, 0x03).mirror(0x04).r(FUNC(boothill_state::reversible_shift_result_r));

	map(0x01, 0x01).w(FUNC(boothill_state::reversible_shift_count_w));
	map(0x02, 0x02).w(m_mb14241, FUNC(mb14241_device::shift_data_w));
	map(0x03, 0x03).w("soundboard", FUNC(gmissile_audio_device::p1_w));
	map(0x04, 0x04).w(m_watchdog, FUNC(watchdog_timer_device::reset_w));
	map(0x05, 0x05).w("soundboard", FUNC(gmissile_audio_device::p2_w));
	// also writes 0x00 to 0x06, but it is not connected
	map(0x07, 0x07).w("soundboard", FUNC(gmissile_audio_device::p3_w));
}


static INPUT_PORTS_START( gmissile )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )  // not connected
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )  // not connected
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )  // not connected
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )  // not connected
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )  // not connected
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )  // not connected
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )  // not connected
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)

	PORT_START("IN2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) ) PORT_CONDITION("IN2", 0x80, EQUALS, 0x80) PORT_DIPLOCATION("D1:1,2")
	PORT_DIPSETTING(    0x01, "2 Coins per Player" )
	PORT_DIPSETTING(    0x00, "2 Coins/1 or 2 Players" )
	PORT_DIPSETTING(    0x03, "1 Coin per Player" )
	PORT_DIPSETTING(    0x02, "1 Coin/1 or 2 Players" )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Game_Time ) ) PORT_CONDITION("IN2", 0x80, EQUALS, 0x80) PORT_DIPLOCATION("D1:3,4")
	PORT_DIPSETTING(    0x00, "60 seconds + 30 extended" )
	PORT_DIPSETTING(    0x08, "70 seconds + 35 extended" )
	PORT_DIPSETTING(    0x04, "80 seconds + 40 extended" )
	PORT_DIPSETTING(    0x0c, "90 seconds + 45 extended" )
	PORT_DIPNAME( 0x30, 0x10, "Extended Time At" ) PORT_CONDITION("IN2", 0x80, EQUALS, 0x80) PORT_DIPLOCATION("D1:5,6")
	PORT_DIPSETTING(    0x00, "500" )
	PORT_DIPSETTING(    0x20, "700" )
	PORT_DIPSETTING(    0x10, "1000" )
	PORT_DIPSETTING(    0x30, "1300" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unused ) ) PORT_CONDITION("IN2", 0x80, EQUALS, 0x80) PORT_DIPLOCATION("D1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_LOW, "D1:8" )
INPUT_PORTS_END


void boothill_state::gmissile(machine_config &config)
{
	mw8080bw_root(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_IO, &boothill_state::gmissile_io_map);

	WATCHDOG_TIMER(config, m_watchdog).set_time(255 * attotime::from_hz(MW8080BW_60HZ));

	// add shifter
	MB14241(config, m_mb14241);

	// audio hardware
	GMISSILE_AUDIO(config, "soundboard");
}



/*************************************
 *
 *  M-4 (PCB #626)
 *
 *************************************/

void boothill_state::m4_io_map(address_map &map)
{
	map.global_mask(0x7);
	map(0x00, 0x00).mirror(0x04).portr("IN0");
	map(0x01, 0x01).mirror(0x04).portr("IN1");
	map(0x02, 0x02).mirror(0x04).portr("IN2");
	map(0x03, 0x03).mirror(0x04).r(FUNC(boothill_state::reversible_shift_result_r));

	map(0x01, 0x01).w(FUNC(boothill_state::reversible_shift_count_w));
	map(0x02, 0x02).w(m_mb14241, FUNC(mb14241_device::shift_data_w));
	map(0x03, 0x03).w("soundboard", FUNC(m4_audio_device::p1_w));
	map(0x04, 0x04).w(m_watchdog, FUNC(watchdog_timer_device::reset_w));
	map(0x05, 0x05).w("soundboard", FUNC(m4_audio_device::p2_w));
}


static INPUT_PORTS_START( m4 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )  // not connected
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )  // not connected
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P2 Trigger") PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P2 Reload") PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )  // not connected
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )  // not connected

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_2WAY PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_2WAY PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Trigger") PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Reload") PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )  // not connected

	PORT_START("IN2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) ) PORT_CONDITION("IN2", 0x10, EQUALS, 0x10) PORT_DIPLOCATION("C1:1,2")
	PORT_DIPSETTING(    0x02, "2 Coins per Player" )
	PORT_DIPSETTING(    0x03, "2 Coins/1 or 2 Players" )
	PORT_DIPSETTING(    0x00, "1 Coin per Player" )
	PORT_DIPSETTING(    0x01, "1 Coin/1 or 2 Players" )
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Game_Time ) ) PORT_CONDITION("IN2", 0x10, EQUALS, 0x10) PORT_DIPLOCATION("C1:3,4")
	PORT_DIPSETTING(    0x00, "60 seconds" )
	PORT_DIPSETTING(    0x04, "70 seconds" )
	PORT_DIPSETTING(    0x08, "80 seconds" )
	PORT_DIPSETTING(    0x0c, "90 seconds" )
	PORT_SERVICE_DIPLOC( 0x10, IP_ACTIVE_LOW, "C1:5" )
	PORT_DIPNAME( 0x20, 0x00, "Extended Play" ) PORT_DIPLOCATION("C1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, "Extended Play At" ) PORT_DIPLOCATION("C1:8,7")
	PORT_DIPSETTING(    0xc0, "70" )
	PORT_DIPSETTING(    0x40, "80" )
	PORT_DIPSETTING(    0x80, "100" )
	PORT_DIPSETTING(    0x00, "110" )
INPUT_PORTS_END


void boothill_state::m4(machine_config &config)
{
	mw8080bw_root(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_IO, &boothill_state::m4_io_map);

	WATCHDOG_TIMER(config, m_watchdog).set_time(255 * attotime::from_hz(MW8080BW_60HZ));

	// add shifter
	MB14241(config, m_mb14241);

	// audio hardware
	M4_AUDIO(config, "soundboard");
}



/*************************************
 *
 *  Clowns (PCB #630)
 *
 *************************************/

#define CLOWNS_CONTROLLER_P1_TAG        ("CONTP1")
#define CLOWNS_CONTROLLER_P2_TAG        ("CONTP2")


void clowns_state::machine_start()
{
	mw8080bw_state::machine_start();

	m_controller_select = 0U;

	save_item(NAME(m_controller_select));
}


ioport_value clowns_state::controller_r()
{
	return m_controllers[m_controller_select]->read();
}


void clowns_state::clowns_io_map(address_map &map)
{
	map.global_mask(0x7);
	map(0x00, 0x00).mirror(0x04).portr("IN0");
	map(0x01, 0x01).mirror(0x04).portr("IN1");
	map(0x02, 0x02).mirror(0x04).portr("IN2");
	map(0x03, 0x03).mirror(0x04).r(m_mb14241, FUNC(mb14241_device::shift_result_r));

	map(0x01, 0x01).w(m_mb14241, FUNC(mb14241_device::shift_count_w));
	map(0x02, 0x02).w(m_mb14241, FUNC(mb14241_device::shift_data_w));
	map(0x03, 0x03).w("soundboard", FUNC(clowns_audio_device::p1_w));
	map(0x04, 0x04).w(m_watchdog, FUNC(watchdog_timer_device::reset_w));
	map(0x05, 0x05).w("soundboard", FUNC(clowns_audio_device::tone_generator_lo_w));
	map(0x06, 0x06).w("soundboard", FUNC(clowns_audio_device::tone_generator_hi_w));
	map(0x07, 0x07).w("soundboard", FUNC(clowns_audio_device::p2_w));
}


static INPUT_PORTS_START( clowns )
	PORT_START("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(clowns_state::controller_r))

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )  // not connected
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )  // not connected
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )  // not connected
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )  // not connected
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )  // not connected

	PORT_START("IN2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) ) PORT_CONDITION("IN2", 0x80, EQUALS, 0x00) PORT_DIPLOCATION("SW:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x00, "Bonus Game" ) PORT_CONDITION("IN2", 0x80, EQUALS, 0x00) PORT_DIPLOCATION("SW:3,4")
	PORT_DIPSETTING(    0x00, "No Bonus" )
	PORT_DIPSETTING(    0x04, "9000" )
	PORT_DIPSETTING(    0x08, "11000" )
	PORT_DIPSETTING(    0x0c, "13000" )
	PORT_DIPNAME( 0x10, 0x00, "Balloon Resets" ) PORT_CONDITION("IN2", 0x80, EQUALS, 0x00) PORT_DIPLOCATION("SW:5")
	PORT_DIPSETTING(    0x00, "Each Row" )
	PORT_DIPSETTING(    0x10, "All Rows" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Bonus_Life ) ) PORT_CONDITION("IN2", 0x80, EQUALS, 0x00) PORT_DIPLOCATION("SW:6")
	PORT_DIPSETTING(    0x00, "3000" )
	PORT_DIPSETTING(    0x20, "4000" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Lives ) ) PORT_CONDITION("IN2", 0x80, EQUALS, 0x00) PORT_DIPLOCATION("SW:7")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	// test mode - press coin button for input test
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_HIGH, "SW:8" )

	// fake ports for two analog controls multiplexed
	PORT_START(CLOWNS_CONTROLLER_P1_TAG)
	PORT_BIT( 0xff, 0x7f, IPT_PADDLE ) PORT_MINMAX(0x01,0xfe) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_PLAYER(1)

	PORT_START(CLOWNS_CONTROLLER_P2_TAG)
	PORT_BIT( 0xff, 0x7f, IPT_PADDLE ) PORT_MINMAX(0x01,0xfe) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( clowns1 )
	PORT_START("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(clowns_state::controller_r))

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) ) PORT_CONDITION("IN2", 0x80, EQUALS, 0x00) PORT_DIPLOCATION("SW:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Lives ) ) PORT_CONDITION("IN2", 0x80, EQUALS, 0x00) PORT_DIPLOCATION("SW:3,4")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPNAME( 0x10, 0x00, "Balloon Resets" ) PORT_CONDITION("IN2", 0x80, EQUALS, 0x00) PORT_DIPLOCATION("SW:5")
	PORT_DIPSETTING(    0x00, "Each Row" )
	PORT_DIPSETTING(    0x10, "All Rows" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Bonus_Life ) ) PORT_CONDITION("IN2", 0x80, EQUALS, 0x00) PORT_DIPLOCATION("SW:6")
	PORT_DIPSETTING(    0x00, "3000" )
	PORT_DIPSETTING(    0x20, "4000" )
	PORT_DIPNAME( 0x40, 0x00, "Input Test"  ) PORT_DIPLOCATION("SW:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_HIGH, "SW:8" )

	PORT_START(CLOWNS_CONTROLLER_P1_TAG)
	PORT_BIT( 0xff, 0x7f, IPT_PADDLE ) PORT_MINMAX(0x01,0xfe) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_PLAYER(1)

	PORT_START(CLOWNS_CONTROLLER_P2_TAG)
	PORT_BIT( 0xff, 0x7f, IPT_PADDLE ) PORT_MINMAX(0x01,0xfe) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_PLAYER(2)
INPUT_PORTS_END


void clowns_state::clowns(machine_config &config)
{
	mw8080bw_root(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_IO, &clowns_state::clowns_io_map);

	WATCHDOG_TIMER(config, m_watchdog).set_time(255 * attotime::from_hz(MW8080BW_60HZ));

	// add shifter
	MB14241(config, m_mb14241);

	// audio hardware
	CLOWNS_AUDIO(config, "soundboard").ctrl_sel_out().set([this] (int state) { m_controller_select = state ? 1U : 0U; });
}



/*************************************
 *
 *  Space Walk (PCB #640)
 *
 *************************************/

void clowns_state::spacwalk_io_map(address_map &map)
{
	map.global_mask(0x7);

	map(0x00, 0x00).portr("IN0");
	map(0x01, 0x01).portr("IN1");
	map(0x02, 0x02).portr("IN2");
	map(0x03, 0x03).r(m_mb14241, FUNC(mb14241_device::shift_result_r));

	map(0x01, 0x01).w(m_mb14241, FUNC(mb14241_device::shift_count_w));
	map(0x02, 0x02).w(m_mb14241, FUNC(mb14241_device::shift_data_w));
	map(0x03, 0x03).w("soundboard", FUNC(spacwalk_audio_device::p1_w));
	map(0x04, 0x04).w(m_watchdog, FUNC(watchdog_timer_device::reset_w));
	map(0x05, 0x05).w("soundboard", FUNC(spacwalk_audio_device::tone_generator_lo_w));
	map(0x06, 0x06).w("soundboard", FUNC(spacwalk_audio_device::tone_generator_hi_w));
	map(0x07, 0x07).w("soundboard", FUNC(spacwalk_audio_device::p2_w));
}

static INPUT_PORTS_START( spacwalk )
	PORT_START("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(clowns_state::controller_r))

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	// 8 pin DIP Switch on location C2 on PCB A084-90700-D640
	/* PCB picture also shows a 2nd DIP Switch on location B2, supposedly for language selection,
	but ROM contents suggests it's not connected (no different languages or unmapped reads) */
	PORT_START("IN2")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Game_Time ) ) PORT_DIPLOCATION("C2:1,2")
	PORT_DIPSETTING(    0x03, "40 seconds + 20 extended" ) PORT_CONDITION("IN2", 0x30, NOTEQUALS, 0x00) // 45 + 20 for 2 players
	PORT_DIPSETTING(    0x02, "50 seconds + 25 extended" ) PORT_CONDITION("IN2", 0x30, NOTEQUALS, 0x00) // 60 + 30 for 2 players
	PORT_DIPSETTING(    0x01, "60 seconds + 30 extended" ) PORT_CONDITION("IN2", 0x30, NOTEQUALS, 0x00) // 75 + 35 for 2 players
	PORT_DIPSETTING(    0x00, "70 seconds + 35 extended" ) PORT_CONDITION("IN2", 0x30, NOTEQUALS, 0x00) // 90 + 45 for 2 players
	PORT_DIPSETTING(    0x03, "40 seconds" ) PORT_CONDITION("IN2", 0x30, EQUALS, 0x00)
	PORT_DIPSETTING(    0x02, "50 seconds" ) PORT_CONDITION("IN2", 0x30, EQUALS, 0x00)
	PORT_DIPSETTING(    0x01, "60 seconds" ) PORT_CONDITION("IN2", 0x30, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, "70 seconds" ) PORT_CONDITION("IN2", 0x30, EQUALS, 0x00)
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("C2:3,4")
	PORT_DIPSETTING(    0x00, "1 Coin per Player" )
	PORT_DIPSETTING(    0x04, "1 Coin/1 or 2 Players" )
	PORT_DIPSETTING(    0x0c, "2 Coins per Player" )
	PORT_DIPSETTING(    0x08, "2 Coins/1 or 2 Players" )
	PORT_DIPNAME( 0x30, 0x00, "Extended Time At" ) PORT_DIPLOCATION("C2:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPSETTING(    0x10, "5000" )
	PORT_DIPSETTING(    0x20, "6000" )
	PORT_DIPSETTING(    0x30, "7000" )
	PORT_DIPNAME( 0x40, 0x00, "Springboard Alignment" ) PORT_DIPLOCATION("C2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(0x80, IP_ACTIVE_HIGH, "C2:8" ) // RAM-ROM Test

	// fake ports for two analog controls multiplexed
	PORT_START(CLOWNS_CONTROLLER_P1_TAG)
	PORT_BIT( 0xff, 0x7f, IPT_PADDLE ) PORT_MINMAX(0x01,0xfe) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_PLAYER(1)

	PORT_START(CLOWNS_CONTROLLER_P2_TAG)
	PORT_BIT( 0xff, 0x7f, IPT_PADDLE ) PORT_MINMAX(0x01,0xfe) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_PLAYER(2)
INPUT_PORTS_END

void clowns_state::spacwalk(machine_config &config)
{
	mw8080bw_root(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_IO, &clowns_state::spacwalk_io_map);

	WATCHDOG_TIMER(config, m_watchdog).set_time(255 * attotime::from_hz(MW8080BW_60HZ));

	// add shifter
	MB14241(config, m_mb14241);

	// audio hardware
	SPACWALK_AUDIO(config, "soundboard").ctrl_sel_out().set([this] (int state) { m_controller_select = state ? 1U : 0U; });
}



/*************************************
 *
 *  Shuffleboard (PCB #643)
 *
 *************************************/

void mw8080bw_state::shuffle_io_map(address_map &map)
{
	map.global_mask(0xf);    /* yes, 4, and no mirroring on the read handlers */
	map(0x01, 0x01).r(m_mb14241, FUNC(mb14241_device::shift_result_r));
	map(0x02, 0x02).portr("IN0");
	map(0x03, 0x03).r(FUNC(mw8080bw_state::mw8080bw_shift_result_rev_r));
	map(0x04, 0x04).portr("IN1");
	map(0x05, 0x05).portr("IN2");
	map(0x06, 0x06).portr("IN3");

	map(0x01, 0x01).mirror(0x08).w(m_mb14241, FUNC(mb14241_device::shift_count_w));
	map(0x02, 0x02).mirror(0x08).w(m_mb14241, FUNC(mb14241_device::shift_data_w));
	map(0x04, 0x04).mirror(0x08).w(m_watchdog, FUNC(watchdog_timer_device::reset_w));
	map(0x05, 0x05).mirror(0x08).w(FUNC(mw8080bw_state::shuffle_audio_1_w));
	map(0x06, 0x06).mirror(0x08).w(FUNC(mw8080bw_state::shuffle_audio_2_w));
}


static INPUT_PORTS_START( shuffle )
	PORT_START("IN0")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Language ) ) PORT_CONDITION("IN0", 0x80, EQUALS, 0x80) PORT_DIPLOCATION("B3:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x01, DEF_STR( French ) )
	PORT_DIPSETTING(    0x02, DEF_STR( German ) )
	/* PORT_DIPSETTING( 0x03, DEF_STR( German ) ) */
	PORT_DIPNAME( 0x0c, 0x04, "Points to Win" ) PORT_CONDITION("IN0", 0x80, EQUALS, 0x80) PORT_DIPLOCATION("B3:3,4")
	PORT_DIPSETTING(    0x00, "Game 1 = 25, Game 2 = 11" )
	PORT_DIPSETTING(    0x04, "Game 1 = 35, Game 2 = 15" )
	PORT_DIPSETTING(    0x08, "Game 1 = 40, Game 2 = 18" )
	PORT_DIPSETTING(    0x0c, "Game 1 = 50, Game 2 = 21" )
	PORT_DIPNAME( 0x30, 0x10, DEF_STR( Coinage ) ) PORT_CONDITION("IN0", 0x80, EQUALS, 0x80) PORT_DIPLOCATION("B3:5,6")
	PORT_DIPSETTING(    0x30, "2 Coins per Player" )
	PORT_DIPSETTING(    0x20, "2 Coins/1 or 2 Players" )
	PORT_DIPSETTING(    0x10, "1 Coin per Player" )
	PORT_DIPSETTING(    0x00, "1 Coin/1 or 2 Players" )
	PORT_DIPNAME( 0x40, 0x40, "Time Limit" ) PORT_CONDITION("IN0", 0x80, EQUALS, 0x80) PORT_DIPLOCATION("B3:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_LOW, "B3:8" )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1  )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Game Select")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )  /* not connected */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )  /* not connected */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )  /* not connected */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )  /* not connected */

	PORT_START("IN2")
	PORT_BIT( 0xff, 0, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(10) PORT_KEYDELTA(50) PORT_PLAYER(1)

	PORT_START("IN3")
	PORT_BIT( 0xff, 0, IPT_TRACKBALL_X ) PORT_SENSITIVITY(10) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(1)
INPUT_PORTS_END


void mw8080bw_state::shuffle(machine_config &config)
{
	mw8080bw_root(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_IO, &mw8080bw_state::shuffle_io_map);

	WATCHDOG_TIMER(config, m_watchdog).set_time(255 * attotime::from_hz(MW8080BW_60HZ));

	/* add shifter */
	MB14241(config, m_mb14241);

	/* audio hardware */
	shuffle_audio(config);
}



/*************************************
 *
 *  Dog Patch (PCB #644)
 *
 *************************************/

void mw8080bw_state::dogpatch_io_map(address_map &map)
{
	map.global_mask(0x7);
	map(0x00, 0x00).mirror(0x04).portr("IN0");
	map(0x01, 0x01).mirror(0x04).portr("IN1");
	map(0x02, 0x02).mirror(0x04).portr("IN2");
	map(0x03, 0x03).mirror(0x04).r(m_mb14241, FUNC(mb14241_device::shift_result_r));

	map(0x01, 0x01).w(m_mb14241, FUNC(mb14241_device::shift_count_w));
	map(0x02, 0x02).w(m_mb14241, FUNC(mb14241_device::shift_data_w));
	map(0x03, 0x03).w("soundboard", FUNC(dogpatch_audio_device::write));
	map(0x04, 0x04).w(m_watchdog, FUNC(watchdog_timer_device::reset_w));
	map(0x05, 0x05).w("soundboard", FUNC(dogpatch_audio_device::tone_generator_lo_w));
	map(0x06, 0x06).w("soundboard", FUNC(dogpatch_audio_device::tone_generator_hi_w));
}


static const ioport_value dogpatch_controller_table[7] =
{
	0x07, 0x06, 0x04, 0x05, 0x01, 0x00, 0x02
};


static INPUT_PORTS_START( dogpatch )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x70, 0x30, IPT_POSITIONAL_V ) PORT_POSITIONS(7) PORT_REMAP_TABLE(dogpatch_controller_table) PORT_SENSITIVITY(5) PORT_KEYDELTA(10) PORT_CODE_DEC(KEYCODE_M) PORT_CODE_INC(KEYCODE_J) PORT_CENTERDELTA(0) PORT_REVERSE PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x70, 0x30, IPT_POSITIONAL_V ) PORT_POSITIONS(7) PORT_REMAP_TABLE(dogpatch_controller_table) PORT_SENSITIVITY(5) PORT_KEYDELTA(10) PORT_CODE_DEC(KEYCODE_N) PORT_CODE_INC(KEYCODE_H) PORT_CENTERDELTA(0) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)

	PORT_START("IN2")
	PORT_DIPNAME( 0x03, 0x02, "Number of Cans" ) PORT_CONDITION("IN2", 0x20, EQUALS, 0x20) PORT_DIPLOCATION("SW:1,2")
	PORT_DIPSETTING(    0x03, "10" )
	PORT_DIPSETTING(    0x02, "15" )
	PORT_DIPSETTING(    0x01, "20" )
	PORT_DIPSETTING(    0x00, "25" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coinage ) ) PORT_CONDITION("IN2", 0x20, EQUALS, 0x20) PORT_DIPLOCATION("SW:3,4")
	PORT_DIPSETTING(    0x08, "2 Coins per Player" )
	PORT_DIPSETTING(    0x0c, "2 Coins/1 or 2 Players" )
	PORT_DIPSETTING(    0x00, "1 Coin per Player" )
	PORT_DIPSETTING(    0x04, "1 Coin/1 or 2 Players" )
	PORT_DIPNAME( 0x10, 0x10, "Extended Time Reward" ) PORT_CONDITION("IN2", 0x20, EQUALS, 0x20) PORT_DIPLOCATION("SW:5")
	PORT_DIPSETTING(    0x10, "3 extra cans" )
	PORT_DIPSETTING(    0x00, "5 extra cans" )
	PORT_SERVICE_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW:6" )
	PORT_DIPNAME( 0xc0, 0x40, "Extended Time At" ) PORT_CONDITION("IN2", 0x20, EQUALS, 0x20) PORT_DIPLOCATION("SW:7,8")
	PORT_DIPSETTING(    0xc0, "150" )
	PORT_DIPSETTING(    0x80, "175" )
	PORT_DIPSETTING(    0x40, "225" )
	PORT_DIPSETTING(    0x00, "275" )
INPUT_PORTS_END


void mw8080bw_state::dogpatch(machine_config &config)
{
	mw8080bw_root(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_IO, &mw8080bw_state::dogpatch_io_map);

	WATCHDOG_TIMER(config, m_watchdog).set_time(255 * attotime::from_hz(MW8080BW_60HZ));

	// add shifter
	MB14241(config, m_mb14241);

	// audio hardware
	DOGPATCH_AUDIO(config, "soundboard");
}



/*************************************
 *
 *  Space Encounters (PCB #645)
 *
 *************************************/

void spcenctr_state::machine_start()
{
	mw8080bw_state::machine_start();

	m_trench_width = 0U;
	m_trench_center = 0U;
	std::fill(std::begin(m_trench_slope), std::end(m_trench_slope), 0U);
	m_bright_control = 0U;
	m_brightness = 0U;

	save_item(NAME(m_trench_width));
	save_item(NAME(m_trench_center));
	save_item(NAME(m_trench_slope));
	save_item(NAME(m_bright_control));
	save_item(NAME(m_brightness));
}


void spcenctr_state::io_w(offs_t offset, u8 data)
{                                           // A7 A6 A5 A4 A3 A2 A1 A0
	if ((offset & 0x07) == 0x00)
		// hex flip-flop B5
		// bit 3: /BRITE
		// bit 2: /NO_PLANET
		// bit 1: /SET_WSL
		// bit 0: COIN_COUNTER
		m_bright_control = BIT(~data, 3);   //  -  -  -  -  -  0  0  0
	else if ((offset & 0x5f) == 0x01)
		m_soundboard->p1_w(data);           //  -  0  -  0  0  0  0  1
	else if ((offset & 0x5f) == 0x09)
		m_soundboard->p2_w(data);           //  -  0  -  0  1  0  0  1
	else if ((offset & 0x5f) == 0x11)
		m_soundboard->p3_w(data);           //  -  0  -  1  0  0  0  1
	else if ((offset & 0x07) == 0x02)
		m_watchdog->watchdog_reset();       //  -  -  -  -  -  0  1  0
	else if ((offset & 0x07) == 0x03)
	{                                       //  -  -  -  -  -  0  1  1
		m_trench_slope[bitswap<4>(offset, 7, 6, 4, 3)] = data;
	}
	else if ((offset & 0x07) == 0x04)
		m_trench_center = data;             //  -  -  -  -  -  1  0  0
	else if ((offset & 0x07) == 0x07)
		m_trench_width = data;              //  -  -  -  -  -  1  1  1
	else
		logerror("%s:  Unmapped I/O port write to %02x = %02x\n", machine().describe_context(), offset, data);
}


void spcenctr_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).mirror(0xfc).portr("IN0");
	map(0x01, 0x01).mirror(0xfc).portr("IN1");
	map(0x02, 0x02).mirror(0xfc).portr("IN2");
	map(0x03, 0x03).mirror(0xfc).nopr();

	map(0x00, 0xff).w(FUNC(spcenctr_state::io_w)); // complicated addressing logic
}


static const ioport_value spcenctr_controller_table[] =
{
	0x3f, 0x3e, 0x3c, 0x3d, 0x39, 0x38, 0x3a, 0x3b,
	0x33, 0x32, 0x30, 0x31, 0x35, 0x34, 0x36, 0x37,
	0x27, 0x26, 0x24, 0x25, 0x21, 0x20, 0x22, 0x23,
	0x2b, 0x2a, 0x28, 0x29, 0x2d, 0x2c, 0x2e, 0x2f,
	0x0f, 0x0e, 0x0c, 0x0d, 0x09, 0x08, 0x0a, 0x0b,
	0x03, 0x02, 0x00, 0x01, 0x05, 0x04, 0x06, 0x07,
	0x17, 0x16, 0x14, 0x15, 0x11, 0x10, 0x12, 0x13,
	0x1b, 0x1a, 0x18, 0x19, 0x1d, 0x1c, 0x1e, 0x1f
};


static INPUT_PORTS_START( spcenctr )
	PORT_START("IN0")
	/* horizontal range is limited to 12 - 46 by stoppers on the control for 35 positions */
	PORT_BIT( 0x3f, 17, IPT_POSITIONAL ) PORT_POSITIONS(35) PORT_REMAP_TABLE(spcenctr_controller_table+12) PORT_SENSITIVITY(5) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_REVERSE /* 6 bit horiz encoder */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	/* vertical range is limited to 22 - 41 by stoppers on the control for 20 positions */
	PORT_BIT( 0x3f, 19, IPT_POSITIONAL_V ) PORT_POSITIONS(20) PORT_REMAP_TABLE(spcenctr_controller_table+22) PORT_SENSITIVITY(5) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_REVERSE /* 6 bit vert encoder - pushing control in makes ship move faster */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )  /* not connected */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )  /* marked as COIN #2, but the software never reads it */

	PORT_START("IN2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Bonus_Life ) ) PORT_CONDITION("IN2", 0x30, EQUALS, 0x00) PORT_DIPLOCATION("F3:1,2")
	PORT_DIPSETTING(    0x00, "2000 4000 8000" )
	PORT_DIPSETTING(    0x01, "3000 6000 12000" )
	PORT_DIPSETTING(    0x02, "4000 8000 16000" )
	PORT_DIPSETTING(    0x03, "5000 10000 20000" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coinage ) ) PORT_CONDITION("IN2", 0x30, NOTEQUALS, 0x10) PORT_DIPLOCATION("F3:3,4")
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x30, 0x00, "Bonus/Test Mode" ) PORT_DIPLOCATION("F3:5,6")
	PORT_DIPSETTING(    0x00, "Bonus On" )
	PORT_DIPSETTING(    0x30, "Bonus Off" )
	PORT_DIPSETTING(    0x20, "Cross Hatch" )
	PORT_DIPSETTING(    0x10, "Test Mode" )
	PORT_DIPNAME( 0xc0, 0x40, "Time" ) PORT_CONDITION("IN2", 0x30, NOTEQUALS, 0x10) PORT_DIPLOCATION("F3:7,8")
	PORT_DIPSETTING(    0x00, "45" )
	PORT_DIPSETTING(    0x40, "60" )
	PORT_DIPSETTING(    0x80, "75" )
	PORT_DIPSETTING(    0xc0, "90" )
INPUT_PORTS_END


void spcenctr_state::spcenctr(machine_config &config)
{
	mw8080bw_root(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_IO, &spcenctr_state::io_map);

	WATCHDOG_TIMER(config, m_watchdog).set_time(255 * attotime::from_hz(MW8080BW_60HZ));

	// video hardware
	m_screen->set_screen_update(FUNC(spcenctr_state::screen_update));

	// audio hardware
	SPCENCTR_AUDIO(config, m_soundboard);
}



/*************************************
 *
 *  Phantom II (PCB #652)
 *
 *************************************/


MACHINE_START_MEMBER(mw8080bw_state,phantom2)
{
	mw8080bw_state::machine_start();

	/* setup for save states */
	save_item(NAME(m_phantom2_cloud_counter));

	m_phantom2_cloud_counter = 0;
}


void mw8080bw_state::phantom2_io_map(address_map &map)
{
	map.global_mask(0x7);
	map(0x00, 0x00).mirror(0x04).r(FUNC(mw8080bw_state::mw8080bw_shift_result_rev_r));
	map(0x01, 0x01).mirror(0x04).portr("IN0");
	map(0x02, 0x02).mirror(0x04).portr("IN1");
	map(0x03, 0x03).mirror(0x04).r(m_mb14241, FUNC(mb14241_device::shift_result_r));

	map(0x01, 0x01).w(m_mb14241, FUNC(mb14241_device::shift_count_w));
	map(0x02, 0x02).w(m_mb14241, FUNC(mb14241_device::shift_data_w));
	map(0x04, 0x04).w(m_watchdog, FUNC(watchdog_timer_device::reset_w));
	map(0x05, 0x05).w("soundboard", FUNC(phantom2_audio_device::p1_w));
	map(0x06, 0x06).w("soundboard", FUNC(phantom2_audio_device::p2_w));
}


static INPUT_PORTS_START( phantom2 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )  /* not connected */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )  /* not connected */

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Coinage ) ) PORT_CONDITION("IN1", 0x20, EQUALS, 0x20) PORT_DIPLOCATION("SW:1")
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Game_Time ) ) PORT_CONDITION("IN1", 0x20, EQUALS, 0x20) PORT_DIPLOCATION("SW:2,3")
	PORT_DIPSETTING(    0x00, "45 seconds + 20 extended (at 20 points)" )
	PORT_DIPSETTING(    0x02, "60 seconds + 25 extended (at 25 points)" )
	PORT_DIPSETTING(    0x04, "75 seconds + 30 extended (at 30 points)" )
	PORT_DIPSETTING(    0x06, "90 seconds + 35 extended (at 35 points)" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) ) PORT_CONDITION("IN1", 0x20, EQUALS, 0x20) PORT_DIPLOCATION("SW:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unused ) ) PORT_CONDITION("IN1", 0x20, EQUALS, 0x20) PORT_DIPLOCATION("SW:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW:6" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unused ) ) PORT_CONDITION("IN1", 0x20, EQUALS, 0x20) PORT_DIPLOCATION("SW:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unused ) ) PORT_CONDITION("IN1", 0x20, EQUALS, 0x20) PORT_DIPLOCATION("SW:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


void mw8080bw_state::phantom2(machine_config &config)
{
	mw8080bw_root(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_IO, &mw8080bw_state::phantom2_io_map);

	MCFG_MACHINE_START_OVERRIDE(mw8080bw_state,phantom2)

	WATCHDOG_TIMER(config, m_watchdog).set_time(255 * attotime::from_hz(MW8080BW_60HZ));

	/* video hardware */
	m_screen->set_screen_update(FUNC(mw8080bw_state::screen_update_phantom2));
	m_screen->screen_vblank().set(FUNC(mw8080bw_state::screen_vblank_phantom2));

	/* add shifter */
	MB14241(config, m_mb14241);

	/* audio hardware */
	PHANTOM2_AUDIO(config, "soundboard");
}



/*************************************
 *
 *  Bowling Alley (PCB #730)
 *
 *************************************/

void bowler_state::machine_start()
{
	mw8080bw_state::machine_start();

	m_200_left_light.resolve();
	m_200_right_light.resolve();
	m_400_left_light.resolve();
	m_400_right_light.resolve();
	m_500_left_light.resolve();
	m_500_right_light.resolve();
	m_700_light.resolve();
	m_x_left_light.resolve();
	m_x_right_light.resolve();
	m_regulation_game_light.resolve();
	m_flash_game_light.resolve();
	m_straight_ball_light.resolve();
	m_hook_ball_light.resolve();
	m_select_game_light.resolve();
}

uint8_t bowler_state::shift_result_r()
{
	/* ZV - not too sure why this is needed, I don't see
	   anything unusual on the schematics that would cause
	   the bits to flip */

	return ~m_mb14241->shift_result_r();
}

void bowler_state::lights_1_w(uint8_t data)
{
	m_200_left_light = BIT(data, 0);

	m_400_left_light = BIT(data, 1);

	m_500_left_light = BIT(data, 2);

	m_700_light = BIT(data, 3);

	m_500_right_light = BIT(data, 4);

	m_400_right_light = BIT(data, 5);

	m_200_right_light = BIT(data, 6);

	m_x_left_light = BIT(data, 7);
	m_x_right_light = BIT(data, 7);
}


void bowler_state::lights_2_w(uint8_t data)
{
	m_regulation_game_light = BIT(data, 0);
	m_flash_game_light = BIT(~data, 0);

	m_straight_ball_light = BIT(data, 1);

	m_hook_ball_light = BIT(data, 2);

	m_select_game_light = BIT(data, 3);

	/* D4-D7 are not connected */
}


void bowler_state::io_map(address_map &map)
{
	map.global_mask(0xf);  /* no masking on the reads, all 4 bits are decoded */
	map(0x01, 0x01).r(FUNC(bowler_state::shift_result_r));
	map(0x02, 0x02).portr("IN0");
	map(0x03, 0x03).r(FUNC(bowler_state::mw8080bw_shift_result_rev_r));
	map(0x04, 0x04).portr("IN1");
	map(0x05, 0x05).portr("IN2");
	map(0x06, 0x06).portr("IN3");

	map(0x01, 0x01).w(m_mb14241, FUNC(mb14241_device::shift_count_w));
	map(0x02, 0x02).w(m_mb14241, FUNC(mb14241_device::shift_data_w));
	map(0x04, 0x04).w(m_watchdog, FUNC(watchdog_timer_device::reset_w));
	map(0x05, 0x05).w(FUNC(bowler_state::audio_1_w));
	map(0x06, 0x06).w(FUNC(bowler_state::audio_2_w));
	map(0x07, 0x07).w(FUNC(bowler_state::lights_1_w));
	map(0x08, 0x08).w(FUNC(bowler_state::audio_3_w));
	map(0x09, 0x09).w(FUNC(bowler_state::audio_4_w));
	map(0x0a, 0x0a).w(FUNC(bowler_state::audio_5_w));
	map(0x0e, 0x0e).w(FUNC(bowler_state::lights_2_w));
	map(0x0f, 0x0f).w(FUNC(bowler_state::audio_6_w));
}


static INPUT_PORTS_START( bowler )
	PORT_START("IN0")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Language ) ) PORT_CONDITION("IN0", 0x80, EQUALS, 0x00) PORT_DIPLOCATION("B3:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x01, DEF_STR( French ) )
	PORT_DIPSETTING(    0x02, DEF_STR( German ) )
	/*PORT_DIPSETTING(  0x03, DEF_STR( German ) ) */
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Demo_Sounds ) ) PORT_CONDITION("IN0", 0x80, EQUALS, 0x00) PORT_DIPLOCATION("B3:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )  /* every 17 minutes */
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Game_Time ) ) PORT_CONDITION("IN0", 0x80, EQUALS, 0x00) PORT_DIPLOCATION("B3:4")
	PORT_DIPSETTING(    0x00, "No Limit" )
	PORT_DIPSETTING(    0x08, "5 Minutes" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Coinage ) ) PORT_CONDITION("IN0", 0x80, EQUALS, 0x00) PORT_DIPLOCATION("B3:5")
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Difficulty ) ) PORT_CONDITION("IN0", 0x80, EQUALS, 0x00) PORT_DIPLOCATION("B3:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) ) PORT_CONDITION("IN0", 0x80, EQUALS, 0x00) PORT_DIPLOCATION("B3:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, "Cocktail (not functional)" )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_HIGH, "B3:8" )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Hook/Straight") PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Game Select") PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0xff, 0, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(10) PORT_KEYDELTA(50) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("IN3")
	PORT_BIT( 0xff, 0, IPT_TRACKBALL_X ) PORT_SENSITIVITY(10) PORT_KEYDELTA(10) PORT_PLAYER(1)
INPUT_PORTS_END


void bowler_state::bowler(machine_config &config)
{
	mw8080bw_root(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_IO, &bowler_state::io_map);

	WATCHDOG_TIMER(config, m_watchdog).set_time(255 * attotime::from_hz(MW8080BW_60HZ));

	/* add shifter */
	MB14241(config, m_mb14241);

	/* audio hardware */
	audio(config);
}



/*************************************
 *
 *  Space Invaders (PCB #739)
 *
 *************************************/

void invaders_state::machine_start()
{
	mw8080bw_state::machine_start();

	m_flip_screen = 0U;

	save_item(NAME(m_flip_screen));
}



ioport_value invaders_state::invaders_sw6_sw7_r()
{
	// upright PCB : switches visible
	// cocktail PCB: HI

	if (is_cabinet_cocktail())
		return 0x03;
	else
		return ioport(INVADERS_SW6_SW7_PORT_TAG)->read();
}


ioport_value invaders_state::invaders_sw5_r()
{
	// upright PCB : switch visible
	// cocktail PCB: HI

	if (is_cabinet_cocktail())
		return 0x01;
	else
		return ioport(INVADERS_SW5_PORT_TAG)->read();
}


ioport_value invaders_state::invaders_in0_control_r()
{
	// upright PCB : P1 controls
	// cocktail PCB: HI

	if (is_cabinet_cocktail())
		return 0x07;
	else
		return m_player_controls[0]->read();
}


ioport_value invaders_state::invaders_in1_control_r()
{
	return m_player_controls[0]->read();
}


ioport_value invaders_state::invaders_in2_control_r()
{
	// upright PCB : P1 controls
	// cocktail PCB: P2 controls

	return m_player_controls[is_cabinet_cocktail() ? 1 : 0]->read();
}


bool invaders_state::is_cabinet_cocktail()
{
	return BIT(m_cabinet_type->read(), 0);
}


void invaders_state::io_map(address_map &map)
{
	map.global_mask(0x7);
	map(0x00, 0x00).mirror(0x04).portr("IN0");
	map(0x01, 0x01).mirror(0x04).portr("IN1");
	map(0x02, 0x02).mirror(0x04).portr("IN2");
	map(0x03, 0x03).mirror(0x04).r(m_mb14241, FUNC(mb14241_device::shift_result_r));

	map(0x02, 0x02).w(m_mb14241, FUNC(mb14241_device::shift_count_w));
	map(0x03, 0x03).w("soundboard", FUNC(invaders_audio_device::p1_w));
	map(0x04, 0x04).w(m_mb14241, FUNC(mb14241_device::shift_data_w));
	map(0x05, 0x05).w("soundboard", FUNC(invaders_audio_device::p2_w));
	map(0x06, 0x06).w(m_watchdog, FUNC(watchdog_timer_device::reset_w));
}


static INPUT_PORTS_START( invaders )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_BIT( 0x06, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(invaders_state::invaders_sw6_sw7_r))
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x70, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(invaders_state::invaders_in0_control_r))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(invaders_state::invaders_sw5_r))

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(invaders_state::direct_coin_count), 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x70, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(invaders_state::invaders_in1_control_r))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW:3,4")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED ) // in the software, this is TILI, but not connected on the Midway PCB. Is this correct?
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW:2")
	PORT_DIPSETTING(    0x08, "1000" )
	PORT_DIPSETTING(    0x00, "1500" )
	PORT_BIT( 0x70, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(invaders_state::invaders_in2_control_r))
	PORT_DIPNAME( 0x80, 0x00, "Display Coinage" ) PORT_DIPLOCATION("SW:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	/* fake port for cabinet type */
	INVADERS_CAB_TYPE_PORT

	/* fake ports for handling the various input ports based on cabinet type */
	PORT_START(INVADERS_SW6_SW7_PORT_TAG)
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START(INVADERS_SW5_PORT_TAG)
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	INVADERS_CONTROL_PORT_P1

	INVADERS_CONTROL_PORT_P2
INPUT_PORTS_END


void invaders_state::invaders(machine_config &config)
{
	mw8080bw_root(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_IO, &invaders_state::io_map);

	WATCHDOG_TIMER(config, m_watchdog).set_time(255 * attotime::from_hz(MW8080BW_60HZ));

	// video hardware
	m_screen->set_screen_update(FUNC(invaders_state::screen_update_invaders));

	// add shifter
	MB14241(config, m_mb14241);

	// audio hardware
	INVADERS_AUDIO(config, "soundboard")
			.flip_screen_out().set(
					[this] (int state)
					{
						if (is_cabinet_cocktail()) // the flip screen line is only connected on the cocktail PCB
							m_flip_screen = state ? 1 : 0;
					});
}



/*************************************
 *
 *  Blue Shark (PCB #742)
 *
 *************************************/

#define BLUESHRK_COIN_INPUT_PORT_TAG    ("COIN")


ioport_value mw8080bw_state::blueshrk_coin_input_r()
{
	uint32_t ret = ioport(BLUESHRK_COIN_INPUT_PORT_TAG)->read();

	// FIXME: use PORT_CHANGED_MEMBER or PORT_WRITE_LINE_MEMBER instead of updating here
	machine().bookkeeping().coin_counter_w(0, !ret);

	return ret;
}


void mw8080bw_state::blueshrk_io_map(address_map &map)
{
	map.global_mask(0x7);
	map(0x00, 0x00).mirror(0x04).r(FUNC(mw8080bw_state::mw8080bw_shift_result_rev_r));
	map(0x01, 0x01).mirror(0x04).portr("IN0");
	map(0x02, 0x02).mirror(0x04).portr("IN1");
	map(0x03, 0x03).mirror(0x04).r(m_mb14241, FUNC(mb14241_device::shift_result_r));

	map(0x01, 0x01).w(m_mb14241, FUNC(mb14241_device::shift_count_w));
	map(0x02, 0x02).w(m_mb14241, FUNC(mb14241_device::shift_data_w));
	map(0x03, 0x03).w(FUNC(mw8080bw_state::blueshrk_audio_w));
	map(0x04, 0x04).w(m_watchdog, FUNC(watchdog_timer_device::reset_w));
}


static INPUT_PORTS_START( blueshrk )
	PORT_START(BLUESHRK_SPEAR_PORT_TAG)
	PORT_BIT( 0xff, 0x45, IPT_PADDLE ) PORT_CROSSHAIR(X, 1.0, 0.0, 0.139) PORT_MINMAX(0x08,0x82) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_PLAYER(1)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(mw8080bw_state::blueshrk_coin_input_r))
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) ) PORT_CONDITION("IN1", 0x80, EQUALS, 0x80) PORT_DIPLOCATION("SW:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT )  /* not shown on the schematics, instead DIP SW4 is connected here */
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) ) PORT_CONDITION("IN1", 0x80, EQUALS, 0x80) PORT_DIPLOCATION("SW:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x40, "Replay" ) PORT_CONDITION("IN1", 0x80, EQUALS, 0x80) PORT_DIPLOCATION("SW:6,7")
	PORT_DIPSETTING(    0x20, "14000" )
	PORT_DIPSETTING(    0x40, "18000" )
	PORT_DIPSETTING(    0x60, "22000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW:8" )

	/* fake port for reading the coin input */
	PORT_START(BLUESHRK_COIN_INPUT_PORT_TAG)
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


void mw8080bw_state::blueshrk(machine_config &config)
{
	mw8080bw_root(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_IO, &mw8080bw_state::blueshrk_io_map);

	WATCHDOG_TIMER(config, m_watchdog).set_time(255 * attotime::from_hz(MW8080BW_60HZ));

	/* add shifter */
	MB14241(config, m_mb14241);

	/* audio hardware */
	blueshrk_audio(config);
}



/*************************************
 *
 *  Space Invaders II (cocktail) (PCB #851)
 *
 *************************************/

void mw8080bw_state::invad2ct_io_map(address_map &map)
{
	map.global_mask(0x7);
	map(0x00, 0x00).mirror(0x04).portr("IN0");
	map(0x01, 0x01).mirror(0x04).portr("IN1");
	map(0x02, 0x02).mirror(0x04).portr("IN2");
	map(0x03, 0x03).mirror(0x04).r(m_mb14241, FUNC(mb14241_device::shift_result_r));

	map(0x01, 0x01).w("soundboard", FUNC(invad2ct_audio_device::p3_w));
	map(0x02, 0x02).w(m_mb14241, FUNC(mb14241_device::shift_count_w));
	map(0x03, 0x03).w("soundboard", FUNC(invad2ct_audio_device::p1_w));
	map(0x04, 0x04).w(m_mb14241, FUNC(mb14241_device::shift_data_w));
	map(0x05, 0x05).w("soundboard", FUNC(invad2ct_audio_device::p2_w));
	map(0x06, 0x06).w(m_watchdog, FUNC(watchdog_timer_device::reset_w));
	map(0x07, 0x07).w("soundboard", FUNC(invad2ct_audio_device::p4_w));
}


static INPUT_PORTS_START( invad2ct )
	PORT_START("IN0")
	PORT_SERVICE_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW:8" )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNUSED )  /* labeled NAMED RESET, but not read by the software */
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(mw8080bw_state::direct_coin_count), 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_START("IN2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW:3,4")
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW:2") /* this switch only changes the orientation of the score */
	PORT_DIPSETTING(    0x08, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY  PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY  PORT_PLAYER(2)
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW:1")
	PORT_DIPSETTING(    0x80, "1500" )
	PORT_DIPSETTING(    0x00, "2000" )
INPUT_PORTS_END


void mw8080bw_state::invad2ct(machine_config &config)
{
	mw8080bw_root(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_IO, &mw8080bw_state::invad2ct_io_map);

	WATCHDOG_TIMER(config, m_watchdog).set_time(255 * attotime::from_hz(MW8080BW_60HZ));

	// add shifter
	MB14241(config, m_mb14241);

	// audio hardware
	INVAD2CT_AUDIO(config, "soundboard");
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( seawolf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sw0041.h",   0x0000, 0x0400, CRC(8f597323) SHA1(b538277d3a633dd8a3179cff202f18d322e6fe17) )
	ROM_LOAD( "sw0042.g",   0x0400, 0x0400, CRC(db980974) SHA1(cc2a99b18695f61e0540c9f6bf8fe3b391dde4a0) )
	ROM_LOAD( "sw0043.f",   0x0800, 0x0400, CRC(e6ffa008) SHA1(385198434b08fe4651ad2c920d44fb49cfe0bc33) )
	ROM_LOAD( "sw0044.e",   0x0c00, 0x0400, CRC(c3557d6a) SHA1(bd345dd72fed8ce15da76c381782b025f71b006f) )
ROM_END

ROM_START( seawolfo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.h1",   0x0000, 0x0200, CRC(941b8f2b) SHA1(1a46f91478d902b1452962972d7097ae217488a3) )
	ROM_LOAD( "2.g1",   0x0200, 0x0200, CRC(c047ef88) SHA1(e731cbcd849ed0ad0c69a28f24e9986bf02c17e8) )
	ROM_LOAD( "3.f1",   0x0400, 0x0200, CRC(9624b1ab) SHA1(a5b234ad3216def8dd006496a0d02ce275b88fa0) )
	ROM_LOAD( "4.e1",   0x0600, 0x0200, CRC(553ff531) SHA1(0382f99f8cf148adae4a66db9693c8625250b3f5) )
	ROM_LOAD( "5.d1",   0x0800, 0x0200, CRC(e8e07d03) SHA1(053b28edcf34400c809d5195b825469ae7744ddb) )
	ROM_LOAD( "6.c1",   0x0a00, 0x0200, CRC(e2ffe499) SHA1(4e62aa14c510504872e76eacc298912d60b2e6fe) )
	ROM_LOAD( "7.b1",   0x0c00, 0x0200, CRC(d40a52b5) SHA1(ffa7bb9109248be748f92f173d22b9a8bed3875f) )
	ROM_LOAD( "8.a1",   0x0e00, 0x0200, CRC(da61df76) SHA1(49cae7772c0ee99aaba3a5d0981f970c85755872) )
ROM_END

ROM_START( seawolfa ) // PCB 80-900d
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "9316b-2863_9316b-h-sw.h1", 0x0000, 0x0800, CRC(e542c042) SHA1(98f4fc9a8c9e14e495fd4628ccd5ca631c575cd3) )
	ROM_LOAD( "9316b-2801_9316b-g-sw.g1", 0x0800, 0x0800, CRC(4c41a1d1) SHA1(673f28da9bdab2a56bdf18b40b69c4a16b6d0e48) )
ROM_END

ROM_START( gunfight )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "7609h.bin",  0x0000, 0x0400, CRC(0b117d73) SHA1(99d01313e251818d336281700e206d9003c71dae) )
	ROM_LOAD( "7609g.bin",  0x0400, 0x0400, CRC(57bc3159) SHA1(c177e3f72db9af17ab99b2481448ca26318184b9) )
	ROM_LOAD( "7609f.bin",  0x0800, 0x0400, CRC(8049a6bd) SHA1(215b068663e431582591001cbe028929fa96d49f) )
	ROM_LOAD( "7609e.bin",  0x0c00, 0x0400, CRC(773264e2) SHA1(de3f2e6841122bbe6e2fda5b87d37842c072289a) )
ROM_END

ROM_START( gunfighto )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gf-h.h",   0x0000, 0x0200, CRC(9d29cc7a) SHA1(3aef38948f1b82539e6c868ada6b9dcf2a743c4e) )
	ROM_LOAD( "gf-g.g",   0x0200, 0x0200, CRC(5816911b) SHA1(eeb5835d3db1db1075d78a95f1f0189489910cce) )
	ROM_LOAD( "gf-f.f",   0x0400, 0x0200, CRC(58f6ee8d) SHA1(03c3743424772202231d3066ce39d9c386887d22) )
	ROM_LOAD( "gf-e.e",   0x0600, 0x0200, CRC(59078036) SHA1(4f3c1f2eb6ce3a1354b4031a225857b37e56cfcd) )
	ROM_LOAD( "gf-d.d",   0x0800, 0x0200, CRC(2b64e17f) SHA1(8a5d52a859866f926ecd324ed97609102fa38e54) )
	ROM_LOAD( "gf-c.c",   0x0a00, 0x0200, CRC(e0bbf98c) SHA1(eada3fdf09a752af98fdefdfad8de0b59beec422) )
	ROM_LOAD( "gf-b.b",   0x0c00, 0x0200, CRC(91114108) SHA1(9480ddb45900b63ec295b983768e2825e06a0d71) )
	ROM_LOAD( "gf-a.a",   0x0e00, 0x0200, CRC(3fbf9a91) SHA1(c74986362bc9db2aa3f881b3c98fe44537632979) )
ROM_END

ROM_START( tornbase )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tb.h",       0x0000, 0x0800, CRC(653f4797) SHA1(feb4c802aa3e0c2a66823cd032496cca5742c883) )
	ROM_LOAD( "tb.g",       0x0800, 0x0800, CRC(b63dcdb3) SHA1(bdaa0985bcb5257204ee10faa11a4e02a38b9ac5) )
	ROM_LOAD( "tb.f",       0x1000, 0x0800, CRC(215e070c) SHA1(425915b37e5315f9216707de0850290145f69a30) )
ROM_END


ROM_START( 280zzzap )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "zzzaph",     0x0000, 0x0400, CRC(1fa86e1c) SHA1(b9cf16eb037ada73631ed24297e9e3b3bf6ab3cd) )
	ROM_LOAD( "zzzapg",     0x0400, 0x0400, CRC(9639bc6b) SHA1(b2e2497e421e79a411d07ebf2eed2bb8dc227003) )
	ROM_LOAD( "zzzapf",     0x0800, 0x0400, CRC(adc6ede1) SHA1(206bf2575696c4b14437f3db37a215ba33211943) )
	ROM_LOAD( "zzzape",     0x0c00, 0x0400, CRC(472493d6) SHA1(ae5cf4481ee4b78ca0d2f4d560d295e922aa04a7) )
	ROM_LOAD( "zzzapd",     0x1000, 0x0400, CRC(4c240ee1) SHA1(972475f80253bb0d24773a10aec26a12f28e7c23) )
	ROM_LOAD( "zzzapc",     0x1400, 0x0400, CRC(6e85aeaf) SHA1(ffa6bb84ef1f7c2d72fd26c24bd33aa014aeab7e) )
ROM_END


ROM_START( maze )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "maze.h",     0x0000, 0x0800, CRC(f2860cff) SHA1(62b3fd3d04bf9c5dd9b50964374fb884dc0ab79c) )
	ROM_LOAD( "maze.g",     0x0800, 0x0800, CRC(65fad839) SHA1(893f0a7621e7df19f777be991faff0db4a9ad571) )
ROM_END


ROM_START( boothill )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "romh.cpu",   0x0000, 0x0800, CRC(1615d077) SHA1(e59a26c2f2fc67ab24301e22d2e3f33043acdf72) )
	ROM_LOAD( "romg.cpu",   0x0800, 0x0800, CRC(65a90420) SHA1(9f36c44b5ae5b912cdbbeb9ff11a42221b8362d2) )
	ROM_LOAD( "romf.cpu",   0x1000, 0x0800, CRC(3fdafd79) SHA1(b18e8ac9df40c4687ac1acd5174eb99f2ef60081) )
	ROM_LOAD( "rome.cpu",   0x1800, 0x0800, CRC(374529f4) SHA1(18c57b79df0c66052eef40a694779a5ade15d0e0) )
ROM_END


ROM_START( checkmat )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "checkmat.h", 0x0000, 0x0400, CRC(3481a6d1) SHA1(f758599d6393398a6a8e6e7399dc1a3862604f65) )
	ROM_LOAD( "checkmat.g", 0x0400, 0x0400, CRC(df5fa551) SHA1(484ff9bfb95166ba09f34c753a7908a73de3cc7d) )
	ROM_LOAD( "checkmat.f", 0x0800, 0x0400, CRC(25586406) SHA1(39e0cf502735819a7e1d933e3686945fcfae21af) )
	ROM_LOAD( "checkmat.e", 0x0c00, 0x0400, CRC(59330d84) SHA1(453f95dd31968d439339c41e625481170437eb0f) )
	ROM_LOAD( "checkmat.d", 0x1000, 0x0400, NO_DUMP )   /* language ROM */
ROM_END


ROM_START( desertgu )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "9316.1h",    0x0000, 0x0800, CRC(c0030d7c) SHA1(4d0a3a59d4f8181c6e30966a6b1d19ba5b29c398) )
	ROM_LOAD( "9316.1g",    0x0800, 0x0800, CRC(1ddde10b) SHA1(8fb8e85844a8ec6c0722883013ecdd4eeaeb08c1) )
	ROM_LOAD( "9316.1f",    0x1000, 0x0800, CRC(808e46f1) SHA1(1cc4e9b0aa7e9546c133bd40d40ede6f2fbe93ba) )
	ROM_LOAD( "desertgu.e", 0x1800, 0x0800, CRC(ac64dc62) SHA1(202433dfb174901bd3b91e843d9d697a8333ef9e) )
ROM_END


ROM_START( roadrunm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "9316.1h",    0x0000, 0x0800, CRC(c0030d7c) SHA1(4d0a3a59d4f8181c6e30966a6b1d19ba5b29c398) )
	ROM_LOAD( "9316.1g",    0x0800, 0x0800, CRC(1ddde10b) SHA1(8fb8e85844a8ec6c0722883013ecdd4eeaeb08c1) )
	ROM_LOAD( "9316.1f",    0x1000, 0x0800, CRC(808e46f1) SHA1(1cc4e9b0aa7e9546c133bd40d40ede6f2fbe93ba) )
	ROM_LOAD( "9316.1e",    0x1800, 0x0800, CRC(06f01571) SHA1(6a72ff96a68aeeb0aca4834843c00b789c9bdaa0) )
ROM_END


ROM_START( dplay )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dplay619.h", 0x0000, 0x0800, CRC(6680669b) SHA1(49ad2333f81613c2f27231de60b415cbc254546a) )
	ROM_LOAD( "dplay619.g", 0x0800, 0x0800, CRC(0eec7e01) SHA1(2661e77061119d7d95d498807bd29d2630c6b6ab) )
	ROM_LOAD( "dplay619.f", 0x1000, 0x0800, CRC(3af4b719) SHA1(3122138ac36b1a129226836ddf1916d763d73e10) )
	ROM_LOAD( "dplay619.e", 0x1800, 0x0800, CRC(65cab4fc) SHA1(1ce7cb832e95e4a6d0005bf730eec39225b2e960) )
ROM_END


ROM_START( lagunar )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lagunar.h",  0x0000, 0x0800, CRC(0cd5a280) SHA1(89a744c912070f11b0b90b0cc92061e238b00b64) )
	ROM_LOAD( "lagunar.g",  0x0800, 0x0800, CRC(824cd6f5) SHA1(a74f6983787cf040eab6f19de2669c019962b9cb) )
	ROM_LOAD( "lagunar.f",  0x1000, 0x0800, CRC(62692ca7) SHA1(d62051bd1b45ca6e60df83942ff26a64ae25a97b) )
	ROM_LOAD( "lagunar.e",  0x1800, 0x0800, CRC(20e098ed) SHA1(e0c52c013f5e93794b363d7762ce0f34ba98c660) )
ROM_END


ROM_START( gmissile )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gm_623.h",   0x0000, 0x0800, CRC(a3ebb792) SHA1(30d9613de849c1a868056c5e28cf2a8608b63e88) )
	ROM_LOAD( "gm_623.g",   0x0800, 0x0800, CRC(a5e740bb) SHA1(963c0984953eb58fe7eab84fabb724ec6e29e706) )
	ROM_LOAD( "gm_623.f",   0x1000, 0x0800, CRC(da381025) SHA1(c9d0511567ed571b424459896ce7de0326850388) )
	ROM_LOAD( "gm_623.e",   0x1800, 0x0800, CRC(f350146b) SHA1(a07000a979b1a735754eca623cc880988924877f) )
ROM_END


ROM_START( m4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "m4.h",       0x0000, 0x0800, CRC(9ee2a0b5) SHA1(b81b4001c90ac6db25edd838652c42913022d9a9) )
	ROM_LOAD( "m4.g",       0x0800, 0x0800, CRC(0e84b9cb) SHA1(a7b74851979aaaa16496e506c487a18df14ab6dc) )
	ROM_LOAD( "m4.f",       0x1000, 0x0800, CRC(9ded9956) SHA1(449204a50efd3345cde815ca5f1fb596843a30ac) )
	ROM_LOAD( "m4.e",       0x1800, 0x0800, CRC(b6983238) SHA1(3f3b99b33135e144c111d2ebaac8f9433c269bc5) )
ROM_END


ROM_START( clowns )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "h2.cpu",     0x0000, 0x0400, CRC(ff4432eb) SHA1(997aee1e3669daa1d8169b4e103d04baaab8ea8d) )
	ROM_LOAD( "g2.cpu",     0x0400, 0x0400, CRC(676c934b) SHA1(72b681ca9ef23d820fdd297cc417932aecc9677b) )
	ROM_LOAD( "f2.cpu",     0x0800, 0x0400, CRC(00757962) SHA1(ef39211493393e97284a08eea63be0757643ac88) )
	ROM_LOAD( "e2.cpu",     0x0c00, 0x0400, CRC(9e506a36) SHA1(8aad486a72d148d8b03e7bec4c12abd14e425c5f) )
	ROM_LOAD( "d2.cpu",     0x1000, 0x0400, CRC(d61b5b47) SHA1(6051c0a2e81d6e975e82c2d48d0e52dc0d4723e3) )
	ROM_LOAD( "c2.cpu",     0x1400, 0x0400, CRC(154d129a) SHA1(61eebb319ee3a6be598b764b295c18a93a953c1e) )
ROM_END


ROM_START( clowns1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "clownsv1.h", 0x0000, 0x0400, CRC(5560c951) SHA1(b6972e1918604263579de577ec58fa6a91e8ff3e) )
	ROM_LOAD( "clownsv1.g", 0x0400, 0x0400, CRC(6a571d66) SHA1(e825f95863e901a1b648c74bb47098c8e74f179b) )
	ROM_LOAD( "clownsv1.f", 0x0800, 0x0400, CRC(a2d56cea) SHA1(61bc07e6a24a1980216453b4dd2688695193a4ae) )
	ROM_LOAD( "clownsv1.e", 0x0c00, 0x0400, CRC(bbd606f6) SHA1(1cbaa21d9834c8d76cf335fd118851591e815c86) )
	ROM_LOAD( "clownsv1.d", 0x1000, 0x0400, CRC(37b6ff0e) SHA1(bf83bebb6c14b3663ca86a180f9ae3cddb84e571) )
	ROM_LOAD( "clownsv1.c", 0x1400, 0x0400, CRC(12968e52) SHA1(71e4f09d30b992a4ac44b0e88e83b4f8a0f63caa) )
ROM_END

ROM_START( spacwalk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sw.h", 0x0000, 0x0400, CRC(1b07fc1f) SHA1(bc6423ebcfcc1d158bc44c1a577485682b0aa79b) )
	ROM_LOAD( "sw.g", 0x0400, 0x0400, CRC(52220910) SHA1(2d479b241d6a57f28a91d6a085f10cc3fd6787a1) )
	ROM_LOAD( "sw.f", 0x0800, 0x0400, CRC(787d4ef6) SHA1(42b24a80e750bb51b81caeaf418014e62f55810d) )
	ROM_LOAD( "sw.e", 0x0c00, 0x0400, CRC(d62d324b) SHA1(1c1ed2f9995d960f6dac79cae53fd4e82cb06640) )
	ROM_LOAD( "sw.d", 0x1000, 0x0400, CRC(17dcc591) SHA1(a6c96da27713e51f4d400ef3bb33654a40214aa8))
	ROM_LOAD( "sw.c", 0x1400, 0x0400, CRC(61aef726) SHA1(fbb8e90e0a0f7de4e5e5a37b9595a1be626ada9b) )
	ROM_LOAD( "sw.b", 0x1800, 0x0400, CRC(c59d45d0) SHA1(5e772772e235ab8c0615ec26334d2e192f297604))
	ROM_LOAD( "sw.a", 0x1c00, 0x0400, CRC(d563da07) SHA1(937b683dddfddbc1c0f2e45571657b569c0c4928) )
ROM_END


ROM_START( einning )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ei.h",       0x0000, 0x0800, CRC(eff9c7af) SHA1(316fffc972bd9935ead5ee4fd629bddc8a8ed5ce) )
	ROM_LOAD( "ei.g",       0x0800, 0x0800, CRC(5d1e66cb) SHA1(a5475362e12b7c251a05d67c2fd070cf7d333ad0) )
	ROM_LOAD( "ei.f",       0x1000, 0x0800, CRC(ed96785d) SHA1(d5557620227fcf6f30dcf6c8f5edd760d77d30ae) )
	ROM_LOAD( "ei.e",       0x1800, 0x0800, CRC(ad096a5d) SHA1(81d48302a0e039b8601a6aed7276e966592af693) )
	ROM_LOAD( "ei.b",       0x5000, 0x0800, CRC(56b407d4) SHA1(95e4be5b2f28192df85c6118079de2e68838b67c) )
ROM_END


ROM_START( shuffle )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "shuffle.h",  0x0000, 0x0800, CRC(0d422a18) SHA1(909c5b9e3c1194abd101cbf993a2ed7c8fbeb5d0) )
	ROM_LOAD( "shuffle.g",  0x0800, 0x0800, CRC(7db7fcf9) SHA1(f41b568f2340e5307a7a45658946cfd4cf4056bf) )
	ROM_LOAD( "shuffle.f",  0x1000, 0x0800, CRC(cd04d848) SHA1(f0f7e9bc483f08934d5c29568b4a7fe084623031) )
	ROM_LOAD( "shuffle.e",  0x1800, 0x0800, CRC(2c118357) SHA1(178db02aaa70963dd8dbcb9b8651209913c539af) )
ROM_END


ROM_START( dogpatch )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dogpatch.h", 0x0000, 0x0800, CRC(74ebdf4d) SHA1(6b31f9563b0f79fe9128ee83e85a3e2f90d7985b) )
	ROM_LOAD( "dogpatch.g", 0x0800, 0x0800, CRC(ac246f70) SHA1(7ee356c3218558a78ee0ff495f9f51ef88cac951) )
	ROM_LOAD( "dogpatch.f", 0x1000, 0x0800, CRC(a975b011) SHA1(fb807d9eefde7177d7fd7ab06fc2dbdc58ae6fcb) )
	ROM_LOAD( "dogpatch.e", 0x1800, 0x0800, CRC(c12b1f60) SHA1(f0504e16d2ce60a0fb3fc2af8c323bfca0143818) )
ROM_END


ROM_START( spcenctr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "9316b-4m33_m645h-09s0.h1", 0x0000, 0x0800, CRC(7458b2db) SHA1(c4f41efb8a35fd8bebc75bff0111476affe2b34d) )
	ROM_LOAD( "9316b-4m32_m645g-09s0.g1", 0x0800, 0x0800, CRC(1b873788) SHA1(6cdf0d602a65c7efcf8abe149c6172b4c7ab87a1) )
	ROM_LOAD( "9316b-4m31_m645f-09s0.f1", 0x1000, 0x0800, CRC(d4319c91) SHA1(30830595c220f490fe150ad018fbf4671bb71e02) )
	ROM_LOAD( "9316b-4m30_m645e-09s0.e1", 0x1800, 0x0800, CRC(9b9a1a45) SHA1(8023a05c13e8b541f9e2fe4d389e6a2dcd4766ea) )
	ROM_LOAD( "9316b-4m29_m645d-09s0.d1", 0x4000, 0x0800, CRC(294d52ce) SHA1(0ee63413c5caf60d45ae8bef08f6c07099d30f79) )
	ROM_LOAD( "9316b-4m28_m645c-09s0.c1", 0x4800, 0x0800, CRC(ce44c923) SHA1(9d35908de3194c5fe6fc8495ae413fa722018744) )
	ROM_LOAD( "9316b-4m27_m645b-09s0.b1", 0x5000, 0x0800, CRC(098070ab) SHA1(72ae344591df0174353dc2e3d22daf5a70e2261f) )
	ROM_LOAD( "9316b-4m26_m645a-09s0.a1", 0x5800, 0x0800, CRC(7f1d1f44) SHA1(2f4951171a55e7ac072742fa24eceeee6aca7e39) )
ROM_END


ROM_START( phantom2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "phantom2.h", 0x0000, 0x0800, CRC(0e3c2439) SHA1(450182e590845c651530b2c84e1f11fe2451dcf6) )
	ROM_LOAD( "phantom2.g", 0x0800, 0x0800, CRC(e8df3e52) SHA1(833925e44e686df4d4056bce4c0ffae3269d57df) )
	ROM_LOAD( "phantom2.f", 0x1000, 0x0800, CRC(30e83c6d) SHA1(fe34a3e4519a7e5ffe66e76fe974049988656b71) )
	ROM_LOAD( "phantom2.e", 0x1800, 0x0800, CRC(8c641cac) SHA1(c4986daacb7ed9efed59b022c6101240b0eddcdc) )

	ROM_REGION( 0x0800, "proms", 0 )      /* cloud graphics */
	ROM_LOAD( "p2clouds.f2",0x0000, 0x0800, CRC(dcdd2927) SHA1(d8d42c6594e36c12b40ee6342a9ad01a8bbdef75) )
ROM_END


ROM_START( bowler )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "h.cpu",      0x0000, 0x0800, CRC(74c29b93) SHA1(9cbd5b7b8a4c889406b6bc065360f74c036320b2) )
	ROM_LOAD( "g.cpu",      0x0800, 0x0800, CRC(ca26d8b4) SHA1(cf18991cde8044a961cf556f18c6eb60a7ade595) )
	ROM_LOAD( "f.cpu",      0x1000, 0x0800, CRC(ba8a0bfa) SHA1(bb017ddac58d031b249596b70ab1068cd1bad499) )
	ROM_LOAD( "e.cpu",      0x1800, 0x0800, CRC(4da65a40) SHA1(7795d59870fa722da89888e72152145662554080) )
	ROM_LOAD( "d.cpu",      0x4000, 0x0800, CRC(e7dbc9d9) SHA1(05049a69ee588de85db86df188e7670778b77e90) )
ROM_END


ROM_START( invaders )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "9316b-0869_m739h.h1", 0x0000, 0x0800, CRC(734f5ad8) SHA1(ff6200af4c9110d8181249cbcef1a8a40fa40b7f) )
	ROM_LOAD( "9316b-0856_m739g.g1", 0x0800, 0x0800, CRC(6bfaca4a) SHA1(16f48649b531bdef8c2d1446c429b5f414524350) )
	ROM_LOAD( "9316b-0855_m739f.f1", 0x1000, 0x0800, CRC(0ccead96) SHA1(537aef03468f63c5b9e11dd61e253f7ae17d9743) )
	ROM_LOAD( "9316b-0854_m739e.e1", 0x1800, 0x0800, CRC(14e538b0) SHA1(1d6ca0c99f9df71e2990b610deb9d7da0125e2d8) )
ROM_END


ROM_START( blueshrk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "blueshrk.h", 0x0000, 0x0800, CRC(4ff94187) SHA1(7cb80e2ccc34983bfd688c549ffc032d6dacf880) )
	ROM_LOAD( "blueshrk.g", 0x0800, 0x0800, CRC(e49368fd) SHA1(2495ba48532bb714361e4f0e94c9317161c6c77f) )
	ROM_LOAD( "blueshrk.f", 0x1000, 0x0800, CRC(86cca79d) SHA1(7b4633fb8033ee2c0e692135c383ebf57deef0e5) )
ROM_END


/*
CPUs
QTY     Type    clock   position    function
1x  unknown DIP40       main PCB 2n
7x  LM3900N         sound PCB ic2-ic8   Quad Operational Amplifier - sound
1x  LM377       sound PCB 1k    Dual Audio Amplifier - sound
1x  oscillator  19.968000   main PCB XTAL 8b

ROMs
QTY     Type    position    status
4x  TMS2708     main PCB 1m 1n 1r 1s    dumped
2x  MCM2708     main PCB 1j 1l  dumped

RAMs
QTY     Type    position
16x     D2107C  main PCB 1-16

Others
1x 18x2 edge connector (main PCB)
1x 36x2 female connector to sound PCB (main PCB)
1x 36x2 edge connector to main PCB (sound PCB)
1x 15x2 edge connector (sound PCB)
8x trimmer (sound PCB VR1-VR8)
1x 7 DIP switches banks (sound PCB 3j)
1x 4 DIP switches banks (sound PCB 3a)

Notes
main PCB is marked "CS210", "SI" and is labeled "C6902575"
sound PCB is marked "CS214" and is labeled "C6902574", "VOLUME"
*/

ROM_START( blueshrkmr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mr14.1s", 0x0000, 0x0400, CRC(ea2ba987) SHA1(929d2c72d81ad15c0f877b8b3421b909fdcc80f6) )
	ROM_LOAD( "mr15.1r", 0x0400, 0x0400, CRC(049337a4) SHA1(d0288ec63b170722c7f20e7d2e17f6ceeaacab27) )
	ROM_LOAD( "mr16.1n", 0x0800, 0x0400, CRC(3f9793ca) SHA1(859d0f43fcba7282017e8f4985e5e9217225a617) )
	ROM_LOAD( "mr17.1m", 0x0c00, 0x0400, CRC(0a73e0eb) SHA1(dfe1c58979642e15f07039d0cec17f975ccd2a48) )
	ROM_LOAD( "mr18.1l", 0x1000, 0x0400, CRC(a18bb930) SHA1(36ed5d6d3b3643ddf9bd087001dc6ece7fb8df63) )
	ROM_LOAD( "mr19.1j", 0x1400, 0x0400, CRC(23c63d02) SHA1(74ce4bd9fe2528896c5574affaf0ca132e62cf9e) )
ROM_END


ROM_START( blueshrkmr2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mr14.1s", 0x0000, 0x0400, CRC(ea2ba987) SHA1(929d2c72d81ad15c0f877b8b3421b909fdcc80f6) )
	ROM_LOAD( "mr15.1r", 0x0400, 0x0400, CRC(049337a4) SHA1(d0288ec63b170722c7f20e7d2e17f6ceeaacab27) )
	ROM_LOAD( "mr16.1n", 0x0800, 0x0400, CRC(8776920f) SHA1(10fe501c8260f2f7ea57e704789299420237af90) ) // sldh, label was peeled off
	ROM_LOAD( "mr17.1m", 0x0c00, 0x0400, CRC(0a73e0eb) SHA1(dfe1c58979642e15f07039d0cec17f975ccd2a48) )
	ROM_LOAD( "mr18.1l", 0x1000, 0x0400, CRC(a18bb930) SHA1(36ed5d6d3b3643ddf9bd087001dc6ece7fb8df63) )
	ROM_LOAD( "mr19.1j", 0x1400, 0x0400, CRC(23c63d02) SHA1(74ce4bd9fe2528896c5574affaf0ca132e62cf9e) )
ROM_END


ROM_START( invad2ct )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "invad2ct.h", 0x0000, 0x0800, CRC(51d02a71) SHA1(2fa82ddc2702a72de0a9559ec244b70ab3db3f18) )
	ROM_LOAD( "invad2ct.g", 0x0800, 0x0800, CRC(533ac770) SHA1(edb65c289027432dad7861a7d6abbda9223c13b1) )
	ROM_LOAD( "invad2ct.f", 0x1000, 0x0800, CRC(d1799f39) SHA1(f7f1ba34d57f9883241ba3ef90e34ed20dfb8003) )
	ROM_LOAD( "invad2ct.e", 0x1800, 0x0800, CRC(291c1418) SHA1(0d9f7973ed81d28c43ef8b96f1180d6629871785) )
	ROM_LOAD( "invad2ct.b", 0x5000, 0x0800, CRC(8d9a07c4) SHA1(4acbe15185d958b5589508dc0ea3a615fbe3bcca) )
	ROM_LOAD( "invad2ct.a", 0x5800, 0x0800, CRC(efdabb03) SHA1(33f4cf249e88e2b7154350e54c479eb4fa86f26f) )
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

// PCB #         year  rom         parent    machine   inp       state           init        monitor company,fullname,flags

/* 596 */ GAMEL( 1976, seawolf,    0,        seawolf,  seawolf,  seawolf_state,  empty_init, ROT0,   "Dave Nutting Associates / Midway", "Sea Wolf (set 1)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_seawolf )
/* 596 */ GAMEL( 1976, seawolfo,   seawolf,  seawolf,  seawolf,  seawolf_state,  empty_init, ROT0,   "Dave Nutting Associates / Midway", "Sea Wolf (set 2)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_seawolf )
/* 596 */ GAMEL( 1976, seawolfa,   seawolf,  seawolf,  seawolf,  seawolf_state,  empty_init, ROT0,   "Dave Nutting Associates / Midway", "Sea Wolf (set 3)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_seawolf )
/* 597 */ GAMEL( 1975, gunfight,   0,        gunfight, gunfight, gunfight_state, empty_init, ROT0,   "Dave Nutting Associates / Midway", "Gun Fight (set 1)", MACHINE_SUPPORTS_SAVE, layout_gunfight )
/* 597 */ GAMEL( 1975, gunfighto,  gunfight, gunfight, gunfight, gunfight_state, empty_init, ROT0,   "Dave Nutting Associates / Midway", "Gun Fight (set 2)", MACHINE_SUPPORTS_SAVE, layout_gunfight )
/* 604 Gun Fight (cocktail, dump does not exist) */
/* 605 */ GAME(  1976, tornbase,   0,        tornbase, tornbase, mw8080bw_state, empty_init, ROT0,   "Dave Nutting Associates / Midway / Taito", "Tornado Baseball / Ball Park", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
/* 610 */ GAMEL( 1976, 280zzzap,   0,        zzzap,    zzzap,    zzzap_state,    empty_init, ROT0,   "Dave Nutting Associates / Midway", "280-ZZZAP", MACHINE_SUPPORTS_SAVE, layout_280zzzap )
/* 611 */ GAMEL( 1976, maze,       0,        maze,     maze,     mw8080bw_state, empty_init, ROT0,   "Midway", "Amazing Maze", MACHINE_SUPPORTS_SAVE, layout_maze )
/* 612 */ GAME(  1977, boothill,   0,        boothill, boothill, boothill_state, empty_init, ROT0,   "Dave Nutting Associates / Midway", "Boot Hill", MACHINE_SUPPORTS_SAVE )
/* 615 */ GAME(  1977, checkmat,   0,        checkmat, checkmat, mw8080bw_state, empty_init, ROT0,   "Dave Nutting Associates / Midway", "Checkmate", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
/* 618 */ GAME(  1977, desertgu,   0,        desertgu, desertgu, desertgu_state, empty_init, ROT0,   "Dave Nutting Associates / Midway", "Desert Gun", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
/* 618 */ GAME(  1977, roadrunm,   desertgu, desertgu, desertgu, desertgu_state, empty_init, ROT0,   "Midway", "Road Runner (Midway)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
/* 619 */ GAME(  1977, dplay,      0,        dplay,    dplay,    dplay_state,    empty_init, ROT0,   "Midway", "Double Play", MACHINE_SUPPORTS_SAVE )
/* 622 */ GAMEL( 1977, lagunar,    0,        lagunar,  lagunar,  zzzap_state,    empty_init, ROT90,  "Midway", "Laguna Racer", MACHINE_SUPPORTS_SAVE, layout_lagunar )
/* 623 */ GAME(  1977, gmissile,   0,        gmissile, gmissile, boothill_state, empty_init, ROT0,   "Midway", "Guided Missile", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
/* 626 */ GAME(  1977, m4,         0,        m4,       m4,       boothill_state, empty_init, ROT0,   "Midway", "M-4", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
/* 630 */ GAMEL( 1978, clowns,     0,        clowns,   clowns,   clowns_state,   empty_init, ROT0,   "Midway", "Clowns (rev. 2)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_clowns )
/* 630 */ GAMEL( 1978, clowns1,    clowns,   clowns,   clowns1,  clowns_state,   empty_init, ROT0,   "Midway", "Clowns (rev. 1)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_clowns )
/* 640 */ GAMEL( 1978, spacwalk,   0,        spacwalk, spacwalk, clowns_state,   empty_init, ROT0,   "Midway", "Space Walk", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_spacwalk )
/* 642 */ GAME(  1978, einning,    0,        dplay,    einning,  dplay_state,    empty_init, ROT0,   "Midway / Taito", "Extra Inning / Ball Park II", MACHINE_SUPPORTS_SAVE )
/* 643 */ GAME(  1978, shuffle,    0,        shuffle,  shuffle,  mw8080bw_state, empty_init, ROT90,  "Midway", "Shuffleboard", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
/* 644 */ GAME(  1977, dogpatch,   0,        dogpatch, dogpatch, mw8080bw_state, empty_init, ROT0,   "Midway", "Dog Patch", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
/* 645 */ GAMEL( 1980, spcenctr,   0,        spcenctr, spcenctr, spcenctr_state, empty_init, ROT0,   "Midway", "Space Encounters", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_spcenctr )
/* 652 */ GAMEL( 1979, phantom2,   0,        phantom2, phantom2, mw8080bw_state, empty_init, ROT0,   "Midway", "Phantom II", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_phantom2 )
/* 730 */ GAME(  1978, bowler,     0,        bowler,   bowler,   bowler_state,   empty_init, ROT90,  "Midway", "Bowling Alley", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
/* 739 */ GAMEL( 1978, invaders,   0,        invaders, invaders, invaders_state, empty_init, ROT270, "Taito / Midway", "Space Invaders / Space Invaders M", MACHINE_SUPPORTS_SAVE, layout_invaders )
/* 742 */ GAME(  1978, blueshrk,   0,        blueshrk, blueshrk, mw8080bw_state, empty_init, ROT0,   "Midway", "Blue Shark", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
		  GAME(  1978, blueshrkmr, blueshrk, blueshrk, blueshrk, mw8080bw_state, empty_init, ROT0,   "bootleg (Model Racing)", "Blue Shark (Model Racing bootleg, set 1)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
		  GAME(  1978, blueshrkmr2,blueshrk, blueshrk, blueshrk, mw8080bw_state, empty_init, ROT0,   "bootleg (Model Racing)", "Blue Shark (Model Racing bootleg, set 2)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
/* 749 4 Player Bowling Alley (cocktail, dump does not exist) */
/* 851 */ GAMEL( 1980, invad2ct,   0,        invad2ct, invad2ct, mw8080bw_state, empty_init, ROT90,  "Midway", "Space Invaders II (Midway, cocktail)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_invad2ct )
/* 852 Space Invaders Deluxe (color hardware, not in this driver) */
/* 870 Space Invaders Deluxe (cocktail, dump does not exist) */
