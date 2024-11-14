// license:BSD-3-Clause
// copyright-holders:Curt Coder, Robbbert
/*

Osborne 4 Vixen

Main PCB Layout
---------------

TODO

Notes:
    Relevant IC's shown.

    CPU     - Zilog Z8400APS Z80A CPU
    FDC     - SMC FDC1797
    8155    - Intel P8155H
    ROM0    -
    ROM1,2  - AMD AM2732-1DC 4Kx8 EPROM
    CN1     - keyboard connector
    CN2     -
    CN3     -
    CN4     - floppy connector
    CN5     - power connector
    CN6     - composite video connector
    SW1     - reset switch
    SW2     -


I/O PCB Layout
--------------

TODO

Notes:
    Relevant IC's shown.

    8155    - Intel P8155H
    8251    - AMD P8251A
    CN1     - IEEE488 connector
    CN2     - RS232 connector
    CN3     -

*/

/*

    TODO:

    - RS232 RI interrupt
    - PCB layouts

*/


#include "emu.h"

#include "bus/ieee488/ieee488.h"
#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "machine/i8155.h"
#include "machine/i8251.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "machine/wd_fdc.h"
#include "sound/discrete.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"
#include "utf8.h"


namespace {

#define Z8400A_TAG      "5f"
#define FDC1797_TAG     "5n"
#define P8155H_TAG      "2n"
#define SCREEN_TAG      "screen"

class vixen_state : public driver_device
{
public:
	vixen_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, Z8400A_TAG)
		, m_fdc(*this, FDC1797_TAG)
		, m_io_i8155(*this, "c7")
		, m_usart(*this, "c3")
		, m_discrete(*this, "discrete")
		, m_ieee488(*this, IEEE488_TAG)
		, m_palette(*this, "palette")
		, m_ram(*this, RAM_TAG)
		, m_floppy(*this, FDC1797_TAG":%u", 0U)
		, m_rs232(*this, "rs232")
		, m_rom(*this, Z8400A_TAG)
		, m_sync_rom(*this, "video")
		, m_char_rom(*this, "chargen")
		, m_video_ram(*this, "video_ram")
		, m_key(*this, "KEY.%u", 0U)
	{ }

	void vixen(machine_config &config);

	void init_vixen();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	uint8_t status_r();
	void cmd_w(uint8_t data);
	uint8_t ieee488_r();
	uint8_t port3_r();
	uint8_t i8155_pa_r();
	void i8155_pb_w(uint8_t data);
	void i8155_pc_w(uint8_t data);
	void io_i8155_pb_w(uint8_t data);
	void io_i8155_pc_w(uint8_t data);
	void io_i8155_to_w(int state);
	void srq_w(int state);
	void atn_w(int state);
	void rxrdy_w(int state);
	void txrdy_w(int state);
	void fdc_intrq_w(int state);
	TIMER_DEVICE_CALLBACK_MEMBER(vsync_tick);
	IRQ_CALLBACK_MEMBER(vixen_int_ack);
	uint8_t opram_r(offs_t offset);
	uint8_t oprom_r(offs_t offset);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void bios_mem(address_map &map) ATTR_COLD;
	void vixen_io(address_map &map) ATTR_COLD;
	void vixen_mem(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<fd1797_device> m_fdc;
	required_device<i8155_device> m_io_i8155;
	required_device<i8251_device> m_usart;
	required_device<discrete_sound_device> m_discrete;
	required_device<ieee488_device> m_ieee488;
	required_device<palette_device> m_palette;
	required_device<ram_device> m_ram;
	required_device_array<floppy_connector, 2> m_floppy;
	required_device<rs232_port_device> m_rs232;
	required_region_ptr<uint8_t> m_rom;
	required_region_ptr<uint8_t> m_sync_rom;
	required_region_ptr<uint8_t> m_char_rom;
	required_shared_ptr<uint8_t> m_video_ram;
	required_ioport_array<8> m_key;

	address_space *m_program = nullptr;

	void update_interrupt();

	// keyboard state
	uint8_t m_col = 0U;

	// interrupt state
	int m_cmd_d0 = 0;
	int m_cmd_d1 = 0;

	bool m_fdint = false;
	int m_vsync = 0;

	int m_srq = 1;
	int m_atn = 1;
	int m_enb_srq_int = 0;
	int m_enb_atn_int = 0;

	int m_rxrdy = 0;
	int m_txrdy = 0;
	int m_int_clk = 0;
	int m_enb_xmt_int = 0;
	int m_enb_rcv_int = 0;
	int m_enb_ring_int = 0;

	// video state
	bool m_alt = false;
	bool m_256 = false;
};



//**************************************************************************
//  INTERRUPTS
//**************************************************************************

void vixen_state::update_interrupt()
{
	int state = (m_cmd_d1 && m_fdint) || m_vsync;// || (!m_enb_srq_int && !m_srq) || (!m_enb_atn_int && !m_atn) || (!m_enb_xmt_int && m_txrdy) || (!m_enb_rcv_int && m_rxrdy);

	m_maincpu->set_input_line(INPUT_LINE_IRQ0, state ? ASSERT_LINE : CLEAR_LINE);
}


uint8_t vixen_state::opram_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		membank("bank3")->set_entry(0); // read videoram
	return m_program->read_byte(offset);
}

