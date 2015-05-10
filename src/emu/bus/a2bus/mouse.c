// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    mouse.c

    Implemention of the Apple II Mouse Card

    Apple II Mouse Interface PCB
    Apple 1983

    This is a mouse interface for the Apple II

    PCB Layout
    ----------
    apple computer
    MOUSE INTERFACE
    670-0030-C (C) 1983
    Printed on back side - MOUSE INTERFACE 820-0104-B (C) 1983 APPLE COMPUTER
    |-----------------------------------|
    |            PAL16R4       6821     |
    |                                   |
    |      J1                           |
    |                  74SC245          |
    |           68705P3        8516     |
    |     74LS74  X1  X2                |
    |-------------------|             |-|
                        |-------------|

    Notes:
          J1      - 9 pin flat cable with female DB9 connector
          68705P3 - Motorola MC68705P3 microcontroller (DIP28) labelled '341-0269 (C) APPLE'
                    PCB printed '(C) APPLE 1983 341-0269 or 342-0285'
          8516    - Fujitsu MB8516 2k x8-bit EPROM (DIP24) labelled '341-0270-C (C) APPLE 1983'
                    PCB printed '(C) APPLE 1983 342-0270'
          PAL16R4 - MMI PAL16R4ACN (DIP20) marked '341-0268-A'
                    PCB printed '(C) APPLE 1983 342-0268'
          6821    - AMI 6821 Peripheral Interface Adapter (DIP40)
          X1/X2   - Jumper pads. X1 is open, X2 is closed.


    Hookup notes:
        PIA port A connects to 68705 port A in its entirety (bi-directional)
        PIA PB4-PB7 connects to 68705 PC0-3 (bi-directional)
        PIA PB0 is 'sync latch'
        PIA PB1 is A8 on the EPROM
        PIA PB2 is A9 on the EPROM
        PIA PB3 is A10 on the EPROM

        68705 PB0 is mouse X1
        68705 PB1 is mouse X0
        68705 PB2 is mouse Y0
        68705 PB3 is mouse Y1
        68705 PB4 and 5 are N/C
        68705 PB6 is IRQ for the slot
        68705 PB7 is the mouse button

        68705 is clocked at 2M
        PIA is clocked at 1M

    See the schematic at:
    http://mirrors.apple2.org.za/Apple%20II%20Documentation%20Project/Interface%20Cards/Digitizers/Apple%20Mouse%20Interface%20Card/Schematics/

*********************************************************************/

#include "mouse.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type A2BUS_MOUSE = &device_creator<a2bus_mouse_device>;

#define MOUSE_ROM_REGION    "a2mse_rom"
#define MOUSE_PIA_TAG       "a2mse_pia"
#define MOUSE_MCU_TAG       "a2mse_mcu"
#define MOUSE_MCU_ROM       "a2mse_mcurom"

#define MOUSE_BUTTON_TAG    "a2mse_button"
#define MOUSE_XAXIS_TAG     "a2mse_x"
#define MOUSE_YAXIS_TAG     "a2mse_y"

#define TIMER_68705         0
#define TIMER_QUADRATURE    1

static ADDRESS_MAP_START( mcu_mem, AS_PROGRAM, 8, a2bus_mouse_device )
	ADDRESS_MAP_GLOBAL_MASK(0x7ff)
	AM_RANGE(0x0000, 0x0000) AM_READWRITE(mcu_port_a_r, mcu_port_a_w)
	AM_RANGE(0x0001, 0x0001) AM_READWRITE(mcu_port_b_r, mcu_port_b_w)
	AM_RANGE(0x0002, 0x0002) AM_READWRITE(mcu_port_c_r, mcu_port_c_w)
	AM_RANGE(0x0004, 0x0004) AM_WRITE(mcu_ddr_a_w)
	AM_RANGE(0x0005, 0x0005) AM_WRITE(mcu_ddr_b_w)
	AM_RANGE(0x0006, 0x0006) AM_WRITE(mcu_ddr_c_w)
	AM_RANGE(0x0008, 0x0009) AM_READWRITE(mcu_timer_r, mcu_timer_w)
	AM_RANGE(0x0010, 0x007f) AM_RAM
	AM_RANGE(0x0080, 0x07ff) AM_ROM AM_REGION(MOUSE_MCU_ROM, 0x80)
