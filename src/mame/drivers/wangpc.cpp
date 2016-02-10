// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

    Wang Professional Computer

    http://www.seasip.info/VintagePC/wangpc.html

    chdman -createblankhd q540.chd 512 8 17 512

*/

/*

    TODO:

    - with quantum perfect cpu gets stuck @ 49c3 mov ss,cs:[52ah]
    - hard disk

*/

#include "includes/wangpc.h"
#include "bus/rs232/rs232.h"
#include "softlist.h"

//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define LOG 0

enum
{
	LED_DIAGNOSTIC = 0
};



//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

void wangpc_state::select_drive()
{
	floppy_image_device *floppy = nullptr;

	if (m_ds1) floppy = m_floppy0;
	if (m_ds2) floppy = m_floppy1;

	m_fdc->set_floppy(floppy);
}

WRITE8_MEMBER( wangpc_state::fdc_ctrl_w )
{
	/*

	    bit     description

	    0       Enable /EOP
	    1       Disable /DREQ2
	    2       Clear drive 1 door disturbed interrupt
	    3       Clear drive 2 door disturbed interrupt
	    4
	    5
	    6
	    7

	*/

	m_enable_eop = BIT(data, 0);
	m_disable_dreq2 = BIT(data, 1);

	if (BIT(data, 2)) m_fdc_dd0 = 0;
	if (BIT(data, 3)) m_fdc_dd1 = 0;

	if (LOG)
	{
		logerror("%s: Enable /EOP %u\n", machine().describe_context(), m_enable_eop);
		logerror("%s: Disable /DREQ2 %u\n", machine().describe_context(), m_disable_dreq2);
	}

	update_fdc_tc();
	update_fdc_drq();
}


READ8_MEMBER( wangpc_state::deselect_drive1_r )
{
	m_ds1 = false;
	select_drive();

	return 0xff;
}

WRITE8_MEMBER( wangpc_state::deselect_drive1_w )
{
	deselect_drive1_r(space, offset);
}

READ8_MEMBER( wangpc_state::select_drive1_r )
{
	m_ds1 = true;
	select_drive();

	return 0xff;
}

WRITE8_MEMBER( wangpc_state::select_drive1_w )
{
	select_drive1_r(space, offset);
}

READ8_MEMBER( wangpc_state::deselect_drive2_r )
{
	m_ds2 = false;
	select_drive();

	return 0xff;
}

WRITE8_MEMBER( wangpc_state::deselect_drive2_w )
{
	deselect_drive2_r(space, offset);
}

READ8_MEMBER( wangpc_state::select_drive2_r )
{
	m_ds2 = true;
	select_drive();

	return 0xff;
}

WRITE8_MEMBER( wangpc_state::select_drive2_w )
{
	select_drive2_r(space, offset);
}

READ8_MEMBER( wangpc_state::motor1_off_r )
{
	if (LOG) logerror("%s: Drive 1 motor OFF\n", machine().describe_context());

	m_floppy0->mon_w(1);

	return 0xff;
}

WRITE8_MEMBER( wangpc_state::motor1_off_w )
{
	motor1_off_r(space, offset);
}

READ8_MEMBER( wangpc_state::motor1_on_r )
{
	if (LOG) logerror("%s: Drive 1 motor ON\n", machine().describe_context());

	m_floppy0->mon_w(0);

	return 0xff;
}

WRITE8_MEMBER( wangpc_state::motor1_on_w )
{
	motor1_on_r(space, offset);
}

READ8_MEMBER( wangpc_state::motor2_off_r )
{
	if (LOG) logerror("%s: Drive 2 motor OFF\n", machine().describe_context());

	m_floppy1->mon_w(1);

	return 0xff;
}

WRITE8_MEMBER( wangpc_state::motor2_off_w )
{
	motor2_off_r(space, offset);
}

READ8_MEMBER( wangpc_state::motor2_on_r )
{
	if (LOG) logerror("%s: Drive 2 motor ON\n", machine().describe_context());

	m_floppy1->mon_w(0);

	return 0xff;
}

WRITE8_MEMBER( wangpc_state::motor2_on_w )
{
	motor2_on_r(space, offset);
}

READ8_MEMBER( wangpc_state::fdc_reset_r )
{
	if (LOG) logerror("%s: FDC reset\n", machine().describe_context());

	m_fdc->reset();

	return 0xff;
}

WRITE8_MEMBER( wangpc_state::fdc_reset_w )
{
	fdc_reset_r(space, offset);
}

READ8_MEMBER( wangpc_state::fdc_tc_r )
{
	if (LOG) logerror("%s: FDC TC\n", machine().describe_context());

	m_fdc->tc_w(1);
	m_fdc->tc_w(0);

	return 0xff;
}

WRITE8_MEMBER( wangpc_state::fdc_tc_w )
{
	fdc_tc_r(space, offset);
}


