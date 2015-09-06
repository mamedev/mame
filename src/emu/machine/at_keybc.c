// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

    IBM PC AT compatibles 8042 keyboard controller

***************************************************************************/

#include "emu.h"
#include "at_keybc.h"
#include "cpu/mcs48/mcs48.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type AT_KEYBOARD_CONTROLLER = &device_creator<at_keyboard_controller_device>;

// i/o map for the 8042
static ADDRESS_MAP_START( at_keybc_io, AS_IO, 8, at_keyboard_controller_device)
	AM_RANGE(MCS48_PORT_T0, MCS48_PORT_T0) AM_READ(t0_r)
	AM_RANGE(MCS48_PORT_T1, MCS48_PORT_T1) AM_READ(t1_r)
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_READ( p1_r)
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_READWRITE(p2_r, p2_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( at_keybc )
	PORT_START("DSW")
	PORT_BIT(     0xbf, 0xbf, IPT_UNUSED )
	PORT_DIPNAME( 0x40, 0x00, "Display switch")
	PORT_DIPSETTING(    0x40, "Monochrome adapter" )
	PORT_DIPSETTING(    0x00, "Color/Graphics adapter" )
INPUT_PORTS_END

// machine fragment
static MACHINE_CONFIG_FRAGMENT( at_keybc )
	MCFG_CPU_ADD("at_keybc", I8042, DERIVED_CLOCK(1,1))
	MCFG_CPU_IO_MAP(at_keybc_io)
MACHINE_CONFIG_END

// rom definition for the 8042 internal rom
ROM_START( at_keybc )
	ROM_REGION(0x800, "at_keybc", 0)

	// unknown controller bios, (c) 1985, 1986 PTL
	ROM_LOAD("yan25d05.bin", 0x000, 0x800, CRC(70c798f1) SHA1(ae9a79c7184a17331b70a50035ff63c757df094c))

	// 1983 ibm controller bios
	ROM_LOAD("1503033.bin", 0x000, 0x800, CRC(5a81c0d2) SHA1(0100f8789fb4de74706ae7f9473a12ec2b9bd729))
ROM_END

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  at_keyboard_controller_device - constructor
//-------------------------------------------------

at_keyboard_controller_device::at_keyboard_controller_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, AT_KEYBOARD_CONTROLLER, "AT Keyboard Controller", tag, owner, clock, "at_keybc", __FILE__),
		m_cpu(NULL),
		m_system_reset_cb(*this),
		m_gate_a20_cb(*this),
		m_input_buffer_full_cb(*this),
		m_output_buffer_empty_cb(*this),
		m_keyboard_clock_cb(*this),
		m_keyboard_data_cb(*this)
{
}

//-------------------------------------------------
//  rom_region - return a pointer to the device's
//  internal ROM region
//-------------------------------------------------

const rom_entry *at_keyboard_controller_device::device_rom_region() const
{
	return ROM_NAME(at_keybc);
}

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor at_keyboard_controller_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( at_keybc );
}

//-------------------------------------------------
//  machine_config_additions - return a pointer to
//  the device's machine fragment
//-------------------------------------------------

machine_config_constructor at_keyboard_controller_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(at_keybc);
}

/*-------------------------------------------------
    device_start - device-specific startup
-------------------------------------------------*/

void at_keyboard_controller_device::device_start()
{
	// find our cpu
	m_cpu = downcast<upi41_cpu_device *>(subdevice("at_keybc"));

	// resolve callbacks
	m_system_reset_cb.resolve_safe();
	m_gate_a20_cb.resolve_safe();
	m_input_buffer_full_cb.resolve_safe();
	m_output_buffer_empty_cb.resolve_safe();
	m_keyboard_clock_cb.resolve_safe();
	m_keyboard_data_cb.resolve_safe();

	// register for save states
	save_item(NAME(m_clock_signal));
	save_item(NAME(m_data_signal));
}

/*-------------------------------------------------
    device_reset - device-specific reset
-------------------------------------------------*/

void at_keyboard_controller_device::device_reset()
{
}


//**************************************************************************
//  INTERNAL 8042 READ/WRITE HANDLERS
//**************************************************************************

READ8_MEMBER( at_keyboard_controller_device::t0_r )
{
	return m_clock_signal;
}

READ8_MEMBER( at_keyboard_controller_device::t1_r )
{
	return m_data_signal;
}

/*
    Port 1 (Input port)
    0 - P10 - Undefined
    1 - P11 - Undefined
    2 - P12 - Undefined
    3 - P13 - Undefined
    4 - P14 - External RAM (1 = Enable external RAM, 0 = Disable external RAM)
    5 - P15 - Manufacturing setting (1 = Setting enabled, 0 = Setting disabled)
    6 - P16 - Display type switch (1 = Monochrome display, 0 = Color display)
    7 - P17 - Keyboard inhibit switch (1 = Keyboard enabled, 0 = Keyboard inhibited)
*/
READ8_MEMBER( at_keyboard_controller_device::p1_r )
{
	return ioport("DSW")->read();
}

READ8_MEMBER( at_keyboard_controller_device::p2_r )
{
	return 0xff;
}

/*
    Port 2 (Output port)
    0 - P20 - System Reset (1 = Normal, 0 = Reset computer)
    1 - P21 - Gate A20
    2 - P22 - Undefined
    3 - P23 - Undefined
    4 - P24 - Input Buffer Full
    5 - P25 - Output Buffer Empty
    6 - P26 - Keyboard Clock (1 = Pull Clock low, 0 = High-Z)
    7 - P27 - Keyboard Data (1 = Pull Data low, 0 = High-Z)
*/
WRITE8_MEMBER( at_keyboard_controller_device::p2_w )
{
	m_system_reset_cb(BIT(data, 0) ? CLEAR_LINE : ASSERT_LINE);
	m_gate_a20_cb(BIT(data, 1) ? ASSERT_LINE : CLEAR_LINE);
	m_input_buffer_full_cb(BIT(data, 4) ? ASSERT_LINE : CLEAR_LINE);
	m_output_buffer_empty_cb(BIT(data, 5) ? ASSERT_LINE : CLEAR_LINE);

	m_clock_signal = !BIT(data, 6);
	m_data_signal = BIT(data, 7);

	m_keyboard_data_cb(m_data_signal);
	m_keyboard_clock_cb(m_clock_signal);
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

READ8_MEMBER( at_keyboard_controller_device::data_r )
{
	return m_cpu->upi41_master_r(space, 0);
}

WRITE8_MEMBER( at_keyboard_controller_device::data_w )
{
	m_cpu->upi41_master_w(space, 0, data);
}

READ8_MEMBER( at_keyboard_controller_device::status_r )
{
	return m_cpu->upi41_master_r(space, 1);
}

WRITE8_MEMBER( at_keyboard_controller_device::command_w )
{
	m_cpu->upi41_master_w(space, 1, data);
}

WRITE_LINE_MEMBER( at_keyboard_controller_device::keyboard_clock_w )
{
	m_clock_signal = state;
}

WRITE_LINE_MEMBER( at_keyboard_controller_device::keyboard_data_w )
{
	m_data_signal = state;
}
