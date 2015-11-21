// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 *   scmp.c
 *
 *   National Semiconductor SC/MP CPU Disassembly
 *
 *****************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "scmp.h"

#define VERBOSE 0

#define LOG(x) do { if (VERBOSE) logerror x; } while (0)


const device_type SCMP = &device_creator<scmp_device>;
const device_type INS8060 = &device_creator<ins8060_device>;


scmp_device::scmp_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, SCMP, "INS 8050 SC/MP", tag, owner, clock, "ins8050", __FILE__)
	, m_program_config("program", ENDIANNESS_LITTLE, 8, 16, 0), m_AC(0), m_ER(0), m_SR(0), m_program(nullptr), m_direct(nullptr), m_icount(0)
	  , m_flag_out_func(*this)
	, m_sout_func(*this)
	, m_sin_func(*this)
	, m_sensea_func(*this)
	, m_senseb_func(*this)
	, m_halt_func(*this)
{
}


scmp_device::scmp_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: cpu_device(mconfig, type, name, tag, owner, clock, shortname, source)
	, m_program_config("program", ENDIANNESS_LITTLE, 8, 16, 0), m_AC(0), m_ER(0), m_SR(0), m_program(nullptr), m_direct(nullptr), m_icount(0)
	  , m_flag_out_func(*this)
	, m_sout_func(*this)
	, m_sin_func(*this)
	, m_sensea_func(*this)
	, m_senseb_func(*this)
	, m_halt_func(*this)
{
}


ins8060_device::ins8060_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: scmp_device(mconfig, INS8060, "INS 8060 SC/MP II", tag, owner, clock, "ins8060", __FILE__)
{
}


offs_t scmp_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( scmp );
	return CPU_DISASSEMBLE_NAME(scmp)(this, buffer, pc, oprom, opram, options);
}


UINT16 scmp_device::ADD12(UINT16 addr, INT8 val)
{
	return ((addr + val) & 0x0fff) | (addr & 0xf000);
}

UINT8 scmp_device::ROP()
{
	UINT16 pc = m_PC.w.l;
	m_PC.w.l = ADD12(m_PC.w.l,1);
	return m_direct->read_byte( pc);
}

UINT8 scmp_device::ARG()
{
	UINT16 pc = m_PC.w.l;
	m_PC.w.l = ADD12(m_PC.w.l,1);
	return m_direct->read_byte(pc);
}

UINT8 scmp_device::RM(UINT32 a)
{
	return m_program->read_byte(a);
}

void scmp_device::WM(UINT32 a, UINT8 v)
{
	m_program->write_byte(a, v);
}

void scmp_device::illegal(UINT8 opcode)
{
#if VERBOSE
	UINT16 pc = m_PC.w.l;
	LOG(("SC/MP illegal instruction %04X $%02X\n", pc-1, opcode));
#endif
}

PAIR *scmp_device::GET_PTR_REG(int num)
{
	switch(num) {
		case 1: return &m_P1;
		case 2: return &m_P2;
		case 3: return &m_P3;
		default :
				return &m_PC;
	}
}

void scmp_device::BIN_ADD(UINT8 val)
{
	UINT16 tmp = m_AC + val + ((m_SR >> 7) & 1);
	UINT8 ov = (((m_AC & 0x80)==(val & 0x80)) && ((m_AC & 0x80)!=(tmp & 0x80))) ? 0x40 : 0x00;

	m_AC = tmp & 0xff;
	m_SR &= 0x3f; // clear CY/L and OV flag
	m_SR |= (tmp & 0x100) ? 0x80 : 0x00; // set CY/L
	m_SR |= ov;
}

void scmp_device::DEC_ADD(UINT8 val)
{
	UINT16 tmp = m_AC + val + ((m_SR >> 7) & 1);
	if ((tmp & 0x0f) > 9) tmp +=6;
	m_AC = tmp % 0xa0;
	m_SR &= 0x7f; // clear CY/L flag
	m_SR |= (tmp > 0x99) ? 0x80 : 0x00;
}