uint8_t vixen_state::oprom_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		membank("bank3")->set_entry(1); // read rom
	return m_rom[offset];
}

uint8_t vixen_state::status_r()
{
	/*

	    bit     description

	    0       VSYNC enable
	    1       FDINT enable
	    2       VSYNC
	    3       1
	    4       1
	    5       1
	    6       1
	    7       1

	*/

	uint8_t data = 0xf8;

	// vertical sync interrupt enable
	data |= m_cmd_d0;

	// floppy interrupt enable
	data |= m_cmd_d1 << 1;

	// vertical sync
	data |= m_vsync << 2;

	return data;
}

void vixen_state::cmd_w(uint8_t data)
{
	/*

	    bit     description

	    0       VSYNC enable
	    1       FDINT enable
	    2
	    3
	    4
	    5
	    6
	    7

	*/

//  logerror("CMD %u\n", data);

	// vertical sync interrupt enable
	m_cmd_d0 = BIT(data, 0);

	if (!m_cmd_d0)
	{
		// clear vertical sync
		m_vsync = 0;
	}

	// floppy interrupt enable
	m_cmd_d1 = BIT(data, 1);

	update_interrupt();
}

uint8_t vixen_state::ieee488_r()
{
	/*

	    bit     description

	    0       ATN
	    1       DAV
	    2       NDAC
	    3       NRFD
	    4       EOI
	    5       SRQ
	    6       IFC
	    7       REN

	*/

	uint8_t data = 0;

	/* attention */
	data |= m_ieee488->atn_r();

	/* data valid */
	data |= m_ieee488->dav_r() << 1;

	/* data not accepted */
	data |= m_ieee488->ndac_r() << 2;

	/* not ready for data */
	data |= m_ieee488->nrfd_r() << 3;

	/* end or identify */
	data |= m_ieee488->eoi_r() << 4;

	/* service request */
	data |= m_ieee488->srq_r() << 5;

	/* interface clear */
	data |= m_ieee488->ifc_r() << 6;

	/* remote enable */
	data |= m_ieee488->ren_r() << 7;

	return data;
}


//-------------------------------------------------
//  port3_r - serial status read
//-------------------------------------------------

uint8_t vixen_state::port3_r()
{
	/*

	    bit     description

	    0       RI
	    1       DCD
	    2       1
	    3       1
	    4       1
	    5       1
	    6       1
	    7       1

	*/

	uint8_t data = 0xfc;

	// ring indicator
	data |= m_rs232->ri_r();

	// data carrier detect
	data |= m_rs232->dcd_r() << 1;

	return data;
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

// when M1 is inactive: read and write of data
void vixen_state::vixen_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xefff).ram();
	map(0xf000, 0xffff).bankr("bank3").bankw("bank4").share("video_ram");
}

