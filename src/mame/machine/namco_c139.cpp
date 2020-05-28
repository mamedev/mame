// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Namco C139 - Serial I/F Controller


    TODO:
    - Make this to actually work!
    - Is RAM shared with a specific CPU other than master/slave?
    - is this another MCU with internal ROM?

***************************************************************************/

#include "emu.h"
#include "namco_c139.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(NAMCO_C139, namco_c139_device, "namco_c139", "Namco C139 Serial")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

void namco_c139_device::data_map(address_map &map)
{
	map(0x0000, 0x3fff).ram().share("sharedram");
}

void namco_c139_device::regs_map(address_map &map)
{
	map(0x00, 0x01).r(FUNC(namco_c139_device::status_r)); // WRITE clears flags
	map(0x02, 0x03).noprw(); // settings?
//  map(0x0a, 0x0b) // WRITE tx_w
//  map(0x0c, 0x0d) // READ rx_r
//  map(0x0e, 0x0f) //
}

//-------------------------------------------------
//  namco_c139_device - constructor
//-------------------------------------------------

namco_c139_device::namco_c139_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NAMCO_C139, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	m_space_config("data", ENDIANNESS_BIG, 16, 14, 0, address_map_constructor(FUNC(namco_c139_device::data_map), this))
{
}




//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void namco_c139_device::device_start()
{
	m_ram = (uint16_t*)memshare("sharedram")->ptr();

}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void namco_c139_device::device_reset()
{
}

//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector namco_c139_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_DATA, &m_space_config)
	};
}

//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

READ16_MEMBER(namco_c139_device::ram_r)
{
	return m_ram[offset];
}

WRITE16_MEMBER(namco_c139_device::ram_w)
{
	COMBINE_DATA(&m_ram[offset]);
}

READ16_MEMBER(namco_c139_device::status_r)
{
	/*
	 x-- RX READY or irq pending?
	 -x- IRQ direction: 1 RX cause - 0 TX cause
	*/
	return 4; // STATUS bit?
}
