// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller, R. Belmont
/*****************************************************************************
 *
 *   sh2.cpp
 *   Portable Hitachi SH-2 (SH7600 family) emulator
 *
 *  This work is based on <tiraniddo@hotmail.com> C/C++ implementation of
 *  the SH-2 CPU core and was adapted to the MAME CPU core requirements.
 *  Thanks also go to Chuck Mason <chukjr@sundail.net> and Olivier Galibert
 *  <galibert@pobox.com> for letting me peek into their SEMU code :-)
 *
 *****************************************************************************/


#include "emu.h"
#include "sh2.h"
#include "sh_dasm.h"
#include "cpu/drcumlsh.h"

//#define VERBOSE 1
#include "logmacro.h"

constexpr int SH2_INT_15 = 15;

sh2_device::sh2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int cpu_type, address_map_constructor internal_map, int addrlines, uint32_t address_mask)
	: sh_common_execution(mconfig, type, tag, owner, clock, ENDIANNESS_BIG, internal_map)
	, m_am(address_mask)
	, m_program_config("program", ENDIANNESS_BIG, 32, addrlines, 0, internal_map)
	, m_decrypted_program_config("decrypted_opcodes", ENDIANNESS_BIG, 32, addrlines, 0)
	, m_drcfe(nullptr)
{
	m_cpu_type = cpu_type;
	m_isdrc = allow_drc();
}

void sh2_device::device_start()
{
	sh_common_execution::device_start();

	m_decrypted_program = has_space(AS_OPCODES) ? &space(AS_OPCODES) : &space(AS_PROGRAM);
	m_decrypted_program->cache(m_cache32);
	m_pr16 = [this](offs_t address) -> u16 { return m_cache32.read_word(address); };
	if (m_decrypted_program->endianness() != ENDIANNESS_NATIVE)
		m_prptr = [this](offs_t address) -> const void * {
			const u16 *ptr = reinterpret_cast<u16 *>(m_cache32.read_ptr(address & ~3));
			if (!(address & 2))
				ptr++;
			return ptr;
		};
	else
		m_prptr = [this](offs_t address) -> const void * {
			const u16 *ptr = reinterpret_cast<u16 *>(m_cache32.read_ptr(address & ~3));
			if (address & 2)
				ptr++;
			return ptr;
		};

	// internals
	save_item(NAME(m_cpu_off));
	save_item(NAME(m_test_irq));
	save_item(NAME(m_irq_line_state));
	save_item(NAME(m_nmi_line_state));
	save_item(NAME(m_internal_irq_vector));

	state_add(STATE_GENPC, "PC", m_sh2_state->pc).mask(m_am).callimport();
	state_add(STATE_GENPCBASE, "CURPC", m_sh2_state->pc).callimport().noshow();

	m_nmi_line_state = 0;

	drc_start();
}

void sh2_device::device_reset()
{
	std::fill(std::begin(m_sh2_state->r), std::end(m_sh2_state->r), 0);
	std::fill(std::begin(m_irq_line_state), std::end(m_irq_line_state), 0);

	m_sh2_state->pc = m_sh2_state->pr = m_sh2_state->sr = m_sh2_state->gbr = m_sh2_state->vbr = m_sh2_state->mach = m_sh2_state->macl = 0;
	m_sh2_state->evec = m_sh2_state->irqsr = 0;
	m_sh2_state->ea = m_sh2_state->m_delay = 0;
	m_sh2_state->pending_irq = 0;
	m_sh2_state->pending_nmi = 0;
	m_sh2_state->sleep_mode = 0;
	m_sh2_state->internal_irq_level = -1;
	m_sh2_state->sr = SH_I;
	m_sh2_state->pc = read_long(0);
	m_sh2_state->r[15] = read_long(4);

	m_test_irq = 0;
	m_cpu_off = 0;
	m_internal_irq_vector = 0;
	m_cache_dirty = true;
}

device_memory_interface::space_config_vector sh2_device::memory_space_config() const
{
	if (has_configured_map(AS_OPCODES))
		return space_config_vector
		{
			std::make_pair(AS_PROGRAM, &m_program_config),
			std::make_pair(AS_OPCODES, &m_decrypted_program_config)
		};
	else
		return space_config_vector
		{
			std::make_pair(AS_PROGRAM, &m_program_config)
		};
}

