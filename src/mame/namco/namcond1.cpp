// license:BSD-3-Clause
// copyright-holders: Mark McDougall, R. Belmont

/*************************************************************

    Namco ND-1 Driver - Mark McDougall
                        R. Belmont

        With contributions from:
            James Jenkins
            Walter Fath

    abcheck TODOs:
    - Ending has a rowscroll GFX bug;
    - Where is the extra data ROM mapped?

    gynotai TODOs:
    - printer (disable it in service mode to suppress POST error);
    - ball sensors aren't understood;
    - Seems to dislike our YGV608 row/colscroll handling
      (for example vertical bounding box is halved offset & size wise for Pac-Man goal stage);

    To make abcheck run when the EEPROM is clear:
    - F2 to enter service mode
    - Player 3 A/B to navigate to GAME OPTIONS
    - Player 1 A to enter, Player 1 B to cancel or go back
    - Go to LOCAL SELECT and choose the Japanese city of your choice (I don't know what it affects yet)
    - Exit test mode (F2) and reset (F3) and the game will boot

-----------------------------------
Guru-Readme for Namco ND-1 hardware
-----------------------------------

Games on this system include...
Namco Classics Volume 1 (Namco 1995)
Namco Classics Volume 2 (Namco 1996)
Abnormal Check          (Namco 1996)


PCB Layout
----------
ND-1 GAME PCB
8655960101 (8655970101) - for Namco Classics 1 & 2

ND-1 GAME(B) PCB
8655960401 (8655970401) - for Namco Classics 1 & 2

ND-1 GAME(C) PCB
8655960500 (8655970500) - for Abnormal Check
|----------------------------------------|
|    LA4705           MAIN0       68000  |
|   4558  LC78815     MAIN1              |
|J  VR1                                  |
|   NFA221             AT28C16           |
|A                                       |
|   NFA222                               |
|M                    CG0   CG1*         |
|          LT1109                        |
|M  NFA222   LM1203                      |
|                 VR2                  J3|
|A                                       |
|   NFA222           YGV608-F            |
|   SW1                                  |
|          VOICE                         |
|  NFA222*               49.152MHz       |
|N                  25.326MHz            |
|A NFA222*                               |
|M         C352                          |
|C NFA221*         MACH210         C416  |
|O     IR2C24*                           |
|4   PC410*                        MB3771|
|8 NFA222*  H8/3002            62256     |
|    PC410*           SUB      62256     |
|----------------------------------------|
Notes:
      68000         - Motorola MC68HC000FN12 Micro-Processor (PLCC68). Clock input 12.288MHz (49.152/4)
      H8/3002       - Hitachi H8/3002 HD6413002F16 Micro-Controller (QFP100). Clock input 16.384MHz (49.152/3)
                      Note the H8/3002 has no internal ROM capability.
      C352          - Namco custom 32-voice 4-channel PCM sound chip (QFP100). Clock input 24.576MHz (49.152/2)
                      Note this is probably a Micro-Controller with internal ROM.
      HSync         - 15.4700kHz
      VSync         - 59.9648Hz
      J3            - 100-pin connector for daughter board (not populated on Namco Classics 1 & 2)
      SW1           - 2-position DIP Switch
      VR1           - Master volume
      VR2           - Brightness adjustment (video level)
      LM1203        - National LM1203 RGB VIDEO AMP (DIP28). Note on some PCB revisions there is a capacitor glued on top of this chip.
      AT28C16       - Atmel 2k x8-bit EEPROM (DIP24)
      YGV608-F      - Yamaha YVG608-F video controller (QFP100)
      LT1109        - Linear Technology LT1109A DC/DC converter (SOIC8). Note on some PCB revisions this is not present. If the IC is required there
                      is an additional 'SREG PCB' with the LT1109 and other support components present at this location.
      C416          - Namco custom (QFP176), Memory/DMA Controller
      MACH210       - AMD MACH211 CPLD, used as Namco "KEYCUS" protection chip (PLCC44)
                       - for Namco Classics 1 stamped 'KC001' at 3C
                       - for Namco Classics 2 stamped 'KC002' at 3C
                       - for Abnormal Check stamped 'KC008' at 3D
      62256         - 32k x8-bit SRAM (SOJ28)
      MB3771        - Fujitsu MB3771 Master Reset IC (SOIC8)
      IR2C24        - Sharp IR2C24 6-Circuit 320mA Transistor Array with Clamping Diodes and Strobe (SOIC16)
      PC410         - Sharp PC410 Ultra-high Speed Response OPIC Photocoupler (SOIC5)
      NFA221        - muRata NFA221 Capacitor Array EMI Suppression Filter
      NFA222        - muRata NFA222 Capacitor Array EMI Suppression Filter
      *             - Not populated on Namco Classics 1 & 2


      ROMs: (note IC locations are different between GAME+GAME(B) and GAME(C) PCBs.

      Namco Classics Volume 1
      -------------------------
      NC2 MAIN0B.14D - 512k x16-bit EPROM type 27C240/27C4002 (for Japan: NC1) (revisions: MAIN0 or MAIN0B)
      NC2 MAIN1B.13D - 512k x16-bit EPROM type 27C240/27C4002 (for Japan: NC1) (revisions: MAIN1 or MAIN1B)
      NC1 SUB.1C     - 512k x16-bit EPROM type 27C240/27C4002
      NC1 CG0.10C    - 16M-bit SOP44 mask ROM
      NC1 VOICE.7B   - 16M-bit SOP44 mask ROM

      Namco Classics Volume 2
      -------------------------
      NCS2 MAIN0B.14D - 512k x16-bit EPROM type 27C240/27C4002 (for Japan: NCS1) (revisions: MAIN0 or MAIN0B)
      NCS2 MAIN1B.13D - 512k x16-bit EPROM type 27C240/27C4002 (for Japan: NCS1) (revisions: MAIN1 or MAIN1B)
      NCS1 SUB.1C     - 512k x16-bit EPROM type 27C240/27C4002
      NCS1 CG0.10C    - 16M-bit SOP44 mask ROM
      NCS1 VOICE.7B   - 16M-bit SOP44 mask ROM

      Abnormal Check
      -------------------------
      AN1 MAIN0B.14E - 512k x16-bit EPROM type 27C240/27C4002
      AN1 MAIN1B.13E - 512k x16-bit EPROM type 27C240/27C4002
      AN1 SUB.1D     - 512k x16-bit EPROM type 27C240/27C4002
      AN1 CG0.10E    - 16M-bit SOP44 mask ROM
      AN1 CG1.10F    - 16M-bit SOP44 mask ROM
      AN1 VOICE.7C   - 16M-bit SOP44 mask ROM


------------------------------------------------------
Additional Guru-Readme for Abnormal Check (Namco 1996)
------------------------------------------------------

Main PCB is common Namco ND-1 hardware documented above.
Several parts that were not populated on the GAME/GAME(B) PCB near the NAMCO48 connector
are populated on this revision and the NAMCO48 connector is used.
Because of the changes the PCB is marked 'GAME(C) PCB' with numbers 8655960500 (8655970500).
The other main difference is the presence of an extra connector on one edge between
the 68000 and the C416, labelled J3.
Most of the parts on the GAME(C) PCB have different locations, however the PCB appears
to be electrically identical to the earlier revision ND-1 GAME PCB.

There is an extra daughter board 115mm x 110mm plugged into connector J3.
The PCB is marked "M122 MEM/PRN PCB 1507960103 (1507970103)
The PCB contains the following parts....
1x ST 27C4002 EPROM (DIP40 at IC1)
2x ST M48Z30Y ZEROPOWER RAM (DIP28 at IC2 & IC3)
2x 74HC244 logic (SOIC20)
2x Toshiba TD64064 Darlington Driver (SOIC18)
1x AMD MACH120 CPLD (PLCC68 at IC8)
1x 10-pin JST connector labelled J11 for connection to the printer.
1x 20-pin flat cable connector labelled J12 for connection to the printer.

Partial Pinout of J12
----------------------
   GND 10b  10a GND
  ERROR 9b  9a
  EMPTY 8b  8a  BUSY
        7b  7a
        6b  6a
        5b  5a
        4b  4a
        3b  3a
   +12V 2b  2a  +5V
   GND  1b  1a  GND

To get the board to boot some pins on the J12 connector must be set to 0 or 1 (tied to ground or +5V).
The status can be checked in test mode in the "Printer Test" menu.
ERROR = HIGH (No Error)
EMPTY = LOW  (Not Empty)
BUSY  = LOW  (Ready)

Connected to J11/J12 is a thermal printer. It prints on a roll of 2 1/4" wide thermal paper.
Bolted onto the metal frame is a small 80mm square PCB. There is no manufacturer name on it
and only some numbers/letters "32-104C SEC-A"
The PCB contains the following parts....
1x 27C010 128k x8-bit EPROM (DIP32 at U9)
1x 16M-bit mask ROM (SOP44 at U4)
1x 8-position DIP Switch labelled SW1. Position 2 is on, all others are off
1x NEC uPC393 Dual Comparator (SIL9)
1x Sanyo LB1650 Dual-Directional Motor Driver (DIP16)
1x Maxim MAX202 RS232 Transceiver (SOIC16)
1x Toshiba TC55257 32k x8-bit SRAM (TSOP28)
1x Toshiba TMP95C061AF TLCS90/900 compatible 16-bit Micro-Controller (TQFP100). Note there is no internal ROM capability.
Some logic, resistors/caps/transistors, some connectors etc.

*************************************************************/

