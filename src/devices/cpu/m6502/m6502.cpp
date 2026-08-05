// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m6502.cpp

    MOS Technology 6502, original NMOS variant

***************************************************************************/

#include "emu.h"
#include "m6502.h"
#include "m6502d.h"

DEFINE_DEVICE_TYPE(M6502, m6502_device, "m6502", "MOS Technology 6502")
DEFINE_DEVICE_TYPE(M6512, m6512_device, "m6512", "MOS Technology 6512")

m6502_device::m6502_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	m6502_device(mconfig, M6502, tag, owner, clock)
{
}

m6512_device::m6512_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	m6502_device(mconfig, M6512, tag, owner, clock)
{
}

m6502_device::m6502_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	cpu_device(mconfig, type, tag, owner, clock),
	m_sync_w(*this),
	m_program_config("program", ENDIANNESS_LITTLE, 8, 16),
	m_sprogram_config("decrypted_opcodes", ENDIANNESS_LITTLE, 8, 16),
	m_mintf(nullptr),
	m_uses_custom_memory_interface(false)
{
}

void m6502_device::device_start()
{
	if(!m_uses_custom_memory_interface)
		m_mintf = space(AS_PROGRAM).addr_width() > 14 ? std::make_unique<mi_default>() : std::make_unique<mi_default14>();

	init();
}

void m6502_device::init()
{
	if(m_mintf) {
		// specific group 1-14 or 15-31
		if(space(AS_PROGRAM).addr_width() > 14) {
			space(AS_PROGRAM).specific(m_mintf->m_program);
			m_mintf->m_cprogram = m_mintf->m_program;
			if(has_space(AS_OPCODES))
				space(AS_OPCODES).specific(m_mintf->m_csprogram);
			else
				m_mintf->m_csprogram = m_mintf->m_program;
		} else {
			space(AS_PROGRAM).specific(m_mintf->m_program14);
			m_mintf->m_cprogram14 = m_mintf->m_program14;
			if(has_space(AS_OPCODES))
				space(AS_OPCODES).specific(m_mintf->m_csprogram14);
			else
				m_mintf->m_csprogram14 = m_mintf->m_program14;
		}
	}

	state_add(STATE_GENPC,     "GENPC",     m_XPC).callexport().noshow();
	state_add(STATE_GENPCBASE, "CURPC",     m_XPC).callexport().noshow();
	state_add(STATE_GENFLAGS,  "GENFLAGS",  m_P).callimport().formatstr("%6s").noshow();
	state_add(M6502_PC,        "PC",        m_NPC).callimport();
	state_add(M6502_A,         "A",         m_A);
	state_add(M6502_X,         "X",         m_X);
	state_add(M6502_Y,         "Y",         m_Y);
	state_add(M6502_P,         "P",         m_P).callimport();
	state_add(M6502_S,         "SP",        m_SP);
	state_add(M6502_IR,        "IR",        m_IR);

	save_item(NAME(m_PC));
	save_item(NAME(m_NPC));
	save_item(NAME(m_PPC));
	save_item(NAME(m_A));
	save_item(NAME(m_X));
	save_item(NAME(m_Y));
	save_item(NAME(m_P));
	save_item(NAME(m_SP));
	save_item(NAME(m_TMP));
	save_item(NAME(m_TMP2));
	save_item(NAME(m_IR));
	save_item(NAME(m_nmi_state));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_apu_irq_state));
	save_item(NAME(m_v_state));
	save_item(NAME(m_nmi_pending));
	save_item(NAME(m_irq_taken));
	save_item(NAME(m_inst_state));
	save_item(NAME(m_inst_substate));
	save_item(NAME(m_inst_state_base));
	save_item(NAME(m_inhibit_interrupts));

	set_icountptr(m_icount);

	m_XPC = 0x0000;
	m_PPC = 0x0000;
	m_PC = 0x0000;
	m_NPC = 0x0000;
	m_A = 0x00;
	m_X = 0x80;
	m_Y = 0x00;
	m_P = 0x36;
	m_SP = 0x0100;
	m_TMP = 0x0000;
	m_TMP2 = 0x00;
	m_IR = 0x00;
	m_nmi_state = false;
	m_irq_state = false;
	m_apu_irq_state = false;
	m_v_state = false;
	m_nmi_pending = false;
	m_irq_taken = false;
	m_inst_state = STATE_RESET;
	m_inst_substate = 0;
	m_inst_state_base = 0;
	m_sync = false;
	m_inhibit_interrupts = false;
	m_count_before_instruction_step = 0;
}

