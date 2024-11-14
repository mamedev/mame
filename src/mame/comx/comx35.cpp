// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

    TODO:

    - tape input/output
    - PL-80 plotter
    - serial printer
    - thermal printer


    Cassette: PSAVE works, and the result can be loaded into Emma02 emulator.
              PLOAD works, but it may be necessary to unplug all slots to get
              a reliable load. This is the same as real hardware.

*/

#include "emu.h"
#include "comx35.h"
#include "screen.h"
#include "softlist_dev.h"

#include "multibyte.h"
#include "utf8.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

#define LOG 0

enum
{
	COMX_TYPE_BINARY = 1,
	COMX_TYPE_BASIC,
	COMX_TYPE_BASIC_FM,
	COMX_TYPE_RESERVED,
	COMX_TYPE_DATA
};

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    image_fread_memory - read image to memory
-------------------------------------------------*/

void comx35_state::image_fread_memory(device_image_interface &image, uint16_t addr, uint32_t count)
{
	uint8_t *ram = m_ram->pointer() + (addr - 0x4000);

	image.fread(ram, count);
}

/*-------------------------------------------------
    QUICKLOAD_LOAD_MEMBER( comx35_state, comx )
-------------------------------------------------*/

QUICKLOAD_LOAD_MEMBER(comx35_state::quickload_cb)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	uint8_t header[16] = {0};
	int size = image.length();

	if (size > m_ram->size())
		return std::make_pair(image_error::INVALIDLENGTH, "Image is larger than RAM");

	image.fread( header, 5);

	if (header[1] != 'C' || header[2] != 'O' || header[3] != 'M' || header[4] != 'X' )
		return std::make_pair(image_error::INVALIDIMAGE, std::string());

	switch (header[0])
	{
	case COMX_TYPE_BINARY:
		/*

		    Type 1: pure machine code (i.e. no basic)

		    Byte 0 to 4: 1 - 'COMX'
		    Byte 5 and 6: Start address (1802 way; see above)
		    Byte 6 and 7: End address
		    Byte 9 and 10: Execution address

		    Byte 11 to Eof, should be stored in ram from start to end; execution address
		    'xxxx' for the CALL (@xxxx) basic statement to actually run the code.

		*/
		{
			uint16_t start_address, end_address, run_address;

			image.fread(header, 6);

			start_address = get_u16be(&header[0]);
			end_address = get_u16be(&header[2]);
			run_address = get_u16be(&header[4]);

			image_fread_memory(image, start_address, end_address - start_address);

			popmessage("Type CALL (@%04x) to start program", run_address);
		}
		break;

	case COMX_TYPE_BASIC:
		/*

		    Type 2: Regular basic code or machine code followed by basic

		    Byte 0 to 4: 2 - 'COMX'
		    Byte 5 and 6: DEFUS value, to be stored on 0x4281 and 0x4282
		    Byte 7 and 8: EOP value, to be stored on 0x4283 and 0x4284
		    Byte 9 and 10: End array, start string to be stored on 0x4292 and 0x4293
		    Byte 11 and 12: start array to be stored on 0x4294 and 0x4295
		    Byte 13 and 14: EOD and end string to be stored on 0x4299 and 0x429A

		    Byte 15 to Eof to be stored on 0x4400 and onwards

		    Byte 0x4281-0x429A (or at least the ones above) should be set otherwise
		    BASIC won't 'see' the code.

		*/

		image_fread_memory(image, 0x4281, 4);
		image_fread_memory(image, 0x4292, 4);
		image_fread_memory(image, 0x4299, 2);
		image_fread_memory(image, 0x4400, size);
		break;

	case COMX_TYPE_BASIC_FM:
		/*

		    Type 3: F&M basic load

		    Not the most important! But we designed our own basic extension, you can
		    find it in the F&M basic folder as F&M Basic.comx. When you run this all
		    basic code should start at address 0x6700 instead of 0x4400 as from
		    0x4400-0x6700 the F&M basic stuff is loaded. So format is identical to Type
		    2 except Byte 15 to Eof should be stored on 0x6700 instead. .comx files of
		    this format can also be found in the same folder as the F&M basic.comx file.

		*/

		image_fread_memory(image, 0x4281, 4);
		image_fread_memory(image, 0x4292, 4);
		image_fread_memory(image, 0x4299, 2);
		image_fread_memory(image, 0x6700, size);
		break;

	case COMX_TYPE_RESERVED:
		/*

		    Type 4: Incorrect DATA format, I suggest to forget this one as it won't work
		    in most cases. Instead I left this one reserved and designed Type 5 instead.

		*/
		break;

	case COMX_TYPE_DATA:
		/*

		    Type 5: Data load

		    Byte 0 to 4: 5 - 'COMX'
		    Byte 5 and 6: Array length
		    Byte 7 to Eof: Basic 'data'

		    To load this first get the 'start array' from the running COMX, i.e. address
		    0x4295/0x4296. Calculate the EOD as 'start array' + length of the data (i.e.
		    file length - 7). Store the EOD back on 0x4299 and ox429A. Calculate the
		    'Start String' as 'start array' + 'Array length' (Byte 5 and 6). Store the
		    'Start String' on 0x4292/0x4293. Load byte 7 and onwards starting from the
		    'start array' value fetched from 0x4295/0x4296.

		*/
		{
			uint16_t start_array, end_array, start_string, array_length;

			image.fread(header, 2);

			array_length = get_u16be(&header[0]);
			start_array = (program.read_byte(0x4295) << 8) | program.read_byte(0x4296);
			end_array = start_array + (size - 7);

			program.write_byte(0x4299, end_array >> 8);
			program.write_byte(0x429a, end_array & 0xff);

			start_string = start_array + array_length;

			program.write_byte(0x4292, start_string >> 8);
			program.write_byte(0x4293, start_string & 0xff);

			image_fread_memory(image, start_array, size);
		}
		break;
	}

	return std::make_pair(std::error_condition(), std::string());
}