std::unique_ptr<util::disasm_interface> sh2_device::create_disassembler()
{
	return std::make_unique<sh_disassembler>(false);
}

uint8_t sh2_device::read_byte(offs_t offset)
{
	if ((offset & 0xf0000000) == 0 || (offset & 0xf0000000) == 0x20000000)
		return m_program->read_byte(offset & m_am);

	return m_program->read_byte(offset);
}

uint16_t sh2_device::read_word(offs_t offset)
{
	if ((offset & 0xf0000000) == 0 || (offset & 0xf0000000) == 0x20000000)
		return m_program->read_word(offset & m_am);

	return m_program->read_word(offset);
}

uint32_t sh2_device::read_long(offs_t offset)
{
	/* 0x20000000 no Cache */
	/* 0x00000000 read thru Cache if CE bit is 1 */
	if ((offset & 0xf0000000) == 0 || (offset & 0xf0000000) == 0x20000000)
		return m_program->read_dword(offset & m_am);

	return m_program->read_dword(offset);
}

uint16_t sh2_device::decrypted_read_word(offs_t offset)
{
	return m_decrypted_program->read_word(offset);
}

void sh2_device::write_byte(offs_t offset, uint8_t data)
{
	if ((offset & 0xf0000000) == 0 || (offset & 0xf0000000) == 0x20000000)
	{
		m_program->write_byte(offset & m_am, data);
		return;
	}

	m_program->write_byte(offset, data);
}

void sh2_device::write_word(offs_t offset, uint16_t data)
{
	if ((offset & 0xf0000000) == 0 || (offset & 0xf0000000) == 0x20000000)
	{
		m_program->write_word(offset & m_am, data);
		return;
	}

	m_program->write_word(offset, data);
}

void sh2_device::write_long(offs_t offset, uint32_t data)
{
	if ((offset & 0xf0000000) == 0 || (offset & 0xf0000000) == 0x20000000)
	{
		m_program->write_dword(offset & m_am, data);
		return;
	}

	/* 0x20000000 no Cache */
	/* 0x00000000 read thru Cache if CE bit is 1 */
	m_program->write_dword(offset, data);
}

void sh2_device::check_pending_irq(const char *message)
{
	if (m_sh2_state->pending_nmi)
	{
		sh2_exception(message, 16);
		m_sh2_state->pending_nmi = 0;
	}
	else
	{
		int irq = m_sh2_state->internal_irq_level;
		if (m_sh2_state->pending_irq)
		{
			int external_irq = 15 - (count_leading_zeros_32(m_sh2_state->pending_irq) - 16);
			if (external_irq >= irq)
			{
				irq = external_irq;
			}
		}

		if (irq >= 0)
		{
			sh2_exception(message, irq);
		}
	}
}

/*  LDC.L   @Rm+,SR */
inline void sh2_device::LDCMSR(const uint16_t opcode) // passes Rn
{
	const uint32_t rn = REG_N;
	m_sh2_state->ea = m_sh2_state->r[rn];
	m_sh2_state->sr = read_long(m_sh2_state->ea) & SH_FLAGS;
	m_sh2_state->r[rn] += 4;
	m_sh2_state->icount -= 2;
	m_test_irq = 1;
}

/*  LDC     Rm,SR */
inline void sh2_device::LDCSR(const uint16_t opcode) // passes Rn
{
	m_sh2_state->sr = m_sh2_state->r[REG_N] & SH_FLAGS;
	m_test_irq = 1;
}

/*  RTE */
inline void sh2_device::RTE()
{
	m_sh2_state->ea = m_sh2_state->r[15];
	m_sh2_state->m_delay = read_long(m_sh2_state->ea);
	m_sh2_state->r[15] += 4;
	m_sh2_state->ea = m_sh2_state->r[15];
	m_sh2_state->sr = read_long(m_sh2_state->ea) & SH_FLAGS;
	m_sh2_state->r[15] += 4;
	m_sh2_state->icount -= 3;
	m_test_irq = 1;
}

