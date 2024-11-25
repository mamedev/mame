// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

SBC6510 from Josip Perusanec

2009-12-18 Skeleton driver.
2012-08-12 Working [Robbbert]

CPU MOS 6510 (1MHz)
ROM 4KB
RAM 128KB
CIA 6526 - for interrupt gen and scan of keyboard
YM2149/AY-3-8910 - sound + HDD/CF IDE
GAL16V8 - address decoder
ATMEGA8 - gen. of PAL video signal (modified TellyMate)
keyboard of C64 computer used

Commands:
A - (unknown)
C - (unknown)
E - (unknown)
F - (unknown)
G - (unknown)
L - (unknown)
R - (unknown)
S - (unknown)

Some commands expect a filename enclosed in double quotes. If the quotes
are not there, it loops forever looking for them. Good example of bad
programming. There is no help and no error messages.

ToDo:

- The ATMEGA8 is a CPU with Timer, 4 IO ports, UART, ADC, Watchdog all
  built in. This enormously complex device needs to be emulated. It also
  contains some (4k?) RAM, of which certain addresses have special meaning.
  For example bytes 0 and 1 control the serial video stream of bits.

- When the system boots, there is a Y in the top corner. This is actually
  Esc-Y, which our terminal does not understand.

- IDE interface.

- Find out the proper way to use the monitor, there are no instructions
  and the slightest mistake can freeze the system.

- No software to test with.

- Natural keyboard & Paste do not work when shift is involved.


****************************************************************************/

#include "emu.h"
#include "cpu/avr8/avr8.h"
#include "cpu/m6502/m6510.h"
#include "machine/mos6526.h"
#include "machine/terminal.h"
#include "sound/ay8910.h"
#include "emupal.h"
#include "speaker.h"


namespace {

class sbc6510_state : public driver_device
{
public:
	sbc6510_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_videocpu(*this, "videocpu")
		, m_terminal(*this, "terminal")
		, m_keyboard(*this, "X%u", 0U)
	{ }

	void sbc6510(machine_config &config);

private:
	u8 a2_r();
	void a2_w(u8 data);
	u8 psg_a_r();
	u8 psg_b_r();
	void key_w(u8 data);
	u8 key_r();

	void main_mem_map(address_map &map) ATTR_COLD;
	void video_data_map(address_map &map) ATTR_COLD;
	void video_mem_map(address_map &map) ATTR_COLD;

	u8 m_key_row = 0U;
	u8 m_2 = 0U;
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	required_device<cpu_device> m_maincpu;
	required_device<atmega88_device> m_videocpu;
	required_device<generic_terminal_device> m_terminal;
	required_ioport_array<9> m_keyboard;
};


void sbc6510_state::main_mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0001).ram();
	map(0x0002, 0x0002).rw(FUNC(sbc6510_state::a2_r), FUNC(sbc6510_state::a2_w));
	map(0x0003, 0xdfff).ram();
	map(0xe000, 0xe00f).mirror(0x1f0).rw("cia6526", FUNC(mos6526_device::read), FUNC(mos6526_device::write));
	map(0xe800, 0xe800).mirror(0x1ff).w("ay8910", FUNC(ay8910_device::address_w));
	map(0xea00, 0xea00).mirror(0x1ff).rw("ay8910", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0xf000, 0xffff).rom().region("maincpu",0);
}

void sbc6510_state::video_mem_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
}

void sbc6510_state::video_data_map(address_map &map)
{
	map(0x0100, 0x04ff).ram();
}

/* Input ports */
static INPUT_PORTS_START( sbc6510 ) // cbm keyboard
	PORT_START( "X0" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Crsr Down Up")    PORT_CODE(KEYCODE_RALT) PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F3)        PORT_CHAR(UCHAR_MAMEKEY(F5)) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2)        PORT_CHAR(UCHAR_MAMEKEY(F3)) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F1)        PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F4)        PORT_CHAR(UCHAR_MAMEKEY(F7)) PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Crsr Right Left") PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Return")          PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Del  Inst")       PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8) PORT_CHAR(UCHAR_MAMEKEY(INSERT))

	PORT_START( "X1" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Shift (Left)")    PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E)         PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S)         PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z)         PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4)         PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A)         PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W)         PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3)         PORT_CHAR('3') PORT_CHAR('#')

	PORT_START( "X2" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X)         PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T)         PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F)         PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C)         PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6)         PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D)         PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)         PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5)         PORT_CHAR('5') PORT_CHAR('%')

	PORT_START( "X3" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V)         PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U)         PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H)         PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B)         PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8)         PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G)         PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y)         PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7)         PORT_CHAR('7')

	PORT_START( "X4" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N)         PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O)         PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K)         PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M)         PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0)         PORT_CHAR('0')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J)         PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I)         PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9)         PORT_CHAR('9') PORT_CHAR(')')

	PORT_START( "X5" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA)     PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('@')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON)     PORT_CHAR(':') PORT_CHAR('[')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP)      PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS)    PORT_CHAR('-')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L)         PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P)         PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS)     PORT_CHAR('+')

	PORT_START( "X6" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH)     PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("\xE2\x86\x91  Pi") PORT_CODE(KEYCODE_DEL) PORT_CHAR(0x2191) PORT_CHAR(0x03C0)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('=')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Shift (Right)")   PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Home  Clr")       PORT_CODE(KEYCODE_INSERT)     PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE)     PORT_CHAR(';') PORT_CHAR(']')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE)    PORT_CHAR('*')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH2)    PORT_CHAR(0xA3)

	PORT_START( "X7" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Stop Run")        PORT_CODE(KEYCODE_HOME)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q)         PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CBM")             PORT_CODE(KEYCODE_LALT)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE)     PORT_CHAR(' ')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2)         PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TAB)       PORT_CHAR(9)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("\xE2\x86\x90")    PORT_CODE(KEYCODE_TILDE)   PORT_CHAR(0x2190) // newline
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1)         PORT_CHAR('1') PORT_CHAR('!')

	PORT_START( "X8" )  /* unused */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Restore")         PORT_CODE(KEYCODE_PRTSCR)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Shift Lock (switch)") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


