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
#include "unspfe.h"

#include "debugger.h"

#include "unspdefs.h"
#include "unspdasm.h"

DEFINE_DEVICE_TYPE(UNSP, unsp_device, "unsp", "SunPlus u'nSP")
DEFINE_DEVICE_TYPE(UNSP_NEWER, unsp_newer_device, "unsp_newer", "SunPlus u'nSP (newer)") // found on GCM394 die, has extra instructions

/* size of the execution code cache */
#define CACHE_SIZE                      (64 * 1024 * 1024)

unsp_device::unsp_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_core(nullptr)
	, m_program_config("program", ENDIANNESS_BIG, 16, 23, -1)
	, m_program(nullptr)
	, m_debugger_temp(0)
#if UNSP_LOG_OPCODES || UNSP_LOG_REGS
	, m_log_ops(0)
#endif
	, m_cache(CACHE_SIZE + sizeof(unsp_device))
	, m_drcuml(nullptr)
	, m_drcfe(nullptr)
	, m_drcoptions(0)
	, m_cache_dirty(0)
	, m_entry(nullptr)
	, m_nocode(nullptr)
	, m_out_of_cycles(nullptr)
	, m_check_interrupts(nullptr)
	, m_trigger_fiq(nullptr)
	, m_trigger_irq(nullptr)
	, m_mem_read(nullptr)
	, m_mem_write(nullptr)
	, m_enable_drc(false)
{
}

unsp_device::unsp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: unsp_device(mconfig, UNSP, tag, owner, clock)
{
}

unsp_newer_device::unsp_newer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: unsp_device(mconfig, UNSP_NEWER, tag, owner, clock)
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

std::unique_ptr<util::disasm_interface> unsp_newer_device::create_disassembler()
{
	return std::make_unique<unsp_newer_disassembler>();
}

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
#if UNSP_LOG_REGS
	log_write(address, data);
#endif
	m_program->write_word(address, data);
}

/*****************************************************************************/

