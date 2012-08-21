/*********************************************************************

    laser128.c

    Helper to implement the Laser 128's built-in slot peripherals

*********************************************************************/

#include "laser128.h"
#include "includes/apple2.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type A2BUS_LASER128 = &device_creator<a2bus_laser128_device>;

MACHINE_CONFIG_FRAGMENT( a2laser128 )
MACHINE_CONFIG_END

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor a2bus_laser128_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( a2laser128 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_laser128_device::a2bus_laser128_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock) :
    device_t(mconfig, type, name, tag, owner, clock),
    device_a2bus_card_interface(mconfig, *this)
{
	m_shortname = "a2laser128";
}

a2bus_laser128_device::a2bus_laser128_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
    device_t(mconfig, A2BUS_LASER128, "VTech Laser 128 Internal Device", tag, owner, clock),
    device_a2bus_card_interface(mconfig, *this)
{
	m_shortname = "a2laser128";
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_laser128_device::device_start()
{
	// set_a2bus_device makes m_slot valid
	set_a2bus_device();
}

void a2bus_laser128_device::device_reset()
{
    m_rom = device().machine().root_device().memregion("maincpu")->base();
    m_slot7_bank = 0;
}

UINT8 a2bus_laser128_device::read_c0nx(address_space &space, UINT8 offset)
{
    return 0x00;
}

void a2bus_laser128_device::write_c0nx(address_space &space, UINT8 offset, UINT8 data)
{
}

UINT8 a2bus_laser128_device::read_cnxx(address_space &space, UINT8 offset)
{
    return m_rom[offset + (m_slot * 0x100) + 0x4000];
}

UINT8 a2bus_laser128_device::read_c800(address_space &space, UINT16 offset)
{
    switch (m_slot)
    {
        case 1:
            return m_rom[(offset & 0x7ff) + 0x4800];

        case 2:
            return m_rom[(offset & 0x7ff) + 0x5800];

        case 5:
            return m_rom[(offset & 0x7ff) + 0x5000];

        case 6:
            return m_rom[(offset & 0x7ff) + 0x7800];

        case 7:
            return m_rom[(offset & 0x7ff) + 0x5c00 + m_slot7_bank];
    }

    return 0xff;
}

void a2bus_laser128_device::write_c800(address_space &space, UINT16 offset, UINT8 data)
{
}

bool a2bus_laser128_device::take_c800()
{
    if ((m_slot == 1) || (m_slot == 2) || (m_slot == 5) || (m_slot == 7))
    {
        return true;
    }

    return false;
}

