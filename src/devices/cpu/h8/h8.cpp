// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h8.h

    H8-300 base cpu emulation


***************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "h8.h"
#include "h8_dma.h"
#include "h8_dtc.h"
#include "h8d.h"

h8_device::h8_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor map_delegate) :
	cpu_device(mconfig, type, tag, owner, clock),
	program_config("program", ENDIANNESS_BIG, 16, 16, 0, map_delegate),
	io_config("io", ENDIANNESS_BIG, 16, 16, -1), PPC(0), NPC(0), PC(0), PIR(0), EXR(0), CCR(0), MAC(0), MACF(0),
	TMP1(0), TMP2(0), TMPR(0), inst_state(0), inst_substate(0), icount(0), bcount(0), irq_vector(0), taken_irq_vector(0), irq_level(0), taken_irq_level(0), irq_required(false), irq_nmi(false)
{
	supports_advanced = false;
	mode_advanced = false;
	mode_a20 = false;
	has_exr = false;
	mac_saturating = false;
	has_trace = false;
	has_hc = true;
}

void h8_device::device_config_complete()
{
	uint8_t addrbits = mode_advanced ? (mode_a20 ? 20 : 24) : 16;
	program_config.m_addr_width = program_config.m_logaddr_width = addrbits;
}

void h8_device::device_start()
{
	space(AS_PROGRAM).cache(cache);
	space(AS_PROGRAM).specific(program);
	space(AS_IO).specific(io);

	uint32_t pcmask = mode_advanced ? 0xffffff : 0xffff;
	state_add<uint32_t>(H8_PC, "PC",
		[this]() { return NPC; },
		[this](uint32_t pc) { PC = PPC = NPC = pc; prefetch_noirq_notrace(); }
	).mask(pcmask);
	state_add<uint32_t>(STATE_GENPC, "GENPC",
		[this]() { return NPC; },
		[this](uint32_t pc) { PC = PPC = NPC = pc; prefetch_noirq_notrace(); }
	).mask(pcmask).noshow();
	state_add(STATE_GENPCBASE, "CURPC",     PPC).mask(pcmask).noshow();
	state_add(H8_CCR,          "CCR",       CCR);
	if(has_exr)
		state_add(STATE_GENFLAGS,  "GENFLAGS",  CCR).formatstr("%11s").noshow();
	else
		state_add(STATE_GENFLAGS,  "GENFLAGS",  CCR).formatstr("%8s").noshow();

	if(has_exr)
		state_add(H8_EXR,          "EXR",       EXR);
	if(!supports_advanced) {
		state_add(H8_R0,           "R0",        R[0]);
		state_add(H8_R1,           "R1",        R[1]);
		state_add(H8_R2,           "R2",        R[2]);
		state_add(H8_R3,           "R3",        R[3]);
		state_add(H8_R4,           "R4",        R[4]);
		state_add(H8_R5,           "R5",        R[5]);
		state_add(H8_R6,           "R6",        R[6]);
		state_add(H8_R7,           "R7",        R[7]);
	} else {
		state_add(H8_R0,           "R0",        R[0]).noshow();
		state_add(H8_R1,           "R1",        R[1]).noshow();
		state_add(H8_R2,           "R2",        R[2]).noshow();
		state_add(H8_R3,           "R3",        R[3]).noshow();
		state_add(H8_R4,           "R4",        R[4]).noshow();
		state_add(H8_R5,           "R5",        R[5]).noshow();
		state_add(H8_R6,           "R6",        R[6]).noshow();
		state_add(H8_R7,           "R7",        R[7]).noshow();
		state_add(H8_E0,           "E0",        R[8]).noshow();
		state_add(H8_E1,           "E1",        R[9]).noshow();
		state_add(H8_E2,           "E2",        R[10]).noshow();
		state_add(H8_E3,           "E3",        R[11]).noshow();
		state_add(H8_E4,           "E4",        R[12]).noshow();
		state_add(H8_E5,           "E5",        R[13]).noshow();
		state_add(H8_E6,           "E6",        R[14]).noshow();
		state_add(H8_E7,           "E7",        R[15]).noshow();
		state_add(H8_R0,           "ER0",       TMPR).callimport().callexport().formatstr("%9s");
		state_add(H8_R1,           "ER1",       TMPR).callimport().callexport().formatstr("%9s");
		state_add(H8_R2,           "ER2",       TMPR).callimport().callexport().formatstr("%9s");
		state_add(H8_R3,           "ER3",       TMPR).callimport().callexport().formatstr("%9s");
		state_add(H8_R4,           "ER4",       TMPR).callimport().callexport().formatstr("%9s");
		state_add(H8_R5,           "ER5",       TMPR).callimport().callexport().formatstr("%9s");
		state_add(H8_R6,           "ER6",       TMPR).callimport().callexport().formatstr("%9s");
		state_add(H8_R7,           "ER7",       TMPR).callimport().callexport().formatstr("%9s");
	}

	save_item(NAME(PPC));
	save_item(NAME(NPC));
	save_item(NAME(PC));
	save_item(NAME(PIR));
	save_item(NAME(IR));
	save_item(NAME(R));
	save_item(NAME(EXR));
	save_item(NAME(CCR));
	save_item(NAME(TMP1));
	save_item(NAME(TMP2));
	save_item(NAME(inst_state));
	save_item(NAME(inst_substate));
	save_item(NAME(irq_vector));
	save_item(NAME(taken_irq_vector));
	save_item(NAME(irq_level));
	save_item(NAME(taken_irq_level));
	save_item(NAME(irq_nmi));

	set_icountptr(icount);

	PC = 0;
	PPC = 0;
	NPC = 0;
	memset(IR, 0, sizeof(IR));
	memset(R, 0, sizeof(R));
	EXR = 0;
	CCR = 0;
	MAC = 0;
	MACF = 0;
	inst_state = STATE_RESET;
	inst_substate = 0;
	count_before_instruction_step = 0;
	requested_state = -1;
	dma_device = nullptr;
	dtc_device = nullptr;
}

