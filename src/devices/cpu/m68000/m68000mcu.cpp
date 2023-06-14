// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "m68000mcu.h"

m68000_mcu_device::m68000_mcu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	m68000_device(mconfig, type, tag, owner, clock)
{
	m_disable_interrupt_callback = true;
}

void m68000_mcu_device::execute_run()
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

	while(m_icount > 0) {
		for(;;) {
			if(m_icount > m_bcount && m_inst_substate)
				(this->*(m_handlers_p[m_inst_state]))();

			while(m_icount > m_bcount) {
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
		if(m_icount > 0)
			while(m_bcount && m_icount <= m_bcount)
				internal_update(total_cycles() + m_icount - m_bcount);
		if(m_icount > 0 && m_inst_substate) {
			(this->*(m_handlers_p[m_inst_state]))();
			if(m_post_run)
				do_post_run();
		}
	}

	if(m_icount < 0) {
		m_count_before_instruction_step = -m_icount;
		m_icount = 0;
	}
}

void m68000_mcu_device::recompute_bcount(uint64_t event_time)
{
	if(!event_time || event_time >= total_cycles() + m_icount) {
		m_bcount = 0;
		return;
	}
	m_bcount = total_cycles() + m_icount - event_time;
}

void m68000_mcu_device::add_event(uint64_t &event_time, uint64_t new_event)
{
	if(!new_event)
		return;
	if(!event_time || event_time > new_event)
		event_time = new_event;
}

void m68000_mcu_device::device_start()
{
	m68000_device::device_start();

	// Theoretically UB, in practice works, the alternative (putting
	// everything in m68000_device) is annoying
	if(m_mmu) {
		m_handlers_f = reinterpret_cast<const handler *>(s_handlers_ifm);
		m_handlers_p = reinterpret_cast<const handler *>(s_handlers_ipm);
	} else {
		m_handlers_f = reinterpret_cast<const handler *>(s_handlers_dfm);
		m_handlers_p = reinterpret_cast<const handler *>(s_handlers_dpm);
	}
}

void m68000_mcu_device::set_current_interrupt_level(u32 level)
{
	if(level == m_int_level)
		return;

	m_int_level = level;

	/* A transition from < 7 to 7 always interrupts (NMI) */
	/* Note: Level 7 can also level trigger like a normal IRQ */
	if(m_int_level == 7)
		m_nmi_pending = true;

	update_interrupt();
}
