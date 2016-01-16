// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**************************\
*
*   SunPlus u'nSP core
*
*    by Ryan Holtz
*
\**************************/

#include "emu.h"
#include "debugger.h"
#include "unsp.h"


const device_type UNSP = &device_creator<unsp_device>;


unsp_device::unsp_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, UNSP, "u'nSP", tag, owner, clock, "unsp", __FILE__)
	, m_program_config("program", ENDIANNESS_BIG, 16, 23, -1), m_irq(0), m_fiq(0), m_curirq(0), m_sirq(0), m_sb(0), m_saved_sb(0), m_program(nullptr), m_icount(0), m_debugger_temp(0)
{
}


offs_t unsp_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( unsp );
	return CPU_DISASSEMBLE_NAME(unsp)(this, buffer, pc, oprom, opram, options);
}


/*****************************************************************************/

#define OP0     (op >> 12)
#define OPA     ((op >> 9) & 7)
#define OP1     ((op >> 6) & 7)
#define OPN     ((op >> 3) & 7)
#define OPB     (op & 7)
#define OPIMM   (op & 0x3f)

#define UNSP_LPC            (((UNSP_REG(SR) & 0x3f) << 16) | UNSP_REG(PC))

#define UNSP_REG(reg)       m_r[UNSP_##reg - 1]
#define UNSP_REG_I(reg)     m_r[reg]
#define UNSP_LREG_I(reg)    (((UNSP_REG(SR) << 6) & 0x3f0000) | UNSP_REG_I(reg))

#define UNSP_N  0x0200
#define UNSP_Z  0x0100
#define UNSP_S  0x0080
#define UNSP_C  0x0040

#define STANDARD_ALU_CASES \
		case 0: \
			lres = r0 + r1; \
			unsp_update_nzsc(lres, r0, r1); \
			break; \
		case 1: \
			lres = r0 + r1; \
			if(UNSP_REG(SR) & UNSP_C) lres++; \
			unsp_update_nzsc(lres, r0, r1); \
			break; \
		case 3: \
			lres = r0 + (~r1 & 0x0000ffff); \
			if(UNSP_REG(SR) & UNSP_C) lres++; \
			unsp_update_nzsc(lres, r0, r1); \
			break; \
		case 2: \
		case 4: \
			lres = r0 + (~r1 & 0x0000ffff) + 1; \
			unsp_update_nzsc(lres, r0, r1); \
			break; \
		case 6: \
			lres = -r1; \
			unsp_update_nz(lres); \
			break; \
		case 8: \
			lres = r0 ^ r1; \
			unsp_update_nz(lres); \
			break; \
		case 9: \
			lres = r1; \
			unsp_update_nz(lres); \
			break; \
		case 10: \
			lres = r0 | r1; \
			unsp_update_nz(lres); \
			break; \
		case 11: \
		case 12: \
			lres = r0 & r1; \
			unsp_update_nz(lres); \
			break

#define WRITEBACK_OPA \
		if(OP0 != 4 && OP0 < 12) \
		{ \
			UNSP_REG_I(OPA) = (UINT16)lres; \
		}

/*****************************************************************************/

void unsp_device::unimplemented_opcode(UINT16 op)
{
	fatalerror("UNSP: unknown opcode %04x at %04x\n", op, UNSP_LPC);
}

/*****************************************************************************/

UINT16 unsp_device::READ16(UINT32 address)
{
	return m_program->read_word(address<<1);
}

void unsp_device::WRITE16(UINT32 address, UINT16 data)
{
	m_program->write_word(address<<1, data);
}

/*****************************************************************************/

