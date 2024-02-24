// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h8.cpp

    H8-300 base cpu emulation

    TODO:
    - use logmacro and be quiet by default, same for H8 peripherals that
      currently have "static constexpr int V"
    - NVRAM won't work properly when it goes into SSBY (software standby
      mode) and the power button triggers an IRQ to wake up instead of RES.
      Obviously, MAME always starts at reset-phase at power-on, so it's more
      like a 'known issue' instead of a TODO since it can't really be fixed.
    - add STBY pin (hardware standby mode, can only wake up with reset)

***************************************************************************/

#include "emu.h"
#include "h8.h"
#include "h8_dma.h"
#include "h8_dtc.h"
#include "h8_port.h"
#include "h8d.h"

h8_device::h8_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor map_delegate) :
	cpu_device(mconfig, type, tag, owner, clock),
	device_nvram_interface(mconfig, *this),
	m_program_config("program", ENDIANNESS_BIG, 16, 16, 0, map_delegate),
	m_internal_ram(*this, "internal_ram"),
	m_read_adc(*this, 0),
	m_read_port(*this, 0xff),
	m_write_port(*this),
	m_sci(*this, "sci%u", 0),
	m_sci_tx(*this),
	m_sci_clk(*this),
	m_standby_cb(*this),
	m_PPC(0), m_NPC(0), m_PC(0), m_PIR(0), m_EXR(0), m_CCR(0), m_MAC(0), m_MACF(0),
	m_TMP1(0), m_TMP2(0), m_TMPR(0), m_inst_state(0), m_inst_substate(0), m_icount(0), m_bcount(0),
	m_irq_vector(0), m_taken_irq_vector(0), m_irq_level(0), m_taken_irq_level(0), m_irq_required(false), m_irq_nmi(false),
	m_standby_pending(false), m_nvram_defval(0), m_nvram_battery(true)
{
	m_supports_advanced = false;
	m_mode_advanced = false;
	m_mode_a20 = false;
	m_has_exr = false;
	m_has_mac = false;
	m_mac_saturating = false;
	m_has_trace = false;
	m_has_hc = true;
	nvram_enable_backup(false); // disable nvram by default

	for(unsigned int i=0; i != m_read_adc.size(); i++)
		m_read_adc[i].bind().set([this, i]() { return adc_default(i); });
	for(int i=0; i != PORT_COUNT; i++) {
		m_read_port[i].bind().set([this, i]() { return port_default_r(i); });
		m_write_port[i].bind().set([this, i](u8 data) { port_default_w(i, data); });
	}
}

u16 h8_device::adc_default(int adc)
{
	if(!machine().side_effects_disabled())
		logerror("read of un-hooked adc %d\n", adc);
	return 0;
}

const char h8_device::port_names[] = "123456789abcdefg";

u8 h8_device::port_default_r(int port)
{
	if(!machine().side_effects_disabled())
		logerror("read of un-hooked port %c (PC=%X)\n", port_names[port], m_PPC);
	return 0xff;
}

void h8_device::port_default_w(int port, u8 data)
{
	logerror("write of un-hooked port %c %02x\n", port_names[port], data);
}

void h8_device::device_config_complete()
{
	u8 addrbits = m_mode_advanced ? (m_mode_a20 ? 20 : 24) : 16;
	m_program_config.m_addr_width = m_program_config.m_logaddr_width = addrbits;
}