#include "emu.h"

#include "ygv608.h"

#include "cpu/h8/h83002.h"
#include "cpu/m68000/m68000.h"
#include "cpu/m6809/m6809.h"
#include "machine/at28c16.h"
#include "machine/nvram.h"
#include "sound/c352.h"

#include "screen.h"
#include "speaker.h"


// configurable logging
#define LOG_CUSKEY     (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_CUSKEY)

#include "logmacro.h"

#define LOGCUSKEY(...)     LOGMASKED(LOG_CUSKEY,     __VA_ARGS__)


namespace {

class namcond1_state : public driver_device
{
public:
	namcond1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mcu(*this, "mcu"),
		m_ygv608(*this, "ygv608"),
		m_shared_ram(*this, "shared_ram") { }

	void abcheck(machine_config &config);
	void namcond1(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<h83002_device> m_mcu;
	required_device<ygv608_device> m_ygv608;

	required_shared_ptr<uint16_t> m_shared_ram;

	uint8_t m_h8_irq5_enabled = 0;
	uint8_t m_p8 = 0;

	uint8_t mcu_p7_read();
	uint8_t mcu_pa_read();
	void mcu_pa_write(uint8_t data);
	uint16_t cuskey_r(offs_t offset);
	void cuskey_w(offs_t offset, uint16_t data);
	uint16_t printer_r();

	INTERRUPT_GEN_MEMBER(mcu_interrupt);
	void abcheck_main_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
	void h8rwmap(address_map &map) ATTR_COLD;
};


// Perform basic machine initialisation


void namcond1_state::machine_start()
{
	save_item(NAME(m_h8_irq5_enabled));
	// save_item(NAME(m_p8)); //isn't read anywhere for the time being
}

void namcond1_state::machine_reset()
{
#ifdef MAME_DEBUG
	/*uint8_t   *ROM = memregion(REGION_CPU1)->base();*/
	/*uint32_t debug_trigger_addr;*/
	/*int             i;*/

#if 0
	// debug trigger patch
	// insert a "move.b $B0000000,D2" into the code
	debug_trigger_addr = 0x152d4; // after ygv_init
	ROM[debug_trigger_addr++] = 0x39;
	ROM[debug_trigger_addr++] = 0x14;
	ROM[debug_trigger_addr++] = 0xb0;
	ROM[debug_trigger_addr++] = 0x00;
	ROM[debug_trigger_addr++] = 0x00;
	ROM[debug_trigger_addr++] = 0x00;
#endif
#endif

	// initialise MCU states
	m_h8_irq5_enabled = 0;

	// halt the MCU
	m_mcu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

// $c3ff00-$c3ffff
uint16_t namcond1_state::cuskey_r(offs_t offset)
{
	switch (offset)
	{
		// this address returns a jump vector inside ISR2
		// - if zero then the ISR returns without jumping
		case (0x2e >> 1):
			return 0x0000;
		case (0x30 >> 1):
			return 0x0000;

		default:
			LOGCUSKEY("%s offset $%X accessed\n",
				machine().describe_context(), offset << 1);
			return 0;
	}
}

void namcond1_state::cuskey_w(offs_t offset, uint16_t data)
{
	switch (offset)
	{
		case (0x0a >> 1):
			// this is a kludge until we emulate the h8
			if ((m_h8_irq5_enabled == 0) && (data != 0x0000))
			{
				m_mcu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
			}
			m_h8_irq5_enabled = (data != 0x0000);
			break;

		case (0x0c >> 1):
			m_ygv608->set_gfxbank(data & 0x0003);
			// bit 0 used in abcheck during garbage screens, tile/color select of some kind?
			break;

		default:
			break;
	}
}


/*************************************************************/

void namcond1_state::main_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x400000, 0x40ffff).ram().share(m_shared_ram);
	map(0x800000, 0x80000f).m(m_ygv608, FUNC(ygv608_device::port_map)).umask16(0xff00);
	map(0xa00000, 0xa00fff).rw("at28c16", FUNC(at28c16_device::read), FUNC(at28c16_device::write)).umask16(0xff00);
	map(0xc3ff00, 0xc3ffff).rw(FUNC(namcond1_state::cuskey_r), FUNC(namcond1_state::cuskey_w));
}

void namcond1_state::abcheck_main_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x400000, 0x40ffff).ram().share(m_shared_ram);
	map(0x600000, 0x607fff).ram().share("zpr1");
	map(0x608000, 0x60ffff).ram().share("zpr2");
	map(0x700000, 0x700001).nopw();
	map(0x740000, 0x740001).nopw();
	map(0x780000, 0x780001).r(FUNC(namcond1_state::printer_r));
	map(0x800000, 0x80000f).m(m_ygv608, FUNC(ygv608_device::port_map)).umask16(0xff00);
	map(0xa00000, 0xa00fff).rw("at28c16", FUNC(at28c16_device::read), FUNC(at28c16_device::write)).umask16(0xff00);
	map(0xc3ff00, 0xc3ffff).rw(FUNC(namcond1_state::cuskey_r), FUNC(namcond1_state::cuskey_w));
}

