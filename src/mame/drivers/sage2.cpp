// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

        Sage II

        For memory map look at :
            http://www.thebattles.net/sage/img/SDT.pdf  (pages 14-)


        06/12/2009 Skeleton driver.

****************************************************************************/

/*

    TODO:

    - floppy loading
    - TMS9914 IEEE-488 controller
    - board 2 (4x 2651 USART)
    - Winchester controller

*/

#include "includes/sage2.h"
#include "bus/rs232/rs232.h"
#include "softlist.h"

//**************************************************************************
//  MEMORY MANAGEMENT UNIT
//**************************************************************************

//-------------------------------------------------
//  read -
//-------------------------------------------------

READ8_MEMBER( sage2_state::read )
{
	UINT8 data = 0xff;

	if (m_reset || (offset >= 0xfe0000 && offset < 0xff4000))
	{
		data = m_rom[offset & 0x1fff];
	}
	else if (offset < 0x080000)
	{
		data = m_ram->pointer()[offset];
	}

	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

WRITE8_MEMBER( sage2_state::write )
{
	if (offset < 0x080000)
	{
		m_ram->pointer()[offset] = data;
	}
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( sage2_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( sage2_mem, AS_PROGRAM, 16, sage2_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0xfeffff) AM_READWRITE8(read, write, 0xffff)
	AM_RANGE(0xffc000, 0xffc007) AM_DEVREADWRITE8(I8253_1_TAG, pit8253_device, read, write, 0x00ff)
	AM_RANGE(0xffc010, 0xffc01f) AM_NOP //AM_DEVREADWRITE8(TMS9914_TAG, tms9914_device, read, write, 0x00ff)
	AM_RANGE(0xffc020, 0xffc027) AM_DEVREADWRITE8(I8255A_0_TAG, i8255_device, read, write, 0x00ff) // i8255, DIPs + Floppy ctrl port
	AM_RANGE(0xffc030, 0xffc031) AM_DEVREADWRITE8(I8251_1_TAG, i8251_device, data_r, data_w, 0x00ff)
	AM_RANGE(0xffc032, 0xffc033) AM_DEVREADWRITE8(I8251_1_TAG, i8251_device, status_r, control_w, 0x00ff)
	AM_RANGE(0xffc040, 0xffc043) AM_DEVREADWRITE8(I8259_TAG, pic8259_device, read, write, 0x00ff)
	AM_RANGE(0xffc050, 0xffc053) AM_DEVICE8(UPD765_TAG, upd765a_device, map, 0x00ff)
	AM_RANGE(0xffc060, 0xffc067) AM_DEVREADWRITE8(I8255A_1_TAG, i8255_device, read, write, 0x00ff) // i8255, Printer
	AM_RANGE(0xffc070, 0xffc071) AM_DEVREADWRITE8(I8251_0_TAG, i8251_device, data_r, data_w, 0x00ff)
	AM_RANGE(0xffc072, 0xffc073) AM_DEVREADWRITE8(I8251_0_TAG, i8251_device, status_r, control_w, 0x00ff)
	AM_RANGE(0xffc080, 0xffc087) AM_MIRROR(0x78) AM_DEVREADWRITE8(I8253_0_TAG, pit8253_device, read, write, 0x00ff)
//  AM_RANGE(0xffc400, 0xffc407) AM_DEVREADWRITE8(S2651_0_TAG, s2651_device, read, write, 0x00ff)
//  AM_RANGE(0xffc440, 0xffc447) AM_DEVREADWRITE8(S2651_1_TAG, s2651_device, read, write, 0x00ff)
//  AM_RANGE(0xffc480, 0xffc487) AM_DEVREADWRITE8(S2651_2_TAG, s2651_device, read, write, 0x00ff)
//  AM_RANGE(0xffc4c0, 0xffc4c7) AM_DEVREADWRITE8(S2651_3_TAG, s2651_device, read, write, 0x00ff)
//  AM_RANGE(0xffc500, 0xffc7ff) // Winchester drive ports
ADDRESS_MAP_END



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( sage2 )
//-------------------------------------------------

static INPUT_PORTS_START( sage2 )
	PORT_START("J7")
	PORT_DIPNAME( 0x07, 0x07, "Terminal Baud Rate" ) PORT_DIPLOCATION("J7:1,2,3")
	PORT_DIPSETTING(    0x07, "19200" )
	PORT_DIPSETTING(    0x06, "9600" )
	PORT_DIPSETTING(    0x05, "4800" )
	PORT_DIPSETTING(    0x04, "2400" )
	PORT_DIPSETTING(    0x03, "1200" )
	PORT_DIPSETTING(    0x02, "600" )
	PORT_DIPSETTING(    0x01, "300" )
	PORT_DIPSETTING(    0x00, "Reserved (19200)" )
	PORT_DIPNAME( 0x08, 0x08, "Parity Control" ) PORT_DIPLOCATION("J7:4")
	PORT_DIPSETTING(    0x08, "Even Parity" )
	PORT_DIPSETTING(    0x00, "Disabled" )
	PORT_DIPNAME( 0x30, 0x20, "Boot Device" ) PORT_DIPLOCATION("J7:5,6")
	PORT_DIPSETTING(    0x30, "Debugger" )
	PORT_DIPSETTING(    0x20, "Floppy Drive 0" )
	PORT_DIPSETTING(    0x10, "Winchester" )
	PORT_DIPSETTING(    0x00, "Reserved (Debugger)" )
	PORT_DIPNAME( 0x40, 0x40, "Floppy Configuration" ) PORT_DIPLOCATION("J7:7")
	PORT_DIPSETTING(    0x40, "96 TPI" )
	PORT_DIPSETTING(    0x00, "48 TPI" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Service_Mode ) ) PORT_DIPLOCATION("J7:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("J6")
	PORT_DIPNAME( 0x1f, 0x07, "IEEE-488 Bus Address" ) PORT_DIPLOCATION("J6:1,2,3,4,5")
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x06, "6" )
	PORT_DIPSETTING(    0x07, "7" )
	PORT_DIPSETTING(    0x08, "8" )
	PORT_DIPSETTING(    0x09, "9" )
	PORT_DIPSETTING(    0x0a, "10" )
	PORT_DIPSETTING(    0x0b, "11" )
	PORT_DIPSETTING(    0x0c, "12" )
	PORT_DIPSETTING(    0x0d, "13" )
	PORT_DIPSETTING(    0x0e, "14" )
	PORT_DIPSETTING(    0x0f, "15" )
	PORT_DIPSETTING(    0x10, "16" )
	PORT_DIPSETTING(    0x11, "17" )
	PORT_DIPSETTING(    0x12, "18" )
	PORT_DIPSETTING(    0x13, "19" )
	PORT_DIPSETTING(    0x14, "20" )
	PORT_DIPSETTING(    0x15, "21" )
	PORT_DIPSETTING(    0x16, "22" )
	PORT_DIPSETTING(    0x17, "23" )
	PORT_DIPSETTING(    0x18, "24" )
	PORT_DIPSETTING(    0x19, "25" )
	PORT_DIPSETTING(    0x1a, "26" )
	PORT_DIPSETTING(    0x1b, "27" )
	PORT_DIPSETTING(    0x1c, "28" )
	PORT_DIPSETTING(    0x1d, "29" )
	PORT_DIPSETTING(    0x1e, "30" )
	PORT_DIPSETTING(    0x1f, "31" )
	PORT_DIPNAME( 0x20, 0x00, "IEEE-488 TALK" ) PORT_DIPLOCATION("J6:6")
	PORT_DIPSETTING(    0x00, "Disabled" )
	PORT_DIPSETTING(    0x20, "Enabled" )
	PORT_DIPNAME( 0x40, 0x00, "IEEE-488 LISTEN" ) PORT_DIPLOCATION("J6:7")
	PORT_DIPSETTING(    0x00, "Disabled" )
	PORT_DIPSETTING(    0x40, "Enabled" )
	PORT_DIPNAME( 0x80, 0x00, "IEEE-488 Consecutive Addresses" ) PORT_DIPLOCATION("J6:8")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x80, "2" )
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  I8255A INTERFACE( ppi0_intf )
//-------------------------------------------------

/*

    IR0     U74 OUT2
    IR1     RX2I+
    IR2     TX1I+
    IR3     TX2I+
    IR4     MI-
    IR5     CNI+
    IR6     U74 OUT0
    IR7     SI+

*/


//-------------------------------------------------
//  I8255A INTERFACE( ppi0_intf )
//-------------------------------------------------

WRITE8_MEMBER( sage2_state::ppi0_pc_w )
{
	/*

	    bit     signal

	    PC0     TC+
	    PC1     RDY+
	    PC2     FDIE+
	    PC3     SL0-
	    PC4     SL1-
	    PC5     MOT-
	    PC6     PCRMP-
	    PC7     FRES+

	*/

	// floppy terminal count
	m_fdc->tc_w(BIT(data, 0));

	// floppy ready
	m_fdc->ready_w(BIT(data, 1));

	// floppy interrupt enable
	m_fdie = BIT(data, 2);
	update_fdc_int();

	// drive select
	m_floppy = NULL;

	if (!BIT(data, 3)) m_floppy = m_floppy0->get_device();
	if (!BIT(data, 4)) m_floppy = m_floppy1->get_device();

	m_fdc->set_floppy(m_floppy);

	// floppy motor
	if (m_floppy) m_floppy->mon_w(BIT(data, 5));

	// FDC reset
	if(BIT(data, 7)) m_fdc->reset();
}


//-------------------------------------------------
//  I8255A INTERFACE( ppi1_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER(sage2_state::write_centronics_ack)
{
	if (!state)
	{
		m_pic->ir5_w(ASSERT_LINE);
	}
}

WRITE_LINE_MEMBER(sage2_state::write_centronics_busy)
{
	m_centronics_busy = state;
}

WRITE_LINE_MEMBER(sage2_state::write_centronics_perror)
{
	m_centronics_perror = state;
}

WRITE_LINE_MEMBER(sage2_state::write_centronics_select)
{
	m_centronics_select = state;
}

WRITE_LINE_MEMBER(sage2_state::write_centronics_fault)
{
	m_centronics_fault = state;
}

READ8_MEMBER( sage2_state::ppi1_pb_r )
{
	/*

	    bit     signal

	    PB0     FDI+
	    PB1     WP+
	    PB2     RG-
	    PB3     CD-
	    PB4     BUSY
	    PB5     PAPER
	    PB6     SEL
	    PB7     FAULT-

	*/

	UINT8 data = 0;

	// floppy interrupt
	data = m_fdc->get_irq();

	// floppy write protected
	data = (m_floppy ? m_floppy->wpt_r() : 1) << 1;

	// RS-232 ring indicator

	// RS-232 carrier detect

	// centronics
	data |= m_centronics_busy << 4;
	data |= m_centronics_perror << 5;
	data |= m_centronics_select << 6;
	data |= m_centronics_fault << 7;

	return data;
}

WRITE8_MEMBER( sage2_state::ppi1_pc_w )
{
	/*

	    bit     signal

	    PC0     PRES-
	    PC1     U8 SC+
	    PC2     SI+
	    PC3     LEDR+
	    PC4     STROBE-
	    PC5     PRIME-
	    PC6     U38 CL-
	    PC7     RMI-

	*/

	if (!BIT(data, 0))
	{
		// clear parity error interrupt
		m_maincpu->set_input_line(M68K_IRQ_7, CLEAR_LINE);
	}

	// s? interrupt
	m_pic->ir7_w(BIT(data, 2));

	// processor LED
	output_set_led_value(0, BIT(data, 3));

	// centronics
	m_centronics->write_strobe(BIT(data, 4));
	m_centronics->write_init(BIT(data, 5));

	if (!BIT(data, 6))
	{
		// clear ACK interrupt
		m_pic->ir5_w(CLEAR_LINE);
	}

	if (!BIT(data, 7))
	{
		// clear modem interrupt
		m_pic->ir4_w(CLEAR_LINE);
	}
}

WRITE_LINE_MEMBER( sage2_state::br1_w )
{
	m_usart0->write_txc(state);
	m_usart0->write_rxc(state);
}

WRITE_LINE_MEMBER( sage2_state::br2_w )
{
	m_usart1->write_txc(state);
	m_usart1->write_rxc(state);
}

//-------------------------------------------------
//  upd765_interface fdc_intf
//-------------------------------------------------

static SLOT_INTERFACE_START( sage2_floppies )
	SLOT_INTERFACE( "525qd", FLOPPY_525_QD ) // Mitsubishi M4859
SLOT_INTERFACE_END

void sage2_state::update_fdc_int()
{
	m_maincpu->set_input_line(M68K_IRQ_6, m_fdie && m_fdc_int);
}

WRITE_LINE_MEMBER( sage2_state::fdc_irq )
{
	m_fdc_int = state;
	update_fdc_int();
}


static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_19200 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_19200 )
	DEVICE_INPUT_DEFAULTS( "RS232_STARTBITS", 0xff, RS232_STARTBITS_1 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_7 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_EVEN )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_START( sage2 )
