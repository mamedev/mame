// license:BSD-3-Clause
// copyright-holders:Curt Coder, Phill Harvey-Smith
/*

    Sinclair QL

*/

/*

    TODO:

    - microdrive
    - ZX8301 memory access slowdown
    - use resnet.h to create palette
    - Tyche bios is broken
    - several disk interfaces (720K/1.44MB/3.2MB)
    - Gold Card (68000 @ 16MHz, 2MB RAM)
    - Super Gold Card (68020 @ 24MHz, 4MB RAM)
    - QLToolkit II ROM
    - GST 68kOS ROM
    - natural keyboard SHIFT does not work, it causes characters to be skipped altogether
    - get modified Danish version to boot (e.g. MD_DESEL was patched to jump to 0x1cd5e)
    - ICL One Per Desk / British Telecom Merlin Tonto / Telecom Australia Computerphone / Mega OPD (US)
    - OPD: 68008P8 @ 7.5MHz, ZX8301 @ 12MHz, CDP65C51E1 @ 1.8432MHz, 128K RAM, 2K NVRAM, 128K ROM (4x32), 16K speech ROM, ADM7910 modem, TMS5220 speech, 2x100K microdrives, RTC 1/1/1970, 1xRS-423
    - Sandy Q-XT 640 (original motherboard, Super QBoard 512KB, Centronics, Super Toolkit II, FDC, 1 or 2 3.5" 1MB drives, 3 expansion slots)
    - CST Thor PC 1F (720K 3.5")
    - CST Thor PC 2F (2x 720k 3.5")
    - CST Thor PC 2FW (20MB Winchester + 720K 3.5")
    - CST Thor 20 (68020)
    - CST Thor 21 (68020+68881)
    - CST Thor XVI CF    (Workstation, 68000 @ 8 MHz, M6802, 68682 DUART)
    - CST Thor XVI IF    (Single Floppy)
    - CST Thor XVI FF    (Dual Floppy)
    - CST Thor XVI W20F  (20MB Winchester, 1 Floppy)
    - CST Thor XVI W20FF (20MB Winchester, 2 Floppies)
    - CST Thor XVI W40F  (40MB Winchester, 1 Floppy)
    - CST Thor XVI W40FF (40MB Winchester, 2 Floppies)


    2011-02-22, P.Harvey-Smith,
        Implemented Miracle Trump Card disk and ram interface.

        The trump card has some interesting memory mapping and re-mapping during initialisation which goes like this

        On reset the card ram is mapped into the entire $40000-$fffff area, this is the official 512K expansion ram area
        plus the area set aside for addon card roms. The trump card onboard rom is also mapped in at $10000-$17fff.

        The main bios then performs a series of loops that initialise and size the ram, this will find either
        0K, 256K, 512K or 768K of expansion ram depending on how many banks of ram are populated.

        After the ram test, the main bios looks for the signature longword of a cartridge rom at $0c000, this read
        triggers the Trump Card to also map it's rom in at $c0000-$c7fff, the first addon rom area. If a cartridge
        rom is found it is first initialised.

        The main bios goes on to search the addon card rom area from $c0000 by searching for the signature word for
        an addon rom at $c0000 and upwards in 16K steps (there is a bug in JM and before that means they only find
        the first addon, but this does not matter).

        The main bios finds the trump card addon rom at $c0000, and calls it's initialisation routine at $c011e, this
        contains a jump instruction to jump to $10124 (in the lower mapped copy of the trumpcard rom).

        The first memory access in the $10000-$17fff range triggers the trumcard to page out the rom at $c0000, and
        page the ram back in. Once it arrives at this state the ram will remain paged in until the next reset.

        I have implemented this with a pair of read handlers at $0c000-$0cfff and $10000-$17fff which do the apropreate
        re-map of the upper area and then remove themselves from the address map and return it to simple rom handlers.

        On the Trump card the above is implemented with a pair of 16L8 PAL chips, the logic of these was decoded with
        the assistance of Malcolm Lear, without who's help working all this out would have been a much harder process.

*/

#include "emu.h"

#include "bus/ql/exp.h"
#include "bus/ql/rom.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68008.h"
#include "cpu/mcs48/mcs48.h"
#include "imagedev/microdrv.h"
#include "qimi.h"
#include "machine/ram.h"
#include "zx8302.h"
#include "sound/spkrdev.h"
#include "zx8301.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "utf8.h"


namespace {

#define SCREEN_TAG  "screen"

#define M68008_TAG  "ic18"
#define I8749_TAG   "ic24"
#define I8051_TAG   "i8051"
#define ZX8301_TAG  "ic22"
#define ZX8302_TAG  "ic23"
#define RS232_A_TAG "ser1"
#define RS232_B_TAG "ser2"
#define QIMI_TAG    "qimi"

#define X1 XTAL(15'000'000)
#define X2 XTAL(32'768)
#define X3 XTAL_4_436MHz
#define X4 XTAL(11'000'000)

class ql_state : public driver_device
{
public:
	ql_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, M68008_TAG),
		m_ipc(*this, I8749_TAG),
		m_zx8301(*this, ZX8301_TAG),
		m_zx8302(*this, ZX8302_TAG),
		m_speaker(*this, "speaker"),
		m_mdv1(*this, MDV_1),
		m_mdv2(*this, MDV_2),
		m_ser1(*this, RS232_A_TAG),
		m_ser2(*this, RS232_B_TAG),
		m_ram(*this, RAM_TAG),
		m_exp(*this, "exp"),
		m_cart(*this, "rom"),
		m_qimi(*this, QIMI_TAG),
		m_rom(*this, M68008_TAG),
		m_y(*this, "Y%u", 0),
		m_joy(*this, "JOY%u", 0),
		m_config(*this, "config"),
		m_extintl(CLEAR_LINE),
		m_keylatch(0),
		m_ipl(0),
		m_comdata_to_ipc(0),
		m_baudx4(0),
		m_qimi_enabled(false),
		m_qimi_extint(CLEAR_LINE)
	{ }

