// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
// thanks-to:rfka01
/***************************************************************************

    K200 64K RAM expansion
    K202 192K RAM expansion
    K208 448K RAM expansion

***************************************************************************/

#include "emu.h"
#include "ram.h"


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(DMV_K200, dmv_k200_device, "dmv_k200", "K200 64K RAM expansion")
DEFINE_DEVICE_TYPE(DMV_K202, dmv_k202_device, "dmv_k202", "K202 192K RAM expansion")
DEFINE_DEVICE_TYPE(DMV_K208, dmv_k208_device, "dmv_k208", "K208 448K RAM expansion")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  dmv_ram_device_base - constructor
//-------------------------------------------------

dmv_ram_device_base::dmv_ram_device_base(const machine_config &mconfig, device_type type, uint32_t size, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_dmvslot_interface(mconfig, *this)
	, m_ram(nullptr)
	, m_size(size)
{
}

//-------------------------------------------------
//  dmv_k200_device - constructor
//-------------------------------------------------

dmv_k200_device::dmv_k200_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: dmv_ram_device_base(mconfig, DMV_K200, 1, tag, owner, clock)
{
}

//-------------------------------------------------
//  dmv_k202_device - constructor
//-------------------------------------------------

dmv_k202_device::dmv_k202_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: dmv_ram_device_base(mconfig, DMV_K202, 3, tag, owner, clock)
{
}

//-------------------------------------------------
//  dmv_k208_device - constructor
//-------------------------------------------------

dmv_k208_device::dmv_k208_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: dmv_ram_device_base(mconfig, DMV_K208, 7, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dmv_ram_device_base::device_start()
{
	m_ram = machine().memory().region_alloc( "expram", m_size * 0x10000, 1, ENDIANNESS_LITTLE )->base();

	// register for state saving
	save_pointer(NAME(m_ram), m_size * 0x10000);
}

//-------------------------------------------------
//  read
//-------------------------------------------------

void dmv_ram_device_base::ram_read(uint8_t cas, offs_t offset, uint8_t &data)
{
	if (cas && cas <= m_size)
		data = m_ram[((cas - 1) << 16) | (offset & 0xffff)];
}

//-------------------------------------------------
//  write
//-------------------------------------------------

void dmv_ram_device_base::ram_write(uint8_t cas, offs_t offset, uint8_t data)
{
	if (cas && cas <= m_size)
		m_ram[((cas - 1) << 16) | (offset & 0xffff)] = data;
}