uint16_t namcond1_state::printer_r()
{
	// bits tested:
	// bit 2 = 0 for paper cut switch on, 1 for off
	// bit 4 = 0 for paper OK, 1 for empty
	// bit 5 = 1 for normal status, 0 for error
	return 0x0020;
}

/*************************************************************/

static INPUT_PORTS_START( namcond1 )
	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0100, 0x0100, "Freeze" )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Test ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE( 0x4000, IP_ACTIVE_LOW )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( gynotai )
	PORT_INCLUDE( namcond1 )

	PORT_MODIFY("P1_P2")
	// TODO: these are presumably ball sensors
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Left 1")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Left 2")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Center 1")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Center 2")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Right 1")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Right 2")
	PORT_BIT( 0xffc0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( abcheck )
	PORT_INCLUDE( namcond1 )

	PORT_MODIFY("P1_P2")
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 A")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 B")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 A")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0f00, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 B")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3) PORT_NAME("P3 A")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3) PORT_NAME("P3 B")
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("DSW")
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_COIN1 )
INPUT_PORTS_END


uint8_t namcond1_state::mcu_p7_read()
{
	return 0xff;
}

uint8_t namcond1_state::mcu_pa_read()
{
	return 0xff;
}

void namcond1_state::mcu_pa_write(uint8_t data)
{
	m_p8 = data;
}