	void ql_ntsc(machine_config &config);
	void opd(machine_config &config);
	void ql(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<i8749_device> m_ipc;
	required_device<zx8301_device> m_zx8301;
	required_device<zx8302_device> m_zx8302;
	required_device<speaker_sound_device> m_speaker;
	required_device<microdrive_image_device> m_mdv1;
	required_device<microdrive_image_device> m_mdv2;
	required_device<rs232_port_device> m_ser1;
	required_device<rs232_port_device> m_ser2;
	required_device<ram_device> m_ram;
	required_device<ql_expansion_slot_device> m_exp;
	required_device<ql_rom_cartridge_slot_device> m_cart;
	optional_device<qimi_device> m_qimi;
	required_memory_region m_rom;
	required_ioport_array<8> m_y;
	required_ioport_array<2> m_joy;
	required_ioport m_config;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	void ipc_w(uint8_t data);
	void ipc_port1_w(uint8_t data);
	void ipc_port2_w(uint8_t data);
	uint8_t ipc_port2_r();
	int ipc_t1_r();
	uint8_t ipc_bus_r();
	void ql_baudx4_w(int state);
	void ql_comdata_w(int state);
	void zx8302_mdselck_w(int state);
	void zx8302_mdrdw_w(int state);
	void zx8302_erase_w(int state);
	void zx8302_raw1_w(int state);
	int zx8302_raw1_r();
	void zx8302_raw2_w(int state);
	int zx8302_raw2_r();
	void exp_extintl_w(int state);
	void qimi_extintl_w(int state);

	void update_interrupt();

	int m_extintl;

	// IPC
	uint8_t m_keylatch;
	int m_ipl;
	int m_comdata_to_ipc;
	int m_baudx4;

	// QIMI
	bool m_qimi_enabled;
	int m_qimi_extint;

	void ipc_io(address_map &map) ATTR_COLD;
	void ql_mem(address_map &map) ATTR_COLD;
};



//**************************************************************************
//  ADDRESS DECODING
//**************************************************************************

//-------------------------------------------------
//  read -
//-------------------------------------------------

uint8_t ql_state::read(offs_t offset)
{
	uint8_t data = 0;
	int cart_romoeh = 0;
	int exp_romoeh = 0;

	if (offset < 0xc000)
	{
		data = m_rom->base()[offset];
	}
	if (offset >= 0xc000 && offset < 0x10000)
	{
		cart_romoeh = 1;
	}
	if (offset >= 0x18000 && offset <= 0x18003)
	{
		data = m_zx8302->rtc_r(offset & 0x03);
	}
	if (offset == 0x18020)
	{
		data = m_zx8302->status_r();
	}
	if (offset == 0x18021)
	{
		data = m_zx8302->irq_status_r();
	}
	if (offset >= 0x18022 && offset <= 0x18023)
	{
		data = m_zx8302->mdv_track_r();
	}
	if (offset >= 0x20000 && offset < 0x40000)
	{
		data = m_zx8301->data_r(offset & 0x1ffff);
	}
	if (offset >= 0xc0000)
	{
		exp_romoeh = 1;
	}
	if (m_qimi_enabled)
	{
		data = m_qimi->read(offset, data);
	}

	m_cart->romoeh_w(cart_romoeh);
	data = m_cart->read(offset & 0x7fff, data);
	m_cart->romoeh_w(0);

	m_exp->romoeh_w(exp_romoeh);
	data = m_exp->read(offset, data);
	m_exp->romoeh_w(0);

	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void ql_state::write(offs_t offset, uint8_t data)
{
	if (offset >= 0x18000 && offset <= 0x18001)
	{
		m_zx8302->rtc_w(data);
	}
	if (offset == 0x18002)
	{
		m_zx8302->control_w(data);
	}
	if (offset == 0x18003)
	{
		m_zx8302->ipc_command_w(data);
	}
	if (offset == 0x18020)
	{
		m_zx8302->mdv_control_w(data);
	}
	if (offset == 0x18021)
	{
		m_zx8302->irq_acknowledge_w(data);
	}
	if (offset == 0x18022)
	{
		m_zx8302->data_w(data);
	}
	if (offset == 0x18063)
	{
		m_zx8301->control_w(data);
	}
	if (offset >= 0x20000 && offset < 0x40000)
	{
			m_zx8301->data_w(offset & 0x1ffff, data);
	}
	if (m_qimi_enabled)
	{
		m_qimi->write(offset, data);
	}

	m_cart->romoeh_w(0);
	m_cart->write(offset & 0x7fff, data);

	m_exp->romoeh_w(0);
	m_exp->write(offset, data);
}



//**************************************************************************
//  INTELLIGENT PERIPHERAL CONTROLLER
//**************************************************************************

//-------------------------------------------------
//  ipc_w -
//-------------------------------------------------

void ql_state::ipc_w(uint8_t data)
{
	m_zx8302->comctl_w(0);
	m_zx8302->comctl_w(1);
}


//-------------------------------------------------
//  ipc_port1_w -
//-------------------------------------------------

void ql_state::ipc_port1_w(uint8_t data)
{
	/*

	    bit     description

	    0       Keyboard column output (KBO0)
	    1       Keyboard column output (KBO1)
	    2       Keyboard column output (KBO2)
	    3       Keyboard column output (KBO3)
	    4       Keyboard column output (KBO4)
	    5       Keyboard column output (KBO5)
	    6       Keyboard column output (KBO6)
	    7       Keyboard column output (KBO7)

	*/

	m_keylatch = data;
}


//-------------------------------------------------
//  ipc_port2_r -
//-------------------------------------------------

uint8_t ql_state::ipc_port2_r()
{
	/*

	    bit     description

	    0       Serial data input (SER2 RxD, SER1 TxD)
	    1
	    2
	    3
	    4
	    5
	    6
	    7       ZX8302 serial link input/output (COMDATA)

	*/

	uint8_t data = 0;

	// SER2 serial data input
	data |= m_ser2->rxd_r();

	// COMDATA
	data |= m_comdata_to_ipc << 7;

	return data;
}


//-------------------------------------------------
//  ipc_port2_w -
//-------------------------------------------------

void ql_state::ipc_port2_w(uint8_t data)
{
	/*

	    bit     description

	    0
	    1       Speaker output
	    2       Interrupt output (IPL0-2)
	    3       Interrupt output (IPL1)
	    4       Serial Clear-to-Send output (SER1 CTS)
	    5       Serial Data Terminal Ready output (SER2 DTR)
	    6
	    7       ZX8302 serial link input/output (COMDATA)

	*/

	// speaker
	m_speaker->level_w(BIT(data, 1));

	// interrupts
	int ipl = (BIT(data, 2) << 1) | BIT(data, 3);

	if (ipl != m_ipl)
	{
		switch (ipl)
		{
		case 0: m_maincpu->set_input_line(M68K_IRQ_7, ASSERT_LINE); break;
		case 1: m_maincpu->set_input_line(M68K_IRQ_5, ASSERT_LINE); break; // CTRL-ALT-7 pressed
		case 2: m_maincpu->set_input_line(M68K_IRQ_2, ASSERT_LINE); break;
		case 3: m_maincpu->set_input_line(M68K_IRQ_7, CLEAR_LINE);  break;
		}

		m_ipl = ipl;
	}

	// TODO SER1 clear to send

	// SER2 data terminal ready
	m_ser2->write_dtr(!BIT(data, 5));

	// COMDATA
	m_zx8302->comdata_w(BIT(data, 7));
}


//-------------------------------------------------
//  ipc_t1_r -
//-------------------------------------------------

int ql_state::ipc_t1_r()
{
	return m_baudx4;
}


//-------------------------------------------------
//  ipc_bus_r -
//-------------------------------------------------

uint8_t ql_state::ipc_bus_r()
{
	/*

	    bit     description

	    0       Keyboard row input (KBI0)
	    1       Keyboard row input (KBI1)
	    2       Keyboard row input (KBI2)
	    3       Keyboard row input (KBI3)
	    4       Keyboard row input (KBI4)
	    5       Keyboard row input (KBI5)
	    6       Keyboard row input (KBI6)
	    7       Keyboard row input (KBI7)

	*/

	uint8_t data = 0;

	if (BIT(m_keylatch, 0)) data |= m_y[0]->read() | m_joy[0]->read();
	if (BIT(m_keylatch, 1)) data |= m_y[1]->read() | m_joy[1]->read();
	if (BIT(m_keylatch, 2)) data |= m_y[2]->read();
	if (BIT(m_keylatch, 3)) data |= m_y[3]->read();
	if (BIT(m_keylatch, 4)) data |= m_y[4]->read();
	if (BIT(m_keylatch, 5)) data |= m_y[5]->read();
	if (BIT(m_keylatch, 6)) data |= m_y[6]->read();
	if (BIT(m_keylatch, 7)) data |= m_y[7]->read();

	return data;
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( ql_mem )
//-------------------------------------------------

void ql_state::ql_mem(address_map &map)
{
	map(0x000000, 0x0fffff).rw(FUNC(ql_state::read), FUNC(ql_state::write));
}


//-------------------------------------------------
//  ADDRESS_MAP( ipc_io )
//-------------------------------------------------

void ql_state::ipc_io(address_map &map)
{
	map(0x00, 0x7f).w(FUNC(ql_state::ipc_w));
	map(0x27, 0x28).nopr(); // IPC reads these to set P0 (bus) to Hi-Z mode
}



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( ql )
//-------------------------------------------------

static INPUT_PORTS_START( ql )
	PORT_START("Y0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4)) PORT_NAME("F4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_NAME("F1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2)) PORT_NAME("F2")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3)) PORT_NAME("F3")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5)) PORT_NAME("F5")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')

	PORT_START("Y1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("ESC  @") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC)) PORT_CHAR(0x1B) PORT_CHAR('@')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(UTF8_DOWN) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))

	PORT_START("Y2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TILDE) PORT_CHAR(U'£') PORT_CHAR('~')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_CHAR('\"')

	PORT_START("Y3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CAPS LOCK") PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')

	PORT_START("Y4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')

	PORT_START("Y5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("TABULATE") PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')

	PORT_START("Y6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')

	PORT_START("Y7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("ALT") PORT_CODE(KEYCODE_LALT) PORT_CHAR(UCHAR_MAMEKEY(LALT)) PORT_CODE(KEYCODE_RALT)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')

	PORT_START("JOY0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )   PORT_PLAYER(1) PORT_8WAY PORT_CODE(KEYCODE_F4)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_8WAY PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_8WAY PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )PORT_PLAYER(1) PORT_8WAY PORT_CODE(KEYCODE_F3)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 )       PORT_PLAYER(1) PORT_CODE(KEYCODE_F5)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("JOY1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_8WAY PORT_CODE(KEYCODE_LEFT)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )   PORT_PLAYER(2) PORT_8WAY PORT_CODE(KEYCODE_UP)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )PORT_PLAYER(2) PORT_8WAY PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 )       PORT_PLAYER(2) PORT_CODE(KEYCODE_SPACE)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_8WAY PORT_CODE(KEYCODE_DOWN)

	PORT_START("config")
	PORT_CONFNAME( 0x01, 0x00, "QL Internal Mouse Interface (QIMI)")
	PORT_CONFSETTING( 0x00, DEF_STR( Off ) )
	PORT_CONFSETTING( 0x01, DEF_STR( On ) )
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( ql_es )
//-------------------------------------------------

static INPUT_PORTS_START( ql_es )
	PORT_INCLUDE(ql)

	PORT_MODIFY("Y1")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR(']') PORT_CHAR(U'Ü',U'ü')

	PORT_MODIFY("Y2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('`') PORT_CHAR('^')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('!')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('[') PORT_CHAR(U'Ç',U'ç')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(';') PORT_CHAR(':')

	PORT_MODIFY("Y3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(U'Ñ',U'ñ')

	PORT_MODIFY("Y4")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR(U'¡')

	PORT_MODIFY("Y6")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR(U'¿')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('/')

	PORT_MODIFY("Y7")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('<') PORT_CHAR('>')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('?')
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( ql_de )
//-------------------------------------------------

static INPUT_PORTS_START( ql_de )
	PORT_INCLUDE(ql)

	PORT_MODIFY("Y0")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('/')

	PORT_MODIFY("Y1")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('<') PORT_CHAR('>')

	PORT_MODIFY("Y2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('+') PORT_CHAR('*')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR(':')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('\\') PORT_CHAR('^')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(U'Ä',U'ä')

	PORT_MODIFY("Y3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR(U'Ü',U'ü')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('#') PORT_CHAR('\'')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(U'Ö',U'ö')

	PORT_MODIFY("Y4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR(U'£')

	PORT_MODIFY("Y5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR(U'ß') PORT_CHAR('?')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')

	PORT_MODIFY("Y6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('=')

	PORT_MODIFY("Y7")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR(';')
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( ql_it )
//-------------------------------------------------

static INPUT_PORTS_START( ql_it )
	PORT_INCLUDE(ql)

	PORT_MODIFY("Y0")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('(') PORT_CHAR('5')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('\'') PORT_CHAR('4')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR(U'è') PORT_CHAR('7')

	PORT_MODIFY("Y1")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('<') PORT_CHAR('>')

	PORT_MODIFY("Y2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('$') PORT_CHAR('&')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR(':') PORT_CHAR('/')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('*') PORT_CHAR(U'§')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('?')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(U'ù') PORT_CHAR('%')

	PORT_MODIFY("Y3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR(U'ì') PORT_CHAR('=')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('-') PORT_CHAR('+')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')

	PORT_MODIFY("Y4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('"') PORT_CHAR('3')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('#') PORT_CHAR('1')

	PORT_MODIFY("Y5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR(U'ç') PORT_CHAR('9')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR(')') PORT_CHAR('\\')

	PORT_MODIFY("Y6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('^') PORT_CHAR('8')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR(U'é') PORT_CHAR('2')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('_') PORT_CHAR('6')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR(U'à') PORT_CHAR('0')

	PORT_MODIFY("Y7")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR(U'ò') PORT_CHAR('!')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('.')
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( ql_fr )
//-------------------------------------------------

static INPUT_PORTS_START( ql_fr )
	PORT_INCLUDE(ql)
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( ql_se )
//-------------------------------------------------

static INPUT_PORTS_START( ql_se )
	PORT_INCLUDE(ql)
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( ql_dk )
//-------------------------------------------------

static INPUT_PORTS_START( ql_dk )
	PORT_INCLUDE(ql)
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  ZX8302_INTERFACE( ql_zx8302_intf )
//-------------------------------------------------

void ql_state::ql_baudx4_w(int state)
{
	m_baudx4 = state;
}

// CPU to IPC
void ql_state::ql_comdata_w(int state)
{
	m_comdata_to_ipc = state;
}

void ql_state::zx8302_mdselck_w(int state)
{
	m_mdv2->clk_w(state);
	m_mdv1->clk_w(state);
}

void ql_state::zx8302_mdrdw_w(int state)
{
	m_mdv1->read_write_w(state);
	m_mdv2->read_write_w(state);
}

void ql_state::zx8302_erase_w(int state)
{
	m_mdv1->erase_w(state);
	m_mdv2->erase_w(state);
}

void ql_state::zx8302_raw1_w(int state)
{
	m_mdv1->data1_w(state);
	m_mdv2->data1_w(state);
}

int ql_state::zx8302_raw1_r()
{
	return m_mdv1->data1_r() | m_mdv2->data1_r();
}

void ql_state::zx8302_raw2_w(int state)
{
	m_mdv1->data2_w(state);
	m_mdv2->data2_w(state);
}

int ql_state::zx8302_raw2_r()
{
	return m_mdv1->data2_r() | m_mdv2->data2_r();
}

void ql_state::update_interrupt()
{
	m_zx8302->extint_w(m_extintl || m_qimi_extint);
}

void ql_state::exp_extintl_w(int state)
{
	m_extintl = state;
	update_interrupt();
}

void ql_state::qimi_extintl_w(int state)
{
	if (m_qimi_enabled)
	{
		m_qimi_extint = state;
		update_interrupt();
	}
}



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

void ql_state::machine_start()
{
	// register for state saving
	save_item(NAME(m_keylatch));
	save_item(NAME(m_ipl));
	save_item(NAME(m_comdata_to_ipc));
	save_item(NAME(m_baudx4));
}

void ql_state::machine_reset()
{
	// QIMI
	m_qimi_enabled = (m_config->read() & 0x01) ? true : false;
}



//**************************************************************************
//  MACHINE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  machine_config( ql )
//-------------------------------------------------

void ql_state::ql(machine_config &config)
{
	// basic machine hardware
	M68008(config, m_maincpu, X1/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &ql_state::ql_mem);

	I8749(config, m_ipc, X4);
	m_ipc->set_addrmap(AS_IO, &ql_state::ipc_io);
	m_ipc->p1_out_cb().set(FUNC(ql_state::ipc_port1_w));
	m_ipc->p2_in_cb().set(FUNC(ql_state::ipc_port2_r));
	m_ipc->p2_out_cb().set(FUNC(ql_state::ipc_port2_w));
	m_ipc->t1_in_cb().set(FUNC(ql_state::ipc_t1_r));
	m_ipc->bus_in_cb().set(FUNC(ql_state::ipc_bus_r));

	// video hardware
	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50.08);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_screen_update(ZX8301_TAG, FUNC(zx8301_device::screen_update));
	screen.set_size(960, 312);
	screen.set_visarea(0, 512-1, 0, 256-1);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);

	// devices
	ZX8301(config, m_zx8301, X1, m_maincpu);
	m_zx8301->vsync_wr_callback().set(m_zx8302, FUNC(zx8302_device::vsync_w));
	m_zx8301->set_screen(SCREEN_TAG);

	ZX8302(config, m_zx8302, X1);
	m_zx8302->set_rtc_clock(X2);
	m_zx8302->out_ipl1l_callback().set_inputline(m_maincpu, M68K_IRQ_2);
	m_zx8302->out_baudx4_callback().set(FUNC(ql_state::ql_baudx4_w));
	m_zx8302->out_comdata_callback().set(FUNC(ql_state::ql_comdata_w));
	// TXD1
	m_zx8302->out_txd2_callback().set(RS232_B_TAG, FUNC(rs232_port_device::write_txd));
	// NETOUT
	m_zx8302->out_mdselck_callback().set(FUNC(ql_state::zx8302_mdselck_w));
	m_zx8302->out_mdseld_callback().set(m_mdv1, FUNC(microdrive_image_device::comms_in_w));
	m_zx8302->out_mdrdw_callback().set(FUNC(ql_state::zx8302_mdrdw_w));
	m_zx8302->out_erase_callback().set(FUNC(ql_state::zx8302_erase_w));
	m_zx8302->out_raw1_callback().set(FUNC(ql_state::zx8302_raw1_w));
	m_zx8302->in_raw1_callback().set(FUNC(ql_state::zx8302_raw1_r));
	m_zx8302->out_raw2_callback().set(FUNC(ql_state::zx8302_raw2_w));
	m_zx8302->in_raw2_callback().set(FUNC(ql_state::zx8302_raw2_r));

	MICRODRIVE(config, m_mdv1);
	m_mdv1->comms_out_wr_callback().set(m_mdv2, FUNC(microdrive_image_device::comms_in_w));
	MICRODRIVE(config, m_mdv2);

	RS232_PORT(config, m_ser1, default_rs232_devices, nullptr); // wired as DCE
	RS232_PORT(config, m_ser2, default_rs232_devices, nullptr); // wired as DTE
	m_ser2->cts_handler().set(m_zx8302, FUNC(zx8302_device::write_cts2));

	QL_EXPANSION_SLOT(config, m_exp, 0, ql_expansion_cards, nullptr); // FIXME: what's the clock on the slot?
	//m_exp->ipl0l_wr_callback().set();
	//m_exp->ipl1l_wr_callback().set();(
	//m_exp->berrl_wr_callback().set();
	m_exp->extintl_wr_callback().set(FUNC(ql_state::exp_extintl_w));

	QL_ROM_CARTRIDGE_SLOT(config, m_cart, ql_rom_cartridge_cards, nullptr);

	QIMI(config, m_qimi, 0);
	m_qimi->extint_wr_callback().set(FUNC(ql_state::qimi_extintl_w));

	// software lists
	SOFTWARE_LIST(config, "cart_list").set_original("ql_cart");
	SOFTWARE_LIST(config, "cass_list").set_original("ql_cass");
	SOFTWARE_LIST(config, "flop_list").set_original("ql_flop");

	// internal ram
	RAM(config, m_ram).set_default_size("128K");
}


//-------------------------------------------------
//  machine_config( ql_ntsc )
//-------------------------------------------------

void ql_state::ql_ntsc(machine_config &config)
{
	ql(config);
	// video hardware
	screen_device &screen(*subdevice<screen_device>(SCREEN_TAG));
	screen.set_refresh_hz(60);
	screen.set_size(960, 262);
	screen.set_visarea(0, 512-1, 0, 256-1);
	}


//-------------------------------------------------
//  machine_config( opd )
//-------------------------------------------------

void ql_state::opd(machine_config &config)
{
	ql(config);
	// internal ram
	m_ram->set_default_size("128K").set_extra_options("256K");
}


/*
//-------------------------------------------------
//  machine_config( megaopd )
//-------------------------------------------------

void ql_state::megaopd(machine_config &config)
{
    ql(config);
    // internal ram
    m_ram->set_default_size("256K");
}
*/


//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( ql )
//-------------------------------------------------

ROM_START( ql )
	ROM_REGION( 0x10000, M68008_TAG, 0 )
	ROM_DEFAULT_BIOS("js")
	ROM_SYSTEM_BIOS( 0, "fb", "v1.00 (FB)" )
	ROMX_LOAD( "fb.ic33", 0x0000, 0x8000, NO_DUMP, ROM_BIOS(0) )
	ROMX_LOAD( "fb.ic34", 0x8000, 0x4000, NO_DUMP, ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "pm", "v1.01 (PM)" )
	ROMX_LOAD( "pm.ic33", 0x0000, 0x8000, NO_DUMP, ROM_BIOS(1) )
	ROMX_LOAD( "pm.ic34", 0x8000, 0x4000, NO_DUMP, ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "ah", "v1.02 (AH)" )
	ROMX_LOAD( "ah.ic33.1", 0x0000, 0x4000, BAD_DUMP CRC(a9b4d2df) SHA1(142d6f01a9621aff5e0ad678bd3cbf5cde0db801), ROM_BIOS(2) )
	ROMX_LOAD( "ah.ic33.2", 0x4000, 0x4000, BAD_DUMP CRC(36488e4e) SHA1(ff6f597b30ea03ce480a3d6728fd1d858da34d6a), ROM_BIOS(2) )
	ROMX_LOAD( "ah.ic34",   0x8000, 0x4000, BAD_DUMP CRC(61259d4c) SHA1(bdd10d111e7ba488551a27c8d3b2743917ff1307), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "jm", "v1.03 (JM)" )
	ROMX_LOAD( "ql.jm 0000.ic33", 0x0000, 0x8000, CRC(1f8e840a) SHA1(7929e716dfe88318bbe99e34f47d039957fe3cc0), ROM_BIOS(3) )
	ROMX_LOAD( "ql.jm 8000.ic34", 0x8000, 0x4000, CRC(9168a2e9) SHA1(1e7c47a59fc40bd96dfefc2f4d86827c15f0199e), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 4, "tb", "v1.0? (TB)" )
	ROMX_LOAD( "tb.ic33", 0x0000, 0x8000, BAD_DUMP CRC(1c86d688) SHA1(7df8028e6671afc4ebd5f65bf6c2d6019181f239), ROM_BIOS(4) )
	ROMX_LOAD( "tb.ic34", 0x8000, 0x4000, BAD_DUMP CRC(de7f9669) SHA1(9d6bc0b794541a4cec2203256ae92c7e68d1011d), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS( 5, "js", "v1.10 (JS)" )
	ROMX_LOAD( "ql.js 0000.ic33", 0x0000, 0x8000, CRC(1bbad3b8) SHA1(59fd4372771a630967ee102760f4652904d7d5fa), ROM_BIOS(5) )
	ROMX_LOAD( "ql.js 8000.ic34", 0x8000, 0x4000, CRC(c970800e) SHA1(b8c9203026a7de6a44bd0942ec9343e8b222cb41), ROM_BIOS(5) )
	ROM_SYSTEM_BIOS( 6, "tyche", "v2.05 (Tyche)" )
	ROMX_LOAD( "tyche.rom", 0x0000, 0x010000, CRC(8724b495) SHA1(5f33a1bc3f23fd09c31844b65bc3aca7616f180a), ROM_BIOS(6) )
	ROM_SYSTEM_BIOS( 7, "min189", "Minerva v1.89" )
	ROMX_LOAD( "minerva.rom", 0x0000, 0x00c000, BAD_DUMP CRC(930befe3) SHA1(84a99c4df13b97f90baf1ec8cb6c2e52e3e1bb4d), ROM_BIOS(7) )

	ROM_REGION( 0x800, I8749_TAG, 0 )
	ROM_LOAD( "ipc8049.ic24", 0x000, 0x800, CRC(6a0d1f20) SHA1(fcb1c97ee7c66e5b6d8fbb57c06fd2f6509f2e1b) )

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "hal16l8.ic38", 0x0000, 0x0400, NO_DUMP )

	ROM_REGION( 0x4000, "printer", 0 ) // original Sinclair QL printer (based on Seikosha SP-1000 / SP-800QL, with custom QL font)
	ROM_LOAD( "bql010-sqpp", 0x0000, 0x4000, CRC(07834797) SHA1(ba94bdad2303a263008b6ea744669a19938d9998) )
ROM_END


//-------------------------------------------------
//  ROM( ql_us )
//-------------------------------------------------

ROM_START( ql_us )
	ROM_REGION( 0xc000, M68008_TAG, 0 )
	ROM_LOAD( "jsu.ic33", 0x0000, 0x8000, BAD_DUMP CRC(e397f49f) SHA1(c06f92eabaf3e6dd298c51cb7f7535d8ef0ef9c5) )
	ROM_LOAD( "jsu.ic34", 0x8000, 0x4000, BAD_DUMP CRC(3debbacc) SHA1(9fbc3e42ec463fa42f9c535d63780ff53a9313ec) )

	ROM_REGION( 0x800, I8749_TAG, 0 )
	ROM_LOAD( "ipc8049.ic24", 0x000, 0x800, CRC(6a0d1f20) SHA1(fcb1c97ee7c66e5b6d8fbb57c06fd2f6509f2e1b) )

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "hal16l8.ic38", 0x0000, 0x0400, NO_DUMP )
ROM_END


//-------------------------------------------------
//  ROM( ql_es )
//-------------------------------------------------

ROM_START( ql_es )
	ROM_REGION( 0xc000, M68008_TAG, 0 )
	ROM_LOAD( "mge.ic33", 0x0000, 0x8000, BAD_DUMP CRC(d5293bde) SHA1(bf5af7e53a472d4e9871f182210787d601db0634) )
	ROM_LOAD( "mge.ic34", 0x8000, 0x4000, BAD_DUMP CRC(a694f8d7) SHA1(bd2868656008de85d7c191598588017ae8aa3339) )

	ROM_REGION( 0x800, I8749_TAG, 0 )
	ROM_LOAD( "ipc8049.ic24", 0x000, 0x800, CRC(6a0d1f20) SHA1(fcb1c97ee7c66e5b6d8fbb57c06fd2f6509f2e1b) )

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "hal16l8.ic38", 0x0000, 0x0400, NO_DUMP )
ROM_END


//-------------------------------------------------
//  ROM( ql_fr )
//-------------------------------------------------

ROM_START( ql_fr )
	ROM_REGION( 0xc000, M68008_TAG, 0 )
	ROM_LOAD( "mgf.ic33", 0x0000, 0x8000, NO_DUMP )
	ROM_LOAD( "mgf.ic34", 0x8000, 0x4000, NO_DUMP )

	ROM_REGION( 0x800, I8749_TAG, 0 )
	ROM_LOAD( "ipc8049.ic24", 0x000, 0x800, CRC(6a0d1f20) SHA1(fcb1c97ee7c66e5b6d8fbb57c06fd2f6509f2e1b) )

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "hal16l8.ic38", 0x0000, 0x0400, NO_DUMP )
ROM_END


//-------------------------------------------------
//  ROM( ql_de )
//-------------------------------------------------

ROM_START( ql_de )
	ROM_REGION( 0xc000, M68008_TAG, 0 )
	ROM_SYSTEM_BIOS( 0, "mg", "v1.10 (MG)" )
	ROMX_LOAD( "mgg.ic33", 0x0000, 0x8000, BAD_DUMP CRC(b4e468fd) SHA1(cd02a3cd79af90d48b65077d0571efc2f12f146e), ROM_BIOS(0) )
	ROMX_LOAD( "mgg.ic34", 0x8000, 0x4000, BAD_DUMP CRC(54959d40) SHA1(ffc0be9649f26019d7be82925c18dc699259877f), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "mf", "v1.14 (MF)" )
	ROMX_LOAD( "mf.ic33", 0x0000, 0x8000, BAD_DUMP CRC(49c40563) SHA1(d3bcd0614cf9b52e9d7fc2832e11463e5030476b), ROM_BIOS(1) )
	ROMX_LOAD( "mf.ic34", 0x8000, 0x4000, BAD_DUMP CRC(5974616b) SHA1(c3603768c08535c25f077eed02fb80128aff13d9), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "ultramg", "Ultrasoft" )
	ROMX_LOAD( "ultramg.rom", 0x0000, 0x00c000, BAD_DUMP CRC(ad12463b) SHA1(0561b3bc7ce090f3101b2142ee957c18c250eefa), ROM_BIOS(2) )

	ROM_REGION( 0x800, I8749_TAG, 0 )
	ROM_LOAD( "ipc8049.ic24", 0x000, 0x800, CRC(6a0d1f20) SHA1(fcb1c97ee7c66e5b6d8fbb57c06fd2f6509f2e1b) )

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "hal16l8.ic38", 0x0000, 0x0400, NO_DUMP )
ROM_END


//-------------------------------------------------
//  ROM( ql_it )
//-------------------------------------------------

ROM_START( ql_it )
	ROM_REGION( 0xc000, M68008_TAG, 0 )
	ROM_LOAD( "mgi.ic33", 0x0000, 0x8000, BAD_DUMP CRC(d5293bde) SHA1(bf5af7e53a472d4e9871f182210787d601db0634) )
	ROM_LOAD( "mgi.ic34", 0x8000, 0x4000, BAD_DUMP CRC(a2fdfb83) SHA1(162b1052737500f3c13497cdf0f813ba006bdae9) )

	ROM_REGION( 0x800, I8749_TAG, 0 )
	ROM_LOAD( "ipc8049.ic24", 0x000, 0x800, CRC(6a0d1f20) SHA1(fcb1c97ee7c66e5b6d8fbb57c06fd2f6509f2e1b) )

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "hal16l8.ic38", 0x0000, 0x0400, NO_DUMP )
ROM_END


//-------------------------------------------------
//  ROM( ql_se )
//-------------------------------------------------

ROM_START( ql_se )
	ROM_REGION( 0xc000, M68008_TAG, 0 )
	ROM_LOAD( "mgs.ic33", 0x0000, 0x8000, NO_DUMP )
	ROM_LOAD( "mgs.ic34", 0x8000, 0x4000, NO_DUMP )

	ROM_REGION( 0x800, I8749_TAG, 0 )
	ROM_LOAD( "ipc8049.ic24", 0x000, 0x800, CRC(6a0d1f20) SHA1(fcb1c97ee7c66e5b6d8fbb57c06fd2f6509f2e1b) )

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "hal16l8.ic38", 0x0000, 0x0400, NO_DUMP )
ROM_END


//-------------------------------------------------
//  ROM( ql_gr )
//-------------------------------------------------

ROM_START( ql_gr )
	ROM_REGION( 0xc000, M68008_TAG, 0 )
	ROM_LOAD( "efp.ic33", 0x0000, 0x8000, BAD_DUMP CRC(eb181641) SHA1(43c1e0215cf540cbbda240b1048910ff55681059) )
	ROM_LOAD( "efp.ic34", 0x8000, 0x4000, BAD_DUMP CRC(4c3b34b7) SHA1(f9dc571d2d4f68520b306ecc7516acaeea69ec0d) )

	ROM_REGION( 0x800, I8749_TAG, 0 )
	ROM_LOAD( "ipc8049.ic24", 0x000, 0x800, CRC(6a0d1f20) SHA1(fcb1c97ee7c66e5b6d8fbb57c06fd2f6509f2e1b) )

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "hal16l8.ic38", 0x0000, 0x0400, NO_DUMP )
ROM_END


//-------------------------------------------------
//  ROM( ql_dk )
//-------------------------------------------------

ROM_START( ql_dk )
	ROM_REGION( 0xc000, M68008_TAG, 0 )
	ROM_LOAD( "mgd.ic33",  0x0000, 0x8000, BAD_DUMP CRC(f57755eb) SHA1(dc57939ffb8741e17967a1d2479c339750ec7ff6) )
	ROM_LOAD( "mgd.ic34",  0x8000, 0x4000, BAD_DUMP CRC(1892465a) SHA1(0ff3046b5276da6639d3fe79b22ae25cc265d540) )

	ROM_REGION( 0x4000, "extra", 0 )
	ROM_LOAD( "extra.rom", 0x0000, 0x4000, NO_DUMP ) // located at 0x1c000 in M68008 memory map

	ROM_REGION( 0x800, I8749_TAG, 0 )
	ROM_LOAD( "ipc8049.ic24", 0x000, 0x800, CRC(6a0d1f20) SHA1(fcb1c97ee7c66e5b6d8fbb57c06fd2f6509f2e1b) )

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "hal16l8.ic38", 0x0000, 0x0400, NO_DUMP )
ROM_END


//-------------------------------------------------
//  ROM( tonto )
//-------------------------------------------------

ROM_START( tonto )
	ROM_REGION( 0x20000, M68008_TAG, 0 )
	ROM_LOAD( "xbaa02.ic4", 0x000000, 0x008000, CRC(86e7915b) SHA1(a4d8369052eaea93d2174cfd3b14e6cf777f54b4) )
	ROM_LOAD( "xbab03.ic5", 0x008000, 0x008000, CRC(97ef393c) SHA1(450c708e8dfbd42d939a9af6a72ef2a33a3dd3b5) )
	ROM_LOAD( "xbac02.ic6", 0x010000, 0x008000, CRC(a7950897) SHA1(7cd4d6e33a350420a9ebd5c1b32708c29cb20799) )
	ROM_LOAD( "xbad02.ic7", 0x018000, 0x008000, CRC(69d59443) SHA1(cdf28b1b4fca00e8cb79930a1963955af2925618) )

	ROM_REGION( 0x800, I8749_TAG, 0 )
	ROM_LOAD( "ipc8049.ic24", 0x000, 0x800, CRC(6a0d1f20) SHA1(fcb1c97ee7c66e5b6d8fbb57c06fd2f6509f2e1b) )

	ROM_REGION( 0x10000, I8051_TAG, 0 )
	ROM_LOAD( "8051-1.rom", 0x000000, 0x010000, NO_DUMP )

	ROM_REGION( 0x10000, "tms5220", 0 )
	ROM_LOAD( "tms5220.rom", 0x000000, 0x004000, NO_DUMP )

	ROM_REGION( 0x400000, "rompack", 0 )
	ROM_LOAD( "xbak02.ic6", 0x000000, 0x008000, CRC(3d61e84e) SHA1(7d0a473b0ed3e3ae3cd6d3fb43cb7a70a550d1d9) )
	ROM_LOAD( "xbaf01.ic7", 0x008000, 0x008000, CRC(c5724357) SHA1(6e765f8e85d2312c31e4b09c9a6645ab3d166e65) )
	ROM_LOAD( "xbag01.ic8", 0x010000, 0x008000, CRC(9af3bcbf) SHA1(9a848926f2bf6f632ab198a8bb03a8ee83ea0e5d) )
	ROM_LOAD( "qcaw02.ic9", 0x018000, 0x008000, CRC(1091a789) SHA1(33881b2a818cbaf610a0746fde4acfef5c91338f) )
	ROM_LOAD( "qcax02.ic10", 0x020000, 0x008000, CRC(396f13d9) SHA1(b2deb5db4565ff7224730c2f3245fbf9df84d728) )
ROM_END


#if 0
//-------------------------------------------------
//  ROM( megaopd )
//-------------------------------------------------

ROM_START( megaopd )
	ROM_REGION( 0x20000, M68008_TAG, 0 )
	ROM_LOAD( "bios-1.rom", 0x000000, 0x008000, NO_DUMP )
	ROM_LOAD( "bios-2.rom", 0x008000, 0x008000, NO_DUMP )
	ROM_LOAD( "bios-3.rom", 0x010000, 0x008000, NO_DUMP )
	ROM_LOAD( "bios-4.rom", 0x018000, 0x008000, NO_DUMP )

	ROM_REGION( 0x10000, I8051_TAG, 0 )
	ROM_LOAD( "8051-1.rom", 0x000000, 0x010000, NO_DUMP )

	ROM_REGION( 0x10000, "tms5220", 0 )
	ROM_LOAD( "tms5220.rom", 0x000000, 0x004000, NO_DUMP )

	ROM_REGION( 0x400000, "rompack", 0 )
	ROM_LOAD( "rompack-1.rom", 0x000000, 0x008000, NO_DUMP )
	ROM_LOAD( "rompack-2.rom", 0x008000, 0x008000, NO_DUMP )
	ROM_LOAD( "rompack-3.rom", 0x010000, 0x008000, NO_DUMP )
	ROM_LOAD( "rompack-4.rom", 0x018000, 0x008000, NO_DUMP )
	ROM_LOAD( "rompack-5.rom", 0x020000, 0x008000, NO_DUMP )
ROM_END
#endif

} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME     PARENT  COMPAT  MACHINE     INPUT   CLASS     INIT        COMPANY                             FULLNAME              FLAGS
COMP( 1984, ql,      0,      0,      ql,         ql,     ql_state, empty_init, "Sinclair Research Ltd",            "QL (UK)",            MACHINE_SUPPORTS_SAVE )
COMP( 1985, ql_us,   ql,     0,      ql_ntsc,    ql,     ql_state, empty_init, "Sinclair Research Ltd",            "QL (USA)",           MACHINE_SUPPORTS_SAVE )
COMP( 1985, ql_es,   ql,     0,      ql,         ql_es,  ql_state, empty_init, "Sinclair Research Ltd",            "QL (Spain)",         MACHINE_SUPPORTS_SAVE )
COMP( 1985, ql_fr,   ql,     0,      ql,         ql_fr,  ql_state, empty_init, "Sinclair Research Ltd",            "QL (France)",        MACHINE_NOT_WORKING )
COMP( 1985, ql_de,   ql,     0,      ql,         ql_de,  ql_state, empty_init, "Sinclair Research Ltd",            "QL (Germany)",       MACHINE_SUPPORTS_SAVE )
COMP( 1985, ql_it,   ql,     0,      ql,         ql_it,  ql_state, empty_init, "Sinclair Research Ltd",            "QL (Italy)",         MACHINE_SUPPORTS_SAVE )
COMP( 1985, ql_se,   ql,     0,      ql,         ql_se,  ql_state, empty_init, "Sinclair Research Ltd",            "QL (Sweden)",        MACHINE_NOT_WORKING )
COMP( 1985, ql_dk,   ql,     0,      ql,         ql_dk,  ql_state, empty_init, "Sinclair Research Ltd",            "QL (Denmark)",       MACHINE_NOT_WORKING )
COMP( 1985, ql_gr,   ql,     0,      ql,         ql,     ql_state, empty_init, "Sinclair Research Ltd",            "QL (Greece)",        MACHINE_SUPPORTS_SAVE )
COMP( 1984, tonto,   0,      0,      opd,        ql,     ql_state, empty_init, "British Telecom Business Systems", "Merlin M1800 Tonto", MACHINE_NOT_WORKING )
//COMP( 1986, megaopd, tonto,  0,      megaopd,    ql,     ql_state, empty_init, "International Computer Limited",   "MegaOPD (USA)",      MACHINE_NOT_WORKING )
