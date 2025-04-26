// license:BSD-3-Clause
// copyright-holders: Ernesto Corvi, Roberto Fresca, Grull Osgo

/****************************************************************************************************************

  Truco-Tron - (c) 198? Playtronic SRL, Argentina.

  Written by Ernesto Corvi, Roberto Fresca, and Grull Osgo.


*****************************************************************************************************************

  This appears to be the first arcade game fully designed in Argentina. The hardware, which is based on a M6809E CPU,
  is unlike anything previously known.

  The game is based on Truco, a traditional and highly strategic Argentine card game. It's known for being difficult
  to play, as the objective is to earn points by playing cards while bluffing about the hand you hold.

  For more about the rules of Truco, see the following links:
  https://www.nhfournier.es/en/como-jugar/truco/
  https://www.wikihow.com/Play-Truco


  Truco-tron focuses on the "1 player vs machine" mode (mano a mano), offering a unique arcade experience.

  All game and technical information, including the emulation and IC's recognition, was obtained
  by reverse-engineering both the hardware and game code.

  The game has various settings hardcoded into non-volatile RAM, which are inaccessible to the user.
  The entire RAM system is powered by a 3.6V lithium battery, which is monitored by a Maxim MAX691 device.
  If power is lost or the battery drains, the game becomes inoperable and requires a fresh RAM image to function again.

  If the RAM contents are missing, a yellow password entry screen appears, but it serves no purpose. It’s believed that
  the original design intended to show a repair screen, as the necessary values are hardcoded and ready to be injected.
  However, the game code attempts to inject incorrect values, which results in garbage data and prevents the game from
  booting properly.

  In normal arcade mode, defeating the machine rewards the player with a free game.

  Though marketed as an arcade game, Truco-tron also contains a hidden gambling mode that is inaccessible by default.
  Enabling this mode requires substantial modifications, such as adding extra inputs and altering the PIA 6821 inputs
  mask that hides them. Once activated, a supervisor key appears, providing two new inputs for adding and removing credits,
  functioning as key-in and key-out.

  The gambling mode features a selectable risk level, allowing players to multiply their winnings but at the cost of
  increasing the amount of credits at risk.

  Another hidden mode, likely used during development for debugging, enables the player to set their cards in the game,
  allowing for a more controlled test environment.


  How to Activate Gambling Mode
  -----------------------------

  Go to the machine's configuration menu and select the gambling game mode.
  Introduce one credit, and then press the button to enter the credits screen.
  Turn the supervisor key (hold the SERVICE button), and you can add credits using the standard coin mechanism
  or the key-in button (Key Q). Use the joystick and game button to select the desired risk level.
  To collect credits, use the TILT button as key-out (Key T).

  You can also press the BOOKS button to view the in/out counters on the credits screen.
  There are jumpers, but they are just holes instead of actual jumpers. It seems they never anticipated a real
  implementation. The effects and settings are visible below.


  Easter Egg
  ----------

  Hidden within the game is an encrypted message, which appears after a specific combination of inputs is entered.
  This can only be triggered when you have 8 credits in gambling mode and have lied about Flor.

  The hidden message reads:
  "ESTE PROGRAMA DE JUEGO DE TRUCO PERTENECE A LOS PROPIETARIOS DE LA MARCA LANZA PERFUME"

  Reaching this message is incredibly complex, and if the wrong action is taken, a small red dot will appear,
  causing the game to "suicide" itself. This will require the RAM contents to be reloaded in order for the game
  to work again.


  Additionally, the game features an unfinished alternate graphics mode with clipped strings, which can be switched
  on using one of the jumpers.

  It is believed that only 300 of these boards were ever produced.


*****************************************************************************************************************

  Mini-board (6"x 7") silkscreened 8901 REV.C
  JAMMA connector.

  1x Xtal 12 MHz.
  1x 3.6V. Lithium Battery (QTC85).

  2 rows of 6 holes for jumpers (JP1, JP2).
  No DIP switches banks.

  All IC's are scratched to avoid the identification.


  PCB layout:
  .---------------------------------------------------.
  |S T  .---. .---. .---. .-.    .-.     .---. .-.    |
  |E R  |   | | U | | U | |U|    |U|     | U | |U|    |
  |R U  |   | | 2 | | 3 | |1|    |5|     | 1 | |1|    |
  |I C  | U | |   | |   | |5|    | |     | 0 | |4|    |
  |E O  | 1 | |   | |   | '-'    '-'     |   | '-'    |
  |     |   | '---' '---'        .-.     '---' .-.    |
  |0 T  |   |                    |U| .-------. |U|    |
  |0 R  |   |                    |1| |BATTERY| |1|---.|
  |0 O  |   |                    |2| | -   + | |9|R12||
  |0 N  '---'                    '-' '-------' '-'---'|
  | .-. .-,   .---. .-.   .---.  .-.   .-.     .-.    |
  | |U| |U|   |   | |U|   |   |  |U|   |U|     |U|    |
  | |1| |1|   |   | |1|   |   |  |6|   |7|     |1|    |
  | |7| |8|   | U | |6|   | U |  | |   | |     |3|   P|
  | '-' '-'   | 4 | '-'   | 9 |  '-'   '-'     | |   L|
  | .----.    |   |       |   |  .-.   .-.     '-'   A|
  | |Xtal|    |   |       |   |  |U|   |U|           Y|
  | '----'    |   |       |   |  |1|   |8|           T|
  | .-.       |   |       |   |  |1|   | |           R|
  | |U|       '---'       '---'  '-'   '-'           O|
  | |2|.---.                                         N|
  | |0||R17|                                         I|
  | '-''---'          JAMMA                          C|
  '----+++++++++++++++++++++++++++++------------------'
       |||||||||||||||||||||||||||||
       '---------------------------'


  IC's Reverse Engineering....

  MARKED   PINS     ID    TYPE        PART                           DETAILS

  - U1 : 40-pin IC  YES   CPU         MOTOROLA M6809EP               8-bit microprocessor.
  - U2 : 28-pin IC  YES   ROM         M27128A (or M27512FI)          NMOS 128K 16K x 8 UV EPROM (or 64K x 8).
  - U3 : 28-pin IC  YES   ROM         M27128A (or M27512FI)          NMOS 128K 16K x 8 UV EPROM (or 64K x 8).
  - U4 : 40-pin IC  YES   I/O         MC6821P                        PIA: Peripheral Interface Adapter.
  - U5 : 16-pin IC  YES   TTL         74HC157                        Quad 2 Channel Multiplexer.
  - U6 : 16-pin IC  YES   TTL         74HC157                        Quad 2 Channel Multiplexer.
  - U7 : 16-pin IC  YES   TTL         74HC157                        Quad 2 Channel Multiplexer.
  - U8 : 16-pin IC  YES   TTL         74HC157                        Quad 2 Channel Multiplexer.
  - U9 : 40-pin IC  YES   CRTC        HD6845 / UM6845 / GM68A45S     CRT Controller.
  - U10: 28-pin IC  YES   RAM         62256-10                       32K x 8 Low Power CMOS Static RAM.
  - U11: 14-pin IC  YES   TTL         74LS95                         4-bit Parallel-Access Shift Registers.
  - U12: 14-pin IC  YES   TTL         74LS95                         4-bit Parallel-Access Shift Registers.
  - U13: 20-pin IC  YES   TTL         74LS244                        Octal Buffers and Line Drivers with 3-State output.
  - U14: 20-pin IC  YES   TTL         74LS374                        Octal D-type Flip-Flops with noninverted 3-state output.
  - U15: 20-pin IC  YES   PLD         PALCE16V8H-25                  EE CMOS Zero-Power 20-Pin Universal Programmable Array Logic.
  - U16: 20-pin IC  YES   PLD         PALCE16V8H-25                  EE CMOS Zero-Power 20-Pin Universal Programmable Array Logic.
  - U17: 14-pin IC  YES   TTL         74LS00                         Quad 2-Input NAND Gates.
  - U18: 14-pin IC  YES   TTL         74HCTLS86                      Quad 2-Input XOR Gates.
  - U19: 16-pin IC  YES   WATCHDOG    MAXIM MAX691                   Microprocessor Supervisory Circuits.
  - U20: 16-pin IC  YES   DARLINGTON  ULN2003                        7 NPN Darlington transistor pairs with high voltage and current capability.

  - R12: 1K pot. Connected through legs 2 & 3 to MAX691 pin 9 (PFI), and a pull-up.
  - R17: 100K pot. Connected to ULN2003 (pin 10), and then to audio out on the edge connector.


                             M6809E
                           .---\/---.
                  VSS/GND 1|        |40 /HALT <--
            (*) --> /NMI  2|        |39 TSC   <--
  PIA /IRQA & B --> /IRQ  3|        |38 LIC   <--
                --> /FIRQ 4|        |37 /RES  <-- PIA /RES & U19(15) MAX691
                <-- BS    5|        |36 AVMA  <--
                <-- BA    6|        |35 Q     <--
                    Vcc   7|   U1   |34 E     <-- PIA E (25)
  PIA /RS0 (36) <-- A0    8|        |33 BUSY  <--
  PIA /RS1 (35) <-- A1    9|Motorola|32 R/!W  --> PIA R/W (21)
                <-- A2   10| 6809E  |31 D0    <-> PIA D0 (33)
                <-- A3   11|        |30 D1    <-> PIA D1 (32)
                <-- A4   12|        |29 D2    <-> PIA D2 (31)
                <-- A5   13|        |28 D3    <-> PIA D3 (30)
                <-- A6   14|        |27 D4    <-> PIA D4 (29)
                <-- A7   15|        |26 D5    <-> PIA D5 (28)
                <-- A8   16|        |25 D6    <-> PIA D6 (27)
                <-- A9   17|        |24 D7    <-> PIA D7 (26)
                <-- A10  18|        |23 A15   -->
                <-- A11  19|        |22 A14   -->
                <-- A12  20|        |21 A13   -->
                           '--------'

    (*) /NMI is connected to U19 pin 10 (MAX691 /PFO line),
         and then through a decoupler to pull-up.


                            PIA 6821
                          .----\/----.
                  VSS/GND |01      40| CA1 --- PIA CB1 (*)
  JAMMA S17 (2P_ST) - PA0 |02      39| CA2 --- U19(11). Watchdog/RESET
  JAMMA S14 (SRVSW) - PA1 |03      38| /IRQA - CPU M6809 !IRQ (03)
  JAMMA C26 (2P_SL) - PA2 |04      37| /IRQB - CPU M6809 !IRQ (03)
  JAMMA S16 (COIN2) - PA3 |05  U4  36| /RS0 -- CPU M6809 A0 (08)
  JAMMA S15 (TLTSW) - PA4 |06      35| /RS1 -- CPU M6809 A1 (09)
  JAMMA C22 (P1_B1) - PA5 |07      34| /RES -- CPU M6809 !RES (37) & U19(15) MAX691
  JAMMA C18/21(U-R) - PA6 |08      33| D0 ---- CPU M6809 D0 (31)
  JAMMA C19/20(D-L) - PA7 |09      32| D1 ---- CPU M6809 D1 (30)
             JP2(4) - PB0 |10      31| D2 ---- CPU M6809 D2 (29)
             JP2(2) - PB1 |11      30| D3 ---- CPU M6809 D3 (28)
            U20(04) - PB2 |12      29| D4 ---- CPU M6809 D4 (27)
            U20(05) - PB3 |13      28| D5 ---- CPU M6809 D5 (26)
             JP1(6) - PB4 |14      27| D6 ---- CPU M6809 D6 (25)
             JP1(4) - PB5 |15      26| D7 ---- CPU M6809 D7 (24)
             JP1(2) - PB6 |16      25| E ----- CPU M6809 E (34)
            U20(07) - PB7 |17      24| CS1 --- VCC (bridge with pin 20)
        PIA CA1 (*) - CB1 |18      23| /CS2 -- PALCE16V8H (U16, pin 13)
            U20(06) - CB2 |19      22| CS0 --- VCC (bridge with pin 20)
                      VCC |20      21| R/W
                          '----------'

    (*) Lines CA1 and CB1 are tied together, being both IN.
        They are connected to JAMMA C16 (COIN1).


  U19:   *** MAX691 ***  Maxim MAX691 Microprocessor Supervisory Circuit.
                         (for battery backup power switching and watchdog).

                            MAX691
                          .---\/---.
    BATT+ <----- [VBATT]--|01    16|--[RESET] ---->
          <------ [VOUT]--|02    15|--[/RESET] ---> CPU M6809 /RES (37) & CA2 (39); PIA /RES (34)
      VCC <------- [VCC]--|03    14|--[/WDO] ----->
      GND <------- [GND]--|04    13|--[/CE IN] ---> GND
          <--- [BATT ON]--|05    12|--[/CE OUT] -->
          <-- [/LOWLINE]--|06    11|--[WDI] ------> PIA CA2
  (*) N/C <---- [OSC IN]--|07    10|--[/PFO] -----> CPU M6809 /NMI (2) --> decoupling --> pullup (**)
  (*) N/C <--- [OSC SEL]--|08    09|--[PFI] ------> R12 pot legs 2 & 3
                          '--------'

  (*)  Set 1.6 seconds as WD timeout.
  (**) After a power failure, the MAX691 attacks the 6809E /NMI line through the /PFO line.
       then the NMI routine put a register in RAM with the error, and halt the system.


  U20:   *** ULN2003 ***  High-voltage, high-current Darlington transistor array.

                   ULN2003
                  .---\/---.
           N/C <--|01    16|--> N/C
           N/C <--|02    15|--> N/C
           N/C <--|03    14|--> N/C
  PIA PB2 (12) <--|04    13|--> JAMMA(S08) Coin Counter 2
  PIA PB3 (13) <--|05    12|--> JAMMA(C08) Coin Counter 1
  PIA CB2 (19) <--|06    11|--> JAMMA(S26)
  PIA PB7 (17) <--|07    10|--> C1 (2.2 uF) --> R17 (100K pot) --> JAMMA(S10) +Speaker
           GND <--|08    09|--> VCC
                  '--------'


  Jumpers and hardcoded switches...

  There are two jumpers banks.
  No pins to connect. The PCB has only the holes.

  JP1:
  01 --> GND
  02 --> PIA PB6 (16) & R5(1K) --> VCC
  03 --> GND
  04 --> PIA PB5 (15) & R7(1K) --> VCC
  05 --> GND
  06 --> PIA PB4 (14) & R8(1K) --> VCC

  JP2:
  01 --> GND
  02 --> PIA PB1 (11) & R9(1K) --> VCC
  03 --> GND
  04 --> PIA PB0 (10) & R10(1K) -> VCC
  05 --> GND
  06 --> Seems N/C


                                 JP1     JP2
               (+5V)   PIA(PB6)
                 |        |       O--GND--O
       .---R22---+---R5---+-------O       O---R9---PIA(PB1)
       |
    PIA(/RES)        PIA(PB5)
                        |         O--GND--O
      R22---(+5V)---R7--+---------O       O---PIA(PB0)---R10--(+5V)


                                  O--GND--O
  (GND)---R22---R8---PIA(PB4)-----O       O


  R5  =  1K
  R7  =  1K
  R8  =  1K
  R9  =  1K
  R10 =  1K
  R22 = 10K



  Game harcoded switches:

  9Eh:  1 = No timeout   /  0 = Game Timeout
  ACh:  1 = Gamble Mode  /  2 = Arcade Mode
  88h:  0 = Normal       /  1 = Choose Cards
  ABh:  0 = Input P0 No Masked / 1 = Masked (Default): Enables Service/Books key that show the counters.


*****************************************************************************************************************

  TODO:

  - Find the function of the P1-4 input line.
  - Find the function of the JP2-2 from PIA PB0


*****************************************************************************************************************/


