// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
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

const device_type DMV_K200 = &device_creator<dmv_k200_device>;
const device_type DMV_K202 = &device_creator<dmv_k202_device>;
const device_type DMV_K208 = &device_creator<dmv_k208_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  dmv_ram_device - constructor
//-------------------------------------------------

dmv_ram_device::dmv_ram_device(const machine_config &mconfig, device_type type, UINT32 size, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source)
		: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_dmvslot_interface( mconfig, *this ), m_ram(nullptr),
		m_size(size)
{
}

//-------------------------------------------------
//  dmv_k200_device - constructor
//-------------------------------------------------

dmv_k200_device::dmv_k200_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
		: dmv_ram_device(mconfig, DMV_K200, 1, "K200 64K RAM expansion", tag, owner, clock, "dmv_k200", __FILE__)
{
}

//-------------------------------------------------
//  dmv_k202_device - constructor
//-------------------------------------------------

dmv_k202_device::dmv_k202_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
		: dmv_ram_device(mconfig, DMV_K202, 3, "K202 192K RAM expansion", tag, owner, clock, "dmv_k202", __FILE__)
{
}

//-------------------------------------------------
//  dmv_k208_device - constructor
//-------------------------------------------------

dmv_k208_device::dmv_k208_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
		: dmv_ram_device(mconfig, DMV_K208, 7 , "K208 448K RAM expansion", tag, owner, clock, "dmv_k208", __FILE__)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dmv_ram_device::device_start()
{
	m_ram = machine().memory().region_alloc( "expram", m_size * 0x10000, 1, ENDIANNESS_LITTLE )->base();
}

//-------------------------------------------------
//  read
//-------------------------------------------------

void dmv_ram_device::ram_read(UINT8 cas, offs_t offset, UINT8 &data)
{
	if (cas && cas <= m_size)
		data = m_ram[((cas - 1) << 16) | (offset & 0xffff)];
}

//-------------------------------------------------
//  write
//-------------------------------------------------

void dmv_ram_device::ram_write(UINT8 cas, offs_t offset, UINT8 data)
{
	if (cas && cas <= m_size)
		m_ram[((cas - 1) << 16) | (offset & 0xffff)] = data;
}