void h8_device::device_start()
{
	space(AS_PROGRAM).cache(m_cache);
	space(AS_PROGRAM).specific(m_program);

	u32 pcmask = m_mode_advanced ? 0xffffff : 0xffff;
	state_add<u32>(H8_PC, "PC",
		[this]() { return m_NPC; },
		[this](u32 pc) { m_PC = m_PPC = m_NPC = pc; m_PIR = read16i(m_PC); m_PC += 2; prefetch_done_noirq_notrace(); }
	).mask(pcmask);
	state_add<u32>(STATE_GENPC, "GENPC",
		[this]() { return m_NPC; },
		[this](u32 pc) { m_PC = m_PPC = m_NPC = pc; m_PIR = read16i(m_PC); m_PC += 2; prefetch_done_noirq_notrace(); }
	).mask(pcmask).noshow();
	state_add(STATE_GENPCBASE, "CURPC",     m_PPC).mask(pcmask).noshow();
	state_add(H8_CCR,          "CCR",       m_CCR);
	if(m_has_exr)
		state_add(STATE_GENFLAGS,  "GENFLAGS",  m_CCR).formatstr("%11s").noshow();
	else
		state_add(STATE_GENFLAGS,  "GENFLAGS",  m_CCR).formatstr("%8s").noshow();

	if(m_has_exr)
		state_add(H8_EXR,          "EXR",       m_EXR);
	if(!m_supports_advanced) {
		state_add(H8_R0,           "R0",        m_R[0]);
		state_add(H8_R1,           "R1",        m_R[1]);
		state_add(H8_R2,           "R2",        m_R[2]);
		state_add(H8_R3,           "R3",        m_R[3]);
		state_add(H8_R4,           "R4",        m_R[4]);
		state_add(H8_R5,           "R5",        m_R[5]);
		state_add(H8_R6,           "R6",        m_R[6]);
		state_add(H8_R7,           "R7",        m_R[7]);
	} else {
		state_add(H8_R0,           "R0",        m_R[0]).noshow();
		state_add(H8_R1,           "R1",        m_R[1]).noshow();
		state_add(H8_R2,           "R2",        m_R[2]).noshow();
		state_add(H8_R3,           "R3",        m_R[3]).noshow();
		state_add(H8_R4,           "R4",        m_R[4]).noshow();
		state_add(H8_R5,           "R5",        m_R[5]).noshow();
		state_add(H8_R6,           "R6",        m_R[6]).noshow();
		state_add(H8_R7,           "R7",        m_R[7]).noshow();
		state_add(H8_E0,           "E0",        m_R[8]).noshow();
		state_add(H8_E1,           "E1",        m_R[9]).noshow();
		state_add(H8_E2,           "E2",        m_R[10]).noshow();
		state_add(H8_E3,           "E3",        m_R[11]).noshow();
		state_add(H8_E4,           "E4",        m_R[12]).noshow();
		state_add(H8_E5,           "E5",        m_R[13]).noshow();
		state_add(H8_E6,           "E6",        m_R[14]).noshow();
		state_add(H8_E7,           "E7",        m_R[15]).noshow();
		state_add(H8_R0,           "ER0",       m_TMPR).callimport().callexport().formatstr("%9s");
		state_add(H8_R1,           "ER1",       m_TMPR).callimport().callexport().formatstr("%9s");
		state_add(H8_R2,           "ER2",       m_TMPR).callimport().callexport().formatstr("%9s");
		state_add(H8_R3,           "ER3",       m_TMPR).callimport().callexport().formatstr("%9s");
		state_add(H8_R4,           "ER4",       m_TMPR).callimport().callexport().formatstr("%9s");
		state_add(H8_R5,           "ER5",       m_TMPR).callimport().callexport().formatstr("%9s");
		state_add(H8_R6,           "ER6",       m_TMPR).callimport().callexport().formatstr("%9s");
		state_add(H8_R7,           "ER7",       m_TMPR).callimport().callexport().formatstr("%9s");
	}

	save_item(NAME(m_PPC));
	save_item(NAME(m_NPC));
	save_item(NAME(m_PC));
	save_item(NAME(m_PIR));
	save_item(NAME(m_IR));
	save_item(NAME(m_R));
	save_item(NAME(m_EXR));
	save_item(NAME(m_CCR));
	save_item(NAME(m_TMP1));
	save_item(NAME(m_TMP2));
	save_item(NAME(m_inst_state));
	save_item(NAME(m_inst_substate));
	save_item(NAME(m_irq_vector));
	save_item(NAME(m_taken_irq_vector));
	save_item(NAME(m_irq_level));
	save_item(NAME(m_taken_irq_level));
	save_item(NAME(m_irq_nmi));
	save_item(NAME(m_current_dma));
	save_item(NAME(m_standby_pending));
	save_item(NAME(m_nvram_battery));

	set_icountptr(m_icount);

	m_PC = 0;
	m_PPC = 0;
	m_NPC = 0;
	memset(m_IR, 0, sizeof(m_IR));
	memset(m_R, 0, sizeof(m_R));
	m_EXR = 0;
	m_CCR = 0;
	m_MAC = 0;
	m_MACF = 0;
	m_inst_state = STATE_RESET;
	m_inst_substate = 0;
	m_count_before_instruction_step = 0;
	m_requested_state = -1;
	m_dma_device = nullptr;
	m_dtc_device = nullptr;

	memset(m_dma_channel, 0, sizeof(m_dma_channel));
}

void h8_device::device_reset()
{
	m_inst_state = STATE_RESET;
	m_inst_substate = 0;
	m_count_before_instruction_step = 0;
	m_requested_state = -1;

	m_irq_vector = 0;
	m_irq_level = -1;
	m_irq_nmi = false;
	m_taken_irq_vector = 0;
	m_taken_irq_level = -1;
	m_current_dma = -1;
	m_current_dtc = nullptr;

	m_standby_pending = false;
	m_standby_cb(0);
}


// nvram handling

bool h8_device::nvram_write(util::write_stream &file)
{
	// if it's currently not battery-backed, don't save at all
	if(!m_nvram_battery)
		return true;

	size_t actual;

	// internal RAM
	if(m_internal_ram) {
		if(file.write(&m_internal_ram[0], m_internal_ram.bytes(), actual) || m_internal_ram.bytes() != actual)
			return false;
	}

	// I/O ports
	for(h8_port_device &port : h8_port_device_enumerator(*this)) {
		if(!port.nvram_write(file))
			return false;
	}

	return true;
}

bool h8_device::nvram_read(util::read_stream &file)
{
	size_t actual;

	// internal RAM
	if(m_internal_ram) {
		if(file.read(&m_internal_ram[0], m_internal_ram.bytes(), actual) || m_internal_ram.bytes() != actual)
			return false;
	}

	// I/O ports
	for(h8_port_device &port : h8_port_device_enumerator(*this)) {
		if(!port.nvram_read(file))
			return false;
	}

	return true;
}

void h8_device::nvram_default()
{
	if(!nvram_backup_enabled() || !m_internal_ram)
		return;

	// default nvram from mytag:nvram region if it exists (only the internal RAM for now)
	memory_region *region = memregion("nvram");
	if(region != nullptr) {
		if(region->bytes() != m_internal_ram.bytes())
			fatalerror("%s: Wrong region size (expected 0x%x, found 0x%x)", region->name(), m_internal_ram.bytes(), region->bytes());

		std::copy_n(&region->as_u16(), m_internal_ram.length(), &m_internal_ram[0]);
	}
	else
		std::fill_n(&m_internal_ram[0], m_internal_ram.length(), m_nvram_defval);
}