void h8_device::device_reset()
{
	inst_state = STATE_RESET;
	inst_substate = 0;
	count_before_instruction_step = 0;
	requested_state = -1;

	irq_vector = 0;
	irq_level = -1;
	irq_nmi = false;
	taken_irq_vector = 0;
	taken_irq_level = -1;
	current_dma = nullptr;
	current_dtc = nullptr;
}

bool h8_device::trigger_dma(int vector)
{
	return (dma_device && dma_device->trigger_dma(vector)) || (dtc_device && dtc_device->trigger_dtc(vector));
}

void h8_device::set_current_dma(h8_dma_state *state)
{
	current_dma = state;
	if(!state)
		logerror("DMA done\n");
	else
		logerror("New current dma s=%x d=%x is=%d id=%d count=%x m=%d autoreq=%d\n",
					state->source, state->dest, state->incs, state->incd,
					state->count, state->mode_16 ? 16 : 8, state->autoreq);

}

void h8_device::set_current_dtc(h8_dtc_state *state)
{
	current_dtc = state;
}

void h8_device::request_state(int state)
{
	requested_state = state;
}

uint32_t h8_device::execute_min_cycles() const noexcept
{
	return 1;
}

uint32_t h8_device::execute_max_cycles() const noexcept
{
	return 1;
}

uint32_t h8_device::execute_input_lines() const noexcept
{
	return 0;
}

bool h8_device::execute_input_edge_triggered(int inputnum) const noexcept
{
	return inputnum == INPUT_LINE_NMI;
}

void h8_device::recompute_bcount(uint64_t event_time)
{
	if(!event_time || event_time >= total_cycles() + icount) {
		bcount = 0;
		return;
	}
	bcount = total_cycles() + icount - event_time;
}

void h8_device::execute_run()
{
	internal_update(total_cycles());

	icount -= count_before_instruction_step;
	if(icount < 0) {
		count_before_instruction_step = -icount;
		icount = 0;
	} else
		count_before_instruction_step = 0;

	while(bcount && icount <= bcount)
		internal_update(total_cycles() + icount - bcount);

	if(icount > 0 && inst_substate)
		do_exec_partial();

	while(icount > 0) {
		while(icount > bcount) {
			if(inst_state < 0x10000) {
				PPC = NPC;
				if(machine().debug_flags & DEBUG_FLAG_ENABLED)
					debugger_instruction_hook(NPC);
			}
			do_exec_full();
		}
		if(icount > 0)
			while(bcount && icount <= bcount)
				internal_update(total_cycles() + icount - bcount);
		if(icount > 0 && inst_substate)
			do_exec_partial();
	}
	if(icount < 0) {
		count_before_instruction_step = -icount;
		icount = 0;
	}
}

void h8_device::add_event(uint64_t &event_time, uint64_t new_event)
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
		std::make_pair(AS_PROGRAM, &program_config),
		std::make_pair(AS_IO,      &io_config)
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
		R[r + 8] = TMPR >> 16;
		R[r] = TMPR;
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
		TMPR = (R[r + 8] << 16) | R[r];
		break;
	}
	}
}

