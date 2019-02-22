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

DEFINE_DEVICE_TYPE(GENERIC_RAM_32K_PLAIN,   generic_ram_32k_plain_device,    "generic_ram32p",  "Generic RAM 32K (plain mapping)")
DEFINE_DEVICE_TYPE(GENERIC_RAM_64K_PLAIN,   generic_ram_64k_plain_device,    "generic_ram64p",  "Generic RAM 64K (plain mapping)")
DEFINE_DEVICE_TYPE(GENERIC_RAM_128K_PLAIN,  generic_ram_128k_plain_device,   "generic_ram128p", "Generic RAM 128K (plain mapping)")
DEFINE_DEVICE_TYPE(GENERIC_RAM_32K_LINEAR,  generic_ram_32k_linear_device,   "generic_ram32l",  "Generic RAM 32K (linear mapping)")
DEFINE_DEVICE_TYPE(GENERIC_RAM_64K_LINEAR,  generic_ram_64k_linear_device,   "generic_ram64l",  "Generic RAM 64K (linear mapping)")
DEFINE_DEVICE_TYPE(GENERIC_RAM_128K_LINEAR, generic_ram_128k_linear_device,  "generic_ram128l", "Generic RAM 128K (linear mapping)")


generic_ram_plain_device::generic_ram_plain_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t size)
	: device_t(mconfig, type, tag, owner, clock)
	, device_generic_cart_interface(mconfig, *this)
	, m_size(size)
{
}

generic_ram_linear_device::generic_ram_linear_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t size)
	: device_t(mconfig, type, tag, owner, clock)
	, device_generic_cart_interface(mconfig, *this)
	, m_size(size)
{
}


generic_ram_32k_plain_device::generic_ram_32k_plain_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: generic_ram_plain_device(mconfig, GENERIC_RAM_32K_PLAIN, tag, owner, clock, 0x8000)
{
}

generic_ram_64k_plain_device::generic_ram_64k_plain_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: generic_ram_plain_device(mconfig, GENERIC_RAM_64K_PLAIN, tag, owner, clock, 0x10000)
{
}

generic_ram_128k_plain_device::generic_ram_128k_plain_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: generic_ram_plain_device(mconfig, GENERIC_RAM_128K_PLAIN, tag, owner, clock, 0x20000)
{
}

generic_ram_32k_linear_device::generic_ram_32k_linear_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: generic_ram_linear_device(mconfig, GENERIC_RAM_32K_LINEAR, tag, owner, clock, 0x8000)
{
}

generic_ram_64k_linear_device::generic_ram_64k_linear_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: generic_ram_linear_device(mconfig, GENERIC_RAM_64K_LINEAR, tag, owner, clock, 0x10000)
{
}

generic_ram_128k_linear_device::generic_ram_128k_linear_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: generic_ram_linear_device(mconfig, GENERIC_RAM_128K_LINEAR, tag, owner, clock, 0x20000)
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

uint8_t generic_ram_plain_device::read_ram(offs_t offset)
{
	if (offset < m_ram.size())
		return m_ram[offset];
	else
		return 0xff;
}

void generic_ram_plain_device::write_ram(offs_t offset, uint8_t data)
{
	if (offset < m_ram.size())
		m_ram[offset] = data;
}


uint8_t generic_ram_linear_device::read_ram(offs_t offset)
{
	return m_ram[offset % m_ram.size()];
}

void generic_ram_linear_device::write_ram(offs_t offset, uint8_t data)
{
	m_ram[offset % m_ram.size()] = data;
}