ADDRESS_MAP_END

MACHINE_CONFIG_FRAGMENT( mouse )
	MCFG_CPU_ADD(MOUSE_MCU_TAG, M68705, 2043600)
	MCFG_CPU_PROGRAM_MAP(mcu_mem)

	MCFG_DEVICE_ADD(MOUSE_PIA_TAG, PIA6821, 1021800)
	MCFG_PIA_READPA_HANDLER(READ8(a2bus_mouse_device, pia_in_a))
	MCFG_PIA_READPB_HANDLER(READ8(a2bus_mouse_device, pia_in_b))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(a2bus_mouse_device, pia_out_a))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(a2bus_mouse_device, pia_out_b))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(a2bus_mouse_device, pia_irqa_w))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(a2bus_mouse_device, pia_irqb_w))
MACHINE_CONFIG_END

ROM_START( mouse )
	ROM_REGION(0x800, MOUSE_ROM_REGION, 0)
	ROM_LOAD( "341-0270-c.4b", 0x000000, 0x000800, CRC(0bcd1e8e) SHA1(3a9d881a8a8d30f55b9719aceebbcf717f829d6f) )

	ROM_REGION(0x800, MOUSE_MCU_ROM, 0)
	ROM_LOAD( "341-0269.2b",  0x000000, 0x000800, CRC(94067f16) SHA1(3a2baa6648efe4456d3ec3721216e57c64f7acfc) )

	ROM_REGION(0xc00, "pal", 0)
	ROM_LOAD( "mmi_pal16r4a(jedec).2a", 0x000000, 0x000b04, CRC(1d620ee5) SHA1(5aa9a515c919ff7a18878649cac5d44f0c2abf28) )
	ROM_LOAD( "mmi_pal16r4a(binary).2a", 0x000000, 0x000100, CRC(1da5c745) SHA1(ba267b69a2fda2a2348b140979ece562411bb37b) )
ROM_END

