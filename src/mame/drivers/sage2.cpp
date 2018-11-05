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

#include "emu.h"
#include "includes/sage2.h"
#include "bus/rs232/rs232.h"
#include "softlist.h"

//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( sage2_mem )
//-------------------------------------------------

void sage2_state::sage2_mem(address_map &map)
{
	map.unmap_value_high();
	map(0xffc000, 0xffc007).rw(I8253_1_TAG, FUNC(pit8253_device::read), FUNC(pit8253_device::write)).umask16(0x00ff);
	map(0xffc010, 0xffc01f).noprw(); //AM_DEVREADWRITE8(TMS9914_TAG, tms9914_device, read, write, 0x00ff)
	map(0xffc020, 0xffc027).rw(I8255A_0_TAG, FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0x00ff); // i8255, DIPs + Floppy ctrl port
	map(0xffc030, 0xffc033).rw(m_usart1, FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0x00ff);
	map(0xffc040, 0xffc043).rw(m_pic, FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
	map(0xffc050, 0xffc053).m(m_fdc, FUNC(upd765a_device::map)).umask16(0x00ff);
	map(0xffc060, 0xffc067).rw(I8255A_1_TAG, FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0x00ff); // i8255, Printer
	map(0xffc070, 0xffc073).rw(m_usart0, FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0x00ff);
	map(0xffc080, 0xffc087).mirror(0x78).rw(I8253_0_TAG, FUNC(pit8253_device::read), FUNC(pit8253_device::write)).umask16(0x00ff);
//  AM_RANGE(0xffc400, 0xffc407) AM_DEVREADWRITE8(S2651_0_TAG, s2651_device, read, write, 0x00ff)
//  AM_RANGE(0xffc440, 0xffc447) AM_DEVREADWRITE8(S2651_1_TAG, s2651_device, read, write, 0x00ff)
//  AM_RANGE(0xffc480, 0xffc487) AM_DEVREADWRITE8(S2651_2_TAG, s2651_device, read, write, 0x00ff)
//  AM_RANGE(0xffc4c0, 0xffc4c7) AM_DEVREADWRITE8(S2651_3_TAG, s2651_device, read, write, 0x00ff)
//  AM_RANGE(0xffc500, 0xffc7ff) // Winchester drive ports
}



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
	m_floppy = nullptr;

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

	uint8_t data = 0;

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
	m_led = BIT(data, 3);

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

static void sage2_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD); // Mitsubishi M4859
}

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
	m_led.resolve();
	address_space &program = m_maincpu->space(AS_PROGRAM);
	program.install_rom(0x000000, 0x001fff, 0x07e000, m_rom->base()); // Avoid the 68000 reading from lalaland in its reset handler
}

void sage2_state::machine_reset()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	program.unmap_readwrite(0x000000, 0x07ffff);
	program.install_rom(0x000000, 0x001fff, 0x07e000, m_rom->base());
	program.install_read_handler(0xfe0000, 0xfe3fff, read16_delegate(FUNC(sage2_state::rom_r), this));
	m_maincpu->reset();
}

READ16_MEMBER(sage2_state::rom_r)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	program.unmap_readwrite(0x000000, 0x07ffff);
	program.install_ram(0, m_ram->size()-1, m_ram->pointer());
	program.install_rom(0xfe0000, 0xfe1fff, 0x002000, m_rom->base());
	return program.read_word(0xfe0000 | (offset*2));
}

//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

//-------------------------------------------------
//  MACHINE_CONFIG( sage2 )
//-------------------------------------------------

