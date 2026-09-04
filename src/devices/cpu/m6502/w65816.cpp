// license:BSD-3-Clause
// copyright-holders:R. Belmont, Olivier Galibert
/***************************************************************************

	w65816.cpp

	Western Design Center W65C816, the 16-bit successor to the 6502
	Emulation by R. Belmont based on m6502 by Olivier Galibert

***************************************************************************/

#include "emu.h"
#include "w65816.h"
#include "w65816d.h"

DEFINE_DEVICE_TYPE(W65816, w65816_device, "w65816", "WDC W65C816S")

w65816_device::w65816_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	w65816_device(mconfig, W65816, tag, owner, clock)
{
}

w65816_device::w65816_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	cpu_device(mconfig, type, tag, owner, clock),
	m_wdm_w(*this),
	m_sync_w(*this),
	m_program_config("program", ENDIANNESS_LITTLE, 8, 24),
	m_sprogram_config("decrypted_opcodes", ENDIANNESS_LITTLE, 8, 24),
	m_vector_config("vectors", ENDIANNESS_LITTLE, 8, 5),
	m_mintf(nullptr)
{
}

void w65816_device::device_start()
{
	if(!m_mintf)
		m_mintf = std::make_unique<mi_default>();

	init();
}

void w65816_device::init()
{
	space(AS_PROGRAM).specific(m_mintf->m_program);
	m_mintf->m_cprogram = m_mintf->m_program;
	if(has_space(AS_OPCODES))
		space(AS_OPCODES).specific(m_mintf->m_csprogram);
	else
		m_mintf->m_csprogram = m_mintf->m_program;

	if(has_space(AS_VECTORS)) {
		space(AS_VECTORS).specific(m_mintf->m_vectors);
		m_mintf->m_has_vectors = true;
	}

	state_add(STATE_GENPC,     "GENPC",     m_XPC).callexport().noshow();
	state_add(STATE_GENPCBASE, "CURPC",     m_XPC).callexport().noshow();
	state_add(STATE_GENFLAGS,  "GENFLAGS",  m_P).callimport().formatstr("%8s").noshow();
	state_add(W65816_PC,       "PC",        m_NPC).callimport();
	state_add(W65816_A,        "A",         m_A);
	state_add(W65816_X,        "X",         m_X);
	state_add(W65816_Y,        "Y",         m_Y);
	state_add(W65816_P,        "P",         m_P).callimport();
	state_add(W65816_S,        "SP",        m_SP).callimport();
	state_add(W65816_D,        "D",         m_D);
	state_add(W65816_DB,       "DB",        m_DB);
	state_add(W65816_PB,       "PB",        m_PB);
	state_add(W65816_E,        "E",         m_E).callimport();
	state_add(W65816_IR,       "IR",        m_IR);

	save_item(NAME(m_PPC));
	save_item(NAME(m_NPC));
	save_item(NAME(m_PC));
	save_item(NAME(m_SP));
	save_item(NAME(m_D));
	save_item(NAME(m_A));
	save_item(NAME(m_X));
	save_item(NAME(m_Y));
	save_item(NAME(m_TMP));
	save_item(NAME(m_TMP2));
	save_item(NAME(m_TMP3));
	save_item(NAME(m_VAL));
	save_item(NAME(m_TMPL));
	save_item(NAME(m_SRC));
	save_item(NAME(m_DST));
	save_item(NAME(m_P));
	save_item(NAME(m_PB));
	save_item(NAME(m_DB));
	save_item(NAME(m_IR));
	save_item(NAME(m_E));
	save_item(NAME(m_inst_state_base));
	save_item(NAME(m_inst_state));
	save_item(NAME(m_inst_substate));
	save_item(NAME(m_nmi_state));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_abort_state));
	save_item(NAME(m_v_state));
	save_item(NAME(m_nmi_pending));
	save_item(NAME(m_irq_taken));
	save_item(NAME(m_inhibit_interrupts));

	set_icountptr(m_icount);

	m_XPC = 0;
	m_PPC = 0;
	m_NPC = 0;
	m_PC = 0;
	m_SP = 0x01ff;
	m_D = 0;
	m_A = 0;
	m_X = 0;
	m_Y = 0;
	m_TMP = 0;
	m_TMP2 = 0;
	m_TMP3 = 0;
	m_VAL = 0;
	m_TMPL = 0;
	m_SRC = 0;
	m_DST = 0;
	m_P = F_M | F_X | F_I;
	m_PB = 0;
	m_DB = 0;
	m_IR = 0;
	m_E = 1;
	m_inst_state_base = STATE_E;
	m_inst_state = STATE_RESET;
	m_inst_substate = 0;
	m_nmi_state = false;
	m_irq_state = false;
	m_abort_state = false;
	m_v_state = false;
	m_nmi_pending = false;
	m_irq_taken = false;
	m_sync = false;
	m_inhibit_interrupts = false;
}