void h8_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch(entry.index()) {
	case STATE_GENFLAGS:
		if(has_exr)
			str = string_format("%c%c %c%c%c%c%c%c%c%c",
					(EXR & EXR_T) ? 'T' : '-',
					'0' + (EXR & EXR_I),
					(CCR & F_I)  ? 'I' : '-',
					(CCR & F_UI) ? 'u' : '-',
					(CCR & F_H)  ? 'H' : '-',
					(CCR & F_U)  ? 'U' : '-',
					(CCR & F_N)  ? 'N' : '-',
					(CCR & F_Z)  ? 'Z' : '-',
					(CCR & F_V)  ? 'V' : '-',
					(CCR & F_C)  ? 'C' : '-');
		else if(has_hc)
			str = string_format("%c%c%c%c%c%c%c%c",
					(CCR & F_I)  ? 'I' : '-',
					(CCR & F_UI) ? 'u' : '-',
					(CCR & F_H)  ? 'H' : '-',
					(CCR & F_U)  ? 'U' : '-',
					(CCR & F_N)  ? 'N' : '-',
					(CCR & F_Z)  ? 'Z' : '-',
					(CCR & F_V)  ? 'V' : '-',
					(CCR & F_C)  ? 'C' : '-');
		else
			str = string_format("%c%c%c%c%c%c%c%c",
					(CCR & F_I)  ? '?' : '-',
					(CCR & F_UI) ? 'u' : '-',
					(CCR & F_H)  ? 'I' : '-',
					(CCR & F_U)  ? 'U' : '-',
					(CCR & F_N)  ? 'N' : '-',
					(CCR & F_Z)  ? 'Z' : '-',
					(CCR & F_V)  ? 'V' : '-',
					(CCR & F_C)  ? 'C' : '-');
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
		str = string_format("%04X %04X", R[r + 8], R[r]);
		break;
	}
	}
}

uint16_t h8_device::read16i(uint32_t adr)
{
	icount--;
	return cache.read_word(adr & ~1);
}

uint16_t h8_device::fetch()
{
	uint16_t res = read16i(PC);
	PC += 2;
	return res;
}

uint8_t h8_device::read8(uint32_t adr)
{
	icount--;
	return program.read_byte(adr);
}

void h8_device::write8(uint32_t adr, uint8_t data)
{
	icount--;
	program.write_byte(adr, data);
}

uint16_t h8_device::read16(uint32_t adr)
{
	icount--;
	return program.read_word(adr & ~1);
}

void h8_device::write16(uint32_t adr, uint16_t data)
{
	icount--;
	program.write_word(adr & ~1, data);
}

bool h8_device::exr_in_stack() const
{
	return false;
}

void h8_device::prefetch_done()
{
	if(requested_state != -1) {
		inst_state = requested_state;
		requested_state = -1;
	} else if(current_dma && !current_dma->suspended)
		inst_state = STATE_DMA;
	else if(current_dtc)
		inst_state = STATE_DTC;
	else if(irq_vector) {
		inst_state = STATE_IRQ;
		taken_irq_vector = irq_vector;
		taken_irq_level = irq_level;
	} else if(has_trace && (EXR & EXR_T) && exr_in_stack())
		inst_state = STATE_TRACE;
	else
		inst_state = IR[0] = PIR;
}

void h8_device::prefetch_done_noirq()
{
	if(has_trace && (EXR & EXR_T) && exr_in_stack())
		inst_state = STATE_TRACE;
	else
		inst_state = IR[0] = PIR;
}

void h8_device::prefetch_done_noirq_notrace()
{
	inst_state = IR[0] = PIR;
}

void h8_device::set_irq(int _irq_vector, int _irq_level, bool _irq_nmi)
{
	irq_vector = _irq_vector;
	irq_level = _irq_level;
	irq_nmi = _irq_nmi;
}

void h8_device::internal(int cycles)
{
	icount -= cycles;
}

void h8_device::illegal()
{
	logerror("Illegal instruction at address %x\n", PPC);
	icount = -10000000;
}

int h8_device::trace_setup()
{
	throw emu_fatalerror("%s: Trace setup called but unimplemented.\n", tag());
}

int h8_device::trapa_setup()
{
	throw emu_fatalerror("%s: Trapa setup called but unimplemented.\n", tag());
}

uint8_t h8_device::do_addx8(uint8_t v1, uint8_t v2)
{
	uint16_t res = v1 + v2 + (CCR & F_C ? 1 : 0);
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if (has_hc)
	{
		CCR &= ~F_H;
		if(((v1 & 0xf) + (v2 & 0xf) + (CCR & F_C ? 1 : 0)) & 0x10)
			CCR |= F_H;
	}
	if(!uint8_t(res))
		CCR |= F_Z;
	else if(int8_t(res) < 0)
		CCR |= F_N;
	if(~(v1^v2) & (v1^res) & 0x80)
		CCR |= F_V;
	if(res & 0x100)
		CCR |= F_C;
	return res;

}

