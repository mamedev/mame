// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m6502mcu.ipp

    Helper for 6502 variants with internal peripherals

***************************************************************************/

#ifndef MAME_CPU_M6502_M6502MCU_IPP
#define MAME_CPU_M6502_M6502MCU_IPP

#pragma once

#include "m6502mcu.h"


template <typename Base>
m6502_mcu_device_base<Base>::m6502_mcu_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	Base(mconfig, type, tag, owner, clock)
{
}


template <typename Base>
void m6502_mcu_device_base<Base>::recompute_bcount(uint64_t event_time)
{
	if(!event_time || event_time >= this->total_cycles() + this->m_icount)
		this->m_bcount = 0;
	else
		this->m_bcount = this->total_cycles() + this->m_icount - event_time;
}

template <typename Base>
void m6502_mcu_device_base<Base>::execute_run()
{
	internal_update(this->total_cycles());

	this->m_icount -= this->m_count_before_instruction_step;
	if(this->m_icount < 0) {
		this->m_count_before_instruction_step = -this->m_icount;
		this->m_icount = 0;
	} else
		this->m_count_before_instruction_step = 0;

	while(this->m_bcount && this->m_icount <= this->m_bcount)
		internal_update(this->total_cycles() + this->m_icount - this->m_bcount);

	if(this->m_icount > 0 && this->m_inst_substate)
		this->do_exec_partial();

	while(this->m_icount > 0) {
		while(this->m_icount > this->m_bcount) {
			if(this->m_inst_state < 0xff00) {
				this->m_PPC = this->m_NPC;
				this->m_inst_state = this->m_IR | this->m_inst_state_base;
				if(this->debugger_enabled())
					this->debugger_instruction_hook(this->m_NPC);
			}
			this->do_exec_full();
		}
		if(this->m_icount > 0)
			while(this->m_bcount && this->m_icount <= this->m_bcount)
				internal_update(this->total_cycles() + this->m_icount - this->m_bcount);
		if(this->m_icount > 0 && this->m_inst_substate)
			this->do_exec_partial();
	}
	if(this->m_icount < 0) {
		this->m_count_before_instruction_step = -this->m_icount;
		this->m_icount = 0;
	}
}

#endif // MAME_CPU_M6502_M6502MCU_IPP
