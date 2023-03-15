// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    eeprom.c

    Base class for EEPROM devices.

***************************************************************************/

#include "emu.h"
#include "machine/eeprom.h"

//#define VERBOSE 1
#include "logmacro.h"


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  eeprom_base_device - constructor
//-------------------------------------------------

eeprom_base_device::eeprom_base_device(const machine_config &mconfig, device_type devtype, const char *tag, device_t *owner)
	: device_t(mconfig, devtype, tag, owner, 0),
		device_nvram_interface(mconfig, *this),
		m_region(*this, DEVICE_SELF),
		m_cells(0),
		m_address_bits(0),
		m_data_bits(0),
		m_default_data(nullptr),
		m_default_data_size(0),
		m_default_value(0),
		m_default_value_set(false),
		m_completion_time(attotime::zero)
{
	// a 2ms write time is too long for rfjetsa
	m_operation_time[WRITE_TIME]        = attotime::from_usec(1750);
	m_operation_time[WRITE_ALL_TIME]    = attotime::from_usec(8000);
	m_operation_time[ERASE_TIME]        = attotime::from_usec(1000);
	m_operation_time[ERASE_ALL_TIME]    = attotime::from_usec(8000);
}


//-------------------------------------------------
//  set_default_data - configuration helpers
//  to set the default data
//-------------------------------------------------

eeprom_base_device& eeprom_base_device::size(int cells, int cellbits)
{
	m_cells = cells;
	m_data_bits = cellbits;

	// compute address bits (validation checks verify cells was an even power of 2)
	cells--;
	m_address_bits = 0;
	while (cells != 0)
	{
		cells >>= 1;
		m_address_bits++;
	}

	return *this;
}


//-------------------------------------------------
//  set_default_data - configuration helpers
//  to set the default data
//-------------------------------------------------

eeprom_base_device& eeprom_base_device::default_data(const uint8_t *data, uint32_t size)
{
	assert(m_data_bits == 8);
	m_default_data = data;
	m_default_data_size = size;
	return *this;
}

eeprom_base_device& eeprom_base_device::default_data(const uint16_t *data, uint32_t size)
{
	assert(m_data_bits == 16);
	m_default_data = data;
	m_default_data_size = size / 2;
	return *this;
}


//-------------------------------------------------
//  read - read data at the given address
//-------------------------------------------------

uint32_t eeprom_base_device::read(offs_t address)
{
	if (!ready())
		logerror("EEPROM: Read performed before previous operation completed!\n");
	return internal_read(address);
}


//-------------------------------------------------
//  write - write data at the given address
//-------------------------------------------------

void eeprom_base_device::write(offs_t address, uint32_t data)
{
	if (!ready())
		logerror("EEPROM: Write performed before previous operation completed!\n");
	internal_write(address, data);
	m_completion_time = machine().time() + m_operation_time[WRITE_TIME];
}


//-------------------------------------------------
//  write_all - write data at all addresses
//  (assumes an erase has previously been
//  performed)
//-------------------------------------------------

void eeprom_base_device::write_all(uint32_t data)
{
	if (!ready())
		logerror("EEPROM: Write all performed before previous operation completed!\n");
	for (offs_t address = 0; address < (1 << m_address_bits); address++)
		internal_write(address, internal_read(address) & data);
	m_completion_time = machine().time() + m_operation_time[WRITE_ALL_TIME];
}


//-------------------------------------------------
//  erase - erase data at the given address
//-------------------------------------------------

void eeprom_base_device::erase(offs_t address)
{
	if (!ready())
		logerror("EEPROM: Erase performed before previous operation completed!\n");
	internal_write(address, ~0);
	m_completion_time = machine().time() + m_operation_time[ERASE_TIME];
}


//-------------------------------------------------
//  erase_all - erase data at all addresses
//-------------------------------------------------

void eeprom_base_device::erase_all()
{
	if (!ready())
		logerror("EEPROM: Erase all performed before previous operation completed!\n");
	for (offs_t address = 0; address < (1 << m_address_bits); address++)
		internal_write(address, ~0);
	m_completion_time = machine().time() + m_operation_time[ERASE_ALL_TIME];
}


