// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Jonathan Gevaryahu
/***************************************************************************

    General Instruments ER-2055 EAROM
    64 word x 8 bit Electrically Alterable Read Only Memory

    Atari often called this part "137161-001" on their technical manuals,
    but their schematics usually called it by its proper ER-2055 name.

    Centipede, Millipede, Dig Dug CTRL port wiring:
    7 6 5 4 3 2 1 0
    x x x x | | | \-- EAROM CLK (directly connected)
    x x x x | | \---- /EAROM C1 (note this is inverted! the value written here is inverted, then connected to EAROM C1 AND to the /OE of the 'input latch' which drive the EAROM bus when writing)
    x x x x | \------ EAROM C2 (directly connected)
    x x x x \-------- EAROM CS1 (positive enable, directly connected)

    Gravitar, Red Baron, Black Widow, Tempest, Liberator, Asteroids Deluxe, Runaway CTRL port wiring:
    7 6 5 4 3 2 1 0
    x x x x | | | \-- EAROM CLK (directly connected)
    x x x x | | \---- EAROM C2 (directly connected)
    x x x x | \------ /EAROM C1 (note this is inverted! the value written here is inverted, then connected to EAROM C1 AND to the /OE of the 'input latch' which drive the EAROM bus when writing)
    x x x x \-------- EAROM CS1 (positive enable, directly connected)

***************************************************************************/

#include "emu.h"
#include "atari_vg.h"

#define ER2055_IDLE (0)
#define ER2055_READ_WAITING_FOR_CLOCK (1)
#define ER2055_READ_DRIVING_BUS (2)
#define ER2055_WRITING_BITS (3)
#define ER2055_ERASING_BITS (4)


// device type definition
const device_type ATARIVGEAROM = &device_creator<atari_vg_earom_device>;

//-------------------------------------------------
//  atari_vg_earom_device - constructor
//-------------------------------------------------

atari_vg_earom_device::atari_vg_earom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ATARIVGEAROM, "Atari VG EAROM", tag, owner, clock, "atari_vg_earom", __FILE__),
		device_nvram_interface(mconfig, *this)
{
}

// Activate the output buffer (LS244)'s /G1 and /G2 lines to read whatever is on the EAROM's output bus
READ8_MEMBER( atari_vg_earom_device::read )
{
	logerror("read from earom output: %02x(%02x):%02x\n", m_in_offset, offset, m_out_data);
	return (m_out_data);
}

// Write to the address input (LS174) and data input (LS374) latches respectively
WRITE8_MEMBER( atari_vg_earom_device::write )
{
	logerror("write to earom buffers: offset:data of %02x:%02x\n", offset, data);
	m_in_offset = offset;
	m_in_data = data;
}

/* CTRL controls the CTRL latch (LS175):
See top of file: there are two possible wirings!
 */
WRITE8_MEMBER( atari_vg_earom_device::ctrl_w )
{
	static const char *const er2055_State[5] = { "IDLE", "RD_WAIT", "RD_OUTPUT", "WRITE", "ERASE" };
	logerror("EAROM_CTRL written with %02x: ", data);
	switch(data&0xe)
	{
		case 0x0: case 0x2: case 0x4: case 0x6: // CS was low, chip is idle
			m_state = ER2055_IDLE;
			break;
		case 0x8: // C1 = 1, C2 = 0: Read, (wait for clock, if clock has arrived, idle).
			if ((m_state==ER2055_READ_WAITING_FOR_CLOCK)&&(((m_old_ctrl&1)==0)&&((data&1)==1))) // rising clock edge?
				m_state = ER2055_READ_DRIVING_BUS;
			else if (m_state!=ER2055_READ_DRIVING_BUS) // if we're already driving the bus, stay driving it. otherwise we're still waiting.
				m_state = ER2055_READ_WAITING_FOR_CLOCK;
			break;
		case 0xA: // C1 = 0, C2 = 0: Write (set gate 0 if the bit is a 0, set gate 1 if the bit is a 1) HACK: we treat this as WRITE always to handle both wiring variants
			m_state = ER2055_WRITING_BITS;
			break;
		case 0xC: // C1 = 1, C2 = 1: Invalid // HACK: we treat this as WRITE always to handle both wiring variants
			logerror("INVALID CTRL STATE!");
			m_state = ER2055_WRITING_BITS;
			break;
		case 0xE: // C1 = 0, C2 = 1: Erase (zero both gates for all bits of the current byte; byte will read as random garbage after this, different each time!)
			m_state = ER2055_ERASING_BITS;
			break;
	}
	logerror("state is now %s\n", er2055_State[m_state]);

	switch(m_state)
	{
		case ER2055_IDLE: // idle, do nothing.
		case ER2055_READ_WAITING_FOR_CLOCK: // waiting for a clock rising edge which we haven't yet seen, do nothing
			m_out_data = 0xFF;
			break;
		case ER2055_READ_DRIVING_BUS: // bus is being driven with data
			m_out_data = m_rom[m_in_offset];
			break;
		case ER2055_WRITING_BITS: // write contents of bus to rom
			m_rom[m_in_offset] = m_in_data;
			m_out_data = m_in_data;
			break;
		case ER2055_ERASING_BITS: // erase contents of rom
			m_rom[m_in_offset] = 0; //TODO: this should actually set the rom contents to an invalid state which reads inconsistently until it is written to by a WRITE command
			m_out_data = m_in_data;
			break;
	}
	m_old_ctrl = data;
}

//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void atari_vg_earom_device::nvram_default()
{
	memset(m_rom,0,EAROM_SIZE);
}

//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

void atari_vg_earom_device::nvram_read(emu_file &file)
{
	file.read(m_rom,EAROM_SIZE);
}

//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

void atari_vg_earom_device::nvram_write(emu_file &file)
{
	file.write(m_rom,EAROM_SIZE);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void atari_vg_earom_device::device_start()
{
	/* register for save states */
	save_item(NAME(m_state));
	save_item(NAME(m_old_ctrl));
	save_item(NAME(m_in_offset));
	save_item(NAME(m_in_data));
	save_item(NAME(m_out_data));
	save_item(NAME(m_rom));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void atari_vg_earom_device::device_reset()
{
	m_state = ER2055_READ_WAITING_FOR_CLOCK; // start in read mode
	m_old_ctrl = 0;
	m_in_offset = 0;
	m_in_data = 0;
	m_out_data = 0xFF; // the value driven to the bus by the resistors if EAROM is open bus
}
