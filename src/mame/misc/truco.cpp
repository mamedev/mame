// license:BSD-3-Clause
// copyright-holders: Ernesto Corvi, Roberto Fresca

/******************************************************************************************************

  Truco-Tron - (c) 198? Playtronic SRL, Argentina.

  Written by Ernesto Corvi
  Additional work by Roberto Fresca.


  Notes:

  - The board uses a battery backed ram for protection, mapped at $7c00-$7fff.
  - If the battery backup data is corrupt, it comes up with some sort of code entry screen.
  - The game stores the current credits count in the battery backed ram.
  - System clock is 12 Mhz. The CPU clock is unknown.
  - The Alternate GFX mode is funky. Not only it has different bitmaps, but also the strings with the
    game options are truncated. Title is also truncated.
  - At least one bootleg board exist.

*******************************************************************************************************

  Mini-board (6"x 7") silkscreened 8901 REV.C
  JAMMA connector.

  1x Xtal 12 MHz.
  1x 3.6V. Lithium Battery (QTC85).

  2 rows of 6 holes for jumpers (JP1, JP2).
  No DIP switches banks.

  All IC's are scratched to avoid the identification.


  PCB layout:
  .--------------------------------------------------.
  |S T  .---. .---. .---. .-.    .-.     .---. .-.   |
  |E R  |   | | U | | U | |U|    |U|     | U | |U|   |
  |R U  |   | | 2 | | 3 | |1|    |5|     | 1 | |1|   |
  |I C  | U | |   | |   | |5|    | |     | 0 | |4|   |
  |E O  | 1 | |   | |   | '-'    '-'     |   | '-'   |
  |     |   | '---' '---'        .-.     '---' .-.   |
  |0 T  |   |                    |U| .-------. |U|   |
  |0 R  |   |                    |1| |BATTERY| |1|   |
  |0 O  |   |                    |2| | -   + | |9|   |
  |0 N  '---'                    '-' '-------' '-'   |
  | .-. .-,   .---. .-.   .---.  .-.   .-.     .-.   |
  | |U| |U|   |   | |U|   |   |  |U|   |U|     |U|   |
  | |1| |1|   |   | |1|   |   |  |6|   |7|     |1|   |
  | |7| |8|   | U | |6|   | U |  | |   | |     |3|  P|
  | '-' '-'   | 4 | '-'   | 9 |  '-'   '-'     | |  L|
  | .----.    |   |       |   |  .-.   .-.     '-'  A|
  | |Xtal|    |   |       |   |  |U|   |U|          Y|
  | '----'    |   |       |   |  |1|   |8|          T|
  | .-.       |   |       |   |  |1|   | |          R|
  | |U|       '---'       '---'  '-'   '-'          O|
  | |2|.---.                                        N|
  | |0||pot|                                        I|
  | '-''---'          JAMMA                         C|
  '----+++++++++++++++++++++++++++++-----------------'
       |||||||||||||||||||||||||||||
       '---------------------------'


  IC's Reverse Engineering....

  MARKED   PINS     ID    TYPE        PART                           DETAILS

  - U1 : 40-pin IC  YES   CPU         MOTOROLA M6809EP               8-bit microprocessor.
  - U2 : 28-pin IC  YES   ROM         M27128A (or M27512FI)          NMOS 128K 16K x 8 UV EPROM (or 64K x 8).
  - U3 : 28-pin IC  YES   ROM         M27128A (or M27512FI)          NMOS 128K 16K x 8 UV EPROM (or 64K x 8).
  - U4 : 40-pin IC  YES   I/O         EF6821P                        PIA: Peripheral Interface Adapter.
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


                             M6809
                           .---\/---.
                    GND   1|        |40 !HALT <--
                --> !NMI  2|        |39 ETAL  <--
  PIA /IRQA & B --> !IRQ  3|        |38 EXTAL <--
                --> !FIRQ 4|        |37 !RES  <-- PIA /RES & U19(15) MAX691
                <-- BS    5|        |36 MRDY  <--
                <-- BA    6|        |35 Q     <--
                    Vcc   7|   U1   |34 E     <-- PIA E (25)
  PIA /RS0 (36) <-- A0    8|        |33 !DMA  <--
  PIA /RS1 (35) <-- A1    9|Motorola|32 R/!W  --> PIA R/W (21)
                <-- A2   10|  6809  |31 D0    <-> PIA D0 (33)
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

                            PIA 6821
                          .----\/----.
                      VSS |01      40| CA1 --- PIA CB1 (*)
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
            U20(07) - PB7 |17      24| CS1
        PIA CA1 (*) - CB1 |18      23| /CS2
            U20(06) - CB2 |19      22| CS0
                      VCC |20      21| R/W
                          '----------'

    (*) Lines CA1 and CB1 are tied together, being both IN.
        They are connected to JAMMA C16 (COIN1).


  U19:   *** MAX691 ***  Maxim MAX691 Microprocessor Supervisory Circuit.
                         (for battery backup power switching and watchdog).

                          MAX691
                        .---\/---.
        <----- [VBATT]--|01    16|--[PFI] ------>
        <------ [VOUT]--|02    15|--[/PFO] ----->
    VCC <------- [VCC]--|03    14|--[WDI] ------> PIA CA2
    GND <------- [GND]--|04    13|--[/CE OUT] -->
        <--- [BATT ON]--|05    12|--[/CE IN] ---> GND
        <-- [/LOWLINE]--|06    11|--[/WDO] ----->
  * N/C <---- [OSC IN]--|07    10|--[/RESET] ---> CPU /RES (37)
  * N/C <--- [OSC SEL]--|08    09|--[RESET] ---->
                        '--------'

  * Set 1.6 seconds as WD timeout.


  U20:   *** ULN2003 ***  High-voltage, high-current Darlington transistor array.

                   ULN2003
                  .---\/---.
           N/C <--|01    16|--> N/C
           N/C <--|02    15|--> N/C
           N/C <--|03    14|--> N/C
  PIA PB2 (12) <--|04    13|--> JAMMA(S08) Coin Counter 2
  PIA PB3 (13) <--|05    12|--> JAMMA(C08) Coin Counter 1
  PIA CB2 (19) <--|06    11|--> JAMMA(S26)
  PIA PB7 (17) <--|07    10|--> CAP --> JAMMA(S10) +Speaker
           GND <--|08    09|--> VCC
                  '--------'


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


*******************************************************************************************************/


