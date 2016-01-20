// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 Generic RAM socket emulation

 This offers generic access to RAM

 generic_ram_plain  : returns 0xff when the system reads beyond the end of the RAM
 generic_ram_linear : maps linearly the RAM in the accessed area (i.e., read/write offset is masked with
 (RAM size - 1) )

 TODO:
   - support variable RAM size
   - possibly support linear mapping when non-power of 2 RAMs are mapped
   - add support for 16bit & 32bit RAM access

 ***********************************************************************************************************/


#include "emu.h"
#include "ram.h"

//-------------------------------------------------
//  constructor
//-------------------------------------------------

const device_type GENERIC_RAM_32K_PLAIN = &device_creator<generic_ram_32k_plain_device>;
const device_type GENERIC_RAM_64K_PLAIN = &device_creator<generic_ram_64k_plain_device>;
const device_type GENERIC_RAM_128K_PLAIN = &device_creator<generic_ram_128k_plain_device>;
const device_type GENERIC_RAM_32K_LINEAR = &device_creator<generic_ram_32k_linear_device>;
const device_type GENERIC_RAM_64K_LINEAR = &device_creator<generic_ram_64k_linear_device>;
const device_type GENERIC_RAM_128K_LINEAR = &device_creator<generic_ram_128k_linear_device>;


generic_ram_plain_device::generic_ram_plain_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, UINT32 size, std::string shortname, std::string source)
						: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
							device_generic_cart_interface(mconfig, *this),
							m_size(size)
{
}

generic_ram_linear_device::generic_ram_linear_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, UINT32 size, std::string shortname, std::string source)
						: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
							device_generic_cart_interface(mconfig, *this),
							m_size(size)
{
}


generic_ram_32k_plain_device::generic_ram_32k_plain_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: generic_ram_plain_device(mconfig, GENERIC_RAM_32K_PLAIN, "Generic RAM 32K (plain mapping)", tag, owner, clock, 0x8000, "generic_ram32p", __FILE__)
{
}

generic_ram_64k_plain_device::generic_ram_64k_plain_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: generic_ram_plain_device(mconfig, GENERIC_RAM_64K_PLAIN, "Generic RAM 64K (plain mapping)", tag, owner, clock, 0x10000, "generic_ram64p", __FILE__)
{
}

generic_ram_128k_plain_device::generic_ram_128k_plain_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: generic_ram_plain_device(mconfig, GENERIC_RAM_128K_PLAIN, "Generic RAM 128K (plain mapping)", tag, owner, clock, 0x20000, "generic_ram128p", __FILE__)
{
}

generic_ram_32k_linear_device::generic_ram_32k_linear_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: generic_ram_linear_device(mconfig, GENERIC_RAM_32K_LINEAR, "Generic RAM 32K (linear mapping)", tag, owner, clock, 0x8000, "generic_ram32l", __FILE__)
{
}

generic_ram_64k_linear_device::generic_ram_64k_linear_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: generic_ram_linear_device(mconfig, GENERIC_RAM_64K_LINEAR, "Generic RAM 64K (linear mapping)", tag, owner, clock, 0x10000, "generic_ram64l", __FILE__)
{
}

generic_ram_128k_linear_device::generic_ram_128k_linear_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: generic_ram_linear_device(mconfig, GENERIC_RAM_128K_LINEAR, "Generic RAM 128K (linear mapping)", tag, owner, clock, 0x20000, "generic_ram128l", __FILE__)
{
}


void generic_ram_plain_device::device_start()
{
	m_ram.resize(m_size);
	save_item(NAME(m_ram));
}

void generic_ram_linear_device::device_start()
{
	m_ram.resize(m_size);
	save_item(NAME(m_ram));
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

READ8_MEMBER(generic_ram_plain_device::read_ram)
{
	if (offset < m_ram.size())
		return m_ram[offset];
	else
		return 0xff;
}

WRITE8_MEMBER(generic_ram_plain_device::write_ram)
{
	if (offset < m_ram.size())
		m_ram[offset] = data;
}


READ8_MEMBER(generic_ram_linear_device::read_ram)
{
	return m_ram[offset % m_ram.size()];
}

WRITE8_MEMBER(generic_ram_linear_device::write_ram)
{
	m_ram[offset % m_ram.size()] = data;
}
