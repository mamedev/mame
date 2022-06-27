// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*********************************************************************

    fmt_icmem.cpp

    FM Towns IC Memory Card
    PCMCIA SRAM Memory Cards, up to 64MB supported

*********************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "fmt_icmem.h"

// device type definition
DEFINE_DEVICE_TYPE(FMT_ICMEM, fmt_icmem_device, "fmt_icmem", "FM Towns IC Memory Card")

//-------------------------------------------------
//  fmt_icmem_device - constructor
//-------------------------------------------------

fmt_icmem_device::fmt_icmem_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, FMT_ICMEM, tag, owner, clock),
		device_memcard_image_interface(mconfig, *this),
		m_writeprotect(*this,"icmem"),
		m_change(false),
		m_attr_select(false),
		m_detect(false),
		m_bank(0)
{
}


static INPUT_PORTS_START( fmt_icmem )
	PORT_START("icmem")
	PORT_CONFNAME(0x01, 0x00, "IC Memory Card Write Protect")
	PORT_CONFSETTING(0x00, DEF_STR( Off ))
	PORT_CONFSETTING(0x01, DEF_STR( On ))
INPUT_PORTS_END


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void fmt_icmem_device::device_start()
{
	m_memcard_ram = std::make_unique<uint8_t[]>(0x1000000);
	m_bank = 0;
	m_detect = false;
	m_change = false;
	save_item(NAME(m_change));
	save_item(NAME(m_detect));
	save_item(NAME(m_bank));
}

ioport_constructor fmt_icmem_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(fmt_icmem);
}

image_init_result fmt_icmem_device::call_load()
{
	memset(m_memcard_ram.get(), 0xff, 0x1000000);
	fseek(0, SEEK_SET);
	size_t ret = fread(m_memcard_ram.get(), 0x1000000);

	if(ret != length())
		return image_init_result::FAIL;

	m_change = true;
	m_detect = true;
	return image_init_result::PASS;
}

void fmt_icmem_device::call_unload()
{
	fseek(0, SEEK_SET);
	if(!m_writeprotect->read())
		fwrite(m_memcard_ram.get(), 0x1000000);
	m_change = true;
	m_detect = false;
}

image_init_result fmt_icmem_device::call_create(int format_type, util::option_resolution *format_options)
{
	memset(m_memcard_ram.get(), 0xff, 0x1000000);

	size_t ret = fwrite(m_memcard_ram.get(), 0x1000000);
	if(ret != 0x1000000)
		return image_init_result::FAIL;

	m_change = true;
	m_detect = true;
	return image_init_result::PASS;
}


uint8_t fmt_icmem_device::static_mem_read(offs_t offset)
{
	return m_memcard_ram[offset];
}

void fmt_icmem_device::static_mem_write(offs_t offset, uint8_t data)
{
	m_memcard_ram[offset] = data;
}

uint8_t fmt_icmem_device::mem_read(offs_t offset)
{
	return m_memcard_ram[(m_bank*0x100000) + offset];
}

void fmt_icmem_device::mem_write(offs_t offset, uint8_t data)
{
	m_memcard_ram[(m_bank*0x100000) + offset] = data;
}

// Memory Card status:
//  bit 0 - 0 = Write Enable, 1 = Write Protect
//  bit 1,2 - Card Detect - 00 = Card Inserted, 11 = No card inserted, 10 or 01 = Card error?
//  bit 3,4,5 - not memory card related (EEPROM and backup battery level)
//  bit 6 - unknown
//  bit 7 - 1 = card changed, flips back to 0 when read
uint8_t fmt_icmem_device::status_r()
{
	uint8_t ret = 0x00;

	ret |= (m_writeprotect->read() & 0x01);
	if(is_readonly())  // if image is read-only, then set write protect.
		ret |= 0x01;
	if(!m_detect)
		ret |= 0x06;
	if(m_change)
		ret |= 0x80;
	m_change = false;

	return ret;
}

// Memory Card bank select (0x490)
//  bit 0-5: bank select (bits 0-3 not used in non-386SX systems?)
// Attribute/Common memory select (0x491)
//  bit 0: 0 = common memory, 1 = attribute memory (TODO)
//  bit 7: 0 indicates that card is JEIDA v4 compliant
uint8_t fmt_icmem_device::bank_r(offs_t offset)
{
	switch(offset)
	{
	case 0:
		return m_bank & 0x0f;
	case 1:
		return m_attr_select ? 1 : 0;
	}
	return 0xff;
}

void fmt_icmem_device::bank_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
	case 0:
		m_bank = data & 0x0f;
		break;
	case 1:
		m_attr_select = data & 0x01;
		break;
	}
}