uint8_t h8_device::do_subx8(uint8_t v1, uint8_t v2)
{
	uint16_t res = v1 - v2 - (CCR & F_C ? 1 : 0);
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if (has_hc)
	{
		CCR &= ~F_H;
		if (((v1 & 0xf) - (v2 & 0xf) - (CCR & F_C ? 1 : 0)) & 0x10)
			CCR |= F_H;
	}
	if(!uint8_t(res))
		CCR |= F_Z;
	else if(int8_t(res) < 0)
		CCR |= F_N;
	if((v1^v2) & (v1^res) & 0x80)
		CCR |= F_V;
	if(res & 0x100)
		CCR |= F_C;
	return res;

}

uint8_t h8_device::do_inc8(uint8_t v1, uint8_t v2)
{
	uint8_t res = v1 + v2;
	CCR &= ~(F_N|F_V|F_Z);
	if(!res)
		CCR |= F_Z;
	else if(int8_t(res) < 0)
		CCR |= F_N;
	if((v1^v2) & (v1^res) & 0x80)
		CCR |= F_V;
	return res;
}

uint16_t h8_device::do_inc16(uint16_t v1, uint16_t v2)
{
	uint16_t res = v1 + v2;
	CCR &= ~(F_N|F_V|F_Z);
	if(!res)
		CCR |= F_Z;
	else if(int16_t(res) < 0)
		CCR |= F_N;
	if((v1^v2) & (v1^res) & 0x8000)
		CCR |= F_V;
	return res;
}

uint32_t h8_device::do_inc32(uint32_t v1, uint32_t v2)
{
	uint32_t res = v1 + v2;
	CCR &= ~(F_N|F_V|F_Z);
	if(!res)
		CCR |= F_Z;
	else if(int32_t(res) < 0)
		CCR |= F_N;
	if((v1^v2) & (v1^res) & 0x80000000)
		CCR |= F_V;
	return res;
}

uint8_t h8_device::do_add8(uint8_t v1, uint8_t v2)
{
	uint16_t res = v1 + v2;
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if (has_hc)
	{
		CCR &= ~F_H;
		if (((v1 & 0xf) + (v2 & 0xf)) & 0x10)
			CCR |= F_H;
	}
	if(!uint8_t(res))
		CCR |= F_Z;
	else if(int8_t(res) < 0)
		CCR |= F_N;
	if(~(v1^v2) & (v1^res) & 0x80)
		CCR |= F_V;
	if(res & 0x100)
		CCR |= F_C;
	return res;

}

uint16_t h8_device::do_add16(uint16_t v1, uint16_t v2)
{
	uint32_t res = v1 + v2;
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if (has_hc)
	{
		CCR &= ~F_H;
		if (((v1 & 0xfff) + (v2 & 0xffff)) & 0x1000)
			CCR |= F_H;
	}
	if(!uint16_t(res))
		CCR |= F_Z;
	else if(int16_t(res) < 0)
		CCR |= F_N;
	if(~(v1^v2) & (v1^res) & 0x8000)
		CCR |= F_V;
	if(res & 0x10000)
		CCR |= F_C;
	return res;

}

uint32_t h8_device::do_add32(uint32_t v1, uint32_t v2)
{
	uint64_t res = uint64_t(v1) + uint64_t(v2);
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if (has_hc)
	{
		CCR &= ~F_H;
		if (((v1 & 0xfffffff) + (v2 & 0xfffffff)) & 0x10000000)
			CCR |= F_H;
	}
	if(!uint32_t(res))
		CCR |= F_Z;
	else if(int32_t(res) < 0)
		CCR |= F_N;
	if(~(v1^v2) & (v1^res) & 0x80000000)
		CCR |= F_V;
	if(res & 0x100000000U)
		CCR |= F_C;
	return res;
}

uint8_t h8_device::do_dec8(uint8_t v1, uint8_t v2)
{
	uint8_t res = v1 - v2;
	CCR &= ~(F_N|F_V|F_Z);
	if(!res)
		CCR |= F_Z;
	else if(int8_t(res) < 0)
		CCR |= F_N;
	if((v1^v2) & (v1^res) & 0x80)
		CCR |= F_V;
	return res;
}

uint16_t h8_device::do_dec16(uint16_t v1, uint16_t v2)
{
	uint16_t res = v1 - v2;
	CCR &= ~(F_N|F_V|F_Z);
	if(!res)
		CCR |= F_Z;
	else if(int16_t(res) < 0)
		CCR |= F_N;
	if((v1^v2) & (v1^res) & 0x8000)
		CCR |= F_V;
	return res;
}

