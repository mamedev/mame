// license:BSD-3-Clause
// copyright-holders:David Haywood, NaokiS
/* Modern device for the MCF5206e Peripherals
 this can be hooked properly to the CPU once the CPU is a modern device too
*/

#include "emu.h"
#include "mcf5206e.h"


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(MCF5206E_PERIPHERAL, mcf5206e_peripheral_device, "mcf5206e_peripheral", "MCF5206E Peripheral")

//-------------------------------------------------
//  mcf5206e_peripheral_device - constructor
//-------------------------------------------------

mcf5206e_peripheral_device::mcf5206e_peripheral_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MCF5206E_PERIPHERAL, tag, owner, clock)
		, device_memory_interface(mconfig, *this)
{
}

device_memory_interface::space_config_vector mcf5206e_peripheral_device::memory_space_config() const
{
	return space_config_vector {
		//std::make_pair(0, &m_space_config)
	};
}



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

uint32_t mcf5206e_peripheral_device::dev_r(offs_t offset, uint32_t mem_mask)
{
	address_space &reg_space = this->space();
	return reg_space.read_dword(offset*4, mem_mask);
}

void mcf5206e_peripheral_device::dev_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	address_space &reg_space = this->space();
	reg_space.write_dword(offset*4, data, mem_mask);
}


// ColdFire peripherals

enum {
	CF_PPDAT    =   0x1c8/4,
	CF_MBSR     =   0x1ec/4
};

void mcf5206e_peripheral_device::seta2_coldfire_regs_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA( &m_coldfire_regs[offset] );
}

uint32_t mcf5206e_peripheral_device::seta2_coldfire_regs_r(offs_t offset)
{
	switch( offset )
	{
		case CF_MBSR:
			return machine().rand();

		case CF_PPDAT:
			return ioport(":BATTERY")->read() << 16;
	}

	return m_coldfire_regs[offset];
}