bool h8_device::trigger_dma(int vector)
{
	bool dma_triggered = false;
	bool drop_interrupt = false;
	for(int i=0; i != 8; i++)
		if(m_dma_channel[i] && ((m_dma_channel[i]->m_flags & (h8_dma_state::ACTIVE|h8_dma_state::SUSPENDED)) == (h8_dma_state::ACTIVE|h8_dma_state::SUSPENDED)) && m_dma_channel[i]->m_trigger_vector == vector) {
			m_dma_channel[i]->m_flags &= ~h8_dma_state::SUSPENDED;
			dma_triggered = true;
			if(m_dma_channel[i]->m_flags & h8_dma_state::EAT_INTERRUPT)
				drop_interrupt = true;
		}

	// DMA can mask interrupt to the DTC
	if(!drop_interrupt && m_dtc_device && m_dtc_device->trigger_dtc(vector))
		drop_interrupt = true;

	if(dma_triggered)
		update_active_dma_channel();

	return drop_interrupt;
}

void h8_device::set_dma_channel(h8_dma_state *state)
{
	m_dma_channel[state->m_id] = state;
}

void h8_device::update_active_dma_channel()
{
	for(int i=0; i != 8; i++) {
		if(m_dma_channel[i] && ((m_dma_channel[i]->m_flags & (h8_dma_state::ACTIVE|h8_dma_state::SUSPENDED)) == h8_dma_state::ACTIVE)) {
			m_current_dma = i;
			return;
		}
	}
	m_current_dma = -1;
}

void h8_device::set_current_dtc(h8_dtc_state *state)
{
	m_current_dtc = state;
}

void h8_device::request_state(int state)
{
	m_requested_state = state;
}

void h8_device::recompute_bcount(u64 event_time)
{
	if(!event_time || event_time >= total_cycles() + m_icount) {
		m_bcount = 0;
		return;
	}
	m_bcount = total_cycles() + m_icount - event_time;
}

void h8_device::execute_run()
{
	internal_update(total_cycles());

	m_icount -= m_count_before_instruction_step;
	if(m_icount < 0) {
		m_count_before_instruction_step = -m_icount;
		m_icount = 0;
	} else
		m_count_before_instruction_step = 0;

	while(m_bcount && m_icount <= m_bcount)
		internal_update(total_cycles() + m_icount - m_bcount);

	if(m_icount > 0 && m_inst_substate)
		do_exec_partial();

	while(m_icount > 0) {
		while(m_icount > m_bcount) {
			if(m_inst_state < 0x10000) {
				m_PPC = m_NPC;
				if(machine().debug_flags & DEBUG_FLAG_ENABLED)
					debugger_instruction_hook(m_NPC);
			}
			do_exec_full();
		}
		if(m_icount > 0)
			while(m_bcount && m_icount <= m_bcount)
				internal_update(total_cycles() + m_icount - m_bcount);
		if(m_icount > 0 && m_inst_substate)
			do_exec_partial();
	}
	if(m_icount < 0) {
		m_count_before_instruction_step = -m_icount;
		m_icount = 0;
	}
}

void h8_device::add_event(u64 &event_time, u64 new_event)
{
	if(!new_event)
		return;
	if(!event_time || event_time > new_event)
		event_time = new_event;
}

void h8_device::internal_update()
{
	internal_update(total_cycles());
}

device_memory_interface::space_config_vector h8_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config)
	};
}


void h8_device::state_import(const device_state_entry &entry)
{
	switch(entry.index()) {
	case H8_R0:
	case H8_R1:
	case H8_R2:
	case H8_R3:
	case H8_R4:
	case H8_R5:
	case H8_R6:
	case H8_R7: {
		int r = entry.index() - H8_R0;
		m_R[r + 8] = m_TMPR >> 16;
		m_R[r] = m_TMPR;
		break;
	}
	}
}

void h8_device::state_export(const device_state_entry &entry)
{
	switch(entry.index()) {
	case H8_R0:
	case H8_R1:
	case H8_R2:
	case H8_R3:
	case H8_R4:
	case H8_R5:
	case H8_R6:
	case H8_R7: {
		int r = entry.index() - H8_R0;
		m_TMPR = (m_R[r + 8] << 16) | m_R[r];
		break;
	}
	}
}

void h8_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch(entry.index()) {
	case STATE_GENFLAGS:
		if(m_has_exr)
			str = string_format("%c%c %c%c%c%c%c%c%c%c",
					(m_EXR & EXR_T) ? 'T' : '-',
					'0' + (m_EXR & EXR_I),
					(m_CCR & F_I)  ? 'I' : '-',
					(m_CCR & F_UI) ? 'u' : '-',
					(m_CCR & F_H)  ? 'H' : '-',
					(m_CCR & F_U)  ? 'U' : '-',
					(m_CCR & F_N)  ? 'N' : '-',
					(m_CCR & F_Z)  ? 'Z' : '-',
					(m_CCR & F_V)  ? 'V' : '-',
					(m_CCR & F_C)  ? 'C' : '-');
		else if(m_has_hc)
			str = string_format("%c%c%c%c%c%c%c%c",
					(m_CCR & F_I)  ? 'I' : '-',
					(m_CCR & F_UI) ? 'u' : '-',
					(m_CCR & F_H)  ? 'H' : '-',
					(m_CCR & F_U)  ? 'U' : '-',
					(m_CCR & F_N)  ? 'N' : '-',
					(m_CCR & F_Z)  ? 'Z' : '-',
					(m_CCR & F_V)  ? 'V' : '-',
					(m_CCR & F_C)  ? 'C' : '-');
		else
			str = string_format("%c%c%c%c%c%c%c%c",
					(m_CCR & F_I)  ? '?' : '-',
					(m_CCR & F_UI) ? 'u' : '-',
					(m_CCR & F_H)  ? 'I' : '-',
					(m_CCR & F_U)  ? 'U' : '-',
					(m_CCR & F_N)  ? 'N' : '-',
					(m_CCR & F_Z)  ? 'Z' : '-',
					(m_CCR & F_V)  ? 'V' : '-',
					(m_CCR & F_C)  ? 'C' : '-');
		break;
	case H8_R0:
	case H8_R1:
	case H8_R2:
	case H8_R3:
	case H8_R4:
	case H8_R5:
	case H8_R6:
	case H8_R7: {
		int r = entry.index() - H8_R0;
		str = string_format("%04X %04X", m_R[r + 8], m_R[r]);
		break;
	}
	}
}

