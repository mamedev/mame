// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Atari Centipede hardware

    Games supported:
        * Centipede (5 sets)
        * Warlords
        * Millipede
        * Maze Invaders (prototype)
        * Bulls Eye Darts

    Millipede:
      136013-109.5p
      136013-110.5r

The Two Bit Score "Dux" hack was circulated and common enough for inclusion

****************************************************************************

    Main clock: XTAL = 12.096 MHz
    6502 Clock: XTAL/8 = 1.512 MHz (0.756 when accessing playfield RAM)
    Horizontal video frequency: HSYNC = XTAL/256/3 = 15.75 kHz
    Video frequency: VSYNC = HSYNC/263 ?? = 59.88593 Hz (not sure, could be /262)
    VBlank duration: 1/VSYNC * (23/263) = 1460 us


                  Centipede Memory map and Dip Switches
                  -------------------------------------

    Memory map for Centipede directly from the Atari schematics (1981).

     Address  R/W  D7 D6 D5 D4 D3 D2 D1 D0   Function
    --------------------------------------------------------------------------------------
    0000-03FF       D  D  D  D  D  D  D  D   RAM
    --------------------------------------------------------------------------------------
    0400-07BF       D  D  D  D  D  D  D  D   Playfield RAM
    07C0-07CF       D  D  D  D  D  D  D  D   Motion Object Picture
    07D0-07DF       D  D  D  D  D  D  D  D   Motion Object Vert.
    07E0-07EF       D  D  D  D  D  D  D  D   Motion Object Horiz.
    07F0-07FF             D  D  D  D  D  D   Motion Object Color
    --------------------------------------------------------------------------------------
    0800       R    D  D  D  D  D  D  D  D   Option Switch 1 (0 = On)
    0801       R    D  D  D  D  D  D  D  D   Option Switch 2 (0 = On)
    --------------------------------------------------------------------------------------
    0C00       R    D           D  D  D  D   Horizontal Mini-Track Ball tm Inputs
               R       D                     VBLANK  (1 = VBlank)
               R          D                  Self-Test  (0 = On)
               R             D               Cocktail Cabinet  (1 = Cocktail)
    0C01       R    D  D  D                  R,C,L Coin Switches (0 = On)
               R             D               SLAM  (0 = On)
               R                D            Player 2 Fire Switch (0 = On)
               R                   D         Player 1 Fire Switch (0 = On)
               R                      D      Player 2 Start Switch (0 = On)
               R                         D   Player 1 Start Switch (0 = On)

    0C02       R    D           D  D  D  D   Vertical Mini-Track Ball tm Inputs
    0C03       R    D  D  D  D               Player 1 Joystick (R,L,Down,Up)
               R                D  D  D  D   Player 2 Joystick   (0 = On)
    --------------------------------------------------------------------------------------
    1000-100F R/W   D  D  D  D  D  D  D  D   Custom Audio Chip
    1404       W                D  D  D  D   Playfield Color RAM
    140C       W                D  D  D  D   Motion Object Color RAM
    --------------------------------------------------------------------------------------
    1600       W    D  D  D  D  D  D  D  D   EA ROM Address & Data Latch
    1680       W                D  D  D  D   EA ROM Control Latch
    1700       R    D  D  D  D  D  D  D  D   EA ROM Read Data
    --------------------------------------------------------------------------------------
    1800       W                             IRQ Acknowledge
    --------------------------------------------------------------------------------------
    1C00       W    D                        Left Coin Counter (1 = On)
    1C01       W    D                        Center Coin Counter (1 = On)
    1C02       W    D                        Right Coin Counter (1 = On)
    1C03       W    D                        Player 1 Start LED (0 = On)
    1C04       W    D                        Player 2 Start LED (0 = On)
    1C07       W    D                        Track Ball Flip Control (0 = Player 1)
    --------------------------------------------------------------------------------------
    2000       W                             WATCHDOG
    2400       W                             Clear Mini-Track Ball Counters
    --------------------------------------------------------------------------------------
    2000-3FFF  R                             Program ROM
    --------------------------------------------------------------------------------------

    -EA ROM is an Erasable Reprogrammable rom to save the top 3 high scores
      and other stuff.


     Dip switches at N9 on the PCB

     8    7    6    5    4    3    2    1    Option
    -------------------------------------------------------------------------------------
                                  On   On    English $
                                  On   Off   German
                                  Off  On    French
                                  Off  Off   Spanish
    -------------------------------------------------------------------------------------
                        On   On              2 lives per game
                        On   Off             3 lives per game $
                        Off  On              4 lives per game
                        Off  Off             5 lives per game
    -------------------------------------------------------------------------------------
                                             Bonus life granted at every:
              On   On                        10,000 points
              On   Off                       12.000 points $
              Off  On                        15,000 points
              Off  Off                       20,000 points
    -------------------------------------------------------------------------------------
         On                                  Hard game difficulty
         Off                                 Easy game difficulty $
    -------------------------------------------------------------------------------------
    On                                       1-credit minimum $
    Off                                      2-credit minimum
    -------------------------------------------------------------------------------------

    $ = Manufacturer's suggested settings


     Dip switches at N8 on the PCB

     8    7    6    5    4    3    2    1    Option
    -------------------------------------------------------------------------------------
                                  On   On    Free play
                                  On   Off   1 coin for 2 credits
                                  Off  On    1 coin for 1 credit $
                                  Off  Off   2 coins for 1 credit
    -------------------------------------------------------------------------------------
                        On   On              Right coin mech X 1 $
                        On   Off             Right coin mech X 4
                        Off  On              Right coin mech X 5
                        Off  Off             Right coin mech X 6
    -------------------------------------------------------------------------------------
                   On                        Left coin mech X 1 $
                   Off                       Left coin mech X 2
    -------------------------------------------------------------------------------------
    On   On   On                             No bonus coins $
    On   On   Off                            For every 2 coins inserted, game logic
                                              adds 1 more coin
    On   Off  On                             For every 4 coins inserted, game logic
                                              adds 1 more coin
    On   Off  Off                            For every 4 coins inserted, game logic
                                              adds 2 more coin
    Off  On   On                             For every 5 coins inserted, game logic
                                              adds 1 more coin
    Off  On   Off                            For every 3 coins inserted, game logic
                                              adds 1 more coin
    -------------------------------------------------------------------------------------
    $ = Manufacturer's suggested settings

    Changes:
        30 Apr 98 LBO
        * Fixed test mode
        * Changed high score to use earom routines
        * Added support for alternate rom set