// H8/3002 MCU stuff
void namcond1_state::h8rwmap(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x200000, 0x20ffff).ram().share(m_shared_ram);
	map(0xa00000, 0xa07fff).rw("c352", FUNC(c352_device::read), FUNC(c352_device::write));
	map(0xc00000, 0xc00001).portr("DSW");
	map(0xc00002, 0xc00003).portr("P1_P2");
	map(0xc00010, 0xc00011).noprw();
	map(0xc00030, 0xc00031).noprw();
	map(0xc00040, 0xc00041).noprw();
	map(0xffff1a, 0xffff1b).noprw(); // abcheck
	map(0xffff1e, 0xffff1f).noprw(); // ^
}

INTERRUPT_GEN_MEMBER(namcond1_state::mcu_interrupt)
{
	if (m_h8_irq5_enabled)
	{
		device.execute().pulse_input_line(5, device.execute().minimum_quantum_time());
	}
}

/******************************************
  ND-1 Master clock = 49.152MHz
  - 68000   = 12288000 (CLK/4)
  - H8/3002 = 16384000 (CLK/3)
  - The level 1 interrupt to the 68k has been measured at 60Hz.
*******************************************/

void namcond1_state::namcond1(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, XTAL(49'152'000) / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &namcond1_state::main_map);
//  m_maincpu->set_vblank_int("screen", FUNC(namcond1_state::irq1_line_hold));

	H83002(config, m_mcu, XTAL(49'152'000) / 3);
	m_mcu->set_addrmap(AS_PROGRAM, &namcond1_state::h8rwmap);
	m_mcu->set_vblank_int("screen", FUNC(namcond1_state::mcu_interrupt));
	m_mcu->read_adc<0>().set_constant(0); // MCU reads these, but the games have no analog controls
	m_mcu->read_adc<1>().set_constant(0);
	m_mcu->read_adc<2>().set_constant(0);
	m_mcu->read_adc<3>().set_constant(0);
	m_mcu->read_port7().set(FUNC(namcond1_state::mcu_p7_read));
	m_mcu->read_porta().set(FUNC(namcond1_state::mcu_pa_read));
	m_mcu->write_porta().set(FUNC(namcond1_state::mcu_pa_write));

	config.set_maximum_quantum(attotime::from_hz(6000));

	YGV608(config, m_ygv608, 0);
	m_ygv608->vblank_callback().set_inputline(m_maincpu, 1);
	m_ygv608->raster_callback().set_inputline(m_maincpu, 2);
	m_ygv608->set_screen("screen");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	/*
	H 804 108 576 48 32
	V 261 26 224 3 0
	*/
	screen.set_raw(XTAL(49'152'000) / 8, 804 / 2, 108 / 2, (108 + 576) / 2, 261, 26, 26 + 224);
	screen.set_screen_update("ygv608", FUNC(ygv608_device::screen_update));
	screen.set_palette("ygv608");

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	c352_device &c352(C352(config, "c352", XTAL(49'152'000) / 2, 288));
	c352.add_route(0, "lspeaker", 1.00);
	c352.add_route(1, "rspeaker", 1.00);
	//c352.add_route(2, "lspeaker", 1.00); // Second DAC not present.
	//c352.add_route(3, "rspeaker", 1.00);

	AT28C16(config, "at28c16", 0);
}

