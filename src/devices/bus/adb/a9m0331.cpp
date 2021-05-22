// license:BSD-3-Clause
// copyright-holders: R. Belmont
/*********************************************************************

    a9m0331.cpp
    Apple standard ADB mouse
    Skeleton by R. Belmont

    TODO: Everything.
    What 6805 or 68705 sub-model is this?
    Is the boot vector actually correct?

*********************************************************************/

#include "emu.h"
#include "a9m0331.h"

DEFINE_DEVICE_TYPE(ADB_A9M0331, a9m0331_device, "a9m0331", "Apple ADB Mouse (A9M0331)");

ROM_START(a9m0331)
	ROM_REGION(0x800, "mcu", 0)
	ROM_LOAD( "lsc84488p_1986_3.0.bin", 0x000000, 0x000800, CRC(572a11e0) SHA1(caab6c60b18670e5ea597e35e526032e33b6e8c7) )
	ROM_LOAD( "zc84506p_1987_ea_3.2_trim.bin", 0x000000, 0x000800, CRC(85f858d9) SHA1(61ebad9953f4f88b6fcabf498f3875918493d138) )
ROM_END

static INPUT_PORTS_START(a9m0331)
	PORT_START("button")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Mouse Button") PORT_CODE(MOUSECODE_BUTTON1)

	PORT_START("mousex")
	PORT_BIT(0xff, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(40) PORT_KEYDELTA(0) PORT_PLAYER(1)

	PORT_START("mousey")
	PORT_BIT(0xff, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(40) PORT_KEYDELTA(0) PORT_PLAYER(1)
INPUT_PORTS_END

/***************************************************************************
    DEVICE CONFIGURATION
***************************************************************************/

/*-------------------------------------------------
    input_ports - device-specific input ports
-------------------------------------------------*/
ioport_constructor a9m0331_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(a9m0331);
}

/*-------------------------------------------------
    device_add_mconfig - device-specific
    machine configurations
-------------------------------------------------*/
void a9m0331_device::device_add_mconfig(machine_config &config)
{
	M68705P3(config, m_mcu, 2048000);
	m_mcu->porta_r().set(FUNC(a9m0331_device::mcu_port_a_r));
	m_mcu->portb_r().set(FUNC(a9m0331_device::mcu_port_b_r));
	m_mcu->portc_r().set(FUNC(a9m0331_device::mcu_port_c_r));
	m_mcu->porta_w().set(FUNC(a9m0331_device::mcu_port_a_w));
	m_mcu->portb_w().set(FUNC(a9m0331_device::mcu_port_b_w));
	m_mcu->portc_w().set(FUNC(a9m0331_device::mcu_port_c_w));
}

/*-------------------------------------------------
    rom_region - device-specific ROM region
-------------------------------------------------*/
const tiny_rom_entry *a9m0331_device::device_rom_region() const
{
	return ROM_NAME(a9m0331);
}

/***************************************************************************
    DEVICE IMPLEMENTATION
***************************************************************************/

a9m0331_device::a9m0331_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	adb_device(mconfig, ADB_A9M0331, tag, owner, clock),
	adb_slot_card_interface(mconfig, *this, DEVICE_SELF),
	m_mcu(*this, "mcu")
{
}

void a9m0331_device::device_start()
{
	adb_device::device_start();
}

void a9m0331_device::device_reset()
{
	adb_device::device_reset();
}

void a9m0331_device::adb_w(int state)
{
}

void a9m0331_device::mcu_port_a_w(u8 data)
{
	logerror("%02x to port A\n", data);
}

void a9m0331_device::mcu_port_b_w(u8 data)
{
	logerror("%02x to port B\n", data);
}

void a9m0331_device::mcu_port_c_w(u8 data)
{
	logerror("%02x to port C\n", data);
}

u8 a9m0331_device::mcu_port_a_r()
{
	return 0xff;
}

u8 a9m0331_device::mcu_port_b_r()
{
	return 0xff;
}

u8 a9m0331_device::mcu_port_c_r()
{
	return 0xff;
}
