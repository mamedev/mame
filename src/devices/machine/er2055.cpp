// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    er2055.c

    GI 64 word x 8 bit electrically alterable read-only memory.

    Atari often called this part "137161-001" on their technical manuals,
    but their schematics usually called it by its proper ER-2055 name.

***************************************************************************/

#include "emu.h"
#include "machine/er2055.h"

#include "logmacro.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(ER2055, er2055_device, "er2055", "ER2055 EAROM (64x8)")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  er2055_device - constructor
//-------------------------------------------------

er2055_device::er2055_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ER2055, tag, owner, clock),
		device_nvram_interface(mconfig, *this),
		m_default_data(*this, DEVICE_SELF),
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

	m_rom_data = std::make_unique<uint8_t[]>(SIZE_DATA);
	save_pointer(NAME(m_rom_data), SIZE_DATA);

	m_control_state = 0;
}


//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void er2055_device::nvram_default()
{
	// default to all-0xff
	std::fill_n(&m_rom_data[0], SIZE_DATA, 0xff);

	// populate from a memory region if present
	if (m_default_data.found())
		std::copy_n(&m_default_data[0], SIZE_DATA, &m_rom_data[0]);
}


//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

bool er2055_device::nvram_read(util::read_stream &file)
{
	size_t actual;
	return !file.read(&m_rom_data[0], SIZE_DATA, actual) && actual == SIZE_DATA;
}


//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

bool er2055_device::nvram_write(util::write_stream &file)
{
	size_t actual;
	return !file.write(&m_rom_data[0], SIZE_DATA, actual) && actual == SIZE_DATA;
}



//**************************************************************************
//  I/O OPERATIONS
//**************************************************************************

//-------------------------------------------------
//  set_control - set the control lines; these
//  must be done simultaneously because the chip
//  reacts to various combinations
//-------------------------------------------------

void er2055_device::set_control(uint8_t cs1, uint8_t cs2, uint8_t c1, uint8_t c2)
{
	// create a new composite control state
	uint8_t oldstate = m_control_state;
	m_control_state = oldstate & CK;
	m_control_state |= (c1 != 0) ? C1 : 0;
	m_control_state |= (c2 != 0) ? C2 : 0;
	m_control_state |= (cs1 != 0) ? CS1 : 0;
	m_control_state |= (cs2 != 0) ? CS2 : 0;

	// if not selected, or if change from previous, we're done
	if ((m_control_state & (CS1 | CS2)) != (CS1 | CS2) || m_control_state == oldstate)
		return;

	update_state();
}


//-------------------------------------------------
//  update_state - update internal state following
//  a transition on clock, control or chip select
//  lines based on what mode we're in
//-------------------------------------------------

void er2055_device::update_state()
{
	switch (m_control_state & (C1 | C2))
	{
		// write mode; erasing is required, so we perform an AND against previous
		// data to simulate incorrect behavior if erasing was not done
		case 0:
			m_rom_data[m_address] &= m_data;
			LOG("Write %02X = %02X\n", m_address, m_data);
			break;

		// erase mode
		case C2:
			m_rom_data[m_address] = 0xff;
			LOG("Erase %02X\n", m_address);
			break;
	}
}


//-------------------------------------------------
//  set_clk - set the CLK line, pulses on which
//  are required for every read operation and for
//  successive write or erase operations
//-------------------------------------------------

WRITE_LINE_MEMBER(er2055_device::set_clk)
{
	uint8_t oldstate = m_control_state;
	if (state)
		m_control_state |= CK;
	else
		m_control_state &= ~CK;

	// updates occur on falling edge when chip is selected
	if ((m_control_state & (CS1 | CS2)) == (CS1 | CS2) && (m_control_state != oldstate) && !state)
	{
		// read mode (C2 is "Don't Care")
		if ((m_control_state & C1) == C1)
		{
			m_data = m_rom_data[m_address];
			LOG("Read %02X = %02X\n", m_address, m_data);
		}

		update_state();
	}
}