//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

void eeprom_base_device::device_validity_check(validity_checker &valid) const
{
	// ensure the number of cells is an even power of 2
	if (m_cells != (1 << m_address_bits))
		osd_printf_error("Invalid EEPROM size %d specified\n", m_cells);

	// ensure only the sizes we support are requested
	if (m_data_bits != 8 && m_data_bits != 16)
		osd_printf_error("Invalid EEPROM data width %d specified\n", m_data_bits);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void eeprom_base_device::device_start()
{
	uint32_t size = (m_data_bits == 8 ? 1 : 2) << m_address_bits;
	m_data = std::make_unique<uint8_t []>(size);

	// save states
	save_item(NAME(m_completion_time));
	save_pointer(NAME(m_data), size);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void eeprom_base_device::device_reset()
{
	// reset any pending operations
	m_completion_time = attotime::zero;
}


//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void eeprom_base_device::nvram_default()
{
	uint32_t eeprom_length = 1 << m_address_bits;
	uint32_t eeprom_bytes = eeprom_length * m_data_bits / 8;

	// initialize to the default value
	uint32_t default_value = m_default_value_set ? m_default_value : ~0;
	for (offs_t offs = 0; offs < eeprom_length; offs++)
		internal_write(offs, default_value);

	// handle hard-coded data from the driver
	if (m_default_data != nullptr)
	{
		osd_printf_verbose("Warning: Driver-specific EEPROM defaults are going away soon.\n");
		for (offs_t offs = 0; offs < m_default_data_size; offs++)
		{
			if (m_data_bits == 8)
				internal_write(offs, static_cast<const u8 *>(m_default_data)[offs]);
			else
				internal_write(offs, static_cast<const u16 *>(m_default_data)[offs]);
		}
	}

	// populate from a memory region if present
	if (m_region.found())
	{
		if (m_region->bytes() != eeprom_bytes)
			fatalerror("eeprom region '%s' wrong size (expected size = 0x%X)\n", tag(), eeprom_bytes);
		if (m_data_bits == 8 && m_region->bytewidth() != 1)
			fatalerror("eeprom region '%s' needs to be an 8-bit region\n", tag());
		if (m_data_bits == 16 && m_region->bytewidth() != 2)
			fatalerror("eeprom region '%s' needs to be a 16-bit region\n", tag());
		osd_printf_verbose("Loading data from EEPROM region '%s'\n", tag());

		memcpy(&m_data[0], m_region->base(), eeprom_bytes);
	}
}


//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

bool eeprom_base_device::nvram_read(util::read_stream &file)
{
	uint32_t eeprom_length = 1 << m_address_bits;
	uint32_t eeprom_bytes = eeprom_length * m_data_bits / 8;
	size_t actual_bytes;

	return !file.read(&m_data[0], eeprom_bytes, actual_bytes) && actual_bytes == eeprom_bytes;
}


//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

bool eeprom_base_device::nvram_write(util::write_stream &file)
{
	uint32_t eeprom_length = 1 << m_address_bits;
	uint32_t eeprom_bytes = eeprom_length * m_data_bits / 8;
	size_t actual_bytes;

	return !file.write(&m_data[0], eeprom_bytes, actual_bytes) && actual_bytes == eeprom_bytes;
}


//-------------------------------------------------
//  internal_read - read data at the given address
//-------------------------------------------------

uint32_t eeprom_base_device::internal_read(offs_t address)
{
	if (m_data_bits == 16)
		return m_data[address * 2] | (m_data[address * 2 + 1] << 8);
	else
		return m_data[address];
}


//-------------------------------------------------
//  internal_write - write data at the given
//  address
//-------------------------------------------------

void eeprom_base_device::internal_write(offs_t address, uint32_t data)
{
	if (m_data_bits == 16)
	{
		m_data[address * 2] = data;
		m_data[address * 2 + 1] = data >> 8;
	} else
		m_data[address] = data;
}