#include "emu.h"

#include "cpu/m6809/m6809.h"
#include "machine/6821pia.h"
#include "machine/watchdog.h"
#include "sound/dac.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


// configurable logging
#define LOG_PIA     (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_PIA)

#include "logmacro.h"

#define LOGPIA(...)     LOGMASKED(LOG_PIA,     __VA_ARGS__)


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
		m_battery_ram(*this, "battery_ram"),
		m_coin(*this, "COIN")
	{ }

	void truco(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<palette_device> m_palette;
	required_device<dac_bit_interface> m_dac;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_battery_ram;

	required_ioport m_coin;

	uint8_t m_trigger = 0;

	void porta_w(uint8_t data);
	void pia_ca2_w(int state);
	void portb_w(uint8_t data);
	void pia_irqa_w(int state);
	void pia_irqb_w(int state);

	void palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(interrupt);
	void main_map(address_map &map) ATTR_COLD;
};


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

void truco_state::porta_w(uint8_t data)
{
	LOGPIA("Port A writes: %2x\n", data);
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
}

void truco_state::portb_w(uint8_t data)
{
	m_dac->write(BIT(data, 7)); // Isolated the bit for Delta-Sigma DAC

	if (data & 0x7f)
		LOGPIA("Port B writes: %2x\n", data);
}

void truco_state::pia_irqa_w(int state)
{
	LOGPIA("PIA irq A: %2x\n", state);
}

void truco_state::pia_irqb_w(int state)
{
	LOGPIA("PIA irq B: %2x\n", state);
}


/*******************************************
*                Memory Map                *
*******************************************/