uint32_t h8_device::do_dec32(uint32_t v1, uint32_t v2)
{
	uint32_t res = v1 - v2;
	CCR &= ~(F_N|F_V|F_Z);
	if(!res)
		CCR |= F_Z;
	else if(int32_t(res) < 0)
		CCR |= F_N;
	if((v1^v2) & (v1^res) & 0x80000000)
		CCR |= F_V;
	return res;
}

uint8_t h8_device::do_sub8(uint8_t v1, uint8_t v2)
{
	uint16_t res = v1 - v2;
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if (has_hc)
	{
		CCR &= ~F_H;
		if (((v1 & 0xf) - (v2 & 0xf)) & 0x10)
			CCR |= F_H;
	}
	if(!uint8_t(res))
		CCR |= F_Z;
	else if(int8_t(res) < 0)
		CCR |= F_N;
	if((v1^v2) & (v1^res) & 0x80)
		CCR |= F_V;
	if(res & 0x100)
		CCR |= F_C;
	return res;

}

uint16_t h8_device::do_sub16(uint16_t v1, uint16_t v2)
{
	uint32_t res = v1 - v2;
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if (has_hc)
	{
		CCR &= ~F_H;
		if (((v1 & 0xfff) - (v2 & 0xffff)) & 0x1000)
			CCR |= F_H;
	}
	if(!uint16_t(res))
		CCR |= F_Z;
	else if(int16_t(res) < 0)
		CCR |= F_N;
	if((v1^v2) & (v1^res) & 0x8000)
		CCR |= F_V;
	if(res & 0x10000)
		CCR |= F_C;
	return res;

}

uint32_t h8_device::do_sub32(uint32_t v1, uint32_t v2)
{
	uint64_t res = uint64_t(v1) - uint64_t(v2);
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if (has_hc)
	{
		CCR &= ~F_H;
		if (((v1 & 0xfffffff) - (v2 & 0xfffffff)) & 0x10000000)
			CCR |= F_H;
	}
	if(!uint32_t(res))
		CCR |= F_Z;
	else if(int32_t(res) < 0)
		CCR |= F_N;
	if((v1^v2) & (v1^res) & 0x80000000)
		CCR |= F_V;
	if(res & 0x100000000U)
		CCR |= F_C;
	return res;
}

uint8_t h8_device::do_shal8(uint8_t v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x80)
		CCR |= F_C;
	if((v & 0xc0) == 0x40 || (v & 0xc0) == 0x80)
		CCR |= F_V;
	v <<= 1;
	if(!v)
		CCR |= F_Z;
	else if(int8_t(v) < 0)
		CCR |= F_N;
	return v;
}

uint16_t h8_device::do_shal16(uint16_t v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x8000)
		CCR |= F_C;
	if((v & 0xc000) == 0x4000 || (v & 0xc000) == 0x8000)
		CCR |= F_V;
	v <<= 1;
	if(!v)
		CCR |= F_Z;
	else if(int16_t(v) < 0)
		CCR |= F_N;
	return v;
}

uint32_t h8_device::do_shal32(uint32_t v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x80000000)
		CCR |= F_C;
	if((v & 0xc0000000) == 0x40000000 || (v & 0xc0000000) == 0x80000000)
		CCR |= F_V;
	v <<= 1;
	if(!v)
		CCR |= F_Z;
	else if(int32_t(v) < 0)
		CCR |= F_N;
	return v;
}

uint8_t h8_device::do_shar8(uint8_t v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 1)
		CCR |= F_C;
	v >>= 1;
	if(!v)
		CCR |= F_Z;
	else if (v & 0x40) {
		v |= 0x80;
		CCR |= F_N;
	}
	return v;
}

uint16_t h8_device::do_shar16(uint16_t v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 1)
		CCR |= F_C;
	v >>= 1;
	if(!v)
		CCR |= F_Z;
	else if (v & 0x4000) {
		v |= 0x8000;
		CCR |= F_N;
	}
	return v;
}

uint32_t h8_device::do_shar32(uint32_t v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 1)
		CCR |= F_C;
	v >>= 1;
	if(!v)
		CCR |= F_Z;
	else if (v & 0x40000000) {
		v |= 0x80000000;
		CCR |= F_N;
	}
	return v;
}

uint8_t h8_device::do_shll8(uint8_t v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x80)
		CCR |= F_C;
	v <<= 1;
	if(!v)
		CCR |= F_Z;
	else if(int8_t(v) < 0)
		CCR |= F_N;
	return v;
}