// FIXME: one-state bus cycles are only provided for on-chip ROM & RAM in H8S/2000 and H8S/2600.
// All other accesses take *at least* two states each, and additional wait states are often programmed for external memory!

u16 h8_device::read16i(u32 adr)
{
	if(m_has_exr)
		m_icount--;
	else
		m_icount -= 2;
	return m_cache.read_word(adr & ~1);
}

u8 h8_device::read8(u32 adr)
{
	if(m_has_exr)
		m_icount--;
	else
		m_icount -= 2;
	return m_program.read_byte(adr);
}

void h8_device::write8(u32 adr, u8 data)
{
	if(m_has_exr)
		m_icount--;
	else
		m_icount -= 2;
	m_program.write_byte(adr, data);
}

u16 h8_device::read16(u32 adr)
{
	if(m_has_exr)
		m_icount--;
	else
		m_icount -= 2;
	return m_program.read_word(adr & ~1);
}

void h8_device::write16(u32 adr, u16 data)
{
	if(m_has_exr)
		m_icount--;
	else
		m_icount -= 2;
	m_program.write_word(adr & ~1, data);
}

bool h8_device::exr_in_stack() const
{
	return false;
}

void h8_device::take_interrupt()
{
	m_inst_state = STATE_IRQ;
	m_taken_irq_vector = m_irq_vector;
	m_taken_irq_level = m_irq_level;
}

void h8_device::prefetch_done()
{
	if(m_requested_state != -1) {
		m_inst_state = m_requested_state;
		m_requested_state = -1;
	} else if(m_current_dma != -1)
		m_inst_state = STATE_DMA;
	else if(m_current_dtc)
		m_inst_state = STATE_DTC;
	else if(m_irq_vector)
		take_interrupt();
	else if(m_has_trace && (m_EXR & EXR_T) && exr_in_stack())
		m_inst_state = STATE_TRACE;
	else
		m_inst_state = m_IR[0] = m_PIR;
}

void h8_device::prefetch_done_noirq()
{
	if(m_has_trace && (m_EXR & EXR_T) && exr_in_stack())
		m_inst_state = STATE_TRACE;
	else
		m_inst_state = m_IR[0] = m_PIR;
}

void h8_device::prefetch_done_notrace()
{
	prefetch_done();
	if(m_inst_state == STATE_TRACE)
		m_inst_state = m_IR[0] = m_PIR;
}

void h8_device::prefetch_done_noirq_notrace()
{
	m_inst_state = m_IR[0] = m_PIR;
}

void h8_device::set_irq(int irq_vector, int irq_level, bool irq_nmi)
{
	m_irq_vector = irq_vector;
	m_irq_level = irq_level;
	m_irq_nmi = irq_nmi;

	// wake up from software standby with an external interrupt
	if(standby() && m_irq_vector) {
		resume(SUSPEND_REASON_CLOCK);
		m_standby_cb(0);
		take_interrupt();
	}
}

void h8_device::internal(int cycles)
{
	// on H8/300, H8/300L, H8/300H (not H8S), all internal operations take an even number of states (at least 2 each)
	if(!m_has_exr)
		m_icount -= cycles + 1;
	else
		m_icount -= cycles;
}

void h8_device::illegal()
{
	logerror("Illegal instruction at address %x\n", m_PPC);
	m_icount = -10000000;
}

int h8_device::trace_setup()
{
	throw emu_fatalerror("%s: Trace setup called but unimplemented.\n", tag());
}

int h8_device::trapa_setup()
{
	throw emu_fatalerror("%s: Trapa setup called but unimplemented.\n", tag());
}

u8 h8_device::do_addx8(u8 v1, u8 v2)
{
	u16 res = v1 + v2 + (m_CCR & F_C ? 1 : 0);
	m_CCR &= ~(F_N|F_V|F_C);
	if(m_has_hc)
	{
		m_CCR &= ~F_H;
		if(((v1 & 0xf) + (v2 & 0xf) + (m_CCR & F_C ? 1 : 0)) & 0x10)
			m_CCR |= F_H;
	}
	if(u8(res))
		m_CCR &= ~F_Z;
	if(s8(res) < 0)
		m_CCR |= F_N;
	if(~(v1^v2) & (v1^res) & 0x80)
		m_CCR |= F_V;
	if(res & 0x100)
		m_CCR |= F_C;
	return res;
}

u8 h8_device::do_subx8(u8 v1, u8 v2)
{
	u16 res = v1 - v2 - (m_CCR & F_C ? 1 : 0);
	m_CCR &= ~(F_N|F_V|F_C);
	if(m_has_hc)
	{
		m_CCR &= ~F_H;
		if(((v1 & 0xf) - (v2 & 0xf) - (m_CCR & F_C ? 1 : 0)) & 0x10)
			m_CCR |= F_H;
	}
	if(u8(res))
		m_CCR &= ~F_Z;
	if(s8(res) < 0)
		m_CCR |= F_N;
	if((v1^v2) & (v1^res) & 0x80)
		m_CCR |= F_V;
	if(res & 0x100)
		m_CCR |= F_C;
	return res;
}

