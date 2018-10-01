// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*****************************************************************************

	SunPlus micro'nSP core

	based primarily on Unununium, by segher

*****************************************************************************/

#include "emu.h"
#include "unsp.h"
#include "unspdasm.h"
#include "debugger.h"


DEFINE_DEVICE_TYPE(UNSP, unsp_device, "unsp", "SunPlus u'nSP")


unsp_device::unsp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cpu_device(mconfig, UNSP, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_BIG, 16, 23, -1)
	, m_enable_irq(false)
	, m_enable_fiq(false)
	, m_irq(false)
	, m_fiq(false)
	, m_curirq(0)
	, m_sirq(0)
	, m_sb(0)
	, m_program(nullptr)
	, m_icount(0)
	, m_debugger_temp(0)
{
}

device_memory_interface::space_config_vector unsp_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config)
	};
}

std::unique_ptr<util::disasm_interface> unsp_device::create_disassembler()
{
	return std::make_unique<unsp_disassembler>();
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

/*****************************************************************************/

void unsp_device::unimplemented_opcode(uint16_t op)
{
	fatalerror("UNSP: unknown opcode %04x at %04x\n", op, UNSP_LPC);
}

/*****************************************************************************/

uint16_t unsp_device::read16(uint32_t address)
{
	return m_program->read_word(address);
}

void unsp_device::write16(uint32_t address, uint16_t data)
{
	m_program->write_word(address, data);
}

/*****************************************************************************/

void unsp_device::device_start()
{
	memset(m_r, 0, sizeof(uint16_t) * 16);
	m_enable_irq = false;
	m_enable_fiq = false;
	m_irq = false;
	m_fiq = false;
	m_curirq = 0;
	m_sirq = 0;
	m_sb = 0;
	memset(m_saved_sb, 0, sizeof(uint8_t) * 3);
	m_debugger_temp = 0;

	m_program = &space(AS_PROGRAM);

	state_add( UNSP_SP,     "SP", UNSP_REG(SP)).formatstr("%04X");
	state_add( UNSP_R1,     "R1", UNSP_REG(R1)).formatstr("%04X");
	state_add( UNSP_R2,     "R2", UNSP_REG(R2)).formatstr("%04X");
	state_add( UNSP_R3,     "R3", UNSP_REG(R3)).formatstr("%04X");
	state_add( UNSP_R4,     "R4", UNSP_REG(R4)).formatstr("%04X");
	state_add( UNSP_BP,     "BP", UNSP_REG(BP)).formatstr("%04X");
	state_add( UNSP_SR,     "SR", UNSP_REG(SR)).formatstr("%04X");
	state_add( UNSP_PC,     "PC", m_debugger_temp).callimport().callexport().formatstr("%06X");
	state_add( UNSP_IRQ_EN, "IRQE", m_enable_irq).formatstr("%1u");
	state_add( UNSP_FIQ_EN, "FIQE", m_enable_fiq).formatstr("%1u");
	state_add( UNSP_IRQ,    "IRQ", m_irq).formatstr("%1u");
	state_add( UNSP_FIQ,    "FIQ", m_fiq).formatstr("%1u");
	state_add( UNSP_SB,     "SB", m_sb).formatstr("%1u");

	state_add(STATE_GENPC, "GENPC", m_debugger_temp).callexport().noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_debugger_temp).callexport().noshow();

	set_icountptr(m_icount);
}

void unsp_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case STATE_GENPC:
		case STATE_GENPCBASE:
		case UNSP_PC:
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
	memset(m_r, 0, sizeof(uint16_t) * 16);

	UNSP_REG(PC) = read16(0xfff7);
	m_enable_irq = false;
	m_enable_fiq = false;
	m_irq = false;
	m_fiq = false;
}

/*****************************************************************************/

void unsp_device::update_nz(uint32_t value)
{
	UNSP_REG(SR) &= ~(UNSP_N | UNSP_Z);
	if(value & 0x8000)
	{
		UNSP_REG(SR) |= UNSP_N;
	}
	if((uint16_t)value == 0)
	{
		UNSP_REG(SR) |= UNSP_Z;
	}
}

void unsp_device::update_sc(uint32_t value, uint16_t r0, uint16_t r1)
{
	UNSP_REG(SR) &= ~(UNSP_C | UNSP_S);
	if(value != (uint16_t)value)
	{
		UNSP_REG(SR) |= UNSP_C;
	}

	if((int16_t)r0 < (int16_t)r1)
	{
		UNSP_REG(SR) |= UNSP_S;
	}
}