void unsp_device::device_start()
{
	memset(m_r, 0, sizeof(UINT16) * UNSP_GPR_COUNT);
	m_irq = 0;
	m_fiq = 0;
	m_curirq = 0;
	m_sirq = 0;
	m_sb = 0;
	m_saved_sb = 0;
	m_debugger_temp = 0;

	m_program = &space(AS_PROGRAM);

	state_add( UNSP_SP,  "SP", UNSP_REG(SP)).formatstr("%04X");
	state_add( UNSP_R1,  "R1", UNSP_REG(R1)).formatstr("%04X");
	state_add( UNSP_R2,  "R2", UNSP_REG(R2)).formatstr("%04X");
	state_add( UNSP_R3,  "R3", UNSP_REG(R3)).formatstr("%04X");
	state_add( UNSP_R4,  "R4", UNSP_REG(R4)).formatstr("%04X");
	state_add( UNSP_BP,  "BP", UNSP_REG(BP)).formatstr("%04X");
	state_add( UNSP_SR,  "SR", UNSP_REG(SR)).formatstr("%04X");
	state_add( UNSP_PC,  "PC", m_debugger_temp).callimport().callexport().formatstr("%06X");
	state_add( UNSP_IRQ, "IRQ", m_irq).formatstr("%1u");
	state_add( UNSP_FIQ, "FIQ", m_fiq).formatstr("%1u");
	state_add( UNSP_SB,  "SB", m_sb).formatstr("%1u");

	state_add(STATE_GENPC, "GENPC", m_debugger_temp).callexport().noshow();

	m_icountptr = &m_icount;
}

void unsp_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case UNSP_PC:
			m_debugger_temp = UNSP_LPC;
			break;

		case STATE_GENPC:
			m_debugger_temp = UNSP_LPC;
			break;
	}
}

void unsp_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case UNSP_PC:
			UNSP_REG(PC) = m_debugger_temp & 0x0000ffff;
			UNSP_REG(SR) = (UNSP_REG(SR) & 0xffc0) | ((m_debugger_temp & 0x003f0000) >> 16);
			break;
	}
}

void unsp_device::device_reset()
{
	memset(m_r, 0, sizeof(UINT16) * UNSP_GPR_COUNT);

	UNSP_REG(PC) = READ16(0xfff7);
	m_irq = 0;
	m_fiq = 0;
}

/*****************************************************************************/

void unsp_device::unsp_update_nz(UINT32 value)
{
	UNSP_REG(SR) &= ~(UNSP_N | UNSP_Z);
	if(value & 0x8000)
	{
		UNSP_REG(SR) |= UNSP_N;
	}
	if((UINT16)value == 0)
	{
		UNSP_REG(SR) |= UNSP_Z;
	}
}

void unsp_device::unsp_update_nzsc(UINT32 value, UINT16 r0, UINT16 r1)
{
	UNSP_REG(SR) &= ~(UNSP_C | UNSP_S);
	unsp_update_nz(value);
	if(value != (UINT16)value)
	{
		UNSP_REG(SR) |= UNSP_C;
	}

	if((INT16)r0 < (INT16)r1)
	{
		UNSP_REG(SR) |= UNSP_S;
	}
}

void unsp_device::unsp_push(UINT16 value, UINT16 *reg)
{
	WRITE16((*reg)--, value);
}

UINT16 unsp_device::unsp_pop(UINT16 *reg)
{
	return READ16(++(*reg));
}