#include "emu.h"

#include "cpu/m6809/m6809.h"
#include "machine/6821pia.h"
#include "machine/watchdog.h"
#include "sound/dac.h"
#include "video/mc6845.h"
#include "machine/nvram.h"
#include "machine/ram.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class truco_state : public driver_device
{
public:
	truco_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_watchdog(*this, "watchdog"),
		m_palette(*this, "palette"),
		m_dac(*this, "dac"),
		m_videoram(*this, "videoram"),
		m_nvram(*this, "nvram"),
		m_ram(*this, RAM_TAG),
		m_coin(*this, "COIN"),
		m_settings(*this, "SETTINGS")
	{ }

	void truco(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<palette_device> m_palette;
	required_device<dac_bit_interface> m_dac;
	required_shared_ptr<uint8_t> m_videoram;
	required_device<nvram_device> m_nvram;
	required_device<ram_device> m_ram;

	required_ioport m_coin;
	required_ioport m_settings;

	uint8_t m_trigger = 0;

	void pia_ca2_w(int state);
	void portb_w(uint8_t data);
	uint8_t pia_ca1_r();
	uint8_t pia_cb1_r();

	void palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
};


/*******************************************
*             Video Hardware               *
*******************************************/

void truco_state::palette(palette_device &palette) const
{
	for (int i = 0; i < palette.entries(); i++)
	{
		int r = (i & 0x8) ? 0xff : 0x00;
		int g = (i & 0x4) ? 0xff : 0x00;
		int b = (i & 0x2) ? 0xff : 0x00;

		int const dim = (i & 0x1);

		if (dim)
		{
			r >>= 1;
			g >>= 1;
			b >>= 1;
		}

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

uint32_t truco_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint8_t const *videoram = m_videoram;

	for (int y = 0; y < 192; y++)
	{
		for (int x = 0; x < 256; x++)
		{
			int const pixel = (videoram[x >> 1] >> ((x & 1) ? 0 : 4)) & 0x0f;

			bitmap.pix(y, x) = m_palette->pen(pixel);
		}

		videoram += 0x80;
	}
	return 0;
}


/*******************************************
*           Read/Write Handlers            *
*******************************************/

uint8_t truco_state::pia_ca1_r()
{
	return m_coin->read() & 1;
}

uint8_t truco_state::pia_cb1_r()
{
	return m_coin->read() & 1;
}

void truco_state::pia_ca2_w(int state)
{
/*  PIA CA2 line is connected to IC U19, leg 11.
    The IC was successfully identified as MAX691.
    The leg 11 is WDI...

    The code toggles 0's & 1's on this line.
    Legs 07 [OSC IN] and 08 [OSC SEL] aren't connected,
    setting 1.6 seconds as WD timeout.
*/
	m_watchdog->watchdog_reset();


/*  Game harcoded switches:

    9Eh:  1 = No timeout   /  0 = Game Timeout
    ACh:  1 = Gamble Mode  /  2 = Arcade Mode
    88h:  0 = Normal       /  1 = Choose Cards
    ABh:  0 = Input P0 No Masked / 1 = Masked (Default)
*/
	m_ram->write(0x88, BIT(m_settings->read(), 1) & 1);
	m_ram->write(0x9e, BIT(m_settings->read(), 2) & 1);
	m_ram->write(0xac, BIT(m_settings->read(), 0) ? 2 : 1);
	m_ram->write(0xab, BIT(m_settings->read(), 0) ? 0xfa : 0xff);  // Enable counters switch via Service Key
}

void truco_state::portb_w(uint8_t data)
{
	m_dac->write(BIT(data, 7)); // Isolated the bit for Delta-Sigma DAC

	machine().bookkeeping().coin_counter_w(0, BIT(data, 3));  // coin in
	machine().bookkeeping().coin_counter_w(1, BIT(data, 2));  // coin out
}


/*******************************************
*                Memory Map                *
*******************************************/

void truco_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rw(m_ram, FUNC(ram_device::read), FUNC(ram_device::write));  // Battery backed RAM
	map(0x1800, 0x7bff).ram().share(m_videoram);
	map(0x8000, 0x8003).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x8004, 0x8004).w("crtc", FUNC(mc6845_device::address_w));
	map(0x8005, 0x8005).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x8008, 0xffff).rom();
}
/*
  CRTC MC6845 initialization routine at $a506 only set the first 14 registers (data at $a4e2)

  Register:   00    01    02    03    04    05    06    07    08    09    10    11    12    13
  Value:     0x5f  0x40  0x4d  0x06  0x0f  0x04  0x0c  0x0e  0x00  0x0f  0x00  0x00  0x00  0xc0

*/


