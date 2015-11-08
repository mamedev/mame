// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    Sord m.5

    http://www.retropc.net/mm/m5/
    http://www.museo8bits.es/wiki/index.php/Sord_M5
    http://k5.web.klfree.net/content/view/10/11/
    http://k5.web.klfree.net/images/stories/sord/m5heap.htm

****************************************************************************/

/*

    TODO:

    - floppy
    - SI-5 serial interface (8251, ROM)
    - 64KB RAM expansions

*/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "formats/m5_dsk.h"
#include "formats/sord_cas.h"
#include "imagedev/cassette.h"
#include "bus/centronics/ctronics.h"
#include "machine/i8255.h"
#include "machine/ram.h"
#include "machine/upd765.h"
#include "machine/z80ctc.h"
#include "sound/sn76496.h"
#include "video/tms9928a.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "includes/m5.h"
#include "softlist.h"


//**************************************************************************
//  MEMORY BANKING
//**************************************************************************

WRITE_LINE_MEMBER( m5_state::write_centronics_busy )
{
	m_centronics_busy = state;
}

//-------------------------------------------------
//  sts_r -
//-------------------------------------------------

READ8_MEMBER( m5_state::sts_r )
{
	/*

	    bit     description

	    0       cassette input
	    1       busy
	    2
	    3
	    4
	    5
	    6
	    7       RESET key

	*/

	UINT8 data = 0;

	// cassette input
	data |= m_cassette->input() >= 0 ? 1 : 0;

	// centronics busy
	data |= m_centronics_busy << 1;

	// RESET key
	data |= m_reset->read();

	return data;
}


//-------------------------------------------------
//  com_w -
//-------------------------------------------------

WRITE8_MEMBER( m5_state::com_w )
{
	/*

	    bit     description

	    0       cassette output, centronics strobe
	    1       cassette remote
	    2
	    3
	    4
	    5
	    6
	    7

	*/

	// cassette output
	m_cassette->output( BIT(data, 0) ? -1.0 : 1.0);

	// centronics strobe
	m_centronics->write_strobe(BIT(data, 0));

	// cassette remote
	m_cassette->change_state(BIT(data,1) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
}



//**************************************************************************
//  FD-5
//**************************************************************************

//-------------------------------------------------
//  fd5_data_r -
//-------------------------------------------------

READ8_MEMBER( m5_state::fd5_data_r )
{
	m_ppi->pc6_w(0);

	return m_fd5_data;
}


//-------------------------------------------------
//  fd5_data_w -
//-------------------------------------------------

WRITE8_MEMBER( m5_state::fd5_data_w )
{
	m_fd5_data = data;

	m_ppi->pc4_w(0);
}


//-------------------------------------------------
//  fd5_com_r -
//-------------------------------------------------

READ8_MEMBER( m5_state::fd5_com_r )
{
	/*

	    bit     description

	    0       ?
	    1       1?
	    2       IBFA?
	    3       OBFA?
	    4
	    5
	    6
	    7

	*/

	return m_obfa << 3 | m_ibfa << 2 | 0x02;
}


//-------------------------------------------------
//  fd5_com_w -
//-------------------------------------------------

WRITE8_MEMBER( m5_state::fd5_com_w )
{
	/*

	    bit     description

	    0       PPI PC2
	    1       PPI PC0
	    2       PPI PC1
	    3
	    4
	    5
	    6
	    7

	*/

	m_fd5_com = data;
}


//-------------------------------------------------
//  fd5_com_w -
//-------------------------------------------------

WRITE8_MEMBER( m5_state::fd5_ctrl_w )
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4
	    5
	    6
	    7

	*/

	m_floppy0->mon_w(!BIT(data, 0));
}


//-------------------------------------------------
//  fd5_com_w -
//-------------------------------------------------

