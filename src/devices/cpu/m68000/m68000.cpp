// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "m68000.h"
#include "m68kdasm.h"

DEFINE_DEVICE_TYPE(M68000,      m68000_device,      "m68000",       "Motorola MC68000")

std::unique_ptr<util::disasm_interface> m68000_device::create_disassembler()
{
	return std::make_unique<m68k_disassembler>(m68k_disassembler::TYPE_68000);
}

m68000_device::m68000_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) : m68000_device(mconfig, M68000, tag, owner, clock)
{
}

m68000_device::m68000_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: m68000_base_device(mconfig, type, tag, owner, clock),
	  m_cmpild_instr_callback(*this),
	  m_rte_instr_callback(*this),
	  m_tas_write_callback(*this),
	  m_program_config("program", ENDIANNESS_BIG, 16, 24),
	  m_opcodes_config("opcodes", ENDIANNESS_BIG, 16, 24),
	  m_uprogram_config("uprogram", ENDIANNESS_BIG, 16, 24),
	  m_uopcodes_config("uopcodes", ENDIANNESS_BIG, 16, 24),
	  m_cpu_space_config("cpu_space", ENDIANNESS_BIG, 16, 24, 0, address_map_constructor(FUNC(m68000_device::default_autovectors_map), this)),
	  m_mmu(nullptr),
	  m_disable_spaces(false),
	  m_disable_specifics(false),
	  m_disable_interrupt_callback(false)
{
}

void m68000_device::set_current_mmu(mmu *mmu)
{
	m_mmu = mmu;

	if(m_mmu)
		m_mmu->set_super(m_sr & SR_S);
}

void m68000_device::abort_access(u32 reason)
{
	m_post_run = reason;
	m_post_run_cycles = m_icount;
	m_icount = 0;
}

void m68000_device::do_post_run()
{
	m_icount = m_post_run_cycles;
	m_post_run_cycles = 0;
	switch(m_post_run) {
	case PR_BERR:
		m_inst_state = S_BUS_ERROR;
		m_inst_substate = 0;
		m_icount -= 10;
		break;
	}
	m_post_run = 0;
}

u64 m68000_device::vpa_sync(offs_t, u64 current_time)
{
	if(m_last_vpa_time >= current_time)
		return m_last_vpa_time;
	u64 mod = current_time % 10;
	if(mod < 7)
		m_last_vpa_time = current_time - mod + 10;
	else
		m_last_vpa_time = current_time - mod + 20;
	return m_last_vpa_time;
}

u32 m68000_device::vpa_after(offs_t)
{
	return 1;
}

void m68000_device::trigger_bus_error()
{
	abort_access(PR_BERR);
}

void m68000_device::berr_w(u16)
{
	trigger_bus_error();
}

u16 m68000_device::berr_r()
{
	if(!machine().side_effects_disabled())
		trigger_bus_error();
	return 0;
}

bool m68000_device::supervisor_mode() const noexcept
{
	return m_sr & SR_S;
}

u16 m68000_device::get_fc() const noexcept
{
	return (m_base_ssw | (m_sr & SR_S ? SSW_S : 0)) & 7;
}

void m68000_device::execute_run()
{
	m_icount -= m_count_before_instruction_step;
	if(m_icount < 0) {
		m_count_before_instruction_step = -m_icount;
		m_icount = 0;
	} else
		m_count_before_instruction_step = 0;

	for(;;) {
		if(m_icount > 0 && m_inst_substate)
			(this->*(m_handlers_p[m_inst_state]))();

		while(m_icount > 0) {
			if(m_inst_state >= S_first_instruction) {
				m_ipc = m_pc - 2;
				m_irdi = m_ird;

				if(machine().debug_flags & DEBUG_FLAG_ENABLED)
					debugger_instruction_hook(m_ipc);
			}
			(this->*(m_handlers_f[m_inst_state]))();
		}

		if(m_post_run)
			do_post_run();
		else
			break;
	}

	if(m_icount < 0) {
		m_count_before_instruction_step = -m_icount;
		m_icount = 0;
	}
}