****************************************************************************

     Millipede driver by Ivan Mackintosh

              Memory map for Millipede from the Atari schematics (SP-217 1982)
              ----------------------------------------------------------------

     Address  R/W  D7 D6 D5 D4 D3 D2 D1 D0   Function
    --------------------------------------------------------------------------------------
    0000-03FF       D  D  D  D  D  D  D  D   RAM
    --------------------------------------------------------------------------------------
    0400-0410  R    D  D  D  D  D  D  D  D   I/OSO (0400-040F POKEY 1)
    0408       R    D  D  D  D  D  D  D  D   Option Switch 0 (0 = On) (Bank at
    --------------------------------------------------------------------------------------
    0800-0810  R    D  D  D  D  D  D  D  D   I/OS1 (0800-080F POKEY 2)
    0808       R    D  D  D  D  D  D  D  D   Option Switch 1 (0 = On) (Bank at
    --------------------------------------------------------------------------------------
    1000-13BF       D  D  D  D  D  D  D  D   Playfield RAM (8x8 TILES, 32x30 SCREEN)
    13C0-13CF       D  D  D  D  D  D  D  D   Motion Object Picture
    13D0-13DF       D  D  D  D  D  D  D  D   Motion Object Vert.
    13E0-13EF       D  D  D  D  D  D  D  D   Motion Object Horiz.
    13F0-13FF             D  D  D  D  D  D   Motion Object Color
    --------------------------------------------------------------------------------------
    2000       R    D                        Horizontal Mini-Track Ball HORIZ DIR
               R       D                     VBLANK  (1 = VBlank)
               R          D                  Player 1 Start
               R             D               Player 1 Fire
               R                D  D  D  D   Horizontal Mini-Track Ball HORIZ COUNT
               R                D  D  D  D   Options Switch 2 (Bottom 4 switches at P8)
    --------------------------------------------------------------------------------------
    2001       R    D                        Horizontal Mini-Track Ball VERT DIR
               R          D                  Player 2 Start
               R             D               Player 2 Fire
               R                D  D  D  D   Horizontal Mini-Track Ball VERT COUNT
               R                D  D  D  D   Options Switch 2 (Top 4 switches at P8)
    --------------------------------------------------------------------------------------
    2010       R    D  D  D                  R,C,L Coin Switches (0 = On)
               R             D               SLAM  (0 = On)
               R                D  D  D  D   P1 Joystick Positions  (0 = On)
    --------------------------------------------------------------------------------------
    2011       R    D                        Self-Test Switch (0 = On)
               R          D                  Cabinet Select  (1 = Upright, 0 = Cocktail)
               R                D  D  D  D   P2 Joystick Positions  (Undocumented)
    --------------------------------------------------------------------------------------
    2030       R    D  D  D  D  D  D  D  D   EA ROM Read Data
    --------------------------------------------------------------------------------------
    2480-248F  W    D  D  D  D  D  D  D  D   STAMP COLOR RAM
    2490-249F  W    D  D  D  D  D  D  D  D   MOTION OBJECT COLOR RAM
    --------------------------------------------------------------------------------------
    2501       W    D                        COIN CNTR L
    2502       W    D                        COIN CNTR R
    2503       W    D                        START LED 1
    2504       W    D                        START LED 2
    2505       W    D                        TRACKBALL ENABLE (TBEN)
    2506       W    D                        VIDEO ROTATE (VIDROT)
    2507       W    D                        CONTROL SELECT (CNTRLSEL) - (P1 = 1, P2 = 0?)
    --------------------------------------------------------------------------------------
    2600       W                             !IRQRES   - IRQ Acknowledge
    2680       W                             !WATCHDOG - CLEAR WATCHDOG
    2700       W                D  D  D  D   !EAROMCON - earom control
    2780       W    D  D  D  D  D  D  D  D   !EAROMWR  - earom write
    --------------------------------------------------------------------------------------
    3000-3FFF  R    D  D  D  D  D  D  D  D   ROM (NOT USED) (Schems listed 300-3fff typo)
    4000-7FFF  R    D  D  D  D  D  D  D  D   ROM            (Schems listed 400-4fff typo)
    --------------------------------------------------------------------------------------

            Switch Settings for Millipede from the Atari schematics (TM-217 1982)
            ---------------------------------------------------------------------

    Switch Settings for Price Options / 8-Toggle Switches on Millipede PCB (at B5)
    --------------------------------------------------------------------------------------
     8    7    6    5    4    3    2    1    Option
    --------------------------------------------------------------------------------------
    On   On                       Off  Off   Demonstration Mode
    On   Off  On                             For every 3 coins inserted, game logic
                                              adds 1 more coin
    On   Off  Off                            For every 5 coins inserted, game logic
                                              adds 1 more coin
    Off  On   On                             For every 4 coins inserted, game logic
                                              adds 2 more coin
    Off  On   Off                            For every 4 coins inserted, game logic
                                              adds 1 more coin
    Off  Off  On                             For every 2 coins inserted, game logic
                                              adds 1 more coin
    Off  Off  Off                            No Bonus Coins $
    --------------------------------------------------------------------------------------
                   Off                       Left coin mech X 1 $
                   On                        Left coin mech X 2
    --------------------------------------------------------------------------------------
                        Off  Off             Right coin mech X 1 $
                        Off  On              Right coin mech X 4
                        On   Off             Right coin mech X 5
                        On   On              Right coin mech X 6
    --------------------------------------------------------------------------------------
                                  On   On    2 coins for 1 credit
                                  On   Off   1 coin for 1 credit $
                                  Off  On    1 coin for 2 credits
                                  Off  Off   Free play
    --------------------------------------------------------------------------------------
    $ = Manufacturer's suggested settings


    Switch Settings for Play Options / 8-Toggle Switches on Millipede PCB (at D5)
    --------------------------------------------------------------------------------------

     8    7    6    5    4    3    2    1    Option
    --------------------------------------------------------------------------------------
    Off                                      Select Mode $
    On                                       No Select Mode
         Off                                 Easy spider $
         On                                  Hard spider
    --------------------------------------------------------------------------------------
              Off  Off                       Bonus life every 12,000 points
              Off  On                        Bonus life every 15,000 points $
              On   Off                       Bonus life every 20,000 points
              On   On                        No bonus life
    --------------------------------------------------------------------------------------
                        Off  Off             2 lives per game
                        Off  On              3 lives per game $
                        On   Off             4 lives per game
                        On   On              5 lives per game
    --------------------------------------------------------------------------------------
                                  Off        Easy beetle $
                                  On         Hard beetle
    --------------------------------------------------------------------------------------
                                       Off   Easy millipede head $
                                       On    Hard millipede head
    --------------------------------------------------------------------------------------
    $ = Manufacturer's suggested settings


    Switch settings for Special Options / 8-Toggle Switches on Millipede PCB (at P8)
    --------------------------------------------------------------------------------------

     8    7    6    5    4    3    2    1    Option
    --------------------------------------------------------------------------------------
    On                                       1 coin counter
    Off                                      2 coin counters
    --------------------------------------------------------------------------------------
         On                                  1 credit minimum $
         Off                                 2 credit minimum
    --------------------------------------------------------------------------------------
         (Switches 5 and 6 unused)           Select Mode Starting Score
                        On   On              0 points
                        On   Off             0 and bonus life level
                        Off  On              0, bonus life level, and 2x bonus life level $
                        Off  Off             0, bonus life level, and 2x bonus life level,
                                                and 3x bonus life level
    --------------------------------------------------------------------------------------
                                  On   On    English $
                                  On   Off   German
                                  Off  On    French
                                  Off  Off   Spanish
    --------------------------------------------------------------------------------------
    $ = Manufacturer's suggested settings


    Notes: 15 Feb 2007 - MSH
        * Real Millipede boards do not have the Joystick circuit hooked up
          to the card edge.
        * The Joytick and T-Ball inputs are both swapped through LS157s by
          the CNTRLSEL signal at 0x2507.
        * How do we hookup TBEN signal at 0x2505?

    Changes: 15 Feb 2007 - MSH
        * Added corrected memory map and dip switch settings from Atari Manuals.
        * Added Cocktail mode (based on v0.36.1 driver from Scott Brasington)
        * Removed unused dip toggles, now set to IPT_UNKNOWN
        * Hooked up 2nd trackball
        * Map P2 Joy inputs to work correctly. (They don't work on a real board)

****************************************************************************

                  Warlords Memory map and Dip Switches
                  ------------------------------------

     Address  R/W  D7 D6 D5 D4 D3 D2 D1 D0   Function
    --------------------------------------------------------------------------------------
    0000-03FF       D  D  D  D  D  D  D  D   RAM
    --------------------------------------------------------------------------------------
    0400-07BF       D  D  D  D  D  D  D  D   Screen RAM (8x8 TILES, 32x32 SCREEN)
    07C0-07CF       D  D  D  D  D  D  D  D   Motion Object Picture
    07D0-07DF       D  D  D  D  D  D  D  D   Motion Object Vert.
    07E0-07EF       D  D  D  D  D  D  D  D   Motion Object Horiz.
    --------------------------------------------------------------------------------------
    0800       R    D  D  D  D  D  D  D  D   Option Switch 1 (0 = On) (DSW 1)
    0801       R    D  D  D  D  D  D  D  D   Option Switch 2 (0 = On) (DSW 2)
    --------------------------------------------------------------------------------------
    0C00       R    D                        Cocktail Cabinet  (0 = Cocktail)
               R       D                     VBLANK  (1 = VBlank)
               R          D                  SELF TEST
               R             D               DIAG STEP (Unused)
    0C01       R    D  D  D                  R,C,L Coin Switches (0 = On)
               R             D               Slam (0 = On)
               R                D            Player 4 Start Switch (0 = On)
               R                   D         Player 3 Start Switch (0 = On)
               R                      D      Player 2 Start Switch (0 = On)
               R                         D   Player 1 Start Switch (0 = On)
    --------------------------------------------------------------------------------------
    1000-100F  W   D  D  D  D  D  D  D  D    Pokey
    --------------------------------------------------------------------------------------
    1800       W                             IRQ Acknowledge
    --------------------------------------------------------------------------------------
    1C00-1C02  W    D  D  D  D  D  D  D  D   Coin Counters
    --------------------------------------------------------------------------------------
    1C03-1C06  W    D  D  D  D  D  D  D  D   LEDs
    --------------------------------------------------------------------------------------
    4000       W                             Watchdog
    --------------------------------------------------------------------------------------
    5000-7FFF  R                             Program ROM
    --------------------------------------------------------------------------------------

    Game Option Settings - J2 (DSW1)
    =========================

    8   7   6   5   4   3   2   1       Option
    ------------------------------------------
                            On  On      English
                            On  Off     French
                            Off On      Spanish
                            Off Off     German
                        On              Music at end of each game
                        Off             Music at end of game for new highscore
            On  On                      1 or 2 player game costs 1 credit
            On  Off                     1 player game=1 credit, 2 player=2 credits
            Off Off                     1 or 2 player game costs 2 credits
            Off On                      Not used
    -------------------------------------------


    Game Price Settings - M2 (DSW2)
    ========================

    8   7   6   5   4   3   2   1       Option
    ------------------------------------------
                            On  On      Free play
                            On  Off     1 coin for 2 credits
                            Off On      1 coin for 1 credit
                            Off Off     2 coins for 1 credit
                    On  On              Right coin mech x 1
                    On  Off             Right coin mech x 4
                    Off On              Right coin mech x 5
                    Off Off             Right coin mech x 6
                On                      Left coin mech x 1
                Off                     Left coin mech x 2
    On  On  On                          No bonus coins
    On  On  Off                         For every 2 coins, add 1 coin
    On  Off On                          For every 4 coins, add 1 coin
    On  Off Off                         For every 4 coins, add 2 coins
    Off On  On                          For every 5 coins, add 1 coin
    ------------------------------------------

Maze Invaders:
http://www.atarigames.com/safestuff/mazeinvaders.html

-  The controls are somewhat like Food Fight in the way that they need to be
'calibrated' as you play or before you start by pushing a couple seconds in
each direction to assign the boundries.

***************************************************************************/

#include "emu.h"
#include "centiped.h"

#include "cpu/m6502/m6502.h"
#include "cpu/s2650/s2650.h"
#include "machine/rescap.h"
#include "machine/watchdog.h"
#include "sound/sn76496.h"
#include "sound/pokey.h"
#include "speaker.h"

/*************************************
 *
 *  Interrupts
 *
 *************************************/

TIMER_DEVICE_CALLBACK_MEMBER(centiped_state::generate_interrupt)
{
	int scanline = param;

	/* IRQ is clocked on the rising edge of 16V, equal to the previous 32V */
	if (scanline & 16)
		m_maincpu->set_input_line(0, ((scanline - 1) & 32) ? ASSERT_LINE : CLEAR_LINE);

	/* do a partial update now to handle sprite multiplexing (Maze Invaders) */
	m_screen->update_partial(scanline);
}

void centiped_state::machine_start()
{
	save_item(NAME(m_oldpos));
	save_item(NAME(m_sign));
	save_item(NAME(m_dsw_select));

	std::fill(std::begin(m_oldpos), std::end(m_oldpos), 0);
	std::fill(std::begin(m_sign), std::end(m_sign), 0);
	m_dsw_select = 0;
}

void multiped_state::machine_start()
{
	centiped_state::machine_start();

	m_prg_bank = 0;
	m_0xxx_base = 0x1400;

	save_item(NAME(m_prg_bank));
	save_item(NAME(m_0xxx_base));
}


MACHINE_RESET_MEMBER(centiped_state,centiped)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);

	if (m_earom.found())
		earom_control_w(0);
}


MACHINE_RESET_MEMBER(centiped_state,magworm)
{
	MACHINE_RESET_CALL_MEMBER(centiped);

	/* kludge: clear RAM so that magworm can be reset cleanly */
	memset(m_rambase, 0, 0x400);
}


void centiped_state::irq_ack_w(uint8_t data)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
}



/*************************************
 *
 *  Input ports
 *
 *************************************/

/*
 * This wrapper routine is necessary because Centipede requires a direction bit
 * to be set or cleared. The direction bit is held until the mouse is moved
 * again.
 *
 * There is a 4-bit counter, and two inputs from the trackball: DIR and CLOCK.
 * CLOCK makes the counter move in the direction of DIR. Since DIR is latched
 * only when a CLOCK arrives, the DIR bit in the input port doesn't change
 * until the trackball actually moves.
 *
 * There is also a CLR input to the counter which could be used by the game to
 * clear the counter, but Centipede doesn't use it (though it would be a good
 * idea to support it anyway).
 *
 * The counter is read 240 times per second. There is no provision whatsoever
 * to prevent the counter from wrapping around between reads.
 */

inline int centiped_state::read_trackball(int idx, int switch_port)
{
	uint8_t newpos;
	static const char *const portnames[] = { "IN0", "IN1", "IN2" };
	static const char *const tracknames[] = { "TRACK0_X", "TRACK0_Y", "TRACK1_X", "TRACK1_Y" };

	/* adjust idx if we're cocktail flipped */
	if (m_flipscreen)
		idx += 2;

	/* if we're to read the dipswitches behind the trackball data, do it now */
	if (m_dsw_select)
		return (ioport(portnames[switch_port])->read() & 0x7f) | m_sign[idx];

	/* get the new position and adjust the result */
	newpos = ioport(tracknames[idx])->read();
	if (newpos != m_oldpos[idx])
	{
		m_sign[idx] = (newpos - m_oldpos[idx]) & 0x80;
		m_oldpos[idx] = newpos;
	}

	/* blend with the bits from the switch port */
	return (ioport(portnames[switch_port])->read() & 0x70) | (m_oldpos[idx] & 0x0f) | m_sign[idx];
}


uint8_t centiped_state::centiped_IN0_r()
{
	return read_trackball(0, 0);
}


uint8_t centiped_state::centiped_IN2_r()
{
	return read_trackball(1, 2);
}


uint8_t centiped_state::milliped_IN1_r()
{
	return read_trackball(1, 1);
}

uint8_t centiped_state::milliped_IN2_r()
{
	uint8_t data = ioport("IN2")->read();

	/* MSH - 15 Feb, 2007
	 * The P2 X Joystick inputs are not properly handled in
	 * the Milliped code, so we are forcing the P2 inputs
	 * into the P1 Joystick handler, this require remapping
	 * the inputs, and has the good side effect of disabling
	 * the actual Joy1 inputs while control_select is no zero.
	 */
	if (m_control_select != 0)
	{
		/* Bottom 4 bits is our joystick inputs */
		uint8_t joy2data = ioport("IN3")->read() & 0x0f;
		data = data & 0xf0; /* Keep the top 4 bits */
		data |= (joy2data & 0x0a) >> 1; /* flip left and up */
		data |= (joy2data & 0x05) << 1; /* flip right and down */
	}
	return data;
}

void centiped_state::input_select_w(int state)
{
	m_dsw_select = !state;
}

/* used P2 controls if 1, P1 controls if 0 */
void centiped_state::control_select_w(int state)
{
	m_control_select = state;
}


uint8_t centiped_state::mazeinv_input_r()
{
	static const char *const sticknames[] = { "STICK0", "STICK1", "STICK2", "STICK3" };

	return ioport(sticknames[m_control_select])->read();
}


void centiped_state::mazeinv_input_select_w(offs_t offset, uint8_t data)
{
	m_control_select = offset & 3;
}

uint8_t centiped_state::bullsdrt_data_port_r()
{
	switch (m_maincpu->pc())
	{
		case 0x0033:
		case 0x6b19:
			return 0x01;

		default:
			break;
	}

	return 0;
}



/*************************************
 *
 *  Output ports
 *
 *************************************/

uint8_t centiped_state::caterplr_unknown_r()
{
	return machine().rand() % 0xff;
}


void centiped_state::coin_counter_left_w(int state)
{
	machine().bookkeeping().coin_counter_w(0, state);
}


void centiped_state::coin_counter_center_w(int state)
{
	machine().bookkeeping().coin_counter_w(1, state);
}


void centiped_state::coin_counter_right_w(int state)
{
	machine().bookkeeping().coin_counter_w(2, state);
}


void centiped_state::bullsdrt_coin_count_w(int state)
{
	machine().bookkeeping().coin_counter_w(0, state);
}



/*************************************
 *
 *  High score EAROM
 *
 *************************************/

uint8_t centiped_state::earom_read()
{
	return m_earom->data();
}