u8 h8_device::do_inc8(u8 v1, u8 v2)
{
	u8 res = v1 + v2;
	m_CCR &= ~(F_N|F_V|F_Z);
	if(!res)
		m_CCR |= F_Z;
	else if(s8(res) < 0)
		m_CCR |= F_N;
	if(~(v1^v2) & (v1^res) & 0x80)
		m_CCR |= F_V;
	return res;
}

u16 h8_device::do_inc16(u16 v1, u16 v2)
{
	u16 res = v1 + v2;
	m_CCR &= ~(F_N|F_V|F_Z);
	if(!res)
		m_CCR |= F_Z;
	else if(s16(res) < 0)
		m_CCR |= F_N;
	if(~(v1^v2) & (v1^res) & 0x8000)
		m_CCR |= F_V;
	return res;
}

u32 h8_device::do_inc32(u32 v1, u32 v2)
{
	u32 res = v1 + v2;
	m_CCR &= ~(F_N|F_V|F_Z);
	if(!res)
		m_CCR |= F_Z;
	else if(s32(res) < 0)
		m_CCR |= F_N;
	if(~(v1^v2) & (v1^res) & 0x80000000)
		m_CCR |= F_V;
	return res;
}

u8 h8_device::do_add8(u8 v1, u8 v2)
{
	u16 res = v1 + v2;
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(m_has_hc)
	{
		m_CCR &= ~F_H;
		if(((v1 & 0xf) + (v2 & 0xf)) & 0x10)
			m_CCR |= F_H;
	}
	if(!u8(res))
		m_CCR |= F_Z;
	else if(s8(res) < 0)
		m_CCR |= F_N;
	if(~(v1^v2) & (v1^res) & 0x80)
		m_CCR |= F_V;
	if(res & 0x100)
		m_CCR |= F_C;
	return res;
}

u16 h8_device::do_add16(u16 v1, u16 v2)
{
	u32 res = v1 + v2;
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(m_has_hc)
	{
		m_CCR &= ~F_H;
		if(((v1 & 0xfff) + (v2 & 0xfff)) & 0x1000)
			m_CCR |= F_H;
	}
	if(!u16(res))
		m_CCR |= F_Z;
	else if(s16(res) < 0)
		m_CCR |= F_N;
	if(~(v1^v2) & (v1^res) & 0x8000)
		m_CCR |= F_V;
	if(res & 0x10000)
		m_CCR |= F_C;
	return res;
}

u32 h8_device::do_add32(u32 v1, u32 v2)
{
	u64 res = u64(v1) + u64(v2);
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(m_has_hc)
	{
		m_CCR &= ~F_H;
		if(((v1 & 0xfffffff) + (v2 & 0xfffffff)) & 0x10000000)
			m_CCR |= F_H;
	}
	if(!u32(res))
		m_CCR |= F_Z;
	else if(s32(res) < 0)
		m_CCR |= F_N;
	if(~(v1^v2) & (v1^res) & 0x80000000)
		m_CCR |= F_V;
	if(res & 0x100000000ULL)
		m_CCR |= F_C;
	return res;
}

u8 h8_device::do_dec8(u8 v1, u8 v2)
{
	u8 res = v1 - v2;
	m_CCR &= ~(F_N|F_V|F_Z);
	if(!res)
		m_CCR |= F_Z;
	else if(s8(res) < 0)
		m_CCR |= F_N;
	if((v1^v2) & (v1^res) & 0x80)
		m_CCR |= F_V;
	return res;
}

u16 h8_device::do_dec16(u16 v1, u16 v2)
{
	u16 res = v1 - v2;
	m_CCR &= ~(F_N|F_V|F_Z);
	if(!res)
		m_CCR |= F_Z;
	else if(s16(res) < 0)
		m_CCR |= F_N;
	if((v1^v2) & (v1^res) & 0x8000)
		m_CCR |= F_V;
	return res;
}

u32 h8_device::do_dec32(u32 v1, u32 v2)
{
	u32 res = v1 - v2;
	m_CCR &= ~(F_N|F_V|F_Z);
	if(!res)
		m_CCR |= F_Z;
	else if(s32(res) < 0)
		m_CCR |= F_N;
	if((v1^v2) & (v1^res) & 0x80000000)
		m_CCR |= F_V;
	return res;
}

u8 h8_device::do_sub8(u8 v1, u8 v2)
{
	u16 res = v1 - v2;
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(m_has_hc)
	{
		m_CCR &= ~F_H;
		if(((v1 & 0xf) - (v2 & 0xf)) & 0x10)
			m_CCR |= F_H;
	}
	if(!u8(res))
		m_CCR |= F_Z;
	else if(s8(res) < 0)
		m_CCR |= F_N;
	if((v1^v2) & (v1^res) & 0x80)
		m_CCR |= F_V;
	if(res & 0x100)
		m_CCR |= F_C;
	return res;
}

u16 h8_device::do_sub16(u16 v1, u16 v2)
{
	u32 res = v1 - v2;
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(m_has_hc)
	{
		m_CCR &= ~F_H;
		if(((v1 & 0xfff) - (v2 & 0xfff)) & 0x1000)
			m_CCR |= F_H;
	}
	if(!u16(res))
		m_CCR |= F_Z;
	else if(s16(res) < 0)
		m_CCR |= F_N;
	if((v1^v2) & (v1^res) & 0x8000)
		m_CCR |= F_V;
	if(res & 0x10000)
		m_CCR |= F_C;
	return res;
}