void truco_state::main_map(address_map &map)
{
	map(0x0000, 0x17ff).ram();                          // General purpose RAM
	map(0x1800, 0x7bff).ram().share(m_videoram);
	map(0x7c00, 0x7fff).ram().share(m_battery_ram);
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
	PORT_START("P1")    // IN0
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )        // Connected to JAMMA S17 (P2 START)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )        // Connected to JAMMA S14 (SERVICE SW)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )        // Connected to JAMMA C26 (P2 SELECT)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )        // Connected to JAMMA S16 (COIN2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )        // Connected to JAMMA S15 (TILT SW)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )        // Connected to JAMMA C22 (P1 BUTTON1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    // Connected to JAMMA C18/21 (JOY UP & JOY RIGHT)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  // Connected to JAMMA C19/20 (JOY DOWN & JOY LEFT)

	PORT_START("COIN")  // IN1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("JMPRS") // JP1-2
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING (   0x01, DEF_STR( Off ) )
	PORT_DIPSETTING (   0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Alt. Graphics" )
	PORT_DIPSETTING (   0x02, DEF_STR( Off ) )
	PORT_DIPSETTING (   0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING (   0x04, DEF_STR( Off ) )
	PORT_DIPSETTING (   0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING (   0x08, DEF_STR( Off ) )
	PORT_DIPSETTING (   0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING (   0x10, DEF_STR( Off ) )
	PORT_DIPSETTING (   0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING (   0x20, DEF_STR( Off ) )
	PORT_DIPSETTING (   0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING (   0x40, DEF_STR( Off ) )
	PORT_DIPSETTING (   0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING (   0x80, DEF_STR( Off ) )
	PORT_DIPSETTING (   0x00, DEF_STR( On ) )
INPUT_PORTS_END


/*******************************************
*          Machine Start & Reset           *
*******************************************/

void truco_state::machine_start()
{
	save_item(NAME(m_trigger));
}

void truco_state::machine_reset()
{
	// Setup the data on the battery backed RAM

	// IRQ check
	m_battery_ram[0x002] = 0x51;
	m_battery_ram[0x024] = 0x49;
	m_battery_ram[0x089] = 0x04;
	m_battery_ram[0x170] = 0x12;
	m_battery_ram[0x1a8] = 0xd5;

	// mainloop check
	m_battery_ram[0x005] = 0x04;
	m_battery_ram[0x22b] = 0x46;
	m_battery_ram[0x236] = 0xfb;
	m_battery_ram[0x2fe] = 0x1d;
	m_battery_ram[0x359] = 0x5a;

	// boot check
	int a = (m_battery_ram[0x000] << 8) | m_battery_ram[0x001];

	a += 0x4d2;

	m_battery_ram[0x01d] = (a >> 8) & 0xff;
	m_battery_ram[0x01e] = a & 0xff;
	m_battery_ram[0x020] = m_battery_ram[0x011];
}


/*******************************************
*           Interrupts Handling            *
*******************************************/

INTERRUPT_GEN_MEMBER(truco_state::interrupt)
{
	// coinup

	if (m_coin->read() & 1)
	{
		if (m_trigger == 0)
		{
			device.execute().set_input_line(M6809_IRQ_LINE, HOLD_LINE);
			m_trigger++;
		}
	}
	else
		m_trigger = 0;
}


/*******************************************
*              Machine Config              *
*******************************************/

void truco_state::truco(machine_config &config)
{
	constexpr XTAL MASTER_CLOCK = XTAL(12'000'000); // confirmed
	constexpr XTAL CPU_CLOCK = MASTER_CLOCK / 16;  // guess
	constexpr XTAL CRTC_CLOCK = MASTER_CLOCK / 8;  // guess

	// basic machine hardware
	M6809(config, m_maincpu, CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &truco_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(truco_state::interrupt));

	WATCHDOG_TIMER(config, m_watchdog).set_time(attotime::from_msec(1600)); // 1.6 seconds

	pia6821_device &pia(PIA6821(config, "pia0"));
	pia.readpa_handler().set_ioport("P1");
	pia.readpb_handler().set_ioport("JMPRS");
	pia.writepa_handler().set(FUNC(truco_state::porta_w));
	pia.writepb_handler().set(FUNC(truco_state::portb_w));
	pia.ca2_handler().set(FUNC(truco_state::pia_ca2_w));
	pia.irqa_handler().set(FUNC(truco_state::pia_irqa_w));
	pia.irqb_handler().set(FUNC(truco_state::pia_irqb_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));  // not accurate
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


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( truco )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "truco.u3",   0x08000, 0x4000, CRC(4642fb96) SHA1(e821f6fd582b141a5ca2d5bd53f817697048fb81) )
	ROM_LOAD( "truco.u2",   0x0c000, 0x4000, CRC(ff355750) SHA1(1538f20b1919928ffca439e4046a104ddfbc756c) )
ROM_END

} // anonymous namespace


//    YEAR  NAME     PARENT  MACHINE  INPUT    STATE        INIT        ROT    COMPANY           FULLNAME     FLAGS
GAME( 198?, truco,   0,      truco,   truco,   truco_state, empty_init, ROT0, "Playtronic SRL", "Truco-Tron", MACHINE_SUPPORTS_SAVE )