void centiped_state::earom_write(offs_t offset, uint8_t data)
{
	m_earom->set_address(offset & 0x3f);
	m_earom->set_data(data);
}

void centiped_state::earom_control_w(uint8_t data)
{
	// CK = DB0, C1 = /DB1, C2 = DB2, CS1 = DB3, /CS2 = GND
	m_earom->set_control(BIT(data, 3), 1, !BIT(data, 1), BIT(data, 2));
	m_earom->set_clk(BIT(data, 0));
}



/*************************************
 *
 *  Centipede CPU memory handlers
 *
 *************************************/

void centiped_state::centiped_base_map(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x03ff).ram().share("rambase");
	map(0x0400, 0x07bf).ram().w(FUNC(centiped_state::centiped_videoram_w)).share("videoram");
	map(0x07c0, 0x07ff).ram().share("spriteram");
	map(0x0800, 0x0800).portr("DSW1");
	map(0x0801, 0x0801).portr("DSW2");
	map(0x0c00, 0x0c00).r(FUNC(centiped_state::centiped_IN0_r));
	map(0x0c01, 0x0c01).portr("IN1");
	map(0x0c02, 0x0c02).r(FUNC(centiped_state::centiped_IN2_r));
	map(0x0c03, 0x0c03).portr("IN3");
	map(0x1400, 0x140f).w(FUNC(centiped_state::centiped_paletteram_w)).share("paletteram");
	map(0x1600, 0x163f).nopr().w(FUNC(centiped_state::earom_write));
	map(0x1680, 0x1680).w(FUNC(centiped_state::earom_control_w));
	map(0x1700, 0x173f).r(FUNC(centiped_state::earom_read));
	map(0x1800, 0x1800).w(FUNC(centiped_state::irq_ack_w));
	map(0x1c00, 0x1c07).nopr().w("outlatch", FUNC(ls259_device::write_d7));
	map(0x2000, 0x2000).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x2000, 0x3fff).rom();
}


void centiped_state::centiped_map(address_map &map)
{
	centiped_base_map(map);
	map(0x1000, 0x100f).rw("pokey", FUNC(pokey_device::read), FUNC(pokey_device::write));
}

void centiped_state::centipedj_map(address_map &map) // no trackball support
{
	centiped_map(map);
	map.global_mask(0x3fff);
	map(0x0c00, 0x0c00).portr("IN0");
	map(0x0c02, 0x0c02).portr("IN2");
}


//// Centipede bootlegs ////

void centiped_state::centipdb_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x03ff).mirror(0x4000).ram();
	map(0x0400, 0x07bf).mirror(0x4000).ram().w(FUNC(centiped_state::centiped_videoram_w)).share("videoram");
	map(0x07c0, 0x07ff).mirror(0x4000).ram().share("spriteram");
	map(0x0800, 0x0800).mirror(0x4000).portr("DSW1");
	map(0x0801, 0x0801).mirror(0x4000).portr("DSW2");
	map(0x0c00, 0x0c00).mirror(0x4000).r(FUNC(centiped_state::centiped_IN0_r));
	map(0x0c01, 0x0c01).mirror(0x4000).portr("IN1");
	map(0x0c02, 0x0c02).mirror(0x4000).r(FUNC(centiped_state::centiped_IN2_r));
	map(0x0c03, 0x0c03).mirror(0x4000).portr("IN3");
	map(0x1000, 0x1001).mirror(0x4000).w(m_aysnd, FUNC(ay8910_device::data_address_w));
	map(0x1001, 0x1001).mirror(0x4000).r(m_aysnd, FUNC(ay8910_device::data_r));
	map(0x1002, 0x100f).mirror(0x4000).noprw();
	map(0x1400, 0x140f).mirror(0x4000).w(FUNC(centiped_state::centiped_paletteram_w)).share("paletteram");
	map(0x1600, 0x163f).mirror(0x4000).nopr().w(FUNC(centiped_state::earom_write));
	map(0x1680, 0x1680).mirror(0x4000).w(FUNC(centiped_state::earom_control_w));
	map(0x1700, 0x173f).mirror(0x4000).r(FUNC(centiped_state::earom_read));
	map(0x1800, 0x1800).mirror(0x4000).w(FUNC(centiped_state::irq_ack_w));
	map(0x1c00, 0x1c07).mirror(0x4000).nopr().w("outlatch", FUNC(ls259_device::write_d7));
	map(0x2000, 0x2000).mirror(0x4000).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x2000, 0x27ff).rom();
	map(0x2800, 0x3fff).mirror(0x4000).rom();
	map(0x6000, 0x67ff).rom();
}


void centiped_state::magworm_map(address_map &map)
{
	centiped_base_map(map);
	map(0x1001, 0x1001).w(m_aysnd, FUNC(ay8910_device::address_w));
	map(0x1003, 0x1003).rw(m_aysnd, FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
}


void centiped_state::caterplr_map(address_map &map)
{
	centiped_base_map(map);
	map(0x1780, 0x1780).r(FUNC(centiped_state::caterplr_unknown_r));
	map(0x1000, 0x100f).rw(FUNC(centiped_state::caterplr_AY8910_r), FUNC(centiped_state::caterplr_AY8910_w));
}

void centiped_state::caterplr_AY8910_w(offs_t offset, uint8_t data)
{
	m_aysnd->address_w(offset);
	m_aysnd->data_w(data);
}

uint8_t centiped_state::caterplr_AY8910_r(offs_t offset)
{
	m_aysnd->address_w(offset);
	return m_aysnd->data_r();
}



/*************************************
 *
 *  Millipede CPU memory handlers
 *
 *************************************/

void centiped_state::milliped_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x03ff).ram();
	map(0x0400, 0x040f).rw("pokey", FUNC(pokey_device::read), FUNC(pokey_device::write));
	map(0x0800, 0x080f).rw("pokey2", FUNC(pokey_device::read), FUNC(pokey_device::write));
	map(0x1000, 0x13bf).ram().w(FUNC(centiped_state::centiped_videoram_w)).share("videoram");
	map(0x13c0, 0x13ff).ram().share("spriteram");
	map(0x2000, 0x2000).r(FUNC(centiped_state::centiped_IN0_r));
	map(0x2001, 0x2001).r(FUNC(centiped_state::milliped_IN1_r));
	map(0x2010, 0x2010).r(FUNC(centiped_state::milliped_IN2_r));
	map(0x2011, 0x2011).portr("IN3");
	map(0x2030, 0x2030).r(FUNC(centiped_state::earom_read));
	map(0x2480, 0x249f).w(FUNC(centiped_state::milliped_paletteram_w)).share("paletteram");
	map(0x2500, 0x2507).w("outlatch", FUNC(ls259_device::write_d7));
	map(0x2600, 0x2600).w(FUNC(centiped_state::irq_ack_w));
	map(0x2680, 0x2680).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x2700, 0x2700).w(FUNC(centiped_state::earom_control_w));
	map(0x2780, 0x27bf).w(FUNC(centiped_state::earom_write));
	map(0x4000, 0x7fff).rom();
}



/*************************************
 *
 *  Multipede CPU memory handlers
 *
 *************************************/

/*
 Multipede is a daughterboard kit produced by Braze Technologies
 from 2002(1st version) to 2004(1.2a). It allows users to play
 Centipede and Millipede on one board.
 The game can be switched at any time with P1+P2 start.

 - M6502 CPU (from main pcb)
 - 27C512 64KB EPROM
 - 93C46P E2PROM for saving highscore/settings
 - PALCE22V10H-25PC/4
 - 74LS245
 - 2*8KB gfx roms, on a separate smaller daughterboard
*/

void multiped_state::multiped_map(address_map &map)
{
	map(0x0000, 0x7fff).mirror(0x8000).bankr("rombank");
	map(0x0000, 0x03ff).ram();
	map(0x0400, 0x07ff).rw(FUNC(multiped_state::multiped_0xxx_r), FUNC(multiped_state::multiped_0xxx_w));
	map(0x0800, 0x080f).rw("pokey2", FUNC(pokey_device::read), FUNC(pokey_device::write));
	map(0x1000, 0x13bf).ram().w(FUNC(multiped_state::centiped_videoram_w)).share("videoram");
	map(0x13c0, 0x13ff).ram().share("spriteram");
	map(0x1400, 0x140f).rw("pokey", FUNC(pokey_device::read), FUNC(pokey_device::write));
	map(0x2000, 0x3fff).r(FUNC(multiped_state::multiped_2xxx_r));
	map(0x2480, 0x249f).mirror(0x8000).w(FUNC(multiped_state::milliped_paletteram_w)).share("paletteram");
	map(0x2500, 0x2507).mirror(0x8000).w("outlatch", FUNC(ls259_device::write_d7));
	map(0x2600, 0x2600).mirror(0x8000).w(FUNC(multiped_state::irq_ack_w));
	map(0x2680, 0x2680).mirror(0x8000).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x2700, 0x2700).mirror(0x8000).nopw();
	map(0x2780, 0x27bf).mirror(0x8000).nopw();
	map(0xa000, 0xa000).r(FUNC(multiped_state::centiped_IN0_r));
	map(0xa001, 0xa001).r(FUNC(multiped_state::milliped_IN1_r));
	map(0xa010, 0xa010).r(FUNC(multiped_state::milliped_IN2_r));
	map(0xa011, 0xa011).portr("IN3");
	map(0xa030, 0xa030).nopr();
	map(0xd000, 0xd7ff).w(FUNC(multiped_state::multiped_eeprom_w));
	map(0xd800, 0xd800).mirror(0x03ff).rw(FUNC(multiped_state::multiped_eeprom_r), FUNC(multiped_state::multiped_prgbank_w));
	map(0xdc00, 0xdc00).mirror(0x03ff).w(FUNC(multiped_state::multiped_gfxbank_w));
}

uint8_t multiped_state::multiped_0xxx_r(offs_t offset)
{
	return m_maincpu->space(AS_PROGRAM).read_byte(m_0xxx_base | offset);
}

void multiped_state::multiped_0xxx_w(offs_t offset, uint8_t data)
{
	m_maincpu->space(AS_PROGRAM).write_byte(m_0xxx_base | offset, data);
}

uint8_t multiped_state::multiped_2xxx_r(offs_t offset)
{
	return m_maincpu->space(AS_PROGRAM).read_byte(0xa000 | offset);
}

uint8_t multiped_state::multiped_eeprom_r()
{
	return m_eeprom->do_read() ? 0x80 : 0;
}

void multiped_state::multiped_eeprom_w(offs_t offset, uint8_t data)
{
	// a0: always high
	// a3-a7: always low
	// a8-a10: same as a0-a2
	// d0-d6: N/C?

	// a1 low: latch bit
	if (~offset & 2)
		m_eeprom->di_write((data & 0x80) ? 1 : 0);

	// a2 low: write latch or select next bit to read
	if (~offset & 4)
		m_eeprom->clk_write((~data & 0x80) ? ASSERT_LINE : CLEAR_LINE);

	// both high: reset
	else if (offset & 2)
		m_eeprom->cs_write((data & 0x80) ? ASSERT_LINE : CLEAR_LINE);
}

void multiped_state::multiped_prgbank_w(uint8_t data)
{
	// d0-d6: N/C?
	// d7: prg (and gfx) rom bank
	int bank = (data & 0x80) ? 1 : 0;
	if (bank != m_prg_bank)
	{
		m_prg_bank = bank;
		multiped_gfxbank_w(m_gfx_bank << 6);
		m_rombank->set_entry(bank);

		if (!bank)
		{
			// bank 0: Millipede
			// $0400-07ff: mirror of $1400-17ff (POKEY)
			// $2000-3fff: mirror of $a000-bfff (I/O ports)
			m_0xxx_base = 0x1400;
			m_maincpu->space(AS_PROGRAM).install_read_handler(0x2000, 0x3fff, read8sm_delegate(*this, FUNC(multiped_state::multiped_2xxx_r)));
		}
		else
		{
			// bank 1: Centipede
			// $0400-07ff: mirror of $1000-13ff (video and sprite RAM)
			// $2000-3fff: ROM
			m_0xxx_base = 0x1000;
			m_maincpu->space(AS_PROGRAM).install_rom(0x2000, 0x3fff, (uint8_t*)m_rombank->base() + 0x2000);
		}
	}
}



/*************************************
 *
 *  Warlords CPU memory handlers
 *
 *************************************/

