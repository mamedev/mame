// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    er2055.c

    GI 512 bit electrically alterable read-only memory.

***************************************************************************/

#include "emu.h"
#include "machine/er2055.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type ER2055 = &device_creator<er2055_device>;

static ADDRESS_MAP_START( er2055_map, AS_PROGRAM, 8, er2055_device )
	AM_RANGE(0x0000, 0x003f) AM_RAM
ADDRESS_MAP_END



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  er2055_device - constructor
//-------------------------------------------------

er2055_device::er2055_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ER2055, "ER2055 EAROM", tag, owner, clock, "er2055", __FILE__),
		device_memory_interface(mconfig, *this),
		device_nvram_interface(mconfig, *this),
		m_space_config("EAROM", ENDIANNESS_BIG, 8, 6, 0, *ADDRESS_MAP_NAME(er2055_map)),
		m_control_state(0),
		m_address(0),
		m_data(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void er2055_device::device_start()
{
	save_item(NAME(m_control_state));
	save_item(NAME(m_address));
	save_item(NAME(m_data));

	m_control_state = 0;
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *er2055_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == 0) ? &m_space_config : NULL;
}


//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void er2055_device::nvram_default()
{
	// default to all-0xff
	for (int byte = 0; byte < SIZE_DATA; byte++)
		m_addrspace[0]->write_byte(byte, 0xff);

	// populate from a memory region if present
	if (m_region != NULL)
	{
		if (m_region->bytes() != SIZE_DATA)
			fatalerror("er2055 region '%s' wrong size (expected size = 0x40)\n", tag());
		if (m_region->bytewidth() != 1)
			fatalerror("er2055 region '%s' needs to be an 8-bit region\n", tag());

		UINT8 *default_data = m_region->base();
		for (int byte = 0; byte < SIZE_DATA; byte++)
			m_addrspace[0]->write_byte(byte, default_data[byte]);
	}
}


//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

void er2055_device::nvram_read(emu_file &file)
{
	UINT8 buffer[SIZE_DATA];
	file.read(buffer, sizeof(buffer));
	for (int byte = 0; byte < SIZE_DATA; byte++)
		m_addrspace[0]->write_byte(byte, buffer[byte]);
}


//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

void er2055_device::nvram_write(emu_file &file)
{
	UINT8 buffer[SIZE_DATA];
	for (int byte = 0; byte < SIZE_DATA; byte++)
		buffer[byte] = m_addrspace[0]->read_byte(byte);
	file.write(buffer, sizeof(buffer));
}



//**************************************************************************
//  I/O OPERATIONS
//**************************************************************************

//-------------------------------------------------
//  set_control - set the control lines; these
//  must be done simultaneously because the chip
//  reacts to various combinations
//-------------------------------------------------

void er2055_device::set_control(UINT8 cs1, UINT8 cs2, UINT8 c1, UINT8 c2, UINT8 ck)
{
	// create a new composite control state
	UINT8 oldstate = m_control_state;
	m_control_state = (ck != 0) ? CK : 0;
	m_control_state |= (c1 != 0) ? C1 : 0;
	m_control_state |= (c2 != 0) ? C2 : 0;
	m_control_state |= (cs1 != 0) ? CS1 : 0;
	m_control_state |= (cs2 != 0) ? CS2 : 0;

	// if not selected, or if change from previous, we're done
	if ((m_control_state & (CS1 | CS2)) != (CS1 | CS2) || m_control_state == oldstate)
		return;

	// something changed, see what it is based on what mode we're in
	switch (m_control_state & (C1 | C2))
	{
		// write mode; erasing is required, so we perform an AND against previous
		// data to simulate incorrect behavior if erasing was not done
		case 0:
			m_addrspace[0]->write_byte(m_address, m_addrspace[0]->read_byte(m_address) & m_data);
//printf("Write %02X = %02X\n", m_address, m_data);
			break;

		// erase mode
		case C2:
			m_addrspace[0]->write_byte(m_address, 0xff);
//printf("Erase %02X\n", m_address);
			break;

		// read mode
		case C1:
			if ((oldstate & CK) != 0 && (m_control_state & CK) == 0)
			{
				m_data = m_addrspace[0]->read_byte(m_address);
//printf("Read %02X = %02X\n", m_address, m_data);
			}
			break;
	}
}