/*  TRAPA   #imm */
inline void sh2_device::TRAPA(uint32_t i)
{
	uint32_t imm = i & 0xff;
	debugger_exception_hook(imm);

	m_sh2_state->ea = m_sh2_state->vbr + imm * 4;

	m_sh2_state->r[15] -= 4;
	write_long(m_sh2_state->r[15], m_sh2_state->sr);
	m_sh2_state->r[15] -= 4;
	write_long(m_sh2_state->r[15], m_sh2_state->pc);

	m_sh2_state->pc = read_long(m_sh2_state->ea);

	m_sh2_state->icount -= 7;
}

/*  ILLEGAL */
inline void sh2_device::ILLEGAL()
{
	//logerror("Illegal opcode at %08x\n", m_sh2_state->pc - 2);
	debugger_exception_hook(4);

	m_sh2_state->r[15] -= 4;
	write_long(m_sh2_state->r[15], m_sh2_state->sr);     /* push SR onto stack */
	m_sh2_state->r[15] -= 4;
	write_long(m_sh2_state->r[15], m_sh2_state->pc - 2); /* push PC onto stack */

	/* fetch PC */
	m_sh2_state->pc = read_long(m_sh2_state->vbr + 4 * 4);

	/* TODO: timing is a guess */
	m_sh2_state->icount -= 5;
}

void sh2_device::execute_one_f000(uint16_t opcode)
{
	ILLEGAL();
}

void sh2_device::execute_run()
{
	if (m_isdrc)
	{
		execute_run_drc();
		return;
	}

	if (m_cpu_off)
	{
		debugger_wait_hook();
		m_sh2_state->icount = 0;
		return;
	}

	do
	{
		debugger_instruction_hook(m_sh2_state->pc);

		const uint16_t opcode = m_decrypted_program->read_word(m_sh2_state->pc >= 0x40000000 ? m_sh2_state->pc : m_sh2_state->pc & m_am);

		if (m_sh2_state->m_delay)
		{
			m_sh2_state->pc = m_sh2_state->m_delay;
			m_sh2_state->m_delay = 0;
		}
		else
			m_sh2_state->pc += 2;

		execute_one(opcode);

		if (m_test_irq && !m_sh2_state->m_delay)
		{
			check_pending_irq("mame_sh2_execute");
			m_test_irq = 0;
		}
		m_sh2_state->icount--;
	} while (m_sh2_state->icount > 0);
}

void sh2_device::init_drc_frontend()
{
	m_drcfe = std::make_unique<sh2_frontend>(this, COMPILE_BACKWARDS_BYTES, COMPILE_FORWARDS_BYTES, SINGLE_INSTRUCTION_MODE ? 1 : COMPILE_MAX_SEQUENCE);
}

void sh2_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c%d%c%c",
					m_sh2_state->sr & SH_M ? 'M':'.',
					m_sh2_state->sr & SH_Q ? 'Q':'.',
					(m_sh2_state->sr & SH_I) >> 4,
					m_sh2_state->sr & SH_S ? 'S':'.',
					m_sh2_state->sr & SH_T ? 'T':'.');
			break;
	}
}

void sh2_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case STATE_GENPC:
		case STATE_GENPCBASE:
			m_sh2_state->m_delay = 0;
			break;

		case SH_SR:
			check_pending_irq("sh2_set_reg");
			break;
	}
}

void sh2_device::execute_set_input(int irqline, int state)
{
	if (irqline == INPUT_LINE_NMI)
	{
		if (m_nmi_line_state == state)
			return;

		m_nmi_line_state = state;

		if (state == CLEAR_LINE)
		{
			LOG("SH-2 cleared NMI\n");
		}
		else
		{
			LOG("SH-2 asserted NMI\n");

			m_sh2_state->pending_nmi = 1;

			if (m_isdrc)
			{
				sh2_exception("Set IRQ line", 16);
			}
			else
			{
				if (m_sh2_state->m_delay)
					m_test_irq = 1;
				else
					check_pending_irq("sh2_set_nmi_line");
			}
		}
	}
	else
	{
		if (m_irq_line_state[irqline] == state)
			return;

		m_irq_line_state[irqline] = state;

		if (state == CLEAR_LINE)
		{
			LOG("SH-2 cleared irq #%d\n", irqline);
			m_sh2_state->pending_irq &= ~(1 << irqline);
		}
		else
		{
			LOG("SH-2 asserted irq #%d\n", irqline);
			m_sh2_state->pending_irq |= 1 << irqline;

			if (m_isdrc)
			{
				m_test_irq = 1;
			}
			else
			{
				if (m_sh2_state->m_delay)
					m_test_irq = 1;
				else
					check_pending_irq("sh2_set_irq_line");
			}
		}
	}
}

