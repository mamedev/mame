/***************************************************************************

    x2212.c

    Xicor X2212 256 x 4 bit Nonvolatile Static RAM.

***************************************************************************/

#include "emu.h"
#include "machine/x2212.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type X2212 = x2212_device_config::static_alloc_device_config;

static ADDRESS_MAP_START( x2212_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x00ff) AM_RAM
ADDRESS_MAP_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  x2212_device_config - constructor
//-------------------------------------------------

x2212_device_config::x2212_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
	: device_config(mconfig, static_alloc_device_config, "X2212", tag, owner, clock),
	  device_config_memory_interface(mconfig, *this),
	  device_config_nvram_interface(mconfig, *this),
	  m_space_config("SRAM", ENDIANNESS_BIG, 8, 8, 0, *ADDRESS_MAP_NAME(x2212_map)),
	  m_auto_save(false)
{
}


//-------------------------------------------------
//  static_alloc_device_config - allocate a new
//  configuration object
//-------------------------------------------------

device_config *x2212_device_config::static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
{
	return global_alloc(x2212_device_config(mconfig, tag, owner, clock));
}


//-------------------------------------------------
//  alloc_device - allocate a new device object
//-------------------------------------------------

device_t *x2212_device_config::alloc_device(running_machine &machine) const
{
	return auto_alloc(&machine, x2212_device(machine, *this));
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *x2212_device_config::memory_space_config(int spacenum) const
{
	return (spacenum == 0) ? &m_space_config : NULL;
}


//-------------------------------------------------
//  static_set_auto_save - configuration helper
//  to set the auto-save flag
//-------------------------------------------------

void x2212_device_config::static_set_auto_save(device_config *device)
{
	downcast<x2212_device_config *>(device)->m_auto_save = true;
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  x2212_device - constructor
//-------------------------------------------------

x2212_device::x2212_device(running_machine &_machine, const x2212_device_config &config)
	: device_t(_machine, config),
	  device_memory_interface(_machine, config, *this),
	  device_nvram_interface(_machine, config, *this),
	  m_config(config),
	  m_store(true),
	  m_array_recall(true)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void x2212_device::device_start()
{
	state_save_register_device_item_array(this, 0, m_e2prom);
	state_save_register_device_item(this, 0, m_store);
	state_save_register_device_item(this, 0, m_array_recall);
}


//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void x2212_device::nvram_default()
{
	// default to all-0xff
	memset(m_e2prom, 0xff, SIZE_DATA);
	for (int byte = 0; byte < SIZE_DATA; byte++)
		m_addrspace[0]->write_byte(byte, 0xff);

	// populate from a memory region if present
	if (m_region != NULL)
	{
		if (m_region->bytes() != SIZE_DATA)
			fatalerror("x2212 region '%s' wrong size (expected size = 0x100)", tag());
		if (m_region->width() != 1)
			fatalerror("x2212 region '%s' needs to be an 8-bit region", tag());

		memcpy(m_e2prom, *m_region, SIZE_DATA);
	}
}


//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

void x2212_device::nvram_read(mame_file &file)
{
	mame_fread(&file, m_e2prom, SIZE_DATA);
}


//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

void x2212_device::nvram_write(mame_file &file)
{
	// auto-save causes an implicit store prior to exiting (writing)
	if (m_config.m_auto_save)
		store();
	mame_fwrite(&file, m_e2prom, SIZE_DATA);
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
		m_e2prom[byte] = m_addrspace[0]->read_byte(byte);
}


//-------------------------------------------------
//  recall - fetch data from the EEPROM into live 
//  RAM
//-------------------------------------------------

void x2212_device::recall()
{
	for (int byte = 0; byte < SIZE_DATA; byte++)
		m_addrspace[0]->write_byte(byte, m_e2prom[byte]);
}



//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

//-------------------------------------------------
//  write - store to the live RAM
//-------------------------------------------------

WRITE8_MEMBER( x2212_device::write )
{
	m_addrspace[0]->write_byte(offset, data & 0x0f);
}


//-------------------------------------------------
//  read - read from the live RAM
//-------------------------------------------------

READ8_MEMBER( x2212_device::read )
{
	return m_addrspace[0]->read_byte(offset) & 0x0f;
}


//-------------------------------------------------
//  store - set the state of the store line
//  (active high)
//-------------------------------------------------

WRITE_LINE_MEMBER( x2212_device::store )
{
	state &= 1;
	if (!state && m_store)
		store();
	m_store = state;
}


//-------------------------------------------------
//  recall - set the state of the recall line
//  (active high)
//-------------------------------------------------

WRITE_LINE_MEMBER( x2212_device::recall )
{
	state &= 1;
	if (!state && m_array_recall)
		recall();
	m_array_recall = state;
}