WRITE8_MEMBER( m5_state::fd5_tc_w )
{
	m_fdc->tc_w(true);
	m_fdc->tc_w(false);
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( m5_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( m5_mem, AS_PROGRAM, 8, m5_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	//AM_RANGE(0x2000, 0x6fff)      // mapped by the cartslot
	AM_RANGE(0x7000, 0x7fff) AM_RAM
	AM_RANGE(0x8000, 0xffff) AM_RAM
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( m5_io )
//-------------------------------------------------

static ADDRESS_MAP_START( m5_io, AS_IO, 8, m5_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_MIRROR(0x0c) AM_DEVREADWRITE(Z80CTC_TAG, z80ctc_device, read, write)
	AM_RANGE(0x10, 0x10) AM_MIRROR(0x0e) AM_DEVREADWRITE("tms9928a", tms9928a_device, vram_read, vram_write)
	AM_RANGE(0x11, 0x11) AM_MIRROR(0x0e) AM_DEVREADWRITE("tms9928a", tms9928a_device, register_read, register_write)
	AM_RANGE(0x20, 0x20) AM_MIRROR(0x0f) AM_DEVWRITE(SN76489AN_TAG, sn76489a_device, write)
	AM_RANGE(0x30, 0x30) AM_MIRROR(0x08) AM_READ_PORT("Y0") // 64KBF bank select
	AM_RANGE(0x31, 0x31) AM_MIRROR(0x08) AM_READ_PORT("Y1")
	AM_RANGE(0x32, 0x32) AM_MIRROR(0x08) AM_READ_PORT("Y2")
	AM_RANGE(0x33, 0x33) AM_MIRROR(0x08) AM_READ_PORT("Y3")
	AM_RANGE(0x34, 0x34) AM_MIRROR(0x08) AM_READ_PORT("Y4")
	AM_RANGE(0x35, 0x35) AM_MIRROR(0x08) AM_READ_PORT("Y5")
	AM_RANGE(0x36, 0x36) AM_MIRROR(0x08) AM_READ_PORT("Y6")
	AM_RANGE(0x37, 0x37) AM_MIRROR(0x08) AM_READ_PORT("JOY")
	AM_RANGE(0x40, 0x40) AM_MIRROR(0x0f) AM_DEVWRITE("cent_data_out", output_latch_device, write)
	AM_RANGE(0x50, 0x50) AM_MIRROR(0x0f) AM_READWRITE(sts_r, com_w)
//  AM_RANGE(0x60, 0x63) SIO
//  AM_RANGE(0x6c, 0x6c) EM-64/64KBI bank select
	AM_RANGE(0x70, 0x73) AM_MIRROR(0x0c) AM_DEVREADWRITE(I8255A_TAG, i8255_device, read, write)
//  AM_RANGE(0x7f, 0x7f) 64KRD/64KRX bank select
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( fd5_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( fd5_mem, AS_PROGRAM, 8, m5_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0xffff) AM_RAM
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( fd5_io )
//-------------------------------------------------

static ADDRESS_MAP_START( fd5_io, AS_IO, 8, m5_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVICE(UPD765_TAG, upd765a_device, map)
	AM_RANGE(0x10, 0x10) AM_READWRITE(fd5_data_r, fd5_data_w)
	AM_RANGE(0x20, 0x20) AM_WRITE(fd5_com_w)
	AM_RANGE(0x30, 0x30) AM_READ(fd5_com_r)
	AM_RANGE(0x40, 0x40) AM_WRITE(fd5_ctrl_w)
	AM_RANGE(0x50, 0x50) AM_WRITE(fd5_tc_w)
ADDRESS_MAP_END



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( m5 )
//-------------------------------------------------

static INPUT_PORTS_START( m5 )
	PORT_START("Y0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Func") PORT_CODE(KEYCODE_TAB) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)      PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT)      PORT_CHAR(UCHAR_MAMEKEY(RSHIFT))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)       PORT_CHAR(' ')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)       PORT_CHAR(13)

	PORT_START("Y1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)           PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)           PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)           PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)           PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)           PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)           PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)           PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)           PORT_CHAR('8') PORT_CHAR('(')

	PORT_START("Y2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)           PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)           PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)           PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)           PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)           PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)           PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)           PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)           PORT_CHAR('i') PORT_CHAR('I')

	PORT_START("Y3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)           PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)           PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)           PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)           PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)           PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)           PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)           PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)           PORT_CHAR('k') PORT_CHAR('K')

	PORT_START("Y4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)           PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)           PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)           PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)           PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)           PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)           PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)           PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)       PORT_CHAR(',') PORT_CHAR('<')

	PORT_START("Y5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)           PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)           PORT_CHAR('0')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)       PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)  PORT_CHAR('^') PORT_CHAR('~')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)        PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)       PORT_CHAR('/') PORT_CHAR('?') PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("_  Triangle") PORT_CODE(KEYCODE_TILDE) PORT_CHAR('_')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2)  PORT_CHAR('\\') PORT_CHAR('|')

	PORT_START("Y6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)           PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)           PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)   PORT_CHAR('@') PORT_CHAR('`') PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)  PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)           PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)       PORT_CHAR(';') PORT_CHAR('+') PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)       PORT_CHAR(':') PORT_CHAR('*') PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)   PORT_CHAR(']') PORT_CHAR('}')

	PORT_START("JOY")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP)    PORT_PLAYER(1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT)  PORT_PLAYER(1)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN)  PORT_PLAYER(1)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP)    PORT_PLAYER(2)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT)  PORT_PLAYER(2)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN)  PORT_PLAYER(2)

	PORT_START("RESET")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Reset") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC)) /* 1st line, 1st key from right! */
INPUT_PORTS_END

//-------------------------------------------------
//  TMS9928a_interface vdp_intf
//-------------------------------------------------

WRITE_LINE_MEMBER(m5_state::sordm5_video_interrupt_callback)
{
	if (state)
	{
		m_ctc->trg3(1);
		m_ctc->trg3(0);
	}
}


//-------------------------------------------------
//  I8255 Interface
//-------------------------------------------------

READ8_MEMBER( m5_state::ppi_pa_r )
{
	return m_fd5_data;
}

READ8_MEMBER( m5_state::ppi_pc_r )
{
	/*

	    bit     description

	    0       ?
	    1       ?
	    2       ?
	    3
	    4       STBA
	    5
	    6       ACKA
	    7

	*/

	return (
			/* FD5 bit 0-> M5 bit 2 */
			((m_fd5_com & 0x01)<<2) |
			/* FD5 bit 2-> M5 bit 1 */
			((m_fd5_com & 0x04)>>1) |
			/* FD5 bit 1-> M5 bit 0 */
			((m_fd5_com & 0x02)>>1)
	);
}

WRITE8_MEMBER( m5_state::ppi_pa_w )
{
	m_fd5_data = data;
}

WRITE8_MEMBER( m5_state::ppi_pb_w )
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4
	    5
	    6
	    7

	*/

	if (data == 0xf0)
	{
		m_fd5cpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
		m_fd5cpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
	}
}

WRITE8_MEMBER( m5_state::ppi_pc_w )
{
	/*

	    bit     description

	    0
	    1
	    2
	    3       INTRA
	    4
	    5       IBFA
	    6
	    7       OBFA

	*/

	m_intra = BIT(data, 3);
	m_ibfa = BIT(data, 5);
	m_obfa = BIT(data, 7);
}

//-------------------------------------------------
//  upd765_interface fdc_intf
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER( m5_state::floppy_formats )
	FLOPPY_M5_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( m5_floppies )
		SLOT_INTERFACE( "525dd", FLOPPY_525_DD )
SLOT_INTERFACE_END

//-------------------------------------------------
//  z80_daisy_config m5_daisy_chain
//-------------------------------------------------

static const z80_daisy_config m5_daisy_chain[] =
{
	{ Z80CTC_TAG },
	{ NULL }
};



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_START( m5 )
//-------------------------------------------------

void m5_state::machine_start()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	// configure RAM
	switch (m_ram->size())
	{
	case 4*1024:
		program.unmap_readwrite(0x8000, 0xffff);
		break;

	case 36*1024:
		break;

	case 68*1024:
		break;
	}

	if (m_cart->exists())
		program.install_read_handler(0x2000, 0x6fff, read8_delegate(FUNC(generic_slot_device::read_rom),(generic_slot_device*)m_cart));

	// register for state saving
	save_item(NAME(m_fd5_data));
	save_item(NAME(m_fd5_com));
	save_item(NAME(m_intra));
	save_item(NAME(m_ibfa));
	save_item(NAME(m_obfa));
}


void m5_state::machine_reset()
{
}



//**************************************************************************
//  MACHINE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_CONFIG( m5 )
//-------------------------------------------------

static MACHINE_CONFIG_START( m5, m5_state )
	// basic machine hardware
	MCFG_CPU_ADD(Z80_TAG, Z80, XTAL_14_31818MHz/4)
	MCFG_CPU_PROGRAM_MAP(m5_mem)
	MCFG_CPU_IO_MAP(m5_io)
	MCFG_CPU_CONFIG(m5_daisy_chain)

	MCFG_CPU_ADD(Z80_FD5_TAG, Z80, XTAL_14_31818MHz/4)
	MCFG_CPU_PROGRAM_MAP(fd5_mem)
	MCFG_CPU_IO_MAP(fd5_io)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SN76489AN_TAG, SN76489A, XTAL_14_31818MHz/4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	// devices
	MCFG_DEVICE_ADD(Z80CTC_TAG, Z80CTC, XTAL_14_31818MHz/4)
	MCFG_Z80CTC_INTR_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))
	// CK0 = EXINT, CK1 = GND, CK2 = TCK, CK3 = VDP INT
	// ZC2 = EXCLK

	MCFG_CENTRONICS_ADD(CENTRONICS_TAG, centronics_devices, "printer")
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(m5_state, write_centronics_busy))

	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", CENTRONICS_TAG)

	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_FORMATS(sordm5_cassette_formats)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_PLAY)
	MCFG_CASSETTE_INTERFACE("m5_cass")

	MCFG_DEVICE_ADD(I8255A_TAG, I8255, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(m5_state, ppi_pa_r))
	MCFG_I8255_OUT_PORTA_CB(WRITE8(m5_state, ppi_pa_w))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(m5_state, ppi_pb_w))
	MCFG_I8255_IN_PORTC_CB(READ8(m5_state, ppi_pc_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(m5_state, ppi_pc_w))

	MCFG_UPD765A_ADD(UPD765_TAG, true, true)
	MCFG_UPD765_INTRQ_CALLBACK(INPUTLINE(Z80_FD5_TAG, INPUT_LINE_IRQ0))
	MCFG_FLOPPY_DRIVE_ADD(UPD765_TAG ":0", m5_floppies, "525dd", m5_state::floppy_formats)

	// cartridge
	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "m5_cart")
	MCFG_GENERIC_EXTENSIONS("bin,rom")
	//MCFG_GENERIC_MANDATORY

	// software lists
	MCFG_SOFTWARE_LIST_ADD("cart_list", "m5_cart")
	MCFG_SOFTWARE_LIST_ADD("cass_list", "m5_cass")

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("4K")
	MCFG_RAM_EXTRA_OPTIONS("36K,68K")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG_DERIVED( ntsc, m5 )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( ntsc, m5 )
	// video hardware
	MCFG_DEVICE_ADD( "tms9928a", TMS9928A, XTAL_10_738635MHz / 2 )
	MCFG_TMS9928A_VRAM_SIZE(0x4000)
	MCFG_TMS9928A_OUT_INT_LINE_CB(WRITELINE(m5_state, sordm5_video_interrupt_callback))
	MCFG_TMS9928A_SCREEN_ADD_NTSC( "screen" )
	MCFG_SCREEN_UPDATE_DEVICE( "tms9928a", tms9928a_device, screen_update )
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG_DERIVED( pal, m5 )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( pal, m5 )
	// video hardware
	MCFG_DEVICE_ADD( "tms9928a", TMS9929A, XTAL_10_738635MHz / 2 )
	MCFG_TMS9928A_VRAM_SIZE(0x4000)
	MCFG_TMS9928A_OUT_INT_LINE_CB(WRITELINE(m5_state, sordm5_video_interrupt_callback))
	MCFG_TMS9928A_SCREEN_ADD_PAL( "screen" )
	MCFG_SCREEN_UPDATE_DEVICE( "tms9928a", tms9928a_device, screen_update )