uint16_t h8_device::do_shll16(uint16_t v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x8000)
		CCR |= F_C;
	v <<= 1;
	if(!v)
		CCR |= F_Z;
	else if(int16_t(v) < 0)
		CCR |= F_N;
	return v;
}

uint32_t h8_device::do_shll32(uint32_t v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x80000000)
		CCR |= F_C;
	v <<= 1;
	if(!v)
		CCR |= F_Z;
	else if(int32_t(v) < 0)
		CCR |= F_N;
	return v;
}

uint8_t h8_device::do_shlr8(uint8_t v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 1)
		CCR |= F_C;
	v >>= 1;
	if(!v)
		CCR |= F_Z;
	return v;
}

uint16_t h8_device::do_shlr16(uint16_t v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 1)
		CCR |= F_C;
	v >>= 1;
	if(!v)
		CCR |= F_Z;
	return v;
}

uint32_t h8_device::do_shlr32(uint32_t v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 1)
		CCR |= F_C;
	v >>= 1;
	if(!v)
		CCR |= F_Z;
	return v;
}

uint8_t h8_device::do_shal2_8(uint8_t v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x40)
		CCR |= F_C;
	if((v & 0xc0) == 0x40 || (v & 0xc0) == 0x80 ||
		(v & 0x60) == 0x20 || (v & 0x60) == 0x40)
		CCR |= F_V;
	v <<= 2;
	if(!v)
		CCR |= F_Z;
	else if(int8_t(v) < 0)
		CCR |= F_N;
	return v;
}

uint16_t h8_device::do_shal2_16(uint16_t v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x4000)
		CCR |= F_C;
	if((v & 0xc000) == 0x4000 || (v & 0xc000) == 0x8000 ||
		(v & 0x6000) == 0x2000 || (v & 0x6000) == 0x4000)
		CCR |= F_V;
	v <<= 2;
	if(!v)
		CCR |= F_Z;
	else if(int16_t(v) < 0)
		CCR |= F_N;
	return v;
}

uint32_t h8_device::do_shal2_32(uint32_t v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x40000000)
		CCR |= F_C;
	if((v & 0xc0000000) == 0x40000000 || (v & 0xc0000000) == 0x80000000 ||
		(v & 0x60000000) == 0x20000000 || (v & 0x60000000) == 0x40000000)
		CCR |= F_V;
	v <<= 2;
	if(!v)
		CCR |= F_Z;
	else if(int32_t(v) < 0)
		CCR |= F_N;
	return v;
}

uint8_t h8_device::do_shar2_8(uint8_t v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 2)
		CCR |= F_C;
	v >>= 2;
	if(!v)
		CCR |= F_Z;
	else if (v & 0x20) {
		v |= 0xc0;
		CCR |= F_N;
	}
	return v;
}

uint16_t h8_device::do_shar2_16(uint16_t v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 2)
		CCR |= F_C;
	v >>= 2;
	if(!v)
		CCR |= F_Z;
	else if (v & 0x2000) {
		v |= 0xc000;
		CCR |= F_N;
	}
	return v;
}

uint32_t h8_device::do_shar2_32(uint32_t v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 2)
		CCR |= F_C;
	v >>= 2;
	if(!v)
		CCR |= F_Z;
	else if (v & 0x20000000) {
		v |= 0xc0000000;
		CCR |= F_N;
	}
	return v;
}

uint8_t h8_device::do_shll2_8(uint8_t v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x40)
		CCR |= F_C;
	v <<= 2;
	if(!v)
		CCR |= F_Z;
	else if(int8_t(v) < 0)
		CCR |= F_N;
	return v;
}

uint16_t h8_device::do_shll2_16(uint16_t v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x4000)
		CCR |= F_C;
	v <<= 2;
	if(!v)
		CCR |= F_Z;
	else if(int16_t(v) < 0)
		CCR |= F_N;
	return v;
}

uint32_t h8_device::do_shll2_32(uint32_t v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x40000000)
		CCR |= F_C;
	v <<= 2;
	if(!v)
		CCR |= F_Z;
	else if(int32_t(v) < 0)
		CCR |= F_N;
	return v;
}

uint8_t h8_device::do_shlr2_8(uint8_t v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 2)
		CCR |= F_C;
	v >>= 2;
	if(!v)
		CCR |= F_Z;
	return v;
}

uint16_t h8_device::do_shlr2_16(uint16_t v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 2)
		CCR |= F_C;
	v >>= 2;
	if(!v)
		CCR |= F_Z;
	return v;
}