void sh2_device::sh2_exception(const char *message, int irqline)
{
	// override this at the individual CPU level when special logic is required
	int vector;

	if (irqline != 16)
	{
		if (irqline <= ((m_sh2_state->sr >> 4) & 15)) /* If the cpu forbids this interrupt */
			return;

		// if this is an sh2 internal irq, use its vector
		if (m_sh2_state->internal_irq_level == irqline)
		{
			vector = m_internal_irq_vector;
			/* avoid spurious irqs with this (TODO: needs a better fix) */
			m_sh2_state->internal_irq_level = -1;
			LOG("SH-2 exception #%d (internal vector: $%x) after [%s]\n", irqline, vector, message);
		}
		else
		{
			standard_irq_callback(irqline, m_sh2_state->pc);
			vector = 64 + irqline/2;
			LOG("SH-2 exception #%d (autovector: $%x) after [%s]\n", irqline, vector, message);
		}
	}
	else
	{
		vector = 11;
		LOG("SH-2 nmi exception (autovector: $%x) after [%s]\n", vector, message);
	}

	sh2_exception_internal(message, irqline, vector);
}

void sh2_device::sh2_exception_internal(const char *message, int irqline, int vector)
{
	debugger_exception_hook(vector);

	if (m_isdrc)
	{
		m_sh2_state->evec = read_long(m_sh2_state->vbr + vector * 4);
		m_sh2_state->evec &= m_am;
		m_sh2_state->irqsr = m_sh2_state->sr;

		/* set I flags in SR */
		if (irqline > SH2_INT_15)
			m_sh2_state->sr = m_sh2_state->sr | SH_I;
		else
			m_sh2_state->sr = (m_sh2_state->sr & ~SH_I) | (irqline << 4);

//  printf("sh2_exception [%s] irqline %x evec %x save SR %x new SR %x\n", message, irqline, m_sh2_state->evec, m_sh2_state->irqsr, m_sh2_state->sr);
	}
	else
	{
		m_sh2_state->r[15] -= 4;
		write_long(m_sh2_state->r[15], m_sh2_state->sr);     /* push SR onto stack */
		m_sh2_state->r[15] -= 4;
		write_long(m_sh2_state->r[15], m_sh2_state->pc);     /* push PC onto stack */

		/* set I flags in SR */
		if (irqline > SH2_INT_15)
			m_sh2_state->sr = m_sh2_state->sr | SH_I;
		else
			m_sh2_state->sr = (m_sh2_state->sr & ~SH_I) | (irqline << 4);

		/* fetch PC */
		m_sh2_state->pc = read_long(m_sh2_state->vbr + vector * 4);
	}

	if (m_sh2_state->sleep_mode == 1)
		m_sh2_state->sleep_mode = 2;
}

/////////
// DRC

const opcode_desc* sh2_device::get_desclist(offs_t pc)
{
	return m_drcfe->describe_code(pc);
}

void sh2_device::func_fastirq()
{
	sh2_exception("fastirq", m_sh2_state->irqline);
}
static void cfunc_fastirq(void *param) { ((sh2_device *)param)->func_fastirq(); };

/*-------------------------------------------------
    static_generate_entry_point - generate a
    static entry point
-------------------------------------------------*/

