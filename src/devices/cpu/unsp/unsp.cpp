// license:GPL-2.0+
// copyright-holders:Segher Boessenkool, Ryan Holtz, David Haywood
/*****************************************************************************

    SunPlus µ'nSP emulator

    Copyright 2008-2017  Segher Boessenkool  <segher@kernel.crashing.org>
    Licensed under the terms of the GNU GPL, version 2
    http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

    Ported to MAME framework by Ryan Holtz

    Notes:

    R3 and R4 together are 'MR' with R4 being the upper part of the 32-bit reg

*****************************************************************************/

#include "emu.h"
#include "unsp.h"
#include "unspfe.h"

#include "debugger.h"

#include "unspdasm.h"

#include <climits>

DEFINE_DEVICE_TYPE(UNSP,    unsp_device,    "unsp",    "SunPlus u'nSP (ISA 1.0)")
// 1.1 is just 1.0 with better CPI?
DEFINE_DEVICE_TYPE(UNSP_11, unsp_11_device, "unsp_11", "SunPlus u'nSP (ISA 1.1)")
 // it's possible that most unSP systems we emulate are 1.2, but are not using 99% of the additional features / instructions over 1.0 (only enable_irq and enable_fiq are meant to be 1.2 specific and used, but that could be a research error)
DEFINE_DEVICE_TYPE(UNSP_12, unsp_12_device, "unsp_12", "SunPlus u'nSP (ISA 1.2)")
// found on GCM394 die (based on use of 2 extended push/pop opcodes in the smartfp irq), has extra instructions
DEFINE_DEVICE_TYPE(UNSP_20, unsp_20_device, "unsp_20", "SunPlus u'nSP (ISA 2.0)")

/* size of the execution code cache */
#define CACHE_SIZE                      (64 * 1024 * 1024)

unsp_device::unsp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor internal)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_core(nullptr)
	, m_program_config("program", ENDIANNESS_BIG, 16, 23, -1, internal)
	, m_debugger_temp(0)
#if UNSP_LOG_OPCODES || UNSP_LOG_REGS
	, m_log_ops(0)
#endif
	, m_drccache(CACHE_SIZE + sizeof(unsp_device))
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
	, m_vectorbase(0xfff0)
{
	m_iso = 10;
	m_numregs = 8;
}

unsp_device::unsp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: unsp_device(mconfig, UNSP, tag, owner, clock, address_map_constructor())
{
	m_iso = 10;
	m_numregs = 8;
}

unsp_11_device::unsp_11_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: unsp_device(mconfig, UNSP_11, tag, owner, clock, address_map_constructor())
{
	m_iso = 11;
	m_numregs = 8;
}

unsp_11_device::unsp_11_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor internal)
	: unsp_device(mconfig, type, tag, owner, clock, internal)
{
	m_iso = 11;
	m_numregs = 8;
}


unsp_12_device::unsp_12_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: unsp_11_device(mconfig, UNSP_12, tag, owner, clock, address_map_constructor())
{
	m_iso = 12;
	m_numregs = 8;
}

unsp_12_device::unsp_12_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor internal)
	: unsp_11_device(mconfig, type, tag, owner, clock, internal)
{
	m_iso = 12;
	m_numregs = 8;
}

unsp_20_device::unsp_20_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: unsp_12_device(mconfig, UNSP_20, tag, owner, clock, address_map_constructor())
{
	m_iso = 20;
	m_numregs = 16;
}

unsp_20_device::unsp_20_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor internal)
	: unsp_12_device(mconfig, type, tag, owner, clock, internal)
{
	m_iso = 20;
	m_numregs = 16;
}

unsp_device::~unsp_device()
{
}

// these are just for logging, can be removed once all ops are implemented
char const* const unsp_device::regs[] =
{
	"sp", "r1", "r2", "r3", "r4", "bp", "sr", "pc"
};

char const* const unsp_device::bitops[] =
{
	"tstb", "setb", "clrb", "invb"
};

// log/barrel shift
char const* const unsp_device::lsft[] =
{
	"asr", "asror", "lsl", "lslor", "lsr", "lsror", "rol", "ror"
};

char const* const unsp_device::extregs[] =
{
	"r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
};

char const* const unsp_device::aluops[] =
{
	"add","adc","sub","sbc","cmp","(invalid)","neg","--","xor","load","or","and","test","store","(invalid)","(invalid)"
};

char const* const unsp_device::forms[] =
{
	"[%s]", "[%s--]", "[%s++]", "[++%s]"
};


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

std::unique_ptr<util::disasm_interface> unsp_12_device::create_disassembler()
{
	return std::make_unique<unsp_12_disassembler>();
}

std::unique_ptr<util::disasm_interface> unsp_20_device::create_disassembler()
{
	return std::make_unique<unsp_20_disassembler>();
}

