/*

    Wang Professional Computer

    http://www.seasip.info/VintagePC/wangpc.html

    chdman -createblankhd q540.chd 512 8 17 512

*/

/*

    TODO:

    - hard disk

*/

#include "includes/wangpc.h"


//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define LOG 0

enum
{
	LED_DIAGNOSTIC = 0
};



//**************************************************************************
//  DIRECT MEMORY ACCESS
//**************************************************************************

void wangpc_state::select_drive(int drive, bool select)
{
	if (LOG) logerror("%s: %sselect drive %u\n", machine().describe_context(), select ? "" : "De", drive + 1);

	int state = select ? 0 : 1;

	if (!drive)
	{
		m_ds1 = state;
		if(state)
			m_fdc->set_floppy(m_floppy0);
	}
	else
	{
		m_ds2 = state;
		if(state)
			m_fdc->set_floppy(m_floppy1);
	}
	if(!m_ds1 && !m_ds2)
		m_fdc->set_floppy(NULL);
}

void wangpc_state::set_motor(int drive, bool motor)
{
	if (LOG) logerror("%s: Motor %u %s\n", machine().describe_context(), drive + 1, motor ? "on" : "off");

	int state = motor ? 0 : 1;

	if (!drive)
	{
		m_floppy0->mon_w(state);
	}
	else
	{
		m_floppy1->mon_w(state);
	}
}

void wangpc_state::fdc_reset()
{
    if (LOG) logerror("%s: FDC reset\n", machine().describe_context());

	m_fdc->reset();
}

void wangpc_state::fdc_tc()
{
    if (LOG) logerror("%s: FDC TC\n", machine().describe_context());

	m_fdc->tc_w(true);
	m_fdc->tc_w(false);
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
	select_drive(0, false);

	return 0xff;
}


WRITE8_MEMBER( wangpc_state::deselect_drive1_w )
{
	select_drive(0, false);
}


READ8_MEMBER( wangpc_state::select_drive1_r )
{
	select_drive(0, true);

	return 0xff;
}


WRITE8_MEMBER( wangpc_state::select_drive1_w )
{
	select_drive(0, true);
}


READ8_MEMBER( wangpc_state::deselect_drive2_r )
{
	select_drive(1, false);

	return 0xff;
}


WRITE8_MEMBER( wangpc_state::deselect_drive2_w )
{
	select_drive(1, false);
}


READ8_MEMBER( wangpc_state::select_drive2_r )
{
	select_drive(1, true);

	return 0xff;
}


WRITE8_MEMBER( wangpc_state::select_drive2_w )
{
	select_drive(1, true);
}


READ8_MEMBER( wangpc_state::motor1_off_r )
{
	set_motor(0, false);

	return 0xff;
}


WRITE8_MEMBER( wangpc_state::motor1_off_w )
{
	set_motor(0, false);
}


READ8_MEMBER( wangpc_state::motor1_on_r )
{
	set_motor(0, true);

	return 0xff;
}


WRITE8_MEMBER( wangpc_state::motor1_on_w )
{
	set_motor(0, true);
}


READ8_MEMBER( wangpc_state::motor2_off_r )
{
	set_motor(1, false);

	return 0xff;
}


WRITE8_MEMBER( wangpc_state::motor2_off_w )
{
	set_motor(1, false);
}


READ8_MEMBER( wangpc_state::motor2_on_r )
{
	set_motor(1, true);

	return 0xff;
}


WRITE8_MEMBER( wangpc_state::motor2_on_w )
{
	set_motor(1, true);
}


READ8_MEMBER( wangpc_state::fdc_reset_r )
{
	fdc_reset();

	return 0xff;
}


WRITE8_MEMBER( wangpc_state::fdc_reset_w )
{
	fdc_reset();
}


READ8_MEMBER( wangpc_state::fdc_tc_r )
{
	fdc_tc();

	return 0xff;
}


WRITE8_MEMBER( wangpc_state::fdc_tc_w )
{
	fdc_tc();
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

	pic8259_ir0_w(m_pic, CLEAR_LINE);
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

	output_set_led_value(LED_DIAGNOSTIC, 1);

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

	return m_centronics->read(space, 0);
}


//-------------------------------------------------
//  centronics_w -
//-------------------------------------------------

WRITE8_MEMBER( wangpc_state::centronics_w )
{
	m_acknlg = 1;
	check_level1_interrupts();

	m_centronics->write(space, 0, data);

	m_centronics->strobe_w(0);
	m_centronics->strobe_w(1);
}