void sh2_device::static_generate_entry_point()
{
	uml::code_label const skip = 1;

	/* begin generating */
	drcuml_block &block(m_drcuml->begin_block(200));

	/* forward references */
	alloc_handle(m_nocode, "nocode");
	alloc_handle(m_write32, "write32");     // necessary?
	alloc_handle(m_entry, "entry");
	UML_HANDLE(block, *m_entry);                         // handle  entry

	/* load fast integer registers */
	load_fast_iregs(block);

	/* check for interrupts */
	UML_MOV(block, mem(&m_sh2_state->irqline), 0xffffffff);     // mov irqline, #-1
	UML_CMP(block, mem(&m_sh2_state->pending_nmi), 0);          // cmp pending_nmi, #0
	UML_JMPc(block, COND_Z, skip+2);                    // jz skip+2

	UML_MOV(block, mem(&m_sh2_state->pending_nmi), 0);          // zap pending_nmi
	UML_JMP(block, skip+1);                     // and then go take it (evec is already set)

	UML_LABEL(block, skip+2);                   // skip+2:
	UML_MOV(block, mem(&m_sh2_state->evec), 0xffffffff);        // mov evec, -1
	UML_MOV(block, I0, 0xffffffff);         // mov r0, -1 (r0 = irq)
	UML_AND(block, I1,  I0, 0xffff);                // and r1, 0xffff

	UML_LZCNT(block, I1, mem(&m_sh2_state->pending_irq));       // lzcnt r1, r1
	UML_CMP(block, I1, 32);             // cmp r1, #32
	UML_JMPc(block, COND_Z, skip+4);                    // jz skip+4

	UML_SUB(block, mem(&m_sh2_state->irqline), 31, I1);     // sub irqline, #31, r1

	UML_LABEL(block, skip+4);                   // skip+4:
	UML_CMP(block, mem(&m_sh2_state->internal_irq_level), 0xffffffff);  // cmp internal_irq_level, #-1
	UML_JMPc(block, COND_Z, skip+3);                    // jz skip+3
	UML_CMP(block, mem(&m_sh2_state->internal_irq_level), mem(&m_sh2_state->irqline));      // cmp internal_irq_level, irqline
	UML_JMPc(block, COND_LE, skip+3);                   // jle skip+3

	UML_MOV(block, mem(&m_sh2_state->irqline), mem(&m_sh2_state->internal_irq_level));      // mov r0, internal_irq_level

	UML_LABEL(block, skip+3);                   // skip+3:
	UML_CMP(block, mem(&m_sh2_state->irqline), 0xffffffff);     // cmp irqline, #-1
	UML_JMPc(block, COND_Z, skip+1);                    // jz skip+1
	UML_CALLC(block, cfunc_fastirq, this);               // callc fastirq

	UML_LABEL(block, skip+1);                   // skip+1:

	UML_CMP(block, mem(&m_sh2_state->evec), 0xffffffff);        // cmp evec, 0xffffffff
	UML_JMPc(block, COND_Z, skip);                  // jz skip

	UML_SUB(block, R32(15), R32(15), 4);            // sub R15, R15, #4
	UML_MOV(block, I0, R32(15));                // mov r0, R15
	UML_MOV(block, I1, mem(&m_sh2_state->irqsr));           // mov r1, irqsr
	UML_CALLH(block, *m_write32);                    // call write32

	UML_SUB(block, R32(15), R32(15), 4);            // sub R15, R15, #4
	UML_MOV(block, I0, R32(15));                // mov r0, R15
	UML_MOV(block, I1, mem(&m_sh2_state->pc));              // mov r1, pc
	UML_CALLH(block, *m_write32);                    // call write32

	UML_MOV(block, mem(&m_sh2_state->pc), mem(&m_sh2_state->evec));             // mov pc, evec

	UML_LABEL(block, skip);                         // skip:

	/* generate a hash jump via the current mode and PC */
	UML_HASHJMP(block, 0, mem(&m_sh2_state->pc), *m_nocode);     // hashjmp <mode>,<pc>,nocode

	block.end();
}


/*-------------------------------------------------
    generate_update_cycles - generate code to
    subtract cycles from the icount and generate
    an exception if out
-------------------------------------------------*/