void unsp_device::device_start()
{
	m_core = (internal_unsp_state *)m_cache.alloc_near(sizeof(internal_unsp_state));
	memset(m_core, 0, sizeof(internal_unsp_state));

#if ENABLE_UNSP_DRC
	m_enable_drc = allow_drc();
#else
	m_enable_drc = false;
#endif

#if UNSP_LOG_REGS
	if (m_enable_drc)
		m_log_file = fopen("unsp_drc.bin", "wb");
	else
		m_log_file = fopen("unsp_interp.bin", "wb");
#endif

	m_debugger_temp = 0;

	m_program = &space(AS_PROGRAM);
	auto cache = m_program->cache<1, -1, ENDIANNESS_BIG>();
	m_pr16 = [cache](offs_t address) -> u16 { return cache->read_word(address); };
	m_prptr = [cache](offs_t address) -> const void * { return cache->read_ptr(address); };

	uint32_t umlflags = 0;
	m_drcuml = std::make_unique<drcuml_state>(*this, m_cache, umlflags, 1, 23, 0);

	// add UML symbols-
	m_drcuml->symbol_add(&m_core->m_r[REG_SP], sizeof(uint32_t), "SP");
	m_drcuml->symbol_add(&m_core->m_r[REG_R1], sizeof(uint32_t), "R1");
	m_drcuml->symbol_add(&m_core->m_r[REG_R2], sizeof(uint32_t), "R2");
	m_drcuml->symbol_add(&m_core->m_r[REG_R3], sizeof(uint32_t), "R3");
	m_drcuml->symbol_add(&m_core->m_r[REG_R4], sizeof(uint32_t), "R4");
	m_drcuml->symbol_add(&m_core->m_r[REG_BP], sizeof(uint32_t), "BP");
	m_drcuml->symbol_add(&m_core->m_r[REG_SR], sizeof(uint32_t), "SR");
	m_drcuml->symbol_add(&m_core->m_r[REG_PC], sizeof(uint32_t), "PC");
	m_drcuml->symbol_add(&m_core->m_enable_irq, sizeof(uint32_t), "IRQE");
	m_drcuml->symbol_add(&m_core->m_enable_fiq, sizeof(uint32_t), "FIQE");
	m_drcuml->symbol_add(&m_core->m_irq, sizeof(uint32_t), "IRQ");
	m_drcuml->symbol_add(&m_core->m_fiq, sizeof(uint32_t), "FIQ");
	m_drcuml->symbol_add(&m_core->m_sb, sizeof(uint32_t), "SB");
	m_drcuml->symbol_add(&m_core->m_icount, sizeof(m_core->m_icount), "icount");

	/* initialize the front-end helper */
	m_drcfe = std::make_unique<unsp_frontend>(this, COMPILE_BACKWARDS_BYTES, COMPILE_FORWARDS_BYTES, SINGLE_INSTRUCTION_MODE ? 1 : COMPILE_MAX_SEQUENCE);

	/* mark the cache dirty so it is updated on next execute */
	m_cache_dirty = true;

	// register our state for the debugger
	state_add(STATE_GENFLAGS, "GENFLAGS", m_core->m_r[REG_SR]).callimport().callexport().formatstr("%4s").noshow();
	state_add(UNSP_SP,     "SP", m_core->m_r[REG_SP]).formatstr("%04X");
	state_add(UNSP_R1,     "R1", m_core->m_r[REG_R1]).formatstr("%04X");
	state_add(UNSP_R2,     "R2", m_core->m_r[REG_R2]).formatstr("%04X");
	state_add(UNSP_R3,     "R3", m_core->m_r[REG_R3]).formatstr("%04X");
	state_add(UNSP_R4,     "R4", m_core->m_r[REG_R4]).formatstr("%04X");
	state_add(UNSP_BP,     "BP", m_core->m_r[REG_BP]).formatstr("%04X");
	state_add(UNSP_SR,     "SR", m_core->m_r[REG_SR]).formatstr("%04X");
	state_add(UNSP_PC,     "PC", m_debugger_temp).callimport().callexport().formatstr("%06X");
	state_add(UNSP_IRQ_EN, "IRQE", m_core->m_enable_irq).formatstr("%1u");
	state_add(UNSP_FIQ_EN, "FIQE", m_core->m_enable_fiq).formatstr("%1u");
	state_add(UNSP_IRQ,    "IRQ", m_core->m_irq).formatstr("%1u");
	state_add(UNSP_FIQ,    "FIQ", m_core->m_fiq).formatstr("%1u");
	state_add(UNSP_SB,     "SB", m_core->m_sb).formatstr("%1u");
#if UNSP_LOG_OPCODES || UNSP_LOG_REGS
	state_add(UNSP_LOG_OPS,"LOG", m_log_ops).formatstr("%1u");
#endif

	state_add(STATE_GENPC, "GENPC", m_debugger_temp).callexport().noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_debugger_temp).callexport().noshow();

	save_item(NAME(m_core->m_r));
	save_item(NAME(m_core->m_enable_irq));
	save_item(NAME(m_core->m_enable_fiq));
	save_item(NAME(m_core->m_irq));
	save_item(NAME(m_core->m_fiq));
	save_item(NAME(m_core->m_curirq));
	save_item(NAME(m_core->m_sirq));
	save_item(NAME(m_core->m_sb));
	save_item(NAME(m_core->m_saved_sb));

	set_icountptr(m_core->m_icount);
}

void unsp_device::device_reset()
{
	memset(m_core->m_r, 0, sizeof(uint32_t) * 8);

	m_core->m_r[REG_PC] = read16(0xfff7);
	m_core->m_enable_irq = 0;
	m_core->m_enable_fiq = 0;
	m_core->m_irq = 0;
	m_core->m_fiq = 0;
}

void unsp_device::device_stop()
{
	if (m_drcfe != nullptr)
	{
		m_drcfe = nullptr;
	}
	if (m_drcuml != nullptr)
	{
		m_drcuml = nullptr;
	}
#if UNSP_LOG_REGS
	fclose(m_log_file);
#endif
}

