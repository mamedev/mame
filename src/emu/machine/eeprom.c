// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    eeprom.c

    Base class for EEPROM devices.

***************************************************************************/

#include "emu.h"
#include "machine/eeprom.h"



//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

static ADDRESS_MAP_START( eeprom_map8, AS_PROGRAM, 8, eeprom_base_device )
	AM_RANGE(0x00000, 0xfffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( eeprom_map16, AS_PROGRAM, 16, eeprom_base_device )
	AM_RANGE(0x00000, 0x7ffff) AM_RAM
ADDRESS_MAP_END



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  eeprom_base_device - constructor
//-------------------------------------------------

eeprom_base_device::eeprom_base_device(const machine_config &mconfig, device_type devtype, const char *name, const char *tag, device_t *owner, const char *shortname, const char *file)
	: device_t(mconfig, devtype, name, tag, owner, 0, shortname, file),
		device_memory_interface(mconfig, *this),
		device_nvram_interface(mconfig, *this),
		m_cells(0),
		m_address_bits(0),
		m_data_bits(0),
		m_default_data(0),
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
//  static_set_default_data - configuration helpers
//  to set the default data
//-------------------------------------------------

void eeprom_base_device::static_set_size(device_t &device, int cells, int cellbits)
{
	eeprom_base_device &eeprom = downcast<eeprom_base_device &>(device);
	eeprom.m_cells = cells;
	eeprom.m_data_bits = cellbits;

	// compute address bits (validation checks verify cells was an even power of 2)
	cells--;
	eeprom.m_address_bits = 0;
	while (cells != 0)
	{
		cells >>= 1;
		eeprom.m_address_bits++;
	}

	// describe our address space
	if (eeprom.m_data_bits == 8)
		eeprom.m_space_config = address_space_config("eeprom", ENDIANNESS_BIG, 8,  eeprom.m_address_bits, 0, *ADDRESS_MAP_NAME(eeprom_map8));
	else
		eeprom.m_space_config = address_space_config("eeprom", ENDIANNESS_BIG, 16, eeprom.m_address_bits * 2, 0, *ADDRESS_MAP_NAME(eeprom_map16));
}


//-------------------------------------------------
//  static_set_default_data - configuration helpers
//  to set the default data
//-------------------------------------------------

void eeprom_base_device::static_set_default_data(device_t &device, const UINT8 *data, UINT32 size)
{
	eeprom_base_device &eeprom = downcast<eeprom_base_device &>(device);
	assert(eeprom.m_data_bits == 8);
	eeprom.m_default_data.u8 = const_cast<UINT8 *>(data);
	eeprom.m_default_data_size = size;
}

void eeprom_base_device::static_set_default_data(device_t &device, const UINT16 *data, UINT32 size)
{
	eeprom_base_device &eeprom = downcast<eeprom_base_device &>(device);
	assert(eeprom.m_data_bits == 16);
	eeprom.m_default_data.u16 = const_cast<UINT16 *>(data);
	eeprom.m_default_data_size = size / 2;
}


//-------------------------------------------------
//  static_set_default_value - configuration helper
//  to set the default value
//-------------------------------------------------

void eeprom_base_device::static_set_default_value(device_t &device, UINT32 value)
{
	eeprom_base_device &eeprom = downcast<eeprom_base_device &>(device);
	eeprom.m_default_value = value;
	eeprom.m_default_value_set = true;
}


//-------------------------------------------------
//  static_set_timing - configuration helper
//  to set timing constants for various operations
//-------------------------------------------------

void eeprom_base_device::static_set_timing(device_t &device, timing_type type, const attotime &duration)
{
	downcast<eeprom_base_device &>(device).m_operation_time[type] = duration;
}


//-------------------------------------------------
//  read - read data at the given address
//-------------------------------------------------

UINT32 eeprom_base_device::read(offs_t address)
{
	if (!ready())
		logerror("EEPROM: Read performed before previous operation completed!");
	return internal_read(address);
}


//-------------------------------------------------
//  write - write data at the given address
//-------------------------------------------------

void eeprom_base_device::write(offs_t address, UINT32 data)
{
	if (!ready())
		logerror("EEPROM: Write performed before previous operation completed!");
	internal_write(address, data);
	m_completion_time = machine().time() + m_operation_time[WRITE_TIME];
}


//-------------------------------------------------
//  write_all - write data at all addresses
//  (assumes an erase has previously been
//  performed)
//-------------------------------------------------

void eeprom_base_device::write_all(UINT32 data)
{
	if (!ready())
		logerror("EEPROM: Write all performed before previous operation completed!");
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
		logerror("EEPROM: Erase performed before previous operation completed!");
	internal_write(address, ~0);
	m_completion_time = machine().time() + m_operation_time[ERASE_TIME];
}


//-------------------------------------------------
//  erase_all - erase data at all addresses
//-------------------------------------------------

void eeprom_base_device::erase_all()
{
	if (!ready())
		logerror("EEPROM: Erase all performed before previous operation completed!");
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
	// save states
	save_item(NAME(m_completion_time));
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
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *eeprom_base_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == 0) ? &m_space_config : NULL;
}