void centiped_state::warlords_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x03ff).ram();
	map(0x0400, 0x07bf).ram().w(FUNC(centiped_state::centiped_videoram_w)).share("videoram");
	map(0x07c0, 0x07ff).ram().share("spriteram");
	map(0x0800, 0x0800).portr("DSW1");
	map(0x0801, 0x0801).portr("DSW2");
	map(0x0c00, 0x0c00).portr("IN0");
	map(0x0c01, 0x0c01).portr("IN1");
	map(0x1000, 0x100f).rw("pokey", FUNC(pokey_device::read), FUNC(pokey_device::write));
	map(0x1800, 0x1800).w(FUNC(centiped_state::irq_ack_w));
	map(0x1c00, 0x1c07).w("outlatch", FUNC(ls259_device::write_d7));
	map(0x4000, 0x4000).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x5000, 0x7fff).rom();
}



/*************************************
 *
 *  Maze Invaders CPU memory handlers
 *
 *************************************/

void centiped_state::mazeinv_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x03ff).ram();
	map(0x0400, 0x040f).rw("pokey", FUNC(pokey_device::read), FUNC(pokey_device::write));
	map(0x0800, 0x080f).rw("pokey2", FUNC(pokey_device::read), FUNC(pokey_device::write));
	map(0x1000, 0x13bf).ram().w(FUNC(centiped_state::centiped_videoram_w)).share("videoram");
	map(0x13c0, 0x13ff).ram().share("spriteram");
	map(0x2000, 0x2000).portr("IN0");
	map(0x2001, 0x2001).portr("IN1");
	map(0x2010, 0x2010).portr("IN2");
	map(0x2011, 0x2011).portr("IN3");
	map(0x2020, 0x2020).r(FUNC(centiped_state::mazeinv_input_r));
	map(0x2030, 0x2030).r(FUNC(centiped_state::earom_read));
	map(0x2480, 0x249f).w(FUNC(centiped_state::mazeinv_paletteram_w)).share("paletteram");
	map(0x2500, 0x2507).w("outlatch", FUNC(ls259_device::write_d7));
	map(0x2580, 0x2583).w(FUNC(centiped_state::mazeinv_input_select_w));
	map(0x2600, 0x2600).w(FUNC(centiped_state::irq_ack_w));
	map(0x2680, 0x2680).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x2700, 0x2700).w(FUNC(centiped_state::earom_control_w));
	map(0x2780, 0x27bf).w(FUNC(centiped_state::earom_write));
	map(0x3000, 0x7fff).rom();
}



/****************************************
 *
 *  Bulls Eye Darts CPU memory handlers
 *
 ****************************************/

void centiped_state::bullsdrt_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
	map(0x1000, 0x1000).mirror(0x6000).portr("DSW1");
	map(0x1080, 0x1080).mirror(0x6000).r(FUNC(centiped_state::centiped_IN0_r));
	map(0x1081, 0x1081).mirror(0x6000).portr("IN1");
	map(0x1082, 0x1082).mirror(0x6000).r(FUNC(centiped_state::centiped_IN2_r));
	map(0x1200, 0x123f).mirror(0x6000).rw(FUNC(centiped_state::earom_read), FUNC(centiped_state::earom_write));
	map(0x1280, 0x1280).mirror(0x6000).w(FUNC(centiped_state::earom_control_w));
	map(0x1300, 0x1300).mirror(0x6000).portr("DSW2");
	map(0x1400, 0x140f).mirror(0x6000).w(FUNC(centiped_state::centiped_paletteram_w)).share("paletteram");
	map(0x1480, 0x1487).mirror(0x6000).w("outlatch", FUNC(ls259_device::write_d7));
	map(0x1500, 0x1500).mirror(0x6000).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x1580, 0x1580).mirror(0x6000).noprw();
	map(0x1800, 0x1bbf).mirror(0x6000).w(FUNC(centiped_state::centiped_videoram_w)).share("videoram");
	map(0x1bc0, 0x1bff).mirror(0x6000).ram().share("spriteram");
	map(0x1c00, 0x1fff).mirror(0x6000).ram();
	map(0x2000, 0x2fff).rom();
	map(0x4000, 0x4fff).rom();
	map(0x6000, 0x6fff).rom();
}

void centiped_state::bullsdrt_port_map(address_map &map)
{
	map(0x00, 0x00).w(FUNC(centiped_state::bullsdrt_sprites_bank_w));
	map(0x20, 0x3f).w(FUNC(centiped_state::bullsdrt_tilesbank_w)).share("bullsdrt_bank");
}

void centiped_state::bullsdrt_data_map(address_map &map)
{
	map(S2650_DATA_PORT, S2650_DATA_PORT).r(FUNC(centiped_state::bullsdrt_data_port_r)).w("snsnd", FUNC(sn76496_device::write));
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

/* The input ports are identical for the real one and the bootleg one, except
   that one of the languages is Italian in the bootleg one instead of Spanish */

static INPUT_PORTS_START( centiped )
	PORT_START("IN0")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_CUSTOM )   /* trackball data */
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Cocktail ) )
	PORT_SERVICE( 0x20, IP_ACTIVE_LOW )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM )   /* trackball sign bit */

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("IN2")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_CUSTOM )   /* trackball data */
	PORT_BIT( 0x70, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM )   /* trackball sign bit */

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Language ) )     PORT_DIPLOCATION("N9:!1,!2")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x01, DEF_STR( German ) )
	PORT_DIPSETTING(    0x02, DEF_STR( French ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Spanish ) )
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Lives ) )        PORT_DIPLOCATION("N9:!3,!4")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPNAME( 0x30, 0x10, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("N9:!5,!6")
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x10, "12000" )
	PORT_DIPSETTING(    0x20, "15000" )
	PORT_DIPSETTING(    0x30, "20000" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("N9:!7")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x80, 0x00, "Credit Minimum" )        PORT_DIPLOCATION("N9:!8")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x80, "2" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Coinage ) )      PORT_DIPLOCATION("N8:!1,!2")
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x00, "Right Coin" )            PORT_DIPLOCATION("N8:!3,!4")
	PORT_DIPSETTING(    0x00, "*1" )
	PORT_DIPSETTING(    0x04, "*4" )
	PORT_DIPSETTING(    0x08, "*5" )
	PORT_DIPSETTING(    0x0c, "*6" )
	PORT_DIPNAME( 0x10, 0x00, "Left Coin" )             PORT_DIPLOCATION("N8:!5")
	PORT_DIPSETTING(    0x00, "*1" )
	PORT_DIPSETTING(    0x10, "*2" )
	PORT_DIPNAME( 0xe0, 0x00, "Bonus Coins" )           PORT_DIPLOCATION("N8:!6,!7,!8")
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPSETTING(    0x20, "3 credits/2 coins" )
	PORT_DIPSETTING(    0x40, "5 credits/4 coins" )
	PORT_DIPSETTING(    0x60, "6 credits/4 coins" )
	PORT_DIPSETTING(    0x80, "6 credits/5 coins" )
	PORT_DIPSETTING(    0xa0, "4 credits/3 coins" )

	PORT_START("TRACK0_X")  /* IN6, fake trackball input port. */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_REVERSE

	PORT_START("TRACK0_Y")  /* IN7, fake trackball input port. */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10)

	PORT_START("TRACK1_X")  /* IN8, fake trackball input port. */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_COCKTAIL

	PORT_START("TRACK1_Y")  /* IN9, fake trackball input port. */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_REVERSE PORT_COCKTAIL
INPUT_PORTS_END

static INPUT_PORTS_START( centipedj ) // no trackball support
	PORT_INCLUDE( centiped )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("TRACK0_X")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("TRACK0_Y")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("TRACK1_X")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("TRACK1_Y")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( caterplr )
	PORT_INCLUDE( centiped )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Language ) )     PORT_DIPLOCATION("N9:!1,!2")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x01, DEF_STR( German ) )
	PORT_DIPSETTING(    0x02, DEF_STR( French ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Italian ) )
INPUT_PORTS_END


static INPUT_PORTS_START( centiped4 )
	PORT_INCLUDE( centiped )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("IN3")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Language ) )     PORT_DIPLOCATION("SW1:!1,!2")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x01, DEF_STR( German ) )
	PORT_DIPSETTING(    0x02, DEF_STR( French ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Spanish ) )
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:!3,!4")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPNAME( 0x30, 0x10, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:!5,!6")
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x10, "12000" )
	PORT_DIPSETTING(    0x20, "15000" )
	PORT_DIPSETTING(    0x30, "20000" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x80, 0x00, "Credit Minimum" )        PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x80, "2" )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW2:!1,!2")
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x1c, 0x00, DEF_STR( Game_Time ) )    PORT_DIPLOCATION("SW2:!3,!4,!5")
	PORT_DIPSETTING(    0x00, "Untimed" )
	PORT_DIPSETTING(    0x04, "1 Minute" )
	PORT_DIPSETTING(    0x08, "2 Minutes" )
	PORT_DIPSETTING(    0x0c, "3 Minutes" )
	PORT_DIPSETTING(    0x10, "4 Minutes" )
	PORT_DIPSETTING(    0x14, "5 Minutes" )
	PORT_DIPSETTING(    0x18, "6 Minutes" )
	PORT_DIPSETTING(    0x1c, "7 Minutes" )
	PORT_DIPNAME( 0xe0, 0x00, "Bonus Coins" )           PORT_DIPLOCATION("SW2:!6,!7,!8")
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPSETTING(    0x20, "3 credits/2 coins" )
	PORT_DIPSETTING(    0x40, "5 credits/4 coins" )
	PORT_DIPSETTING(    0x60, "6 credits/4 coins" )
	PORT_DIPSETTING(    0x80, "6 credits/5 coins" )
	PORT_DIPSETTING(    0xa0, "4 credits/3 coins" )

	PORT_MODIFY("TRACK1_X") /* IN8, place for cocktail trackball (not used) */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("TRACK1_Y") /* IN9, place for cocktail trackball (not used) */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( magworm )
	PORT_INCLUDE( centiped )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_MODIFY("IN3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:!1,!2")
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x00, "Right Coin" )            PORT_DIPLOCATION("SW1:!3,!4")
	PORT_DIPSETTING(    0x00, "*3" )
	PORT_DIPSETTING(    0x04, "*7" )
	PORT_DIPSETTING(    0x08, "*1/2" )
	PORT_DIPSETTING(    0x0c, "*6" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Language ) )     PORT_DIPLOCATION("SW1:!5,!6")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x10, DEF_STR( German ) )
	PORT_DIPSETTING(    0x20, DEF_STR( French ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Spanish ) )
	PORT_DIPNAME( 0xc0, 0x40, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:!7,!8")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x40, "3" )
	PORT_DIPSETTING(    0x80, "4" )
	PORT_DIPSETTING(    0xc0, "5" )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "Left Coin" )             PORT_DIPLOCATION("SW2:!1")
	PORT_DIPSETTING(    0x00, "*1" )
	PORT_DIPSETTING(    0x01, "*2" )
	PORT_DIPNAME( 0x0e, 0x00, "Bonus Coins" )           PORT_DIPLOCATION("SW2:!2,!3,!4")
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPSETTING(    0x02, "3 credits/2 coins" )
	PORT_DIPSETTING(    0x04, "5 credits/4 coins" )
	PORT_DIPSETTING(    0x06, "6 credits/4 coins" )
	PORT_DIPSETTING(    0x08, "6 credits/5 coins" )
	PORT_DIPSETTING(    0x0a, "4 credits/3 coins" )
	PORT_DIPNAME( 0x30, 0x10, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:!5,!6")
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x10, "12000" )
	PORT_DIPSETTING(    0x20, "15000" )
	PORT_DIPSETTING(    0x30, "20000" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x80, 0x00, "Credit Minimum" )        PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x80, "2" )
INPUT_PORTS_END


