// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Enterprise Sixty Four / One Two Eight emulation

**********************************************************************/

/*

Enterprise Sixty Four / Enterprise One Two Eight
Developed by Intelligent Software, marketed by Enterprise Computers Ltd. 1985

MAIN PCB Layout
---------------
                                        DUAL
|-----------| 9V                        TAPE_IN/OUT                             RES
|HEATSINK   | DC_IN             MON1    AND REMOTE     SR1    PR1   CN2A CN1A   |-|
|           |--||---|--||--|--| |--| |-||-||-||-||-| |----| |-----| |--| |--| |-|-|------|
|-----------|7805   | MOD  |  |-|  |-|             |-|    |-|     |-|  |-|  |-|   EXP2 --|EXTERNAL
        |---+7805   |      |                         74LS06 74LS273 74LS86 74LS32      --|EXPANSION
        |   |       |------|             74LS244                                       --|CONNECTOR
        |   |78L12 POT1  LM386                               |-----|   |-----|   EXP1  --|
   CART |   |          LM1889 LM1886                   74LS04|NICK |   |DAVE |         --|
   CONN |   |   KEYB_8     POT2     KEYB_10                  |     |   |     |         --|
        |   |            4.433619MHz     74LS145 POT3 LED    |-----|   |-----|         --|
        |---+                                                                          --|
            |                            74LS74      74LS244  74LS244  74LS245 |---------|
            |                                                                  |
            | 74LS373                    4164  4164  74F157                    |
            |                                                                  |
            | 16-2-103   LM339    LM324  4164  4164  74F157           EXOS.IC2 |
            |                                                 8MHz             |
            | 74LS373                    4164  4164                            |
            |                                                74LS04    Z80A    |
            | 74LS273                    4164  4164                            |
            |------------------------------------------------------------------|
Notes: (all IC's shown)
           Z80A - Z80A CPU, clock input 4MHz [8/2]
       EXOS.IC2 - 32k x8-bit mask ROM usually 23256 manufactured by GI (DIP24). Contains EXOS operating
                  system and built-in word processor software. A few official revisions were made and
                  there were a few unofficial revision made with additional capabilities and bug-fixes
                  ROM labelling of some official versions found....
                  9256DS-0019 (C)1984 INTELLIGENT SOFTWARE LTD ENTER 08-45-A GI
                  9256DS-0036 (C)1984 INTELLIGENT SOFTWARE LTD ENTER 05-23-A GI
           4164 - 64k x1-bit Dynamic RAM with Page Mode (DIP16)
           NICK - Custom graphics chip (QFP72)
           DAVE - Custom sound chip (QFP72)
          LM386 - National Semiconductor LM386 Low Voltage Audio Power Amplifier (DIP8)
         LM1889 - National Semiconductor LM1889 TV Video Modulator (DIP18)
         LM1886 - National Semiconductor LM1886 TV Video Matrix DAC (DIP20)
          LM339 - SGS LM339 Low Power Low Offset Voltage Quad Comparator (DIP8)
          LM324 - SGS LM324 Quad Operational Amplifier (DIP8)
         74LS04 - Hex Inverter (DIP14)
         74LS06 - Hex Inverter/Buffer with Open-Collector Outputs (DIP14)
         74LS32 - Quad 2-Input Positive OR Gate (DIP14)
         74LS74 - Dual Positive-Edge-Triggered D Flip-Flops with Preset, Clear and Complementary Outputs (DIP14)
         74LS86 - Quad 2-Input Exclusive OR Gate (DIP14)
        74LS145 - BCD to Deccimal Decoder/Driver (DIP16)
         74F157 - Quad 2-Line to 1-Line Data Selector/Multiplexer (DIP16). Early rev mainboards use 74LS158 instead
        74LS244 - Octal 3-State Noninverting Buffer/Line Driver/Line Receiver (DIP20)
        74LS245 - Octal Bus Tranceiver with Tri-State Outputs (DIP20)
        74LS273 - Octal D-Type Flip-Flop With Clear (DIP20)
        74LS373 - Octal D-Type Transparent Latches and Edge-Triggered Flip-Flops (DIP20)
           7805 - Voltage regulator. +9V DC input from DC power pack, +5V DC output
          78L12 - Voltage regulator. Voltage input via small transformer on PCB. +12V DC output
           POT1 - Potentiometer located near modulator and video output circuitry
           POT2 - Potentiometer located near video output circuitry. Probably used to fine-tune the video clock
           POT3 - Potentiometer. Possibly for video/NICK-related adjustments
            LED - LED to show +5V is present
       16-2-103 - Resistor Array (DIP16)
         KEYB_8 - 8 pin keyboard connector
        KEYB_10 - 10 pin keyboard connector
           EXP1 - 28 pin internal expansion connector (solder holes only) used for internal 64k memory expansion card
           EXP2 - 10 pin internal expansion connector (solder holes only) used for internal 64k memory expansion card
            MOD - Astec UM1233 TV modulator
            RES - Reset push button switch
           MON1 - Monitor output connector. Pinout is....

                            Green  A1 B1 NC
                            Ground A2 B2 Ground
                  Mono Comp. Video A3 B3 Blue
                             HSync A4 B4 Red
                             VSync A5 B5 Composite Sync
                                NC A6 B6 Mode Switch
                        Left Audio A7 B7 Right Audio

            SR1 - Serial/Network connector. Pinout is....

                         Reference A1 B1 Ground
                                 - A2 B2 -
                               RTS A3 B3 Data Out
                               CTS A4 B4 Data In

            PR1 - Printer connector. Pinout is....

                            Ground A1 B1 Ground
                            Strobe A2 B2 Ready
                            Data 3 A3 B3 Data 4
                                 - A4 B4 -
                            Data 2 A5 B5 Data 5
                            Data 1 A6 B6 Data 6
                            Data 0 A7 B7 Data 7

           CN2A - Joystick 2 connector
           CN1A - Joystick 1 connector
                  Pinout is....

                          Keyboard J A1 B1 Ground
                          Keyboard L A2 B2 Keyboard K
                                   - A3 B3 -
                               Right A4 B4 +5V
                                Down A5 B5 Left
                                Fire A6 B6 Up


Internal Memory Expansion PCB Layout
------------------------------------

|---------------------|
|  4164 74LS00 PL3 PL2|
|4164 74LS30 74F157   |
|4164 4164            |
|4164 4164   74F157   |
|4164 4164 74LS32  PL1|
|---------------------|
Notes: (All IC's shown)
          PL1 - 28-pin connector (solder pads only) hard-wired to solder pads EXP1 on mainboard
      PL2/PL3 - 5-pin connectors (solder pads only) hard-wired to solder pads EXP2 on mainboard
         4164 - 64k x1-bit Dynamic RAM with Page Mode (DIP16)
       74LS00 - Quad 2-Input NAND Gate (DIP14)
       74LS30 - 8-input NAND Gate (DIP14)
       74LS32 - Quad 2-Input Positive OR Gate (DIP14)
       74F157 - Quad 2-Line to 1-Line Data Selector/Multiplexer (DIP16). Early rev memory boards use 74LS158 instead

*/