u8 sbc6510_state::a2_r()
{
	return m_2;
}

void sbc6510_state::a2_w(u8 data)
{
	m_2 = data;
	m_terminal->write(data);
}

void sbc6510_state::machine_start()
{
	save_item(NAME(m_key_row));
	save_item(NAME(m_2));
}


void sbc6510_state::machine_reset()
{
}

u8 sbc6510_state::psg_a_r()
{
	return 0xff;
}

u8 sbc6510_state::psg_b_r()
{
	return 0x7f;
}

u8 sbc6510_state::key_r()
{
	u8 i, data=0;

	for (i = 0; i < 8; i++)
		if (!BIT(m_key_row, i))
			data |= m_keyboard[i]->read();

	return ~data;
}

void sbc6510_state::key_w(u8 data)
{
	m_key_row = data;
}

static const gfx_layout charset_8x16 =
{
	8, 9,
	256,
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ 0*1024*2, 1*1024*2, 2*1024*2, 3*1024*2, 4*1024*2, 5*1024*2, 6*1024*2, 7*1024*2, 8*1024*2 },
	8
};


static GFXDECODE_START( gfx_sbc6510 )
	GFXDECODE_ENTRY( "videocpu", 0x1500, charset_8x16, 0, 128 )
GFXDECODE_END


void sbc6510_state::sbc6510(machine_config &config)
{
	/* basic machine hardware */
	M6510(config, m_maincpu, XTAL(1'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &sbc6510_state::main_mem_map);

	ATMEGA88(config, m_videocpu, XTAL(16'000'000)); // Video CPU trips SLEEP opcode, needs to be emulated
	m_videocpu->set_addrmap(AS_PROGRAM, &sbc6510_state::video_mem_map);
	m_videocpu->set_addrmap(AS_DATA, &sbc6510_state::video_data_map);
	m_videocpu->set_eeprom_tag("eeprom");

	PALETTE(config, "palette", palette_device::MONOCHROME); // for F4 displayer only
	GFXDECODE(config, "gfxdecode", "palette", gfx_sbc6510);

	/* video hardware */
	GENERIC_TERMINAL(config, m_terminal, 0);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	ay8910_device &ay8910(AY8910(config, "ay8910", XTAL(1'000'000)));
	// Ports A and B connect to the IDE socket
	ay8910.port_a_read_callback().set(FUNC(sbc6510_state::psg_a_r));
	ay8910.port_b_read_callback().set(FUNC(sbc6510_state::psg_b_r));
	ay8910.add_route(ALL_OUTPUTS, "mono", 1.00);

	mos6526_device &cia(MOS6526(config, "cia6526", XTAL(1'000'000)));
	cia.set_tod_clock(50);
	cia.irq_wr_callback().set_inputline("maincpu", M6510_IRQ_LINE);
	cia.pa_wr_callback().set(FUNC(sbc6510_state::key_w));
	cia.pb_rd_callback().set(FUNC(sbc6510_state::key_r));
}

/* ROM definition */
ROM_START( sbc6510 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "sbc6510.rom", 0x0000, 0x1000, CRC(e13a5e62) SHA1(1e7482e9b98b39d0cc456254fbe8fd0981e9377e))

	ROM_REGION( 0x2000, "videocpu", 0 ) // ATMEGA8 at 16MHz
	ROM_LOAD( "video.bin",   0x0000, 0x2000, CRC(809f31ce) SHA1(4639de5f7b8f6c036d74f217ba85e7e897039094))

	ROM_REGION( 0x0200, "gal", ROMREGION_ERASEFF )
	ROM_LOAD( "sbc6510.gal", 0x0000, 0x0117, CRC(f78f9927) SHA1(b951163958f5722032826d0d17a07c81dbd5f68e))

	ROM_REGION( 0x2000, "eeprom", ROMREGION_ERASEFF )
ROM_END

} // anonymous namespace


/* Driver */

/*    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY            FULLNAME   FLAGS */
COMP( 2009, sbc6510, 0,      0,      sbc6510, sbc6510, sbc6510_state, empty_init, "Josip Perusanec", "SBC6510", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