static INPUT_PORTS_START( milliped )
	PORT_START("IN0")   /* $2000 */ /* see port 6 for x trackball */
	PORT_DIPNAME(0x03, 0x00, DEF_STR( Language ) )      PORT_DIPLOCATION("P8:!1,!2")
	PORT_DIPSETTING(   0x00, DEF_STR( English ) )
	PORT_DIPSETTING(   0x01, DEF_STR( German ) )
	PORT_DIPSETTING(   0x02, DEF_STR( French ) )
	PORT_DIPSETTING(   0x03, DEF_STR( Spanish ) )
	PORT_DIPNAME(0x0c, 0x04, "Bonus" )                  PORT_DIPLOCATION("P8:!3,!4")
	PORT_DIPSETTING(   0x00, "0" )
	PORT_DIPSETTING(   0x04, "0 1x" )
	PORT_DIPSETTING(   0x08, "0 1x 2x" )
	PORT_DIPSETTING(   0x0c, "0 1x 2x 3x" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM )       /* trackball sign bit */

	PORT_START("IN1")   /* $2001 */ /* see port 7 for y trackball */
	/* these bits are unused */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME(0x04, 0x00, "Credit Minimum" )         PORT_DIPLOCATION("P8:!7")
	PORT_DIPSETTING(   0x00, "1" )
	PORT_DIPSETTING(   0x04, "2" )
	PORT_DIPNAME(0x08, 0x00, "Coin Counters" )          PORT_DIPLOCATION("P8:!8")
	PORT_DIPSETTING(   0x00, "1" )
	PORT_DIPSETTING(   0x08, "2" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM )       /* trackball sign bit */

	PORT_START("IN2")   /* $2010 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("IN3")   /* $2011 */
	/* Note, joystick X input for player 2 are bad in software */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME(0x20, 0x20, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(   0x20, DEF_STR( Upright ) )
	PORT_DIPSETTING(   0x00, DEF_STR( Cocktail ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("DSW1")  /* $0408 */
	PORT_DIPNAME(0x01, 0x00, "Millipede Head" )         PORT_DIPLOCATION("D5:!1")
	PORT_DIPSETTING(   0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(   0x01, DEF_STR( Hard ) )
	PORT_DIPNAME(0x02, 0x00, "Beetle" )                 PORT_DIPLOCATION("D5:!2")
	PORT_DIPSETTING(   0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(   0x02, DEF_STR( Hard ) )
	PORT_DIPNAME(0x0c, 0x04, DEF_STR( Lives ) )         PORT_DIPLOCATION("D5:!3,!4")
	PORT_DIPSETTING(   0x00, "2" )
	PORT_DIPSETTING(   0x04, "3" )
	PORT_DIPSETTING(   0x08, "4" )
	PORT_DIPSETTING(   0x0c, "5" )
	PORT_DIPNAME(0x30, 0x10, DEF_STR( Bonus_Life ) )    PORT_DIPLOCATION("D5:!5,!6")
	PORT_DIPSETTING(   0x00, "12000" )
	PORT_DIPSETTING(   0x10, "15000" )
	PORT_DIPSETTING(   0x20, "20000" )
	PORT_DIPSETTING(   0x30, DEF_STR( None ) )
	PORT_DIPNAME(0x40, 0x00, "Spider" )                 PORT_DIPLOCATION("D5:!7")
	PORT_DIPSETTING(   0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(   0x40, DEF_STR( Hard ) )
	PORT_DIPNAME(0x80, 0x00, "Starting Score Select" )  PORT_DIPLOCATION("D5:!8")
	PORT_DIPSETTING(   0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x00, DEF_STR( On ) )

	PORT_START("DSW2")  /* $0808 */
	PORT_DIPNAME(0x03, 0x02, DEF_STR( Coinage ) )       PORT_DIPLOCATION("B5:!1,!2")
	PORT_DIPSETTING(   0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(   0x02, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(   0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(   0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME(0x0c, 0x00, "Right Coin" )             PORT_DIPLOCATION("B5:!3,!4")
	PORT_DIPSETTING(   0x00, "*1" )
	PORT_DIPSETTING(   0x04, "*4" )
	PORT_DIPSETTING(   0x08, "*5" )
	PORT_DIPSETTING(   0x0c, "*6" )
	PORT_DIPNAME(0x10, 0x00, "Left Coin" )              PORT_DIPLOCATION("B5:!5")
	PORT_DIPSETTING(   0x00, "*1" )
	PORT_DIPSETTING(   0x10, "*2" )
	PORT_DIPNAME(0xe0, 0x00, "Bonus Coins" )            PORT_DIPLOCATION("B5:!6,!7,!8")
	PORT_DIPSETTING(   0x00, DEF_STR( None ) )
	PORT_DIPSETTING(   0x20, "3 credits/2 coins" )
	PORT_DIPSETTING(   0x40, "5 credits/4 coins" )
	PORT_DIPSETTING(   0x60, "6 credits/4 coins" )
	PORT_DIPSETTING(   0x80, "6 credits/5 coins" )
	PORT_DIPSETTING(   0xa0, "4 credits/3 coins" )
	PORT_DIPSETTING(   0xc0, "Demo Mode" )

	PORT_START("TRACK0_X")  /* IN6, fake trackball input port. */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_REVERSE

	PORT_START("TRACK0_Y")  /* IN7, fake trackball input port. */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10)

	PORT_START("TRACK1_X")  /* IN8, fake trackball input port. */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_COCKTAIL

	PORT_START("TRACK1_Y")  /* IN9, fake trackball input port. */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_REVERSE PORT_COCKTAIL
INPUT_PORTS_END


static INPUT_PORTS_START( multiped )
	// milliped board, but hard-dips are not used
	PORT_INCLUDE( milliped )

	PORT_MODIFY("IN0")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x00, "P8:!1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x00, "P8:!2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x00, "P8:!3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x00, "P8:!4" )

	PORT_MODIFY("IN1")
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x00, "P8:!7" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x00, "P8:!8" )

	PORT_MODIFY("DSW1")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x00, "D5:!1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x00, "D5:!2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x00, "D5:!3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x00, "D5:!4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x00, "D5:!5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x00, "D5:!6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x00, "D5:!7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x00, "D5:!8" )

	PORT_MODIFY("DSW2")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x00, "B5:!1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x00, "B5:!2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x00, "B5:!3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x00, "B5:!4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x00, "B5:!5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x00, "B5:!6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x00, "B5:!7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x00, "B5:!8" )
INPUT_PORTS_END


static INPUT_PORTS_START( warlords )
	PORT_START("IN0")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x10, 0x00, "Diag Step" )             /* Not referenced */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_SERVICE( 0x20, IP_ACTIVE_LOW )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x80, "Upright (overlay)" )
	PORT_DIPSETTING(    0x00, "Cocktail (no overlay)" )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Language ) )     PORT_DIPLOCATION("J2:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x01, DEF_STR( French ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Spanish ) )
	PORT_DIPSETTING(    0x03, DEF_STR( German ) )
	PORT_DIPNAME( 0x04, 0x00, "Music" )                 PORT_DIPLOCATION("J2:3")
	PORT_DIPSETTING(    0x00, "End of game" )
	PORT_DIPSETTING(    0x04, "High score only" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x00, "J2:4" )
	PORT_DIPNAME( 0x30, 0x00, "Credits" )               PORT_DIPLOCATION("J2:5,6")
	PORT_DIPSETTING(    0x00, "1p/2p = 1 credit" )
	PORT_DIPSETTING(    0x10, "1p = 1, 2p = 2" )
	PORT_DIPSETTING(    0x20, "1p/2p = 2 credits" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x00, "J2:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x00, "J2:8" )


	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Coinage ) )      PORT_DIPLOCATION("M2:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x00, "Right Coin" )            PORT_DIPLOCATION("M2:3,4")
	PORT_DIPSETTING(    0x00, "*1" )
	PORT_DIPSETTING(    0x04, "*4" )
	PORT_DIPSETTING(    0x08, "*5" )
	PORT_DIPSETTING(    0x0c, "*6" )
	PORT_DIPNAME( 0x10, 0x00, "Left Coin" )             PORT_DIPLOCATION("M2:5")
	PORT_DIPSETTING(    0x00, "*1" )
	PORT_DIPSETTING(    0x10, "*2" )
	PORT_DIPNAME( 0xe0, 0x00, "Bonus Coins" )           PORT_DIPLOCATION("M2:6,7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPSETTING(    0x20, "3 credits/2 coins" )
	PORT_DIPSETTING(    0x40, "5 credits/4 coins" )
	PORT_DIPSETTING(    0x60, "6 credits/4 coins" )
	PORT_DIPSETTING(    0x80, "6 credits/5 coins" )

	/* IN4-7 fake to control player paddles */
	PORT_START("PADDLE0")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x1d,0xcb) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_PLAYER(1)

	PORT_START("PADDLE1")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x1d,0xcb) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_PLAYER(2)

	PORT_START("PADDLE2")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x1d,0xcb) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_PLAYER(3)

	PORT_START("PADDLE3")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x1d,0xcb) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_PLAYER(4)
INPUT_PORTS_END


static INPUT_PORTS_START( mazeinv )
	PORT_START("IN0")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Language ) )     PORT_DIPLOCATION("SW0:!1,!2")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x01, DEF_STR( German ) )
	PORT_DIPSETTING(    0x02, DEF_STR( French ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Spanish ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "SW0:!3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x00, "SW0:!4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x00, "SW0:!5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x00, "SW0:!6" )
	PORT_DIPNAME( 0x40, 0x00, "Minimum credits" )       PORT_DIPLOCATION("SW0:!7")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))

	PORT_START("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Reverse")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Fire")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_TILT )       // No Function?
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN3 )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P1 Reverse")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Fire")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Cabinet ) )
	PORT_DIPSETTING( 0x20, DEF_STR( Upright ) )
	PORT_DIPSETTING( 0x00, DEF_STR( Cocktail ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, "Doors for bonus" )       PORT_DIPLOCATION("SW1:!1,!2")
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPSETTING(    0x01, "12" )
	PORT_DIPSETTING(    0x02, "14" )
	PORT_DIPSETTING(    0x03, "16" )
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:!3,!4")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPNAME( 0x30, 0x00, "Extra life at" )         PORT_DIPLOCATION("SW1:!5,!6")
	PORT_DIPSETTING(    0x00, "20000" )
	PORT_DIPSETTING(    0x10, "25000" )
	PORT_DIPSETTING(    0x20, "30000" )
	PORT_DIPSETTING(    0x30, "Never" )
	PORT_DIPNAME( 0xc0, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:!7,!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Easiest ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Harder ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW2:!1,!2")
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x00, "Coin 2" )                PORT_DIPLOCATION("SW2:!3,!4")
	PORT_DIPSETTING(    0x00, "*1" )
	PORT_DIPSETTING(    0x04, "*4" )
	PORT_DIPSETTING(    0x08, "*5" )
	PORT_DIPSETTING(    0x0c, "*6" )
	PORT_DIPNAME( 0x10, 0x00, "Coin 3" )                PORT_DIPLOCATION("SW2:!5")
	PORT_DIPSETTING(    0x00, "*1" )
	PORT_DIPSETTING(    0x10, "*2" )
	PORT_DIPNAME( 0xe0, 0x00, "Bonus Coins" )           PORT_DIPLOCATION("SW2:!6,!7,!8")
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPSETTING(    0x20, "3 credits/2 coins" )
	PORT_DIPSETTING(    0x40, "5 credits/4 coins" )
	PORT_DIPSETTING(    0x60, "6 credits/4 coins" )
	PORT_DIPSETTING(    0x80, "6 credits/5 coins" )

	/* IN6-9 fake to control player joysticks */
	PORT_START("STICK0")
	PORT_BIT( 0xff, 0x7f, IPT_AD_STICK_Y ) PORT_MINMAX(0x40, 0xbf) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("STICK1")
	PORT_BIT( 0xff, 0x7f, IPT_AD_STICK_Y ) PORT_MINMAX(0x40, 0xbf) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("STICK2")
	PORT_BIT( 0xff, 0x7f, IPT_AD_STICK_X ) PORT_MINMAX(0x40, 0xbf) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("STICK3")
	PORT_BIT( 0xff, 0x7f, IPT_AD_STICK_X ) PORT_MINMAX(0x40, 0xbf) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( bullsdrt )
	PORT_START("IN0")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_CUSTOM )   /* trackball data */
	PORT_BIT( 0x30, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM )   /* trackball sign bit */

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN3 )

	PORT_START("IN2")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_CUSTOM )   /* trackball data */
	PORT_BIT( 0x70, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM )   /* trackball sign bit */

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Award Free Game" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("TRACK0_X")  /* fake trackball input port. */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_REVERSE

	PORT_START("TRACK0_Y")  /* fake trackball input port. */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10)

	/* 2008-06 FP: was bullsdrt available as cocktail? If not, these can be removed */
	PORT_START("TRACK1_X")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("TRACK1_Y")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics layouts: Centipede/Millipede
 *
 *************************************/

static const gfx_layout spritelayout =
{
	8,16,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(1,2), 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	16*8
};

static GFXDECODE_START( gfx_centiped )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x2_planar,     0, 1 )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout,         4, 4*4*4 )
GFXDECODE_END