void sh2_device::generate_update_cycles(drcuml_block &block, compiler_state &compiler, uml::parameter param, bool allow_exception)
{
	/* check full interrupts if pending */
	if (compiler.checkints)
	{
		uml::code_label const skip = compiler.labelnum++;

		compiler.checkints = false;
		compiler.labelnum += 4;

		/* check for interrupts */
		UML_MOV(block, mem(&m_sh2_state->irqline), 0xffffffff);     // mov irqline, #-1
		UML_CMP(block, mem(&m_sh2_state->pending_nmi), 0);          // cmp pending_nmi, #0
		UML_JMPc(block, COND_Z, skip+2);                    // jz skip+2

		UML_MOV(block, mem(&m_sh2_state->pending_nmi), 0);          // zap pending_nmi
		UML_JMP(block, skip+1);                     // and then go take it (evec is already set)

		UML_LABEL(block, skip+2);                   // skip+2:
		UML_MOV(block, mem(&m_sh2_state->evec), 0xffffffff);        // mov evec, -1
		UML_MOV(block, I0, 0xffffffff);         // mov r0, -1 (r0 = irq)
		UML_AND(block, I1,  I0, 0xffff);                // and r1, r0, 0xffff

		UML_LZCNT(block, I1, mem(&m_sh2_state->pending_irq));       // lzcnt r1, pending_irq
		UML_CMP(block, I1, 32);             // cmp r1, #32
		UML_JMPc(block, COND_Z, skip+4);                    // jz skip+4

		UML_SUB(block, mem(&m_sh2_state->irqline), 31, I1);     // sub irqline, #31, r1

		UML_LABEL(block, skip+4);                   // skip+4:
		UML_CMP(block, mem(&m_sh2_state->internal_irq_level), 0xffffffff);  // cmp internal_irq_level, #-1
		UML_JMPc(block, COND_Z, skip+3);                    // jz skip+3
		UML_CMP(block, mem(&m_sh2_state->internal_irq_level), mem(&m_sh2_state->irqline));      // cmp internal_irq_level, irqline
		UML_JMPc(block, COND_LE, skip+3);                   // jle skip+3

		UML_MOV(block, mem(&m_sh2_state->irqline), mem(&m_sh2_state->internal_irq_level));      // mov r0, internal_irq_level

		UML_LABEL(block, skip+3);                   // skip+3:
		UML_CMP(block, mem(&m_sh2_state->irqline), 0xffffffff);     // cmp irqline, #-1
		UML_JMPc(block, COND_Z, skip+1);                    // jz skip+1
		UML_CALLC(block, cfunc_fastirq, this);               // callc fastirq

		UML_LABEL(block, skip+1);                   // skip+1:
		UML_CMP(block, mem(&m_sh2_state->evec), 0xffffffff);        // cmp evec, 0xffffffff
		UML_JMPc(block, COND_Z, skip);                  // jz skip

		UML_SUB(block, R32(15), R32(15), 4);            // sub R15, R15, #4
		UML_MOV(block, I0, R32(15));                // mov r0, R15
		UML_MOV(block, I1, mem(&m_sh2_state->irqsr));           // mov r1, irqsr
		UML_CALLH(block, *m_write32);                    // call write32

		UML_SUB(block, R32(15), R32(15), 4);            // sub R15, R15, #4
		UML_MOV(block, I0, R32(15));                // mov r0, R15
		UML_MOV(block, I1, param);              // mov r1, nextpc
		UML_CALLH(block, *m_write32);                    // call write32

		UML_HASHJMP(block, 0, mem(&m_sh2_state->evec), *m_nocode);       // hashjmp m_sh2_state->evec

		UML_LABEL(block, skip);                         // skip:
	}

	/* account for cycles */
	if (compiler.cycles > 0)
	{
		UML_SUB(block, mem(&m_sh2_state->icount), mem(&m_sh2_state->icount), MAPVAR_CYCLES);    // sub     icount,icount,cycles
		UML_MAPVAR(block, MAPVAR_CYCLES, 0);                                        // mapvar  cycles,0
		if (allow_exception)
			UML_EXHc(block, COND_S, *m_out_of_cycles, param);
																					// exh     out_of_cycles,nextpc
	}
	compiler.cycles = 0;
}

/*------------------------------------------------------------------
    static_generate_memory_accessor
------------------------------------------------------------------*/