uint32_t h8_device::do_shlr2_32(uint32_t v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 2)
		CCR |= F_C;
	v >>= 2;
	if(!v)
		CCR |= F_Z;
	return v;
}

uint8_t h8_device::do_rotl8(uint8_t v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x80)
		CCR |= F_C;
	v = (v << 1) | (v >> 7);
	if(!v)
		CCR |= F_Z;
	else if(int8_t(v) < 0)
		CCR |= F_N;
	return v;
}

uint16_t h8_device::do_rotl16(uint16_t v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x8000)
		CCR |= F_C;
	v = (v << 1) | (v >> 15);
	if(!v)
		CCR |= F_Z;
	else if(int16_t(v) < 0)
		CCR |= F_N;
	return v;
}

uint32_t h8_device::do_rotl32(uint32_t v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x80000000)
		CCR |= F_C;
	v = (v << 1) | (v >> 31);
	if(!v)
		CCR |= F_Z;
	else if(int32_t(v) < 0)
		CCR |= F_N;
	return v;
}

uint8_t h8_device::do_rotr8(uint8_t v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x01)
		CCR |= F_C;
	v = (v << 7) | (v >> 1);
	if(!v)
		CCR |= F_Z;
	else if(int8_t(v) < 0)
		CCR |= F_N;
	return v;
}

uint16_t h8_device::do_rotr16(uint16_t v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x0001)
		CCR |= F_C;
	v = (v << 15) | (v >> 1);
	if(!v)
		CCR |= F_Z;
	else if(int16_t(v) < 0)
		CCR |= F_N;
	return v;
}

uint32_t h8_device::do_rotr32(uint32_t v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x00000001)
		CCR |= F_C;
	v = (v << 31) | (v >> 1);
	if(!v)
		CCR |= F_Z;
	else if(int32_t(v) < 0)
		CCR |= F_N;
	return v;
}

uint8_t h8_device::do_rotxl8(uint8_t v)
{
	uint8_t c = CCR & F_C ? 1 : 0;
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x80)
		CCR |= F_C;
	v = (v << 1) | c;
	if(!v)
		CCR |= F_Z;
	else if(int8_t(v) < 0)
		CCR |= F_N;
	return v;
}

uint16_t h8_device::do_rotxl16(uint16_t v)
{
	uint16_t c = CCR & F_C ? 1 : 0;
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x8000)
		CCR |= F_C;
	v = (v << 1) | c;
	if(!v)
		CCR |= F_Z;
	else if(int16_t(v) < 0)
		CCR |= F_N;
	return v;
}

uint32_t h8_device::do_rotxl32(uint32_t v)
{
	uint32_t c = CCR & F_C ? 1 : 0;
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x80000000)
		CCR |= F_C;
	v = (v << 1) | c;
	if(!v)
		CCR |= F_Z;
	else if(int32_t(v) < 0)
		CCR |= F_N;
	return v;
}

uint8_t h8_device::do_rotxr8(uint8_t v)
{
	uint8_t c = CCR & F_C ? 1 : 0;
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x01)
		CCR |= F_C;
	v = (v >> 1) | (c << 7);
	if(!v)
		CCR |= F_Z;
	else if(int8_t(v) < 0)
		CCR |= F_N;
	return v;
}

uint16_t h8_device::do_rotxr16(uint16_t v)
{
	uint8_t c = CCR & F_C ? 1 : 0;
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x0001)
		CCR |= F_C;
	v = (v >> 1) | (c << 15);
	if(!v)
		CCR |= F_Z;
	else if(int16_t(v) < 0)
		CCR |= F_N;
	return v;
}

uint32_t h8_device::do_rotxr32(uint32_t v)
{
	uint8_t c = CCR & F_C ? 1 : 0;
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x00000001)
		CCR |= F_C;
	v = (v >> 1) | (c << 31);
	if(!v)
		CCR |= F_Z;
	else if(int32_t(v) < 0)
		CCR |= F_N;
	return v;
}

uint8_t h8_device::do_rotl2_8(uint8_t v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x40)
		CCR |= F_C;
	v = (v << 2) | (v >> 6);
	if(!v)
		CCR |= F_Z;
	else if(int8_t(v) < 0)
		CCR |= F_N;
	return v;
}

uint16_t h8_device::do_rotl2_16(uint16_t v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x4000)
		CCR |= F_C;
	v = (v << 2) | (v >> 14);
	if(!v)
		CCR |= F_Z;
	else if(int16_t(v) < 0)
		CCR |= F_N;
	return v;
}

uint32_t h8_device::do_rotl2_32(uint32_t v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x40000000)
		CCR |= F_C;
	v = (v << 2) | (v >> 30);
	if(!v)
		CCR |= F_Z;
	else if(int32_t(v) < 0)
		CCR |= F_N;
	return v;
}

