// license:GPL-2.0+
// copyright-holders:Segher Boessenkool,Ryan Holtz
/*****************************************************************************

    SunPlus µ'nSP emulator

    Copyright 2008-2017  Segher Boessenkool  <segher@kernel.crashing.org>
    Licensed under the terms of the GNU GPL, version 2
    http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

    Ported to MAME framework by Ryan Holtz

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
#if UNSP_LOG_OPCODES
	, m_log_ops(0)
#endif
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

#define UNSP_LPC            (((m_r[REG_SR] & 0x3f) << 16) | m_r[REG_PC])
#define UNSP_LREG_I(reg)    (((m_r[REG_SR] << 6) & 0x3f0000) | m_r[reg])

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

	state_add(STATE_GENFLAGS, "GENFLAGS", m_r[REG_SR]).callimport().callexport().formatstr("%4s").noshow();
	state_add(UNSP_SP,     "SP", m_r[REG_SP]).formatstr("%04X");
	state_add(UNSP_R1,     "R1", m_r[REG_R1]).formatstr("%04X");
	state_add(UNSP_R2,     "R2", m_r[REG_R2]).formatstr("%04X");
	state_add(UNSP_R3,     "R3", m_r[REG_R3]).formatstr("%04X");
	state_add(UNSP_R4,     "R4", m_r[REG_R4]).formatstr("%04X");
	state_add(UNSP_BP,     "BP", m_r[REG_BP]).formatstr("%04X");
	state_add(UNSP_SR,     "SR", m_r[REG_SR]).formatstr("%04X");
	state_add(UNSP_PC,     "PC", m_debugger_temp).callimport().callexport().formatstr("%06X");
	state_add(UNSP_IRQ_EN, "IRQE", m_enable_irq).formatstr("%1u");
	state_add(UNSP_FIQ_EN, "FIQE", m_enable_fiq).formatstr("%1u");
	state_add(UNSP_IRQ,    "IRQ", m_irq).formatstr("%1u");
	state_add(UNSP_FIQ,    "FIQ", m_fiq).formatstr("%1u");
	state_add(UNSP_SB,     "SB", m_sb).formatstr("%1u");
#if UNSP_LOG_OPCODES
	state_add(UNSP_LOG_OPS,"LOG", m_log_ops).formatstr("%1u");
#endif

	state_add(STATE_GENPC, "GENPC", m_debugger_temp).callexport().noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_debugger_temp).callexport().noshow();

	set_icountptr(m_icount);

	save_item(NAME(m_r));
	save_item(NAME(m_enable_irq));
	save_item(NAME(m_enable_fiq));
	save_item(NAME(m_irq));
	save_item(NAME(m_fiq));
	save_item(NAME(m_curirq));
	save_item(NAME(m_sirq));
	save_item(NAME(m_sb));
	save_item(NAME(m_saved_sb));
}

void unsp_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
		{
			const uint16_t sr = m_r[REG_SR];
			str = string_format("%c%c%c%c", (sr & UNSP_N) ? 'N' : ' ', (sr & UNSP_Z) ? 'Z' : ' ', (sr & UNSP_S) ? 'S' : ' ', (sr & UNSP_C) ? 'C' : ' ');
		}
	}
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
			m_r[REG_PC] = m_debugger_temp & 0x0000ffff;
			m_r[REG_SR] = (m_r[REG_SR] & 0xffc0) | ((m_debugger_temp & 0x003f0000) >> 16);
			break;
	}
}

void unsp_device::device_reset()
{
	memset(m_r, 0, sizeof(uint16_t) * 16);

	m_r[REG_PC] = read16(0xfff7);
	m_enable_irq = false;
	m_enable_fiq = false;
	m_irq = false;
	m_fiq = false;
}

/*****************************************************************************/

void unsp_device::update_nzsc(uint32_t value, uint16_t r0, uint16_t r1)
{
	m_r[REG_SR] &= ~(UNSP_N | UNSP_Z | UNSP_S | UNSP_C);
	if(value & 0x8000)
		m_r[REG_SR] |= UNSP_N;
	if((uint16_t)value == 0)
		m_r[REG_SR] |= UNSP_Z;
	if(value != (uint16_t)value)
		m_r[REG_SR] |= UNSP_C;
	if((int16_t)r0 < (int16_t)r1)
		m_r[REG_SR] |= UNSP_S;
}