//-------------------------------------------------
//  dma_page_w -
//-------------------------------------------------

WRITE8_MEMBER( wangpc_state::dma_page_w )
{
	if (LOG) logerror("%s: DMA page %u: %06x\n", machine().describe_context(), offset + 1, (data & 0x0f) << 16);

	m_dma_page[offset + 1] = data & 0x0f;
}


//-------------------------------------------------
//  status_r -
//-------------------------------------------------

READ8_MEMBER( wangpc_state::status_r )
{
	/*

	    bit     description

	    0       Memory Parity Flag
	    1       I/O Error Flag
	    2       Unassigned
	    3       FDC Interrupt Flag
	    4       Door disturbed on drive 1
	    5       Door disturbed on drive 2
	    6       Door open on drive 1
	    7       Door open on drive 2

	*/

	UINT8 data = 0x03;

	// floppy interrupts
	data |= m_fdc->get_irq() << 3;
	data |= m_fdc_dd0 << 4;
	data |= m_fdc_dd1 << 5;
	data |= m_floppy0->exists() ? 0 : 0x40;
	data |= m_floppy1->exists() ? 0 : 0x80;

	return data;
}


//-------------------------------------------------
//  timer0_int_clr_w -
//-------------------------------------------------

WRITE8_MEMBER( wangpc_state::timer0_irq_clr_w )
{
	//if (LOG) logerror("%s: Timer 0 IRQ clear\n", machine().describe_context());

	m_pic->ir0_w(CLEAR_LINE);
}


//-------------------------------------------------
//  timer2_irq_clr_r -
//-------------------------------------------------

READ8_MEMBER( wangpc_state::timer2_irq_clr_r )
{
	//if (LOG) logerror("%s: Timer 2 IRQ clear\n", machine().describe_context());

	m_timer2_irq = 1;
	check_level1_interrupts();

	return 0xff;
}


//-------------------------------------------------
//  nmi_mask_w -
//-------------------------------------------------

WRITE8_MEMBER( wangpc_state::nmi_mask_w )
{
	if (LOG) logerror("%s: NMI mask %02x\n", machine().describe_context(), data);
}


//-------------------------------------------------
//  led_on_r -
//-------------------------------------------------

READ8_MEMBER( wangpc_state::led_on_r )
{
	if (LOG) logerror("%s: Diagnostic LED on\n", machine().describe_context());

	output().set_led_value(LED_DIAGNOSTIC, 1);

	return 0xff;
}


//-------------------------------------------------
//  fpu_mask_w -
//-------------------------------------------------

WRITE8_MEMBER( wangpc_state::fpu_mask_w )
{
	if (LOG) logerror("%s: FPU mask %02x\n", machine().describe_context(), data);
}


//-------------------------------------------------
//  dma_eop_clr_r -
//-------------------------------------------------

READ8_MEMBER( wangpc_state::dma_eop_clr_r )
{
	if (LOG) logerror("%s: EOP clear\n", machine().describe_context());

	m_dma_eop = 1;

	check_level2_interrupts();

	return 0xff;
}


//-------------------------------------------------
//  uart_tbre_clr_w -
//-------------------------------------------------

WRITE8_MEMBER( wangpc_state::uart_tbre_clr_w  )
{
	if (LOG) logerror("%s: TBRE clear\n", machine().describe_context());

	m_uart_tbre = 0;

	check_level2_interrupts();
}


//-------------------------------------------------
//  uart_r -
//-------------------------------------------------

READ8_MEMBER( wangpc_state::uart_r )
{
	m_uart_dr = 0;

	check_level2_interrupts();

	UINT8 data = m_uart->read(space, 0);

	if (LOG) logerror("%s: UART read %02x\n", machine().describe_context(), data);

	return data;
}


//-------------------------------------------------
//  uart_w -
//-------------------------------------------------

WRITE8_MEMBER( wangpc_state::uart_w  )
{
	if (LOG) logerror("%s: UART write %02x\n", machine().describe_context(), data);

	switch (data)
	{
	case 0x10: m_led[0] = 1; break;
	case 0x11: m_led[0] = 0; break;
	case 0x12: m_led[1] = 1; break;
	case 0x13: m_led[1] = 0; break;
	case 0x14: m_led[2] = 1; break;
	case 0x15: m_led[2] = 0; break;
	case 0x16: m_led[3] = 1; break;
	case 0x17: m_led[3] = 0; break;
	case 0x18: m_led[4] = 1; break;
	case 0x19: m_led[4] = 0; break;
	case 0x1a: m_led[5] = 1; break;
	case 0x1b: m_led[5] = 0; break;
	case 0x1c: m_led[0] = m_led[1] = m_led[2] = m_led[3] = m_led[4] = m_led[5] = 1; break;
	case 0x1d: m_led[0] = m_led[1] = m_led[2] = m_led[3] = m_led[4] = m_led[5] = 0; break;
	}

	if (LOG) popmessage("%u%u%u%u%u%u", m_led[0], m_led[1], m_led[2], m_led[3], m_led[4], m_led[5]);

	m_uart_tbre = 0;
	check_level2_interrupts();

	m_uart->write(space, 0, data);
}


