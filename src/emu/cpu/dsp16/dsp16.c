/***************************************************************************

    dsp16.h

    WE|AT&T DSP16 series emulator.

***************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "dsp16.h"


//**************************************************************************
//  DEVICE INTERFACE
//**************************************************************************

// device type definition
const device_type DSP16 = &device_creator<dsp16_device>;


//-------------------------------------------------
//  dsp16_device - constructor
//-------------------------------------------------

dsp16_device::dsp16_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, DSP16, "DSP16", tag, owner, clock),
	  m_program_config("program", ENDIANNESS_LITTLE, 16, 16, -1),
      m_pc(0),
      m_ppc(0),
      m_icount(0)
{
    // Allocate & setup
}



//-------------------------------------------------
//  device_start - start up the device
//-------------------------------------------------

void dsp16_device::device_start()
{
	// get our address spaces
	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();

	save_item(NAME(m_pc));

	// register state with the debugger
	state_add(DSP16_PC,      "PC",        m_pc);

	// set our instruction counter
	m_icountptr = &m_icount;
}


//-------------------------------------------------
//  device_reset - reset the device
//-------------------------------------------------

void dsp16_device::device_reset()
{
    m_pc = m_ppc = 0x0000;
}


//-------------------------------------------------
//  memory_space_config - return the configuration
//  of the specified address space, or NULL if
//  the space doesn't exist
//-------------------------------------------------

const address_space_config *dsp16_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == AS_PROGRAM) ? &m_program_config : NULL;
}


//-------------------------------------------------
//  state_import - import state into the device,
//  after it has been set
//-------------------------------------------------

void dsp16_device::state_import(const device_state_entry &entry)
{

}


//-------------------------------------------------
//  state_string_export - export state as a string
//  for the debugger
//-------------------------------------------------

void dsp16_device::state_string_export(const device_state_entry &entry, astring &string)
{
    string.printf("");
}


//-------------------------------------------------
//  disasm_min_opcode_bytes - return the length
//  of the shortest instruction, in bytes
//-------------------------------------------------

UINT32 dsp16_device::disasm_min_opcode_bytes() const
{
	return 2;
}


//-------------------------------------------------
//  disasm_max_opcode_bytes - return the length
//  of the longest instruction, in bytes
//-------------------------------------------------

UINT32 dsp16_device::disasm_max_opcode_bytes() const
{
	return 4;
}


//-------------------------------------------------
//  disasm_disassemble - call the disassembly
//  helper function
//-------------------------------------------------

offs_t dsp16_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( dsp16 );
	return CPU_DISASSEMBLE_NAME(dsp16)(NULL, buffer, pc, oprom, opram, 0);
}



/***************************************************************************
    MEMORY ACCESSORS
***************************************************************************/

inline UINT32 dsp16_device::program_read(UINT32 addr)
{
	return m_program->read_dword(addr << 1);
}

inline void dsp16_device::program_write(UINT32 addr, UINT32 data)
{
	m_program->write_dword(addr << 1, data & 0xffff);
}

inline UINT32 dsp16_device::opcode_read()
{
	return m_direct->read_decrypted_dword(m_pc << 1);
}


/***************************************************************************
    CORE EXECUTION LOOP
***************************************************************************/

//-------------------------------------------------
//  execute_min_cycles - return minimum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

UINT32 dsp16_device::execute_min_cycles() const
{
	return 1;
}


//-------------------------------------------------
//  execute_max_cycles - return maximum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

UINT32 dsp16_device::execute_max_cycles() const
{
	return 1;
}


//-------------------------------------------------
//  execute_input_lines - return the number of
//  input/interrupt lines
//-------------------------------------------------

UINT32 dsp16_device::execute_input_lines() const
{
	return 1;   // TODO
}


void dsp16_device::execute_set_input(int inputnum, int state)
{
}


void dsp16_device::execute_run()
{
	bool check_debugger = ((device_t::machine().debug_flags & DEBUG_FLAG_ENABLED) != 0);

	do
	{
		// debugging
		m_ppc = m_pc;	// copy PC to previous PC
		if (check_debugger)
			debugger_instruction_hook(this, m_pc);

		// instruction fetch
		//UINT16 op = opcode_read();

        m_icount--;
    } while (m_icount > 0);
}