// when M1 is active: read opcodes
void vixen_state::bios_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xefff).r(FUNC(vixen_state::opram_r));
	map(0xf000, 0xffff).r(FUNC(vixen_state::oprom_r));
}

void vixen_state::vixen_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x03).rw(m_fdc, FUNC(fd1797_device::read), FUNC(fd1797_device::write));
	map(0x04, 0x04).mirror(0x03).rw(FUNC(vixen_state::status_r), FUNC(vixen_state::cmd_w));
	map(0x08, 0x08).mirror(0x01).rw(P8155H_TAG, FUNC(i8155_device::data_r), FUNC(i8155_device::data_w));
	map(0x0c, 0x0d).w(P8155H_TAG, FUNC(i8155_device::ale_w));
	map(0x10, 0x10).mirror(0x07).r(m_ieee488, FUNC(ieee488_device::dio_r));
	map(0x18, 0x18).mirror(0x07).r(FUNC(vixen_state::ieee488_r));
	map(0x20, 0x21).mirror(0x04).w(m_io_i8155, FUNC(i8155_device::ale_w));
	map(0x28, 0x28).mirror(0x05).rw(m_io_i8155, FUNC(i8155_device::data_r), FUNC(i8155_device::data_w));
	map(0x30, 0x31).mirror(0x06).rw(m_usart, FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x38, 0x38).mirror(0x07).r(FUNC(vixen_state::port3_r));
//  map(0xf0, 0xff) Hard Disk?
}



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