//-------------------------------------------------
//  busy_clr_r -
//-------------------------------------------------

READ8_MEMBER( wangpc_state::busy_clr_r )
{
    if (LOG) logerror("%s: BUSY clear\n", machine().describe_context());

	m_busy = 1;
	check_level1_interrupts();

	return 0xff;
}


//-------------------------------------------------
//  acknlg_clr_w -
//-------------------------------------------------

WRITE8_MEMBER( wangpc_state::acknlg_clr_w )
{
    if (LOG) logerror("%s: ACKNLG clear\n", machine().describe_context());

	m_acknlg = 1;
	check_level1_interrupts();
}


//-------------------------------------------------
//  led_off_r -
//-------------------------------------------------

READ8_MEMBER( wangpc_state::led_off_r )
{
    if (LOG) logerror("%s: Diagnostic LED off\n", machine().describe_context());

	output_set_led_value(LED_DIAGNOSTIC, 0);

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
	data |= (m_fdc_dd0 | m_fdc_dd1 | (int) m_fdc->get_irq()) << 7;

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
	AM_RANGE(0x1040, 0x1047) AM_DEVREADWRITE8_LEGACY(I8253_TAG, pit8253_r, pit8253_w, 0x00ff)
	AM_RANGE(0x1060, 0x1063) AM_DEVREADWRITE8_LEGACY(I8259A_TAG, pic8259_r, pic8259_w, 0x00ff)
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
	PORT_DIPNAME( 0x0f, 0x0f, "CPU Baud Rate" )	PORT_DIPLOCATION("SW:1,2,3,4")
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
//  I8237_INTERFACE( dmac_intf )
//-------------------------------------------------

void wangpc_state::update_fdc_tc()
{
	if (m_enable_eop)
		m_fdc->tc_w(m_fdc_tc);
	else
		m_fdc->tc_w(false);
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
		m_fdc_tc = !state;
		update_fdc_tc();
	}

	if (!state)
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

static AM9517A_INTERFACE( dmac_intf )
{
	DEVCB_DRIVER_LINE_MEMBER(wangpc_state, hrq_w),
	DEVCB_DRIVER_LINE_MEMBER(wangpc_state, eop_w),
	DEVCB_DRIVER_MEMBER(wangpc_state, memr_r),
	DEVCB_DRIVER_MEMBER(wangpc_state, memw_w),
	{ DEVCB_NULL,
	  DEVCB_DEVICE_MEMBER(WANGPC_BUS_TAG, wangpcbus_device, dack1_r),
	  DEVCB_DRIVER_MEMBER(wangpc_state, ior2_r),
	  DEVCB_DEVICE_MEMBER(WANGPC_BUS_TAG, wangpcbus_device, dack3_r) },
	{ DEVCB_NULL,
	  DEVCB_DEVICE_MEMBER(WANGPC_BUS_TAG, wangpcbus_device, dack1_w),
	  DEVCB_DRIVER_MEMBER(wangpc_state, iow2_w),
	  DEVCB_DEVICE_MEMBER(WANGPC_BUS_TAG, wangpcbus_device, dack3_w) },
	{ DEVCB_DRIVER_LINE_MEMBER(wangpc_state, dack0_w),
	  DEVCB_DRIVER_LINE_MEMBER(wangpc_state, dack1_w),
	  DEVCB_DRIVER_LINE_MEMBER(wangpc_state, dack2_w),
	  DEVCB_DRIVER_LINE_MEMBER(wangpc_state, dack3_w) }
};


//-------------------------------------------------
//  pic8259_interface pic_intf
//-------------------------------------------------

void wangpc_state::check_level1_interrupts()
{
	int state = !m_timer2_irq || m_epci->rxrdy_r() || m_epci->txemt_r() || !m_acknlg || !m_dav || !m_busy;

	pic8259_ir1_w(m_pic, state);
}

void wangpc_state::check_level2_interrupts()
{
	int state = !m_dma_eop || m_uart_dr || m_uart_tbre || m_fdc_dd0 || m_fdc_dd1 || m_fdc->get_irq() || m_fpu_irq || m_bus_irq2;

	pic8259_ir2_w(m_pic, state);
}

static IRQ_CALLBACK( wangpc_irq_callback )
{
	wangpc_state *state = device->machine().driver_data<wangpc_state>();

	return pic8259_acknowledge(state->m_pic);
}

static const struct pic8259_interface pic_intf =
{
	DEVCB_CPU_INPUT_LINE(I8086_TAG, INPUT_LINE_IRQ0),
	DEVCB_LINE_VCC,
	DEVCB_NULL
};


//-------------------------------------------------
//  I8255A_INTERFACE( ppi_intf )
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
	data |= m_centronics->busy_r() << 4;
	data |= m_centronics->fault_r() << 5;
	data |= m_centronics->pe_r() << 6;
	data |= m_acknlg << 7;

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
	data |= m_acknlg << 2;

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

	return ioport("SW")->read() << 4;
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

	m_centronics->autofeed_w(BIT(data, 0));
	m_centronics->init_prime_w(BIT(data, 2));
}