//-------------------------------------------------

void sage2_state::machine_start()
{
	// find memory regions
	m_rom = memregion(M68000_TAG)->base();
}


void sage2_state::machine_reset()
{
	m_reset = 1;
}



//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

//-------------------------------------------------
//  MACHINE_CONFIG( sage2 )
//-------------------------------------------------

static MACHINE_CONFIG_START( sage2, sage2_state )
	// basic machine hardware
	MCFG_CPU_ADD(M68000_TAG, M68000, XTAL_16MHz/2)
	MCFG_CPU_PROGRAM_MAP(sage2_mem)

	// devices
	MCFG_PIC8259_ADD(I8259_TAG, INPUTLINE(M68000_TAG, M68K_IRQ_1), VCC, NULL)

	MCFG_DEVICE_ADD(I8255A_0_TAG, I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(IOPORT("J7"))
	MCFG_I8255_IN_PORTB_CB(IOPORT("J6"))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(sage2_state, ppi0_pc_w))

	MCFG_DEVICE_ADD(I8255A_1_TAG, I8255A, 0)
	MCFG_I8255_OUT_PORTA_CB(DEVWRITE8("cent_data_out", output_latch_device, write))
	MCFG_I8255_IN_PORTB_CB(READ8(sage2_state, ppi1_pb_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(sage2_state, ppi1_pc_w))

	MCFG_DEVICE_ADD(I8253_0_TAG, PIT8253, 0)
	MCFG_PIT8253_CLK0(0) // from U75 OUT0
	MCFG_PIT8253_OUT0_HANDLER(DEVWRITELINE(I8259_TAG, pic8259_device, ir6_w))
	MCFG_PIT8253_CLK1(XTAL_16MHz/2/125)
	MCFG_PIT8253_OUT1_HANDLER(DEVWRITELINE(I8253_0_TAG, pit8253_device, write_clk2))
	MCFG_PIT8253_CLK2(0) // from OUT2
	MCFG_PIT8253_OUT2_HANDLER(DEVWRITELINE(I8259_TAG, pic8259_device, ir0_w))

	MCFG_DEVICE_ADD(I8253_1_TAG, PIT8253, 0)
	MCFG_PIT8253_CLK0(XTAL_16MHz/2/125)
	MCFG_PIT8253_OUT0_HANDLER(DEVWRITELINE(I8253_0_TAG, pit8253_device, write_clk0))
	MCFG_PIT8253_CLK1(XTAL_16MHz/2/13)
	MCFG_PIT8253_OUT1_HANDLER(WRITELINE(sage2_state, br1_w))
	MCFG_PIT8253_CLK2(XTAL_16MHz/2/13)
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(sage2_state, br2_w))

	MCFG_DEVICE_ADD(I8251_0_TAG, I8251, 0)
	MCFG_I8251_TXD_HANDLER(DEVWRITELINE(RS232_A_TAG, rs232_port_device, write_txd))
	MCFG_I8251_DTR_HANDLER(DEVWRITELINE(RS232_A_TAG, rs232_port_device, write_dtr))
	MCFG_I8251_RTS_HANDLER(DEVWRITELINE(RS232_A_TAG, rs232_port_device, write_rts))
	MCFG_I8251_RXRDY_HANDLER(DEVWRITELINE(M68000_TAG, m68000_base_device, write_irq5))
	MCFG_I8251_TXRDY_HANDLER(DEVWRITELINE(I8259_TAG, pic8259_device, ir2_w))

	MCFG_RS232_PORT_ADD(RS232_A_TAG, default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(I8251_0_TAG, i8251_device, write_rxd))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE(I8251_0_TAG, i8251_device, write_dsr))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(I8251_0_TAG, i8251_device, write_cts))
	MCFG_DEVICE_CARD_DEVICE_INPUT_DEFAULTS("terminal", terminal)

	MCFG_DEVICE_ADD(I8251_1_TAG, I8251, 0)
	MCFG_I8251_TXD_HANDLER(DEVWRITELINE(RS232_B_TAG, rs232_port_device, write_txd))
	MCFG_I8251_DTR_HANDLER(DEVWRITELINE(RS232_B_TAG, rs232_port_device, write_dtr))
	MCFG_I8251_RTS_HANDLER(DEVWRITELINE(RS232_B_TAG, rs232_port_device, write_rts))
	MCFG_I8251_RXRDY_HANDLER(DEVWRITELINE(I8259_TAG, pic8259_device, ir1_w))
	MCFG_I8251_TXRDY_HANDLER(DEVWRITELINE(I8259_TAG, pic8259_device, ir3_w))

	MCFG_RS232_PORT_ADD(RS232_B_TAG, default_rs232_devices, NULL)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(I8251_1_TAG, i8251_device, write_rxd))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE(I8251_1_TAG, i8251_device, write_dsr))

	MCFG_UPD765A_ADD(UPD765_TAG, false, false)
	MCFG_UPD765_INTRQ_CALLBACK(WRITELINE(sage2_state, fdc_irq))

	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_ACK_HANDLER(WRITELINE(sage2_state, write_centronics_ack))
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(sage2_state, write_centronics_busy))
	MCFG_CENTRONICS_PERROR_HANDLER(WRITELINE(sage2_state, write_centronics_perror))
	MCFG_CENTRONICS_SELECT_HANDLER(WRITELINE(sage2_state, write_centronics_select))
	MCFG_CENTRONICS_FAULT_HANDLER(WRITELINE(sage2_state, write_centronics_fault))

	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", "centronics")

	MCFG_FLOPPY_DRIVE_ADD(UPD765_TAG ":0", sage2_floppies, "525qd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(UPD765_TAG ":1", sage2_floppies, "525qd", floppy_image_device::default_floppy_formats)
	MCFG_IEEE488_BUS_ADD()

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("512K")

	// software list
	MCFG_SOFTWARE_LIST_ADD("flop_list", "sage2")
MACHINE_CONFIG_END



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( sage2 )
//-------------------------------------------------

ROM_START( sage2 )
	ROM_REGION( 0x2000, M68000_TAG, 0 )
	ROM_LOAD16_BYTE( "sage2.u18", 0x0000, 0x1000, CRC(ca9b312d) SHA1(99436a6d166aa5280c3b2d28355c4d20528fe48c) )
	ROM_LOAD16_BYTE( "sage2.u17", 0x0001, 0x1000, CRC(27e25045) SHA1(041cd9d4617473d089f31f18cbb375046c3b61bb) )
ROM_END



//**************************************************************************
//  DRIVER INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  DRIVER_INIT( sage2 )
//-------------------------------------------------

DIRECT_UPDATE_MEMBER(sage2_state::sage2_direct_update_handler)
{
	if (m_reset && address >= 0xfe0000)
	{
		m_reset = 0;
	}

	return address;
}

DRIVER_INIT_MEMBER(sage2_state,sage2)
{
	address_space &program = machine().device<cpu_device>(M68000_TAG)->space(AS_PROGRAM);
	program.set_direct_update_handler(direct_update_delegate(FUNC(sage2_state::sage2_direct_update_handler), this));
}



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME    PARENT  COMPAT  MACHINE INPUT   INIT    COMPANY                             FULLNAME    FLAGS
COMP( 1982, sage2,  0,       0,      sage2,     sage2, sage2_state,    sage2, "Sage Technology", "Sage II", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