device_memory_interface::space_config_vector m68000_device::memory_space_config() const
{
	device_memory_interface::space_config_vector scv;
	scv.push_back(std::make_pair(AS_PROGRAM, &m_program_config));
	if(has_configured_map(AS_OPCODES))
		scv.push_back(std::make_pair(AS_OPCODES, &m_opcodes_config));
	if(has_configured_map(AS_USER_PROGRAM))
		scv.push_back(std::make_pair(AS_USER_PROGRAM, &m_uprogram_config));
	if(has_configured_map(AS_USER_OPCODES))
		scv.push_back(std::make_pair(AS_USER_OPCODES, &m_opcodes_config));
	if(m_cpu_space_id == AS_CPU_SPACE)
		scv.push_back(std::make_pair(AS_CPU_SPACE, &m_cpu_space_config));
	return scv;
}

void m68000_device::default_autovectors_map(address_map &map)
{
	if(m_cpu_space_id == AS_CPU_SPACE && !has_configured_map(AS_CPU_SPACE)) {
		offs_t mask = make_bitmask<offs_t>(m_cpu_space_config.m_addr_width) - 0xf;
		map(mask + 0x3, mask + 0x3).before_time(*this, FUNC(m68000_device::vpa_sync)).after_delay(*this, FUNC(m68000_device::vpa_after)).lr8(NAME([] () -> u8 { return autovector(1); }));
		map(mask + 0x5, mask + 0x5).before_time(*this, FUNC(m68000_device::vpa_sync)).after_delay(*this, FUNC(m68000_device::vpa_after)).lr8(NAME([] () -> u8 { return autovector(2); }));
		map(mask + 0x7, mask + 0x7).before_time(*this, FUNC(m68000_device::vpa_sync)).after_delay(*this, FUNC(m68000_device::vpa_after)).lr8(NAME([] () -> u8 { return autovector(3); }));
		map(mask + 0x9, mask + 0x9).before_time(*this, FUNC(m68000_device::vpa_sync)).after_delay(*this, FUNC(m68000_device::vpa_after)).lr8(NAME([] () -> u8 { return autovector(4); }));
		map(mask + 0xb, mask + 0xb).before_time(*this, FUNC(m68000_device::vpa_sync)).after_delay(*this, FUNC(m68000_device::vpa_after)).lr8(NAME([] () -> u8 { return autovector(5); }));
		map(mask + 0xd, mask + 0xd).before_time(*this, FUNC(m68000_device::vpa_sync)).after_delay(*this, FUNC(m68000_device::vpa_after)).lr8(NAME([] () -> u8 { return autovector(6); }));
		map(mask + 0xf, mask + 0xf).before_time(*this, FUNC(m68000_device::vpa_sync)).after_delay(*this, FUNC(m68000_device::vpa_after)).lr8(NAME([] () -> u8 { return autovector(7); }));
	}
}