static I8255A_INTERFACE( ppi_intf )
{
	DEVCB_DRIVER_MEMBER(wangpc_state, ppi_pa_r),
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(wangpc_state, ppi_pb_r),
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(wangpc_state, ppi_pc_r),
	DEVCB_DRIVER_MEMBER(wangpc_state, ppi_pc_w)
};


//-------------------------------------------------
//  pit8253_config pit_intf
//-------------------------------------------------

WRITE_LINE_MEMBER( wangpc_state::pit2_w )
{
	if (state)
	{
		m_timer2_irq = 0;
		check_level1_interrupts();
	}
}

static const struct pit8253_config pit_intf =
{
	{
		{
			500000,
			DEVCB_LINE_VCC,
			DEVCB_DEVICE_LINE(I8259A_TAG, pic8259_ir0_w)
		}, {
			2000000,
			DEVCB_LINE_VCC,
			DEVCB_NULL
		}, {
			500000,
			DEVCB_LINE_VCC,
			DEVCB_DRIVER_LINE_MEMBER(wangpc_state, pit2_w)
		}
	}
};


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

static IM6402_INTERFACE( uart_intf )
{
	0, // HACK should be 62500
	62500,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_LINE_MEMBER(wangpc_state, uart_dr_w),
	DEVCB_DRIVER_LINE_MEMBER(wangpc_state, uart_tbre_w),
	DEVCB_NULL
};


//-------------------------------------------------
//  SCN2661_INTERFACE( epci_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( wangpc_state::epci_irq_w )
{
	check_level1_interrupts();
}

static MC2661_INTERFACE( epci_intf )
{
	0,
	0,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_LINE_MEMBER(wangpc_state, epci_irq_w),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_LINE_MEMBER(wangpc_state, epci_irq_w),
	DEVCB_NULL,
	DEVCB_NULL
};


//-------------------------------------------------
//  upd765_interface fdc_intf
//-------------------------------------------------

static SLOT_INTERFACE_START( wangpc_floppies )
	SLOT_INTERFACE( "525dd", FLOPPY_525_DD )
SLOT_INTERFACE_END

void wangpc_state::fdc_irq(bool state)
{
    if (LOG) logerror("FDC INT %u\n", state);

	check_level2_interrupts();
}

void wangpc_state::fdc_drq(bool state)
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

WRITE_LINE_MEMBER( wangpc_state::ack_w )
{
    if (LOG) logerror("ACKNLG %u\n", state);

	m_acknlg = state;

	check_level1_interrupts();
}

WRITE_LINE_MEMBER( wangpc_state::busy_w )
{
    if (LOG) logerror("BUSY %u\n", state);

	m_busy = state;

	check_level1_interrupts();
}

static const centronics_interface centronics_intf =
{
	DEVCB_DRIVER_LINE_MEMBER(wangpc_state, ack_w),
	DEVCB_DRIVER_LINE_MEMBER(wangpc_state, busy_w),
	DEVCB_NULL
};


//-------------------------------------------------
//  WANGPC_BUS_INTERFACE( kb_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( wangpc_state::bus_irq2_w )
{
    if (LOG) logerror("Bus IRQ2 %u\n", state);

	m_bus_irq2 = state;

	check_level2_interrupts();
}

static WANGPC_BUS_INTERFACE( bus_intf )
{
	DEVCB_DRIVER_LINE_MEMBER(wangpc_state, bus_irq2_w),
	DEVCB_DEVICE_LINE(I8259A_TAG, pic8259_ir3_w),
	DEVCB_DEVICE_LINE(I8259A_TAG, pic8259_ir4_w),
	DEVCB_DEVICE_LINE(I8259A_TAG, pic8259_ir5_w),
	DEVCB_DEVICE_LINE(I8259A_TAG, pic8259_ir6_w),
	DEVCB_DEVICE_LINE(I8259A_TAG, pic8259_ir7_w),
	DEVCB_DEVICE_LINE_MEMBER(AM9517A_TAG, am9517a_device, dreq1_w),
	DEVCB_DEVICE_LINE_MEMBER(AM9517A_TAG, am9517a_device, dreq2_w),
	DEVCB_DEVICE_LINE_MEMBER(AM9517A_TAG, am9517a_device, dreq3_w),
	DEVCB_CPU_INPUT_LINE(I8086_TAG, INPUT_LINE_NMI)
};

