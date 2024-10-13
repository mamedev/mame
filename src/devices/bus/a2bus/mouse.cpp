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
        PIA port A connects to 68705 port A in its entirety (bi-directional with internal pullups)
        PIA PB4-PB7 connects to 68705 PC0-3 (bi-directional but should not be pulled up)
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

#include "emu.h"
#include "mouse.h"

#include "machine/6821pia.h"
#include "cpu/m6805/m68705.h"


namespace {

/***************************************************************************
    CONSTANTS
***************************************************************************/

#define MOUSE_ROM_REGION    "a2mse_rom"
#define MOUSE_PIA_TAG       "a2mse_pia"
#define MOUSE_MCU_TAG       "a2mse_mcu"

#define MOUSE_BUTTON_TAG    "a2mse_button"
#define MOUSE_XAXIS_TAG     "a2mse_x"
#define MOUSE_YAXIS_TAG     "a2mse_y"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_mouse_device:
		public device_t,
		public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_mouse_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

protected:
	a2bus_mouse_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_cnxx(uint8_t offset) override;

	void pia_out_a(uint8_t data);
	void pia_out_b(uint8_t data);
	void pia_irqa_w(int state);
	void pia_irqb_w(int state);

	uint8_t mcu_port_a_r();
	uint8_t mcu_port_b_r();
	void mcu_port_a_w(uint8_t data);
	void mcu_port_b_w(uint8_t data);
	void mcu_port_c_w(uint8_t data);

	required_device<pia6821_device> m_pia;
	required_device<m68705p_device> m_mcu;
	required_ioport m_mouseb;
	required_ioport_array<2> m_mousexy;

private:
	template <unsigned AXIS, u8 DIR, u8 CLK> void update_axis();

	required_region_ptr<uint8_t> m_rom;
	int m_rom_bank;
	uint8_t m_port_a_in, m_port_b_in;
	int m_last[2], m_count[2];
};


ROM_START( mouse )
	ROM_REGION(0x800, MOUSE_ROM_REGION, 0)
	ROM_LOAD( "341-0270-c.4b", 0x000000, 0x000800, CRC(0bcd1e8e) SHA1(3a9d881a8a8d30f55b9719aceebbcf717f829d6f) )

	ROM_REGION(0x800, MOUSE_MCU_TAG, 0)
	ROM_LOAD( "341-0269.2b",  0x000000, 0x000800, CRC(94067f16) SHA1(3a2baa6648efe4456d3ec3721216e57c64f7acfc) )

	ROM_REGION(0xc00, "pal", 0)
	ROM_LOAD( "mmi_pal16r4a,jedec.2a", 0x000000, 0x000b04, CRC(1d620ee5) SHA1(5aa9a515c919ff7a18878649cac5d44f0c2abf28) )
	ROM_LOAD( "mmi_pal16r4a,binary.2a", 0x000000, 0x000100, CRC(1da5c745) SHA1(ba267b69a2fda2a2348b140979ece562411bb37b) )
ROM_END

INPUT_PORTS_START( mouse )
	PORT_START(MOUSE_BUTTON_TAG) /* Mouse - button */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Mouse Button") PORT_CODE(MOUSECODE_BUTTON1)

	PORT_START(MOUSE_XAXIS_TAG) /* Mouse - X AXIS */
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(40) PORT_KEYDELTA(0) PORT_PLAYER(1)

	PORT_START(MOUSE_YAXIS_TAG) /* Mouse - Y AXIS */
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(40) PORT_KEYDELTA(0) PORT_PLAYER(1)
INPUT_PORTS_END


/***************************************************************************
    DEVICE CONFIGURATION
***************************************************************************/

/*-------------------------------------------------
    input_ports - device-specific input ports
-------------------------------------------------*/

ioport_constructor a2bus_mouse_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(mouse);
}

/*-------------------------------------------------
    device_add_mconfig - device-specific
    machine configurations
-------------------------------------------------*/

void a2bus_mouse_device::device_add_mconfig(machine_config &config)
{
	M68705P3(config, m_mcu, 2043600);
	m_mcu->porta_r().set(FUNC(a2bus_mouse_device::mcu_port_a_r));
	m_mcu->portb_r().set(FUNC(a2bus_mouse_device::mcu_port_b_r));
	m_mcu->porta_w().set(FUNC(a2bus_mouse_device::mcu_port_a_w));
	m_mcu->portb_w().set(FUNC(a2bus_mouse_device::mcu_port_b_w));
	m_mcu->portc_w().set(FUNC(a2bus_mouse_device::mcu_port_c_w));

	PIA6821(config, m_pia, 1021800);
	m_pia->writepa_handler().set(FUNC(a2bus_mouse_device::pia_out_a));
	m_pia->writepb_handler().set(FUNC(a2bus_mouse_device::pia_out_b));
	m_pia->tspb_handler().set_constant(0x00);
	m_pia->irqa_handler().set(FUNC(a2bus_mouse_device::pia_irqa_w));
	m_pia->irqb_handler().set(FUNC(a2bus_mouse_device::pia_irqb_w));
}