#if UNSP_LOG_REGS
void unsp_device::log_regs()
{
	if (m_log_ops == 0)
		return;
	fwrite(m_core->m_r, sizeof(uint32_t), 8, m_log_file);
	fwrite(&m_core->m_sb, sizeof(uint32_t), 1, m_log_file);
	fwrite(&m_core->m_icount, sizeof(uint32_t), 1, m_log_file);
}

void unsp_device::log_write(uint32_t addr, uint32_t data)
{
	addr |= 0x80000000;
	fwrite(&addr, sizeof(uint32_t), 1, m_log_file);
	fwrite(&data, sizeof(uint32_t), 1, m_log_file);
}

#endif

void unsp_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
		{
			const uint16_t sr = m_core->m_r[REG_SR];
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
			m_core->m_r[REG_PC] = m_debugger_temp & 0x0000ffff;
			m_core->m_r[REG_SR] = (m_core->m_r[REG_SR] & 0xffc0) | ((m_debugger_temp & 0x003f0000) >> 16);
			break;
	}
}

/*****************************************************************************/

void unsp_device::update_nzsc(uint32_t value, uint16_t r0, uint16_t r1)
{
	m_core->m_r[REG_SR] &= ~(UNSP_N | UNSP_Z | UNSP_S | UNSP_C);
	if (int16_t(r0) < int16_t(~r1))
		m_core->m_r[REG_SR] |= UNSP_S;
	if (BIT(value, 15))
		m_core->m_r[REG_SR] |= UNSP_N;
	if((uint16_t)value == 0)
		m_core->m_r[REG_SR] |= UNSP_Z;
	if (BIT(value, 16))
		m_core->m_r[REG_SR] |= UNSP_C;
}

void unsp_device::update_nz(uint32_t value)
{
	m_core->m_r[REG_SR] &= ~(UNSP_N | UNSP_Z);
	if(value & 0x8000)
		m_core->m_r[REG_SR] |= UNSP_N;
	if((uint16_t)value == 0)
		m_core->m_r[REG_SR] |= UNSP_Z;
}

void unsp_device::push(uint32_t value, uint32_t *reg)
{
	write16(*reg, (uint16_t)value);
	*reg = (uint16_t)(*reg - 1);
}

uint16_t unsp_device::pop(uint32_t *reg)
{
	*reg = (uint16_t)(*reg + 1);
	return (uint16_t)read16(*reg);
}

void unsp_device::trigger_fiq()
{
	if (!m_core->m_enable_fiq || m_core->m_fiq || m_core->m_irq)
	{
		return;
	}

	m_core->m_fiq = 1;

	m_core->m_saved_sb[m_core->m_irq ? 1 : 0] = m_core->m_sb;
	m_core->m_sb = m_core->m_saved_sb[2];

	push(m_core->m_r[REG_PC], &m_core->m_r[REG_SP]);
	push(m_core->m_r[REG_SR], &m_core->m_r[REG_SP]);
	m_core->m_r[REG_PC] = read16(0xfff6);
	m_core->m_r[REG_SR] = 0;
}

void unsp_device::trigger_irq(int line)
{
	if (!m_core->m_enable_irq || m_core->m_irq || m_core->m_fiq)
		return;

	m_core->m_irq = 1;

	m_core->m_saved_sb[0] = m_core->m_sb;
	m_core->m_sb = m_core->m_saved_sb[1];

	push(m_core->m_r[REG_PC], &m_core->m_r[REG_SP]);
	push(m_core->m_r[REG_SR], &m_core->m_r[REG_SP]);
	m_core->m_r[REG_PC] = read16(0xfff8 + line);
	m_core->m_r[REG_SR] = 0;
}