void unsp_device::unimplemented_opcode(uint16_t op)
{
	fatalerror("UNSP: unknown opcode %04x at %04x\n", op, UNSP_LPC);
}

void unsp_device::unimplemented_opcode(uint16_t op, uint16_t ximm)
{
	fatalerror("UNSP: unknown opcode %04x %04x at %04x\n", op, ximm, UNSP_LPC);
}


void unsp_device::unimplemented_opcode(uint16_t op, uint16_t ximm, uint16_t ximm_2)
{
	fatalerror("UNSP: unknown opcode %04x %04x %04x at %04x\n", op, ximm, ximm_2, UNSP_LPC);
}

void unsp_device::device_start()
{

	m_core = (internal_unsp_state *)m_drccache.alloc_near(sizeof(internal_unsp_state));
	memset(m_core, 0, sizeof(internal_unsp_state));

#if ENABLE_UNSP_DRC
	m_enable_drc = allow_drc() && (m_iso < 12);
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

	space(AS_PROGRAM).cache(m_cache);
	space(AS_PROGRAM).specific(m_program);

	uint32_t umlflags = 0;
	m_drcuml = std::make_unique<drcuml_state>(*this, m_drccache, umlflags, 1, 23, 0);

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
	m_drcuml->symbol_add(&m_core->m_fir_move, sizeof(uint32_t), "FIR_MOV");
	m_drcuml->symbol_add(&m_core->m_sb, sizeof(uint32_t), "SB");
	m_drcuml->symbol_add(&m_core->m_aq, sizeof(uint32_t), "AQ");
	m_drcuml->symbol_add(&m_core->m_fra, sizeof(uint32_t), "FRA");
	m_drcuml->symbol_add(&m_core->m_bnk, sizeof(uint32_t), "BNK");
	m_drcuml->symbol_add(&m_core->m_ine, sizeof(uint32_t), "INE");
	m_drcuml->symbol_add(&m_core->m_pri, sizeof(uint32_t), "PRI");
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
	state_add(UNSP_FIR_MOV_EN, "FIR_MOV", m_core->m_fir_move).formatstr("%1u");
	state_add(UNSP_SB,     "SB", m_core->m_sb).formatstr("%1X");
	state_add(UNSP_AQ,     "AQ", m_core->m_aq).formatstr("%1u");
	state_add(UNSP_FRA,    "FRA", m_core->m_fra).formatstr("%1u");
	state_add(UNSP_BNK,    "BNK", m_core->m_bnk).formatstr("%1u");
	state_add(UNSP_INE,    "INE", m_core->m_ine).formatstr("%1u");
	state_add(UNSP_PRI,    "PRI", m_core->m_pri).formatstr("%1u");
#if UNSP_LOG_OPCODES || UNSP_LOG_REGS
	state_add(UNSP_LOG_OPS,"LOG", m_log_ops).formatstr("%1u");
#endif

	state_add(STATE_GENPC, "GENPC", m_debugger_temp).callexport().noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_debugger_temp).callexport().noshow();

	save_item(NAME(m_core->m_r));
	save_item(NAME(m_core->m_enable_irq));
	save_item(NAME(m_core->m_enable_fiq));
	save_item(NAME(m_core->m_irq));
	save_item(NAME(m_core->m_fir_move));
	save_item(NAME(m_core->m_sirq));
	save_item(NAME(m_core->m_sb));
	save_item(NAME(m_core->m_aq));
	save_item(NAME(m_core->m_fra));
	save_item(NAME(m_core->m_bnk));
	save_item(NAME(m_core->m_ine));
	save_item(NAME(m_core->m_pri));
	save_item(NAME(m_core->m_divq_bit));
	save_item(NAME(m_core->m_divq_dividend));
	save_item(NAME(m_core->m_divq_divisor));

	set_icountptr(m_core->m_icount);
}

void unsp_20_device::device_start()
{
	unsp_12_device::device_start();

#if UNSP_LOG_OPCODES || UNSP_LOG_REGS
	int baseindex = UNSP_LOG_OPS + 1;
#else
	int baseindex = UNSP_PRI + 1;
#endif

	state_add(baseindex + UNSP20_R8,     "R8", m_core->m_r[UNSP20_R8+8]).formatstr("%04X");
	state_add(baseindex + UNSP20_R9,     "R9", m_core->m_r[UNSP20_R9+8]).formatstr("%04X");
	state_add(baseindex + UNSP20_R10,    "R10", m_core->m_r[UNSP20_R10+8]).formatstr("%04X");
	state_add(baseindex + UNSP20_R11,    "R11", m_core->m_r[UNSP20_R11+8]).formatstr("%04X");
	state_add(baseindex + UNSP20_R12,    "R12", m_core->m_r[UNSP20_R12+8]).formatstr("%04X");
	state_add(baseindex + UNSP20_R13,    "R13", m_core->m_r[UNSP20_R13+8]).formatstr("%04X");
	state_add(baseindex + UNSP20_R14,    "R14", m_core->m_r[UNSP20_R14+8]).formatstr("%04X");
	state_add(baseindex + UNSP20_R15,    "R15", m_core->m_r[UNSP20_R15+8]).formatstr("%04X");
}