//**************************************************************************
//  MEMORY ACCESS
//**************************************************************************

//-------------------------------------------------
//  mem_r - memory read
//-------------------------------------------------

uint8_t comx35_state::mem_r(offs_t offset)
{
	int extrom = 1;

	uint8_t data = m_exp->mrd_r(offset, &extrom);

	if (offset < 0x4000)
	{
		if (extrom) data = m_rom->base()[offset & 0x3fff];
	}
	else if (offset >= 0x4000 && offset < 0xc000)
	{
		data = m_ram->pointer()[offset - 0x4000];
	}
	else if (offset >= 0xf400 && offset < 0xf800)
	{
		data = m_vis->char_ram_r(offset & 0x3ff);
	}

	return data;
}


//-------------------------------------------------
//  mem_w - memory write
//-------------------------------------------------

void comx35_state::mem_w(offs_t offset, uint8_t data)
{
	m_exp->mwr_w(offset, data);

	if (offset >= 0x4000 && offset < 0xc000)
	{
		m_ram->pointer()[offset - 0x4000] = data;
	}
	else if (offset >= 0xf400 && offset < 0xf800)
	{
		m_vis->char_ram_w(offset & 0x3ff, data);
	}
	else if (offset >= 0xf800)
	{
		m_vis->page_ram_w(offset & 0x3ff, data);
	}
}


//-------------------------------------------------
//  io_r - I/O read
//-------------------------------------------------

uint8_t comx35_state::io_r(offs_t offset)
{
	uint8_t data = m_exp->io_r(offset);

	if (offset == 3)
	{
		data = m_kbe->read();
	}

	return data;
}


//-------------------------------------------------
//  io_w - I/O write
//-------------------------------------------------

void comx35_state::io_w(offs_t offset, uint8_t data)
{
	m_exp->io_w(offset, data);

	if (offset >= 3)
	{
		cdp1869_w(offset, data);
	}
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( comx35_mem )
//-------------------------------------------------

void comx35_state::comx35_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).rw(FUNC(comx35_state::mem_r), FUNC(comx35_state::mem_w));
}


//-------------------------------------------------
//  ADDRESS_MAP( comx35_io )
//-------------------------------------------------

void comx35_state::comx35_io(address_map &map)
{
	map.unmap_value_high();
	map(0x00, 0x07).rw(FUNC(comx35_state::io_r), FUNC(comx35_state::io_w));
}



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_CHANGED_MEMBER( comx35_reset )
//-------------------------------------------------

INPUT_CHANGED_MEMBER( comx35_state::trigger_reset )
{
	if (newval && BIT(m_d6->read(), 7))
	{
		machine_reset();
	}
}


//-------------------------------------------------
//  INPUT_PORTS( comx35 )
//-------------------------------------------------