MACHINE_CONFIG_END



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( m5 )
//-------------------------------------------------

ROM_START( m5 )
	ROM_REGION( 0x7000, Z80_TAG, ROMREGION_ERASEFF )
	ROM_LOAD( "sordjap.ic21", 0x0000, 0x2000, CRC(92cf9353) SHA1(b0a4b3658fde68cb1f344dfb095bac16a78e9b3e) )

	ROM_REGION( 0x4000, Z80_FD5_TAG, 0 )
	ROM_LOAD( "sordfd5.rom", 0x0000, 0x4000, CRC(7263bbc5) SHA1(b729500d3d2b2e807d384d44b76ea5ad23996f4a))
ROM_END


//-------------------------------------------------
//  ROM( m5p )
//-------------------------------------------------

ROM_START( m5p )
	ROM_REGION( 0x7000, Z80_TAG, ROMREGION_ERASEFF )
	ROM_LOAD( "sordint.ic21", 0x0000, 0x2000, CRC(78848d39) SHA1(ac042c4ae8272ad6abe09ae83492ef9a0026d0b2) )

	ROM_REGION( 0x4000, Z80_FD5_TAG, 0 )
	ROM_LOAD( "sordfd5.rom", 0x0000, 0x4000, CRC(7263bbc5) SHA1(b729500d3d2b2e807d384d44b76ea5ad23996f4a))
ROM_END



//**************************************************************************
//  DRIVER INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  ROM( ntsc )
//-------------------------------------------------

DRIVER_INIT_MEMBER(m5_state,ntsc)
{
}


//-------------------------------------------------
//  ROM( pal )
//-------------------------------------------------

DRIVER_INIT_MEMBER(m5_state,pal)
{
}



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME    PARENT  COMPAT  MACHINE INPUT   INIT    COMPANY     FULLNAME            FLAGS
COMP( 1983, m5,     0,      0,      ntsc,   m5, m5_state,       ntsc,   "Sord",     "m.5 (Japan)",      0 )
COMP( 1983, m5p,    m5,     0,      pal,    m5, m5_state,       pal,    "Sord",     "m.5 (Europe)",     0 )
