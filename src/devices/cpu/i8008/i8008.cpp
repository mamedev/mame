// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 *   i8008.c
 *
 *   Intel 8008 CPU
 *
 *****************************************************************************/
#include "emu.h"
#include "debugger.h"
#include "i8008.h"

//**************************************************************************
//  MACROS
//**************************************************************************

#define REG_1                   ((opcode >> 3) & 7)
#define REG_2                   (opcode & 7)
#define GET_PC                  (m_ADDR[m_pc_pos])

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type I8008 = &device_creator<i8008_device>;

//**************************************************************************
//  DEVICE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  i8008_device - constructor
//-------------------------------------------------
i8008_device::i8008_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, I8008, "i8008", tag, owner, clock, "i8008", __FILE__),
		m_program_config("program", ENDIANNESS_LITTLE, 8, 14),
		m_io_config("io", ENDIANNESS_LITTLE, 8, 8),
		m_program(nullptr),
		m_direct(nullptr)
{
	// set our instruction counter
	m_icountptr = &m_icount;
}

//-------------------------------------------------
//  device_start - start up the device
//-------------------------------------------------

void i8008_device::device_start()
{
	// find address spaces
	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();
	m_io = &space(AS_IO);

	// save state
	save_item(NAME(m_PC));
	save_item(NAME(m_A));
	save_item(NAME(m_B));
	save_item(NAME(m_C));
	save_item(NAME(m_D));
	save_item(NAME(m_E));
	save_item(NAME(m_H));
	save_item(NAME(m_L));
	save_item(NAME(m_CF));
	save_item(NAME(m_SF));
	save_item(NAME(m_ZF));
	save_item(NAME(m_PF));
	save_item(NAME(m_pc_pos));
	save_item(NAME(m_ADDR[0]));
	save_item(NAME(m_ADDR[1]));
	save_item(NAME(m_ADDR[2]));
	save_item(NAME(m_ADDR[3]));
	save_item(NAME(m_ADDR[4]));
	save_item(NAME(m_ADDR[5]));
	save_item(NAME(m_ADDR[6]));
	save_item(NAME(m_ADDR[7]));
	save_item(NAME(m_HALT));
	save_item(NAME(m_irq_state));

	// register our state for the debugger
	state_add(I8008_PC,       "PC",       m_PC.w.l).mask(0x3fff);
	state_add(STATE_GENPC,    "GENPC",    m_PC.w.l).mask(0x3fff).noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_flags).mask(0x0f).callimport().callexport().noshow().formatstr("%4s");
	state_add(I8008_A,        "A",        m_A);
	state_add(I8008_B,        "B",        m_B);
	state_add(I8008_C,        "C",        m_C);
	state_add(I8008_D,        "D",        m_D);
	state_add(I8008_E,        "E",        m_E);
	state_add(I8008_H,        "H",        m_H);
	state_add(I8008_L,        "L",        m_L);

	for (int addrnum = 0; addrnum < 8; addrnum++)
		state_add(I8008_ADDR1 + addrnum, strformat("ADDR%d", addrnum + 1).c_str(), m_ADDR[addrnum].w.l).mask(0xfff);

	init_tables();
}

void i8008_device::init_tables (void)
{
	int i;
	UINT8 p;
	for (i = 0; i < 256; i++)
	{
		p = 0;
		if (BIT(i,0)) p++;
		if (BIT(i,1)) p++;
		if (BIT(i,2)) p++;
		if (BIT(i,3)) p++;
		if (BIT(i,4)) p++;
		if (BIT(i,5)) p++;
		if (BIT(i,6)) p++;
		if (BIT(i,7)) p++;
		m_PARITY[i] = ((p&1) ? 0 : 1);
	}
}

//-------------------------------------------------
//  device_reset - reset the device
//-------------------------------------------------

void i8008_device::device_reset()
{
	m_CF = m_SF = m_ZF = m_PF = 0;
	m_A = m_B = m_C = m_D = m_E = m_H = m_L = 0;
	m_PC.d = 0;
	m_pc_pos = 0;
	m_HALT = 0;
	m_irq_state = CLEAR_LINE;
	memset(m_ADDR,0,sizeof(m_ADDR));
}

//-------------------------------------------------
//  memory_space_config - return the configuration
//  of the specified address space, or NULL if
//  the space doesn't exist
//-------------------------------------------------

const address_space_config *i8008_device::memory_space_config(address_spacenum spacenum) const
{
	return  (spacenum == AS_PROGRAM) ? &m_program_config :
			(spacenum == AS_IO) ? &m_io_config :
			nullptr;
}

//-------------------------------------------------
//  state_import - import state into the device,
//  after it has been set
//-------------------------------------------------