void m68000_device::device_start()
{
	init_decode_table();

	m_cmpild_instr_callback.resolve();
	m_rte_instr_callback.resolve();
	m_tas_write_callback.resolve();

	if(!m_disable_spaces) {
		m_s_program = &space(AS_PROGRAM);
		m_s_opcodes = has_space(AS_OPCODES) ? &space(AS_OPCODES) : m_s_program;
		m_s_uprogram = has_space(AS_USER_PROGRAM) ? &space(AS_USER_PROGRAM) : m_s_program;
		m_s_uopcodes = has_space(AS_USER_OPCODES) ? &space(AS_USER_OPCODES) : has_space(AS_USER_PROGRAM) ? m_s_uprogram : m_s_opcodes;
		m_s_cpu_space = &space(m_cpu_space_id);
	}

	if(!(m_disable_specifics || m_disable_spaces)) {
		m_s_program->specific(m_r_program);
		m_s_opcodes->specific(m_r_opcodes);
		m_s_uprogram->specific(m_r_uprogram);
		m_s_uopcodes->specific(m_r_uopcodes);
		m_s_cpu_space->specific(m_cpu_space);
	}

	if(m_mmu) {
		m_handlers_f = s_handlers_if;
		m_handlers_p = s_handlers_ip;
	} else {
		m_handlers_f = s_handlers_df;
		m_handlers_p = s_handlers_dp;
	}

	save_item(NAME(m_da));
	save_item(NAME(m_ipc));
	save_item(NAME(m_pc));
	save_item(NAME(m_au));
	save_item(NAME(m_at));
	save_item(NAME(m_aob));
	save_item(NAME(m_dt));
	save_item(NAME(m_int_vector));
	save_item(NAME(m_sp));
	save_item(NAME(m_bcount));
	save_item(NAME(m_count_before_instruction_step));
	save_item(NAME(m_t));
	save_item(NAME(m_movems));
	save_item(NAME(m_isr));
	save_item(NAME(m_sr));
	save_item(NAME(m_new_sr));
	save_item(NAME(m_dbin));
	save_item(NAME(m_dbout));
	save_item(NAME(m_edb));
	save_item(NAME(m_irc));
	save_item(NAME(m_ir));
	save_item(NAME(m_ird));
	save_item(NAME(m_ftu));
	save_item(NAME(m_aluo));
	save_item(NAME(m_alue));
	save_item(NAME(m_alub));
	save_item(NAME(m_movemr));
	save_item(NAME(m_irdi));
	save_item(NAME(m_base_ssw));
	save_item(NAME(m_ssw));
	save_item(NAME(m_dcr));

	save_item(NAME(m_virq_state));
	save_item(NAME(m_nmi_pending));
	save_item(NAME(m_int_level));
	save_item(NAME(m_int_next_state));
	save_item(NAME(m_nmi_uses_generic));
	save_item(NAME(m_last_vpa_time));

	save_item(NAME(m_inst_state));
	save_item(NAME(m_inst_substate));
	save_item(NAME(m_next_state));
	save_item(NAME(m_post_run));
	save_item(NAME(m_post_run_cycles));

	memset(m_da, 0, sizeof(m_da));
	m_ipc = 0;
	m_pc = 0;
	m_au = 0;
	m_at = 0;
	m_aob = 0;
	m_dt = 0;
	m_int_vector = 0;
	m_sp = 0;
	m_bcount = 0;
	m_count_before_instruction_step = 0;
	m_t = 0;
	m_movems = 0;
	m_isr = 0;
	m_sr = 0;
	m_new_sr = 0;
	m_dbin = 0;
	m_dbout = 0;
	m_edb = 0;
	m_irc = 0;
	m_ir = 0;
	m_ird = 0;
	m_ftu = 0;
	m_aluo = 0;
	m_alue = 0;
	m_alub = 0;
	m_movemr = 0;
	m_irdi = 0;
	m_base_ssw = 0;
	m_ssw = 0;
	m_dcr = 0;

	m_virq_state = 0;
	m_nmi_pending = 0;
	m_int_level = 0;
	m_int_next_state = 0;
	m_nmi_uses_generic = false;
	m_last_vpa_time = 0;

	m_inst_state = 0;
	m_inst_substate = 0;
	m_next_state = 0;
	m_post_run = 0;
	m_post_run_cycles = 0;

	state_add(STATE_GENPCBASE, "CURPC", m_ipc).callimport();
	state_add(STATE_GENPC,     "PC",    m_pc).callimport();
	state_add(M68K_IR,         "IR",    m_ir);
	state_add(STATE_GENFLAGS,  "GENFLAGS", m_sr).noshow().callexport().formatstr("%16s");
	state_add(M68K_SR,         "SR",  m_sr).callimport();
	for(int r = 0; r != 8; r++)
		state_add(M68K_D0 + r, string_format("D%d", r).c_str(), m_da[r]);
	for(int r = 0; r != 7; r++)
		state_add(M68K_A0 + r, string_format("A%d", r).c_str(), m_da[r+8]);
	state_add(M68K_USP, "USP", m_da[15]);
	state_add(M68K_SP,  "SP", m_da[16]);

	set_icountptr(m_icount);
}

void m68000_device::state_import(const device_state_entry &entry)
{
	switch(entry.index()) {
	case STATE_GENPC:
		m_ipc = m_pc;
		[[fallthrough]];

	case STATE_GENPCBASE: {
		m_pc = m_ipc+2;
		m_au = m_ipc+4;
		auto dis = machine().disable_side_effects();
		m_ir = m_ird = m_irdi = m_opcodes.read_word(m_ipc);
		m_irc = m_dbin = m_opcodes.read_word(m_pc);
		set_ftu_const();
		m_inst_state = m_decode_table[m_ird];
		m_inst_substate = 0;
		break;
	}
	case M68K_SR:
		m_sr &= (SR_SR|SR_CCR);
		break;
	}
}

void m68000_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch(entry.index()) {
	case STATE_GENFLAGS:
		str = util::string_format("%c%c%d %c%c%c%c%c",
								  m_sr & SR_T ? 'T' : '.',
								  m_sr & SR_S ? 'S' : '.',
								  (m_sr & SR_I) >> 8,
								  m_sr & SR_X ? 'X' : '.',
								  m_sr & SR_N ? 'N' : '.',
								  m_sr & SR_Z ? 'Z' : '.',
								  m_sr & SR_V ? 'V' : '.',
								  m_sr & SR_C ? 'C' : '.');
		break;
	}
}