//-------------------------------------------------
//  centronics_r -
//-------------------------------------------------

READ8_MEMBER( wangpc_state::centronics_r )
{
	m_dav = 1;
	check_level1_interrupts();

	return m_cent_data_in->read();
}


//-------------------------------------------------
//  centronics_w -
//-------------------------------------------------

WRITE8_MEMBER( wangpc_state::centronics_w )
{
	m_centronics_ack = 1;
	check_level1_interrupts();

	m_cent_data_out->write(data);

	m_centronics->write_strobe(0);
	m_centronics->write_strobe(1);
}


//-------------------------------------------------
//  busy_clr_r -
//-------------------------------------------------

READ8_MEMBER( wangpc_state::busy_clr_r )
{
	if (LOG) logerror("%s: BUSY clear\n", machine().describe_context());

	m_centronics_busy = 1;
	check_level1_interrupts();

	return 0xff;
}


//-------------------------------------------------
//  acknlg_clr_w -
//-------------------------------------------------

WRITE8_MEMBER( wangpc_state::acknlg_clr_w )
{
	if (LOG) logerror("%s: ACKNLG clear\n", machine().describe_context());

	m_centronics_ack = 1;
	check_level1_interrupts();
}


//-------------------------------------------------
//  led_off_r -
//-------------------------------------------------

READ8_MEMBER( wangpc_state::led_off_r )
{
	if (LOG) logerror("%s: Diagnostic LED off\n", machine().describe_context());

	output().set_led_value(LED_DIAGNOSTIC, 0);

	return 0xff;
}


//-------------------------------------------------
//  parity_nmi_clr_w -
//-------------------------------------------------

WRITE8_MEMBER( wangpc_state::parity_nmi_clr_w )
{
	if (LOG) logerror("%s: Parity NMI clear\n", machine().describe_context());
}


//-------------------------------------------------
//  option_id_r -
//-------------------------------------------------