static GFXDECODE_START( gfx_milliped )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x2_planar,     0, 4 )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout,       4*4, 4*4*4*4 )
GFXDECODE_END



/*************************************
 *
 *  Graphics layouts: Warlords
 *
 *************************************/

static const gfx_layout warlords_charlayout =
{
	8,8,
	RGN_FRAC(1,4),
	2,
	{ RGN_FRAC(1,2), 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( gfx_warlords )
	GFXDECODE_ENTRY( "gfx1", 0x000, warlords_charlayout, 0,   8 )
	GFXDECODE_ENTRY( "gfx1", 0x200, warlords_charlayout, 8*4, 8*4 )
GFXDECODE_END


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

void centiped_state::centiped_base(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, 12096000/8);  /* 1.512 MHz (slows down to 0.75MHz while accessing playfield RAM) */

	MCFG_MACHINE_RESET_OVERRIDE(centiped_state,centiped)

	ER2055(config, m_earom);

	LS259(config, m_outlatch);
	m_outlatch->q_out_cb<0>().set(FUNC(centiped_state::coin_counter_left_w));
	m_outlatch->q_out_cb<1>().set(FUNC(centiped_state::coin_counter_center_w));
	m_outlatch->q_out_cb<2>().set(FUNC(centiped_state::coin_counter_right_w));
	m_outlatch->q_out_cb<3>().set_output("led0").invert(); // LED 1
	m_outlatch->q_out_cb<4>().set_output("led1").invert(); // LED 2

	WATCHDOG_TIMER(config, "watchdog").set_vblank_count("screen", 8);

	/* timer */
	TIMER(config, "32v").configure_scanline(FUNC(centiped_state::generate_interrupt), "screen", 0, 16);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 0*8, 30*8-1);
	m_screen->set_screen_update(FUNC(centiped_state::screen_update_centiped));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_centiped);
	PALETTE(config, m_palette).set_entries(4+4*4*4*4);

	MCFG_VIDEO_START_OVERRIDE(centiped_state,centiped)
}

void centiped_state::centiped(machine_config &config)
{
	centiped_base(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &centiped_state::centiped_map);

	// M10
	m_outlatch->q_out_cb<7>().set(FUNC(centiped_state::flip_screen_w));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	pokey_device &pokey(POKEY(config, "pokey", 12096000/8));
	pokey.set_output_opamp_low_pass(RES_K(3.3), CAP_U(0.01), 5.0);
	pokey.add_route(ALL_OUTPUTS, "mono", 0.5);
}

void centiped_state::centipedj(machine_config &config)
{
	centiped(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &centiped_state::centipedj_map);
}

void centiped_state::caterplr(machine_config &config)
{
	centiped_base(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &centiped_state::caterplr_map);

	m_outlatch->q_out_cb<7>().set(FUNC(centiped_state::flip_screen_w));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	AY8910(config, m_aysnd, 12096000/8).add_route(ALL_OUTPUTS, "mono", 1.0);
}

void centiped_state::centipdb(machine_config &config)
{
	centiped_base(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &centiped_state::centipdb_map);

	m_outlatch->q_out_cb<7>().set(FUNC(centiped_state::flip_screen_w));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	AY8910(config, m_aysnd, 12096000/8);
	m_aysnd->port_a_read_callback().set(FUNC(centiped_state::caterplr_unknown_r));
	m_aysnd->add_route(ALL_OUTPUTS, "mono", 2.0);
}

void centiped_state::magworm(machine_config &config)
{
	centiped_base(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &centiped_state::magworm_map);

	MCFG_MACHINE_RESET_OVERRIDE(centiped_state,magworm)

	// 12A
	m_outlatch->q_out_cb<7>().set(FUNC(centiped_state::flip_screen_w));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	AY8912(config, m_aysnd, 12096000/8).add_route(ALL_OUTPUTS, "mono", 2.0); // AY-3-8912 at 2/3H
}

void centiped_state::milliped(machine_config &config)
{
	centiped_base(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &centiped_state::milliped_map);

	// 12E
	m_outlatch->q_out_cb<5>().set(FUNC(centiped_state::input_select_w)); // TBEN
	m_outlatch->q_out_cb<6>().set(FUNC(centiped_state::flip_screen_w));
	m_outlatch->q_out_cb<7>().set(FUNC(centiped_state::control_select_w)); // CNTRLSEL

	/* video hardware */
	m_gfxdecode->set_info(gfx_milliped);
	m_palette->set_entries(4*4+4*4*4*4*4);

	MCFG_VIDEO_START_OVERRIDE(centiped_state,milliped)
	m_screen->set_screen_update(FUNC(centiped_state::screen_update_milliped));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	pokey_device &pokey1(POKEY(config, "pokey", 12096000/8));
	pokey1.allpot_r().set_ioport("DSW1");
	pokey1.add_route(ALL_OUTPUTS, "mono", 0.50);

	pokey_device &pokey2(POKEY(config, "pokey2", 12096000/8));
	pokey2.allpot_r().set_ioport("DSW2");
	pokey2.add_route(ALL_OUTPUTS, "mono", 0.50);
}

void multiped_state::multiped(machine_config &config)
{
	milliped(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &multiped_state::multiped_map);

	config.device_remove("earom");
	EEPROM_93C46_8BIT(config, m_eeprom);
}

void centiped_state::warlords(machine_config &config)
{
	centiped_base(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &centiped_state::warlords_map);

	config.device_remove("earom");

	// these extra LEDs also appear on Centipede schematics
	// P9
	m_outlatch->q_out_cb<5>().set_output("led2").invert(); // LED 3
	m_outlatch->q_out_cb<6>().set_output("led3").invert(); // LED 4

	/* video hardware */
	m_gfxdecode->set_info(gfx_warlords);
	m_palette->set_entries(8*4+8*4);
	m_palette->set_init(FUNC(centiped_state::warlords_palette));

	MCFG_VIDEO_START_OVERRIDE(centiped_state,warlords)

	m_screen->set_screen_update(FUNC(centiped_state::screen_update_warlords));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	pokey_device &pokey(POKEY(config, "pokey", 12096000/8));
	pokey.pot_r<0>().set_ioport("PADDLE0");
	pokey.pot_r<1>().set_ioport("PADDLE1");
	pokey.pot_r<2>().set_ioport("PADDLE2");
	pokey.pot_r<3>().set_ioport("PADDLE3");
	pokey.add_route(ALL_OUTPUTS, "mono", 1.0);
}

void centiped_state::mazeinv(machine_config &config)
{
	milliped(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &centiped_state::mazeinv_map);

	m_outlatch->q_out_cb<7>().set_nop();

	m_screen->set_screen_update(FUNC(centiped_state::screen_update_centiped));
}

void centiped_state::bullsdrt(machine_config &config)
{
	/* basic machine hardware */
	S2650(config, m_maincpu, 12096000/8);
	m_maincpu->set_addrmap(AS_PROGRAM, &centiped_state::bullsdrt_map);
	m_maincpu->set_addrmap(AS_IO, &centiped_state::bullsdrt_port_map);
	m_maincpu->set_addrmap(AS_DATA, &centiped_state::bullsdrt_data_map);

	ER2055(config, m_earom);

	LS259(config, m_outlatch);
	m_outlatch->q_out_cb<1>().set(FUNC(centiped_state::bullsdrt_coin_count_w));
	m_outlatch->q_out_cb<3>().set_output("led0").invert();
	m_outlatch->q_out_cb<4>().set_output("led1").invert();
	m_outlatch->q_out_cb<7>().set(FUNC(centiped_state::flip_screen_w));

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 0*8, 30*8-1);
	m_screen->set_screen_update(FUNC(centiped_state::screen_update_bullsdrt));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_centiped);
	PALETTE(config, m_palette).set_entries(4+4*4*4*4);

	MCFG_VIDEO_START_OVERRIDE(centiped_state,bullsdrt)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	sn76496_device &snsnd(SN76496(config, "snsnd", 12096000/8));
	snsnd.add_route(ALL_OUTPUTS, "mono", 1.0);
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

/*
    Alt. part numbers for mask ROMs:

    136001-*03 = 136001-*07
    136001-*04 = 136001-*08
    136001-*05 = 136001-*09
    136001-*06 = 136001-*10
    136001-*01 = 136001-*11
    136001-*02 = 136001-*12
*/

ROM_START( centiped )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "136001-407.d1",  0x2000, 0x0800, CRC(c4d995eb) SHA1(d0b2f0461cfa35842045d40ffb65e777703b773e) )
	ROM_LOAD( "136001-408.e1",  0x2800, 0x0800, CRC(bcdebe1b) SHA1(53f3bf88a79ce40661c0a9381928e55d8c61777a) )
	ROM_LOAD( "136001-409.fh1", 0x3000, 0x0800, CRC(66d7b04a) SHA1(8fa758095b618085090491dfb5ea114cdc87f9df) )
	ROM_LOAD( "136001-410.j1",  0x3800, 0x0800, CRC(33ce4640) SHA1(780c2eb320f64fad6b265c0dada961646ed30174) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "136001-211.f7",  0x0000, 0x0800, CRC(880acfb9) SHA1(6c862352c329776f2f9974a0df9dbe41f9dbc361) )
	ROM_LOAD( "136001-212.hj7", 0x0800, 0x0800, CRC(b1397029) SHA1(974c03d29aeca672fffa4dfc00a06be6a851aacb) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "136001-213.p4",  0x0000, 0x0100, CRC(6fa3093a) SHA1(2b7aeca74c1ae4156bf1878453a047330f96f0a8) )
ROM_END


ROM_START( centiped3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "136001-307.d1",  0x2000, 0x0800, CRC(5ab0d9de) SHA1(8ea6e3304202831aabaf31dbd0f970a7b3bfe421) )
	ROM_LOAD( "136001-308.e1",  0x2800, 0x0800, CRC(4c07fd3e) SHA1(af4fdbf32c23b1864819d620a874e7f205da3cdb) )
	ROM_LOAD( "136001-309.fh1", 0x3000, 0x0800, CRC(ff69b424) SHA1(689fa560d40a384dcbcad7c8095bc12e91875580) )
	ROM_LOAD( "136001-310.j1",  0x3800, 0x0800, CRC(44e40fa4) SHA1(c557db83876afc8ab52047ab1a3c3bfef34d6351) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "136001-211.f7",  0x0000, 0x0800, CRC(880acfb9) SHA1(6c862352c329776f2f9974a0df9dbe41f9dbc361) )
	ROM_LOAD( "136001-212.hj7", 0x0800, 0x0800, CRC(b1397029) SHA1(974c03d29aeca672fffa4dfc00a06be6a851aacb) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "136001-213.p4",  0x0000, 0x0100, CRC(6fa3093a) SHA1(2b7aeca74c1ae4156bf1878453a047330f96f0a8) )
ROM_END


ROM_START( centipdd ) /* Centipede "Dux" graphics hack by Two-Bit Score in 1989 */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "136001-307.d1",  0x2000, 0x0800, CRC(5ab0d9de) SHA1(8ea6e3304202831aabaf31dbd0f970a7b3bfe421) )
	ROM_LOAD( "136001-308.e1",  0x2800, 0x0800, CRC(4c07fd3e) SHA1(af4fdbf32c23b1864819d620a874e7f205da3cdb) )
	ROM_LOAD( "136001-309.fh1", 0x3000, 0x0800, CRC(ff69b424) SHA1(689fa560d40a384dcbcad7c8095bc12e91875580) )
	ROM_LOAD( "136001-310.j1",  0x3800, 0x0800, CRC(44e40fa4) SHA1(c557db83876afc8ab52047ab1a3c3bfef34d6351) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "dux-211.f7",  0x0000, 0x0800, CRC(fee15594) SHA1(1193c295b57904d1c19c7f0400ef4c1893a80f55) )
	ROM_LOAD( "dux-212.hj7", 0x0800, 0x0800, CRC(f980c777) SHA1(3997a45ed38d7ae68dddf70b37da6e2e0c6a7710) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "136001-213.p4",  0x0000, 0x0100, CRC(6fa3093a) SHA1(2b7aeca74c1ae4156bf1878453a047330f96f0a8) )
ROM_END