void unsp_device::update_nz(uint32_t value)
{
	m_r[REG_SR] &= ~(UNSP_N | UNSP_Z);
	if(value & 0x8000)
		m_r[REG_SR] |= UNSP_N;
	if((uint16_t)value == 0)
		m_r[REG_SR] |= UNSP_Z;
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
	if (!m_enable_fiq || m_fiq || m_irq)
	{
		return;
	}

	m_fiq = true;

	m_saved_sb[m_irq ? 1 : 0] = m_sb;
	m_sb = m_saved_sb[2];

	push(m_r[REG_PC], &m_r[REG_SP]);
	push(m_r[REG_SR], &m_r[REG_SP]);
	m_r[REG_PC] = read16(0xfff6);
	m_r[REG_SR] = 0;
}

void unsp_device::trigger_irq(int line)
{
	if (!m_enable_irq || m_irq || m_fiq)
		return;

	m_irq = true;

	m_saved_sb[0] = m_sb;
	m_sb = m_saved_sb[1];

	push(m_r[REG_PC], &m_r[REG_SP]);
	push(m_r[REG_SR], &m_r[REG_SP]);
	m_r[REG_PC] = read16(0xfff8 + line);
	m_r[REG_SR] = 0;
}

void unsp_device::check_irqs()
{
	if (!m_sirq)
		return;

	int highest_irq = -1;
	for (int i = 0; i <= 8; i++)
	{
		if (BIT(m_sirq, i))
		{
			highest_irq = i;
			break;
		}
	}

	if (highest_irq == UNSP_FIQ_LINE)
		trigger_fiq();
	else
		trigger_irq(highest_irq - 1);
}

void unsp_device::add_lpc(const int32_t offset)
{
	const uint32_t new_lpc = UNSP_LPC + offset;
	m_r[REG_PC] = (uint16_t)new_lpc;
	m_r[REG_SR] &= 0xffc0;
	m_r[REG_SR] |= (new_lpc >> 16) & 0x3f;
}