READ8_MEMBER( wangpc_state::option_id_r )
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
	    7       FDC Interrupt Flag

	*/

	UINT8 data = 0;

	// FDC interrupt
	data |= (m_fdc_dd0 || m_fdc_dd1 || (int) m_fdc->get_irq()) << 7;

	return data;
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( wangpc_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( wangpc_mem, AS_PROGRAM, 16, wangpc_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0x1ffff) AM_RAM
	AM_RANGE(0x40000, 0xf3fff) AM_DEVREADWRITE(WANGPC_BUS_TAG, wangpcbus_device, mrdc_r, amwc_w)
	AM_RANGE(0xfc000, 0xfffff) AM_ROM AM_REGION(I8086_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( wangpc_io )
//-------------------------------------------------

static ADDRESS_MAP_START( wangpc_io, AS_IO, 16, wangpc_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x1000, 0x1001) AM_WRITE8(fdc_ctrl_w, 0x00ff)
	AM_RANGE(0x1004, 0x1005) AM_READWRITE8(deselect_drive1_r, deselect_drive1_w, 0x00ff)
	AM_RANGE(0x1006, 0x1007) AM_READWRITE8(select_drive1_r, select_drive1_w, 0x00ff)
	AM_RANGE(0x1008, 0x1009) AM_READWRITE8(deselect_drive2_r, deselect_drive2_w, 0x00ff)
	AM_RANGE(0x100a, 0x100b) AM_READWRITE8(select_drive2_r, select_drive2_w, 0x00ff)
	AM_RANGE(0x100c, 0x100d) AM_READWRITE8(motor1_off_r, motor1_off_w, 0x00ff)
	AM_RANGE(0x100e, 0x100f) AM_READWRITE8(motor1_on_r, motor1_on_w, 0x00ff)
	AM_RANGE(0x1010, 0x1011) AM_READWRITE8(motor2_off_r, motor2_off_w, 0x00ff)
	AM_RANGE(0x1012, 0x1013) AM_READWRITE8(motor2_on_r, motor2_on_w, 0x00ff)
	AM_RANGE(0x1014, 0x1017) AM_DEVICE8(UPD765_TAG, upd765a_device, map, 0x00ff)
	AM_RANGE(0x1018, 0x1019) AM_MIRROR(0x0002) AM_READWRITE8(fdc_reset_r, fdc_reset_w, 0x00ff)
	AM_RANGE(0x101c, 0x101d) AM_MIRROR(0x0002) AM_READWRITE8(fdc_tc_r, fdc_tc_w, 0x00ff)
	AM_RANGE(0x1020, 0x1027) AM_DEVREADWRITE8(I8255A_TAG, i8255_device, read, write, 0x00ff)
	AM_RANGE(0x1028, 0x1029) //AM_WRITE(?)
	AM_RANGE(0x1040, 0x1047) AM_DEVREADWRITE8(I8253_TAG, pit8253_device, read, write, 0x00ff)
	AM_RANGE(0x1060, 0x1063) AM_DEVREADWRITE8(I8259A_TAG, pic8259_device, read, write, 0x00ff)
	AM_RANGE(0x1080, 0x1087) AM_DEVREAD8(SCN2661_TAG, mc2661_device, read, 0x00ff)
	AM_RANGE(0x1088, 0x108f) AM_DEVWRITE8(SCN2661_TAG, mc2661_device, write, 0x00ff)
	AM_RANGE(0x10a0, 0x10bf) AM_DEVREADWRITE8(AM9517A_TAG, am9517a_device, read, write, 0x00ff)
	AM_RANGE(0x10c2, 0x10c7) AM_WRITE8(dma_page_w, 0x00ff)
	AM_RANGE(0x10e0, 0x10e1) AM_READWRITE8(status_r, timer0_irq_clr_w, 0x00ff)
	AM_RANGE(0x10e2, 0x10e3) AM_READWRITE8(timer2_irq_clr_r, nmi_mask_w, 0x00ff)
	AM_RANGE(0x10e4, 0x10e5) AM_READWRITE8(led_on_r, fpu_mask_w, 0x00ff)
	AM_RANGE(0x10e6, 0x10e7) AM_READWRITE8(dma_eop_clr_r, uart_tbre_clr_w, 0x00ff)
	AM_RANGE(0x10e8, 0x10e9) AM_READWRITE8(uart_r, uart_w, 0x00ff)
	AM_RANGE(0x10ea, 0x10eb) AM_READWRITE8(centronics_r, centronics_w, 0x00ff)
	AM_RANGE(0x10ec, 0x10ed) AM_READWRITE8(busy_clr_r, acknlg_clr_w, 0x00ff)
	AM_RANGE(0x10ee, 0x10ef) AM_READWRITE8(led_off_r, parity_nmi_clr_w, 0x00ff)
	AM_RANGE(0x10fe, 0x10ff) AM_READ8(option_id_r, 0x00ff)
	AM_RANGE(0x1100, 0x1fff) AM_DEVREADWRITE(WANGPC_BUS_TAG, wangpcbus_device, sad_r, sad_w)
ADDRESS_MAP_END



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( wangpc )
//-------------------------------------------------

static INPUT_PORTS_START( wangpc )
	// keyboard defined in machine/wangpckb.c

	PORT_START("SW")
	PORT_DIPNAME( 0x0f, 0x0f, "CPU Baud Rate" ) PORT_DIPLOCATION("SW:1,2,3,4")
	PORT_DIPSETTING(    0x0f, "19200" )
	PORT_DIPSETTING(    0x0e, "9600" )
	PORT_DIPSETTING(    0x0d, "7200" )
	PORT_DIPSETTING(    0x0c, "4800" )
	PORT_DIPSETTING(    0x0b, "3600" )
	PORT_DIPSETTING(    0x0a, "2400" )
	PORT_DIPSETTING(    0x09, "2000" )
	PORT_DIPSETTING(    0x08, "1800" )
	PORT_DIPSETTING(    0x07, "1200" )
	PORT_DIPSETTING(    0x06, "600" )
	PORT_DIPSETTING(    0x05, "300" )
	PORT_DIPSETTING(    0x04, "150" )
	PORT_DIPSETTING(    0x03, "134.5" )
	PORT_DIPSETTING(    0x02, "110" )
	PORT_DIPSETTING(    0x01, "75" )
	PORT_DIPSETTING(    0x00, "50 (Loop on Power-Up)" )
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  I8237
//-------------------------------------------------

void wangpc_state::update_fdc_tc()
{
	if (m_enable_eop)
		m_fdc->tc_w(m_fdc_tc);
	else
		m_fdc->tc_w(0);
}

WRITE_LINE_MEMBER( wangpc_state::hrq_w )
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	m_dmac->hack_w(state);
}

WRITE_LINE_MEMBER( wangpc_state::eop_w )
{
	if (m_dack == 2)
	{
		m_fdc_tc = state;
		update_fdc_tc();
	}

	if (state)
	{
		if (LOG) logerror("EOP set\n");

		m_dma_eop = 0;
		check_level2_interrupts();
	}

	m_bus->tc_w(state);
}

READ8_MEMBER( wangpc_state::memr_r )
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	offs_t addr = (m_dma_page[m_dack] << 16) | offset;

	return program.read_byte(addr);
}