ROM_START( centiped2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "136001-207.d1",  0x2000, 0x0800, CRC(b2909e2f) SHA1(90ec90bd1e262861730afd5b113ec8dddd958ed8) )
	ROM_LOAD( "136001-208.e1",  0x2800, 0x0800, CRC(110e04ff) SHA1(4cb481792411b6aefac561744cfbe107aba8bab3) )
	ROM_LOAD( "136001-209.fh1", 0x3000, 0x0800, CRC(cc2edb26) SHA1(b3ea580afa6a1ac44662051fae19c1efc320fcd3) )
	ROM_LOAD( "136001-210.j1",  0x3800, 0x0800, CRC(93999153) SHA1(8788c2b39fc5bfbb147a5e7c26ad360bba8d1063) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "136001-211.f7",  0x0000, 0x0800, CRC(880acfb9) SHA1(6c862352c329776f2f9974a0df9dbe41f9dbc361) )
	ROM_LOAD( "136001-212.hj7", 0x0800, 0x0800, CRC(b1397029) SHA1(974c03d29aeca672fffa4dfc00a06be6a851aacb) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "136001-213.p4",  0x0000, 0x0100, CRC(6fa3093a) SHA1(2b7aeca74c1ae4156bf1878453a047330f96f0a8) )
ROM_END


ROM_START( centiped1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "136001-107.d1",  0x2000, 0x0800, CRC(674ab844) SHA1(b328cc6be05d4531a2dc8abbd7a0ea8f612d6084) )
	ROM_LOAD( "136001-108.e1",  0x2800, 0x0800, CRC(47ceddc9) SHA1(bdce4ed0812d8f73a4935f6302065fd3f2eff8c7) )
	ROM_LOAD( "136001-109.fh1", 0x3000, 0x0800, CRC(7ee80952) SHA1(a913be14f551368480badbe519be658689ec3027) )
	ROM_LOAD( "136001-110.j1",  0x3800, 0x0800, CRC(93b33dce) SHA1(b17f1c2186a7a0b98e07f85b4da9b623c2f3c05b) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "136001-111.f7",  0x0000, 0x0800, CRC(880acfb9) SHA1(6c862352c329776f2f9974a0df9dbe41f9dbc361) )
	ROM_LOAD( "136001-112.hj7", 0x0800, 0x0800, CRC(b1397029) SHA1(974c03d29aeca672fffa4dfc00a06be6a851aacb) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "136001-113.p4",  0x0000, 0x0100, CRC(4cd24c85) SHA1(00f4876279255f3a2d136a9d916b388812cbd1fc) )
ROM_END


ROM_START( centipedj ) // PCB C2-00158 CENTIPEDE, manufactured by Sanritsu but marked Atari
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cen-00.8e",  0x2000, 0x2000, CRC(7f174187) SHA1(879d728cb6b1bc6ef80bbf2f5a18e85b81b0bbac) ) // 2764

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "cen-20.3b",  0x0000, 0x0800, CRC(28acc45f) SHA1(3f8d01181737b3de6dfddf4d481c61f6e9ae8992) ) // 2732, 1xxxxxxxxxxx = 0xFF
	ROM_IGNORE(0x800)
	ROM_LOAD( "cen-21.3c",  0x0800, 0x0800, CRC(064a6b8b) SHA1(50fd0c18b6ba03759f117feaef799c6e310bbeed) ) // 2732, 1xxxxxxxxxxx = 0xFF
	ROM_IGNORE(0x800)

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "cen_60.1h",  0x0000, 0x0100, CRC(6fa3093a) SHA1(2b7aeca74c1ae4156bf1878453a047330f96f0a8) )
ROM_END


ROM_START( caterplr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "olympia.c28",  0x2000, 0x0800, CRC(8a744e57) SHA1(0bc83fe01d929af4e5c7f2a8d1236560df41f9ce) )
	ROM_LOAD( "olympia.c29",  0x2800, 0x0800, CRC(bb897b10) SHA1(bb1039fe64774277870f675eb72dd9f3f596f865) )
	ROM_LOAD( "olympia.c30",  0x3000, 0x0800, CRC(2297c2ac) SHA1(129d111f80b837f7b44852162f4abfba31fc0d75) )
	ROM_LOAD( "olympia.c31",  0x3800, 0x0800, CRC(cc529d6b) SHA1(80d86371b0f969b434af6ffb3834adaf11d05ac2) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "olympia.c32",  0x0000, 0x0800, CRC(d91b9724) SHA1(5ff9ccb2769c853b44764bfe829ad1df08686dc6) )
	ROM_LOAD( "olympia.c33",  0x0800, 0x0800, CRC(c2b08489) SHA1(9427e54537312ee0a70ec7bd1c039e92f8cfadad) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "136001-213.p4", 0x0000, 0x0100, CRC(6fa3093a) SHA1(2b7aeca74c1ae4156bf1878453a047330f96f0a8) )
ROM_END


ROM_START( centipdb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "d1",  0x2000, 0x0800, CRC(b17b8e0b) SHA1(01944cf040cf23aeb4c50d4f2e63181e08a07310) )
	ROM_LOAD( "e1",  0x2800, 0x0800, CRC(7684398e) SHA1(eea8e05506a7af2fec55c2689e3caafc62ea524f) )
	ROM_LOAD( "h1",  0x3000, 0x0800, CRC(74580fe4) SHA1(35b8a8675e4e020e234e51c3e4bd4ee5c24b79d2) )
	ROM_LOAD( "j1",  0x3800, 0x0800, CRC(84600161) SHA1(e9a6801c6f59e2b34e692e9aa71845d2e64a2379) )
	ROM_LOAD( "k1",  0x6000, 0x0800, CRC(f1aa329b) SHA1(e4689de0f94d11f125ee7548a3f8128ff8e8da51) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "136001-211.f7",  0x0000, 0x0800, CRC(880acfb9) SHA1(6c862352c329776f2f9974a0df9dbe41f9dbc361) )
	ROM_LOAD( "136001-212.hj7", 0x0800, 0x0800, CRC(b1397029) SHA1(974c03d29aeca672fffa4dfc00a06be6a851aacb) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "136001-213.p4", 0x0000, 0x0100, CRC(6fa3093a) SHA1(2b7aeca74c1ae4156bf1878453a047330f96f0a8) )
ROM_END

ROM_START( centipdb2 ) // Single PCB with hand-written "Millipede", but is a Centipede bootleg instead
	ROM_REGION( 0x10000, "maincpu", 0 ) // all 2716
	ROM_LOAD( "mp1.d1",  0x2000, 0x0800, CRC(b17b8e0b) SHA1(01944cf040cf23aeb4c50d4f2e63181e08a07310) )
	ROM_LOAD( "mp2.e1",  0x2800, 0x0800, CRC(7684398e) SHA1(eea8e05506a7af2fec55c2689e3caafc62ea524f) )
	ROM_LOAD( "mp3.h1",  0x3000, 0x0800, CRC(74580fe4) SHA1(35b8a8675e4e020e234e51c3e4bd4ee5c24b79d2) )
	ROM_LOAD( "mp4.j1",  0x3800, 0x0800, CRC(849b1614) SHA1(9060e39ec1d5c66e26c8d28a86818bcc1801c610) )
	ROM_LOAD( "mp5.k1",  0x6000, 0x0800, CRC(9d3ad0e5) SHA1(ad7520b3c95d729bfd022553817f868485e9a191) )

	ROM_REGION( 0x1000, "gfx1", 0 ) // all 2716
	ROM_LOAD( "mp6.f7",  0x0000, 0x0800, CRC(880acfb9) SHA1(6c862352c329776f2f9974a0df9dbe41f9dbc361) )
	ROM_LOAD( "mp7.j7",  0x0800, 0x0800, CRC(b1397029) SHA1(974c03d29aeca672fffa4dfc00a06be6a851aacb) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.p4", 0x0000, 0x0100, CRC(6fa3093a) SHA1(2b7aeca74c1ae4156bf1878453a047330f96f0a8) )
ROM_END

ROM_START( millpac )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "millpac1.1d",  0x2000, 0x0800, CRC(4dd6913d) SHA1(9eca634e1a827f9bbcf3c532d44e175ac4751755) )
	ROM_LOAD( "millpac2.1e",  0x2800, 0x0800, CRC(411c81f1) SHA1(15184642522f0b7eab81301295d435c10ce2d78d) )
	ROM_LOAD( "millpac3.1h",  0x3000, 0x0800, CRC(577076cc) SHA1(3124fcfb56f33ebd17d2c0da1098023474187066) )
	ROM_LOAD( "millpac4.1j",  0x3800, 0x0800, CRC(89aedd75) SHA1(74635079e7103bf6fa9577f5980e1adaa34d9be0) )
	ROM_LOAD( "millpac5.1k",  0x6000, 0x0800, CRC(67ac481b) SHA1(cef839d1c9dd207fdf41ae47d5f279b783f2f4cf) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "millpac6.7f",  0x0000, 0x0800, CRC(79f05520) SHA1(25c9f3b4826f48830f5c42d41d4f030b49e58d6a) )
	ROM_LOAD( "millpac7.7j",  0x0800, 0x0800, CRC(4880b2bd) SHA1(1909b8fb275f38f1b57bf53ba348b866cc48a599) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "millpac.4p",   0x0000, 0x0100, CRC(ba40e1a9) SHA1(f7914ba974e5bdd0c24d415a537b5fe567a4de50) ) /* not used */
ROM_END


ROM_START( magworm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "11005-0.k0",  0x2000, 0x0800, CRC(a88e970a) SHA1(f0cc6fdcdecf05f11cef7ebae4e11783a8bbc5ba) )
	ROM_LOAD( "11005-1.k1",  0x2800, 0x0800, CRC(7a04047e) SHA1(3c00756c8ffbc5e78d4a7409802cc2ed8f668264) )
	ROM_LOAD( "11005-2.k2",  0x3000, 0x0800, CRC(f127f1c3) SHA1(3fddcd6f458ac60eaebacef921b522dd2c7b8141) )
	ROM_LOAD( "11005-3p.k3", 0x3800, 0x0800, CRC(478d92b4) SHA1(99cce957c50ca80ddde408d9188fc2ed04d8da68) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "11005-4.c4",  0x0000, 0x0800, CRC(cea64e1a) SHA1(9022102124e1ad93f912ce8bdf85f8a886b0879b) )
	ROM_LOAD( "11005-5.c5",  0x0800, 0x0800, CRC(24558ea5) SHA1(8cd7131e19afd7a96191b1b3c3fba7ae9a140f4b) )
ROM_END


ROM_START( magworma )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "h41.1d",  0x2000, 0x0800, CRC(773a3da6) SHA1(577ae0578231df83a768d6a50b86dcaf715a32d7) )
	ROM_LOAD( "h42.1e",  0x2800, 0x0800, CRC(482c7808) SHA1(6d274a988e603d33131cb6ffbfe2cdd22fabf25b) )
	ROM_LOAD( "h43.1fh", 0x3000, 0x0800, CRC(1c46f769) SHA1(dc25ab3d9abf2180d3ff20f96b69fd6ed3b0e08c) )
	ROM_LOAD( "h44.1j",  0x3800, 0x0800, CRC(19a79e26) SHA1(0485ae6d21ad7c9d04475ad7858c47205830df84) )

	ROM_REGION( 0x1000, "gfx1", 0 ) /* same as magworm */
	ROM_LOAD( "h45.7hj", 0x0000, 0x0800, CRC(cea64e1a) SHA1(9022102124e1ad93f912ce8bdf85f8a886b0879b) )
	ROM_LOAD( "h45.7f",  0x0800, 0x0800, CRC(24558ea5) SHA1(8cd7131e19afd7a96191b1b3c3fba7ae9a140f4b) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "prom.4p", 0x0000, 0x0200, CRC(a5101624) SHA1(36d11e343a78046b99ea1a2daba4908ec60ae61e) ) /* not used */
ROM_END


ROM_START( milliped )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "136013-104.mn1", 0x4000, 0x1000, CRC(40711675) SHA1(b595d6a0f5d3c611ade1b83a94c3b909d2124dc4) )
	ROM_LOAD( "136013-103.l1",  0x5000, 0x1000, CRC(fb01baf2) SHA1(9c1d0bbc20bf25dd21761a311fd1ed80aa029241) )
	ROM_LOAD( "136013-102.jk1", 0x6000, 0x1000, CRC(62e137e0) SHA1(9fe40db55ba1d20d4f11704f7f5df9ff75b87f30) )
	ROM_LOAD( "136013-101.h1",  0x7000, 0x1000, CRC(46752c7d) SHA1(ab06b1fd80271849946f90757b3837b617394929) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "136013-107.r5", 0x0000, 0x0800, CRC(68c3437a) SHA1(4c7ea33d9501456ee8f5a642da7d6c972f2bb90d) )
	ROM_LOAD( "136013-106.p5", 0x0800, 0x0800, CRC(f4468045) SHA1(602fcc7290f9f4eacb841c76665961ebf4307f80) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "136001-213.e7", 0x0000, 0x0100, CRC(6fa3093a) SHA1(2b7aeca74c1ae4156bf1878453a047330f96f0a8) ) // Sync PROM