/*-------------------------------------------------
    rom_region - device-specific ROM region
-------------------------------------------------*/

const tiny_rom_entry *a2bus_mouse_device::device_rom_region() const
{
	return ROM_NAME(mouse);
}


/***************************************************************************
    LIVE DEVICE
***************************************************************************/

a2bus_mouse_device::a2bus_mouse_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_pia(*this, MOUSE_PIA_TAG),
	m_mcu(*this, MOUSE_MCU_TAG),
	m_mouseb(*this, MOUSE_BUTTON_TAG), m_mousexy(*this, { MOUSE_XAXIS_TAG, MOUSE_YAXIS_TAG }),
	m_rom(*this, MOUSE_ROM_REGION),
	m_rom_bank(0),
	m_port_a_in(0), m_port_b_in(0),
	m_last{ 0, 0 }, m_count{ 0, 0 }
{
}

a2bus_mouse_device::a2bus_mouse_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_mouse_device(mconfig, A2BUS_MOUSE, tag, owner, clock)
{
}

/*-------------------------------------------------
    device_start - device-specific startup
-------------------------------------------------*/

void a2bus_mouse_device::device_start()
{
	// register save state variables
	save_item(NAME(m_port_a_in));
	save_item(NAME(m_port_b_in));
	save_item(NAME(m_last));
	save_item(NAME(m_count));
}

void a2bus_mouse_device::device_reset()
{
	m_last[0] = m_last[1] = m_count[0] = m_count[1] = 0;
}

/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

uint8_t a2bus_mouse_device::read_c0nx(uint8_t offset)
{
	return m_pia->read(offset & 3);
}

/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_mouse_device::write_c0nx(uint8_t offset, uint8_t data)
{
	m_pia->write(offset & 3, data);
}

/*-------------------------------------------------
    read_cnxx - called for reads from this card's cnxx space
-------------------------------------------------*/

uint8_t a2bus_mouse_device::read_cnxx(uint8_t offset)
{
	return m_rom[offset+m_rom_bank];
}

void a2bus_mouse_device::pia_out_a(uint8_t data)
{
	m_port_a_in = data;
}

void a2bus_mouse_device::pia_out_b(uint8_t data)
{
	m_mcu->pc_w(data >> 4);

	m_rom_bank = (data & 0xe) << 7;
}

void a2bus_mouse_device::pia_irqa_w(int state)
{
}

void a2bus_mouse_device::pia_irqb_w(int state)
{
}

uint8_t a2bus_mouse_device::mcu_port_a_r()
{
	return m_port_a_in;
}

void a2bus_mouse_device::mcu_port_a_w(uint8_t data)
{
	m_pia->set_a_input(data);
}

uint8_t a2bus_mouse_device::mcu_port_b_r()
{
	enum { XAXIS, YAXIS };
	constexpr u8 BUTTON = 0x80;
	constexpr u8 X0 = 0x02; // gate, must go 0/1 for each pixel moved
	constexpr u8 X1 = 0x01; // direction, 0 = left, 1 = right
	constexpr u8 Y0 = 0x04; // direction, 0 = up, 1 = down
	constexpr u8 Y1 = 0x08; // gate, must go 0/1 for each pixel moved

	// update button now
	if (m_mouseb->read())
	{
		m_port_b_in &= ~BUTTON;
	}
	else
	{
		m_port_b_in |= BUTTON;
	}

	// update the axes
	update_axis<XAXIS, X1, X0>();
	update_axis<YAXIS, Y0, Y1>();

	return m_port_b_in;
}

void a2bus_mouse_device::mcu_port_b_w(uint8_t data)
{
	if (!BIT(data, 6))
	{
		raise_slot_irq();
	}
	else
	{
		lower_slot_irq();
	}
}

void a2bus_mouse_device::mcu_port_c_w(uint8_t data)
{
	m_pia->portb_w(data << 4);
}

template <unsigned AXIS, u8 DIR, u8 CLK> void a2bus_mouse_device::update_axis()
{
	// read the axis and check for changes
	const int new_m = m_mousexy[AXIS]->read();
	int diff = new_m - m_last[AXIS];

	// check for wrap
	if (diff > 0x80)
		diff -= 0x100;
	else if (diff < -0x80)
		diff += 0x100;

	m_count[AXIS] += diff;
	m_last[AXIS] = new_m;

	if (m_count[AXIS])
	{
		m_port_b_in ^= CLK;
		if (m_count[AXIS] < 0)
		{
			m_count[AXIS]++;
			if (m_port_b_in & CLK) m_port_b_in &= ~DIR;
		}
		else
		{
			m_count[AXIS]--;
			if (m_port_b_in & CLK) m_port_b_in |= DIR;
		}
	}
}

}


/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_MOUSE, device_a2bus_card_interface, a2bus_mouse_device, "a2mouse", "Apple II Mouse Card")