void unsp_device::device_reset()
{
	for (int i = 0; i < std::size(m_core->m_r); i++)
	{
		if (i < m_numregs)
			m_core->m_r[i] = 0;
		else
			m_core->m_r[i] = 0xdeadbeef;
	}

	m_core->m_r[REG_PC] = read16(m_vectorbase + 0x7);
	m_core->m_enable_irq = 0;
	m_core->m_enable_fiq = 0;
	m_core->m_fir_move = 1;
	m_core->m_sb = 0;
	m_core->m_aq = 0;
	m_core->m_fra = 0;
	m_core->m_bnk = 0;
	m_core->m_ine = 0;
	m_core->m_pri = 8;
	m_core->m_fiq = 0;
	m_core->m_irq = 0;
	m_core->m_sirq = 0;
	m_core->m_divq_bit = UINT_MAX;
}

void unsp_20_device::device_reset()
{
	unsp_12_device::device_reset();
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
	if (m_log_ops == 0)
		return;
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
	if (BIT(value, 16) != BIT((r0 ^ r1), 15))
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

inline void unsp_device::trigger_fiq()
{
	if (!m_core->m_enable_fiq || m_core->m_fiq)
		return;

	m_core->m_fiq = 1;

	push(m_core->m_r[REG_PC], &m_core->m_r[REG_SP]);
	push(m_core->m_r[REG_SR], &m_core->m_r[REG_SP]);
	m_core->m_r[REG_PC] = read16(m_vectorbase + 0x06);
	m_core->m_r[REG_SR] = 0;
	standard_irq_callback(UNSP_FIQ_LINE);
}

inline void unsp_device::trigger_irq(int line)
{
	if ((m_core->m_ine == 0 && m_core->m_irq == 1) || m_core->m_pri <= line || !m_core->m_enable_irq)
		return;

	m_core->m_irq = 1;

	push(m_core->m_r[REG_PC], &m_core->m_r[REG_SP]);
	push(m_core->m_r[REG_SR], &m_core->m_r[REG_SP]);
	if (m_core->m_ine)
	{
		push(get_fr(), &m_core->m_r[REG_SP]);
	}

	if (m_core->m_ine)
		m_core->m_pri = line;

	m_core->m_r[REG_PC] = read16(m_vectorbase + 0x08 + line);
	m_core->m_r[REG_SR] = 0;
	standard_irq_callback(UNSP_IRQ0_LINE+line);
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


inline void unsp_device::execute_one(const uint16_t op)
{
	const uint16_t op0 = (op >> 12) & 15;
	const uint16_t opa = (op >> 9) & 7;
	const uint16_t op1 = (op >> 6) & 7;

	if (op0 == 0xf)
		return execute_fxxx_group(op);

	if(op0 < 0xf && opa == 0x7 && op1 < 2)
		return execute_jumps(op);

	if (op0 == 0xe)
		return execute_exxx_group(op);

	execute_remaining(op);
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

		if (op != 0x9a98)
		{
			check_irqs();
		}
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

void unsp_device::set_fr(uint16_t fr)
{
	m_core->m_aq = BIT(fr, 14);
	const uint32_t old_bank = m_core->m_bnk;
	m_core->m_bnk = BIT(fr, 13);
	if (m_core->m_bnk != old_bank)
	{
		std::swap<uint32_t>(m_core->m_r[REG_R1], m_core->m_secbank[REG_SR1]);
		std::swap<uint32_t>(m_core->m_r[REG_R2], m_core->m_secbank[REG_SR2]);
		std::swap<uint32_t>(m_core->m_r[REG_R3], m_core->m_secbank[REG_SR3]);
		std::swap<uint32_t>(m_core->m_r[REG_R4], m_core->m_secbank[REG_SR4]);
	}
	m_core->m_fra = BIT(fr, 12);
	m_core->m_fir_move = BIT(fr, 11);
	m_core->m_sb = (fr >> 7) & 0xf;
	m_core->m_enable_fiq = BIT(fr, 6);
	m_core->m_enable_irq = BIT(fr, 5);
	m_core->m_ine = BIT(fr, 4);
	m_core->m_pri = fr & 0xf;
}

uint16_t unsp_device::get_fr()
{
	uint16_t fr = m_core->m_aq << 14;
	fr |= m_core->m_bnk << 13;
	fr |= m_core->m_fra << 12;
	fr |= m_core->m_fir_move << 11;
	fr |= m_core->m_sb << 7;
	fr |= m_core->m_enable_fiq << 6;
	fr |= m_core->m_enable_irq << 5;
	fr |= m_core->m_ine << 4;
	fr |= m_core->m_pri;
	return fr;
}
