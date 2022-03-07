// license:BSD-3-Clause
// copyright-holders:smf,Barry Rodewald
/***************************************************************************

    x2212.c

    Xicor X2212 256 x 4 bit Nonvolatile Static RAM.

***************************************************************************/

#include "emu.h"
#include "machine/x2212.h"

#include <algorithm>


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(X2212, x2212_device, "x2212", "Xicor X2212 256x4 NOVRAM")
DEFINE_DEVICE_TYPE(X2210, x2210_device, "x2210", "Xicor X2210 64x4 NOVRAM")

//-------------------------------------------------
//  x2212_device - constructor
//-------------------------------------------------

x2212_device::x2212_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: x2212_device(mconfig, X2212, tag, owner, clock, 0x100)
{
}

x2212_device::x2212_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int size_data)
	: device_t(mconfig, type, tag, owner, clock)
	, device_nvram_interface(mconfig, *this)
	, m_auto_save(false)
	, m_store(false)
	, m_array_recall(false)
	, m_size_data(size_data)
	, m_default_data(*this, DEVICE_SELF)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void x2212_device::device_start()
{
	m_sram = std::make_unique<u8[]>(m_size_data);
	m_e2prom = std::make_unique<u8[]>(m_size_data);
	std::fill_n(&m_sram[0], m_size_data, 0xff);

	save_item(NAME(m_store));
	save_item(NAME(m_array_recall));
	save_pointer(NAME(m_sram), m_size_data);
	save_pointer(NAME(m_e2prom), m_size_data);
}


//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void x2212_device::nvram_default()
{
	// default to all-0xff
	std::fill_n(&m_e2prom[0], m_size_data, 0xff);

	// populate from a memory region if present
	if (m_default_data.found())
		std::copy_n(&m_default_data[0], m_size_data, &m_e2prom[0]);
}


//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

bool x2212_device::nvram_read(util::read_stream &file)
{
	size_t actual;
	return !file.read(&m_e2prom[0], m_size_data, actual) && actual == m_size_data;
}


//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

bool x2212_device::nvram_write(util::write_stream &file)
{
	// auto-save causes an implicit store prior to exiting (writing)
	if (m_auto_save)
		do_store();

	size_t actual;
	return !file.write(&m_e2prom[0], m_size_data, actual) && actual == m_size_data;
}



//**************************************************************************
//  INTERNAL HELPERS
//**************************************************************************

//-------------------------------------------------
//  do_store - store data from live RAM into the
//  EEPROM
//-------------------------------------------------

void x2212_device::do_store()
{
	std::copy_n(&m_sram[0], m_size_data, &m_e2prom[0]);
}


//-------------------------------------------------
//  do_recall - fetch data from the EEPROM into live
//  RAM
//-------------------------------------------------

void x2212_device::do_recall()
{
	std::copy_n(&m_e2prom[0], m_size_data, &m_sram[0]);
}



//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

//-------------------------------------------------
//  write - store to the live RAM
//-------------------------------------------------

void x2212_device::write(offs_t offset, uint8_t data)
{
	assert(offset < m_size_data);
	m_sram[offset] = data & 0x0f;
}


//-------------------------------------------------
//  read - read from the live RAM
//-------------------------------------------------

u8 x2212_device::read(address_space &space, offs_t offset)
{
	assert(offset < m_size_data);
	return (m_sram[offset] & 0x0f) | (space.unmap() & 0xf0);
}


//-------------------------------------------------
//  store - set the state of the store line
//  (FIXME: actually active low, not active high)
//-------------------------------------------------

void x2212_device::store(int state)
{
	if (state != 0 && !m_store)
		do_store();
	m_store = (state != 0);
}


//-------------------------------------------------
//  recall - set the state of the recall line
//  (FIXME: actually active low, not active high)
//-------------------------------------------------

void x2212_device::recall(int state)
{
	if (state != 0 && !m_array_recall)
		do_recall();
	m_array_recall = (state != 0);
}


x2210_device::x2210_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: x2212_device(mconfig, X2210, tag, owner, clock, 0x40)
{
}