u32 h8_device::do_sub32(u32 v1, u32 v2)
{
	u64 res = u64(v1) - u64(v2);
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(m_has_hc)
	{
		m_CCR &= ~F_H;
		if(((v1 & 0xfffffff) - (v2 & 0xfffffff)) & 0x10000000)
			m_CCR |= F_H;
	}
	if(!u32(res))
		m_CCR |= F_Z;
	else if(s32(res) < 0)
		m_CCR |= F_N;
	if((v1^v2) & (v1^res) & 0x80000000)
		m_CCR |= F_V;
	if(res & 0x100000000ULL)
		m_CCR |= F_C;
	return res;
}

u8 h8_device::do_shal8(u8 v)
{
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x80)
		m_CCR |= F_C;
	if((v & 0xc0) == 0x40 || (v & 0xc0) == 0x80)
		m_CCR |= F_V;
	v <<= 1;
	if(!v)
		m_CCR |= F_Z;
	else if(s8(v) < 0)
		m_CCR |= F_N;
	return v;
}

u16 h8_device::do_shal16(u16 v)
{
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x8000)
		m_CCR |= F_C;
	if((v & 0xc000) == 0x4000 || (v & 0xc000) == 0x8000)
		m_CCR |= F_V;
	v <<= 1;
	if(!v)
		m_CCR |= F_Z;
	else if(s16(v) < 0)
		m_CCR |= F_N;
	return v;
}

u32 h8_device::do_shal32(u32 v)
{
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x80000000)
		m_CCR |= F_C;
	if((v & 0xc0000000) == 0x40000000 || (v & 0xc0000000) == 0x80000000)
		m_CCR |= F_V;
	v <<= 1;
	if(!v)
		m_CCR |= F_Z;
	else if(s32(v) < 0)
		m_CCR |= F_N;
	return v;
}

u8 h8_device::do_shar8(u8 v)
{
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 1)
		m_CCR |= F_C;
	v >>= 1;
	if(!v)
		m_CCR |= F_Z;
	else if(v & 0x40) {
		v |= 0x80;
		m_CCR |= F_N;
	}
	return v;
}

u16 h8_device::do_shar16(u16 v)
{
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 1)
		m_CCR |= F_C;
	v >>= 1;
	if(!v)
		m_CCR |= F_Z;
	else if(v & 0x4000) {
		v |= 0x8000;
		m_CCR |= F_N;
	}
	return v;
}

u32 h8_device::do_shar32(u32 v)
{
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 1)
		m_CCR |= F_C;
	v >>= 1;
	if(!v)
		m_CCR |= F_Z;
	else if(v & 0x40000000) {
		v |= 0x80000000;
		m_CCR |= F_N;
	}
	return v;
}

u8 h8_device::do_shll8(u8 v)
{
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x80)
		m_CCR |= F_C;
	v <<= 1;
	if(!v)
		m_CCR |= F_Z;
	else if(s8(v) < 0)
		m_CCR |= F_N;
	return v;
}

u16 h8_device::do_shll16(u16 v)
{
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x8000)
		m_CCR |= F_C;
	v <<= 1;
	if(!v)
		m_CCR |= F_Z;
	else if(s16(v) < 0)
		m_CCR |= F_N;
	return v;
}

u32 h8_device::do_shll32(u32 v)
{
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x80000000)
		m_CCR |= F_C;
	v <<= 1;
	if(!v)
		m_CCR |= F_Z;
	else if(s32(v) < 0)
		m_CCR |= F_N;
	return v;
}

u8 h8_device::do_shlr8(u8 v)
{
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 1)
		m_CCR |= F_C;
	v >>= 1;
	if(!v)
		m_CCR |= F_Z;
	return v;
}

u16 h8_device::do_shlr16(u16 v)
{
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 1)
		m_CCR |= F_C;
	v >>= 1;
	if(!v)
		m_CCR |= F_Z;
	return v;
}

u32 h8_device::do_shlr32(u32 v)
{
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 1)
		m_CCR |= F_C;
	v >>= 1;
	if(!v)
		m_CCR |= F_Z;
	return v;
}

u8 h8_device::do_shal2_8(u8 v)
{
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x40)
		m_CCR |= F_C;
	if((v & 0xc0) == 0x40 || (v & 0xc0) == 0x80 ||
		(v & 0x60) == 0x20 || (v & 0x60) == 0x40)
		m_CCR |= F_V;
	v <<= 2;
	if(!v)
		m_CCR |= F_Z;
	else if(s8(v) < 0)
		m_CCR |= F_N;
	return v;
}

u16 h8_device::do_shal2_16(u16 v)
{
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x4000)
		m_CCR |= F_C;
	if((v & 0xc000) == 0x4000 || (v & 0xc000) == 0x8000 ||
		(v & 0x6000) == 0x2000 || (v & 0x6000) == 0x4000)
		m_CCR |= F_V;
	v <<= 2;
	if(!v)
		m_CCR |= F_Z;
	else if(s16(v) < 0)
		m_CCR |= F_N;
	return v;
}

u32 h8_device::do_shal2_32(u32 v)
{
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x40000000)
		m_CCR |= F_C;
	if((v & 0xc0000000) == 0x40000000 || (v & 0xc0000000) == 0x80000000 ||
		(v & 0x60000000) == 0x20000000 || (v & 0x60000000) == 0x40000000)
		m_CCR |= F_V;
	v <<= 2;
	if(!v)
		m_CCR |= F_Z;
	else if(s32(v) < 0)
		m_CCR |= F_N;
	return v;
}

u8 h8_device::do_shar2_8(u8 v)
{
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 2)
		m_CCR |= F_C;
	v >>= 2;
	if(!v)
		m_CCR |= F_Z;
	else if(v & 0x20) {
		v |= 0xc0;
		m_CCR |= F_N;
	}
	return v;
}