uint8_t h8_device::do_rotr2_8(uint8_t v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x02)
		CCR |= F_C;
	v = (v << 6) | (v >> 2);
	if(!v)
		CCR |= F_Z;
	else if(int8_t(v) < 0)
		CCR |= F_N;
	return v;
}

uint16_t h8_device::do_rotr2_16(uint16_t v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x0002)
		CCR |= F_C;
	v = (v << 14) | (v >> 2);
	if(!v)
		CCR |= F_Z;
	else if(int16_t(v) < 0)
		CCR |= F_N;
	return v;
}

uint32_t h8_device::do_rotr2_32(uint32_t v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x00000002)
		CCR |= F_C;
	v = (v << 30) | (v >> 2);
	if(!v)
		CCR |= F_Z;
	else if(int32_t(v) < 0)
		CCR |= F_N;
	return v;
}

uint8_t h8_device::do_rotxl2_8(uint8_t v)
{
	uint8_t c = CCR & F_C ? 1 : 0;
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x40)
		CCR |= F_C;
	v = (v << 2) | (c << 1) | ((v >> 6) & 0x01);
	if(!v)
		CCR |= F_Z;
	else if(int8_t(v) < 0)
		CCR |= F_N;
	return v;
}

uint16_t h8_device::do_rotxl2_16(uint16_t v)
{
	uint16_t c = CCR & F_C ? 1 : 0;
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x4000)
		CCR |= F_C;
	v = (v << 2) | (c << 1) | ((v >> 14) & 0x0001);
	if(!v)
		CCR |= F_Z;
	else if(int16_t(v) < 0)
		CCR |= F_N;
	return v;
}

uint32_t h8_device::do_rotxl2_32(uint32_t v)
{
	uint32_t c = CCR & F_C ? 1 : 0;
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x40000000)
		CCR |= F_C;
	v = (v << 2) | (c << 1) | ((v >> 30) & 0x00000001);
	if(!v)
		CCR |= F_Z;
	else if(int32_t(v) < 0)
		CCR |= F_N;
	return v;
}

uint8_t h8_device::do_rotxr2_8(uint8_t v)
{
	uint8_t c = CCR & F_C ? 1 : 0;
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x02)
		CCR |= F_C;
	v = (v >> 2) | (c << 6) | (v << 7);
	if(!v)
		CCR |= F_Z;
	else if(int8_t(v) < 0)
		CCR |= F_N;
	return v;
}

uint16_t h8_device::do_rotxr2_16(uint16_t v)
{
	uint16_t c = CCR & F_C ? 1 : 0;
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x0002)
		CCR |= F_C;
	v = (v >> 2) | (c << 14) | (v << 15);
	if(!v)
		CCR |= F_Z;
	else if(int16_t(v) < 0)
		CCR |= F_N;
	return v;
}

uint32_t h8_device::do_rotxr2_32(uint32_t v)
{
	uint32_t c = CCR & F_C ? 1 : 0;
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x00000002)
		CCR |= F_C;
	v = (v >> 2) | (c << 30) | (v << 31);
	if(!v)
		CCR |= F_Z;
	else if(int32_t(v) < 0)
		CCR |= F_N;
	return v;
}

void h8_device::set_nzv8(uint8_t v)
{
	CCR &= ~(F_N|F_V|F_Z);
	if(!v)
		CCR |= F_Z;
	else if(int8_t(v) < 0)
		CCR |= F_N;
}

void h8_device::set_nzv16(uint16_t v)
{
	CCR &= ~(F_N|F_V|F_Z);
	if(!v)
		CCR |= F_Z;
	else if(int16_t(v) < 0)
		CCR |= F_N;
}

void h8_device::set_nzv32(uint32_t v)
{
	CCR &= ~(F_N|F_V|F_Z);
	if(!v)
		CCR |= F_Z;
	else if(int32_t(v) < 0)
		CCR |= F_N;
}

void h8_device::set_nz16(uint16_t v)
{
	CCR &= ~(F_N|F_Z);
	if(!v)
		CCR |= F_Z;
	else if(int16_t(v) < 0)
		CCR |= F_N;
}

void h8_device::set_nz32(uint32_t v)
{
	CCR &= ~(F_N|F_Z);
	if(!v)
		CCR |= F_Z;
	else if(int32_t(v) < 0)
		CCR |= F_N;
}

std::unique_ptr<util::disasm_interface> h8_device::create_disassembler()
{
	return std::make_unique<h8_disassembler>();
}

#include "cpu/h8/h8.hxx"