UINT16 scmp_device::GET_ADDR(UINT8 code)
{
	UINT16 addr = 0;
	INT8 offset = 0;
	UINT16 retVal = 0;
	UINT16 ptr = GET_PTR_REG(code & 0x03)->w.l;

	UINT8 arg = ARG();
	if (arg == 0x80) {
		offset = m_ER;
	} else {
		if (arg & 0x80) {
			offset = (INT8)arg;
		} else {
			offset = arg;
		}
	}

	addr = ADD12(ptr,offset);

	if (code & 0x04) {
		if (code & 0x03) {
			// Auto-indexed
			if (offset < 0) {
				// pre decrement
				GET_PTR_REG(code & 0x03)->w.l = addr;
				retVal = addr;
			} else {
				// post increment
				retVal = ptr;
				GET_PTR_REG(code & 0x03)->w.l = addr;
			}
		} else {
			// Immediate
		}
	} else {
		// Indexed
		retVal = addr;
	}
	return retVal;
}

void scmp_device::execute_one(int opcode)
{
	UINT8 tmp;
	UINT8 ptr = opcode & 3;
	if (BIT(opcode,7)) {
		// two bytes instructions
		switch (opcode)
		{
			// Memory Reference Instructions
			case 0xc0 : case 0xc1 : case 0xc2 : case 0xc3 :
			case 0xc5 : case 0xc6 : case 0xc7 :
						//LD
						m_icount -= 18;
						m_AC = RM(GET_ADDR(opcode));
						break;
			case 0xc8 : case 0xc9 : case 0xca : case 0xcb :
			case 0xcd : case 0xce : case 0xcf :
						// ST
						m_icount -= 18;
						WM(GET_ADDR(opcode),m_AC);
						break;
			case 0xd0 : case 0xd1 : case 0xd2 : case 0xd3 :
						case 0xd5 : case 0xd6 : case 0xd7 :
						// AND
						m_icount -= 18;
						m_AC &= RM(GET_ADDR(opcode));
						break;
			case 0xd8 : case 0xd9 : case 0xda : case 0xdb :
						case 0xdd : case 0xde : case 0xdf :
						//OR
						m_icount -= 18;
						m_AC |= RM(GET_ADDR(opcode));
						break;
			case 0xe0 : case 0xe1 : case 0xe2 : case 0xe3 :
						case 0xe5 : case 0xe6 : case 0xe7 :
						// XOR
						m_icount -= 18;
						m_AC ^= RM(GET_ADDR(opcode));
						break;
			case 0xe8 : case 0xe9 : case 0xea : case 0xeb :
						case 0xed : case 0xee : case 0xef :
						// DAD
						m_icount -= 23;
						DEC_ADD(RM(GET_ADDR(opcode)));
						break;
			case 0xf0 : case 0xf1 : case 0xf2 : case 0xf3 :
						case 0xf5 : case 0xf6 : case 0xf7 :
						// ADD
						m_icount -= 19;
						BIN_ADD(RM(GET_ADDR(opcode)));
						break;
			case 0xf8 : case 0xf9 : case 0xfa : case 0xfb :
						case 0xfd : case 0xfe : case 0xff :
						// CAD
						m_icount -= 20;
						BIN_ADD(~RM(GET_ADDR(opcode)));
						break;
			// Memory Increment/Decrement Instructions
			case 0xa8 : case 0xa9 : case 0xaa : case 0xab :
						// IDL
						{
							UINT16 addr = GET_ADDR(opcode);
							m_icount -= 22;
							m_AC = RM(addr) + 1;
							WM(addr,m_AC);
						}
						break;
			case 0xb8 : case 0xb9 : case 0xba : case 0xbb :
						// DLD
						{
							UINT16 addr = GET_ADDR(opcode);
							m_icount -= 22;
							m_AC = RM(addr) - 1;
							WM(addr,m_AC);
						}
						break;
			// Immediate Instructions
			case 0xc4 : // LDI
						m_icount -= 10;
						m_AC = ARG();
						break;
			case 0xd4 : // ANI
						m_icount -= 10;
						m_AC &= ARG();
						break;
			case 0xdc : // ORI
						m_icount -= 10;
						m_AC |= ARG();
						break;
			case 0xe4 : // XRI
						m_icount -= 10;
						m_AC ^= ARG();
						break;
			case 0xec : // DAI
						m_icount -= 15;
						DEC_ADD(ARG());
						break;
			case 0xf4 : // ADI
						m_icount -= 11;
						BIN_ADD(ARG());
						break;
			case 0xfc : // CAI
						m_icount -= 12;
						BIN_ADD(~ARG());
						break;
			// Transfer Instructions
			case 0x90 : case 0x91 : case 0x92 : case 0x93 :// JMP
						m_icount -= 11;
						m_PC.w.l = ADD12(GET_PTR_REG(ptr)->w.l,(INT8)ARG());
						break;
			case 0x94 : case 0x95 : case 0x96 : case 0x97 :
						// JP
						m_icount -= 9;
						tmp = ARG();
						if (!(m_AC & 0x80)) {
							m_PC.w.l = ADD12(GET_PTR_REG(ptr)->w.l,(INT8)tmp);
							m_icount -= 2;
						}
						break;
			case 0x98 : case 0x99 : case 0x9a : case 0x9b :
						// JZ
						m_icount -= 9;
						tmp = ARG();
						if (!m_AC) {
							m_PC.w.l = ADD12(GET_PTR_REG(ptr)->w.l,(INT8)tmp);
							m_icount -= 2;
						}
						break;
			case 0x9c : case 0x9d : case 0x9e : case 0x9f :
						// JNZ
						m_icount -= 9;
						tmp = ARG();
						if (m_AC) {
							m_PC.w.l = ADD12(GET_PTR_REG(ptr)->w.l,(INT8)tmp);
							m_icount -= 2;
						}
						break;
			// Double-Byte Miscellaneous Instructions
			case 0x8f:  // DLY
						tmp = ARG();
						m_icount -= 13 + (m_AC * 2) + (((UINT32)tmp) << 1) + (((UINT32)tmp) << 9);
						m_AC = 0xff;
						break;
			// Others are illegal
			default :   m_icount -= 1;
						illegal (opcode);
						break;
		}
	} else {
		// one byte instructions
		switch (opcode)
		{
			// Extension Register Instructions
			case 0x40:  // LDE
						m_icount -= 6;
						m_AC = m_ER;
						break;
			case 0x01:  // XAE
						m_icount -= 7;
						tmp = m_AC;
						m_AC = m_ER;
						m_ER = tmp;
						break;
			case 0x50:  // ANE
						m_icount -= 6;
						m_AC &= m_ER;
						break;
			case 0x58:  // ORE
						m_icount -= 6;
						m_AC |= m_ER;
						break;
			case 0x60:  // XRE
						m_icount -= 6;
						m_AC ^= m_ER;
						break;
			case 0x68:  // DAE
						m_icount -= 11;
						DEC_ADD(m_ER);
						break;
			case 0x70:  // ADE
						m_icount -= 7;
						BIN_ADD(m_ER);
						break;
			case 0x78:  // CAE
						m_icount -= 8;
						BIN_ADD(~m_ER);
						break;
			// Pointer Register Move Instructions
			case 0x30: case 0x31: case 0x32: case 0x33: // XPAL
						m_icount -= 8;
						tmp = m_AC;
						m_AC = GET_PTR_REG(ptr)->b.l;
						GET_PTR_REG(ptr)->b.l = tmp;
						break;
			case 0x34:  case 0x35 :case 0x36: case 0x37:
						// XPAH
						m_icount -= 8;
						tmp = m_AC;
						m_AC = GET_PTR_REG(ptr)->b.h;
						GET_PTR_REG(ptr)->b.h = tmp;
						break;
			case 0x3c:  case 0x3d :case 0x3e: case 0x3f:
						// XPPC
						{
							UINT16 tmp16 = ADD12(m_PC.w.l,-1); // Since PC is incremented we need to fix it
							m_icount -= 7;
							m_PC.w.l = GET_PTR_REG(ptr)->w.l;
							GET_PTR_REG(ptr)->w.l = tmp16;
							// After exchange CPU increment PC
							m_PC.w.l = ADD12(m_PC.w.l,1);
						}
						break;
			// Shift, Rotate, Serial I/O Instructions
			case 0x19:  // SIO
						m_icount -= 5;
						m_sout_func(m_ER & 0x01);
						m_ER >>= 1;
						m_ER |= m_sin_func() ? 0x80 : 0x00;
						break;
			case 0x1c:  // SR
						m_icount -= 5;
						m_AC >>= 1;
						break;
			case 0x1d:  // SRL
						m_icount -= 5;
						m_AC >>= 1;
						m_AC |= m_SR & 0x80; // add C/L flag
						break;
			case 0x1e:  // RR
						m_icount -= 5;
						m_AC =  (m_AC >> 1) | ((m_AC & 0x01) << 7);
						break;
			case 0x1f:  // RRL
						m_icount -= 5;
						tmp = (m_AC & 0x01) << 7;
						m_AC =  (m_AC >> 1) | (m_SR & 0x80);
						m_SR = (m_SR & 0x7f) | tmp;
						break;
			// Single Byte Miscellaneous Instructions
			case 0x00:  // HALT
						m_icount -= 8;
						m_halt_func(1);
						m_halt_func(0);
						break;
			case 0x02:  // CCL
						m_icount -= 5;
						m_SR &= 0x7f;
						break;
			case 0x03:  // SCL
						m_icount -= 5;
						m_SR |= 0x80;
						break;
			case 0x04:  // DINT
						m_icount -= 6;
						m_SR &= 0xf7;
						break;
			case 0x05:  // IEN
						m_icount -= 6;
						m_SR |= 0x08;
						break;
			case 0x06:  // CSA
						m_icount -= 5;
						m_SR &= 0xcf; // clear SA and SB flags
						m_SR |= m_sensea_func() ? 0x10 : 0x00;
						m_SR |= m_senseb_func() ? 0x20 : 0x00;
						m_AC = m_SR;
						break;
			case 0x07:  // CAS
						m_icount -= 6;
						m_SR = m_AC;
						m_flag_out_func(m_SR & 0x07);
						break;
			case 0x08:  // NOP
						m_icount -= 5;
						break;
			// Others are illegal
			default :   m_icount -= 1;
						illegal (opcode);
						break;
		}
	}
}