void unsp_device::push(uint16_t value, uint16_t *reg)
{
	write16((*reg)--, value);
}

uint16_t unsp_device::pop(uint16_t *reg)
{
	return read16(++(*reg));
}

void unsp_device::trigger_fiq()
{
	if (!m_enable_fiq || m_fiq)
		return;

	m_fiq = true;

	m_saved_sb[m_irq ? 1 : 0] = m_sb;
	m_sb = m_saved_sb[2];

	push(UNSP_REG(PC), &UNSP_REG(SP));
	push(UNSP_REG(SR), &UNSP_REG(SP));
	UNSP_REG(PC) = read16(0xfff6);
	UNSP_REG(SR) = 0;
}

void unsp_device::trigger_irq(int line)
{
	if (!m_enable_irq || m_irq)
		return;

	m_irq = true;

	m_saved_sb[0] = m_sb;
	m_sb = m_saved_sb[1];

	push(UNSP_REG(PC), &UNSP_REG(SP));
	push(UNSP_REG(SR), &UNSP_REG(SP));
	UNSP_REG(PC) = read16(0xfff8 + line);
	UNSP_REG(SR) = 0;
}

void unsp_device::check_irqs()
{
	int highest_irq = -1;
	for (int i = 0; i <= 8; i++)
	{
		if (BIT(m_sirq, i))
		{
			highest_irq = i;
			break;
		}
	}

	if (highest_irq < 0)
		return;

	if (highest_irq == UNSP_FIQ_LINE)
	{
		trigger_fiq();
	}
	else
	{
		trigger_irq(highest_irq - 1);
	}
}