void unsp_device::execute_run()
{
	UINT32 op;
	UINT32 lres;
	UINT16 r0, r1;
	lres = 0;

	while (m_icount > 0)
	{
		debugger_instruction_hook(this, UNSP_LPC);
		op = READ16(UNSP_LPC);

		UNSP_REG(PC)++;

		if(OP0 < 0xf && OPA == 0x7 && OP1 < 2)
		{
			switch(OP0)
			{
				case 0: // JB
					if(!(UNSP_REG(SR) & UNSP_C))
					{
						UNSP_REG(PC) += (OP1 == 0) ? OPIMM : (0 - OPIMM);
					}
					break;
				case 1: // JAE
					if(UNSP_REG(SR) & UNSP_C)
					{
						UNSP_REG(PC) += (OP1 == 0) ? OPIMM : (0 - OPIMM);
					}
					break;
				case 2: // JGE
					if(!(UNSP_REG(SR) & UNSP_S))
					{
						UNSP_REG(PC) += (OP1 == 0) ? OPIMM : (0 - OPIMM);
					}
					break;
				case 3: // JL
					if(UNSP_REG(SR) & UNSP_S)
					{
						UNSP_REG(PC) += (OP1 == 0) ? OPIMM : (0 - OPIMM);
					}
					break;
				case 4: // JNE
					if(!(UNSP_REG(SR) & UNSP_Z))
					{
						UNSP_REG(PC) += (OP1 == 0) ? OPIMM : (0 - OPIMM);
					}
					break;
				case 5: // JE
					if(UNSP_REG(SR) & UNSP_Z)
					{
						UNSP_REG(PC) += (OP1 == 0) ? OPIMM : (0 - OPIMM);
					}
					break;
				case 6: // JPL
					if(!(UNSP_REG(SR) & UNSP_N))
					{
						UNSP_REG(PC) += (OP1 == 0) ? OPIMM : (0 - OPIMM);
					}
					break;
				case 7: // JMI
					if(UNSP_REG(SR) & UNSP_N)
					{
						UNSP_REG(PC) += (OP1 == 0) ? OPIMM : (0 - OPIMM);
					}
					break;
				case 8: // JBE
					if((UNSP_REG(SR) & (UNSP_Z | UNSP_C)) != UNSP_C)
					{
						UNSP_REG(PC) += (OP1 == 0) ? OPIMM : (0 - OPIMM);
					}
					break;
				case 9: // JA
					if((UNSP_REG(SR) & (UNSP_Z | UNSP_C)) == UNSP_C)
					{
						UNSP_REG(PC) += (OP1 == 0) ? OPIMM : (0 - OPIMM);
					}
					break;
				case 10: // JLE
					if(UNSP_REG(SR) & (UNSP_Z | UNSP_S))
					{
						UNSP_REG(PC) += (OP1 == 0) ? OPIMM : (0 - OPIMM);
					}
					break;
				case 11: // JG
					if(!(UNSP_REG(SR) & (UNSP_Z | UNSP_S)))
					{
						UNSP_REG(PC) += (OP1 == 0) ? OPIMM : (0 - OPIMM);
					}
					break;
				case 14: // JMP
					UNSP_REG(PC) += (OP1 == 0) ? OPIMM : (0 - OPIMM);
					break;
			}
		}
		else
		{
			switch((OP1 << 4) | OP0)
			{
				// r, [bp+imm6]
				case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x06: case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d:
					r0 = UNSP_REG_I(OPA);
					r1 = READ16(UNSP_REG(BP) + OPIMM);
					switch(OP0)
					{
						STANDARD_ALU_CASES;
						case 13: // store r, [bp+imm6]
							WRITE16(UNSP_REG(BP) + OPIMM, UNSP_REG_I(OPA));
							break;
						default:
							unimplemented_opcode(op);
							break;
					}
					WRITEBACK_OPA;
					break;

				// r, imm6
				case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x16: case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c:
					r0 = UNSP_REG_I(OPA);
					r1 = OPIMM;
					switch(OP0)
					{
						STANDARD_ALU_CASES;
						default:
							unimplemented_opcode(op);
							break;
					}
					WRITEBACK_OPA;
					break;

				// Pop / Interrupt return
				case 0x29:
					if(op == 0x9a90) // retf
					{
						UNSP_REG(SR) = unsp_pop(&UNSP_REG(SP));
						UNSP_REG(PC) = unsp_pop(&UNSP_REG(SP));
						break;
					}
					else if(op == 0x9a98) // reti
					{
						int i;
						UNSP_REG(SR) = unsp_pop(&UNSP_REG(SP));
						UNSP_REG(PC) = unsp_pop(&UNSP_REG(SP));
						if(m_fiq & 2)
						{
							m_fiq &= 1;
						}
						else if(m_irq & 2)
						{
							m_irq &= 1;
						}
						m_sirq &= ~(1 << m_curirq);
						for(i = 0; i < 9; i++)
						{
							if((m_sirq & (1 << i)) != 0 && i != m_curirq)
							{
								m_sirq &= ~(1 << i);
								m_curirq = 0;
								execute_set_input(UNSP_IRQ0_LINE + i, 1);
								i = -1;
								break;
							}
						}
						if(i != -1)
						{
							m_curirq = 0;
						}
						break;
					}
					else
					{
						r0 = OPN;
						r1 = OPA;
						while(r0--)
						{
							UNSP_REG_I(++r1) = unsp_pop(&UNSP_REG_I(OPB));
						}
					}
					break;

				// Push
				case 0x2d:
					r0 = OPN;
					r1 = OPA;
					while(r0--)
					{
						unsp_push(UNSP_REG_I(r1--), &UNSP_REG_I(OPB));
					}
					break;

				// ALU, Indirect
				case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x36: case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d:
					switch(OPN & 3)
					{
						case 0: // r, [r]
							r0 = UNSP_REG_I(OPA);
							r1 = READ16((OPN & 4) ? UNSP_LREG_I(OPB) : UNSP_REG_I(OPB));
							switch(OP0)
							{
								STANDARD_ALU_CASES;
								case 13: // store r, [r]
									WRITE16((OPN & 4) ? UNSP_LREG_I(OPB) : UNSP_REG_I(OPB), UNSP_REG_I(OPA));
									break;
								default:
									unimplemented_opcode(op);
									break;
							}
							WRITEBACK_OPA;
							break;
						case 1: // r, [<ds:>r--]
							r0 = UNSP_REG_I(OPA);
							r1 = READ16((OPN & 4) ? UNSP_LREG_I(OPB) : UNSP_REG_I(OPB));
							switch(OP0)
							{
								STANDARD_ALU_CASES;
								case 13: // store r, [<ds:>r--]
									WRITE16((OPN & 4) ? UNSP_LREG_I(OPB) : UNSP_REG_I(OPB), UNSP_REG_I(OPA));
									break;
								default:
									unimplemented_opcode(op);
									break;
							}
							UNSP_REG_I(OPB)--;
							WRITEBACK_OPA;
							break;
						case 2: // r, [<ds:>r++]
							r0 = UNSP_REG_I(OPA);
							r1 = READ16((OPN & 4) ? UNSP_LREG_I(OPB) : UNSP_REG_I(OPB));
							switch(OP0)
							{
								STANDARD_ALU_CASES;
								case 13: // store r, [<ds:>r++]
									WRITE16((OPN & 4) ? UNSP_LREG_I(OPB) : UNSP_REG_I(OPB), UNSP_REG_I(OPA));
									break;
								default:
									unimplemented_opcode(op);
									break;
							}
							UNSP_REG_I(OPB)++;
							WRITEBACK_OPA;
							break;
						case 3: // r, [<ds:>++r]
							UNSP_REG_I(OPB)++;
							r0 = UNSP_REG_I(OPA);
							r1 = READ16((OPN & 4) ? UNSP_LREG_I(OPB) : UNSP_REG_I(OPB));
							switch(OP0)
							{
								STANDARD_ALU_CASES;
								default:
									unimplemented_opcode(op);
									break;
							}
							WRITEBACK_OPA;
							break;
					}
					break;

				// ALU, 16-bit ops
				case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x46: case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c:
					switch(OPN)
					{
						// r, r
						case 0:
							r0 = UNSP_REG_I(OPA);
							r1 = UNSP_REG_I(OPB);
							switch(OP0)
							{
								STANDARD_ALU_CASES;
								default:
									unimplemented_opcode(op);
									break;
							}
							WRITEBACK_OPA;
							break;

						// ALU, 16-bit Immediate
						case 1: // r, r, imm16
							if(!((OP0 == 4 || OP0 == 6 || OP0 == 9 || OP0 == 12) && OPA != OPB))
							{
								r0 = UNSP_REG_I(OPB);
								r1 = READ16(UNSP_LPC);
								UNSP_REG(PC)++;
								switch(OP0)
								{
									STANDARD_ALU_CASES;
									default:
										unimplemented_opcode(op);
										break;
								}
								WRITEBACK_OPA;
							}
							else
							{
								unimplemented_opcode(op);
							}
							break;

						// ALU, Direct 16
						case 2: // r, [imm16]
							r0 = UNSP_REG_I(OPB);
							r1 = READ16(READ16(UNSP_LPC));
							UNSP_REG(PC)++;
							switch(OP0)
							{
								STANDARD_ALU_CASES;
								default:
									unimplemented_opcode(op);
									break;
							}
							WRITEBACK_OPA;
							break;

						// ALU, Direct 16
						case 3: // [imm16], r
							r0 = UNSP_REG_I(OPB);
							r1 = UNSP_REG_I(OPA);
							switch(OP0)
							{
								STANDARD_ALU_CASES;
								default:
									unimplemented_opcode(op);
									break;
							}
							if(OP0 != 4 && OP0 < 12)
							{
								WRITE16(READ16(UNSP_LPC), (UINT16)lres);
							}
							UNSP_REG(PC)++;
							break;

						// ALU, Shifted
						default:
						{
							UINT32 shift = (UNSP_REG_I(OPB) << 4) | m_sb;
							if(shift & 0x80000)
							{
								shift |= 0xf00000;
							}
							shift >>= (OPN - 3);
							m_sb = shift & 0x0f;
							r1 = (shift >> 4) & 0x0000ffff;

							switch(OP0)
							{
								case 9: // load r, r asr n
									unsp_update_nz(r1);
									UNSP_REG_I(OPA) = r1;
									break;
								default:
									unimplemented_opcode(op);
									break;
							}
							break;
						}
					}
					break;

				case 0x4d:
					if(OPN == 3)
					{
						if(OPA == OPB)
						{
							WRITE16(READ16(UNSP_LPC), UNSP_REG_I(OPB));
							UNSP_REG(PC)++;
						}
						else
						{
							unimplemented_opcode(op);
						}
					}
					else
					{
						unimplemented_opcode(op);
					}
					break;

				// ALU, Shifted
				case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x56: case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c:
					if(OPN & 4)
					{
						switch(OP0)
						{
							case 9: // load r, r >> imm2
								lres = ((UNSP_REG_I(OPB) << 4) | m_sb) >> (OPN - 3);
								m_sb = lres & 0x0f;
								unsp_update_nz((UINT16)(lres >> 4));
								UNSP_REG_I(OPA) = (UINT16)(lres >> 4);
								break;
							default:
								unimplemented_opcode(op);
								break;
						}
					}
					else
					{
						UINT32 shift = ((m_sb << 16) | UNSP_REG_I(OPB)) << (OPN + 1);
						m_sb = (shift >> 16) & 0x0f;
						r0 = UNSP_REG_I(OPA);
						r1 = shift & 0x0000ffff;

						switch(OP0)
						{
							case 0: // add r, r << imm2
								lres = r0 + r1;
								unsp_update_nzsc(lres, r0, r1);
								UNSP_REG_I(OPA) = (UINT16)lres;
								break;
							case 9: // load r, r << imm2
								lres = r1;
								unsp_update_nz(lres);
								UNSP_REG_I(OPA) = (UINT16)lres;
								break;
							case 10: // or r, r << imm2
								lres = r0 | r1;
								unsp_update_nz(lres);
								UNSP_REG_I(OPA) = (UINT16)lres;
								break;
							default:
								unimplemented_opcode(op);
								break;
						}
					}
					break;

				// ALU, Rotated
				case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x66: case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c:
					if(OPN & 4) // ROR
					{
						lres = ((((m_sb << 16) | UNSP_REG_I(OPB)) << 4) | m_sb) >> (OPN - 3);
						m_sb = lres & 0x0f;
						r1 = (UINT16)(lres >> 4);
					}
					else
					{
						lres = ((((m_sb << 16) | UNSP_REG_I(OPB)) << 4) | m_sb) << (OPN + 1);
						m_sb = (lres >> 20) & 0x0f;
						r1 = (UINT16)(lres >> 4);
					}

					switch(OP0)
					{
						case 9: // load r, r ror imm2
							unsp_update_nz(r1);
							UNSP_REG_I(OPA) = r1;
							break;
						default:
							unimplemented_opcode(op);
							break;
					}
					break;

				// ALU, Direct 8
				case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x76: case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c:
					//print("%s %s, [%02x]", alu[OP0], reg[OPA], OPIMM);
					unimplemented_opcode(op);
					break;

				// Call
				case 0x1f:
					if(OPA == 0)
					{
						r1 = READ16(UNSP_LPC);
						UNSP_REG(PC)++;
						unsp_push(UNSP_REG(PC), &UNSP_REG(SP));
						unsp_push(UNSP_REG(SR), &UNSP_REG(SP));
						UNSP_REG(PC) = r1;
						UNSP_REG(SR) &= 0xffc0;
						UNSP_REG(SR) |= OPIMM;
					}
					else
					{
						unimplemented_opcode(op);
					}
					break;

				// Far Jump
				case 0x2f: case 0x3f: case 0x6f: case 0x7f:
					if (OPA == 7 && OP1 == 2)
					{
						UNSP_REG(PC) = READ16(UNSP_LPC);
						UNSP_REG(SR) &= 0xffc0;
						UNSP_REG(SR) |= OPIMM;
					}
					break;

				// Multiply, Unsigned * Signed
				case 0x0f:
					if(OPN == 1 && OPA != 7)
					{
						lres = UNSP_REG_I(OPA) * UNSP_REG_I(OPB);
						if(UNSP_REG_I(OPB) & 0x8000)
						{
							lres -= UNSP_REG_I(OPA) << 16;
						}
						UNSP_REG(R4) = lres >> 16;
						UNSP_REG(R3) = (UINT16)lres;
						break;
					}
					else
					{
						unimplemented_opcode(op);
					}
					break;

				// Multiply, Signed * Signed
				case 0x4f:
					if(OPN == 1 && OPA != 7)
					{
						lres = UNSP_REG_I(OPA) * UNSP_REG_I(OPB);
						if(UNSP_REG_I(OPB) & 0x8000)
						{
							lres -= UNSP_REG_I(OPA) << 16;
						}
						if(UNSP_REG_I(OPA) & 0x8000)
						{
							lres -= UNSP_REG_I(OPB) << 16;
						}
						UNSP_REG(R4) = lres >> 16;
						UNSP_REG(R3) = (UINT16)lres;
						break;
					}
					else
					{
						unimplemented_opcode(op);
					}
					break;

				// Interrupt flags
				case 0x5f:
					if(OPA == 0)
					{
						switch(OPIMM)
						{
							case 0:
								m_irq &= ~1;
								m_fiq &= ~1;
								break;
							case 1:
								m_irq |=  1;
								m_fiq &= ~1;
								break;
							case 2:
								m_irq &= ~1;
								m_fiq |=  1;
								break;
							case 3:
								m_irq |=  1;
								m_fiq |=  1;
								break;
							case 8: // irq off
								m_irq &= ~1;
								break;
							case 9: // irq on
								m_irq |= 1;
								break;
							case 12: // fiq off
								m_fiq &= ~1;
								break;
							case 13: // fiq on
								m_fiq |= 1;
								break;
							case 37: // nop
								break;
						}
					}
					else
					{
						unimplemented_opcode(op);
					}
					break;
			}
		}

		m_icount -= 5;
		m_icount = MAX(m_icount, 0);
	}
}


