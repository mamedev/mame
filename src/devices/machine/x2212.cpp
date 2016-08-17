// license:BSD-3-Clause
// copyright-holders:smf,Barry Rodewald
/***************************************************************************

    x2212.c

    Xicor X2212 256 x 4 bit Nonvolatile Static RAM.

***************************************************************************/

#include "emu.h"
#include "machine/x2212.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

static ADDRESS_MAP_START( x2212_sram_map, AS_0, 8, x2212_device )
	AM_RANGE(0x0000, 0x00ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( x2212_e2prom_map, AS_1, 8, x2212_device )
	AM_RANGE(0x0000, 0x00ff) AM_RAM
ADDRESS_MAP_END



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
const device_type X2212 = &device_creator<x2212_device>;
const device_type X2210 = &device_creator<x2210_device>;

//-------------------------------------------------
//  x2212_device - constructor
//-------------------------------------------------

x2212_device::x2212_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, X2212, "X2212 NVRAM", tag, owner, clock, "x2212", __FILE__),
		device_memory_interface(mconfig, *this),
		device_nvram_interface(mconfig, *this),
		m_auto_save(false),
		m_sram_space_config("SRAM", ENDIANNESS_BIG, 8, 8, 0, *ADDRESS_MAP_NAME(x2212_sram_map)),
		m_e2prom_space_config("E2PROM", ENDIANNESS_BIG, 8, 8, 0, *ADDRESS_MAP_NAME(x2212_e2prom_map)),
		m_store(false),
		m_array_recall(false),
		m_size_data(0x100),
		m_default_data(*this, DEVICE_SELF, 0x100)
{
}

x2212_device::x2212_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source, int size_data)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_memory_interface(mconfig, *this),
		device_nvram_interface(mconfig, *this),
		m_auto_save(false),
		m_sram_space_config("SRAM", ENDIANNESS_BIG, 8, 8, 0, *ADDRESS_MAP_NAME(x2212_sram_map)),
		m_e2prom_space_config("E2PROM", ENDIANNESS_BIG, 8, 8, 0, *ADDRESS_MAP_NAME(x2212_e2prom_map)),
		m_store(false),
		m_array_recall(false),
		m_size_data(size_data),
		m_default_data(*this, DEVICE_SELF, size_data)
{
}

//-------------------------------------------------
//  static_set_auto_save - configuration helper
//  to set the auto-save flag
//-------------------------------------------------

void x2212_device::static_set_auto_save(device_t &device)
{
	downcast<x2212_device &>(device).m_auto_save = true;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void x2212_device::device_start()
{
	save_item(NAME(m_store));
	save_item(NAME(m_array_recall));
	m_sram = &space(AS_0);
	m_e2prom = &space(AS_1);
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *x2212_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == 0) ? &m_sram_space_config : (spacenum == 1) ? &m_e2prom_space_config : nullptr;
}


//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void x2212_device::nvram_default()
{
	// default to all-0xff
	for (int byte = 0; byte < m_size_data; byte++)
	{
		m_sram->write_byte(byte, 0xff);
		m_e2prom->write_byte(byte, 0xff);
	}

	// populate from a memory region if present
	if (m_default_data.found())
	{
		for (int byte = 0; byte < m_size_data; byte++)
			m_e2prom->write_byte(byte, m_default_data[byte]);
	}
}


//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

void x2212_device::nvram_read(emu_file &file)
{
	UINT8 *buffer = (UINT8 *) alloca(m_size_data);
	file.read(buffer, m_size_data);
	for (int byte = 0; byte < m_size_data; byte++)
	{
		m_sram->write_byte(byte, 0xff);
		m_e2prom->write_byte(byte, buffer[byte]);
	}
}


//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

void x2212_device::nvram_write(emu_file &file)
{
	// auto-save causes an implicit store prior to exiting (writing)
	if (m_auto_save)
		store();

	UINT8 *buffer = (UINT8 *) alloca(m_size_data);
	for (int byte = 0; byte < m_size_data; byte++)
		buffer[byte] = m_e2prom->read_byte(byte);
	file.write(buffer, m_size_data);
}



//**************************************************************************
//  INTERNAL HELPERS
//**************************************************************************

//-------------------------------------------------
//  store - store data from live RAM into the
//  EEPROM
//-------------------------------------------------

void x2212_device::store()
{
	for (int byte = 0; byte < m_size_data; byte++)
		m_e2prom->write_byte(byte, m_sram->read_byte(byte));
}


//-------------------------------------------------
//  recall - fetch data from the EEPROM into live
//  RAM
//-------------------------------------------------

void x2212_device::recall()
{
	for (int byte = 0; byte < m_size_data; byte++)
		m_sram->write_byte(byte, m_e2prom->read_byte(byte));
}



//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

//-------------------------------------------------
//  write - store to the live RAM
//-------------------------------------------------

WRITE8_MEMBER( x2212_device::write )
{
	m_sram->write_byte(offset, data & 0x0f);
}


//-------------------------------------------------
//  read - read from the live RAM
//-------------------------------------------------

READ8_MEMBER( x2212_device::read )
{
	return (m_sram->read_byte(offset) & 0x0f) | (space.unmap() & 0xf0);
}


//-------------------------------------------------
//  store - set the state of the store line
//  (active high)
//-------------------------------------------------

WRITE_LINE_MEMBER( x2212_device::store )
{
	if (state != 0 && !m_store)
		store();
	m_store = (state != 0);
}


//-------------------------------------------------
//  recall - set the state of the recall line
//  (active high)
//-------------------------------------------------

WRITE_LINE_MEMBER( x2212_device::recall )
{
	if (state != 0 && !m_array_recall)
		recall();
	m_array_recall = (state != 0);
}


x2210_device::x2210_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: x2212_device(mconfig, X2210, "X2210", tag, owner, clock, "x2210", __FILE__, 0x40)
{
}
