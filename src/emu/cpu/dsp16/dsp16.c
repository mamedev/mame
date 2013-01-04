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
	  m_i(0),
	  m_pc(0),
	  m_pt(0),
	  m_pr(0),
	  m_pi(0),
	  m_j(0),
	  m_k(0),
	  m_rb(0),
	  m_re(0),
	  m_r0(0),
	  m_r1(0),
	  m_r2(0),
	  m_r3(0),
	  m_x(0),
	  m_y(0),
	  m_p(0),
	  m_a0(0),
	  m_a1(0),
	  m_auc(0),
	  m_psw(0),
	  m_c0(0),
	  m_c1(0),
	  m_c2(0),
	  m_sioc(0),
	  m_pioc(0),
	  m_ppc(0),
	  m_program(NULL),
	  m_direct(NULL),
	  m_icount(0)
{
    // Allocate & setup
}



//-------------------------------------------------
//  device_start - start up the device
//-------------------------------------------------

void dsp16_device::device_start()
{
	// register state with the debugger
	state_add(STATE_GENPC,    "GENPC",     m_pc).noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS",  m_psw).callimport().callexport().formatstr("%10s").noshow();
	state_add(DSP16_PC,       "PC",        m_pc);
	state_add(DSP16_I,        "I",         m_i);
	state_add(DSP16_PT,       "PT",        m_pt);
	state_add(DSP16_PR,       "PR",        m_pr);
	state_add(DSP16_PI,       "PI",        m_pi);
	state_add(DSP16_J,        "J",         m_j);
	state_add(DSP16_K,        "K",         m_k);
	state_add(DSP16_RB,       "RB",        m_rb);
	state_add(DSP16_RE,       "RE",        m_re);
	state_add(DSP16_R0,       "R0",        m_r0);
	state_add(DSP16_R1,       "R1",        m_r1);
	state_add(DSP16_R2,       "R2",        m_r2);
	state_add(DSP16_R3,       "R3",        m_r3);
	state_add(DSP16_X,        "X",         m_x);
	state_add(DSP16_Y,        "Y",         m_y);
	state_add(DSP16_P,        "P",         m_p);
	state_add(DSP16_A0,       "A0",        m_a0).mask(0xfffffffff);
	state_add(DSP16_A1,       "A1",        m_a1).mask(0xfffffffff);
	state_add(DSP16_AUC,      "AUC",       m_auc); //.formatstr("%6s");
	state_add(DSP16_PSW,      "PSW",       m_psw); //.formatstr("%16s");
	state_add(DSP16_C0,       "C0",        m_c0);
	state_add(DSP16_C1,       "C1",        m_c1);
	state_add(DSP16_C2,       "C2",        m_c2);
	state_add(DSP16_SIOC,     "SIOC",      m_sioc).formatstr("%16s");
	state_add(DSP16_PIOC,     "PIOC",      m_pioc); //.formatstr("%16s");

	save_item(NAME(m_i));
	save_item(NAME(m_pc));
	save_item(NAME(m_pt));
	save_item(NAME(m_pr));
	save_item(NAME(m_pi));
	save_item(NAME(m_j));
	save_item(NAME(m_k));
	save_item(NAME(m_rb));
	save_item(NAME(m_re));
	save_item(NAME(m_r0));
	save_item(NAME(m_r1));
	save_item(NAME(m_r2));
	save_item(NAME(m_r3));
	save_item(NAME(m_x));
	save_item(NAME(m_y));
	save_item(NAME(m_p));
	save_item(NAME(m_a0));
	save_item(NAME(m_a1));
	save_item(NAME(m_auc));
	save_item(NAME(m_psw));
	save_item(NAME(m_c0));
	save_item(NAME(m_c1));
	save_item(NAME(m_c2));
	save_item(NAME(m_sioc));
	save_item(NAME(m_pioc));

	// get our address spaces
	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();

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
//  state_string_export - export state as a string
//  for the debugger
//-------------------------------------------------

void dsp16_device::state_string_export(const device_state_entry &entry, astring &string)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
		string.printf("(see below)");
			break;

		// Placeholder for a better view later (TODO)
		case DSP16_SIOC:
			string.printf("%04x", *(UINT16*)entry.dataptr());
			break;
	}
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
	extern CPU_DISASSEMBLE( dsp16a );
	return CPU_DISASSEMBLE_NAME(dsp16a)(NULL, buffer, pc, oprom, opram, 0);
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