/*******************************************
*         Input Ports Definition           *
*******************************************/

static INPUT_PORTS_START( truco )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE )  PORT_NAME("Keyout enable Key")         // Connected to JAMMA S17 (P2 START)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )                                            // Connected to JAMMA S14 (SERVICE SW)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )         PORT_NAME("P1-4")PORT_CODE(KEYCODE_D)  // still not clear... Connected to JAMMA C26 (P2 SELECT)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN)                                            // Connected to JAMMA S16 (COIN2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_TILT )            PORT_NAME("Tilt / Keyout")             // 'tilt' line. once turned the key behaves as keyout.
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )                                                // Connected to JAMMA C22 (P1 BUTTON1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )                                            // Connected to JAMMA C18/21 (JOY UP & JOY RIGHT)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )                                          // Connected to JAMMA C19/20 (JOY DOWN & JOY LEFT)

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )


	PORT_START("JUMPERS") // JP1-2

	PORT_DIPNAME( 0x01, 0x01, "JP2-2 - Unknown" )  PORT_DIPLOCATION("JP2:2")  // JP2-2 (PB0)
	PORT_DIPSETTING (   0x01, DEF_STR( Off ) )
	PORT_DIPSETTING (   0x00, DEF_STR( On ) )

	PORT_DIPNAME( 0x02, 0x02, "Graphics" )         PORT_DIPLOCATION("JP2:1")  // JP2-1 (PB1)
	PORT_DIPSETTING (   0x02, "Normal" )
	PORT_DIPSETTING (   0x00, "Alt GFX" )