MACHINE_CONFIG_START(sage2_state::sage2)
	// basic machine hardware
	MCFG_DEVICE_ADD(M68000_TAG, M68000, XTAL(16'000'000)/2)
	MCFG_DEVICE_PROGRAM_MAP(sage2_mem)

	// devices
	MCFG_DEVICE_ADD(I8259_TAG, PIC8259, 0)
	MCFG_PIC8259_OUT_INT_CB(INPUTLINE(M68000_TAG, M68K_IRQ_1))

	i8255_device &ppi0(I8255A(config, I8255A_0_TAG));
	ppi0.in_pa_callback().set_ioport("J7");
	ppi0.in_pb_callback().set_ioport("J6");
	ppi0.out_pc_callback().set(FUNC(sage2_state::ppi0_pc_w));

	i8255_device &ppi1(I8255A(config, I8255A_1_TAG));
	ppi1.out_pa_callback().set("cent_data_out", FUNC(output_latch_device::bus_w));
	ppi1.in_pb_callback().set(FUNC(sage2_state::ppi1_pb_r));
	ppi1.out_pc_callback().set(FUNC(sage2_state::ppi1_pc_w));

	MCFG_DEVICE_ADD(I8253_0_TAG, PIT8253, 0)
	MCFG_PIT8253_CLK0(0) // from U75 OUT0
	MCFG_PIT8253_OUT0_HANDLER(WRITELINE(I8259_TAG, pic8259_device, ir6_w))
	MCFG_PIT8253_CLK1(XTAL(16'000'000)/2/125)
	MCFG_PIT8253_OUT1_HANDLER(WRITELINE(I8253_0_TAG, pit8253_device, write_clk2))
	MCFG_PIT8253_CLK2(0) // from OUT2
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(I8259_TAG, pic8259_device, ir0_w))

	MCFG_DEVICE_ADD(I8253_1_TAG, PIT8253, 0)
	MCFG_PIT8253_CLK0(XTAL(16'000'000)/2/125)
	MCFG_PIT8253_OUT0_HANDLER(WRITELINE(I8253_0_TAG, pit8253_device, write_clk0))
	MCFG_PIT8253_CLK1(XTAL(16'000'000)/2/13)
	MCFG_PIT8253_OUT1_HANDLER(WRITELINE(*this, sage2_state, br1_w))
	MCFG_PIT8253_CLK2(XTAL(16'000'000)/2/13)
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(*this, sage2_state, br2_w))

	I8251(config, m_usart0, 0);
	m_usart0->txd_handler().set(RS232_A_TAG, FUNC(rs232_port_device::write_txd));
	m_usart0->dtr_handler().set(RS232_A_TAG, FUNC(rs232_port_device::write_dtr));
	m_usart0->rts_handler().set(RS232_A_TAG, FUNC(rs232_port_device::write_rts));
	m_usart0->rxrdy_handler().set_inputline(M68000_TAG, M68K_IRQ_5);
	m_usart0->txrdy_handler().set(I8259_TAG, FUNC(pic8259_device::ir2_w));

	rs232_port_device &rs232a(RS232_PORT(config, RS232_A_TAG, default_rs232_devices, "terminal"));
	rs232a.rxd_handler().set(m_usart0, FUNC(i8251_device::write_rxd));
	rs232a.dsr_handler().set(m_usart0, FUNC(i8251_device::write_dsr));
	rs232a.cts_handler().set(m_usart0, FUNC(i8251_device::write_cts));
	rs232a.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));

	I8251(config, m_usart1, 0);
	m_usart1->txd_handler().set(RS232_B_TAG, FUNC(rs232_port_device::write_txd));
	m_usart1->dtr_handler().set(RS232_B_TAG, FUNC(rs232_port_device::write_dtr));
	m_usart1->rts_handler().set(RS232_B_TAG, FUNC(rs232_port_device::write_rts));
	m_usart1->rxrdy_handler().set(I8259_TAG, FUNC(pic8259_device::ir1_w));
	m_usart1->txrdy_handler().set(I8259_TAG, FUNC(pic8259_device::ir3_w));

	rs232_port_device &rs232b(RS232_PORT(config, RS232_B_TAG, default_rs232_devices, nullptr));
	rs232b.rxd_handler().set(m_usart1, FUNC(i8251_device::write_rxd));
	rs232b.dsr_handler().set(m_usart1, FUNC(i8251_device::write_dsr));

	UPD765A(config, m_fdc, false, false);
	m_fdc->intrq_wr_callback().set(FUNC(sage2_state::fdc_irq));

	MCFG_DEVICE_ADD(m_centronics, CENTRONICS, centronics_devices, "printer")
	MCFG_CENTRONICS_ACK_HANDLER(WRITELINE(*this, sage2_state, write_centronics_ack))
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(*this, sage2_state, write_centronics_busy))
	MCFG_CENTRONICS_PERROR_HANDLER(WRITELINE(*this, sage2_state, write_centronics_perror))
	MCFG_CENTRONICS_SELECT_HANDLER(WRITELINE(*this, sage2_state, write_centronics_select))
	MCFG_CENTRONICS_FAULT_HANDLER(WRITELINE(*this, sage2_state, write_centronics_fault))

	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", "centronics")

	MCFG_FLOPPY_DRIVE_ADD(UPD765_TAG ":0", sage2_floppies, "525qd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(UPD765_TAG ":1", sage2_floppies, "525qd", floppy_image_device::default_floppy_formats)
	MCFG_IEEE488_BUS_ADD()

	// internal ram
	RAM(config, RAM_TAG).set_default_size("512K");

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
	ROM_REGION16_BE( 0x2000, M68000_TAG, 0 )
	ROM_LOAD16_BYTE( "sage2.u18", 0x0001, 0x1000, CRC(ca9b312d) SHA1(99436a6d166aa5280c3b2d28355c4d20528fe48c) )
	ROM_LOAD16_BYTE( "sage2.u17", 0x0000, 0x1000, CRC(27e25045) SHA1(041cd9d4617473d089f31f18cbb375046c3b61bb) )
ROM_END



//**************************************************************************
//  DRIVER INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  DRIVER_INIT( sage2 )
//-------------------------------------------------

void sage2_state::init_sage2()
{
}



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  STATE        INIT        COMPANY            FULLNAME   FLAGS
COMP( 1982, sage2,  0,      0,      sage2,   sage2, sage2_state, init_sage2, "Sage Technology", "Sage II", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