WRITE8_MEMBER( wangpc_state::memw_w )
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	offs_t addr = (m_dma_page[m_dack] << 16) | offset;

	program.write_byte(addr, data);
}

READ8_MEMBER( wangpc_state::ior2_r )
{
	if (m_disable_dreq2)
		return m_bus->dack_r(space, 2);
	else
		return m_fdc->dma_r();
}

WRITE8_MEMBER( wangpc_state::iow2_w )
{
	if (m_disable_dreq2)
		m_bus->dack_w(space, 2, data);
	else
		m_fdc->dma_w(data);
}

WRITE_LINE_MEMBER( wangpc_state::dack0_w )
{
	if (!state) m_dack = 0;
}

WRITE_LINE_MEMBER( wangpc_state::dack1_w )
{
	if (!state) m_dack = 1;
}

WRITE_LINE_MEMBER( wangpc_state::dack2_w )
{
	if (!state) m_dack = 2;
}

WRITE_LINE_MEMBER( wangpc_state::dack3_w )
{
	if (!state) m_dack = 3;
}

//-------------------------------------------------
//  pic8259_interface pic_intf
//-------------------------------------------------

void wangpc_state::check_level1_interrupts()
{
	int state = !m_timer2_irq || m_epci->rxrdy_r() || m_epci->txemt_r() || !m_centronics_ack || !m_dav || !m_centronics_busy;

	m_pic->ir1_w(state);
}

void wangpc_state::check_level2_interrupts()
{
	int state = !m_dma_eop || m_uart_dr || m_uart_tbre || m_fdc_dd0 || m_fdc_dd1 || m_fdc->get_irq() || m_fpu_irq || m_bus_irq2;

	m_pic->ir2_w(state);
}

//-------------------------------------------------
//  I8255A INTERFACE
//-------------------------------------------------

READ8_MEMBER( wangpc_state::ppi_pa_r )
{
	/*

	    bit     description

	    0       /POWER ON
	    1       /SMART
	    2       /DATA AVAILABLE
	    3       SLCT
	    4       BUSY
	    5       /FAULT
	    6       PE
	    7       ACKNOWLEDGE

	*/

	UINT8 data = 0x08 | 0x02 | 0x01;

	data |= m_dav << 2;
	data |= m_centronics_busy << 4;
	data |= m_centronics_fault << 5;
	data |= m_centronics_perror << 6;
	data |= m_centronics_ack << 7;

	return data;
}

READ8_MEMBER( wangpc_state::ppi_pb_r )
{
	/*

	    bit     description

	    0       /TIMER 2 INTERRUPT
	    1       /SERIAL INTERRUPT
	    2       /PARALLEL PORT INTERRUPT
	    3       /DMA INTERRUPT
	    4       KBD INTERRUPT TRANSMIT
	    5       KBD INTERRUPT RECEIVE
	    6       FLOPPY DISK INTERRUPT
	    7       8087 INTERRUPT

	*/

	UINT8 data = 0;

	// timer 2 interrupt
	data |= m_timer2_irq;

	// serial interrupt
	data |= !(m_epci->rxrdy_r() | m_epci->txemt_r()) << 1;

	// parallel port interrupt
	data |= m_centronics_ack << 2;

	// DMA interrupt
	data |= m_dma_eop << 3;

	// keyboard interrupt
	data |= m_uart_tbre << 4;
	data |= m_uart_dr << 5;

	// FDC interrupt
	data |= m_fdc->get_irq() << 6;

	// 8087 interrupt
	data |= m_fpu_irq << 7;

	return data;
}

READ8_MEMBER( wangpc_state::ppi_pc_r )
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4       SW1
	    5       SW2
	    6       SW3
	    7       SW4

	*/

	return m_sw->read() << 4;
}

WRITE8_MEMBER( wangpc_state::ppi_pc_w )
{
	/*

	    bit     description

	    0       /USR0 (pin 14)
	    1       /USR1 (pin 36)
	    2       /RESET (pin 31)
	    3       Unassigned
	    4
	    5
	    6
	    7

	*/

	m_centronics->write_autofd(BIT(data, 0));
	m_centronics->write_select_in(BIT(data, 1));
	m_centronics->write_init(BIT(data, 2));
}

WRITE_LINE_MEMBER( wangpc_state::pit2_w )
{
	if (state)
	{
		m_timer2_irq = 0;
		check_level1_interrupts();
	}
}

//-------------------------------------------------
//  IM6402_INTERFACE( uart_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( wangpc_state::uart_dr_w )
{
	if (state)
	{
		if (LOG) logerror("DR set\n");

		m_uart_dr = 1;
		check_level2_interrupts();
	}
}