void i8008_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			m_CF = (m_flags >> 3) & 1;
			m_ZF = (m_flags >> 2) & 1;
			m_SF = (m_flags >> 1) & 1;
			m_PF = (m_flags >> 0) & 1;
			break;
	}
}

//-------------------------------------------------
//  state_export - export state from the device,
//  to a known location where it can be read
//-------------------------------------------------

void i8008_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			m_flags = (m_CF ? 0x08 : 0x00) |
								(m_ZF ? 0x04 : 0x00) |
								(m_SF ? 0x02 : 0x00) |
								(m_PF ? 0x01 : 0x00);
			break;
	}
}

//-------------------------------------------------
//  state_string_export - export state as a string
//  for the debugger
//-------------------------------------------------

void i8008_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			strprintf(str, "%c%c%c%c",
				m_CF ? 'C':'.',
				m_ZF ? 'Z':'.',
				m_SF ? 'S':'.',
				m_PF ? 'P':'.');
			break;
	}
}

//-------------------------------------------------
//  disasm_min_opcode_bytes - return the length
//  of the shortest instruction, in bytes
//-------------------------------------------------

UINT32 i8008_device::disasm_min_opcode_bytes() const
{
	return 1;
}

//-------------------------------------------------
//  disasm_max_opcode_bytes - return the length
//  of the longest instruction, in bytes
//-------------------------------------------------

UINT32 i8008_device::disasm_max_opcode_bytes() const
{
	return 3;
}

//-------------------------------------------------
//  disasm_disassemble - call the disassembly
//  helper function
//-------------------------------------------------

offs_t i8008_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( i8008 );
	return CPU_DISASSEMBLE_NAME(i8008)(this, buffer, pc, oprom, opram, options);
}

//**************************************************************************
//  EXECUTION
//**************************************************************************

//-------------------------------------------------
//  execute_min_cycles - return minimum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

UINT32 i8008_device::execute_min_cycles() const
{
	return 8;
}

//-------------------------------------------------
//  execute_max_cycles - return maximum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

UINT32 i8008_device::execute_max_cycles() const
{
	return 16;
}

//-------------------------------------------------
//  execute_set_input - set input and IRQ lines
//-------------------------------------------------

void i8008_device::execute_set_input(int inputnum, int state)
{
	m_irq_state = state;
}

//-------------------------------------------------
//  execute_run - execute until our icount expires
//-------------------------------------------------

void i8008_device::execute_run()
{
	do
	{
		if (m_irq_state != CLEAR_LINE) {
			take_interrupt();
		}
		debugger_instruction_hook(this, m_PC.d);
		execute_one(rop());
	} while (m_icount > 0);
}

inline void i8008_device::illegal(UINT8 opcode)
{
	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
	{
		logerror("I8008 illegal instruction %04X $%02X\n", m_PC.w.l, opcode);
	}
}

void i8008_device::take_interrupt()
{
	if (m_HALT) {
		GET_PC.w.l = (GET_PC.w.l + 1) & 0x3fff;
		m_PC = GET_PC;
		m_HALT = 0;
	}
	// For now only support one byte operation to be executed
	execute_one(standard_irq_callback(0));
}