static SLOT_INTERFACE_START( wangpc_cards )
	SLOT_INTERFACE("emb", WANGPC_EMB) // extended memory board
	SLOT_INTERFACE("lic", WANGPC_LIC) // local interconnect option card
	SLOT_INTERFACE("lvc", WANGPC_LVC) // low-resolution video controller
	SLOT_INTERFACE("mcc", WANGPC_MCC) // multiport communications controller
	SLOT_INTERFACE("mvc", WANGPC_MVC) // medium-resolution video controller
	SLOT_INTERFACE("rtc", WANGPC_RTC) // remote telecommunications controller
	SLOT_INTERFACE("tig", WANGPC_TIG) // text/image/graphics controller
	SLOT_INTERFACE("wdc", WANGPC_WDC) // Winchester disk controller
SLOT_INTERFACE_END



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_START( wangpc )
//-------------------------------------------------

void wangpc_state::machine_start()
{
	// register CPU IRQ callback
	m_maincpu->set_irq_acknowledge_callback(wangpc_irq_callback);

	// connect serial keyboard
	m_uart->connect(m_kb);

	// connect floppy callbacks
	m_floppy0->setup_load_cb(floppy_image_device::load_cb(FUNC(wangpc_state::on_disk0_load), this));
	m_floppy0->setup_unload_cb(floppy_image_device::unload_cb(FUNC(wangpc_state::on_disk0_unload), this));
	m_floppy1->setup_load_cb(floppy_image_device::load_cb(FUNC(wangpc_state::on_disk1_load), this));
	m_floppy1->setup_unload_cb(floppy_image_device::unload_cb(FUNC(wangpc_state::on_disk1_unload), this));

	m_fdc->setup_intrq_cb(upd765a_device::line_cb(FUNC(wangpc_state::fdc_irq), this));
	m_fdc->setup_drq_cb(upd765a_device::line_cb(FUNC(wangpc_state::fdc_drq), this));

	// state saving
	save_item(NAME(m_dma_page));
	save_item(NAME(m_dack));
	save_item(NAME(m_timer2_irq));
	save_item(NAME(m_acknlg));
	save_item(NAME(m_dav));
	save_item(NAME(m_busy));
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


//-------------------------------------------------
//  MACHINE_RESET( wangpc )
//-------------------------------------------------

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

	// devices
	MCFG_AM9517A_ADD(AM9517A_TAG, 4000000, dmac_intf)
	MCFG_PIC8259_ADD(I8259A_TAG, pic_intf)
	MCFG_I8255A_ADD(I8255A_TAG, ppi_intf)
	MCFG_PIT8253_ADD(I8253_TAG, pit_intf)
	MCFG_IM6402_ADD(IM6402_TAG, uart_intf)
	MCFG_MC2661_ADD(SCN2661_TAG, 0, epci_intf)
	MCFG_UPD765A_ADD(UPD765_TAG, false, false)
	MCFG_FLOPPY_DRIVE_ADD(UPD765_TAG ":0", wangpc_floppies, "525dd", 0, floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(UPD765_TAG ":1", wangpc_floppies, "525dd", 0, floppy_image_device::default_floppy_formats)
	MCFG_CENTRONICS_PRINTER_ADD(CENTRONICS_TAG, centronics_intf)
	MCFG_WANGPC_KEYBOARD_ADD()

	// bus
	MCFG_WANGPC_BUS_ADD(bus_intf)
	MCFG_WANGPC_BUS_SLOT_ADD("slot1", 1, wangpc_cards, NULL, NULL)
	MCFG_WANGPC_BUS_SLOT_ADD("slot2", 2, wangpc_cards, "lvc", NULL)
	MCFG_WANGPC_BUS_SLOT_ADD("slot3", 3, wangpc_cards, NULL, NULL)
	MCFG_WANGPC_BUS_SLOT_ADD("slot4", 4, wangpc_cards, NULL, NULL)
	MCFG_WANGPC_BUS_SLOT_ADD("slot5", 5, wangpc_cards, NULL, NULL)

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

COMP( 1985, wangpc, 0, 0, wangpc, wangpc, driver_device, 0, "Wang Laboratories", "Wang Professional Computer", GAME_SUPPORTS_SAVE )