WRITE_LINE_MEMBER( wangpc_state::uart_tbre_w )
{
	if (state)
	{
		if (LOG) logerror("TBRE set\n");

		m_uart_tbre = 1;
		check_level2_interrupts();
	}
}


//-------------------------------------------------
//  SCN2661_INTERFACE( epci_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( wangpc_state::epci_irq_w )
{
	check_level1_interrupts();
}


//-------------------------------------------------
//  upd765_interface fdc_intf
//-------------------------------------------------

static SLOT_INTERFACE_START( wangpc_floppies )
	SLOT_INTERFACE( "525dd", FLOPPY_525_DD )
SLOT_INTERFACE_END

FLOPPY_FORMATS_MEMBER( wangpc_state::floppy_formats )
	FLOPPY_PC_FORMAT
FLOPPY_FORMATS_END

WRITE_LINE_MEMBER( wangpc_state::fdc_irq )
{
	if (LOG) logerror("FDC INT %u\n", state);

	check_level2_interrupts();
}

WRITE_LINE_MEMBER( wangpc_state::fdc_drq )
{
	if (LOG) logerror("FDC DRQ %u\n", state);

	m_fdc_drq = state;
	update_fdc_drq();
}

void wangpc_state::update_fdc_drq()
{
	if (m_disable_dreq2)
		m_dmac->dreq2_w(1);
	else
		m_dmac->dreq2_w(!m_fdc_drq);
}


//-------------------------------------------------
//  centronics_interface centronics_intf
//-------------------------------------------------

WRITE_LINE_MEMBER( wangpc_state::write_centronics_ack )
{
	if (LOG) logerror("ACKNLG %u\n", state);

	m_centronics_ack = state;

	check_level1_interrupts();
}

WRITE_LINE_MEMBER( wangpc_state::write_centronics_busy )
{
	if (LOG) logerror("BUSY %u\n", state);

	m_centronics_busy = state;

	check_level1_interrupts();
}

WRITE_LINE_MEMBER( wangpc_state::write_centronics_fault )
{
	m_centronics_fault = state;
}

WRITE_LINE_MEMBER( wangpc_state::write_centronics_perror )
{
	m_centronics_perror = state;
}

//-------------------------------------------------
//  WANGPC_BUS_INTERFACE( kb_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( wangpc_state::bus_irq2_w )
{
	if (LOG) logerror("Bus IRQ2 %u\n", state);

	m_bus_irq2 = state;

	check_level2_interrupts();
}



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_START( wangpc )
//-------------------------------------------------

void wangpc_state::machine_start()
{
	// connect floppy callbacks
	m_floppy0->setup_load_cb(floppy_image_device::load_cb(FUNC(wangpc_state::on_disk0_load), this));
	m_floppy0->setup_unload_cb(floppy_image_device::unload_cb(FUNC(wangpc_state::on_disk0_unload), this));
	m_floppy1->setup_load_cb(floppy_image_device::load_cb(FUNC(wangpc_state::on_disk1_load), this));
	m_floppy1->setup_unload_cb(floppy_image_device::unload_cb(FUNC(wangpc_state::on_disk1_unload), this));

	// state saving
	save_item(NAME(m_dma_page));
	save_item(NAME(m_dack));
	save_item(NAME(m_timer2_irq));
	save_item(NAME(m_centronics_ack));
	save_item(NAME(m_centronics_busy));
	save_item(NAME(m_centronics_fault));
	save_item(NAME(m_centronics_perror));
	save_item(NAME(m_dav));
	save_item(NAME(m_dma_eop));
	save_item(NAME(m_uart_dr));
	save_item(NAME(m_uart_tbre));
	save_item(NAME(m_fpu_irq));
	save_item(NAME(m_bus_irq2));
	save_item(NAME(m_enable_eop));
	save_item(NAME(m_disable_dreq2));
	save_item(NAME(m_fdc_drq));
	save_item(NAME(m_ds1));
	save_item(NAME(m_ds2));
}


void wangpc_state::machine_reset()
{
	// initialize UART
	m_uart->cls1_w(1);
	m_uart->cls2_w(1);
	m_uart->pi_w(1);
	m_uart->sbs_w(1);
	m_uart->crl_w(1);
}


//-------------------------------------------------
//  on_disk0_change -
//-------------------------------------------------

int wangpc_state::on_disk0_load(floppy_image_device *image)
{
	on_disk0_unload(image);

	return IMAGE_INIT_PASS;
}

void wangpc_state::on_disk0_unload(floppy_image_device *image)
{
	if (LOG) logerror("Door 1 disturbed\n");

	m_fdc_dd0 = 1;
	check_level2_interrupts();
}


//-------------------------------------------------
//  on_disk1_change -
//-------------------------------------------------

int wangpc_state::on_disk1_load(floppy_image_device *image)
{
	on_disk1_unload(image);

	return IMAGE_INIT_PASS;
}