//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void eeprom_base_device::nvram_default()
{
	UINT32 eeprom_length = 1 << m_address_bits;
	UINT32 eeprom_bytes = eeprom_length * m_data_bits / 8;

	// initialize to the default value
	UINT32 default_value = m_default_value_set ? m_default_value : ~0;
	for (offs_t offs = 0; offs < eeprom_length; offs++)
		if (m_data_bits == 8)
			m_addrspace[0]->write_byte(offs, default_value);
		else
			m_addrspace[0]->write_word(offs * 2, default_value);

	// handle hard-coded data from the driver
	if (m_default_data.u8 != NULL)
	{
		osd_printf_verbose("Warning: Driver-specific EEPROM defaults are going away soon.\n");
		for (offs_t offs = 0; offs < m_default_data_size; offs++)
		{
			if (m_data_bits == 8)
				m_addrspace[0]->write_byte(offs, m_default_data.u8[offs]);
			else
				m_addrspace[0]->write_word(offs * 2, m_default_data.u16[offs]);
		}
	}

	// populate from a memory region if present
	if (m_region != NULL)
	{
		if (m_region->bytes() != eeprom_bytes)
			fatalerror("eeprom region '%s' wrong size (expected size = 0x%X)\n", tag(), eeprom_bytes);
		if (m_data_bits == 8 && m_region->bytewidth() != 1)
			fatalerror("eeprom region '%s' needs to be an 8-bit region\n", tag());
		if (m_data_bits == 16 && (m_region->bytewidth() != 2 || m_region->endianness() != ENDIANNESS_BIG))
			fatalerror("eeprom region '%s' needs to be a 16-bit big-endian region\n", tag());
		osd_printf_verbose("Loading data from EEPROM region '%s'\n", tag());

		if (m_data_bits == 8)
		{
			UINT8 *default_data = m_region->base();
			for (offs_t offs = 0; offs < eeprom_length; offs++)
				m_addrspace[0]->write_byte(offs, default_data[offs]);
		}
		else
		{
			UINT16 *default_data = (UINT16 *)(m_region->base());
			for (offs_t offs = 0; offs < eeprom_length; offs++)
				m_addrspace[0]->write_word(offs * 2, default_data[offs]);
		}
	}
}


//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

void eeprom_base_device::nvram_read(emu_file &file)
{
	UINT32 eeprom_length = 1 << m_address_bits;
	UINT32 eeprom_bytes = eeprom_length * m_data_bits / 8;

	dynamic_buffer buffer(eeprom_bytes);
	file.read(&buffer[0], eeprom_bytes);
	for (offs_t offs = 0; offs < eeprom_bytes; offs++)
		m_addrspace[0]->write_byte(offs, buffer[offs]);
}


//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

void eeprom_base_device::nvram_write(emu_file &file)
{
	UINT32 eeprom_length = 1 << m_address_bits;
	UINT32 eeprom_bytes = eeprom_length * m_data_bits / 8;

	dynamic_buffer buffer(eeprom_bytes);
	for (offs_t offs = 0; offs < eeprom_bytes; offs++)
		buffer[offs] = m_addrspace[0]->read_byte(offs);
	file.write(&buffer[0], eeprom_bytes);
}


//-------------------------------------------------
//  internal_read - read data at the given address
//-------------------------------------------------

UINT32 eeprom_base_device::internal_read(offs_t address)
{
	if (m_data_bits == 16)
		return m_addrspace[0]->read_word(address * 2);
	else
		return m_addrspace[0]->read_byte(address);
}


//-------------------------------------------------
//  internal_write - write data at the given
//  address
//-------------------------------------------------

void eeprom_base_device::internal_write(offs_t address, UINT32 data)
{
	if (m_data_bits == 16)
		m_addrspace[0]->write_word(address * 2, data);
	else
		m_addrspace[0]->write_byte(address, data);
}