void w65816_device::device_reset()
{
	// §2.25 and the reset column of Table 7-1: E is forced set, which in turn
	// forces M and X high and SH to 01; D, DBR and PBR are cleared and the
	// index register high bytes go to 00.  S low, A and B survive.
	m_E = 1;
	m_P = (m_P | F_M | F_X | F_I) & ~F_D;
	m_SP = 0x0100 | (m_SP & 0xff);
	m_X &= 0xff;
	m_Y &= 0xff;
	m_D = 0;
	m_DB = 0;
	m_PB = 0;

	m_inst_state_base = STATE_E;
	m_inst_state = STATE_RESET;
	m_inst_substate = 0;
	m_nmi_pending = false;
	m_irq_taken = false;
	m_sync = false;
	m_sync_w(CLEAR_LINE);
	m_inhibit_interrupts = false;
}

uint32_t w65816_device::execute_min_cycles() const noexcept
{
	return 1;
}

uint32_t w65816_device::execute_max_cycles() const noexcept
{
	return 10;
}

bool w65816_device::execute_input_edge_triggered(int inputnum) const noexcept
{
	return inputnum == NMI_LINE || inputnum == V_LINE || inputnum == ABORT_LINE;
}

void w65816_device::execute_run()
{
	if(m_inst_substate)
		do_exec_partial();

	while(m_icount > 0) {
		if(m_inst_state < 0xff00) {
			m_PPC = m_NPC;
			m_inst_state = m_IR | m_inst_state_base;
			if(debugger_enabled())
				debugger_instruction_hook(pc_to_external(m_NPC));
		}
		do_exec_full();
	}
}

void w65816_device::execute_set_input(int inputnum, int state)
{
	switch(inputnum) {
	case IRQ_LINE:
		m_irq_state = state == ASSERT_LINE;
		break;
	case NMI_LINE:
		// don't accept an NMI edge at exactly the same time RESET is cleared
		if(!m_nmi_state && state == ASSERT_LINE && total_cycles())
			m_nmi_pending = true;
		m_nmi_state = state == ASSERT_LINE;
		break;
	case ABORT_LINE:
		// ABORTB is latched but the abort sequence itself is not implemented yet
		m_abort_state = state == ASSERT_LINE;
		break;
	case V_LINE:
		if(!m_v_state && state == ASSERT_LINE && total_cycles())
			m_P |= F_V;
		m_v_state = state == ASSERT_LINE;
		break;
	}
}

device_memory_interface::space_config_vector w65816_device::memory_space_config() const
{
	space_config_vector r;
	r.emplace_back(AS_PROGRAM, &m_program_config);
	if(has_configured_map(AS_OPCODES))
		r.emplace_back(AS_OPCODES, &m_sprogram_config);
	if(has_configured_map(AS_VECTORS))
		r.emplace_back(AS_VECTORS, &m_vector_config);
	return r;
}

offs_t w65816_device::pc_to_external(uint16_t pc)
{
	return (m_PB << 16) | pc;
}

u32 w65816_device::get_state_base() const
{
	return m_inst_state_base;
}

// Select the instruction bank from E, M and X.  Called after anything that can
// change them: XCE, REP, SEP, PLP and RTI.
void w65816_device::update_state_base()
{
	if(m_E) {
		// M and X read as 1 in emulation mode and the index high bytes are 0
		m_P |= F_M | F_X;
		m_inst_state_base = STATE_E;
		return;
	}
	m_inst_state_base = STATE_M0X0 + ((m_P & F_M) ? 0x100 : 0) + ((m_P & F_X) ? 0x200 : 0);
}

void w65816_device::state_import(const device_state_entry &entry)
{
	switch(entry.index()) {
	case STATE_GENFLAGS:
	case W65816_P:
	case W65816_E:
		if(m_E) {
			m_P |= F_M | F_X;
			m_SP = 0x0100 | (m_SP & 0xff);
		}
		if(m_P & F_X) {
			m_X &= 0xff;
			m_Y &= 0xff;
		}
		update_state_base();
		break;
	case W65816_S:
		if(m_E)
			m_SP = 0x0100 | (m_SP & 0xff);
		break;
	case W65816_PC:
		m_PC = m_NPC;
		m_irq_taken = false;
		// force substate to 0 so we start at the beginning of the instruction at the new PC
		m_inst_substate = 0;
		prefetch_start();
		m_IR = read_sync(m_PC);
		prefetch_end();
		m_PPC = m_NPC;
		m_inst_state = m_IR | m_inst_state_base;
		break;
	}
}

