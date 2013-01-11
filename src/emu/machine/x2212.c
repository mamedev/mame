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

//-------------------------------------------------
//  x2212_device - constructor
//-------------------------------------------------

x2212_device::x2212_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, X2212, "X2212", tag, owner, clock),
		device_memory_interface(mconfig, *this),
		device_nvram_interface(mconfig, *this),
		m_auto_save(false),
		m_sram_space_config("SRAM", ENDIANNESS_BIG, 8, 8, 0, *ADDRESS_MAP_NAME(x2212_sram_map)),
		m_e2prom_space_config("E2PROM", ENDIANNESS_BIG, 8, 8, 0, *ADDRESS_MAP_NAME(x2212_e2prom_map)),
		m_store(false),
		m_array_recall(false)
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

	m_sram = m_addrspace[0];
	m_e2prom = m_addrspace[1];
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *x2212_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == 0) ? &m_sram_space_config : (spacenum == 1) ? &m_e2prom_space_config : NULL;
}


//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void x2212_device::nvram_default()
{
	// default to all-0xff
	for (int byte = 0; byte < SIZE_DATA; byte++)
	{
		m_sram->write_byte(byte, 0xff);
		m_e2prom->write_byte(byte, 0xff);
	}

	// populate from a memory region if present
	if (m_region != NULL)
	{
		if (m_region->bytes() != SIZE_DATA)
			fatalerror("x2212 region '%s' wrong size (expected size = 0x100)\n", tag());
		if (m_region->width() != 1)
			fatalerror("x2212 region '%s' needs to be an 8-bit region\n", tag());

		for (int byte = 0; byte < SIZE_DATA; byte++)
			m_e2prom->write_byte(byte, m_region->u8(byte));
	}
}


//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

void x2212_device::nvram_read(emu_file &file)
{
	UINT8 buffer[SIZE_DATA];
	file.read(buffer, sizeof(buffer));
	for (int byte = 0; byte < SIZE_DATA; byte++)
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

	UINT8 buffer[SIZE_DATA];
	for (int byte = 0; byte < SIZE_DATA; byte++)
		buffer[byte] = m_e2prom->read_byte(byte);
	file.write(buffer, sizeof(buffer));
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
	for (int byte = 0; byte < SIZE_DATA; byte++)
		m_e2prom->write_byte(byte, m_sram->read_byte(byte));
}


//-------------------------------------------------
//  recall - fetch data from the EEPROM into live
//  RAM
//-------------------------------------------------

void x2212_device::recall()
{
	for (int byte = 0; byte < SIZE_DATA; byte++)
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
