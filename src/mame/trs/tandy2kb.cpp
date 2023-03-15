// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Tandy 2000 keyboard emulation

*********************************************************************/

#include "emu.h"
#include "tandy2kb.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define I8048_TAG       "m1"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(TANDY2K_KEYBOARD, tandy2k_keyboard_device, "tandy2kb", "Tandy 2000 Keyboard")



//-------------------------------------------------
//  ROM( tandy2k_keyboard )
//-------------------------------------------------

ROM_START( tandy2k_keyboard )
	ROM_REGION( 0x400, I8048_TAG, 0 )
	ROM_LOAD( "keyboard.m1", 0x0000, 0x0400, NO_DUMP )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *tandy2k_keyboard_device::device_rom_region() const
{
	return ROM_NAME( tandy2k_keyboard );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void tandy2k_keyboard_device::device_add_mconfig(machine_config &config)
{
	I8048(config, m_maincpu, 1000000); // ?
	m_maincpu->p1_out_cb().set(FUNC(tandy2k_keyboard_device::kb_p1_w));
	m_maincpu->p2_out_cb().set(FUNC(tandy2k_keyboard_device::kb_p2_w));
	m_maincpu->bus_in_cb().set(FUNC(tandy2k_keyboard_device::kb_p1_r));
	m_maincpu->set_disable(); // TODO
}


//-------------------------------------------------
//  INPUT_PORTS( tandy2k_keyboard )
//-------------------------------------------------

INPUT_PORTS_START( tandy2k_keyboard )
	PORT_START("Y0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y2")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y3")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y4")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y5")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y6")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y8")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y9")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y10")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y11")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor tandy2k_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( tandy2k_keyboard );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  tandy2k_keyboard_device - constructor
//-------------------------------------------------

tandy2k_keyboard_device::tandy2k_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, TANDY2K_KEYBOARD, tag, owner, clock),
	m_maincpu(*this, I8048_TAG),
	m_y(*this, "Y%u", 0),
	m_leds(*this, "led%u", 0U),
	m_write_clock(*this),
	m_write_data(*this),
	m_keylatch(0xffff),
	m_clock(0),
	m_data(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tandy2k_keyboard_device::device_start()
{
	m_leds.resolve();
	// resolve callbacks
	m_write_clock.resolve_safe();
	m_write_data.resolve_safe();

	// state saving
	save_item(NAME(m_keylatch));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tandy2k_keyboard_device::device_reset()
{
}


//-------------------------------------------------
//  power_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( tandy2k_keyboard_device::power_w )
{
	// TODO
}


//-------------------------------------------------
//  reset_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( tandy2k_keyboard_device::reset_w )
{
	if (!state)
	{
		device_reset();
	}
}


//-------------------------------------------------
//  busy_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( tandy2k_keyboard_device::busy_w )
{
	m_maincpu->set_input_line(MCS48_INPUT_IRQ, state ? CLEAR_LINE : ASSERT_LINE);
}


//-------------------------------------------------
//  data_r -
//-------------------------------------------------

READ_LINE_MEMBER( tandy2k_keyboard_device::data_r )
{
	return m_data;
}


//-------------------------------------------------
//  kb_p1_r -
//-------------------------------------------------

uint8_t tandy2k_keyboard_device::kb_p1_r()
{
	/*

	    bit     description

	    0       X0
	    1       X1
	    2       X2
	    3       X3
	    4       X4
	    5       X5
	    6       X6
	    7       X7

	*/

	uint8_t data = 0xff;

	for (int i = 0; i < 12; i++)
	{
		if (!BIT(m_keylatch, i)) data &= m_y[i]->read();
	}

	return ~data;
}


//-------------------------------------------------
//  kb_p1_w -
//-------------------------------------------------

void tandy2k_keyboard_device::kb_p1_w(uint8_t data)
{
	/*

	    bit     description

	    0       Y0
	    1       Y1
	    2       Y2
	    3       Y3
	    4       Y4
	    5       Y5
	    6       Y6
	    7       Y7

	*/

	// keyboard row
	m_keylatch = (m_keylatch & 0xff00) | data;
}


//-------------------------------------------------
//  kb_p2_w -
//-------------------------------------------------

void tandy2k_keyboard_device::kb_p2_w(uint8_t data)
{
	/*

	    bit     description

	    0       Y8
	    1       Y9
	    2       Y10
	    3       Y11
	    4       LED 2
	    5       LED 1
	    6       CLOCK
	    7       DATA

	*/

	// keyboard row
	m_keylatch = ((data & 0x0f) << 8) | (m_keylatch & 0xff);

	// led output
	m_leds[LED_2] = BIT(~data, 4);
	m_leds[LED_1] = BIT(~data, 5);

	// keyboard clock
	int clock = BIT(data, 6);

	if (m_clock != clock)
	{
		m_clock = clock;

		m_write_clock(m_clock);
	}

	// keyboard data
	int kbddat = BIT(data, 7);

	if (m_data != kbddat)
	{
		m_data = kbddat;

		m_write_data(m_data);
	}
}