void unsp_device::check_irqs()
{
	if (!m_core->m_sirq)
		return;

	int highest_irq = -1;
	for (int i = 0; i <= 8; i++)
	{
		if (BIT(m_core->m_sirq, i))
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
	m_core->m_r[REG_PC] = (uint16_t)new_lpc;
	m_core->m_r[REG_SR] &= 0xffc0;
	m_core->m_r[REG_SR] |= (new_lpc >> 16) & 0x3f;
}

void unsp_device::execute_f_group(const uint16_t op)
{
	unimplemented_opcode(op);
}

void unsp_newer_device::execute_f_group(const uint16_t op)
{
	logerror("UNSP: unknown extended opcode %04x at %04x\n", op, UNSP_LPC);
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
				if(!(m_core->m_r[REG_SR] & UNSP_C))
				{
					m_core->m_icount -= 4;
					add_lpc((op1 == 0) ? opimm : (0 - opimm));
				}
				else
				{
					m_core->m_icount -= 2;
				}
				return;
			case 1: // JAE
				if(m_core->m_r[REG_SR] & UNSP_C)
				{
					m_core->m_icount -= 4;
					add_lpc((op1 == 0) ? opimm : (0 - opimm));
				}
				else
				{
					m_core->m_icount -= 2;
				}
				return;
			case 2: // JGE
				if(!(m_core->m_r[REG_SR] & UNSP_S))
				{
					m_core->m_icount -= 4;
					add_lpc((op1 == 0) ? opimm : (0 - opimm));
				}
				else
				{
					m_core->m_icount -= 2;
				}
				return;
			case 3: // JL
				if(m_core->m_r[REG_SR] & UNSP_S)
				{
					m_core->m_icount -= 4;
					add_lpc((op1 == 0) ? opimm : (0 - opimm));
				}
				else
				{
					m_core->m_icount -= 2;
				}
				return;
			case 4: // JNE
				if(!(m_core->m_r[REG_SR] & UNSP_Z))
				{
					m_core->m_icount -= 4;
					add_lpc((op1 == 0) ? opimm : (0 - opimm));
				}
				else
				{
					m_core->m_icount -= 2;
				}
				return;
			case 5: // JE
				if(m_core->m_r[REG_SR] & UNSP_Z)
				{
					m_core->m_icount -= 4;
					add_lpc((op1 == 0) ? opimm : (0 - opimm));
				}
				else
				{
					m_core->m_icount -= 2;
				}
				return;
			case 6: // JPL
				if(!(m_core->m_r[REG_SR] & UNSP_N))
				{
					m_core->m_icount -= 4;
					add_lpc((op1 == 0) ? opimm : (0 - opimm));
				}
				else
				{
					m_core->m_icount -= 2;
				}
				return;
			case 7: // JMI
				if(m_core->m_r[REG_SR] & UNSP_N)
				{
					m_core->m_icount -= 4;
					add_lpc((op1 == 0) ? opimm : (0 - opimm));
				}
				else
				{
					m_core->m_icount -= 2;
				}
				return;
			case 8: // JBE
				if((m_core->m_r[REG_SR] & (UNSP_Z | UNSP_C)) != UNSP_C) // branch if (!UNSP_Z && !UNSP_C) || UNSP_Z
				{
					m_core->m_icount -= 4;
					add_lpc((op1 == 0) ? opimm : (0 - opimm));
				}
				else
				{
					m_core->m_icount -= 2;
				}
				return;
			case 9: // JA
				if((m_core->m_r[REG_SR] & (UNSP_Z | UNSP_C)) == UNSP_C) // branch if !UNSP_Z && UNSP_C
				{
					m_core->m_icount -= 4;
					add_lpc((op1 == 0) ? opimm : (0 - opimm));
				}
				else
				{
					m_core->m_icount -= 2;
				}
				return;
			case 10: // JLE
				if(m_core->m_r[REG_SR] & (UNSP_Z | UNSP_S))
				{
					m_core->m_icount -= 4;
					add_lpc((op1 == 0) ? opimm : (0 - opimm));
				}
				else
				{
					m_core->m_icount -= 2;
				}
				return;
			case 11: // JG
				if(!(m_core->m_r[REG_SR] & (UNSP_Z | UNSP_S)))
				{
					m_core->m_icount -= 4;
					add_lpc((op1 == 0) ? opimm : (0 - opimm));
				}
				else
				{
					m_core->m_icount -= 2;
				}
				return;
			case 14: // JMP
				add_lpc((op1 == 0) ? opimm : (0 - opimm));
				m_core->m_icount -= 4;
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
		m_core->m_icount -= 4 + 2 * r0;

		while (r0--)
		{
			push(m_core->m_r[r1--], &m_core->m_r[opb]);
		}
		return;
	}
	else if (lower_op == 0x29)
	{
		if (op == 0x9a98) // reti
		{
			m_core->m_icount -= 8;
			m_core->m_r[REG_SR] = pop(&m_core->m_r[REG_SP]);
			m_core->m_r[REG_PC] = pop(&m_core->m_r[REG_SP]);

			if(m_core->m_fiq)
			{
				m_core->m_fiq = 0;
				m_core->m_saved_sb[2] = m_core->m_sb;
				m_core->m_sb = m_core->m_saved_sb[m_core->m_irq ? 1 : 0];
			}
			else if(m_core->m_irq)
			{
				m_core->m_irq = 0;
				m_core->m_saved_sb[1] = m_core->m_sb;
				m_core->m_sb = m_core->m_saved_sb[0];
			}
			m_core->m_curirq = 0;
			check_irqs();
			return;
		}
		else // pop
		{
			r0 = opn;
			r1 = opa;
			m_core->m_icount -= 4 + 2 * r0;

			while (r0--)
			{
				m_core->m_r[++r1] = pop(&m_core->m_r[opb]);
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
					m_core->m_icount -= 12;
					lres = m_core->m_r[opa] * m_core->m_r[opb];
					if(m_core->m_r[opb] & 0x8000)
					{
						lres -= m_core->m_r[opa] << 16;
					}
					m_core->m_r[REG_R4] = lres >> 16;
					m_core->m_r[REG_R3] = (uint16_t)lres;
				}
				else
				{
					unimplemented_opcode(op);
				}
				return;

			case 0x01: // Call
				if(!(opa & 1))
				{
					m_core->m_icount -= 9;
					r1 = read16(UNSP_LPC);
					add_lpc(1);
					push(m_core->m_r[REG_PC], &m_core->m_r[REG_SP]);
					push(m_core->m_r[REG_SR], &m_core->m_r[REG_SP]);
					m_core->m_r[REG_PC] = r1;
					m_core->m_r[REG_SR] &= 0xffc0;
					m_core->m_r[REG_SR] |= op & 0x3f;
				}
				else
				{
					unimplemented_opcode(op);
				}
				return;

			case 0x02: // Far Jump
				if (opa == 7)
				{
					m_core->m_icount -= 5;
					m_core->m_r[REG_PC] = read16(UNSP_LPC);
					m_core->m_r[REG_SR] &= 0xffc0;
					m_core->m_r[REG_SR] |= op & 0x3f;
				}
				else
				{
					unimplemented_opcode(op);
				}
				return;

			case 0x04: // Multiply, Signed * Signed
				if(opn == 1 && opa != 7)
				{
					m_core->m_icount -= 12;
					lres = m_core->m_r[opa] * m_core->m_r[opb];
					if(m_core->m_r[opb] & 0x8000)
					{
						lres -= m_core->m_r[opa] << 16;
					}
					if(m_core->m_r[opa] & 0x8000)
					{
						lres -= m_core->m_r[opb] << 16;
					}
					m_core->m_r[REG_R4] = lres >> 16;
					m_core->m_r[REG_R3] = (uint16_t)lres;
				}
				else
				{
					unimplemented_opcode(op);
				}
				return;

			case 0x05: // Interrupt flags
				m_core->m_icount -= 2;
				switch(op & 0x3f)
				{
					case 0:
						m_core->m_enable_irq = 0;
						m_core->m_enable_fiq = 0;
						break;
					case 1:
						m_core->m_enable_irq = 1;
						m_core->m_enable_fiq = 0;
						break;
					case 2:
						m_core->m_enable_irq = 0;
						m_core->m_enable_fiq = 1;
						break;
					case 3:
						m_core->m_enable_irq = 1;
						m_core->m_enable_fiq = 1;
						break;
					case 8: // irq off
						m_core->m_enable_irq = 0;
						break;
					case 9: // irq on
						m_core->m_enable_irq = 1;
						break;
					case 12: // fiq off
						m_core->m_enable_fiq = 0;
						break;
					case 14: // fiq on
						m_core->m_enable_fiq = 1;
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

	r0 = m_core->m_r[opa];

	switch (op1)
	{
		case 0x00: // r, [bp+imm6]
			m_core->m_icount -= 6;

			r2 = (uint16_t)(m_core->m_r[REG_BP] + (op & 0x3f));
			if (op0 != 0x0d)
				r1 = read16(r2);
			break;

		case 0x01: // r, imm6
			m_core->m_icount -= 2;

			r1 = op & 0x3f;
			break;

		case 0x03: // Indirect
		{
			m_core->m_icount -= (opa == 7 ? 7 : 6);

			const uint8_t lsbits = opn & 3;
			if (opn & 4)
			{
				switch (lsbits)
				{
					case 0: // r, [<ds:>r]
						r2 = UNSP_LREG_I(opb);
						if (op0 != 0x0d)
							r1 = read16(r2);
						break;

					case 1: // r, [<ds:>r--]
						r2 = UNSP_LREG_I(opb);
						if (op0 != 0x0d)
							r1 = read16(r2);
						m_core->m_r[opb] = (uint16_t)(m_core->m_r[opb] - 1);
						if (m_core->m_r[opb] == 0xffff)
							m_core->m_r[REG_SR] -= 0x0400;
						break;
					case 2: // r, [<ds:>r++]
						r2 = UNSP_LREG_I(opb);
						if (op0 != 0x0d)
							r1 = read16(r2);
						m_core->m_r[opb] = (uint16_t)(m_core->m_r[opb] + 1);
						if (m_core->m_r[opb] == 0x0000)
							m_core->m_r[REG_SR] += 0x0400;
						break;
					case 3: // r, [<ds:>++r]
						m_core->m_r[opb] = (uint16_t)(m_core->m_r[opb] + 1);
						if (m_core->m_r[opb] == 0x0000)
							m_core->m_r[REG_SR] += 0x0400;
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
						r2 = m_core->m_r[opb];
						if (op0 != 0x0d)
							r1 = read16(r2);
						break;
					case 1: // r, [r--]
						r2 = m_core->m_r[opb];
						if (op0 != 0x0d)
							r1 = read16(r2);
						m_core->m_r[opb] = (uint16_t)(m_core->m_r[opb] - 1);
						break;
					case 2: // r, [r++]
						r2 = m_core->m_r[opb];
						if (op0 != 0x0d)
							r1 = read16(r2);
						m_core->m_r[opb] = (uint16_t)(m_core->m_r[opb] + 1);
						break;
					case 3: // r, [++r]
						m_core->m_r[opb] = (uint16_t)(m_core->m_r[opb] + 1);
						r2 = m_core->m_r[opb];
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
					m_core->m_icount -= (opa == 7 ? 5 : 3);
					r1 = m_core->m_r[opb];
					break;

				case 0x01: // imm16
					m_core->m_icount -= (opa == 7 ? 5 : 4);
					r0 = m_core->m_r[opb];
					r1 = read16(UNSP_LPC);
					add_lpc(1);
					break;

				case 0x02: // [imm16]
					m_core->m_icount -= (opa == 7 ? 8 : 7);
					r0 = m_core->m_r[opb];
					r2 = read16(UNSP_LPC);
					add_lpc(1);

					if (op0 != 0x0d)
					{
						r1 = read16(r2);
					}
					break;

				case 0x03: // store [imm16], r
					m_core->m_icount -= (opa == 7 ? 8 : 7);
					r1 = r0;
					r0 = m_core->m_r[opb];
					r2 = read16(UNSP_LPC);
					add_lpc(1);
					break;

				default: // Shifted ops
				{
					m_core->m_icount -= (opa == 7 ? 5 : 3);
					uint32_t shift = (m_core->m_r[opb] << 4) | m_core->m_sb;
					if (shift & 0x80000)
						shift |= 0xf00000;
					shift >>= (opn - 3);
					m_core->m_sb = shift & 0x0f;
					r1 = (uint16_t)(shift >> 4);
					break;
				}
			}
			break;

		case 0x05: // More shifted ops
			m_core->m_icount -= (opa == 7 ? 5 : 3);

			if (opn & 4) // Shift right
			{
				const uint32_t shift = ((m_core->m_r[opb] << 4) | m_core->m_sb) >> (opn - 3);
				m_core->m_sb = shift & 0x0f;
				r1 = (uint16_t)(shift >> 4);
			}
			else // Shift left
			{
				const uint32_t shift = ((m_core->m_sb << 16) | m_core->m_r[opb]) << (opn + 1);
				m_core->m_sb = (shift >> 16) & 0x0f;
				r1 = (uint16_t)shift;
			}
			break;

		case 0x06: // Rotated ops
		{
			m_core->m_icount -= (opa == 7 ? 5 : 3);

			uint32_t shift = (((m_core->m_sb << 16) | m_core->m_r[opb]) << 4) | m_core->m_sb;
			if (opn & 4) // Rotate right
			{
				shift >>= (opn - 3);
				m_core->m_sb = shift & 0x0f;
			}
			else
			{
				shift <<= (opn + 1);
				m_core->m_sb = (shift >> 20) & 0x0f;
			}
			r1 = (uint16_t)(shift >> 4);
			break;
		}

		case 0x07: // Direct 8
			m_core->m_icount -= (opa == 7 ? 6 : 5);
			r2 = op & 0x3f;
			r1 = read16(r2);
			break;

		default:
			break;
	}

	switch (op0)
	{
		case 0x00: // Add
		{
			lres = r0 + r1;
			if (opa != 7)
				update_nzsc(lres, r0, r1);
			break;
		}
		case 0x01: // Add w/ carry
		{
			uint32_t c = (m_core->m_r[REG_SR] & UNSP_C) ? 1 : 0;
			lres = r0 + r1 + c;
			if (opa != 7)
				update_nzsc(lres, r0, r1);
			break;
		}
		case 0x02: // Subtract
			lres = r0 + (uint16_t)(~r1) + uint32_t(1);
			if (opa != 7)
				update_nzsc(lres, r0, ~r1);
			break;
		case 0x03: // Subtract w/ carry
		{
			uint32_t c = (m_core->m_r[REG_SR] & UNSP_C) ? 1 : 0;
			lres = r0 + (uint16_t)(~r1) + c;
			if (opa != 7)
				update_nzsc(lres, r0, ~r1);
			break;
		}
		case 0x04: // Compare
			lres = r0 + (uint16_t)(~r1) + uint32_t(1);
			if (opa != 7)
				update_nzsc(lres, r0, ~r1);
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

		case 0x0f: // extended opcodes
			execute_f_group(op);
			return;

		default:
			unimplemented_opcode(op);
			return;
	}

	if (op1 == 0x04 && opn == 0x03) // store [imm16], r
		write16(r2, lres);
	else
		m_core->m_r[opa] = (uint16_t)lres;
}

void unsp_device::execute_run()
{
	if (m_enable_drc)
	{
		execute_run_drc();
		return;
	}

#if UNSP_LOG_OPCODES
	unsp_disassembler dasm;
#endif

	while (m_core->m_icount >= 0)
	{
		debugger_instruction_hook(UNSP_LPC);
		const uint32_t op = read16(UNSP_LPC);

#if UNSP_LOG_REGS
		log_regs();
#endif

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

void unsp_device::execute_set_input(int inputnum, int state)
{
	set_state_unsynced(inputnum, state);
}

uint8_t unsp_device::get_csb()
{
	return 1 << ((UNSP_LPC >> 20) & 3);
}

void unsp_device::set_state_unsynced(int inputnum, int state)
{
	m_core->m_sirq &= ~(1 << inputnum);

	if(!state)
	{
		return;
	}

	switch (inputnum)
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
			m_core->m_sirq |= (1 << inputnum);
			break;
		case UNSP_BRK_LINE:
			break;
	}
}

uint16_t unsp_device::get_ds()
{
	return (m_core->m_r[REG_SR] >> 10) & 0x3f;
}

void unsp_device::set_ds(uint16_t ds)
{
	m_core->m_r[REG_SR] &= 0x03ff;
	m_core->m_r[REG_SR] |= (ds & 0x3f) << 10;
}