void w65816_device::state_export(const device_state_entry &entry)
{
	switch(entry.index()) {
	case STATE_GENPC:     m_XPC = (m_PB << 16) | m_PPC; break;
	case STATE_GENPCBASE: m_XPC = (m_PB << 16) | m_NPC; break;
	}
}

void w65816_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch(entry.index()) {
	case STATE_GENFLAGS:
	case W65816_P:
		str = string_format("%c%c%c%c%c%c%c%c",
						m_P & F_N ? 'N' : '.',
						m_P & F_V ? 'V' : '.',
						m_E ? 'E' : (m_P & F_M ? 'M' : '.'),
						m_E ? '-' : (m_P & F_X ? 'X' : '.'),
						m_P & F_D ? 'D' : '.',
						m_P & F_I ? 'I' : '.',
						m_P & F_Z ? 'Z' : '.',
						m_P & F_C ? 'C' : '.');
		break;
	}
}

std::unique_ptr<util::disasm_interface> w65816_device::create_disassembler()
{
	return std::make_unique<w65816_disassembler>(this);
}

void w65816_device::prefetch_start()
{
	if(m_E)
		m_SP = 0x0100 | (m_SP & 0xff);
	m_sync = true;
	if(!m_sync_w.isunset())
		m_sync_w(ASSERT_LINE);
	m_NPC = m_PC;
}

void w65816_device::prefetch_end()
{
	m_sync = false;
	if(!m_sync_w.isunset())
		m_sync_w(CLEAR_LINE);

	if((m_nmi_pending || (m_irq_state && !(m_P & F_I))) && !m_inhibit_interrupts) {
		m_irq_taken = true;
		m_IR = 0x00;
	} else
		m_PC++;
}

void w65816_device::prefetch_end_noirq()
{
	m_sync = false;
	if(!m_sync_w.isunset())
		m_sync_w(CLEAR_LINE);
	m_PC++;
}

void w65816_device::set_nz(uint8_t v)
{
	m_P &= ~(F_Z|F_N);
	if(v & 0x80)
		m_P |= F_N;
	if(!v)
		m_P |= F_Z;
}

void w65816_device::set_nz16(uint16_t v)
{
	m_P &= ~(F_Z|F_N);
	if(v & 0x8000)
		m_P |= F_N;
	if(!v)
		m_P |= F_Z;
}

// ---------------------------------------------------------------------------
// ALU
//
// The decimal paths adjust one nibble at a time, carrying between them.  This
// is the formulation g65816 and bsnes both use; on the 65816 (unlike the
// 65C02) it costs no extra cycle, and N, V and Z all come out valid.
// ---------------------------------------------------------------------------

void w65816_device::do_adc(uint8_t val)
{
	int32_t result;
	uint8_t a = m_A;

	if(!(m_P & F_D))
		result = a + val + (m_P & F_C ? 1 : 0);
	else {
		result = (a & 0x0f) + (val & 0x0f) + (m_P & F_C ? 1 : 0);
		if(result > 0x09)
			result += 0x06;
		result = (a & 0xf0) + (val & 0xf0) + (result > 0x0f ? 0x10 : 0) + (result & 0x0f);
	}

	m_P &= ~(F_V|F_C);
	if(~(a ^ val) & (a ^ result) & 0x80)
		m_P |= F_V;
	if((m_P & F_D) && result > 0x9f)
		result += 0x60;
	if(result > 0xff)
		m_P |= F_C;

	m_A = set_l(m_A, result);
	set_nz(m_A);
}