/*

    TODO:

    - POST RAM errors
    - rewrite DAVE to output to discrete DAC
    - rewrite NICK
    - cassette
    - external joysticks

    http://ep.homeserver.hu/Dokumentacio/Konyvek/

*/

#include "emu.h"

#include "bus/centronics/ctronics.h"
#include "bus/ep64/exp.h"
#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "machine/ram.h"
#include "sound/dave.h"
#include "nick.h"

#include "softlist_dev.h"
#include "speaker.h"


namespace {

#define Z80_TAG         "u1"
#define DAVE_TAG        "u3"
#define NICK_TAG        "u4"
#define CENTRONICS_TAG  "centronics"
#define RS232_TAG       "rs232"
#define CASSETTE1_TAG   "cassette1"
#define CASSETTE2_TAG   "cassette2"
#define SCREEN_TAG      "screen"
#define EP64_EXPANSION_BUS_TAG  "exp"

class ep64_state : public driver_device
{
public:
	ep64_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, Z80_TAG),
		m_dave(*this, DAVE_TAG),
		m_nick(*this, NICK_TAG),
		m_centronics(*this, CENTRONICS_TAG),
		m_rs232(*this, RS232_TAG),
		m_cassette1(*this, CASSETTE1_TAG),
		m_cassette2(*this, CASSETTE2_TAG),
		m_cart(*this, "cartslot"),
		m_exp(*this, EP64_EXPANSION_BUS_TAG),
		m_ram(*this, RAM_TAG),
		m_rom(*this, Z80_TAG),
		m_y(*this, "Y%u", 0)
	{ }