void namcond1_state::abcheck(machine_config &config)
{
	namcond1(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &namcond1_state::abcheck_main_map);
//  m_maincpu->set_vblank_int("screen", FUNC(namcond1_state::irq1_line_hold));

	NVRAM(config, "zpr1", nvram_device::DEFAULT_ALL_0);
	NVRAM(config, "zpr2", nvram_device::DEFAULT_ALL_0);
}

ROM_START( ncv1 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD( "nc2main0.14d", 0x00000, 0x80000, CRC(4ffc530b) SHA1(23d622d0261a3584236a77b2cefa522a0f46490e) )
	ROM_LOAD16_WORD( "nc2main1.13d", 0x80000, 0x80000, CRC(26499a4e) SHA1(4af0c365713b4a51da684a3423b07cbb70d9599b) )

	ROM_REGION( 0x80000, "mcu", 0 )
	ROM_LOAD( "nc1sub.1c",          0x00000, 0x80000, CRC(48ea0de2) SHA1(33e57c8d084a960ccbda462d18e355de44ec7ad9) )

	ROM_REGION( 0x800000, "ygv608", 0 )    // 2MB character generator
	ROM_LOAD( "nc1cg0.10c",         0x000000, 0x200000, CRC(d4383199) SHA1(3263874ba838651631ad6d11afb150206de760bb) )
	ROM_RELOAD(                     0x200000, 0x200000 )
	ROM_RELOAD(                     0x400000, 0x200000 )
	ROM_RELOAD(                     0x600000, 0x200000 )

	ROM_REGION( 0x200000, "c352", 0 ) // Samples
	ROM_LOAD( "nc1voice.7b",     0x000000, 0x200000, CRC(91c85bd6) SHA1(c2af8b1518b2b601f2b14c3f327e7e3eae9e29fc) )
ROM_END

ROM_START( ncv1j )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD( "nc1main0.14d",  0x00000, 0x80000, CRC(48ce0b2b) SHA1(07dfca8ba935ee0151211f9eb4d453f2da1d4bd7) )
	ROM_LOAD16_WORD( "nc1main1.13d",  0x80000, 0x80000, CRC(49f99235) SHA1(97afde7f7dddd8538de78a74325d0038cb1217f7) )

	ROM_REGION( 0x80000, "mcu", 0 )
	ROM_LOAD( "nc1sub.1c",          0x00000, 0x80000, CRC(48ea0de2) SHA1(33e57c8d084a960ccbda462d18e355de44ec7ad9) )

	ROM_REGION( 0x800000, "ygv608", 0 )    // 2MB character generator
	ROM_LOAD( "nc1cg0.10c",         0x000000, 0x200000, CRC(d4383199) SHA1(3263874ba838651631ad6d11afb150206de760bb) )
	ROM_RELOAD(                     0x200000, 0x200000 )
	ROM_RELOAD(                     0x400000, 0x200000 )
	ROM_RELOAD(                     0x600000, 0x200000 )

	ROM_REGION( 0x200000, "c352", 0 ) // Samples
	ROM_LOAD( "nc1voice.7b",     0x000000, 0x200000, CRC(91c85bd6) SHA1(c2af8b1518b2b601f2b14c3f327e7e3eae9e29fc) )
ROM_END

ROM_START( ncv1j2 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD( "nc1main0b.14d", 0x00000, 0x80000, CRC(7207469d) SHA1(73faf1973a57c1bc2163e9ee3fe2febd3b8763a4) )
	ROM_LOAD16_WORD( "nc1main1b.13d", 0x80000, 0x80000, CRC(52401b17) SHA1(60c9f20831d0101c02dafbc0bd15422f71f3ad81) )

	ROM_REGION( 0x80000, "mcu", 0 )
	ROM_LOAD( "nc1sub.1c",          0x00000, 0x80000, CRC(48ea0de2) SHA1(33e57c8d084a960ccbda462d18e355de44ec7ad9) )

	ROM_REGION( 0x800000, "ygv608", 0 )    // 2MB character generator
	ROM_LOAD( "nc1cg0.10c",         0x000000, 0x200000, CRC(d4383199) SHA1(3263874ba838651631ad6d11afb150206de760bb) )
	ROM_RELOAD(                     0x200000, 0x200000 )
	ROM_RELOAD(                     0x400000, 0x200000 )
	ROM_RELOAD(                     0x600000, 0x200000 )

	ROM_REGION( 0x200000, "c352", 0 ) // Samples
	ROM_LOAD( "nc1voice.7b",     0x000000, 0x200000, CRC(91c85bd6) SHA1(c2af8b1518b2b601f2b14c3f327e7e3eae9e29fc) )
ROM_END

ROM_START( ncv2 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD( "ncs2main0.14e", 0x00000, 0x80000, CRC(fb8a4123) SHA1(47acdfe9b5441d0e3649aaa9780e676f760c4e42) )
	ROM_LOAD16_WORD( "ncs2main1.13e", 0x80000, 0x80000, CRC(7a5ef23b) SHA1(0408742424a6abad512b5baff63409fe44353e10) )

	ROM_REGION( 0x80000, "mcu", 0 )
	ROM_LOAD( "ncs1sub.1d",          0x00000, 0x80000, CRC(365cadbf) SHA1(7263220e1630239e3e88b828c00389d02628bd7d) )

	ROM_REGION( 0x800000, "ygv608", 0 )    // 4MB character generator
	ROM_LOAD( "ncs1cg0.10e",         0x000000, 0x200000, CRC(fdd24dbe) SHA1(4dceaae3d853075f58a7408be879afc91d80292e) )
	ROM_RELOAD(                      0x200000, 0x200000 )
	ROM_LOAD( "ncs1cg1.10f",         0x400000, 0x200000, CRC(007b19de) SHA1(d3c093543511ec1dd2f8be6db45f33820123cabc) )
	ROM_RELOAD(                      0x600000, 0x200000 )

	ROM_REGION( 0x200000, "c352", 0 ) // Samples
	ROM_LOAD( "ncs1voic.7c",     0x000000, 0x200000, CRC(ed05fd88) SHA1(ad88632c89a9946708fc6b4c9247e1bae9b2944b) )
ROM_END

ROM_START( ncv2j )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD( "ncs1main0.14e", 0x00000, 0x80000, CRC(99991192) SHA1(e0b0e15ae23560b77119b3d3e4b2d2bb9d8b36c9) )
	ROM_LOAD16_WORD( "ncs1main1.13e", 0x80000, 0x80000, CRC(af4ba4f6) SHA1(ff5adfdd462cfd3f17fbe2401dfc88ff8c71b6f8) )

	ROM_REGION( 0x80000, "mcu", 0 )
	ROM_LOAD("ncs1sub.1d",          0x00000, 0x80000, CRC(365cadbf) SHA1(7263220e1630239e3e88b828c00389d02628bd7d) )

	ROM_REGION( 0x800000, "ygv608", 0 )    // 4MB character generator
	ROM_LOAD( "ncs1cg0.10e",         0x000000, 0x200000, CRC(fdd24dbe) SHA1(4dceaae3d853075f58a7408be879afc91d80292e) )
	ROM_RELOAD(                      0x200000, 0x200000 )
	ROM_LOAD( "ncs1cg1.10f",         0x400000, 0x200000, CRC(007b19de) SHA1(d3c093543511ec1dd2f8be6db45f33820123cabc) )
	ROM_RELOAD(                      0x600000, 0x200000 )

	ROM_REGION( 0x1000000, "c352", 0 ) // Samples
	ROM_LOAD( "ncs1voic.7c",     0x000000, 0x200000, CRC(ed05fd88) SHA1(ad88632c89a9946708fc6b4c9247e1bae9b2944b) )
ROM_END

ROM_START( gynotai )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "gy1main0.14e", 0x000000, 0x080000, CRC(1421dbf5) SHA1(7e4322cddc3317c9ed82a97c0fe387ce1364cf9b) )
	ROM_LOAD( "gy1main1.13e", 0x080000, 0x080000, CRC(dc10a4a7) SHA1(01a6b5aae8599de9015d6e332f5bd286bc84c807) )

	ROM_REGION( 0x80000, "mcu", 0 )
	ROM_LOAD( "gy1sub0.1d",   0x000000, 0x080000, CRC(fd31e963) SHA1(b658921dd29cfad0c366465ae37a356c3d2fb4d3) )

	ROM_REGION( 0x800000, "ygv608", 0 )    // 8MB character generator
	ROM_LOAD( "gy1cg0.10e",   0x000000, 0x400000, CRC(938c7912) SHA1(36278a945a00e1549ae55ec65a9b4001537023b0) )
	ROM_LOAD( "gy1cg1.10f",   0x400000, 0x400000, CRC(5a518733) SHA1(b6ea91629bc6ddf67c47c4189084aa947f4e31ed) )

	ROM_REGION( 0x200000, "c352", 0 ) // Samples
	ROM_LOAD( "gy1voice.7c",  0x000000, 0x200000, CRC(f135e79b) SHA1(01ce3e3b366d0b9045ad8599b60ca33c6d21f150) )
ROM_END

ROM_START( abcheck )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "an1main0b.14e", 0x000000, 0x080000, CRC(f1b9777d) SHA1(b28f4106e1e145dc1aaa5af455b6f991d2b04c59) )
	ROM_LOAD( "an1main1b.13e", 0x080000, 0x080000, CRC(d40ccdcc) SHA1(05f864d84bf34a1722c598378ed8d27fba00f575) )

	ROM_REGION( 0x80000, "mcu", 0 )
	ROM_LOAD( "an1sub.1d",    0x000000, 0x080000, CRC(50de9130) SHA1(470b3977f4bf12ca65bc42631ccdf81753ef56fd) )

	ROM_REGION( 0x800000, "ygv608", 0 )    // 4MB character generator
	ROM_LOAD( "an1cg0.10e",   0x000000, 0x400000, CRC(14425378) SHA1(c690bd0f48fa2bc285b63e6bc379d2b345eafc7b) )
	ROM_LOAD( "an1cg1.10f",   0x400000, 0x400000, CRC(0428d718) SHA1(4b4dca7196b9ba1a01558f41c761d3211be67fb0) )

	ROM_REGION( 0x1000000, "c352", 0 ) // Samples
	ROM_LOAD( "an1voice.7c",  0x000000, 0x200000, CRC(d2bfa453) SHA1(6b7d6bb4d65290d8fd3df5d12b41ae7dce5f3f1c) )

	ROM_REGION( 0x80000, "data", 0 )    // game data?
	ROM_LOAD( "an1dat0.ic1",  0x000000, 0x080000, CRC(44dc7da1) SHA1(dd57670a2b07c4988ca30bba134931c1701a926f) )

	ROM_REGION( 0x8000, "zpr1", 0 )
	ROM_LOAD( "m48z30y.ic2",  0x000000, 0x008000, CRC(a816d989) SHA1(c78fe06b049c31cf8de2a79025823dbc0c95d526) )

	ROM_REGION( 0x8000, "zpr2", 0 )
	ROM_LOAD( "m48z30y.ic3",  0x000000, 0x008000, CRC(bfa687bb) SHA1(463ae40f21b675f3b4155efda9c965b71519a49e) )

	ROM_REGION( 0x800, "at28c16", 0 )
	ROM_LOAD( "at28c16.12e",  0x000000, 0x000800, CRC(df92af14) SHA1(1ae8c318f1eb2628e97914d15a06779c7bb87506) )

	ROM_REGION( 0x220000, "printer", 0 )
	ROM_LOAD( "np-b205_nmc_ver1.00.u9", 0x000000, 0x020000, CRC(445ceb0d) SHA1(49491b936f50577564196992df3a3c93aa3fcc99) )
	ROM_LOAD( "npg1624lc.u4", 0x020000, 0x200000, CRC(7e00254f) SHA1(b0fa8f979e8322d71f842de5358ae2a2e36386f7) )
ROM_END

} // anonymous namespace