/***************************************************************************
    COMMON EXECUTION
***************************************************************************/
void scmp_device::take_interrupt()
{
	UINT16 tmp = ADD12(m_PC.w.l,-1); // We fix PC so at return it goes to current location
	m_SR &= 0xf7; // clear IE flag

	m_icount -= 8; // assumption
	// do XPPC 3
	m_PC.w.l = GET_PTR_REG(3)->w.l;
	GET_PTR_REG(3)->w.l = tmp;
	// After exchange CPU increment PC
	m_PC.w.l = ADD12(m_PC.w.l,1);
}

void scmp_device::execute_run()
{
	do
	{
		if ((m_SR & 0x08) && (m_sensea_func())) {
			take_interrupt();
		}
		debugger_instruction_hook(this, m_PC.d);
		execute_one(ROP());

	} while (m_icount > 0);
}

/***************************************************************************
    CORE INITIALIZATION
***************************************************************************/

void scmp_device::device_start()
{
	/* set up the state table */
	{
		state_add(SCMP_PC,     "PC",    m_PC.w.l);
		state_add(STATE_GENPC, "GENPC", m_PC.w.l).noshow();
		state_add(STATE_GENFLAGS, "GENFLAGS", m_SR).noshow().formatstr("%8s");
		state_add(SCMP_P1,     "P1",    m_P1.w.l);
		state_add(SCMP_P2,     "P2",    m_P2.w.l);
		state_add(SCMP_P3,     "P3",    m_P3.w.l);
		state_add(SCMP_AC,     "AC",    m_AC);
		state_add(SCMP_ER,     "ER",    m_ER);
		state_add(SCMP_SR,     "SR",    m_SR);
	}

	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();

	/* resolve callbacks */
	m_flag_out_func.resolve_safe();
	m_sout_func.resolve_safe();
	m_sin_func.resolve_safe(0);
	m_sensea_func.resolve_safe(0);
	m_senseb_func.resolve_safe(0);
	m_halt_func.resolve_safe();

	save_item(NAME(m_PC));
	save_item(NAME(m_P1));
	save_item(NAME(m_P2));
	save_item(NAME(m_P3));
	save_item(NAME(m_AC));
	save_item(NAME(m_ER));
	save_item(NAME(m_SR));

	m_icountptr = &m_icount;
}



/***************************************************************************
    COMMON RESET
***************************************************************************/

void scmp_device::device_reset()
{
	m_PC.d = 0;
	m_P1.d = 0;
	m_P2.d = 0;
	m_P3.d = 0;
	m_AC = 0;
	m_ER = 0;
	m_SR = 0;
}



/***************************************************************************
    COMMON STATE IMPORT/EXPORT
***************************************************************************/

void scmp_device::state_string_export(const device_state_entry &entry, std::string &str)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			strprintf(str, "%c%c%c%c%c%c%c%c",
				(m_SR & 0x80) ? 'C' : '.',
				(m_SR & 0x40) ? 'V' : '.',
				(m_SR & 0x20) ? 'B' : '.',
				(m_SR & 0x10) ? 'A' : '.',
				(m_SR & 0x08) ? 'I' : '.',
				(m_SR & 0x04) ? '2' : '.',
				(m_SR & 0x02) ? '1' : '.',
				(m_SR & 0x01) ? '0' : '.');
			break;
	}
}