void m6502_device::device_reset()
{
	m_inst_state = STATE_RESET;
	m_inst_substate = 0;
	m_inst_state_base = 0;
	m_nmi_pending = false;
	m_irq_taken = false;
	m_sync = false;
	m_sync_w(CLEAR_LINE);
	m_inhibit_interrupts = false;
}


uint32_t m6502_device::execute_min_cycles() const noexcept
{
	return 1;
}

uint32_t m6502_device::execute_max_cycles() const noexcept
{
	return 10;
}

bool m6502_device::execute_input_edge_triggered(int inputnum) const noexcept
{
	return inputnum == NMI_LINE || inputnum == V_LINE;
}

void m6502_device::do_adc_d(uint8_t val)
{
	uint8_t c = m_P & F_C ? 1 : 0;
	m_P &= ~(F_N|F_V|F_Z|F_C);
	uint8_t al = (m_A & 15) + (val & 15) + c;
	if(al > 9)
		al += 6;
	uint8_t ah = (m_A >> 4) + (val >> 4) + (al > 15);
	if(!uint8_t(m_A + val + c))
		m_P |= F_Z;
	else if(ah & 8)
		m_P |= F_N;
	if(~(m_A^val) & (m_A^(ah << 4)) & 0x80)
		m_P |= F_V;
	if(ah > 9)
		ah += 6;
	if(ah > 15)
		m_P |= F_C;
	m_A = (ah << 4) | (al & 15);
}

void m6502_device::do_adc_nd(uint8_t val)
{
	uint16_t sum;
	sum = m_A + val + (m_P & F_C ? 1 : 0);
	m_P &= ~(F_N|F_V|F_Z|F_C);
	if(!uint8_t(sum))
		m_P |= F_Z;
	else if(int8_t(sum) < 0)
		m_P |= F_N;
	if(~(m_A^val) & (m_A^sum) & 0x80)
		m_P |= F_V;
	if(sum & 0xff00)
		m_P |= F_C;
	m_A = sum;
}

void m6502_device::do_adc(uint8_t val)
{
	if(m_P & F_D)
		do_adc_d(val);
	else
		do_adc_nd(val);
}

void m6502_device::do_arr_nd()
{
	bool c = m_P & F_C;
	m_P &= ~(F_N|F_Z|F_C|F_V);
	m_A >>= 1;
	if(c)
		m_A |= 0x80;
	if(!m_A)
		m_P |= F_Z;
	else if(int8_t(m_A)<0)
		m_P |= F_N;
	if(m_A & 0x40)
		m_P |= F_V|F_C;
	if(m_A & 0x20)
		m_P ^= F_V;
}

void m6502_device::do_arr_d()
{
	// The adc/ror interaction gives an extremely weird result
	bool c = m_P & F_C;
	m_P &= ~(F_N|F_Z|F_C|F_V);
	uint8_t a = m_A >> 1;
	if(c)
		a |= 0x80;
	if(!a)
		m_P |= F_Z;
	else if(int8_t(a) < 0)
		m_P |= F_N;
	if((a ^ m_A) & 0x40)
		m_P |= F_V;

	if((m_A & 0x0f) >= 0x05)
		a = ((a + 6) & 0x0f) | (a & 0xf0);

	if((m_A & 0xf0) >= 0x50) {
		a += 0x60;
		m_P |= F_C;
	}
	m_A = a;
}

void m6502_device::do_arr()
{
	if(m_P & F_D)
		do_arr_d();
	else
		do_arr_nd();
}

void m6502_device::do_cmp(uint8_t val1, uint8_t val2)
{
	m_P &= ~(F_N|F_Z|F_C);
	uint16_t r = val1-val2;
	if(!r)
		m_P |= F_Z;
	else if(int8_t(r) < 0)
		m_P |= F_N;
	if(!(r & 0xff00))
		m_P |= F_C;
}