void wangpc_state::on_disk1_unload(floppy_image_device *image)
{
	if (LOG) logerror("Door 2 disturbed\n");

	m_fdc_dd1 = 1;
	check_level2_interrupts();
}



//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

//-------------------------------------------------
//  MACHINE_CONFIG( wangpc )
//-------------------------------------------------

static MACHINE_CONFIG_START( wangpc, wangpc_state )
	MCFG_CPU_ADD(I8086_TAG, I8086, 8000000)
	MCFG_CPU_PROGRAM_MAP(wangpc_mem)
	MCFG_CPU_IO_MAP(wangpc_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE(I8259A_TAG, pic8259_device, inta_cb)
	//MCFG_QUANTUM_PERFECT_CPU(I8086_TAG)

	// devices
	MCFG_DEVICE_ADD(AM9517A_TAG, AM9517A, 4000000)
	MCFG_AM9517A_OUT_HREQ_CB(WRITELINE(wangpc_state, hrq_w))
	MCFG_AM9517A_OUT_EOP_CB(WRITELINE(wangpc_state, eop_w))
	MCFG_AM9517A_IN_MEMR_CB(READ8(wangpc_state, memr_r))
	MCFG_AM9517A_OUT_MEMW_CB(WRITE8(wangpc_state, memw_w))
	MCFG_AM9517A_IN_IOR_1_CB(DEVREAD8(WANGPC_BUS_TAG, wangpcbus_device, dack1_r))
	MCFG_AM9517A_IN_IOR_2_CB(READ8(wangpc_state, ior2_r))
	MCFG_AM9517A_IN_IOR_3_CB(DEVREAD8(WANGPC_BUS_TAG, wangpcbus_device, dack3_r))
	MCFG_AM9517A_OUT_IOW_1_CB(DEVWRITE8(WANGPC_BUS_TAG, wangpcbus_device, dack1_w))
	MCFG_AM9517A_OUT_IOW_2_CB(WRITE8(wangpc_state, iow2_w))
	MCFG_AM9517A_OUT_IOW_3_CB(DEVWRITE8(WANGPC_BUS_TAG, wangpcbus_device, dack3_w))
	MCFG_AM9517A_OUT_DACK_0_CB(WRITELINE(wangpc_state, dack0_w))
	MCFG_AM9517A_OUT_DACK_1_CB(WRITELINE(wangpc_state, dack1_w))
	MCFG_AM9517A_OUT_DACK_2_CB(WRITELINE(wangpc_state, dack2_w))
	MCFG_AM9517A_OUT_DACK_3_CB(WRITELINE(wangpc_state, dack3_w))

	MCFG_PIC8259_ADD(I8259A_TAG, INPUTLINE(I8086_TAG, INPUT_LINE_IRQ0), VCC, NULL)

	MCFG_DEVICE_ADD(I8255A_TAG, I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(wangpc_state, ppi_pa_r))
	MCFG_I8255_IN_PORTB_CB(READ8(wangpc_state, ppi_pb_r))
	MCFG_I8255_IN_PORTC_CB(READ8(wangpc_state, ppi_pc_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(wangpc_state, ppi_pc_w))

	MCFG_DEVICE_ADD(I8253_TAG, PIT8253, 0)
	MCFG_PIT8253_CLK0(500000)
	MCFG_PIT8253_OUT0_HANDLER(DEVWRITELINE(I8259A_TAG, pic8259_device, ir0_w))
	MCFG_PIT8253_CLK1(2000000)
	MCFG_PIT8253_CLK2(500000)
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(wangpc_state, pit2_w))

	MCFG_IM6402_ADD(IM6402_TAG, 0, 62500*16) // HACK for wangpckb in IM6402 derives clocks from data line
	MCFG_IM6402_TRO_CALLBACK(DEVWRITELINE(WANGPC_KEYBOARD_TAG, wangpc_keyboard_device, write_rxd))
	MCFG_IM6402_DR_CALLBACK(WRITELINE(wangpc_state, uart_dr_w))
	MCFG_IM6402_TBRE_CALLBACK(WRITELINE(wangpc_state, uart_tbre_w))

	MCFG_DEVICE_ADD(SCN2661_TAG, MC2661, 0)
	MCFG_MC2661_TXD_HANDLER(DEVWRITELINE(RS232_TAG, rs232_port_device, write_txd))
	MCFG_MC2661_RXRDY_HANDLER(WRITELINE(wangpc_state, epci_irq_w))
	MCFG_MC2661_RTS_HANDLER(DEVWRITELINE(RS232_TAG, rs232_port_device, write_rts))
	MCFG_MC2661_DTR_HANDLER(DEVWRITELINE(RS232_TAG, rs232_port_device, write_dtr))
	MCFG_MC2661_TXEMT_DSCHG_HANDLER(WRITELINE(wangpc_state, epci_irq_w))

	MCFG_UPD765A_ADD(UPD765_TAG, false, false)
	MCFG_UPD765_INTRQ_CALLBACK(WRITELINE(wangpc_state, fdc_irq))
	MCFG_UPD765_DRQ_CALLBACK(WRITELINE(wangpc_state, fdc_drq))
	MCFG_FLOPPY_DRIVE_ADD(UPD765_TAG ":0", wangpc_floppies, "525dd", wangpc_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(UPD765_TAG ":1", wangpc_floppies, "525dd", wangpc_state::floppy_formats)

	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_DATA_INPUT_BUFFER("cent_data_in")
	MCFG_CENTRONICS_ACK_HANDLER(WRITELINE(wangpc_state, write_centronics_ack))
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(wangpc_state, write_centronics_busy))
	MCFG_CENTRONICS_FAULT_HANDLER(WRITELINE(wangpc_state, write_centronics_fault))
	MCFG_CENTRONICS_PERROR_HANDLER(WRITELINE(wangpc_state, write_centronics_perror))

	MCFG_DEVICE_ADD("cent_data_in", INPUT_BUFFER, 0)
	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", CENTRONICS_TAG)

	MCFG_RS232_PORT_ADD(RS232_TAG, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(SCN2661_TAG, mc2661_device, rx_w))

	MCFG_DEVICE_ADD(WANGPC_KEYBOARD_TAG, WANGPC_KEYBOARD, 0)
	MCFG_WANGPCKB_TXD_HANDLER(DEVWRITELINE(IM6402_TAG, im6402_device, write_rri))

	// bus
	MCFG_WANGPC_BUS_ADD()
	MCFG_WANGPC_BUS_IRQ2_CALLBACK(WRITELINE(wangpc_state, bus_irq2_w))
	MCFG_WANGPC_BUS_IRQ3_CALLBACK(DEVWRITELINE(I8259A_TAG, pic8259_device, ir3_w))
	MCFG_WANGPC_BUS_IRQ4_CALLBACK(DEVWRITELINE(I8259A_TAG, pic8259_device, ir4_w))
	MCFG_WANGPC_BUS_IRQ5_CALLBACK(DEVWRITELINE(I8259A_TAG, pic8259_device, ir5_w))
	MCFG_WANGPC_BUS_IRQ6_CALLBACK(DEVWRITELINE(I8259A_TAG, pic8259_device, ir6_w))
	MCFG_WANGPC_BUS_IRQ7_CALLBACK(DEVWRITELINE(I8259A_TAG, pic8259_device, ir7_w))
	MCFG_WANGPC_BUS_DRQ1_CALLBACK(DEVWRITELINE(AM9517A_TAG, am9517a_device, dreq1_w))
	MCFG_WANGPC_BUS_DRQ2_CALLBACK(DEVWRITELINE(AM9517A_TAG, am9517a_device, dreq2_w))
	MCFG_WANGPC_BUS_DRQ3_CALLBACK(DEVWRITELINE(AM9517A_TAG, am9517a_device, dreq3_w))
	MCFG_WANGPC_BUS_IOERROR_CALLBACK(INPUTLINE(I8086_TAG, INPUT_LINE_NMI))
	MCFG_WANGPC_BUS_SLOT_ADD("slot1", 1, wangpc_cards, nullptr)
	MCFG_WANGPC_BUS_SLOT_ADD("slot2", 2, wangpc_cards, "mvc")
	MCFG_WANGPC_BUS_SLOT_ADD("slot3", 3, wangpc_cards, nullptr)
	MCFG_WANGPC_BUS_SLOT_ADD("slot4", 4, wangpc_cards, nullptr)
	MCFG_WANGPC_BUS_SLOT_ADD("slot5", 5, wangpc_cards, nullptr)

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("128K")

	// software list
	MCFG_SOFTWARE_LIST_ADD("flop_list", "wangpc")
MACHINE_CONFIG_END



//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

//-------------------------------------------------
//  ROM( wangpc )
//-------------------------------------------------

ROM_START( wangpc )
	ROM_REGION16_LE( 0x4000, I8086_TAG, 0)
	ROM_LOAD16_BYTE( "0001 r2.l94", 0x0001, 0x2000, CRC(f9f41304) SHA1(1815295809ef11573d724ede47446f9ac7aee713) )
	ROM_LOAD16_BYTE( "379-0000 r2.l115", 0x0000, 0x2000, CRC(67b37684) SHA1(70d9f68eb88cc2bc9f53f949cc77411c09a4266e) )
ROM_END



//**************************************************************************
//  GAME DRIVERS
//**************************************************************************

COMP( 1985, wangpc, 0, 0, wangpc, wangpc, driver_device, 0, "Wang Laboratories", "Wang Professional Computer", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