/*****************************************************************************/

void unsp_device::execute_set_input(int irqline, int state)
{
	UINT16 irq_vector = 0;

	m_sirq &= ~(1 << irqline);

	if(!state)
	{
		logerror("clearing irq %d (%04x, %04x)\n", irqline, m_sirq, m_curirq);
		return;
	}

	switch (irqline)
	{
		case UNSP_IRQ0_LINE:
		case UNSP_IRQ1_LINE:
		case UNSP_IRQ2_LINE:
		case UNSP_IRQ3_LINE:
		case UNSP_IRQ4_LINE:
		case UNSP_IRQ5_LINE:
		case UNSP_IRQ6_LINE:
		case UNSP_IRQ7_LINE:
			if(m_fiq & 2)
			{
				// FIQ is being serviced, ignore this IRQ trigger.
				m_sirq |= state << irqline;
				return;
			}
			if(m_irq != 1)
			{
				// IRQ is disabled, ignore this IRQ trigger.
				m_sirq |= state << irqline;
				return;
			}
			m_irq |= 2;
			m_curirq |= (1 << irqline);
			logerror("taking irq %d (%04x, %04x)\n", irqline, m_sirq, m_curirq);
			irq_vector = 0xfff8 + (irqline - UNSP_IRQ0_LINE);
			break;
		case UNSP_FIQ_LINE:
			if(m_fiq != 1)
			{
				// FIQ is disabled, ignore this FIQ trigger.
				m_sirq |= state << irqline;
				return;
			}
			m_fiq |= 2;
			m_curirq |= (1 << irqline);
			logerror("taking fiq %d (%04x, %04x)\n", irqline, m_sirq, m_curirq);
			irq_vector = 0xfff6;
			break;
		case UNSP_BRK_LINE:
			break;
	}

	m_saved_sb = m_sb;
	unsp_push(UNSP_REG(PC), &UNSP_REG(SP));
	unsp_push(UNSP_REG(SR), &UNSP_REG(SP));
	UNSP_REG(PC) = READ16(irq_vector);
	UNSP_REG(SR) = 0;
}