static INPUT_PORTS_START( comx35 )
	PORT_START("D1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("0 \xE2\x96\xA0") PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('@')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR(39)

	PORT_START("D2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('[')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(']')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR(':')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR(';')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('<') PORT_CHAR('(')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('=') PORT_CHAR('^')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('>') PORT_CHAR(')')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('\\') PORT_CHAR('_')

	PORT_START("D3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('?')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G')

	PORT_START("D4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O')

	PORT_START("D5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W')

	PORT_START("D6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR('+')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('-')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('*')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')

	PORT_START("D7")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("D8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CR") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(UTF8_DOWN) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("DEL") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("D9")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("D10")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("D11")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("MODIFIERS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1) PORT_WRITE_LINE_DEVICE_MEMBER(CDP1871_TAG, FUNC(cdp1871_device::shift_w))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CNTL") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL)) PORT_CHAR(UCHAR_MAMEKEY(RCONTROL)) PORT_WRITE_LINE_DEVICE_MEMBER(CDP1871_TAG, FUNC(cdp1871_device::control_w))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("RESET")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RT") PORT_CODE(KEYCODE_F10) PORT_CHAR(UCHAR_MAMEKEY(F10)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(comx35_state::trigger_reset), 0)
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  COSMAC_INTERFACE( cosmac_intf )
//-------------------------------------------------

void comx35_state::check_interrupt()
{
	m_maincpu->set_input_line(COSMAC_INPUT_LINE_INT, m_cr1 || m_int);
}

int comx35_state::clear_r()
{
	return m_clear;
}

int comx35_state::ef2_r()
{
	if (m_iden)
	{
		// interrupts disabled: PAL/NTSC
		return m_vis->pal_ntsc_r();
	}
	else
	{
		// interrupts enabled: keyboard repeat
		return m_kbe->rpt_r();
	}
}

int comx35_state::ef4_r()
{
	return m_exp->ef4_r() | ((m_cassette->input() > 0.0f) ? 1 : 0);
}

void comx35_state::q_w(int state)
{
	m_q = state;

	if (m_iden && state)
	{
		// enable interrupts
		m_iden = 0;
	}

	// cassette output
	m_cassette->output(state ? +1.0 : -1.0);

	// expansion bus
	m_exp->q_w(state);
}

void comx35_state::sc_w(uint8_t data)
{
	switch (data)
	{
	case COSMAC_STATE_CODE_S0_FETCH:
		// not connected
		break;

	case COSMAC_STATE_CODE_S1_EXECUTE:
		// every other S1 triggers a DMAOUT request
		if (m_dma)
		{
			m_dma = 0;

			if (!m_iden)
			{
				m_maincpu->set_input_line(COSMAC_INPUT_LINE_DMAOUT, ASSERT_LINE);
			}
		}
		else
		{
			m_dma = 1;
		}
		break;

	case COSMAC_STATE_CODE_S2_DMA:
		// DMA acknowledge clears the DMAOUT request
		m_maincpu->set_input_line(COSMAC_INPUT_LINE_DMAOUT, CLEAR_LINE);
		break;

	case COSMAC_STATE_CODE_S3_INTERRUPT:
		// interrupt acknowledge clears the INT request
		m_maincpu->set_input_line(COSMAC_INPUT_LINE_INT, CLEAR_LINE);
		break;
	}
}


//-------------------------------------------------
//  COMX_EXPANSION_INTERFACE( expansion_intf )
//-------------------------------------------------

void comx35_state::irq_w(int state)
{
	m_int = state;
	check_interrupt();
}



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  reset_done - flag that reset is complete
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(comx35_state::reset_done)
{
	m_clear = 1;
}


//-------------------------------------------------
//  machine_start
//-------------------------------------------------

void comx35_state::machine_start()
{
	// clear the RAM since DOS card will go crazy if RAM is not all zeroes
	uint8_t *ram = m_ram->pointer();
	memset(ram, 0, m_ram->size());

	// register for state saving
	save_item(NAME(m_clear));
	save_item(NAME(m_q));
	save_item(NAME(m_iden));
	save_item(NAME(m_dma));
	save_item(NAME(m_int));
	save_item(NAME(m_prd));
	save_item(NAME(m_cr1));

	m_reset_done_timer = timer_alloc(FUNC(comx35_state::reset_done), this);
}


void comx35_state::machine_reset()
{
	m_exp->reset();

	int t = RES_K(27) * CAP_U(1) * 1000; // t = R1 * C1

	m_clear = 0;
	m_iden = 1;
	m_cr1 = 1;
	m_int = CLEAR_LINE;
	m_prd = CLEAR_LINE;

	m_reset_done_timer->adjust(attotime::from_msec(t));
}



//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

//-------------------------------------------------
//  machine_config( pal )
//-------------------------------------------------

void comx35_state::base(machine_config &config, const XTAL clock)
{
	// basic system hardware
	CDP1802(config, m_maincpu, clock);
	m_maincpu->set_addrmap(AS_PROGRAM, &comx35_state::comx35_mem);
	m_maincpu->set_addrmap(AS_IO, &comx35_state::comx35_io);
	m_maincpu->wait_cb().set_constant(1);
	m_maincpu->clear_cb().set(FUNC(comx35_state::clear_r));
	m_maincpu->ef2_cb().set(FUNC(comx35_state::ef2_r));
	m_maincpu->ef4_cb().set(FUNC(comx35_state::ef4_r));
	m_maincpu->q_cb().set(FUNC(comx35_state::q_w));
	m_maincpu->sc_cb().set(FUNC(comx35_state::sc_w));
	m_maincpu->sc_cb().append(EXPANSION_TAG, FUNC(comx_expansion_slot_device::sc_w));
	m_maincpu->tpb_cb().set(EXPANSION_TAG, FUNC(comx_expansion_slot_device::tpb_w));

	// peripheral hardware
	CDP1871(config, m_kbe, cdp1869_device::CPU_CLK_PAL/8);
	m_kbe->d1_callback().set_ioport("D1");
	m_kbe->d2_callback().set_ioport("D2");
	m_kbe->d3_callback().set_ioport("D3");
	m_kbe->d4_callback().set_ioport("D4");
	m_kbe->d5_callback().set_ioport("D5");
	m_kbe->d6_callback().set_ioport("D6");
	m_kbe->d7_callback().set_ioport("D7");
	m_kbe->d8_callback().set_ioport("D8");
	m_kbe->d9_callback().set_ioport("D9");
	m_kbe->d10_callback().set_ioport("D10");
	m_kbe->d11_callback().set_ioport("D11");
	m_kbe->da_callback().set_inputline(m_maincpu, COSMAC_INPUT_LINE_EF3);

	QUICKLOAD(config, "quickload", "comx").set_load_callback(FUNC(comx35_state::quickload_cb));

	CASSETTE(config, m_cassette).set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);

	// expansion bus
	COMX_EXPANSION_SLOT(config, m_exp, 0, comx_expansion_cards, "eb").irq_callback().set(FUNC(comx35_state::irq_w));

	// internal ram
	RAM(config, m_ram).set_default_size("32K");

	// software lists
	SOFTWARE_LIST(config, "flop_list").set_original("comx35_flop");
}

void comx35_state::pal(machine_config &config)
{
	base(config, cdp1869_device::CPU_CLK_PAL);
	comx35_pal_video(config);
}

void comx35_state::ntsc(machine_config &config)
{
	base(config, cdp1869_device::CPU_CLK_NTSC);
	comx35_ntsc_video(config);
}



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( comx35p )
//-------------------------------------------------

ROM_START( comx35p )
	ROM_REGION( 0x4000, CDP1802_TAG, 0 )
	ROM_DEFAULT_BIOS( "basic100" )
	ROM_SYSTEM_BIOS( 0, "basic100", "COMX BASIC V1.00" )
	ROMX_LOAD( "comx_10.u21", 0x0000, 0x4000, CRC(68d0db2d) SHA1(062328361629019ceed9375afac18e2b7849ce47), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "basic101", "COMX BASIC V1.01" )
	ROMX_LOAD( "comx_11.u21", 0x0000, 0x4000, CRC(609d89cd) SHA1(799646810510d8236fbfafaff7a73d5170990f16), ROM_BIOS(1) )
ROM_END


//-------------------------------------------------
//  ROM( comx35n )
//-------------------------------------------------

#define rom_comx35n rom_comx35p



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME     PARENT   COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY                      FULLNAME          FLAGS
COMP( 1983, comx35p, 0,       0,      pal,     comx35, comx35_state, empty_init, "Comx World Operations Ltd", "COMX 35 (PAL)",  MACHINE_IMPERFECT_SOUND )
COMP( 1983, comx35n, comx35p, 0,      ntsc,    comx35, comx35_state, empty_init, "Comx World Operations Ltd", "COMX 35 (NTSC)", MACHINE_IMPERFECT_SOUND )