inline void i8008_device::execute_one(int opcode)
{
	UINT16 tmp;

	switch (opcode >> 6)
	{
		case 0x03:  // starting with 11
					if (opcode==0xff) {
						// HLT
						m_icount -= 4;
						GET_PC.w.l = GET_PC.w.l - 1;
						m_PC = GET_PC;
						m_HALT = 1;
					} else {
						// Lrr
						m_icount -= 5;
						if (REG_1==7) m_icount -= 2;
						if (REG_2==7) m_icount -= 3;
						set_reg(REG_1, get_reg(REG_2));
					}
					break;
		case 0x00:  // starting with 00
					switch(opcode & 7) {
						case 0 :    if(((opcode >> 3) & 7)==0) {
										// HLT
										m_icount -= 4;
										GET_PC.w.l = GET_PC.w.l - 1;
										m_PC = GET_PC;
										m_HALT = 1;
									} else {
										if(((opcode >> 3) & 7)==7) {
											// ILLEGAL
											m_icount -= 5;
											illegal(opcode);
										} else {
											// INr
											m_icount -= 5;
											tmp = get_reg(REG_1) + 1;
											set_reg(REG_1, tmp & 0xff);
											update_flags(tmp & 0xff);
										}
									}
									break;
						case 1 :    if(((opcode >> 3) & 7)==0) {
										// HLT
										m_icount -= 4;
										GET_PC.w.l = GET_PC.w.l - 1;
										m_PC = GET_PC;
										m_HALT = 1;
									} else {
										if(((opcode >> 3) & 7)==7) {
											// ILLEGAL
											m_icount -= 5;
											illegal(opcode);
										} else {
											// DCr
											m_icount -= 5;
											tmp = get_reg(REG_1) - 1;
											set_reg(REG_1, tmp & 0xff);
											update_flags(tmp & 0xff);
										}
									}
									break;
						case 2 :    {
										// All instuction from this group have same timing
										m_icount -= 5;
										switch((opcode >> 3) & 7) {
											case 0 :
												// RLC
												tmp = m_A;
												m_A = (m_A << 1) | BIT(tmp,7);
												m_CF = BIT(tmp,7);
												break;
											case 1 :
												// RRC
												tmp = m_A;
												m_A = (m_A >> 1) | (BIT(tmp,0) ? 0x80 : 0x00);
												m_CF = BIT(tmp,0);
												break;
											case 2 :
												// RAL
												tmp = m_A;
												m_A = (m_A << 1) | m_CF;
												m_CF = BIT(tmp,7);
												break;
											case 3 :
												// RAR
												tmp = m_A;
												m_A = (m_A >> 1) | (m_CF ? 0x80 : 0x00);
												m_CF = BIT(tmp,0);
												break;
											default :
												// ILLEGAL
												illegal(opcode);
												break;
										}
									}
									break;
						case 3 :
									// Rcc
									{
										m_icount -= 3;
										if (do_condition(opcode)==1) {
											m_icount -= 2;
											pop_stack();
											m_PC = GET_PC;
										}
									}
									break;
						case 4 :    {
										m_icount -= 8;
										switch((opcode >> 3) & 7) {
											case 0 :
												// ADI
												tmp = get_reg(0) + arg();
												set_reg(0,tmp & 0xff);
												update_flags(tmp & 0xff);
												m_CF = (tmp >> 8) & 1;
												break;
											case 1 :
												// ACI
												tmp = get_reg(0) + arg() + m_CF;
												set_reg(0,tmp & 0xff);
												update_flags(tmp & 0xff);
												m_CF = (tmp >> 8) & 1;
												break;
											case 2 :
												// SUI
												tmp = get_reg(0) - arg();
												set_reg(0,tmp & 0xff);
												update_flags(tmp & 0xff);
												m_CF = (tmp >> 8) & 1;
												break;
											case 3 :
												// SBI
												tmp = get_reg(0) - arg() - m_CF;
												set_reg(0,tmp & 0xff);
												update_flags(tmp & 0xff);
												m_CF = (tmp >> 8) & 1;
												break;
											case 4 :
												// NDI
												tmp = get_reg(0) & arg();
												set_reg(0,tmp & 0xff);
												update_flags(tmp & 0xff);
												m_CF = 0;
												break;
											case 5 :
												// XRI
												tmp = get_reg(0) ^ arg();
												set_reg(0,tmp & 0xff);
												update_flags(tmp & 0xff);
												m_CF = 0;
												break;
											case 6 :
												// ORI
												tmp = get_reg(0) | arg();
												set_reg(0,tmp & 0xff);
												update_flags(tmp & 0xff);
												m_CF = 0;
												break;
											case 7 :
												// CPI
												tmp = get_reg(0) - arg();
												update_flags(tmp & 0xff);
												m_CF = (tmp >> 8) & 1;
												break;
										}
									}
									break;
						case 5 :    // RST
									m_icount -= 5;
									push_stack();
									GET_PC.w.l = opcode & 0x38;
									m_PC = GET_PC;
									break;
						case 6 :    // LrI
									m_icount -= 8;
									if (REG_1==7) m_icount -= 1; // LMI
									set_reg(REG_1, arg());
									break;
						case 7 :    // RET
									m_icount -= 5;
									pop_stack();
									m_PC = GET_PC;
									break;
					}
					break;

		case 0x01:  // starting with 01
					switch(opcode & 7) {
						case 0 :
							// Jcc
							m_icount -= 9;
							tmp = get_addr();
							if (do_condition(opcode)==1) {
								m_icount -= 2;
								GET_PC.w.l = tmp;
								m_PC = GET_PC;
							}
							break;
						case 2 :
							// Ccc
							m_icount -= 9;
							tmp = get_addr();
							if (do_condition(opcode)==1) {
								m_icount -= 2;
								push_stack();
								GET_PC.w.l = tmp;
								m_PC = GET_PC;
							}
							break;
						case 4 :
							// JMP
							m_icount -= 11;
							GET_PC.w.l = get_addr();
							m_PC = GET_PC;
							break;
						case 6 :
							// CAL
							m_icount -= 11;
							tmp = get_addr();
							push_stack();
							GET_PC.w.l = tmp;
							m_PC = GET_PC;
							break;
						default :
							if (((opcode>>4)&3)==0) {
								// INP
								m_icount -= 8;
								m_A = m_io->read_byte((opcode >> 1) & 0x1f);
							} else {
								// OUT
								m_icount -= 6;
								m_io->write_byte((opcode >> 1) & 0x1f, m_A);
							}
							break;
					}
					break;
		case 0x02:  // starting with 10
					m_icount -= 5;
					if ((opcode & 7)==7) m_icount -= 3; // operations with memory
					switch((opcode >> 3) & 7) {
						case 0 :
							// ADx
							tmp = get_reg(0) + get_reg(opcode & 7);
							set_reg(0,tmp & 0xff);
							update_flags(tmp & 0xff);
							m_CF = (tmp >> 8) & 1;
							break;
						case 1 :
							// ACx
							tmp = get_reg(0) + get_reg(opcode & 7) + m_CF;
							set_reg(0,tmp & 0xff);
							update_flags(tmp & 0xff);
							m_CF = (tmp >> 8) & 1;
							break;
						case 2 :
							// SUx
							tmp = get_reg(0) - get_reg(opcode & 7);
							set_reg(0,tmp & 0xff);
							update_flags(tmp & 0xff);
							m_CF = (tmp >> 8) & 1;
							break;
						case 3 :
							// SBx
							tmp = get_reg(0) - get_reg(opcode & 7) - m_CF;
							set_reg(0,tmp & 0xff);
							update_flags(tmp & 0xff);
							m_CF = (tmp >> 8) & 1;
							break;
						case 4 :
							// NDx
							tmp = get_reg(0) & get_reg(opcode & 7);
							set_reg(0,tmp & 0xff);
							update_flags(tmp & 0xff);
							m_CF = 0;
							break;
						case 5 :
							// XRx
							tmp = get_reg(0) ^ get_reg(opcode & 7);
							set_reg(0,tmp & 0xff);
							update_flags(tmp & 0xff);
							m_CF = 0;
							break;
						case 6 :
							// ORx
							tmp = get_reg(0) | get_reg(opcode & 7);
							set_reg(0,tmp & 0xff);
							update_flags(tmp & 0xff);
							m_CF = 0;
							break;
						case 7 :
							// CPx
							tmp = get_reg(0) - get_reg(opcode & 7);
							update_flags(tmp & 0xff);
							m_CF = (tmp >> 8) & 1;
							break;
					}
					break;
	}
}

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