void w65816_device::do_adc16(uint16_t val)
{
	int32_t result;
	uint16_t a = m_A;

	if(!(m_P & F_D))
		result = a + val + (m_P & F_C ? 1 : 0);
	else {
		result = (a & 0x000f) + (val & 0x000f) + (m_P & F_C ? 1 : 0);
		if(result > 0x0009)
			result += 0x0006;
		result = (a & 0x00f0) + (val & 0x00f0) + (result > 0x000f ? 0x0010 : 0) + (result & 0x000f);
		if(result > 0x009f)
			result += 0x0060;
		result = (a & 0x0f00) + (val & 0x0f00) + (result > 0x00ff ? 0x0100 : 0) + (result & 0x00ff);
		if(result > 0x09ff)
			result += 0x0600;
		result = (a & 0xf000) + (val & 0xf000) + (result > 0x0fff ? 0x1000 : 0) + (result & 0x0fff);
	}

	m_P &= ~(F_V|F_C);
	if(~(a ^ val) & (a ^ result) & 0x8000)
		m_P |= F_V;
	if((m_P & F_D) && result > 0x9fff)
		result += 0x6000;
	if(result > 0xffff)
		m_P |= F_C;

	m_A = result;
	set_nz16(m_A);
}

void w65816_device::do_sbc(uint8_t val)
{
	int32_t result;
	uint8_t a = m_A;

	val = ~val;
	if(!(m_P & F_D))
		result = a + val + (m_P & F_C ? 1 : 0);
	else {
		result = (a & 0x0f) + (val & 0x0f) + (m_P & F_C ? 1 : 0);
		if(result <= 0x0f)
			result -= 0x06;
		result = (a & 0xf0) + (val & 0xf0) + (result > 0x0f ? 0x10 : 0) + (result & 0x0f);
	}

	m_P &= ~(F_V|F_C);
	if(~(a ^ val) & (a ^ result) & 0x80)
		m_P |= F_V;
	if((m_P & F_D) && result <= 0xff)
		result -= 0x60;
	if(result > 0xff)
		m_P |= F_C;

	m_A = set_l(m_A, result);
	set_nz(m_A);
}

void w65816_device::do_sbc16(uint16_t val)
{
	int32_t result;
	uint16_t a = m_A;

	val = ~val;
	if(!(m_P & F_D))
		result = a + val + (m_P & F_C ? 1 : 0);
	else {
		result = (a & 0x000f) + (val & 0x000f) + (m_P & F_C ? 1 : 0);
		if(result <= 0x000f)
			result -= 0x0006;
		result = (a & 0x00f0) + (val & 0x00f0) + (result > 0x000f ? 0x0010 : 0) + (result & 0x000f);
		if(result <= 0x00ff)
			result -= 0x0060;
		result = (a & 0x0f00) + (val & 0x0f00) + (result > 0x00ff ? 0x0100 : 0) + (result & 0x00ff);
		if(result <= 0x0fff)
			result -= 0x0600;
		result = (a & 0xf000) + (val & 0xf000) + (result > 0x0fff ? 0x1000 : 0) + (result & 0x0fff);
	}

	m_P &= ~(F_V|F_C);
	if(~(a ^ val) & (a ^ result) & 0x8000)
		m_P |= F_V;
	if((m_P & F_D) && result <= 0xffff)
		result -= 0x6000;
	if(result > 0xffff)
		m_P |= F_C;

	m_A = result;
	set_nz16(m_A);
}

void w65816_device::do_cmp(uint16_t val1, uint8_t val2)
{
	uint16_t r = uint8_t(val1) - val2;
	m_P &= ~(F_N|F_Z|F_C);
	if(!uint8_t(r))
		m_P |= F_Z;
	else if(r & 0x80)
		m_P |= F_N;
	if(!(r & 0xff00))
		m_P |= F_C;
}

void w65816_device::do_cmp16(uint16_t val1, uint16_t val2)
{
	uint32_t r = val1 - val2;
	m_P &= ~(F_N|F_Z|F_C);
	if(!uint16_t(r))
		m_P |= F_Z;
	else if(r & 0x8000)
		m_P |= F_N;
	if(!(r & 0xffff0000))
		m_P |= F_C;
}

void w65816_device::do_bit(uint8_t val)
{
	m_P &= ~(F_N|F_Z|F_V);
	if(!(uint8_t(m_A) & val))
		m_P |= F_Z;
	if(val & 0x80)
		m_P |= F_N;
	if(val & 0x40)
		m_P |= F_V;
}

void w65816_device::do_bit16(uint16_t val)
{
	m_P &= ~(F_N|F_Z|F_V);
	if(!(m_A & val))
		m_P |= F_Z;
	if(val & 0x8000)
		m_P |= F_N;
	if(val & 0x4000)
		m_P |= F_V;
}

void w65816_device::do_bit_imm(uint8_t val)
{
	m_P &= ~F_Z;
	if(!(uint8_t(m_A) & val))
		m_P |= F_Z;
}

void w65816_device::do_bit_imm16(uint16_t val)
{
	m_P &= ~F_Z;
	if(!(m_A & val))
		m_P |= F_Z;
}