void unsp_device::execute_run()
{
	uint32_t lres = 0;
	uint16_t r0 = 0;
	uint16_t r1 = 0;
	uint32_t r2 = 0;

	while (m_icount > 0)
	{
		check_irqs();

		debugger_instruction_hook(UNSP_LPC);
		const uint32_t op = read16(UNSP_LPC);

		UNSP_REG(PC)++;

		const uint8_t lower_op = (OP1 << 4) | OP0;

		if(OP0 < 0xf && OPA == 0x7 && OP1 < 2)
		{
			switch(OP0)
			{
				case 0: // JB
					if(!(UNSP_REG(SR) & UNSP_C))
					{
						m_icount -= 4;
						UNSP_REG(PC) += (OP1 == 0) ? OPIMM : (0 - OPIMM);
					}
					else
						m_icount -= 2;
					continue;
				case 1: // JAE
					if(UNSP_REG(SR) & UNSP_C)
					{
						m_icount -= 4;
						UNSP_REG(PC) += (OP1 == 0) ? OPIMM : (0 - OPIMM);
					}
					else
						m_icount -= 2;
					continue;
				case 2: // JGE
					if(!(UNSP_REG(SR) & UNSP_S))
					{
						m_icount -= 4;
						UNSP_REG(PC) += (OP1 == 0) ? OPIMM : (0 - OPIMM);
					}
					else
						m_icount -= 2;
					continue;
				case 3: // JL
					if(UNSP_REG(SR) & UNSP_S)
					{
						m_icount -= 4;
						UNSP_REG(PC) += (OP1 == 0) ? OPIMM : (0 - OPIMM);
					}
					else
						m_icount -= 2;
					continue;
				case 4: // JNE
					if(!(UNSP_REG(SR) & UNSP_Z))
					{
						m_icount -= 4;
						UNSP_REG(PC) += (OP1 == 0) ? OPIMM : (0 - OPIMM);
					}
					else
						m_icount -= 2;
					continue;
				case 5: // JE
					if(UNSP_REG(SR) & UNSP_Z)
					{
						m_icount -= 4;
						UNSP_REG(PC) += (OP1 == 0) ? OPIMM : (0 - OPIMM);
					}
					else
						m_icount -= 2;
					continue;
				case 6: // JPL
					if(!(UNSP_REG(SR) & UNSP_N))
					{
						m_icount -= 4;
						UNSP_REG(PC) += (OP1 == 0) ? OPIMM : (0 - OPIMM);
					}
					else
						m_icount -= 2;
					continue;
				case 7: // JMI
					if(UNSP_REG(SR) & UNSP_N)
					{
						m_icount -= 4;
						UNSP_REG(PC) += (OP1 == 0) ? OPIMM : (0 - OPIMM);
					}
					else
						m_icount -= 2;
					continue;
				case 8: // JBE
					if((UNSP_REG(SR) & (UNSP_Z | UNSP_C)) != UNSP_C)
					{
						m_icount -= 4;
						UNSP_REG(PC) += (OP1 == 0) ? OPIMM : (0 - OPIMM);
					}
					else
						m_icount -= 2;
					continue;
				case 9: // JA
					if((UNSP_REG(SR) & (UNSP_Z | UNSP_C)) == UNSP_C)
					{
						m_icount -= 4;
						UNSP_REG(PC) += (OP1 == 0) ? OPIMM : (0 - OPIMM);
					}
					else
						m_icount -= 2;
					continue;
				case 10: // JLE
					if(UNSP_REG(SR) & (UNSP_Z | UNSP_S))
					{
						m_icount -= 4;
						UNSP_REG(PC) += (OP1 == 0) ? OPIMM : (0 - OPIMM);
					}
					else
						m_icount -= 2;
					continue;
				case 11: // JG
					if(!(UNSP_REG(SR) & (UNSP_Z | UNSP_S)))
					{
						m_icount -= 4;
						UNSP_REG(PC) += (OP1 == 0) ? OPIMM : (0 - OPIMM);
					}
					else
						m_icount -= 2;
					continue;
				case 14: // JMP
					UNSP_REG(PC) += (OP1 == 0) ? OPIMM : (0 - OPIMM);
					m_icount -= 4;
					continue;
				default:
					unimplemented_opcode(op);
					continue;
			}
		}
		else if (lower_op == 0x2d) // Push
		{
			r0 = OPN;
			r1 = OPA;
			m_icount -= 4 + 2 * r0;

			while (r0--)
			{
				push(UNSP_REG_I(r1--), &UNSP_REG_I(OPB));
			}
			continue;
		}
		else if (lower_op == 0x29) // Pop
		{
			if (op == 0x9a98) // reti
			{
				m_icount -= 8;
				UNSP_REG(SR) = pop(&UNSP_REG(SP));
				UNSP_REG(PC) = pop(&UNSP_REG(SP));

				if(m_fiq)
				{
					m_fiq = false;
					m_saved_sb[2] = m_sb;
					m_sb = m_saved_sb[m_irq ? 1 : 0];
				}
				else if(m_irq)
				{
					m_irq = false;
					m_saved_sb[1] = m_sb;
					m_sb = m_saved_sb[0];
				}

				check_irqs();
				continue;
			}
			else
			{
				r0 = OPN;
				r1 = OPA;
				m_icount -= 4 + 2 * r0;

				while (r0--)
				{
					UNSP_REG_I(++r1) = pop(&UNSP_REG_I(OPB));
				}
				continue;
			}
		}
		else if (OP0 == 0xf)
		{
			switch (OP1)
			{
				case 0x00: // Multiply, Unsigned * Signed
					if(OPN == 1 && OPA != 7)
					{
						m_icount -= 12;
						lres = UNSP_REG_I(OPA) * UNSP_REG_I(OPB);
						if(UNSP_REG_I(OPB) & 0x8000)
						{
							lres -= UNSP_REG_I(OPA) << 16;
						}
						UNSP_REG(R4) = lres >> 16;
						UNSP_REG(R3) = (uint16_t)lres;
					}
					else
					{
						unimplemented_opcode(op);
					}
					continue;

				case 0x01: // Call
					if(!(OPA & 1))
					{
						m_icount -= 9;
						r1 = read16(UNSP_LPC);
						UNSP_REG(PC)++;
						push(UNSP_REG(PC), &UNSP_REG(SP));
						push(UNSP_REG(SR), &UNSP_REG(SP));
						UNSP_REG(PC) = r1;
						UNSP_REG(SR) &= 0xffc0;
						UNSP_REG(SR) |= OPIMM;
					}
					else
					{
						unimplemented_opcode(op);
					}
					continue;

				case 0x02: // Far Jump
					if (OPA == 7)
					{
						m_icount -= 5;
						UNSP_REG(PC) = read16(UNSP_LPC);
						UNSP_REG(PC)++;
						UNSP_REG(SR) &= 0xffc0;
						UNSP_REG(SR) |= OPIMM;
					}
					else
					{
						unimplemented_opcode(op);
					}
					continue;

				case 0x04: // Multiply, Signed * Signed
					if(OPN == 1 && OPA != 7)
					{
						m_icount -= 12;
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
						UNSP_REG(R3) = (uint16_t)lres;
					}
					else
					{
						unimplemented_opcode(op);
					}
					continue;

				case 0x05: // Interrupt flags
					m_icount -= 2;
					switch(OPIMM)
					{
						case 0:
							m_enable_irq = false;
							m_enable_fiq = false;
							break;
						case 1:
							m_enable_irq = true;
							m_enable_fiq = false;
							break;
						case 2:
							m_enable_irq = false;
							m_enable_fiq = true;
							break;
						case 3:
							m_enable_irq = true;
							m_enable_fiq = true;
							break;
						case 8: // irq off
							m_enable_irq = false;
							break;
						case 9: // irq on
							m_enable_irq = true;
							break;
						case 12: // fiq off
							m_enable_fiq = false;
							break;
						case 14: // fiq on
							m_enable_fiq = true;
							break;
						case 37: // nop
							break;
						default:
							unimplemented_opcode(op);
							break;
					}
					continue;

				default:
					unimplemented_opcode(op);
					continue;
			}
		}

		// At this point, we should be dealing solely with ALU ops.

		r0 = UNSP_REG_I(OPA);

		switch (OP1)
		{
			case 0x00: // r, [bp+imm6]
				m_icount -= 6;

				r2 = UNSP_REG(BP) + OPIMM;
				if (OP0 != 0x0d)
					r1 = read16(r2);
				break;

			case 0x01: // r, imm6
				m_icount -= 2;

				r1 = OPIMM;
				break;

			case 0x03: // Indirect
			{
				m_icount -= 6;
				if (OPA == 7)
					m_icount -= 1;

				const uint8_t lsbits = OPN & 3;
				const uint8_t opb = OPB;
				if (OPN & 4)
				{
					switch (lsbits)
					{
						case 0: // r, [r]
							r2 = UNSP_LREG_I(opb);
							if (OP0 != 0x0d)
								r1 = read16(r2);
							break;
						case 1: // r, [<ds:>r--]
							r2 = UNSP_LREG_I(opb);
							if (OP0 != 0x0d)
								r1 = read16(r2);
							UNSP_REG_I(opb)--;
							if (UNSP_REG_I(opb) == 0xffff)
								UNSP_REG(SR) -= 0x0400;
							break;
						case 2: // r, [<ds:>r++]
							r2 = UNSP_LREG_I(opb);
							if (OP0 != 0x0d)
								r1 = read16(r2);
							UNSP_REG_I(opb)++;
							if (UNSP_REG_I(opb) == 0x0000)
								UNSP_REG(SR) += 0x0400;
							break;
						case 3: // r, [<ds:>++r]
							UNSP_REG_I(opb)++;
							if (UNSP_REG_I(opb) == 0x0000)
								UNSP_REG(SR) += 0x0400;
							r2 = UNSP_LREG_I(opb);
							if (OP0 != 0x0d)
								r1 = read16(r2);
							break;
						default:
							break;
					}
				}
				else
				{
					switch (lsbits)
					{
						case 0: // r, [r]
							r2 = UNSP_REG_I(opb);
							if (OP0 != 0x0d)
								r1 = read16(r2);
							break;
						case 1: // r, [<ds:>r--]
							r2 = UNSP_REG_I(opb);
							if (OP0 != 0x0d)
								r1 = read16(r2);
							UNSP_REG_I(opb)--;
							break;
						case 2: // r, [<ds:>r++]
							r2 = UNSP_REG_I(opb);
							if (OP0 != 0x0d)
								r1 = read16(r2);
							UNSP_REG_I(opb)++;
							break;
						case 3: // r, [<ds:>++r]
							UNSP_REG_I(opb)++;
							r2 = UNSP_REG_I(opb);
							if (OP0 != 0x0d)
								r1 = read16(r2);
							break;
						default:
							break;
					}
				}
				break;
			}

			case 0x04: // 16-bit ops
				switch (OPN)
				{
					case 0x00: // r
						m_icount -= 3;
						if (OPA == 7)
							m_icount -= 2;

						r1 = UNSP_REG_I(OPB);
						break;

					case 0x01: // imm16
						m_icount -= 4;
						if (OPA == 7)
							m_icount -= 1;

						r0 = UNSP_REG_I(OPB);
						r1 = read16(UNSP_LPC);
						UNSP_REG(PC)++;
						break;

					case 0x02: // [imm16]
						m_icount -= 7;
						if (OPA == 7)
							m_icount -= 1;

						r0 = UNSP_REG_I(OPB);
						r2 = read16(UNSP_LPC);
						UNSP_REG(PC)++;

						if (OP0 != 0x0d)
						{
							r1 = read16(r2);
						}
						break;

					case 0x03: // store [imm16], r
						m_icount -= 7;
						if (OPA == 7)
							m_icount -= 1;

						r1 = r0;
						r0 = UNSP_REG_I(OPB);
						r2 = read16(UNSP_LPC);
						UNSP_REG(PC)++;
						break;

					default: // Shifted ops
					{
						m_icount -= 3;
						if (OPA == 7)
							m_icount -= 2;

						uint32_t shift = (UNSP_REG_I(OPB) << 4) | m_sb;
						if (shift & 0x80000)
							shift |= 0xf00000;
						shift >>= (OPN - 3);
						m_sb = shift & 0x0f;
						r1 = (uint16_t)(shift >> 4);
						break;
					}
				}
				break;

			case 0x05: // More shifted ops
				m_icount -= 3;
				if (OPA == 7)
					m_icount -= 2;

				if (OPN & 4) // Shift right
				{
					const uint32_t shift = ((UNSP_REG_I(OPB) << 4) | m_sb) >> (OPN - 3);
					m_sb = shift & 0x0f;
					r1 = (uint16_t)(shift >> 4);
				}
				else // Shift left
				{
					const uint32_t shift = ((m_sb << 16) | UNSP_REG_I(OPB)) << (OPN + 1);
					m_sb = (shift >> 16) & 0x0f;
					r1 = (uint16_t)shift;
				}
				break;

			case 0x06: // Rotated ops
			{
				m_icount -= 3;
				if (OPA == 7)
					m_icount -= 2;

				uint32_t shift = (((m_sb << 16) | UNSP_REG_I(OPB)) << 4) | m_sb;
				if (OPN & 4) // Rotate right
				{
					shift >>= (OPN - 3);
					m_sb = shift & 0x0f;
				}
				else
				{
					shift <<= (OPN + 1);
					m_sb = (shift >> 20) & 0x0f;
				}
				r1 = (uint16_t)(shift >> 4);
				break;
			}

			case 0x07: // Direct 8
				m_icount -= 5;
				if (OPA == 7)
					m_icount -= 1;

				r1 = read16(OPIMM);
				break;

			default:
				break;
		}

		switch (OP0)
		{
			case 0x00: // Add
				lres = r0 + r1;
				break;
			case 0x01: // Add w/ carry
				lres = r0 + r1;
				if(UNSP_REG(SR) & UNSP_C)
					lres++;
				break;
			case 0x02: // Subtract
			case 0x04: // Compare
				lres = r0 + (uint16_t)(~r1) + 1;
				break;
			case 0x03: // Subtract w/ carry
				lres = r0 + (uint16_t)(~r1);
				if(UNSP_REG(SR) & UNSP_C)
					lres++;
				break;
			case 0x06: // Negate
				lres = -r1;
				break;
			case 0x08: // XOR
				lres = r0 ^ r1;
				break;
			case 0x09: // Load
				lres = r1;
				break;
			case 0x0a: // OR
				lres = r0 | r1;
				break;
			case 0x0b: // AND
			case 0x0c: // Test
				lres = r0 & r1;
				break;
			case 0x0d: // Store
				write16(r2, r0);
				continue;
			default:
				unimplemented_opcode(op);
				continue;
		}

		if (OP0 != 0x0d && OPA != 0x07) // If not a store opcode and not updating the PC, update negative/zero flags.
			update_nz(lres);

		if (OP0 < 0x05 && OPA != 0x07) // If Add, Add w/ Carry, Subtract, Subtract w/ Carry, Compare, and not updating the PC, update sign/carry flags.
			update_sc(lres, r0, r1);

		if (OP0 == 0x04 || OP0 == 0x0c) // Compare and Test don't write back results.
			continue;

		if (OP1 == 0x04 && OPN == 0x03) // store [imm16], r
			write16(r2, lres);
		else
			UNSP_REG_I(OPA) = lres;
	}
}


/*****************************************************************************/

void unsp_device::execute_set_input(int irqline, int state)
{
	m_sirq &= ~(1 << irqline);

	if(!state)
	{
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
		case UNSP_FIQ_LINE:
			m_sirq |= (1 << irqline);
			break;
		case UNSP_BRK_LINE:
			break;
	}
}