u16 h8_device::do_shar2_16(u16 v)
{
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 2)
		m_CCR |= F_C;
	v >>= 2;
	if(!v)
		m_CCR |= F_Z;
	else if(v & 0x2000) {
		v |= 0xc000;
		m_CCR |= F_N;
	}
	return v;
}

u32 h8_device::do_shar2_32(u32 v)
{
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 2)
		m_CCR |= F_C;
	v >>= 2;
	if(!v)
		m_CCR |= F_Z;
	else if(v & 0x20000000) {
		v |= 0xc0000000;
		m_CCR |= F_N;
	}
	return v;
}

u8 h8_device::do_shll2_8(u8 v)
{
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x40)
		m_CCR |= F_C;
	v <<= 2;
	if(!v)
		m_CCR |= F_Z;
	else if(s8(v) < 0)
		m_CCR |= F_N;
	return v;
}

u16 h8_device::do_shll2_16(u16 v)
{
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x4000)
		m_CCR |= F_C;
	v <<= 2;
	if(!v)
		m_CCR |= F_Z;
	else if(s16(v) < 0)
		m_CCR |= F_N;
	return v;
}

u32 h8_device::do_shll2_32(u32 v)
{
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x40000000)
		m_CCR |= F_C;
	v <<= 2;
	if(!v)
		m_CCR |= F_Z;
	else if(s32(v) < 0)
		m_CCR |= F_N;
	return v;
}

u8 h8_device::do_shlr2_8(u8 v)
{
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 2)
		m_CCR |= F_C;
	v >>= 2;
	if(!v)
		m_CCR |= F_Z;
	return v;
}

u16 h8_device::do_shlr2_16(u16 v)
{
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 2)
		m_CCR |= F_C;
	v >>= 2;
	if(!v)
		m_CCR |= F_Z;
	return v;
}

u32 h8_device::do_shlr2_32(u32 v)
{
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 2)
		m_CCR |= F_C;
	v >>= 2;
	if(!v)
		m_CCR |= F_Z;
	return v;
}

u8 h8_device::do_rotl8(u8 v)
{
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x80)
		m_CCR |= F_C;
	v = (v << 1) | (v >> 7);
	if(!v)
		m_CCR |= F_Z;
	else if(s8(v) < 0)
		m_CCR |= F_N;
	return v;
}

u16 h8_device::do_rotl16(u16 v)
{
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x8000)
		m_CCR |= F_C;
	v = (v << 1) | (v >> 15);
	if(!v)
		m_CCR |= F_Z;
	else if(s16(v) < 0)
		m_CCR |= F_N;
	return v;
}

u32 h8_device::do_rotl32(u32 v)
{
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x80000000)
		m_CCR |= F_C;
	v = (v << 1) | (v >> 31);
	if(!v)
		m_CCR |= F_Z;
	else if(s32(v) < 0)
		m_CCR |= F_N;
	return v;
}

u8 h8_device::do_rotr8(u8 v)
{
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x01)
		m_CCR |= F_C;
	v = (v << 7) | (v >> 1);
	if(!v)
		m_CCR |= F_Z;
	else if(s8(v) < 0)
		m_CCR |= F_N;
	return v;
}

u16 h8_device::do_rotr16(u16 v)
{
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x0001)
		m_CCR |= F_C;
	v = (v << 15) | (v >> 1);
	if(!v)
		m_CCR |= F_Z;
	else if(s16(v) < 0)
		m_CCR |= F_N;
	return v;
}

u32 h8_device::do_rotr32(u32 v)
{
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x00000001)
		m_CCR |= F_C;
	v = (v << 31) | (v >> 1);
	if(!v)
		m_CCR |= F_Z;
	else if(s32(v) < 0)
		m_CCR |= F_N;
	return v;
}

u8 h8_device::do_rotxl8(u8 v)
{
	u8 c = m_CCR & F_C ? 1 : 0;
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x80)
		m_CCR |= F_C;
	v = (v << 1) | c;
	if(!v)
		m_CCR |= F_Z;
	else if(s8(v) < 0)
		m_CCR |= F_N;
	return v;
}

u16 h8_device::do_rotxl16(u16 v)
{
	u16 c = m_CCR & F_C ? 1 : 0;
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x8000)
		m_CCR |= F_C;
	v = (v << 1) | c;
	if(!v)
		m_CCR |= F_Z;
	else if(s16(v) < 0)
		m_CCR |= F_N;
	return v;
}

u32 h8_device::do_rotxl32(u32 v)
{
	u32 c = m_CCR & F_C ? 1 : 0;
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x80000000)
		m_CCR |= F_C;
	v = (v << 1) | c;
	if(!v)
		m_CCR |= F_Z;
	else if(s32(v) < 0)
		m_CCR |= F_N;
	return v;
}

u8 h8_device::do_rotxr8(u8 v)
{
	u8 c = m_CCR & F_C ? 1 : 0;
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x01)
		m_CCR |= F_C;
	v = (v >> 1) | (c << 7);
	if(!v)
		m_CCR |= F_Z;
	else if(s8(v) < 0)
		m_CCR |= F_N;
	return v;
}

u16 h8_device::do_rotxr16(u16 v)
{
	u8 c = m_CCR & F_C ? 1 : 0;
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x0001)
		m_CCR |= F_C;
	v = (v >> 1) | (c << 15);
	if(!v)
		m_CCR |= F_Z;
	else if(s16(v) < 0)
		m_CCR |= F_N;
	return v;
}