	void ep128(machine_config &config);
	void ep64(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<dave_device> m_dave;
	required_device<nick_device> m_nick;
	required_device<centronics_device> m_centronics;
	required_device<rs232_port_device> m_rs232;
	required_device<cassette_image_device> m_cassette1;
	required_device<cassette_image_device> m_cassette2;
	required_device<generic_slot_device> m_cart;
	required_device<ep64_expansion_bus_slot_device> m_exp;
	required_device<ram_device> m_ram;
	required_memory_region m_rom;
	required_ioport_array<10> m_y;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint8_t rd0_r();
	void wr0_w(uint8_t data);
	uint8_t rd1_r();
	void wr2_w(uint8_t data);

	uint8_t m_key;

	void write_centronics_busy(int state);
	int m_centronics_busy;
	void dave_128k_mem(address_map &map) ATTR_COLD;
	void dave_64k_mem(address_map &map) ATTR_COLD;
	void dave_io(address_map &map) ATTR_COLD;
	void ep64_io(address_map &map) ATTR_COLD;
	void ep64_mem(address_map &map) ATTR_COLD;
};



//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

//-------------------------------------------------
//  rd0_r -
//-------------------------------------------------

uint8_t ep64_state::rd0_r()
{
	uint8_t data = 0xff;

	if (m_key < 10)
	{
		data &= m_y[m_key]->read();
	}

	return data;
}


//-------------------------------------------------
//  rd0_r -
//-------------------------------------------------

void ep64_state::wr0_w(uint8_t data)
{
	/*

	    bit     description

	    0       KEY A
	    1       KEY B
	    2       KEY C
	    3       KEY D
	    4       PRINTER _STB
	    5       CASSETTE OUT
	    6       REMOTE 1
	    7       REMOTE 2

	*/

	// keyboard
	m_key = data & 0x0f;

	// printer
	m_centronics->write_strobe(!BIT(data, 4));

	// cassette
	m_cassette1->output(BIT(data, 5) ? -1.0 : +1.0);
	m_cassette2->output(BIT(data, 5) ? -1.0 : +1.0);

	// cassette
	m_cassette1->change_state(BIT(data, 6) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
	m_cassette2->change_state(BIT(data, 7) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
}

void ep64_state::write_centronics_busy(int state)
{
	m_centronics_busy = state;
}

//-------------------------------------------------
//  rd1_r -
//-------------------------------------------------

uint8_t ep64_state::rd1_r()
{
	/*

	    bit     description

	    0       KBJ
	    1       KBK
	    2       KBL
	    3       PRINTER _RDY
	    4       SERIAL/NET DATA IN
	    5       SERIAL/NET STATUS IN
	    6       CASSETTE IN
	    7       ?

	*/

	uint8_t data = 0;

	// printer
	data |= m_centronics_busy << 3;

	// serial
	data |= m_rs232->rxd_r() << 4;
	data |= m_rs232->cts_r() << 5;

	// cassette
	data |= ((m_cassette1->input() < 0) || (m_cassette2->input() < 0)) << 6;

	return data;
}


//-------------------------------------------------
//  wr2_w -
//-------------------------------------------------

void ep64_state::wr2_w(uint8_t data)
{
	/*

	    bit     description

	    0       SERIAL/NET DATA OUT
	    1       SERIAL/NET STATUS OUT
	    2
	    3
	    4
	    5
	    6
	    7

	*/

	// serial
	m_rs232->write_txd(!BIT(data, 0));
	m_rs232->write_rts(!BIT(data, 1));
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( ep64_mem )
//-------------------------------------------------

void ep64_state::ep64_mem(address_map &map)
{
	map(0x0000, 0xffff).m(m_dave, FUNC(dave_device::z80_program_map));
}


//-------------------------------------------------
//  ADDRESS_MAP( ep64_io )
//-------------------------------------------------

void ep64_state::ep64_io(address_map &map)
{
	map(0x0000, 0xffff).m(m_dave, FUNC(dave_device::z80_io_map));
}


//-------------------------------------------------
//  ADDRESS_MAP( dave_64k_mem )
//-------------------------------------------------

void ep64_state::dave_64k_mem(address_map &map)
{
	map(0x000000, 0x007fff).rom().region(Z80_TAG, 0);
	//map(0x010000, 0x01ffff)      // mapped by the cartslot
	map(0x3f0000, 0x3fffff).m(m_nick, FUNC(nick_device::vram_map));
}


//-------------------------------------------------
//  ADDRESS_MAP( dave_128k_mem )
//-------------------------------------------------

void ep64_state::dave_128k_mem(address_map &map)
{
	dave_64k_mem(map);
	map(0x3e0000, 0x3effff).ram();
}


//-------------------------------------------------
//  ADDRESS_MAP( dave_io )
//-------------------------------------------------

void ep64_state::dave_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x80, 0x8f).m(m_nick, FUNC(nick_device::vio_map));
	map(0xb5, 0xb5).rw(FUNC(ep64_state::rd0_r), FUNC(ep64_state::wr0_w));
	map(0xb6, 0xb6).r(FUNC(ep64_state::rd1_r)).w("cent_data_out", FUNC(output_latch_device::write));
	map(0xb7, 0xb7).w(FUNC(ep64_state::wr2_w));
}



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( ep64 )
//-------------------------------------------------

static INPUT_PORTS_START( ep64 )
	PORT_START("Y0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Left SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("Y1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL)

	PORT_START("Y2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("TAB") PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t')

	PORT_START("Y3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR(U'£')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))

	PORT_START("Y4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("FUNCTION 4") PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("FUNCTION 8") PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("FUNCTION 3") PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("FUNCTION 6") PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("FUNCTION 5") PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("FUNCTION 7") PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("FUNCTION 2") PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("FUNCTION 1") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))

	PORT_START("Y5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('_')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('^') PORT_CHAR('~')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ERASE") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("STOP") PORT_CODE(KEYCODE_END)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("HOLD") PORT_CODE(KEYCODE_HOME)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER) PORT_CHAR('\r')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ALT") PORT_CODE(KEYCODE_LALT) PORT_CHAR(UCHAR_MAMEKEY(LALT))

	PORT_START("Y8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("DEL") PORT_CODE(KEYCODE_DEL) PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Right SHIFT") PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("INS") PORT_CODE(KEYCODE_INSERT) PORT_CHAR(UCHAR_MAMEKEY(INSERT))

	PORT_START("Y9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('@') PORT_CHAR('`')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_START( ep64 )
//-------------------------------------------------

void ep64_state::machine_start()
{
	if (m_cart->exists())
		m_dave->space(AS_PROGRAM).install_read_handler(0x010000, 0x01ffff, read8sm_delegate(*m_cart, FUNC(generic_slot_device::read_rom)));

	// state saving
	save_item(NAME(m_key));
	save_item(NAME(m_centronics_busy));
}


void ep64_state::machine_reset()
{
	m_dave->reset();
	m_nick->reset();

	wr0_w(0);
	subdevice<output_latch_device>("cent_data_out")->write(0);
	wr2_w(0);
}



//**************************************************************************
//  MACHINE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  machine_config( ep64 )
//-------------------------------------------------

void ep64_state::ep64(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, XTAL(8'000'000)/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &ep64_state::ep64_mem);
	m_maincpu->set_addrmap(AS_IO, &ep64_state::ep64_io);

	// video hardware
	screen_device& screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(ENTERPRISE_SCREEN_WIDTH, ENTERPRISE_SCREEN_HEIGHT);
	screen.set_visarea(0, ENTERPRISE_SCREEN_WIDTH-1, 0, ENTERPRISE_SCREEN_HEIGHT-1);
	screen.set_screen_update(NICK_TAG, FUNC(nick_device::screen_update));

	NICK(config, m_nick, XTAL(8'000'000), SCREEN_TAG);
	m_nick->virq_wr_callback().set(m_dave, FUNC(dave_device::int1_w));

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	DAVE(config, m_dave, XTAL(8'000'000));
	m_dave->set_addrmap(AS_PROGRAM, &ep64_state::dave_64k_mem);
	m_dave->set_addrmap(AS_IO, &ep64_state::dave_io);
	m_dave->irq_wr().set_inputline(Z80_TAG, INPUT_LINE_IRQ0);
	m_dave->add_route(0, "lspeaker", 0.25);
	m_dave->add_route(1, "rspeaker", 0.25);

	// devices
	EP64_EXPANSION_BUS_SLOT(config, m_exp, nullptr);
	m_exp->set_program_space(m_dave, AS_PROGRAM);
	m_exp->set_io_space(m_dave, AS_IO);
	m_exp->irq_wr().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_exp->nmi_wr().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_exp->wait_wr().set_inputline(m_maincpu, Z80_INPUT_LINE_WAIT);

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(FUNC(ep64_state::write_centronics_busy));
	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(cent_data_out);

	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->rxd_handler().set(m_dave, FUNC(dave_device::int2_w));

	CASSETTE(config, m_cassette1);
	m_cassette1->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette1->set_interface("ep64_cass");
	m_cassette1->add_route(ALL_OUTPUTS, "lspeaker", 0.05);

	CASSETTE(config, m_cassette2);
	m_cassette2->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette2->set_interface("ep64_cass");
	m_cassette2->add_route(ALL_OUTPUTS, "rspeaker", 0.05);

	// internal RAM
	RAM(config, m_ram).set_default_size("64K");

	// cartridge
	GENERIC_CARTSLOT(config, m_cart, generic_linear_slot, "ep64_cart", "bin,rom");

	// software lists
	SOFTWARE_LIST(config, "cart_list").set_original("ep64_cart");
	SOFTWARE_LIST(config, "cass_list").set_original("ep64_cass");
	SOFTWARE_LIST(config, "flop_list").set_original("ep64_flop");
}


//-------------------------------------------------
//  machine_config( ep128 )
//-------------------------------------------------

void ep64_state::ep128(machine_config &config)
{
	ep64(config);
	m_dave->set_addrmap(AS_PROGRAM, &ep64_state::dave_128k_mem);

	// internal RAM
	m_ram->set_default_size("128K");
}



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( ep64 )
//-------------------------------------------------

ROM_START( ep64 )
	ROM_REGION( 0x8000, Z80_TAG, 0 )
	ROM_LOAD( "9256ds-0038_enter05-23-a.u2", 0x0000, 0x8000, CRC(d421795f) SHA1(6033a0535136c40c47137e4d1cd9273c06d5fdff) )
ROM_END

#define rom_phc64   rom_ep64


//-------------------------------------------------
//  ROM( ep128 )
//-------------------------------------------------

ROM_START( ep128 )
	ROM_REGION( 0x8000, Z80_TAG, 0 )
	ROM_LOAD( "9256ds-0019_enter08-45-a.u2", 0x0000, 0x8000, CRC(982a3b44) SHA1(55315b20fecb4441a07ee4bc5dc7153f396e0a2e) )
ROM_END

} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY                                        FULLNAME                     FLAGS
COMP( 1985, ep64,  0,      0,      ep64,    ep64,  ep64_state, empty_init, "Intelligent Software / Enterprise Computers", "Enterprise Sixty Four",     MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
COMP( 1985, phc64, ep64,   0,      ep64,    ep64,  ep64_state, empty_init, "Intelligent Software / Hegener + Glaser",     "Mephisto PHC 64 (Germany)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
COMP( 1986, ep128, ep64,   0,      ep128,   ep64,  ep64_state, empty_init, "Intelligent Software / Enterprise Computers", "Enterprise One Two Eight",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