void m6502_device::do_sbc_d(uint8_t val)
{
	uint8_t c = m_P & F_C ? 0 : 1;
	m_P &= ~(F_N|F_V|F_Z|F_C);
	uint16_t diff = m_A - val - c;
	uint8_t al = (m_A & 15) - (val & 15) - c;
	uint8_t ah = (m_A >> 4) - (val >> 4) - (int8_t(al) < 0);
	if(!uint8_t(diff))
		m_P |= F_Z;
	else if(diff & 0x80)
		m_P |= F_N;
	if((m_A^val) & (m_A^diff) & 0x80)
		m_P |= F_V;
	if(!(diff & 0xff00))
		m_P |= F_C;
	if(int8_t(al) < 0)
		al -= 6;
	if(int8_t(ah) < 0)
		ah -= 6;
	m_A = (ah << 4) | (al & 15);
}

void m6502_device::do_sbc_nd(uint8_t val)
{
	uint16_t diff = m_A - val - (m_P & F_C ? 0 : 1);
	m_P &= ~(F_N|F_V|F_Z|F_C);
	if(!uint8_t(diff))
		m_P |= F_Z;
	else if(int8_t(diff) < 0)
		m_P |= F_N;
	if((m_A^val) & (m_A^diff) & 0x80)
		m_P |= F_V;
	if(!(diff & 0xff00))
		m_P |= F_C;
	m_A = diff;
}

void m6502_device::do_sbc(uint8_t val)
{
	if(m_P & F_D)
		do_sbc_d(val);
	else
		do_sbc_nd(val);
}

void m6502_device::do_bit(uint8_t val)
{
	m_P &= ~(F_N|F_Z|F_V);
	uint8_t r = m_A & val;
	if(!r)
		m_P |= F_Z;
	if(val & 0x80)
		m_P |= F_N;
	if(val & 0x40)
		m_P |= F_V;
}

uint8_t m6502_device::do_asl(uint8_t v)
{
	m_P &= ~(F_N|F_Z|F_C);
	uint8_t r = v<<1;
	if(!r)
		m_P |= F_Z;
	else if(int8_t(r) < 0)
		m_P |= F_N;
	if(v & 0x80)
		m_P |= F_C;
	return r;
}

uint8_t m6502_device::do_lsr(uint8_t v)
{
	m_P &= ~(F_N|F_Z|F_C);
	if(v & 1)
		m_P |= F_C;
	v >>= 1;
	if(!v)
		m_P |= F_Z;
	return v;
}

uint8_t m6502_device::do_ror(uint8_t v)
{
	bool c = m_P & F_C;
	m_P &= ~(F_N|F_Z|F_C);
	if(v & 1)
		m_P |= F_C;
	v >>= 1;
	if(c)
		v |= 0x80;
	if(!v)
		m_P |= F_Z;
	else if(int8_t(v)<0)
		m_P |= F_N;
	return v;
}

uint8_t m6502_device::do_rol(uint8_t v)
{
	bool c = m_P & F_C;
	m_P &= ~(F_N|F_Z|F_C);
	if(v & 0x80)
		m_P |= F_C;
	v <<= 1;
	if(c)
		v |= 0x01;
	if(!v)
		m_P |= F_Z;
	else if(int8_t(v)<0)
		m_P |= F_N;
	return v;
}

uint8_t m6502_device::do_asr(uint8_t v)
{
	m_P &= ~(F_N|F_Z|F_C);
	if(v & 1)
		m_P |= F_C;
	v >>= 1;
	if(!v)
		m_P |= F_Z;
	else if(v & 0x40) {
		m_P |= F_N;
		v |= 0x80;
	}
	return v;
}

offs_t m6502_device::pc_to_external(u16 pc)
{
	return pc;
}

void m6502_device::execute_run()
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

void m6502_device::execute_set_input(int inputnum, int state)
{
	switch(inputnum) {
	case IRQ_LINE: m_irq_state = state == ASSERT_LINE; break;
	case APU_IRQ_LINE: m_apu_irq_state = state == ASSERT_LINE; break;
	case NMI_LINE:
		// don't accept NMI edge at exactly the same time RESET is cleared
		if(!m_nmi_state && state == ASSERT_LINE && total_cycles())
			m_nmi_pending = true;
		m_nmi_state = state == ASSERT_LINE;
		break;
	case V_LINE:
		// don't accept SO edge at exactly the same time RESET is cleared
		if(!m_v_state && state == ASSERT_LINE && total_cycles())
			m_P |= F_V;
		m_v_state = state == ASSERT_LINE;
		break;
	}
}


