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
	if(!event_time || event_time >= this->total_cycles() + this->icount)
		this->bcount = 0;
	else
		this->bcount = this->total_cycles() + this->icount - event_time;
}

template <typename Base>
void m6502_mcu_device_base<Base>::execute_run()
{
	internal_update(this->total_cycles());

	this->icount -= this->count_before_instruction_step;
	if(this->icount < 0) {
		this->count_before_instruction_step = -this->icount;
		this->icount = 0;
	} else
		this->count_before_instruction_step = 0;

	while(this->bcount && this->icount <= this->bcount)
		internal_update(this->total_cycles() + this->icount - this->bcount);

	if(this->icount > 0 && this->inst_substate)
		this->do_exec_partial();

	while(this->icount > 0) {
		while(this->icount > this->bcount) {
			if(this->inst_state < 0xff00) {
				this->PPC = this->NPC;
				this->inst_state = this->IR | this->inst_state_base;
				if(this->machine().debug_flags & DEBUG_FLAG_ENABLED)
					this->debugger_instruction_hook(this->NPC);
			}
			this->do_exec_full();
		}
		if(this->icount > 0)
			while(this->bcount && this->icount <= this->bcount)
				internal_update(this->total_cycles() + this->icount - this->bcount);
		if(this->icount > 0 && this->inst_substate)
			this->do_exec_partial();
	}
	if(this->icount < 0) {
		this->count_before_instruction_step = -this->icount;
		this->icount = 0;
	}
}

#endif // MAME_CPU_M6502_M6502MCU_IPP