uint8_t w65816_device::do_asl(uint8_t v)
{
	m_P &= ~F_C;
	if(v & 0x80)
		m_P |= F_C;
	v <<= 1;
	set_nz(v);
	return v;
}

uint16_t w65816_device::do_asl16(uint16_t v)
{
	m_P &= ~F_C;
	if(v & 0x8000)
		m_P |= F_C;
	v <<= 1;
	set_nz16(v);
	return v;
}

uint8_t w65816_device::do_lsr(uint8_t v)
{
	m_P &= ~F_C;
	if(v & 1)
		m_P |= F_C;
	v >>= 1;
	set_nz(v);
	return v;
}

uint16_t w65816_device::do_lsr16(uint16_t v)
{
	m_P &= ~F_C;
	if(v & 1)
		m_P |= F_C;
	v >>= 1;
	set_nz16(v);
	return v;
}

uint8_t w65816_device::do_rol(uint8_t v)
{
	bool c = m_P & F_C;
	m_P &= ~F_C;
	if(v & 0x80)
		m_P |= F_C;
	v <<= 1;
	if(c)
		v |= 0x01;
	set_nz(v);
	return v;
}

uint16_t w65816_device::do_rol16(uint16_t v)
{
	bool c = m_P & F_C;
	m_P &= ~F_C;
	if(v & 0x8000)
		m_P |= F_C;
	v <<= 1;
	if(c)
		v |= 0x0001;
	set_nz16(v);
	return v;
}

uint8_t w65816_device::do_ror(uint8_t v)
{
	bool c = m_P & F_C;
	m_P &= ~F_C;
	if(v & 1)
		m_P |= F_C;
	v >>= 1;
	if(c)
		v |= 0x80;
	set_nz(v);
	return v;
}

uint16_t w65816_device::do_ror16(uint16_t v)
{
	bool c = m_P & F_C;
	m_P &= ~F_C;
	if(v & 1)
		m_P |= F_C;
	v >>= 1;
	if(c)
		v |= 0x8000;
	set_nz16(v);
	return v;
}

uint8_t w65816_device::do_inc(uint8_t v)
{
	v++;
	set_nz(v);
	return v;
}

uint16_t w65816_device::do_inc16(uint16_t v)
{
	v++;
	set_nz16(v);
	return v;
}

uint8_t w65816_device::do_dec(uint8_t v)
{
	v--;
	set_nz(v);
	return v;
}

uint16_t w65816_device::do_dec16(uint16_t v)
{
	v--;
	set_nz16(v);
	return v;
}

// TSB and TRB set Z from the accumulator ANDed with memory, and touch no
// other flag.
uint8_t w65816_device::do_tsb(uint8_t v)
{
	m_P &= ~F_Z;
	if(!(uint8_t(m_A) & v))
		m_P |= F_Z;
	return v | uint8_t(m_A);
}

uint16_t w65816_device::do_tsb16(uint16_t v)
{
	m_P &= ~F_Z;
	if(!(m_A & v))
		m_P |= F_Z;
	return v | m_A;
}

uint8_t w65816_device::do_trb(uint8_t v)
{
	m_P &= ~F_Z;
	if(!(uint8_t(m_A) & v))
		m_P |= F_Z;
	return v & ~uint8_t(m_A);
}

uint16_t w65816_device::do_trb16(uint16_t v)
{
	m_P &= ~F_Z;
	if(!(m_A & v))
		m_P |= F_Z;
	return v & ~m_A;
}

uint8_t w65816_device::mi_default::read(uint32_t adr)
{
	return m_program.read_interruptible(adr & 0xffffff);
}

uint8_t w65816_device::mi_default::read_sync(uint32_t adr)
{
	return m_csprogram.read_interruptible(adr & 0xffffff);
}

uint8_t w65816_device::mi_default::read_arg(uint32_t adr)
{
	return m_cprogram.read_interruptible(adr & 0xffffff);
}

uint8_t w65816_device::mi_default::read_vector(uint16_t adr)
{
	// VPB is asserted for the two vector-fetch cycles (§2.28); a driver that
	// configures AS_VECTORS gets to redirect them.
	if(m_has_vectors)
		return m_vectors.read_interruptible(adr & 0x1f);
	return m_program.read_interruptible(adr);
}

void w65816_device::mi_default::write(uint32_t adr, uint8_t val)
{
	m_program.write_interruptible(adr & 0xffffff, val);
}

#include "cpu/m6502/w65816.hxx"
