// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    Fujitsu Micro MB9061x Microcontroller Family
    Emulation by R. Belmont
*/

#include "emu.h"
#include "mb9061x.h"

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(MB90610A, mb90610_device, "mb90610a", "Fujitsu MB90610A")
DEFINE_DEVICE_TYPE(MB90611A, mb90611_device, "mb90611a", "Fujitsu MB90611A")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mb9061x_device - constructor
//-------------------------------------------------
mb9061x_device::mb9061x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor internal_map) :
	f2mc16_device(mconfig, type, tag, owner, clock),
	m_program_config("program", ENDIANNESS_LITTLE, 8, 24, 0, internal_map)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mb9061x_device::device_start()
{
	f2mc16_device::device_start();
}


device_memory_interface::space_config_vector mb9061x_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config)
	};
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mb9061x_device::device_reset()
{
	f2mc16_device::device_reset();
}

void mb9061x_device::execute_set_input(int inputnum, int state)
{
}

/* MB90610 - "Evaluation device" with extra RAM */
void mb90610_device::mb90610_map(address_map &map)
{
	map(0x0100, 0x10ff).ram();  // 4K of internal RAM from 0x100 to 0x1100
}

mb90610_device::mb90610_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	mb90610_device(mconfig, MB90610A, tag, owner, clock)
{
}

mb90610_device::mb90610_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	mb9061x_device(mconfig, type, tag, owner, clock, address_map_constructor(FUNC(mb90610_device::mb90610_map), this))
{
}

/* MB90611 - Production version of this series */
void mb90611_device::mb90611_map(address_map &map)
{
	map(0x0100, 0x04ff).ram();  // 1K of internal RAM from 0x100 to 0x500
}

mb90611_device::mb90611_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	mb90611_device(mconfig, MB90611A, tag, owner, clock)
{
}

mb90611_device::mb90611_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	mb9061x_device(mconfig, type, tag, owner, clock, address_map_constructor(FUNC(mb90611_device::mb90611_map), this))
{
}