INPUT_PORTS_START( vixen )
	PORT_START("KEY.0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC) PORT_CHAR(0x1B)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TAB) PORT_CHAR(0x09)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(0x0D)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(0x27) PORT_CHAR(0x22)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR(']')

	PORT_START("KEY.1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*')

	PORT_START("KEY.2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q') PORT_CHAR(0x11)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W') PORT_CHAR(0x17)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E') PORT_CHAR(0x05)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R') PORT_CHAR(0x12)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T') PORT_CHAR(0x14)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y') PORT_CHAR(0x19)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U') PORT_CHAR(0x15)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I') PORT_CHAR(0x09)

	PORT_START("KEY.3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A') PORT_CHAR(0x01)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S') PORT_CHAR(0x13)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D') PORT_CHAR(0x04)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F') PORT_CHAR(0x06)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G') PORT_CHAR(0x07)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H') PORT_CHAR(0x08)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J') PORT_CHAR(0x0a)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K') PORT_CHAR(0x0b)

	PORT_START("KEY.4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z') PORT_CHAR(0x1a)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X') PORT_CHAR(0x18)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C') PORT_CHAR(0x03)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V') PORT_CHAR(0x16)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B') PORT_CHAR(0x02)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N') PORT_CHAR(0x0e)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M') PORT_CHAR(0x0d)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')

	PORT_START("KEY.5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_LEFT) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8, UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P') PORT_CHAR(0x10)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O') PORT_CHAR(0x0f)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')

	PORT_START("KEY.6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_DOWN) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("- _") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_') PORT_CHAR(0x1F)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?') PORT_CHAR(0x7E)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\\ |") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|') PORT_CHAR(0x1C)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+') PORT_CHAR(0x60)

	PORT_START("KEY.7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("DEL") PORT_CODE(KEYCODE_DEL) PORT_CHAR(127)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('{') PORT_CHAR('}')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("FUNC") PORT_CODE(KEYCODE_END)
INPUT_PORTS_END



//**************************************************************************
//  VIDEO
//**************************************************************************

TIMER_DEVICE_CALLBACK_MEMBER(vixen_state::vsync_tick)
{
	if (m_cmd_d0)
	{
		m_vsync = 1;
		update_interrupt();
	}
}

uint32_t vixen_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const pen_t *pen = m_palette->pens();
	uint8_t x, y, chr, gfx, inv, ra;

	for (y = 0; y < 26; y++)
	{
		for (ra = 0; ra < 10; ra++)
		{
			for (x = 0; x < 128; x++)
			{
				uint16_t sync_addr = ((y+1) << 7) + x + 1; // it's out by a row and a column
				uint8_t sync_data = m_sync_rom[sync_addr & 0xfff];
				bool blank = BIT(sync_data, 4);
				/*
				int clrchadr = BIT(sync_data, 7);
				int hsync = BIT(sync_data, 6);
				int clrtxadr = BIT(sync_data, 5);
				int vsync = BIT(sync_data, 3);
				int comp_sync = BIT(sync_data, 2);

				logerror("SYNC %03x:%02x TXADR %u SCAN %u CHADR %u : COMPSYNC %u VSYNC %u BLANK %u CLRTXADR %u HSYNC %u CLRCHADR %u\n",
				    sync_addr,sync_data,txadr,scan,chadr,comp_sync,vsync,blank,clrtxadr,hsync,clrchadr);
				*/

				chr = m_video_ram[(y<<7) + x];

				if (m_256)
				{
					gfx = m_char_rom[(BIT(chr, 7) << 11) | (ra << 7) | (chr & 0x7f)];
					inv = m_alt ? 0xff : 0;
				}
				else
				{
					gfx = m_char_rom[(ra << 7) | (chr & 0x7f)];
					inv = BIT(chr, 7) ? 0xff : 0;
				}

				gfx = (blank) ? 0 : (gfx ^ inv);

				for (int b = 0; b < 8; b++)
				{
					int color = BIT(gfx, 7 - b);

					bitmap.pix((y * 10) + ra, (x * 8) + b) = pen[color];
				}
			}
		}
	}

	return 0;
}



//**************************************************************************
//  SOUND
//**************************************************************************

static DISCRETE_SOUND_START( vixen_discrete )
	DISCRETE_INPUT_LOGIC(NODE_01)
	DISCRETE_SQUAREWAVE(NODE_02, NODE_01, (23.9616_MHz_XTAL / 15360).dvalue(), 100, 50, 0, 90)
	DISCRETE_OUTPUT(NODE_02, 2000)
DISCRETE_SOUND_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  I8155 interface
//-------------------------------------------------

uint8_t vixen_state::i8155_pa_r()
{
	uint8_t data = 0xff;

	for (int i = 0; i < 8; i++)
		if (!BIT(m_col, i)) data &= m_key[i]->read();

	return data;
}

void vixen_state::i8155_pb_w(uint8_t data)
{
	m_col = data;
}

void vixen_state::i8155_pc_w(uint8_t data)
{
	/*

	    bit     description

	    0       DSEL1/
	    1       DSEL2/
	    2       DDEN/
	    3       ALT CHARSET/
	    4       256 CHARS
	    5       BEEP ENABLE
	    6
	    7

	*/

	// drive select
	floppy_image_device *floppy = nullptr;

	if (!BIT(data, 0)) floppy = m_floppy[0]->get_device();
	if (!BIT(data, 1)) floppy = m_floppy[1]->get_device();

	m_fdc->set_floppy(floppy);

	if (floppy) floppy->mon_w(0);

	// density select
	m_fdc->dden_w(BIT(data, 2));

	// charset
	m_alt = !BIT(data, 3);
	m_256 = !BIT(data, 4);

	// beep enable
	m_discrete->write(NODE_01, !BIT(data, 5));
}

//-------------------------------------------------
//  I8155 IO interface
//-------------------------------------------------

void vixen_state::io_i8155_pb_w(uint8_t data)
{
	/*

	    bit     description

	    PB0     ATN
	    PB1     DAV
	    PB2     NDAC
	    PB3     NRFD
	    PB4     EOI
	    PB5     SRQ
	    PB6     IFC
	    PB7     REN

	*/

	/* data valid */
	m_ieee488->host_atn_w(BIT(data, 0));

	/* end or identify */
	m_ieee488->host_dav_w(BIT(data, 1));

	/* remote enable */
	m_ieee488->host_ndac_w(BIT(data, 2));

	/* attention */
	m_ieee488->host_nrfd_w(BIT(data, 3));

	/* interface clear */
	m_ieee488->host_eoi_w(BIT(data, 4));

	/* service request */
	m_ieee488->host_srq_w(BIT(data, 5));

	/* not ready for data */
	m_ieee488->host_ifc_w(BIT(data, 6));

	/* data not accepted */
	m_ieee488->host_ren_w(BIT(data, 7));
}

void vixen_state::io_i8155_pc_w(uint8_t data)
{
	/*

	    bit     description

	    PC0     select internal clock
	    PC1     ENB RING INT
	    PC2     ENB RCV INT
	    PC3     ENB XMT INT
	    PC4     ENB ATN INT
	    PC5     ENB SRQ INT
	    PC6
	    PC7

	*/

	m_int_clk = BIT(data, 0);
	m_enb_ring_int = BIT(data, 1);
	m_enb_rcv_int = BIT(data, 2);
	m_enb_xmt_int = BIT(data, 3);
	m_enb_atn_int = BIT(data, 4);
	m_enb_srq_int = BIT(data, 5);
}

void vixen_state::io_i8155_to_w(int state)
{
	if (m_int_clk)
	{
		m_usart->write_txc(state);
		m_usart->write_rxc(state);
	}
}

//-------------------------------------------------
//  i8251_interface usart_intf
//-------------------------------------------------

void vixen_state::rxrdy_w(int state)
{
	m_rxrdy = state;
	update_interrupt();
}

void vixen_state::txrdy_w(int state)
{
	m_txrdy = state;
	update_interrupt();
}

//-------------------------------------------------
//  IEEE488 interface
//-------------------------------------------------

void vixen_state::srq_w(int state)
{
	m_srq = state;
	update_interrupt();
}

void vixen_state::atn_w(int state)
{
	m_atn = state;
	update_interrupt();
}

static void vixen_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
}

void vixen_state::fdc_intrq_w(int state)
{
	m_fdint = state;
	update_interrupt();
}



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

IRQ_CALLBACK_MEMBER(vixen_state::vixen_int_ack)
{
	// D0 is pulled low
	return 0xfe;
}

void vixen_state::machine_start()
{
	// configure memory banking

	membank("bank3")->configure_entry(0, m_video_ram);
	membank("bank3")->configure_entry(1, m_rom);

	membank("bank4")->configure_entry(0, m_video_ram);

	// register for state saving
	save_item(NAME(m_col));
	save_item(NAME(m_cmd_d0));
	save_item(NAME(m_cmd_d1));
	save_item(NAME(m_fdint));
	save_item(NAME(m_alt));
	save_item(NAME(m_256));
	save_item(NAME(m_vsync));
	save_item(NAME(m_srq));
	save_item(NAME(m_atn));
	save_item(NAME(m_enb_srq_int));
	save_item(NAME(m_enb_atn_int));
	save_item(NAME(m_rxrdy));
	save_item(NAME(m_txrdy));
	save_item(NAME(m_int_clk));
	save_item(NAME(m_enb_xmt_int));
	save_item(NAME(m_enb_rcv_int));
	save_item(NAME(m_enb_ring_int));

}

void vixen_state::machine_reset()
{
	membank("bank3")->set_entry(1);

	m_vsync = 0;
	m_cmd_d0 = 0;
	m_cmd_d1 = 0;
	update_interrupt();

	m_fdc->reset();
	m_io_i8155->reset();
	m_usart->reset();
	m_maincpu->set_state_int(Z80_PC, 0xf000);
}



//**************************************************************************
//  MACHINE CONFIGURATION
//**************************************************************************

void vixen_state::vixen(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 23.9616_MHz_XTAL / 6);
	m_maincpu->set_addrmap(AS_PROGRAM, &vixen_state::vixen_mem);
	m_maincpu->set_addrmap(AS_OPCODES, &vixen_state::bios_mem);
	m_maincpu->set_addrmap(AS_IO, &vixen_state::vixen_io);
	m_maincpu->set_irq_acknowledge_callback(FUNC(vixen_state::vixen_int_ack));

	// video hardware
	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER));
	screen.set_color(rgb_t::amber());
	screen.set_screen_update(FUNC(vixen_state::screen_update));
	screen.set_raw(23.9616_MHz_XTAL / 2, 96*8, 0*8, 81*8, 27*10, 0*10, 26*10);

	TIMER(config, "vsync").configure_scanline(FUNC(vixen_state::vsync_tick), SCREEN_TAG, 26*10, 27*10);

	PALETTE(config, m_palette, palette_device::MONOCHROME);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	DISCRETE(config, m_discrete, vixen_discrete).add_route(ALL_OUTPUTS, "mono", 0.20);

	// devices
	i8155_device &i8155(I8155(config, P8155H_TAG, 23.9616_MHz_XTAL / 6));
	i8155.in_pa_callback().set(FUNC(vixen_state::i8155_pa_r));
	i8155.out_pb_callback().set(FUNC(vixen_state::i8155_pb_w));
	i8155.out_pc_callback().set(FUNC(vixen_state::i8155_pc_w));

	I8155(config, m_io_i8155, 23.9616_MHz_XTAL / 6);
	m_io_i8155->out_pa_callback().set(m_ieee488, FUNC(ieee488_device::host_dio_w));
	m_io_i8155->out_pb_callback().set(FUNC(vixen_state::io_i8155_pb_w));
	m_io_i8155->out_pc_callback().set(FUNC(vixen_state::io_i8155_pc_w));
	m_io_i8155->out_to_callback().set(FUNC(vixen_state::io_i8155_to_w));

	I8251(config, m_usart, 0);
	m_usart->txd_handler().set(m_rs232, FUNC(rs232_port_device::write_txd));
	m_usart->dtr_handler().set(m_rs232, FUNC(rs232_port_device::write_dtr));
	m_usart->rts_handler().set(m_rs232, FUNC(rs232_port_device::write_rts));
	m_usart->rxrdy_handler().set(FUNC(vixen_state::rxrdy_w));
	m_usart->txrdy_handler().set(FUNC(vixen_state::txrdy_w));

	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->rxd_handler().set(m_usart, FUNC(i8251_device::write_rxd));
	m_rs232->dsr_handler().set(m_usart, FUNC(i8251_device::write_dsr));

	FD1797(config, m_fdc, 23.9616_MHz_XTAL / 24);
	m_fdc->intrq_wr_callback().set(FUNC(vixen_state::fdc_intrq_w));
	FLOPPY_CONNECTOR(config, m_floppy[0], vixen_floppies, "525dd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[1], vixen_floppies, "525dd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	IEEE488(config, m_ieee488);
	m_ieee488->srq_callback().set(FUNC(vixen_state::srq_w));
	m_ieee488->atn_callback().set(FUNC(vixen_state::atn_w));

	/* software lists */
	SOFTWARE_LIST(config, "disk_list").set_original("vixen");

	// internal ram
	RAM(config, RAM_TAG).set_default_size("64K");
}



//**************************************************************************
//  ROMS
//**************************************************************************

ROM_START( vixen )
	ROM_REGION( 0x1000, Z8400A_TAG, 0 )
	ROM_LOAD( "osborne 4 mon rom v1.04 3p40082-03 a0a9.4c", 0x0000, 0x1000, CRC(5f1038ce) SHA1(e6809fac23650bbb4689e58edc768d917d80a2df) ) // OSBORNE 4 MON ROM / V1.04  3P40082-03 / A0A9 (c) OCC 1985

	ROM_REGION( 0x1000, "video", 0 )
	ROM_LOAD( "v1.10.3j", 0x0000, 0x1000, CRC(1f93e2d7) SHA1(0c479bfd3ac8d9959c285c020d0096930a9c6867) )

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "v1.00 l.1j", 0x0000, 0x1000, CRC(f97c50d9) SHA1(39f73afad68508c4b8a4d241c064f9978098d8f2) )
ROM_END



//**************************************************************************
//  DRIVER INITIALIZATION
//**************************************************************************

void vixen_state::init_vixen()
{
	m_program = &m_maincpu->space(AS_PROGRAM);
}


} // anonymous namespace



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT   CLASS        INIT        COMPANY    FULLNAME  FLAGS
COMP( 1984, vixen, 0,       0,     vixen,   vixen,  vixen_state, init_vixen, "Osborne", "Vixen",  MACHINE_SUPPORTS_SAVE )