//  The following jumper was designed to switch between arcade and gambling modes
//  but the code was NOP'ed to avoid this option.
	PORT_DIPNAME( 0x10, 0x10, "JP1-3 - Unused" )   PORT_DIPLOCATION("JP1:3")  // JP1-3 (PB4)
	PORT_DIPSETTING (   0x10, DEF_STR( Off ) )
	PORT_DIPSETTING (   0x00, DEF_STR( On ) )

	PORT_DIPNAME( 0x60, 0x60, "Odds" )             PORT_DIPLOCATION("JP1:1,2")  // JP1-1&2 (PB5 & PB6)
	PORT_DIPSETTING (   0x00, "100%" )
	PORT_DIPSETTING (   0x20, " 80%" )
	PORT_DIPSETTING (   0x40, " 75%" )
	PORT_DIPSETTING (   0x60, " 70%" )
	PORT_BIT( 0x8c, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SETTINGS")  // The hidden functions and modes...
	PORT_CONFNAME( 0x01, 0x01, "Game Mode" )
	PORT_CONFSETTING(    0x01, "Arcade" )
	PORT_CONFSETTING(    0x00, "Gamble" )
	PORT_CONFNAME( 0x02, 0x00, "Draw Cards Mode" )
	PORT_CONFSETTING (   0x02, "Manual Set" )
	PORT_CONFSETTING (   0x00, "Normal" )
	PORT_CONFNAME( 0x04, 0x00, "Game Timer" )
	PORT_CONFSETTING (   0x04, "Disabled" )
	PORT_CONFSETTING (   0x00, "Normal" )

INPUT_PORTS_END


/*******************************************
*               Machine Start              *
*******************************************/

void truco_state::machine_start()
{
	save_item(NAME(m_trigger));
	m_nvram->set_base(m_ram->pointer(), m_ram->size());
}


/*******************************************
*              Machine Config              *
*******************************************/

void truco_state::truco(machine_config &config)
{
	constexpr XTAL MASTER_CLOCK = XTAL(12'000'000); // confirmed
	constexpr XTAL CPU_CLOCK = MASTER_CLOCK / 16;   // confirmed
	constexpr XTAL CRTC_CLOCK = MASTER_CLOCK / 8;   // confirmed

	// basic machine hardware
	MC6809E(config, m_maincpu, CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &truco_state::main_map);

	WATCHDOG_TIMER(config, m_watchdog).set_time(attotime::from_msec(1600));  // 1.6 seconds

	RAM(config, m_ram).set_default_size("32K");
	NVRAM(config, m_nvram, nvram_device::DEFAULT_ALL_0);

	pia6821_device &pia(PIA6821(config, "pia0"));
	pia.readpa_handler().set_ioport("P1");
	pia.readpb_handler().set_ioport("JUMPERS");
	pia.writepb_handler().set(FUNC(truco_state::portb_w));
	pia.ca2_handler().set(FUNC(truco_state::pia_ca2_w));
	pia.readca1_handler().set(FUNC(truco_state::pia_ca1_r));
	pia.readcb1_handler().set(FUNC(truco_state::pia_cb1_r));
	pia.irqa_handler().set_inputline(m_maincpu, M6809_IRQ_LINE);
	pia.irqb_handler().set_inputline(m_maincpu, M6809_IRQ_LINE);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(256, 192);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(truco_state::screen_update));

	PALETTE(config, "palette", FUNC(truco_state::palette), 16);

	mc6845_device &crtc(MC6845(config, "crtc", CRTC_CLOCK));    // identified as UM6845
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(4);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.4);
}


/*******************************************
*                 ROM Load                 *
*******************************************/

ROM_START( truco )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "truco.u3",   0x08000, 0x4000, CRC(4642fb96) SHA1(e821f6fd582b141a5ca2d5bd53f817697048fb81) )
	ROM_LOAD( "truco.u2",   0x0c000, 0x4000, CRC(ff355750) SHA1(1538f20b1919928ffca439e4046a104ddfbc756c) )

	ROM_REGION( 0x8000, "nvram", 0 )  // default NVRAM, otherwise the game doesn't boot
	ROM_LOAD( "truco_nvram.bin", 0x0000, 0x8000, CRC(70dba1e7) SHA1(0a0b2f2a59b7167ffefe1ef4214ca1794d442e34) )
ROM_END

} // anonymous namespace


/*********************************************
*                Game Drivers                *
*********************************************/

//    YEAR  NAME     PARENT  MACHINE  INPUT    STATE        INIT        ROT    COMPANY           FULLNAME     FLAGS
GAME( 198?, truco,   0,      truco,   truco,   truco_state, empty_init, ROT0, "Playtronic SRL", "Truco-Tron", MACHINE_SUPPORTS_SAVE )