ROM_END


ROM_START( millipdd ) /* Millipede "Dux" graphics hack by Two-Bit Score in 1989 */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "136013-104.1mn", 0x4000, 0x1000, CRC(40711675) SHA1(b595d6a0f5d3c611ade1b83a94c3b909d2124dc4) )
	ROM_LOAD( "136013-103.1l",  0x5000, 0x1000, CRC(fb01baf2) SHA1(9c1d0bbc20bf25dd21761a311fd1ed80aa029241) )
	ROM_LOAD( "136013-102.1jk", 0x6000, 0x1000, CRC(62e137e0) SHA1(9fe40db55ba1d20d4f11704f7f5df9ff75b87f30) )
	ROM_LOAD( "136013-101.1h",  0x7000, 0x1000, CRC(46752c7d) SHA1(ab06b1fd80271849946f90757b3837b617394929) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "mil-dux.5r", 0x0000, 0x0800, CRC(b6c617b6) SHA1(a672a9b35b773677aea6b9a5c4305939180f6854) )
	ROM_LOAD( "mil-dux.5p", 0x0800, 0x0800, CRC(2a6ef4b0) SHA1(832dae8c1b1f959bb8582f9503d84bea9d50c08c) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "136001-213.7e", 0x0000, 0x0100, CRC(6fa3093a) SHA1(2b7aeca74c1ae4156bf1878453a047330f96f0a8) ) // Sync PROM
ROM_END


ROM_START( multiped )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 ) // banked, decrypted rom goes here

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD("cm_12a.bin", 0x00000, 0x10000, CRC(8248d42c) SHA1(613f449de99659714abdf86d1be4d9d68f624da2) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "cm_5r.bin", 0x0000, 0x2000, CRC(45613f22) SHA1(5d512a286ca9e008bd2f0fdbc2f8cb9c006e3ac7) )
	ROM_LOAD( "cm_5p.bin", 0x2000, 0x2000, CRC(cfbc3622) SHA1(a22da8f9b28effcd90794cb87ebc2afdf3fc4282) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "136001-213.7e", 0x0000, 0x0100, CRC(6fa3093a) SHA1(2b7aeca74c1ae4156bf1878453a047330f96f0a8) ) // Sync PROM
ROM_END


ROM_START( mazeinv )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "136011-005.p1",  0x3000, 0x1000, CRC(37129536) SHA1(356cb986a40b332100e00fb72194fd4dade2cba7) )
	ROM_LOAD( "136011-004.mn1", 0x4000, 0x1000, CRC(2d0fbf2f) SHA1(9d4c2bc9f80604d1ff5c5bf5a4a78378efdd8b33) )
	ROM_LOAD( "136011-003.l1",  0x5000, 0x1000, CRC(0ff3747c) SHA1(1a7e1c487c24875dada967fb3a9ceaca25b7e2a7) )
	ROM_LOAD( "136011-002.k1",  0x6000, 0x1000, CRC(96478e07) SHA1(e99d9970dc12ed36520d91646f1955cce87b55b6) )
	ROM_LOAD( "136011-001.hj1", 0x7000, 0x1000, CRC(d5c29a01) SHA1(5201a6cd42e4954c6cd4298f7b6cee4a8c181248) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "136011-007.hj6", 0x0000, 0x0800, CRC(16e738f4) SHA1(96335afc4510aae6b4ee6dfd8f5c1b2baa8c2798) )
	ROM_LOAD( "136011-006.f6",  0x0800, 0x0800, CRC(d4705e4e) SHA1(e099f3df0d2f56d557631e69bc76ae2f09a80b42) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "136011-009.c11", 0x0000, 0x0020, CRC(dcc48de5) SHA1(5594568dd6a605d6d9d9646b1af645af72a7f53d) ) // Color PROM
	ROM_LOAD( "136011-008.c10", 0x0020, 0x0100, CRC(6fa3093a) SHA1(2b7aeca74c1ae4156bf1878453a047330f96f0a8) ) // Sync PROM
ROM_END


ROM_START( warlords )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "037154-01.m1", 0x5000, 0x0800, CRC(18006c87) SHA1(6b4aab1b1710819d29f4bbc29269eb9c915626c0) )
	ROM_LOAD( "037153-01.k1", 0x5800, 0x0800, CRC(67758f4c) SHA1(b65ca677b54de7a8202838207d9a7bb0aed3e0f2) )
	ROM_LOAD( "037158-01.j1", 0x6000, 0x0800, CRC(1f043a86) SHA1(b1e271c0979d62202ae86c4b6860fb67bbef6400) )
	ROM_LOAD( "037157-01.h1", 0x6800, 0x0800, CRC(1a639100) SHA1(41ec333aee7192f8aeef49e5257f201f4db01cff) )
	ROM_LOAD( "037156-01.e1", 0x7000, 0x0800, CRC(534f34b4) SHA1(1680982ded17350c2ae10bb47f7eb8908bb10db2) )
	ROM_LOAD( "037155-01.d1", 0x7800, 0x0800, CRC(23b94210) SHA1(d74c1ca90caf15942805043b4ebe4ee077799da0) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "037159-01.e6", 0x0000, 0x0800, CRC(ff979a08) SHA1(422053473e41e3e1f71eb28e40eedc78f22326b3) )

	ROM_REGION( 0x0200, "proms", 0 ) // dumped from 3 different PCBs
	/* Only the first 0x80 bytes are used by the hardware. A7 is grounded. */
	/* Bytes 0x00-0x3f are used for the color cocktail version. */
	/* Bytes 0x40-0x7f are for the upright version of the cabinet with a */
	/* mirror and painted background. */
	ROM_LOAD( "037235-01.n7", 0x0000, 0x0100, CRC(a2c5c277) SHA1(f04de9fb6ee9619b4a4aae10c92b16b3123046cf) )
	ROM_LOAD( "037161-01.m6", 0x0100, 0x0100, CRC(6fa3093a) SHA1(2b7aeca74c1ae4156bf1878453a047330f96f0a8) ) // Sync PROM
ROM_END


/*
This is a conversion daughterboard for Atari Centipede PCBs.
The Bullseye Darts PCB is screened (C) 1984 CVS Technology.
PROM and PAL data recovered by mark shostak - 09 Feb 2003.

The PCB utilizes a MAB2650A CPU; SN76496N DAC for sound; Century 109
40-pin custom chip for protection, one 2114 (1024x4) SRAM and
one 8-pin dip that has been rendered unidentifiable.

Two jumper wires have been run to the Centipede PCB. The red wire
runs from the 'audio' designation on the Bullseye Darts PCB (TP.A)
to both C65 and R81. The second jumper wire, blue, is run from
TP.B to the 'negative' side of R12.

The board has two barnicles as follows:
1) 2650 pin 22 to N82SS153 pin 4
2) 2114 pin 10 to N82SS153 pin 14
*/

ROM_START( bullsdrt )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "27128.bin", 0x0000, 0x1000, CRC(2729f585) SHA1(6ffbfa5b62c497c3932ab71d0e3f407cae99cb59) )
	ROM_CONTINUE(          0x2000, 0x1000 )
	ROM_CONTINUE(          0x4000, 0x1000 )
	ROM_CONTINUE(          0x6000, 0x1000 )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "2764_b.bin",   0x0000, 0x2000, CRC(49a19aba) SHA1(77869b7a7aae24dcbc4c7f1a3d4bcd26ea3f4fac) )
	ROM_LOAD( "2764_a.bin",   0x2000, 0x2000, CRC(361ff09d) SHA1(4b57417085bc9ee174196ca638dc7e6d3626f801) )

	ROM_REGION( 0x0200, "proms", 0 ) /* unknown */
	ROM_LOAD( "82s147.bin",   0x0000, 0x0200, CRC(d841b7e0) SHA1(aab32645a613cd027aed98437db24704763cc147) )

	ROM_REGION( 0xeb, "plds", 0 )
	ROM_LOAD( "82s153.bin",   0x00, 0xeb, CRC(ff7e0ced) SHA1(b11dcb1937e01c23a3fad1d8c3cf0d969a80a122) )
ROM_END



/*************************************
 *
 *  Driver initialization
 *
 *************************************/

void centiped_state::init_bullsdrt()
{
	m_dsw_select = 0;
}


void multiped_state::init_multiped()
{
	uint8_t *src = memregion("user1")->base();
	uint8_t *dest = memregion("maincpu")->base();

	// descramble rom and put in maincpu region
	for (int i = 0; i < 0x10000; i++)
		dest[(i ^ (~i << 4 & 0x1000) ^ (~i >> 3 & 0x400))] = bitswap<8>(src[bitswap<16>(i, 15, 14, 13, 1, 8, 11, 4, 7, 10, 5, 6, 9, 12, 0, 3, 2)], 0, 2, 1, 3, 4, 5, 6, 7);

	m_rombank->configure_entries(0, 2, dest, 0x8000);
}



/*************************************
 *
 *  Game drivers
 *
 *************************************/

// Centipede, Millipede, and clones
GAME( 1980, centiped,  0,        centiped,  centiped4, centiped_state, empty_init,    ROT270, "Atari", "Centipede (revision 4)", MACHINE_SUPPORTS_SAVE ) /* 1 Player Only with Timer Options */
GAME( 1980, centiped3, centiped, centiped,  centiped,  centiped_state, empty_init,    ROT270, "Atari", "Centipede (revision 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, centiped2, centiped, centiped,  centiped,  centiped_state, empty_init,    ROT270, "Atari", "Centipede (revision 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, centiped1, centiped, centiped,  centiped,  centiped_state, empty_init,    ROT270, "Atari", "Centipede (revision 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, centipedj, centiped, centipedj, centipedj, centiped_state, empty_init,    ROT270, "Atari", "Centipede (Japan, revision 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, centipdb,  centiped, centipdb,  centiped,  centiped_state, empty_init,    ROT270, "bootleg", "Centipede (bootleg, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, centipdb2, centiped, centipdb,  centiped,  centiped_state, empty_init,    ROT270, "bootleg", "Centipede (bootleg, set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, centipdd,  centiped, centiped,  centiped,  centiped_state, empty_init,    ROT270, "hack (Two-Bit Score)", "Centipede Dux (hack)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, caterplr,  centiped, caterplr,  caterplr,  centiped_state, empty_init,    ROT270, "bootleg (Olympia)", "Caterpillar (bootleg of Centipede)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, millpac,   centiped, centipdb,  centiped,  centiped_state, empty_init,    ROT270, "bootleg? (Valadon Automation)", "Millpac (bootleg of Centipede)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, magworm,   centiped, magworm,   magworm,   centiped_state, empty_init,    ROT270, "bootleg (Sidam)", "Magic Worm (bootleg of Centipede, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, magworma,  centiped, magworm,   magworm,   centiped_state, empty_init,    ROT270, "bootleg", "Magic Worm (bootleg of Centipede, set 2)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 1982, milliped,  0,        milliped,  milliped,  centiped_state, empty_init,    ROT270, "Atari", "Millipede", MACHINE_SUPPORTS_SAVE )
GAME( 1989, millipdd,  milliped, milliped,  milliped,  centiped_state, empty_init,    ROT270, "hack (Two-Bit Score)", "Millipede Dux (hack)", MACHINE_SUPPORTS_SAVE )
GAME( 2002, multiped,  0,        multiped,  multiped,  multiped_state, init_multiped, ROT270, "hack (Braze Technologies)", "Multipede (Centipede/Millipede multigame kit)", MACHINE_SUPPORTS_SAVE )

// other Atari games
GAME( 1980, warlords,  0,        warlords,  warlords,  centiped_state, empty_init,    ROT0,   "Atari", "Warlords", MACHINE_SUPPORTS_SAVE )
GAME( 1981, mazeinv,   0,        mazeinv,   mazeinv,   centiped_state, empty_init,    ROT270, "Atari", "Maze Invaders (prototype)", 0 )

// other manufacturers
GAME( 1985, bullsdrt,  0,        bullsdrt,  bullsdrt,  centiped_state, init_bullsdrt, ROT270, "Shinkai Inc. (Magic Electronics Inc. license)", "Bulls Eye Darts", MACHINE_WRONG_COLORS | MACHINE_SUPPORTS_SAVE )