inline UINT32 dsp16_device::opcode_read(const UINT8 pcOffset)
{
	const UINT16 readPC = m_pc + pcOffset;
	return m_direct->read_decrypted_dword(readPC << 1);
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
	return 1;
}


void dsp16_device::execute_set_input(int inputnum, int state)
{
	// Only has one external IRQ line
}


void dsp16_device::execute_run()
{
	do
	{
		// debugging
		m_ppc = m_pc;	// copy PC to previous PC
		debugger_instruction_hook(this, m_pc);

		// instruction fetch & execute
		UINT8 cycles;
		UINT8 pcAdvance;
		const UINT16 op = opcode_read();
		execute_one(op, cycles, pcAdvance);

		// step forward
		m_pc += pcAdvance;
		m_icount -= cycles;

	} while (m_icount > 0);
}


void dsp16_device::execute_one(const UINT16 op, UINT8& cycles, UINT8& pcAdvance)
{
	cycles = 1;
	pcAdvance = 1;

	const UINT8 opcode = (op >> 11) & 0x1f;
	switch(opcode)
	{
		// Format 1: Multiply/ALU Read/Write Group
		case 0x06:
		{
			// F1, Y
			//const UINT8 Y = (op & 0x000f);
			//const UINT8 S = (op & 0x0200) >> 9;
			//const UINT8 D = (op & 0x0400) >> 10;
			//const UINT8 F1 = (op & 0x01e0) >> 5;
			break;
		}
		case 0x04: case 0x1c:
		{
			// F1 Y=a0[1] | F1 Y=a1[1]
			//const UINT8 Y = (op & 0x000f);
			//const UINT8 S = (op & 0x0200) >> 9;
			//const UINT8 D = (op & 0x0400) >> 10;
			//const UINT8 F1 = (op & 0x01e0) >> 5;
			break;
		}
		case 0x16:
		{
			// F1, x = Y
			//const UINT8 Y = (op & 0x000f);
			//const UINT8 S = (op & 0x0200) >> 9;
			//const UINT8 D = (op & 0x0400) >> 10;
			//const UINT8 F1 = (op & 0x01e0) >> 5;
			break;
		}
		case 0x17:
		{
			// F1, y[l] = Y
			//const UINT8 Y = (op & 0x000f);
			//const UINT8 X = (op & 0x0010) >> 4;
			//const UINT8 S = (op & 0x0200) >> 9;
			//const UINT8 D = (op & 0x0400) >> 10;
			//const UINT8 F1 = (op & 0x01e0) >> 5;
			break;
		}
		case 0x1f:
		{
			// F1, y = Y, x = *pt++[i]
			//const UINT8 Y = (op & 0x000f);
			//const UINT8 X = (op & 0x0010) >> 4;
			//const UINT8 S = (op & 0x0200) >> 9;
			//const UINT8 D = (op & 0x0400) >> 10;
			//const UINT8 F1 = (op & 0x01e0) >> 5;
			break;
		}
		case 0x19: case 0x1b:
		{
			// F1, y = a0|1, x = *pt++[i]
			//const UINT8 Y = (op & 0x000f);
			//const UINT8 X = (op & 0x0010) >> 4;
			//const UINT8 S = (op & 0x0200) >> 9;
			//const UINT8 D = (op & 0x0400) >> 10;
			//const UINT8 F1 = (op & 0x01e0) >> 5;
			break;
		}
		case 0x14:
		{
			// F1, Y = y[1]
			//const UINT8 Y = (op & 0x000f);
			//const UINT8 X = (op & 0x0010) >> 4;
			//const UINT8 S = (op & 0x0200) >> 9;
			//const UINT8 D = (op & 0x0400) >> 10;
			//const UINT8 F1 = (op & 0x01e0) >> 5;
			break;
		}

		// Format 1a: Multiply/ALU Read/Write Group (major typo in docs on p3-51)
		case 0x07:
		{
			// F1, At[1] = Y
			//const UINT8 Y = (op & 0x000f);
			//const UINT8 S = (op & 0x0200) >> 9;
			//const UINT8 aT = (op & 0x0400) >> 10;
			//const UINT8 F1 = (op & 0x01e0) >> 5;
			break;
		}

		// Format 2: Multiply/ALU Read/Write Group
		case 0x15:
		{
			// F1, Z : y[1]
			//const UINT8 Z = (op & 0x000f);
			//const UINT8 X = (op & 0x0010) >> 4;
			//const UINT8 S = (op & 0x0200) >> 9;
			//const UINT8 D = (op & 0x0400) >> 10;
			//const UINT8 F1 = (op & 0x01e0) >> 5;
			break;
		}
		case 0x1d:
		{
			// F1, Z : y, x=*pt++[i]
			//const UINT8 Z = (op & 0x000f);
			//const UINT8 X = (op & 0x0010) >> 4;
			//const UINT8 S = (op & 0x0200) >> 9;
			//const UINT8 D = (op & 0x0400) >> 10;
			//const UINT8 F1 = (op & 0x01e0) >> 5;
			break;
		}

		// Format 2a: Multiply/ALU Read/Write Group
		case 0x05:
		{
			// F1, Z : aT[1]
			//const UINT8 Z = (op & 0x000f);
			//const UINT8 X = (op & 0x0010) >> 4;
			//const UINT8 S = (op & 0x0200) >> 9;
			//const UINT8 aT = (op & 0x0400) >> 10;
			//const UINT8 F1 = (op & 0x01e0) >> 5;
			break;
		}

		// Format 3: Special Functions
		case 0x12:
		case 0x13:
		{
			// if|ifc CON F2
			//const UINT8 CON = (op & 0x001f);
			//const UINT8 S = (op & 0x0200) >> 9;
			//const UINT8 D = (op & 0x0400) >> 10;
			//const UINT8 F2 = (op & 0x01e0) >> 5;
			break;
		}

		// Format 4: Branch Direct Group
		case 0x00: case 0x01:
		{
			// goto JA
			const UINT16 JA = (op & 0x0fff) | (m_pc & 0xf000);
			m_pc = JA;
			pcAdvance = 0;
			break;
		}

		case 0x10: case 0x11:
		{
			// call JA
			//const UINT16 JA = (op & 0x0fff) | (m_pc & 0xf000);
			break;
		}

		// Format 5: Branch Indirect Group
		case 0x18:
		{
			// goto B
			//const UINT8 B = (op & 0x0700) >> 8;
			break;
		}

		// Format 6: Contitional Branch Qualifier/Software Interrupt (icall)
		case 0x1a:
		{
			// if CON [goto/call/return]
			//const UINT8 CON = (op & 0x001f);
			break;
		}

		// Format 7: Data Move Group
		case 0x09: case 0x0b:
		{
			// R = aS
			//const UINT8 R = (op & 0x03f0) >> 4;
			//const UINT8 S = (op & 0x1000) >> 12;
			break;
		}
		case 0x08:
		{
			// aT = R
			//const UINT8 R  = (op & 0x03f0) >> 4;
			//const UINT8 aT = (op & 0x0400) >> 10;
			break;
		}
		case 0x0f:
		{
			// R = Y
			//const UINT8 Y = (op & 0x000f);
			//const UINT8 R = (op & 0x03f0) >> 4;
			break;
		}
		case 0x0c:
		{
			// Y = R
			//const UINT8 Y = (op & 0x000f);
			//const UINT8 R = (op & 0x03f0) >> 4;
			break;
		}
		case 0x0d:
		{
			// Z : R
			//const UINT8 Z = (op & 0x000f);
			//const UINT8 R = (op & 0x03f0) >> 4;
			break;
		}

		// Format 8: Data Move (immediate operand - 2 words)
		case 0x0a:
		{
			// R = N
			//const UINT8 R = (op & 0x03f0) >> 4;
			pcAdvance++;
			break;
		}

		// Format 9: Short Immediate Group
		case 0x02: case 0x03:
		{
			// R = M
			//const UINT8 M = (op & 0x00ff);
			//const UINT8 R = (op & 0x0e00) >> 9;
			break;
		}

		// Format 10: do - redo
		case 0x0e:
		{
			// do|redo K
			//const UINT8 K = (op & 0x007f);
			//const UINT8 NI = (op & 0x0780) >> 7;
			break;
		}

		// RESERVED
		case 0x1e:
		{
			break;
		}

		// UNKNOWN
		default:
		{
			break;
		}
	}
}