inline void i8008_device::push_stack()
{
	m_pc_pos = (m_pc_pos + 1) & 7;
}

inline void i8008_device::pop_stack()
{
	m_ADDR[m_pc_pos].d = 0;
	m_pc_pos = (m_pc_pos - 1) & 7;
}

inline UINT8 i8008_device::rop()
{
	UINT8 retVal = m_direct->read_byte(GET_PC.w.l);
	GET_PC.w.l = (GET_PC.w.l + 1) & 0x3fff;
	m_PC = GET_PC;
	return retVal;
}

inline UINT8 i8008_device::get_reg(UINT8 reg)
{
	UINT8 retVal;
	switch(reg) {
		case 0 : retVal = m_A; break;
		case 1 : retVal = m_B; break;
		case 2 : retVal = m_C; break;
		case 3 : retVal = m_D; break;
		case 4 : retVal = m_E; break;
		case 5 : retVal = m_H; break;
		case 6 : retVal = m_L; break;
		default: retVal = m_program->read_byte((m_H << 8) + m_L); break;
	}
	return retVal;
}

inline void i8008_device::set_reg(UINT8 reg, UINT8 val)
{
	switch(reg) {
		case 0 : m_A = val; break;
		case 1 : m_B = val; break;
		case 2 : m_C = val; break;
		case 3 : m_D = val; break;
		case 4 : m_E = val; break;
		case 5 : m_H = val; break;
		case 6 : m_L = val; break;
		default: m_program->write_byte((m_H << 8) + m_L, val); break;
	}
}

inline UINT8 i8008_device::arg()
{
	UINT8 retVal = m_direct->read_byte(GET_PC.w.l);
	GET_PC.w.l = (GET_PC.w.l + 1) & 0x3fff;
	m_PC = GET_PC;
	return retVal;
}

inline void i8008_device::update_flags(UINT8 val)
{
	m_ZF = (val == 0) ? 1 : 0;
	m_SF = (val & 0x80) ? 1 : 0;
	m_PF = m_PARITY[val];
}

inline UINT8 i8008_device::do_condition(UINT8 val)
{
	UINT8 v = (val >> 5) & 1;
	UINT8 cond = 0;
	switch((val>> 3) & 0x03) {
		case 0 :
				if (m_CF==v) cond = 1;
				break;
		case 1 :
				if (m_ZF==v) cond = 1;
				break;
		case 2 :
				if (m_SF==v) cond = 1;
				break;
		case 3 :
				if (m_PF==v) cond = 1;
				break;
	}
	return cond;
}

inline UINT16 i8008_device::get_addr()
{
	UINT8 lo = arg();
	UINT8 hi = arg();
	return ((hi & 0x3f) << 8) + lo;
}