static INPUT_PORTS_START( mouse )
	PORT_START(MOUSE_BUTTON_TAG) /* Mouse - button */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Mouse Button") PORT_CODE(MOUSECODE_BUTTON1)

	PORT_START(MOUSE_XAXIS_TAG) /* Mouse - X AXIS */
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(40) PORT_KEYDELTA(0) PORT_PLAYER(1)

	PORT_START(MOUSE_YAXIS_TAG) /* Mouse - Y AXIS */
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(40) PORT_KEYDELTA(0) PORT_PLAYER(1)
INPUT_PORTS_END

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor a2bus_mouse_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( mouse );
}

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor a2bus_mouse_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( mouse );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *a2bus_mouse_device::device_rom_region() const
{
	return ROM_NAME( mouse );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_mouse_device::a2bus_mouse_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	device_a2bus_card_interface(mconfig, *this),
	m_pia(*this, MOUSE_PIA_TAG),
	m_mcu(*this, MOUSE_MCU_TAG),
	m_mouseb(*this, MOUSE_BUTTON_TAG),
	m_mousex(*this, MOUSE_XAXIS_TAG),
	m_mousey(*this, MOUSE_YAXIS_TAG)
{
	m_started = false;
	m_rom_bank = 0;
}

a2bus_mouse_device::a2bus_mouse_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, A2BUS_MOUSE, "Apple II Mouse Card", tag, owner, clock, "a2mouse", __FILE__),
	device_a2bus_card_interface(mconfig, *this),
	m_pia(*this, MOUSE_PIA_TAG),
	m_mcu(*this, MOUSE_MCU_TAG),
	m_mouseb(*this, MOUSE_BUTTON_TAG),
	m_mousex(*this, MOUSE_XAXIS_TAG),
	m_mousey(*this, MOUSE_YAXIS_TAG)
{
	m_started = false;
	m_rom_bank = 0;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_mouse_device::device_start()
{
	// set_a2bus_device makes m_slot valid
	set_a2bus_device();

	m_rom = device().machine().root_device().memregion(this->subtag(MOUSE_ROM_REGION).c_str())->base();

	// allocate two timers: one for the 68705, one for the quadrature magic
	m_timer = timer_alloc(TIMER_68705, NULL);
	m_read_timer = timer_alloc(TIMER_QUADRATURE, NULL);
	m_timer->adjust(attotime::never, TIMER_68705);
	m_read_timer->adjust(attotime::never, TIMER_QUADRATURE);

	// get 68705P3 mask option byte
	m_mask_option = m_rom[0x784];

	// register save state variables
	save_item(NAME(m_ddr_a));
	save_item(NAME(m_ddr_b));
	save_item(NAME(m_ddr_c));
	save_item(NAME(m_port_a_out));
	save_item(NAME(m_port_b_out));
	save_item(NAME(m_port_c_out));
	save_item(NAME(m_port_a_in));
	save_item(NAME(m_port_b_in));
	save_item(NAME(m_port_c_in));
	save_item(NAME(m_timer_cnt));
	save_item(NAME(m_timer_ctl));
	save_item(NAME(last_mx));
	save_item(NAME(last_my));
	save_item(NAME(count_x));
	save_item(NAME(count_y));
}

void a2bus_mouse_device::device_reset()
{
	m_started = true;
	m_rom_bank = 0;
	last_mx = last_my = count_x = count_y = 0;
	m_timer_cnt = 0xff;
	m_timer_ctl = 0x40; // disable interrupt, everything else clear
	m_port_a_in = 0;
	m_port_b_in = 0x80;
	m_port_c_in = 0;

	// are we emulating the mask part with a semi-programmable timer?
	if (m_mask_option & 0x40)
	{
		m_timer_ctl |= m_mask_option & 0x17;
	}

	m_read_timer->adjust(attotime::from_hz(600.0), TIMER_QUADRATURE, attotime::from_hz(600.0));
}

/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

UINT8 a2bus_mouse_device::read_c0nx(address_space &space, UINT8 offset)
{
	return m_pia->read(space, offset & 3);
}


/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_mouse_device::write_c0nx(address_space &space, UINT8 offset, UINT8 data)
{
	m_pia->write(space, offset & 3, data);
}

/*-------------------------------------------------
    read_cnxx - called for reads from this card's cnxx space
-------------------------------------------------*/

UINT8 a2bus_mouse_device::read_cnxx(address_space &space, UINT8 offset)
{
	return m_rom[offset+m_rom_bank];
}

READ8_MEMBER(a2bus_mouse_device::pia_in_a)
{
	return m_port_a_out;
}

READ8_MEMBER(a2bus_mouse_device::pia_in_b)
{
	return (m_port_c_out << 4);
}

WRITE8_MEMBER(a2bus_mouse_device::pia_out_a)
{
	m_port_a_in = data;
}

WRITE8_MEMBER(a2bus_mouse_device::pia_out_b)
{
	m_port_c_in &= 0xf0;
	m_port_c_in |= ((data >> 4) & 0xf);

	m_rom_bank = (data & 0xe) << 7;
}

WRITE_LINE_MEMBER(a2bus_mouse_device::pia_irqa_w)
{
}

WRITE_LINE_MEMBER(a2bus_mouse_device::pia_irqb_w)
{
}

READ8_MEMBER(a2bus_mouse_device::mcu_port_a_r)
{
	return (m_port_a_out & m_ddr_a) | (m_port_a_in & ~m_ddr_a);
}

WRITE8_MEMBER(a2bus_mouse_device::mcu_port_a_w)
{
	m_port_a_out = data;
}

WRITE8_MEMBER(a2bus_mouse_device::mcu_ddr_a_w)
{
	m_ddr_a = data;
}

READ8_MEMBER(a2bus_mouse_device::mcu_port_b_r)
{
	UINT8 b_in = m_port_b_in;

	// clear the gates, leave everything else alone between pulses
	m_port_b_in &= 0x85;

	return (m_port_b_out & m_ddr_b) | (b_in & ~m_ddr_b);
}

WRITE8_MEMBER(a2bus_mouse_device::mcu_port_b_w)
{
	m_port_b_out = data;

	if (!(data & 0x40))
	{
		raise_slot_irq();
	}
	else
	{
		lower_slot_irq();
	}
}

WRITE8_MEMBER(a2bus_mouse_device::mcu_ddr_b_w)
{
	m_ddr_b = data;
}

READ8_MEMBER(a2bus_mouse_device::mcu_port_c_r)
{
	return (m_port_c_out & m_ddr_c) | (m_port_c_in & ~m_ddr_c);
}

WRITE8_MEMBER(a2bus_mouse_device::mcu_port_c_w)
{
	m_port_c_out = data;
}

WRITE8_MEMBER(a2bus_mouse_device::mcu_ddr_c_w)
{
	m_ddr_c = data;
}

READ8_MEMBER(a2bus_mouse_device::mcu_timer_r)
{
	if (offset == 1)
	{
		return m_timer_ctl;
	}

	return m_timer_cnt;
}

WRITE8_MEMBER(a2bus_mouse_device::mcu_timer_w)
{
	static const int prescale[8] = { 1, 2, 4, 8, 16, 32, 64, 128 };
	bool recalc = false;

	// offset 0 = timer data (counts down)
	if (offset == 0)
	{
		m_timer_cnt = data;
		recalc = true;
	}
	// offset 1 = timer control:  b7 = IRQ, b6 = IRQ mask (1=suppress),
	//                            b5 = input select (0=CPU clk, 1=ext),
	//                            b4 = enable external timer input,
	//                            b3 = clear, b2-b0 = scaler (1/2/4/8/16/32/64/128)
	else
	{
		// clearing the interrupt?
		if ((m_timer_ctl & 0x80) && !(data & 0x80))
		{
			m_mcu->set_input_line(M68705_INT_TIMER, CLEAR_LINE);
		}

		if (m_mask_option & 0x40)
		{
			m_timer_ctl &= 0x3f;
			m_timer_ctl |= (data & 0xc0);
		}
		else
		{
			// if any parameters that affect the timer changed, recalc now
			if ((data & 0x3f) != (m_timer_ctl & 0x3f))
			{
				recalc = true;
			}

			// if prescaler reset, recalc
			if (data & 0x8)
			{
				recalc = true;
			}

			m_timer_ctl = data;
		}

	}

	if (recalc)
	{
		// recalculate the timer now
		UINT32 m_ticks = 2043600 / 4;
		m_ticks /= prescale[m_timer_ctl & 7];
		m_ticks /= (int)(m_timer_cnt + 1);
		m_timer->adjust(attotime::from_hz((double)m_ticks), TIMER_68705, attotime::from_hz((double)m_ticks));
	}
}

/*
    X0 = direction, 0 = left, 1 = right
    X1 = gate, must go 0/1 for each pixel moved
    Y0 = direction, 0 = up, 1 = down
    Y1 = gate, must go 0/1 for each pixel moved

    The direction must stay constant for a given train of gate pulses or the MCU will get confused.
*/
void a2bus_mouse_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id == TIMER_68705)  // 68705's built-in timer
	{
		m_timer_ctl |= 0x80;    // indicate timer expired
		if (!(m_timer_ctl & 0x40))  // if interrupt not suppressed, fire!
		{
			m_mcu->set_input_line(M68705_INT_TIMER, ASSERT_LINE);
		}
	}
	else if (id == TIMER_QUADRATURE)
	{
		int new_mx, new_my;
		m_port_b_in = 0x80;

		// update button now
		if (m_mouseb->read()) m_port_b_in &= ~0x80;

		// read the axes
		new_mx = m_mousex->read();
		new_my = m_mousey->read();

		// did X change?
		if (new_mx != last_mx)
		{
			int diff = new_mx - last_mx;

			/* check for wrap */
			if (diff > 0x80)
				diff = 0x100-diff;
			if  (diff < -0x80)
				diff = -0x100-diff;

			count_x += diff;
			last_mx = new_mx;
		}

		// did Y change?
		if (new_my != last_my)
		{
			int diff = new_my - last_my;

			/* check for wrap */
			if (diff > 0x80)
				diff = 0x100-diff;
			if  (diff < -0x80)
				diff = -0x100-diff;

			count_y += diff;
			last_my = new_my;
		}

		if (count_x)
		{
			if (count_x < 0)
			{
				count_x++;
			}
			else
			{
				count_x--;
				m_port_b_in |= 0x01;    // X1
			}
			m_port_b_in |= 0x02;    // X0
		}
		else if (count_y)
		{
			if (count_y < 0)
			{
				count_y++;
			}
			else
			{
				count_y--;
				m_port_b_in |= 0x04;    // Y0
			}
			m_port_b_in |= 0x08;    // Y1
		}
	}
}