u32 h8_device::do_rotxr32(u32 v)
{
	u8 c = m_CCR & F_C ? 1 : 0;
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x00000001)
		m_CCR |= F_C;
	v = (v >> 1) | (c << 31);
	if(!v)
		m_CCR |= F_Z;
	else if(s32(v) < 0)
		m_CCR |= F_N;
	return v;
}

u8 h8_device::do_rotl2_8(u8 v)
{
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x40)
		m_CCR |= F_C;
	v = (v << 2) | (v >> 6);
	if(!v)
		m_CCR |= F_Z;
	else if(s8(v) < 0)
		m_CCR |= F_N;
	return v;
}

u16 h8_device::do_rotl2_16(u16 v)
{
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x4000)
		m_CCR |= F_C;
	v = (v << 2) | (v >> 14);
	if(!v)
		m_CCR |= F_Z;
	else if(s16(v) < 0)
		m_CCR |= F_N;
	return v;
}

u32 h8_device::do_rotl2_32(u32 v)
{
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x40000000)
		m_CCR |= F_C;
	v = (v << 2) | (v >> 30);
	if(!v)
		m_CCR |= F_Z;
	else if(s32(v) < 0)
		m_CCR |= F_N;
	return v;
}

u8 h8_device::do_rotr2_8(u8 v)
{
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x02)
		m_CCR |= F_C;
	v = (v << 6) | (v >> 2);
	if(!v)
		m_CCR |= F_Z;
	else if(s8(v) < 0)
		m_CCR |= F_N;
	return v;
}

u16 h8_device::do_rotr2_16(u16 v)
{
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x0002)
		m_CCR |= F_C;
	v = (v << 14) | (v >> 2);
	if(!v)
		m_CCR |= F_Z;
	else if(s16(v) < 0)
		m_CCR |= F_N;
	return v;
}

u32 h8_device::do_rotr2_32(u32 v)
{
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x00000002)
		m_CCR |= F_C;
	v = (v << 30) | (v >> 2);
	if(!v)
		m_CCR |= F_Z;
	else if(s32(v) < 0)
		m_CCR |= F_N;
	return v;
}

u8 h8_device::do_rotxl2_8(u8 v)
{
	u8 c = m_CCR & F_C ? 1 : 0;
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x40)
		m_CCR |= F_C;
	v = (v << 2) | (c << 1) | (v >> 7);
	if(!v)
		m_CCR |= F_Z;
	else if(s8(v) < 0)
		m_CCR |= F_N;
	return v;
}

u16 h8_device::do_rotxl2_16(u16 v)
{
	u16 c = m_CCR & F_C ? 1 : 0;
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x4000)
		m_CCR |= F_C;
	v = (v << 2) | (c << 1) | (v >> 15);
	if(!v)
		m_CCR |= F_Z;
	else if(s16(v) < 0)
		m_CCR |= F_N;
	return v;
}

u32 h8_device::do_rotxl2_32(u32 v)
{
	u32 c = m_CCR & F_C ? 1 : 0;
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x40000000)
		m_CCR |= F_C;
	v = (v << 2) | (c << 1) | (v >> 31);
	if(!v)
		m_CCR |= F_Z;
	else if(s32(v) < 0)
		m_CCR |= F_N;
	return v;
}

u8 h8_device::do_rotxr2_8(u8 v)
{
	u8 c = m_CCR & F_C ? 1 : 0;
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x02)
		m_CCR |= F_C;
	v = (v >> 2) | (c << 6) | (v << 7);
	if(!v)
		m_CCR |= F_Z;
	else if(s8(v) < 0)
		m_CCR |= F_N;
	return v;
}

u16 h8_device::do_rotxr2_16(u16 v)
{
	u16 c = m_CCR & F_C ? 1 : 0;
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x0002)
		m_CCR |= F_C;
	v = (v >> 2) | (c << 14) | (v << 15);
	if(!v)
		m_CCR |= F_Z;
	else if(s16(v) < 0)
		m_CCR |= F_N;
	return v;
}

u32 h8_device::do_rotxr2_32(u32 v)
{
	u32 c = m_CCR & F_C ? 1 : 0;
	m_CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x00000002)
		m_CCR |= F_C;
	v = (v >> 2) | (c << 30) | (v << 31);
	if(!v)
		m_CCR |= F_Z;
	else if(s32(v) < 0)
		m_CCR |= F_N;
	return v;
}

void h8_device::set_nzv8(u8 v)
{
	m_CCR &= ~(F_N|F_V|F_Z);
	if(!v)
		m_CCR |= F_Z;
	else if(s8(v) < 0)
		m_CCR |= F_N;
}

void h8_device::set_nzv16(u16 v)
{
	m_CCR &= ~(F_N|F_V|F_Z);
	if(!v)
		m_CCR |= F_Z;
	else if(s16(v) < 0)
		m_CCR |= F_N;
}

void h8_device::set_nzv32(u32 v)
{
	m_CCR &= ~(F_N|F_V|F_Z);
	if(!v)
		m_CCR |= F_Z;
	else if(s32(v) < 0)
		m_CCR |= F_N;
}

void h8_device::set_nz16(u16 v)
{
	m_CCR &= ~(F_N|F_Z);
	if(!v)
		m_CCR |= F_Z;
	else if(s16(v) < 0)
		m_CCR |= F_N;
}

void h8_device::set_nz32(u32 v)
{
	m_CCR &= ~(F_N|F_Z);
	if(!v)
		m_CCR |= F_Z;
	else if(s32(v) < 0)
		m_CCR |= F_N;
}

std::unique_ptr<util::disasm_interface> h8_device::create_disassembler()
{
	return std::make_unique<h8_disassembler>();
}

#include "cpu/h8/h8.hxx"