// FWIW it looks like version numbering at POST is for the ND1 framework build the games are based on.
GAME( 1995, ncv1,    0,    namcond1, namcond1, namcond1_state, empty_init, ROT90, "Namco", "Namco Classic Collection Vol.1",                MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // 1.00
GAME( 1995, ncv1j,   ncv1, namcond1, namcond1, namcond1_state, empty_init, ROT90, "Namco", "Namco Classic Collection Vol.1 (Japan, v1.00)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1995, ncv1j2,  ncv1, namcond1, namcond1, namcond1_state, empty_init, ROT90, "Namco", "Namco Classic Collection Vol.1 (Japan, v1.03)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // 1.03

GAME( 1996, gynotai, 0,    namcond1, gynotai,  namcond1_state, empty_init, ROT0,  "Namco", "Gynotai (Japan)",                               MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING | MACHINE_NODEVICE_PRINTER | MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE ) // 1.04

GAME( 1996, ncv2,    0,    namcond1, namcond1, namcond1_state, empty_init, ROT90, "Namco", "Namco Classic Collection Vol.2",                MACHINE_IMPERFECT_GRAPHICS | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE ) // 1.10
GAME( 1996, ncv2j,   ncv2, namcond1, namcond1, namcond1_state, empty_init, ROT90, "Namco", "Namco Classic Collection Vol.2 (Japan)",        MACHINE_IMPERFECT_GRAPHICS | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE )

GAME( 1996, abcheck, 0,    abcheck,  abcheck,  namcond1_state, empty_init, ROT0,  "Namco", "Abnormal Check",                                MACHINE_IMPERFECT_GRAPHICS | MACHINE_UNEMULATED_PROTECTION | MACHINE_NODEVICE_PRINTER | MACHINE_SUPPORTS_SAVE ) // 1.20EM