void m68000_device::device_reset()
{
	m_inst_state = S_RESET;
	m_inst_substate = 0;
	m_count_before_instruction_step = 0;
	m_post_run = 0;
	m_post_run_cycles = 0;
	m_last_vpa_time = 0;
	update_user_super();
}

void m68000_device::update_user_super()
{
	if(m_sr & SR_S) {
		m_sp = 16;
		m_program = m_r_program;
		m_opcodes = m_r_opcodes;
		if(m_mmu)
			m_mmu->set_super(true);
	} else {
		m_sp = 15;
		m_program = m_r_uprogram;
		m_opcodes = m_r_uopcodes;
		if(m_mmu)
			m_mmu->set_super(false);
	}
}

void m68000_device::execute_set_input(int inputnum, int state)
{
	if(inputnum == INPUT_LINE_NMI) {
		inputnum = 7;
		m_nmi_uses_generic = true;
	} else if(inputnum == 7)
		m_nmi_uses_generic = false;

	if(inputnum > (m_interrupt_mixer ? 7 : 3))
		return;

	u32 old_level = m_int_level;
	u32 vstate = m_virq_state;
	u32 blevel;

	if(state == ASSERT_LINE)
		vstate |= 1 << inputnum;
	else
		vstate &= ~(1 << inputnum);
	m_virq_state = vstate;

	if(m_interrupt_mixer) {
		for(blevel = 7; blevel > 0; blevel--)
			if(vstate & (1 << blevel))
				break;
	} else
		blevel = vstate;

	m_int_level = blevel;

	/* A transition from < 7 to 7 always interrupts (NMI) */
	/* Note: Level 7 can also level trigger like a normal IRQ */
	// FIXME: This may cause unintended level 7 interrupts if one or two IPL lines are asserted
	// immediately before others are cleared. The actual 68000 imposes an input hold time.
	if(old_level != 7 && m_int_level == 7)
		m_nmi_pending = true;

	update_interrupt();
}

void m68000_device::update_interrupt()
{
	if(m_nmi_pending)
		m_int_next_state = (7 << 24) | S_INTERRUPT;
	else if(m_int_level > ((m_sr >> 8) & 7))
		m_int_next_state = (m_int_level << 24) | S_INTERRUPT;
	else
		m_int_next_state = 0;
}

void m68000_device::init_decode_table()
{
	m_decode_table.resize(0x10000, S_ILLEGAL);

	for(int i=0; s_packed_decode_table[i].mask; i++) {
		u16 value = s_packed_decode_table[i].value;
		u16 mask = s_packed_decode_table[i].mask;
		u16 state = s_packed_decode_table[i].state;

		u16 cvalue = 0;
		do {
			//          logerror("%04x/%04x %04x %4d\n", value, mask, cvalue, state);
			if(m_decode_table[value | cvalue] == S_ILLEGAL)
				m_decode_table[value | cvalue] = state;
			else if(((value | cvalue) & 0xf0ff) != 0x6000)
				logerror("Collision on %04x (%d / %d)\n", value | cvalue, state, m_decode_table[value | cvalue]);
			cvalue = ((cvalue | mask) + 1) & ~mask;
		} while(cvalue);
	}
}

void m68000_device::set_ftu_const()
{
	switch(m_ird >> 12) {
	case 0x4: m_ftu = 0x80; break;
	case 0x5: case 0xe: m_ftu = (m_ird >> 9) & 7; if(!m_ftu) m_ftu = 8; break;
	case 0x6: case 0x7: m_ftu = s8(m_ird); break;
	case 0x8: case 0xc: m_ftu = 0xf; break;
	default: m_ftu = 0; break;
	}
}

void m68000_device::start_interrupt_vector_lookup()
{
	// flag for berr -> spurious

	int level = m_next_state >> 24;
	if(!m_disable_interrupt_callback) {
		if(m_interrupt_mixer)
			standard_irq_callback(level == 7 && m_nmi_uses_generic ? INPUT_LINE_NMI : level, m_pc);
		else {
			for(int i=0; i<3; i++)
				if(level & (1<<i))
					standard_irq_callback(i, m_pc);
		}
	}

	// Clear the nmi flag
	if(level == 7) {
		m_nmi_pending = false;
		update_interrupt();
	}
}

void m68000_device::end_interrupt_vector_lookup()
{
	m_int_vector = (m_edb & 0xff) << 2;
	m_int_next_state = 0;
}