void sh2_device::static_generate_memory_accessor(int size, int iswrite, const char *name, uml::code_handle *&handleptr)
{
	/* on entry, address is in I0; data for writes is in I1 */
	/* on exit, read result is in I0 */
	/* routine trashes I0 */
	int label = 1;

	/* begin generating */
	drcuml_block &block(m_drcuml->begin_block(1024));

	/* add a global entry for this */
	alloc_handle(handleptr, name);
	UML_HANDLE(block, *handleptr);                         // handle  *handleptr

	// with internal handlers this becomes easier.
	// if addr < 0x40000000 AND it with AM and do the read/write, else just do the read/write
	UML_TEST(block, I0, 0x80000000);        // test r0, #0x80000000
	UML_JMPc(block, COND_NZ, label);                // if high bit is set, don't mask

	UML_CMP(block, I0, 0x40000000);     // cmp #0x40000000, r0
	UML_JMPc(block, COND_AE, label);            // bae label

	UML_AND(block, I0, I0, m_am);     // and r0, r0, #AM (0xc7ffffff)

	UML_LABEL(block, label++);              // label:

	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) == 0)
	{
		for (auto & elem : m_fastram)
		{
			if (elem.base != nullptr && (!iswrite || !elem.readonly))
			{
				void *fastbase = (uint8_t *)elem.base - elem.start;
				uint32_t skip = label++;
				if (elem.end != 0xffffffff)
				{
					UML_CMP(block, I0, elem.end);   // cmp     i0,end
					UML_JMPc(block, COND_A, skip);                                      // ja      skip
				}
				if (elem.start != 0x00000000)
				{
					UML_CMP(block, I0, elem.start);// cmp     i0,fastram_start
					UML_JMPc(block, COND_B, skip);                                      // jb      skip
				}

				if (!iswrite)
				{
					if (size == 1)
					{
						UML_XOR(block, I0, I0, BYTE4_XOR_BE(0));
						UML_LOAD(block, I0, fastbase, I0, SIZE_BYTE, SCALE_x1);             // load    i0,fastbase,i0,byte
					}
					else if (size == 2)
					{
						UML_XOR(block, I0, I0, WORD_XOR_BE(0));
						UML_LOAD(block, I0, fastbase, I0, SIZE_WORD, SCALE_x1);         // load    i0,fastbase,i0,word_x1
					}
					else if (size == 4)
					{
						UML_LOAD(block, I0, fastbase, I0, SIZE_DWORD, SCALE_x1);            // load    i0,fastbase,i0,dword_x1
					}
					UML_RET(block);                                                     // ret
				}
				else
				{
					if (size == 1)
					{
						UML_XOR(block, I0, I0, BYTE4_XOR_BE(0));
						UML_STORE(block, fastbase, I0, I1, SIZE_BYTE, SCALE_x1);// store   fastbase,i0,i1,byte
					}
					else if (size == 2)
					{
						UML_XOR(block, I0, I0, WORD_XOR_BE(0));
						UML_STORE(block, fastbase, I0, I1, SIZE_WORD, SCALE_x1);// store   fastbase,i0,i1,word_x1
					}
					else if (size == 4)
					{
						UML_STORE(block, fastbase, I0, I1, SIZE_DWORD, SCALE_x1);       // store   fastbase,i0,i1,dword_x1
					}
					UML_RET(block);                                                     // ret
				}

				UML_LABEL(block, skip);                                             // skip:
			}
		}
	}

	if (iswrite)
	{
		switch (size)
		{
			case 1:
				UML_WRITE(block, I0, I1, SIZE_BYTE, SPACE_PROGRAM); // write r0, r1, program_byte
				break;

			case 2:
				UML_WRITE(block, I0, I1, SIZE_WORD, SPACE_PROGRAM); // write r0, r1, program_word
				break;

			case 4:
				UML_WRITE(block, I0, I1, SIZE_DWORD, SPACE_PROGRAM);    // write r0, r1, program_dword
				break;
		}
	}
	else
	{
		switch (size)
		{
			case 1:
				UML_READ(block, I0, I0, SIZE_BYTE, SPACE_PROGRAM);  // read r0, program_byte
				break;

			case 2:
				UML_READ(block, I0, I0, SIZE_WORD, SPACE_PROGRAM);  // read r0, program_word
				break;

			case 4:
				UML_READ(block, I0, I0, SIZE_DWORD, SPACE_PROGRAM); // read r0, program_dword
				break;
		}
	}

	UML_RET(block);                         // ret

	block.end();
}