device_memory_interface::space_config_vector m6502_device::memory_space_config() const
{
	if(has_configured_map(AS_OPCODES))
		return space_config_vector {
			std::make_pair(AS_PROGRAM, &m_program_config),
			std::make_pair(AS_OPCODES, &m_sprogram_config)
		};
	else
		return space_config_vector {
			std::make_pair(AS_PROGRAM, &m_program_config)
		};
}


void m6502_device::state_import(const device_state_entry &entry)
{
	switch(entry.index()) {
	case STATE_GENFLAGS:
	case M6502_P:
		m_P = m_P | (F_B|F_E);
		break;
	case M6502_PC:
		m_PC = m_NPC;
		m_irq_taken = false;
		prefetch_start();
		m_IR = read_sync(m_PC);
		prefetch_end();
		m_PPC = m_NPC;
		m_inst_state = m_IR | m_inst_state_base;
		break;
	}
}

void m6502_device::state_export(const device_state_entry &entry)
{
	switch(entry.index()) {
	case STATE_GENPC:     m_XPC = pc_to_external(m_PPC); break;
	case STATE_GENPCBASE: m_XPC = pc_to_external(m_NPC); break;
	}
}

void m6502_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch(entry.index()) {
	case STATE_GENFLAGS:
	case M6502_P:
		str = string_format("%c%c%c%c%c%c",
						m_P & F_N ? 'N' : '.',
						m_P & F_V ? 'V' : '.',
						m_P & F_D ? 'D' : '.',
						m_P & F_I ? 'I' : '.',
						m_P & F_Z ? 'Z' : '.',
						m_P & F_C ? 'C' : '.');
		break;
	}
}

void m6502_device::prefetch_start()
{
	m_sync = true;
	if(!m_sync_w.isunset())
		m_sync_w(ASSERT_LINE);
	m_NPC = m_PC;
}

void m6502_device::prefetch_end()
{
	m_sync = false;
	if(!m_sync_w.isunset())
		m_sync_w(CLEAR_LINE);

	if((m_nmi_pending || ((m_irq_state || m_apu_irq_state) && !(m_P & F_I))) && !m_inhibit_interrupts) {
		m_irq_taken = true;
		m_IR = 0x00;
	} else
		m_PC++;
}

void m6502_device::prefetch_end_noirq()
{
	m_sync = false;
	if(!m_sync_w.isunset())
		m_sync_w(CLEAR_LINE);
	m_PC++;
}

void m6502_device::set_nz(uint8_t v)
{
	m_P &= ~(F_Z|F_N);
	if(v & 0x80)
		m_P |= F_N;
	if(!v)
		m_P |= F_Z;
}

std::unique_ptr<util::disasm_interface> m6502_device::create_disassembler()
{
	return std::make_unique<m6502_disassembler>();
}

uint8_t m6502_device::memory_interface::read_9(uint16_t adr)
{
	return read(adr);
}

void m6502_device::memory_interface::write_9(uint16_t adr, uint8_t val)
{
	write(adr, val);
}


uint8_t m6502_device::mi_default::read(uint16_t adr)
{
	return m_program.read_interruptible(adr);
}

uint8_t m6502_device::mi_default::read_sync(uint16_t adr)
{
	return m_csprogram.read_interruptible(adr);
}

uint8_t m6502_device::mi_default::read_arg(uint16_t adr)
{
	return m_cprogram.read_interruptible(adr);
}

void m6502_device::mi_default::write(uint16_t adr, uint8_t val)
{
	m_program.write_interruptible(adr, val);
}

uint8_t m6502_device::mi_default14::read(uint16_t adr)
{
	return m_program14.read_interruptible(adr);
}

uint8_t m6502_device::mi_default14::read_sync(uint16_t adr)
{
	return m_csprogram14.read_interruptible(adr);
}

uint8_t m6502_device::mi_default14::read_arg(uint16_t adr)
{
	return m_cprogram14.read_interruptible(adr);
}

void m6502_device::mi_default14::write(uint16_t adr, uint8_t val)
{
	m_program14.write_interruptible(adr, val);
}


#include "cpu/m6502/m6502.hxx"