inline void unsp_device::execute_one(const uint16_t op)
{
	uint32_t lres = 0;
	uint16_t r0 = 0;
	uint16_t r1 = 0;
	uint32_t r2 = 0;

	const uint16_t op0 = (op >> 12) & 15;
	const uint16_t opa = (op >> 9) & 7;
	const uint16_t op1 = (op >> 6) & 7;
	const uint16_t opn = (op >> 3) & 7;
	const uint16_t opb = op & 7;

	const uint8_t lower_op = (op1 << 4) | op0;

	if(op0 < 0xf && opa == 0x7 && op1 < 2)
	{
		const uint32_t opimm = op & 0x3f;
		switch(op0)
		{
			case 0: // JB
				if(!(m_r[REG_SR] & UNSP_C))
				{
					m_icount -= 4;
					add_lpc((op1 == 0) ? opimm : (0 - opimm));
				}
				else
				{
					m_icount -= 2;
				}
				return;
			case 1: // JAE
				if(m_r[REG_SR] & UNSP_C)
				{
					m_icount -= 4;
					add_lpc((op1 == 0) ? opimm : (0 - opimm));
				}
				else
				{
					m_icount -= 2;
				}
				return;
			case 2: // JGE
				if(!(m_r[REG_SR] & UNSP_S))
				{
					m_icount -= 4;
					add_lpc((op1 == 0) ? opimm : (0 - opimm));
				}
				else
				{
					m_icount -= 2;
				}
				return;
			case 3: // JL
				if(m_r[REG_SR] & UNSP_S)
				{
					m_icount -= 4;
					add_lpc((op1 == 0) ? opimm : (0 - opimm));
				}
				else
				{
					m_icount -= 2;
				}
				return;
			case 4: // JNE
				if(!(m_r[REG_SR] & UNSP_Z))
				{
					m_icount -= 4;
					add_lpc((op1 == 0) ? opimm : (0 - opimm));
				}
				else
				{
					m_icount -= 2;
				}
				return;
			case 5: // JE
				if(m_r[REG_SR] & UNSP_Z)
				{
					m_icount -= 4;
					add_lpc((op1 == 0) ? opimm : (0 - opimm));
				}
				else
				{
					m_icount -= 2;
				}
				return;
			case 6: // JPL
				if(!(m_r[REG_SR] & UNSP_N))
				{
					m_icount -= 4;
					add_lpc((op1 == 0) ? opimm : (0 - opimm));
				}
				else
				{
					m_icount -= 2;
				}
				return;
			case 7: // JMI
				if(m_r[REG_SR] & UNSP_N)
				{
					m_icount -= 4;
					add_lpc((op1 == 0) ? opimm : (0 - opimm));
				}
				else
				{
					m_icount -= 2;
				}
				return;
			case 8: // JBE
				if((m_r[REG_SR] & (UNSP_Z | UNSP_C)) != UNSP_C)
				{
					m_icount -= 4;
					add_lpc((op1 == 0) ? opimm : (0 - opimm));
				}
				else
				{
					m_icount -= 2;
				}
				return;
			case 9: // JA
				if((m_r[REG_SR] & (UNSP_Z | UNSP_C)) == UNSP_C)
				{
					m_icount -= 4;
					add_lpc((op1 == 0) ? opimm : (0 - opimm));
				}
				else
				{
					m_icount -= 2;
				}
				return;
			case 10: // JLE
				if(m_r[REG_SR] & (UNSP_Z | UNSP_S))
				{
					m_icount -= 4;
					add_lpc((op1 == 0) ? opimm : (0 - opimm));
				}
				else
				{
					m_icount -= 2;
				}
				return;
			case 11: // JG
				if(!(m_r[REG_SR] & (UNSP_Z | UNSP_S)))
				{
					m_icount -= 4;
					add_lpc((op1 == 0) ? opimm : (0 - opimm));
				}
				else
				{
					m_icount -= 2;
				}
				return;
			case 14: // JMP
				add_lpc((op1 == 0) ? opimm : (0 - opimm));
				m_icount -= 4;
				return;
			default:
				unimplemented_opcode(op);
				return;
		}
	}
	else if (lower_op == 0x2d) // Push
	{
		r0 = opn;
		r1 = opa;
		m_icount -= 4 + 2 * r0;

		while (r0--)
		{
			push(m_r[r1--], &m_r[opb]);
		}
		return;
	}
	else if (lower_op == 0x29) // Pop
	{
		if (op == 0x9a98) // reti
		{
			m_icount -= 8;
			m_r[REG_SR] = pop(&m_r[REG_SP]);
			m_r[REG_PC] = pop(&m_r[REG_SP]);

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
			m_curirq = 0;

			check_irqs();
			return;
		}
		else
		{
			r0 = opn;
			r1 = opa;
			m_icount -= 4 + 2 * r0;

			while (r0--)
			{
				m_r[++r1] = pop(&m_r[opb]);
			}
			return;
		}
	}
	else if (op0 == 0xf)
	{
		switch (op1)
		{
			case 0x00: // Multiply, Unsigned * Signed
				if(opn == 1 && opa != 7)
				{
					m_icount -= 12;
					lres = m_r[opa] * m_r[opb];
					if(m_r[opb] & 0x8000)
					{
						lres -= m_r[opa] << 16;
					}
					m_r[REG_R4] = lres >> 16;
					m_r[REG_R3] = (uint16_t)lres;
				}
				else
				{
					unimplemented_opcode(op);
				}
				return;

			case 0x01: // Call
				if(!(opa & 1))
				{
					m_icount -= 9;
					r1 = read16(UNSP_LPC);
					add_lpc(1);
					push(m_r[REG_PC], &m_r[REG_SP]);
					push(m_r[REG_SR], &m_r[REG_SP]);
					m_r[REG_PC] = r1;
					m_r[REG_SR] &= 0xffc0;
					m_r[REG_SR] |= op & 0x3f;
				}
				else
				{
					unimplemented_opcode(op);
				}
				return;

			case 0x02: // Far Jump
				if (opa == 7)
				{
					m_icount -= 5;
					m_r[REG_PC] = read16(UNSP_LPC);
					m_r[REG_SR] &= 0xffc0;
					m_r[REG_SR] |= op & 0x3f;
				}
				else
				{
					unimplemented_opcode(op);
				}
				return;

			case 0x04: // Multiply, Signed * Signed
				if(opn == 1 && opa != 7)
				{
					m_icount -= 12;
					lres = m_r[opa] * m_r[opb];
					if(m_r[opb] & 0x8000)
					{
						lres -= m_r[opa] << 16;
					}
					if(m_r[opa] & 0x8000)
					{
						lres -= m_r[opb] << 16;
					}
					m_r[REG_R4] = lres >> 16;
					m_r[REG_R3] = (uint16_t)lres;
				}
				else
				{
					unimplemented_opcode(op);
				}
				return;

			case 0x05: // Interrupt flags
				m_icount -= 2;
				switch(op & 0x3f)
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
				return;

			default:
				unimplemented_opcode(op);
				return;
		}
	}

	// At this point, we should be dealing solely with ALU ops.

	r0 = m_r[opa];

	switch (op1)
	{
		case 0x00: // r, [bp+imm6]
			m_icount -= 6;

			r2 = (uint16_t)(m_r[REG_BP] + (op & 0x3f));
			if (op0 != 0x0d)
				r1 = read16(r2);
			break;

		case 0x01: // r, imm6
			m_icount -= 2;

			r1 = op & 0x3f;
			break;

		case 0x03: // Indirect
		{
			m_icount -= (opa == 7 ? 7 : 6);

			const uint8_t lsbits = opn & 3;
			if (opn & 4)
			{
				switch (lsbits)
				{
					case 0: // r, [r]
						r2 = UNSP_LREG_I(opb);
						if (op0 != 0x0d)
							r1 = read16(r2);
						break;
					case 1: // r, [<ds:>r--]
						r2 = UNSP_LREG_I(opb);
						if (op0 != 0x0d)
							r1 = read16(r2);
						m_r[opb]--;
						if (m_r[opb] == 0xffff)
							m_r[REG_SR] -= 0x0400;
						break;
					case 2: // r, [<ds:>r++]
						r2 = UNSP_LREG_I(opb);
						if (op0 != 0x0d)
							r1 = read16(r2);
						m_r[opb]++;
						if (m_r[opb] == 0x0000)
							m_r[REG_SR] += 0x0400;
						break;
					case 3: // r, [<ds:>++r]
						m_r[opb]++;
						if (m_r[opb] == 0x0000)
							m_r[REG_SR] += 0x0400;
						r2 = UNSP_LREG_I(opb);
						if (op0 != 0x0d)
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
						r2 = m_r[opb];
						if (op0 != 0x0d)
							r1 = read16(r2);
						break;
					case 1: // r, [r--]
						r2 = m_r[opb];
						if (op0 != 0x0d)
							r1 = read16(r2);
						m_r[opb]--;
						break;
					case 2: // r, [r++]
						r2 = m_r[opb];
						if (op0 != 0x0d)
							r1 = read16(r2);
						m_r[opb]++;
						break;
					case 3: // r, [++r]
						m_r[opb]++;
						r2 = m_r[opb];
						if (op0 != 0x0d)
							r1 = read16(r2);
						break;
					default:
						break;
				}
			}
			break;
		}

		case 0x04: // 16-bit ops
			switch (opn)
			{
				case 0x00: // r
					m_icount -= (opa == 7 ? 5 : 3);
					r1 = m_r[opb];
					break;

				case 0x01: // imm16
					m_icount -= (opa == 7 ? 5 : 4);
					r0 = m_r[opb];
					r1 = read16(UNSP_LPC);
					add_lpc(1);
					break;

				case 0x02: // [imm16]
					m_icount -= (opa == 7 ? 8 : 7);
					r0 = m_r[opb];
					r2 = read16(UNSP_LPC);
					add_lpc(1);

					if (op0 != 0x0d)
					{
						r1 = read16(r2);
					}
					break;

				case 0x03: // store [imm16], r
					m_icount -= (opa == 7 ? 8 : 7);
					r1 = r0;
					r0 = m_r[opb];
					r2 = read16(UNSP_LPC);
					add_lpc(1);
					break;

				default: // Shifted ops
				{
					m_icount -= (opa == 7 ? 5 : 3);
					uint32_t shift = (m_r[opb] << 4) | m_sb;
					if (shift & 0x80000)
						shift |= 0xf00000;
					shift >>= (opn - 3);
					m_sb = shift & 0x0f;
					r1 = (uint16_t)(shift >> 4);
					break;
				}
			}
			break;

		case 0x05: // More shifted ops
			m_icount -= (opa == 7 ? 5 : 3);

			if (opn & 4) // Shift right
			{
				const uint32_t shift = ((m_r[opb] << 4) | m_sb) >> (opn - 3);
				m_sb = shift & 0x0f;
				r1 = (uint16_t)(shift >> 4);
			}
			else // Shift left
			{
				const uint32_t shift = ((m_sb << 16) | m_r[opb]) << (opn + 1);
				m_sb = (shift >> 16) & 0x0f;
				r1 = (uint16_t)shift;
			}
			break;

		case 0x06: // Rotated ops
		{
			m_icount -= (opa == 7 ? 5 : 3);

			uint32_t shift = (((m_sb << 16) | m_r[opb]) << 4) | m_sb;
			if (opn & 4) // Rotate right
			{
				shift >>= (opn - 3);
				m_sb = shift & 0x0f;
			}
			else
			{
				shift <<= (opn + 1);
				m_sb = (shift >> 20) & 0x0f;
			}
			r1 = (uint16_t)(shift >> 4);
			break;
		}

		case 0x07: // Direct 8
			m_icount -= (opa == 7 ? 6 : 5);
			r2 = op & 0x3f;
			r1 = read16(r2);
			break;

		default:
			break;
	}

	switch (op0)
	{
		case 0x00: // Add
			lres = r0 + r1;
			if (opa != 7)
				update_nzsc(lres, r0, r1);
			break;
		case 0x01: // Add w/ carry
			lres = r0 + r1;
			if(m_r[REG_SR] & UNSP_C)
				lres++;
			if (opa != 7)
				update_nzsc(lres, r0, r1);
			break;
		case 0x02: // Subtract
			lres = r0 + (uint16_t)(~r1) + 1;
			if (opa != 7)
				update_nzsc(lres, r0, r1);
			break;
		case 0x03: // Subtract w/ carry
			lres = r0 + (uint16_t)(~r1);
			if(m_r[REG_SR] & UNSP_C)
				lres++;
			if (opa != 7)
				update_nzsc(lres, r0, r1);
			break;
		case 0x04: // Compare
			lres = r0 + (uint16_t)(~r1) + 1;
			if (opa != 7)
				update_nzsc(lres, r0, r1);
			return;
		case 0x06: // Negate
			lres = -r1;
			if (opa != 7)
				update_nz(lres);
			break;
		case 0x08: // XOR
			lres = r0 ^ r1;
			if (opa != 7)
				update_nz(lres);
			break;
		case 0x09: // Load
			lres = r1;
			if (opa != 7)
				update_nz(lres);
			break;
		case 0x0a: // OR
			lres = r0 | r1;
			if (opa != 7)
				update_nz(lres);
			break;
		case 0x0b: // AND
			lres = r0 & r1;
			if (opa != 7)
				update_nz(lres);
			break;
		case 0x0c: // Test
			lres = r0 & r1;
			if (opa != 7)
				update_nz(lres);
			return;
		case 0x0d: // Store
			write16(r2, r0);
			return;
		default:
			unimplemented_opcode(op);
			if (opa != 7)
				update_nz(lres);
			return;
	}

	if (op1 == 0x04 && opn == 0x03) // store [imm16], r
		write16(r2, lres);
	else
		m_r[opa] = lres;
}

void unsp_device::execute_run()
{
#if UNSP_LOG_OPCODES
	unsp_disassembler dasm;
#endif

	while (m_icount > 0)
	{
		debugger_instruction_hook(UNSP_LPC);
		const uint32_t op = read16(UNSP_LPC);

#if UNSP_LOG_OPCODES
		if (m_log_ops)
		{
			std::stringstream strbuffer;
			dasm.disassemble(strbuffer, UNSP_LPC, op, read16(UNSP_LPC+1));
			logerror("%x: %s\n", UNSP_LPC, strbuffer.str().c_str());
		}
#endif

		add_lpc(1);

		execute_one(op);

		check_irqs();
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

uint8_t unsp_device::get_csb()
{
	return 1 << ((UNSP_LPC >> 20) & 3);
}
