// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m6502.cpp

    MOS Technology 6502, original NMOS variant

***************************************************************************/

#include "emu.h"
#include "m6502.h"
#include "m6502d.h"
//#include "bus/nes/mmc5.h"

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
	sync_w(*this),
	m_mmc5_reset_scanline_irq(*this),
	m_mmc5_ppuctrl_write(*this),
	m_mmc5_ppumask_write(*this),
	program_config("program", ENDIANNESS_LITTLE, 8, 16),
	sprogram_config("decrypted_opcodes", ENDIANNESS_LITTLE, 8, 16),
	mintf(nullptr),
	uses_custom_memory_interface(false)
{
}

void m6502_device::device_start()
{
	if(!uses_custom_memory_interface)
		mintf = space(AS_PROGRAM).addr_width() > 14 ? std::make_unique<mi_default>() : std::make_unique<mi_default14>();

	init();
}

void m6502_device::init()
{
	if(mintf) {
		space(AS_PROGRAM).cache(mintf->cprogram);
		space(has_space(AS_OPCODES) ? AS_OPCODES : AS_PROGRAM).cache(mintf->csprogram);

		// specific group 1-14 or 15-31
		if(space(AS_PROGRAM).addr_width() > 14) {
			space(AS_PROGRAM).specific(mintf->program);
			space(AS_PROGRAM).specific(mintf->m_program);
			mintf->m_cprogram = mintf->m_program;
			if (has_space(AS_OPCODES))
				space(AS_OPCODES).specific(mintf->m_csprogram);
			else
				mintf->m_csprogram = mintf->m_program;
		} else {
			space(AS_PROGRAM).specific(mintf->program14);
			space(AS_PROGRAM).specific(mintf->m_program14);
			mintf->m_cprogram14 = mintf->m_program14;
			if (has_space(AS_OPCODES))
				space(AS_OPCODES).specific(mintf->m_csprogram14);
			else
				mintf->m_csprogram14 = mintf->m_program14;
		}
	}

	state_add(STATE_GENPC,     "GENPC",     XPC).callexport().noshow();
	state_add(STATE_GENPCBASE, "CURPC",     XPC).callexport().noshow();
	state_add(STATE_GENFLAGS,  "GENFLAGS",  P).callimport().formatstr("%6s").noshow();
	state_add(M6502_PC,        "PC",        NPC).callimport();
	state_add(M6502_A,         "A",         A);
	state_add(M6502_X,         "X",         X);
	state_add(M6502_Y,         "Y",         Y);
	state_add(M6502_P,         "P",         P).callimport();
	state_add(M6502_S,         "SP",        SP);
	state_add(M6502_IR,        "IR",        IR);

	save_item(NAME(PC));
	save_item(NAME(NPC));
	save_item(NAME(PPC));
	save_item(NAME(A));
	save_item(NAME(X));
	save_item(NAME(Y));
	save_item(NAME(P));
	save_item(NAME(SP));
	save_item(NAME(TMP));
	save_item(NAME(TMP2));
	save_item(NAME(IR));
	save_item(NAME(nmi_state));
	save_item(NAME(irq_state));
	save_item(NAME(apu_irq_state));
	save_item(NAME(v_state));
	save_item(NAME(nmi_pending));
	save_item(NAME(irq_taken));
	save_item(NAME(inst_state));
	save_item(NAME(inst_substate));
	save_item(NAME(inst_state_base));
	save_item(NAME(inhibit_interrupts));
	
	save_item(NAME(sync));
	save_item(NAME(count_before_instruction_step));

	save_item(NAME(prev_IR));
	save_item(NAME(next_IR));
	save_item(NAME(cpu_is_reading));
	save_item(NAME(cpu_data_bus));
	save_item(NAME(cpu_external_bus));
	save_item(NAME(adr_bus));
	save_item(NAME(delay));
	save_item(NAME(nmi_pending_1));
	save_item(NAME(irq_delay));
	save_item(NAME(apu_irq_delay));
	save_item(NAME(nmi_delay));
	save_item(NAME(branched));
	save_item(NAME(paged));
	save_item(NAME(oam_dma_halt_cycle));
	save_item(NAME(rmw_1));
	save_item(NAME(apu_irq_branch_delay));
	save_item(NAME(irq_branch_delay));
	save_item(NAME(nmi_branch_delay));
	save_item(NAME(apu_clk1_is_high));
	save_item(NAME(dmc_halt));
	save_item(NAME(oam_halt));
	save_item(NAME(dmc_dma_explicit_stop));
	save_item(NAME(write_cycles_since_dma_halt_request));
	save_item(NAME(dmc_dma_reload));
	save_item(NAME(prev_4016_write));
	save_item(NAME(prev_4017_write));
	save_item(NAME(prev_4016_read));
	save_item(NAME(prev_4017_read));
	save_item(NAME(last_4016_val));
	save_item(NAME(last_4017_val));
	save_item(NAME(inst_halted));
	save_item(NAME(next_read));
	save_item(NAME(prev_next_read));
	save_item(NAME(need_irq));
	save_item(NAME(prevReadAddress));
	save_item(NAME(m_exram_control));
	//save_item(NAME(is_mmc5));
	save_item(NAME(m_open_bus_ranges));
	save_item(NAME(m_ob_count));
	save_item(NAME(nmi_cpu_cycle));
	save_item(NAME(nmi_overlap_brk_irq));
	save_item(NAME(m_real_brk));	
	save_item(NAME(mapper_irq));
	save_item(NAME(mapper_irq_delay));
	save_item(NAME(mapper_irq_cpu_cycle));	
	save_item(NAME(last_cpu_write_latch));

	set_icountptr(icount);

	XPC = 0x0000;
	PPC = 0x0000;
	PC = 0x0000;
	NPC = 0x0000;
	A = 0x00;
	X = 0x00; //80
	Y = 0x00;
	P = 0x00; //36 //34 //24 //30
	//P = F_I;
	SP = 0x0100; //0x0100 //fd
	TMP = 0x0000;
	TMP2 = 0x00;
	IR = 0x00;
	nmi_state = false;
	irq_state = false;
	apu_irq_state = false;
	v_state = false;
	nmi_pending = false;
	irq_taken = false;
	inst_state = STATE_RESET;
	inst_substate = 0;
	inst_state_base = 0;
	sync = false;
	inhibit_interrupts = false;
	count_before_instruction_step = 0;
	
	prev_IR = 0x00;
	next_IR = 0x00;
	cpu_is_reading = true;
	cpu_data_bus = 0x00;
	cpu_external_bus = 0x00;
	adr_bus = 0x0000;
	delay = 0;
	nmi_pending_1 = false;
	irq_delay = false;
	apu_irq_delay = false;
	nmi_delay = false;
	branched = false;
	paged = false;
	oam_dma_halt_cycle = 0;
	rmw_1 = false;
	apu_irq_branch_delay = false;
	irq_branch_delay = false;
	nmi_branch_delay = false;
	apu_clk1_is_high = false;
	dmc_halt = false;
	oam_halt = false;
	dmc_dma_explicit_stop = false;
	write_cycles_since_dma_halt_request = 0;
	dmc_dma_reload = false;
	prev_4016_write = 0;
	prev_4017_write = 0;
	prev_4016_read = 0;
	prev_4017_read = 0;
	last_4016_val = 0;
	last_4017_val = 0;
	inst_halted = false;
	next_read = false;
	prev_next_read = false;
	need_irq = false;
	prevReadAddress = 0x0000;
	m_exram_control = 0;
	//is_mmc5 = false;
	//m_mmc5 = nullptr;
	std::fill(std::begin(m_open_bus_ranges), std::end(m_open_bus_ranges), 0);
	m_ob_count = 0;
	nmi_cpu_cycle = 0;
	nmi_overlap_brk_irq = false;
	m_real_brk = false;
	mapper_irq = false;
	mapper_irq_delay = 0;
	mapper_irq_cpu_cycle = 0;
	last_cpu_write_latch = 0x00;
	m_last_cpu_write_cycle = 0;
	m_previous_cpu_write_cycle = 0;
}

void m6502_device::device_reset()
{
	inst_state = STATE_RESET;
	inst_substate = 0;
	inst_state_base = 0;
	nmi_pending = false;
	irq_taken = false;
	sync = false;
	sync_w(CLEAR_LINE);
	inhibit_interrupts = false;
	
	prev_IR = 0x00;
	next_IR = 0x00;
	cpu_is_reading = true;
	cpu_data_bus = 0x00;
	cpu_external_bus = 0x00;
	adr_bus = 0x0000;
	delay = 0;
	nmi_pending_1 = false;
	irq_delay = false;
	apu_irq_delay = false;
	nmi_delay = false;
	branched = false;
	paged = false;
	oam_dma_halt_cycle = 0;
	rmw_1 = false;
	apu_irq_branch_delay = false;
	irq_branch_delay = false;
	nmi_branch_delay = false;
	apu_clk1_is_high = false;
	dmc_halt = false;
	oam_halt = false;
	dmc_dma_explicit_stop = false;
	write_cycles_since_dma_halt_request = 0;
	dmc_dma_reload = false;
	prev_4016_write = 0;
	prev_4017_write = 0;
	prev_4016_read = 0;
	prev_4017_read = 0;
	last_4016_val = 0;
	last_4017_val = 0;
	inst_halted = false;
	next_read = false;
	prev_next_read = false;
	need_irq = false;
	prevReadAddress = 0x0000;
	nmi_state = false;
	irq_state = false;
	apu_irq_state = false;
	v_state = false;
	XPC = 0x0000;
	nmi_cpu_cycle = 0;
	nmi_overlap_brk_irq = false;
	m_real_brk = false;
	mapper_irq = false;
	mapper_irq_delay = 0;
	mapper_irq_cpu_cycle = 0;
	last_cpu_write_latch = 0x00;
	m_last_cpu_write_cycle = 0;
	m_previous_cpu_write_cycle = 0;
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
	uint8_t c = P & F_C ? 1 : 0;
	P &= ~(F_N|F_V|F_Z|F_C);
	uint8_t al = (A & 15) + (val & 15) + c;
	if(al > 9)
		al += 6;
	uint8_t ah = (A >> 4) + (val >> 4) + (al > 15);
	if(!uint8_t(A + val + c))
		P |= F_Z;
	else if(ah & 8)
		P |= F_N;
	if(~(A^val) & (A^(ah << 4)) & 0x80)
		P |= F_V;
	if(ah > 9)
		ah += 6;
	if(ah > 15)
		P |= F_C;
	A = (ah << 4) | (al & 15);
}

void m6502_device::do_adc_nd(uint8_t val)
{
	uint16_t sum;
	sum = A + val + (P & F_C ? 1 : 0);
	P &= ~(F_N|F_V|F_Z|F_C);
	if(!uint8_t(sum))
		P |= F_Z;
	else if(int8_t(sum) < 0)
		P |= F_N;
	if(~(A^val) & (A^sum) & 0x80)
		P |= F_V;
	if(sum & 0xff00)
		P |= F_C;
	A = sum;
}

void m6502_device::do_adc(uint8_t val)
{
	if(P & F_D)
		do_adc_d(val);
	else
		do_adc_nd(val);
}

void m6502_device::do_arr_nd()
{
	bool c = P & F_C;
	P &= ~(F_N|F_Z|F_C|F_V);
	A >>= 1;
	if(c)
		A |= 0x80;
	if(!A)
		P |= F_Z;
	else if(int8_t(A)<0)
		P |= F_N;
	if(A & 0x40)
		P |= F_V|F_C;
	if(A & 0x20)
		P ^= F_V;
}

void m6502_device::do_arr_d()
{
	// The adc/ror interaction gives an extremely weird result
	bool c = P & F_C;
	P &= ~(F_N|F_Z|F_C|F_V);
	uint8_t a = A >> 1;
	if(c)
		a |= 0x80;
	if(!a)
		P |= F_Z;
	else if(int8_t(a) < 0)
		P |= F_N;
	if((a ^ A) & 0x40)
		P |= F_V;

	if((A & 0x0f) >= 0x05)
		a = ((a + 6) & 0x0f) | (a & 0xf0);

	if((A & 0xf0) >= 0x50) {
		a += 0x60;
		P |= F_C;
	}
	A = a;
}

void m6502_device::do_arr()
{
	if(P & F_D)
		do_arr_d();
	else
		do_arr_nd();
}

void m6502_device::do_cmp(uint8_t val1, uint8_t val2)
{
	P &= ~(F_N|F_Z|F_C);
	uint16_t r = val1-val2;
	if(!r)
		P |= F_Z;
	else if(int8_t(r) < 0)
		P |= F_N;
	if(!(r & 0xff00))
		P |= F_C;
}

void m6502_device::do_sbc_d(uint8_t val)
{
	uint8_t c = P & F_C ? 0 : 1;
	P &= ~(F_N|F_V|F_Z|F_C);
	uint16_t diff = A - val - c;
	uint8_t al = (A & 15) - (val & 15) - c;
	if(int8_t(al) < 0)
		al -= 6;
	uint8_t ah = (A >> 4) - (val >> 4) - (int8_t(al) < 0);
	if(!uint8_t(diff))
		P |= F_Z;
	else if(diff & 0x80)
		P |= F_N;
	if((A^val) & (A^diff) & 0x80)
		P |= F_V;
	if(!(diff & 0xff00))
		P |= F_C;
	if(int8_t(ah) < 0)
		ah -= 6;
	A = (ah << 4) | (al & 15);
}

void m6502_device::do_sbc_nd(uint8_t val)
{
	uint16_t diff = A - val - (P & F_C ? 0 : 1);
	P &= ~(F_N|F_V|F_Z|F_C);
	if(!uint8_t(diff))
		P |= F_Z;
	else if(int8_t(diff) < 0)
		P |= F_N;
	if((A^val) & (A^diff) & 0x80)
		P |= F_V;
	if(!(diff & 0xff00))
		P |= F_C;
	A = diff;
}

void m6502_device::do_sbc(uint8_t val)
{
	if(P & F_D)
		do_sbc_d(val);
	else
		do_sbc_nd(val);
}

void m6502_device::do_bit(uint8_t val)
{
	P &= ~(F_N|F_Z|F_V);
	uint8_t r = A & val;
	if(!r)
		P |= F_Z;
	if(val & 0x80)
		P |= F_N;
	if(val & 0x40)
		P |= F_V;
}

uint8_t m6502_device::do_asl(uint8_t v)
{
	P &= ~(F_N|F_Z|F_C);
	uint8_t r = v<<1;
	if(!r)
		P |= F_Z;
	else if(int8_t(r) < 0)
		P |= F_N;
	if(v & 0x80)
		P |= F_C;
	return r;
}

uint8_t m6502_device::do_lsr(uint8_t v)
{
	P &= ~(F_N|F_Z|F_C);
	if(v & 1)
		P |= F_C;
	v >>= 1;
	if(!v)
		P |= F_Z;
	return v;
}

uint8_t m6502_device::do_ror(uint8_t v)
{
	bool c = P & F_C;
	P &= ~(F_N|F_Z|F_C);
	if(v & 1)
		P |= F_C;
	v >>= 1;
	if(c)
		v |= 0x80;
	if(!v)
		P |= F_Z;
	else if(int8_t(v)<0)
		P |= F_N;
	return v;
}

uint8_t m6502_device::do_rol(uint8_t v)
{
	bool c = P & F_C;
	P &= ~(F_N|F_Z|F_C);
	if(v & 0x80)
		P |= F_C;
	v <<= 1;
	if(c)
		v |= 0x01;
	if(!v)
		P |= F_Z;
	else if(int8_t(v)<0)
		P |= F_N;
	return v;
}

uint8_t m6502_device::do_asr(uint8_t v)
{
	P &= ~(F_N|F_Z|F_C);
	if(v & 1)
		P |= F_C;
	v >>= 1;
	if(!v)
		P |= F_Z;
	else if(v & 0x40) {
		P |= F_N;
		v |= 0x80;
	}
	return v;
}

offs_t m6502_device::pc_to_external(u16 pc)
{
	return pc;
}

void m6502_device::handle_dma_rdy_stall()
{
    // NES DMA drives RDY low after a haltable CPU bus cycle has already executed
    // in this generated MAME 6502 core.  We model the stolen cycle by suspending
    // the CPU for one cycle, then rolling back the micro-op state so the same
    // bus cycle is retried when execution resumes.
    if (suspended() && (oam_halt || dmc_halt))
    {
        inst_halted = true;

        if (oam_halt)
            mark_oam_dma_halt_cycle();

        if (inst_substate > 0)
        {
            inst_substate--;
        }
        else
        {
            logerror("DMA/RDY stall with inst_substate == 0 pc=%04x IR=%02x cycle=%llu\n",
                PC, IR, (unsigned long long)total_cycles());
        }

        dmc_halt = false;
        oam_halt = false;

        // If the stolen cycle overlapped opcode fetch/sync, restore the previous
        // opcode identity so the retried cycle doesn't advance the instruction stream.
        if (sync)
        {
            next_IR = IR;
            IR = prev_IR;
        }
    }
}

void m6502_device::execute_run()
{	
	apu_clk1_is_high = (((total_cycles()) & 0x01) == 0);
	prev_next_read = next_read;
	
	/* 
		Pass NMI_BRK and NMI_IRQ tests from Blarrg
		-- NMI Timing to interupt BRK/IRQ -- 
		Fire NMI deep into the BRK/IRQ opcode here
		but only before set/clear B flag and not after
	*/
	if (nmi_pending_1 && IR == 0x00 && !nmi_overlap_brk_irq)
	{
		nmi_overlap_brk_irq = true;

		if (m_real_brk)
		{
			if (inst_substate < 9)
			{
				nmi_pending_1 = false;
				delay = 0;
				pulse_input_line(m6502_device::NMI_LINE, attotime::zero);
				osd_printf_info("Detected: NMI interrupt BRK = Now, delay: %d\n", delay);
			}
			else
			{
				delay = 2;
				osd_printf_info("Detected: NMI interrupt BRK = Delay 1 OpCode, delay: %d\n", delay);
			}
		}
		else if (irq_taken)
		{
			if (inst_substate < 9)
			{
				nmi_pending_1 = false;
				delay = 0;
				pulse_input_line(m6502_device::NMI_LINE, attotime::zero);
				osd_printf_info("Detected: NMI interrupt IRQ = Now, delay: %d\n", delay);
			}
			else
			{
				delay = 2;
				osd_printf_info("Detected: NMI interrupt IRQ = Delay 1 OpCode, delay: %d\n", delay);
			}
		}
	}
	
	/*	Run the CPU */
	if (inst_substate) {
		do_exec_partial();
	}

	while (icount > 0)
	{
		if (inst_state < 0xff00) {
			PPC = NPC;
			inst_state = IR | inst_state_base;
			
			if (machine().debug_flags & DEBUG_FLAG_ENABLED)
				debugger_instruction_hook(pc_to_external(NPC));
		}
		do_exec_full();
	}

	handle_dma_rdy_stall();
}

bool m6502_device::is_branch_opcode(u8 op)
{
	switch (op)
	{
		case 0x10: case 0x30: case 0x50: case 0x70:
		case 0x90: case 0xB0: case 0xD0: case 0xF0:
			return true;
		default:
			return false;
	}
}

bool m6502_device::is_irq_oam_dma_window() 
{
	const int delta = total_cycles() - oam_dma_halt_cycle;
	return ((((delta <= 3) && (delta >= 1)) || (delta == 514)) && total_cycles() > 600);
}

bool m6502_device::branch_delay_match_for_new_irq(int state) 
{
	// In this generated MAME 6502 core, a late interrupt can arrive after
	// IR has already advanced/prefetched.  prev_IR is the branch opcode that
	// created the non-page-crossing branch delay window.
	return is_branch_opcode(prev_IR) &&
		(state == ASSERT_LINE) &&
		branched &&
		!paged &&
		(inst_substate > 6);
}

bool m6502_device::branch_need_irq_match() 
{
	// Used while the branch opcode is still active. The late-arrival delay
	// check uses prev_IR because IR may have advanced by that point.
	return is_branch_opcode(IR);
}

bool m6502_device::dma_window_interrupt_eligible() 
{
	return (nmi_pending || ((irq_state || apu_irq_state) && !(P & F_I))) && !inhibit_interrupts;
}


void m6502_device::execute_set_input(int inputnum, int state)
{
	switch (inputnum)
	{
	case IRQ_LINE:
	{
		// Taken non-page-crossing branch ignores IRQ/NMI on its last cycle
		if (branch_delay_match_for_new_irq(state) && !irq_state)
			irq_branch_delay = true;

		// Keep IRQ "wanted" across the branch window
		if (branch_need_irq_match() && (irq_state || state == ASSERT_LINE))
			need_irq = true;

		// If IRQ fell before we consumed the delayed branch case, force it to remain visible
		if (branch_need_irq_match() && need_irq && !state)
			state = ASSERT_LINE;

		irq_state = (state == ASSERT_LINE);

		// IRQ during DMA window
		if (is_irq_oam_dma_window() && dma_window_interrupt_eligible())
			irq_branch_delay = true;
		break;
	}

	case APU_IRQ_LINE:
	{
		// Same branch-delay behavior for APU IRQ
		if (branch_delay_match_for_new_irq(state) && !apu_irq_state)
			apu_irq_branch_delay = true;

		// Keep IRQ "wanted" across the branch window
		if (branch_need_irq_match() && (apu_irq_state || state == ASSERT_LINE))
			need_irq = true;

		// If IRQ fell before we consumed the delayed branch case, force it to remain visible
		if (branch_need_irq_match() && need_irq && !state)
			state = ASSERT_LINE;

		apu_irq_state = (state == ASSERT_LINE);

		// APU IRQ during DMA window
		if (is_irq_oam_dma_window() && dma_window_interrupt_eligible())
			apu_irq_branch_delay = true;

		break;
	}

	case NMI_LINE:
	{
		// Taken non-page-crossing branch ignores NMI on its last cycle too
		if (branch_delay_match_for_new_irq(state) && !nmi_state)
		{
			nmi_branch_delay = true;
			nmi_pending_1 = false;
		}

		// NMI is edge-triggered
		if (!nmi_state && state == ASSERT_LINE)
			nmi_pending = true;

		nmi_state = (state == ASSERT_LINE);

		// NMI during DMA window
		if (is_irq_oam_dma_window() && dma_window_interrupt_eligible())
		{
			nmi_branch_delay = true;
			nmi_pending_1 = false;
		}

		break;
	}

	case V_LINE:
		if (!v_state && state == ASSERT_LINE)
			P |= F_V;
		v_state = (state == ASSERT_LINE);
		break;
	}
}

device_memory_interface::space_config_vector m6502_device::memory_space_config() const
{
	if(has_configured_map(AS_OPCODES))
		return space_config_vector {
			std::make_pair(AS_PROGRAM, &program_config),
			std::make_pair(AS_OPCODES, &sprogram_config)
		};
	else
		return space_config_vector {
			std::make_pair(AS_PROGRAM, &program_config)
		};
}

void m6502_device::state_import(const device_state_entry &entry)
{
	switch(entry.index()) {
	case STATE_GENFLAGS:
	case M6502_P:
		P = P | (F_B|F_E);
		break;
	case M6502_PC:
		PC = NPC;
		irq_taken = false;
		prefetch_start();
		IR = mintf->read_sync(PC);
		prefetch_end();
		PPC = NPC;
		inst_state = IR | inst_state_base;
		break;
	}
}

void m6502_device::state_export(const device_state_entry &entry)
{
	switch(entry.index()) {
	case STATE_GENPC:     XPC = pc_to_external(PPC); break;
	case STATE_GENPCBASE: XPC = pc_to_external(NPC); break;
	}
}

void m6502_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch(entry.index()) {
	case STATE_GENFLAGS:
	case M6502_P:
		str = string_format("%c%c%c%c%c%c",
						P & F_N ? 'N' : '.',
						P & F_V ? 'V' : '.',
						P & F_D ? 'D' : '.',
						P & F_I ? 'I' : '.',
						P & F_Z ? 'Z' : '.',
						P & F_C ? 'C' : '.');
		break;
	}
}

void m6502_device::prefetch_start()
{		
	sync = true;
	sync_w(ASSERT_LINE);
	NPC = PC;
	
	prev_IR = IR;	
	
	if(!nmi_pending_1)
		nmi_overlap_brk_irq = false;
}

void m6502_device::queue_delayed_mapper_irq(int cycles)
{
	// Don't stack duplicate delayed IRQs
	if (!mapper_irq && !irq_state)
	{
		mapper_irq = true;
		mapper_irq_delay = cycles;
		mapper_irq_cpu_cycle = total_cycles() - 1;
	}
}

void m6502_device::cancel_delayed_mapper_irq()
{
	// MMC3 $E000 disables/acknowledges the mapper IRQ line.
	// Cancel the delayed mapper source and any one-opcode branch deferral
	// created from that source.
	mapper_irq = false;
	mapper_irq_delay = 0;
	mapper_irq_cpu_cycle = 0;

	irq_state = false;
	irq_delay = false;
	irq_branch_delay = false;
	need_irq = false;
}

void m6502_device::queue_delayed_nmi(int cycles)
{
	// Don't stack duplicate delayed NMIs
	if (!nmi_pending_1 && !nmi_pending)
	{
		nmi_pending_1 = true;
		delay = cycles;
		nmi_cpu_cycle = total_cycles() - 1;
	}
}

void m6502_device::cancel_delayed_nmi()
{
	// Only cancel before it has arrived in the normal NMI pending path
	if (nmi_pending_1 && !nmi_pending)
	{
		nmi_pending_1 = false;
		delay = 0;
		nmi_cpu_cycle = 0;
	}
}

void m6502_device::prefetch_end()
{
	m_real_brk = (IR == 0x00) && !irq_taken && !nmi_pending;
	sync = false;
	sync_w(CLEAR_LINE);
	
	//logic for branch command IRQ and NMI to fire on next command
	if (irq_delay) {
		execute_set_input(IRQ_LINE, ASSERT_LINE);
		irq_delay = false;
	}
	
	if (apu_irq_delay) {
		execute_set_input(APU_IRQ_LINE, ASSERT_LINE);
		apu_irq_delay = false;
	}
	
	if (nmi_delay) {
		pulse_input_line(m6502_device::NMI_LINE, attotime::zero);
		nmi_delay = false;
	}
	
	//logic for branch command IRQ and NMI to skip this time around but get it next time
	if (irq_branch_delay) {
		irq_state = false;
		irq_delay = true;
		irq_branch_delay = false;
	}
	
	if (apu_irq_branch_delay) {
		apu_irq_state = false;
		apu_irq_delay = true;
		apu_irq_branch_delay = false;
	}	
	
	if(nmi_branch_delay) {
		nmi_pending = false;
		nmi_delay = true;
		nmi_branch_delay = false;
		nmi_pending_1 = false;
	}
	
	// NMI interrupts BRK/IRQ Delay - Fire NMI
	// NMI delayed arrival into the normal CPU interrupt pending state
	if (nmi_pending_1)
	{
		if (delay > 0)
			--delay;

		if (delay == 0)
		{
			nmi_pending_1 = false;
			if(total_cycles() - nmi_cpu_cycle < 3) {
				pulse_input_line(m6502_device::NMI_LINE, attotime::zero);
			} else {
				nmi_pending = true;
			}
		}
	}
	
	if (mapper_irq)
	{
		if (mapper_irq_delay > 0)
			--mapper_irq_delay;

		if (mapper_irq_delay == 0)
		{
			mapper_irq = false;
			irq_state = true;
		}
	}
	
	if((nmi_pending || ((irq_state || apu_irq_state) && !(P & F_I))) && !inhibit_interrupts) {
		irq_taken = true;
		IR = 0x00;
	} else {
		PC++;
	}
	
	branched = false;
	paged = false;
	inst_halted = false;
	need_irq = false;
}

void m6502_device::prefetch_end_noirq()
{
	sync = false;
	sync_w(CLEAR_LINE);
	PC++;
}

uint8_t m6502_device::read(uint16_t adr)
{
	if ((adr == 0xfffa || adr == 0xfffb) && !m_mmc5_reset_scanline_irq.isnull()) {
		m_mmc5_reset_scanline_irq();
	}
	
	adr_bus = adr;
	prevReadAddress = adr;
	set_var_read();

	// --- 4016/4017 special timing ---
	if (adr == 0x4016 || adr == 0x4017)
	{
		int64_t tc = suspended() ? (total_cycles() - 1) : total_cycles();

		if (adr == 0x4016)
		{
			if (tc - prev_4016_read == 1)
			{
				prev_4016_read = tc;
				return last_4016_val;
			}
			prev_4016_read = tc;
		}
		else
		{
			if (tc - prev_4017_read == 1)
			{
				prev_4017_read = tc;
				return last_4017_val;
			}
			prev_4017_read = tc;
		}
	}

	// --- OPEN BUS CHECK ---
	if (is_open_bus_address(adr))
	{
		logerror("OPENBUS DATA: PC=%04X IR=%02X address=%04X value=%02X\n", PC, IR, adr, cpu_external_bus);
		cpu_data_bus = cpu_external_bus;
		return cpu_data_bus;
	}

	// --- 4015 reads use APU side ---
	if (adr == 0x4015) {
		cpu_data_bus = mintf->read(adr);
		return cpu_data_bus;
	}

	// --- normal read ---
	cpu_data_bus = mintf->read(adr);
	cpu_external_bus = cpu_data_bus;
		
	// Cache the real returned values so the double-read returns the SAME byte later
	if (adr == 0x4016)
		last_4016_val = cpu_data_bus;
	else if (adr == 0x4017)
		last_4017_val = cpu_data_bus;
		
	return cpu_data_bus;
}


uint8_t m6502_device::read_9(uint16_t adr)
{
	if ((adr == 0xfffa || adr == 0xfffb) && !m_mmc5_reset_scanline_irq.isnull()) {
		m_mmc5_reset_scanline_irq();
	}
	
	adr_bus = adr;
	prevReadAddress = adr;
	set_var_read();

	if (adr == 0x4016 || adr == 0x4017)
	{
		int64_t tc = suspended() ? (total_cycles() - 1) : total_cycles();

		if (adr == 0x4016)
		{
			if (tc - prev_4016_read == 1)
			{
				prev_4016_read = tc;
				return last_4016_val;
			}
			prev_4016_read = tc;
		}
		else
		{
			if (tc - prev_4017_read == 1)
			{
				prev_4017_read = tc;
				return last_4017_val;
			}
			prev_4017_read = tc;
		}
	}

	if (is_open_bus_address(adr))
	{
		logerror("OPENBUS DATA: PC=%04X IR=%02X address=%04X value=%02X\n", PC, IR, adr, cpu_external_bus);
		cpu_data_bus = cpu_external_bus;
		return cpu_data_bus;
	}

	if (adr == 0x4015) {
		cpu_data_bus = mintf->read_9(adr);
		return cpu_data_bus;
	}

	cpu_data_bus = mintf->read_9(adr);
	cpu_external_bus = cpu_data_bus;
		
	// Cache the real returned values so the double-read returns the SAME byte later
	if (adr == 0x4016)
		last_4016_val = cpu_data_bus;
	else if (adr == 0x4017)
		last_4017_val = cpu_data_bus;
		
	return cpu_data_bus;
}

	
void m6502_device::write(uint16_t adr, uint8_t val) {
	if (adr == 0x2000 && !m_mmc5_ppuctrl_write.isnull()) {
		m_mmc5_ppuctrl_write(val);
	}
	else if (adr == 0x2001 && !m_mmc5_ppumask_write.isnull()) {
		m_mmc5_ppumask_write(val);
	}
	else if (adr == 0x4014 && !m_mmc5_reset_scanline_irq.isnull()) {
		m_mmc5_reset_scanline_irq();
	}
	
	write_cycles_since_dma_halt_request++; 
	adr_bus = adr;
	cpu_is_reading = false;
	
	// Some cartridge protection logic, including BMW8544 / mapper 292,
	// observes the last data byte driven by a CPU write, not just writes to
	// the mapper's own address range.  Keep this CPU-side so cart code can
	// query the true last CPU write without coupling the CPU to a mapper.
	last_cpu_write_latch = val;
	m_previous_cpu_write_cycle = m_last_cpu_write_cycle;
	m_last_cpu_write_cycle = total_cycles();
	if (adr == 0x4016 || adr == 0x4017) {
		bool const suppress = rmw_1 && get_apu_clk1_is_high() && !(val & 1);

		if (adr == 0x4016)
			prev_4016_write = total_cycles();
		else
			prev_4017_write = total_cycles();

		if (suppress) {
			rmw_1 = false;
			cpu_data_bus = val;
			cpu_external_bus = val;
			return;
		}
	}

	rmw_1 = false; 
	mintf->write(adr, val); 
	cpu_data_bus = val;
	cpu_external_bus = val;
}
	
//Read Modify Write
void m6502_device::write_1(uint16_t adr, uint8_t val) { 
	if (adr == 0x2000 && !m_mmc5_ppuctrl_write.isnull()) {
		m_mmc5_ppuctrl_write(val);
	}
	else if (adr == 0x2001 && !m_mmc5_ppumask_write.isnull()) {
		m_mmc5_ppumask_write(val);
	}
	else if (adr == 0x4014 && !m_mmc5_reset_scanline_irq.isnull()) {
		m_mmc5_reset_scanline_irq();
	}
	
	write_cycles_since_dma_halt_request++; 
	adr_bus = adr;
	rmw_1 = true; 
	cpu_is_reading = false; 
	
	// Some cartridge protection logic, including BMW8544 / mapper 292,
	// observes the last data byte driven by a CPU write, not just writes to
	// the mapper's own address range.  Keep this CPU-side so cart code can
	// query the true last CPU write without coupling the CPU to a mapper.
	last_cpu_write_latch = val;
	m_previous_cpu_write_cycle = m_last_cpu_write_cycle;
	m_last_cpu_write_cycle = total_cycles();
	if (adr == 0x4016 || adr == 0x4017) {
		if (adr == 0x4016)
			prev_4016_write = total_cycles();
		else
			prev_4017_write = total_cycles();
	}

	mintf->write(adr, val); 
	cpu_data_bus = val; 
	cpu_external_bus = val;
}
	
void m6502_device::write_9(uint16_t adr, uint8_t val) { 
	if (adr == 0x2000 && !m_mmc5_ppuctrl_write.isnull()) {
		m_mmc5_ppuctrl_write(val);
	}
	else if (adr == 0x2001 && !m_mmc5_ppumask_write.isnull()) {
		m_mmc5_ppumask_write(val);
	}
	else if (adr == 0x4014 && !m_mmc5_reset_scanline_irq.isnull()) {
		m_mmc5_reset_scanline_irq();
	}

	write_cycles_since_dma_halt_request++; 
	rmw_1 = false; 
	adr_bus = adr;
	cpu_is_reading = false; 
	
	// Some cartridge protection logic, including BMW8544 / mapper 292,
	// observes the last data byte driven by a CPU write, not just writes to
	// the mapper's own address range.  Keep this CPU-side so cart code can
	// query the true last CPU write without coupling the CPU to a mapper.
	last_cpu_write_latch = val;
	m_previous_cpu_write_cycle = m_last_cpu_write_cycle;
	m_last_cpu_write_cycle = total_cycles();
	if (adr == 0x4016) {
		if ((total_cycles() - prev_4016_write == 1) && get_apu_clk1_is_high() && !(val & 1)) {
			//osd_printf_info("0x4016 Detected: Controllers should not be strobed when the CPU transitions from a \"put\" cycle to a \"get\" cycle.\n");
			cpu_data_bus = val;
			cpu_external_bus = val;
			return;
		}
		prev_4016_write = total_cycles();
	}
	else if (adr == 0x4017) {
		if ((total_cycles() - prev_4017_write == 1) && get_apu_clk1_is_high() && !(val & 1)) {
			//osd_printf_info("0x4017 Detected: Controllers should not be strobed when the CPU transitions from a \"put\" cycle to a \"get\" cycle.\n");
			cpu_data_bus = val;
			cpu_external_bus = val;
			return;
		}
		prev_4017_write = total_cycles();
	}

	mintf->write_9(adr, val); 
	cpu_data_bus = val; 
	cpu_external_bus = val;
}

uint8_t m6502_device::read_arg(uint16_t adr)
{
	if ((adr == 0xfffa || adr == 0xfffb) && !m_mmc5_reset_scanline_irq.isnull()) {
		m_mmc5_reset_scanline_irq();
	}
	
	adr_bus = adr;
	prevReadAddress = adr;
	set_var_read();

	if (adr == 0x4016 || adr == 0x4017)
	{
		int64_t tc = suspended() ? (total_cycles() - 1) : total_cycles();

		if (adr == 0x4016)
		{
			if (tc - prev_4016_read == 1)
			{
				prev_4016_read = tc;
				return last_4016_val;
			}
			prev_4016_read = tc;
		}
		else
		{
			if (tc - prev_4017_read == 1)
			{
				prev_4017_read = tc;
				return last_4017_val;
			}
			prev_4017_read = tc;
		}
	}

	if (is_open_bus_address(adr))
	{
		logerror("OPENBUS DATA: PC=%04X IR=%02X address=%04X value=%02X\n", PC, IR, adr, cpu_external_bus);
		cpu_data_bus = cpu_external_bus;
		return cpu_data_bus;
	}

	if (adr == 0x4015) {
		cpu_data_bus =  mintf->read_arg(adr);
		return cpu_data_bus;
	}

	cpu_data_bus = mintf->read_arg(adr);
	cpu_external_bus = cpu_data_bus;
		
	// Cache the real returned values so the double-read returns the SAME byte later
	if (adr == 0x4016)
		last_4016_val = cpu_data_bus;
	else if (adr == 0x4017)
		last_4017_val = cpu_data_bus;
	
	return cpu_data_bus;
}

	
uint8_t m6502_device::read_pc()
{
	set_var_read();

	uint16_t adr = PC;
	adr_bus = adr;
	prevReadAddress = adr;
	
	if ((adr == 0xfffa || adr == 0xfffb) && !m_mmc5_reset_scanline_irq.isnull()) {
		m_mmc5_reset_scanline_irq();
	}
		
	if (adr == 0x4016 || adr == 0x4017)
	{
		int64_t tc = suspended() ? (total_cycles() - 1) : total_cycles();

		if (adr == 0x4016)
		{
			if (tc - prev_4016_read == 1)
			{
				prev_4016_read = tc;
				return last_4016_val;
			}
			prev_4016_read = tc;
		}
		else
		{
			if (tc - prev_4017_read == 1)
			{
				prev_4017_read = tc;
				return last_4017_val;
			}
			prev_4017_read = tc;
		}
	}

	if (is_open_bus_address(adr))
	{
		logerror("OPENBUS OPCODE: PC=%04X IR=%02X address=%04X value=%02X\n", PC, IR, adr, cpu_external_bus);
		cpu_data_bus = cpu_external_bus;
		return cpu_data_bus;
	}

	if (adr == 0x4015) {
		cpu_data_bus = mintf->read_arg(adr);
		return cpu_data_bus;
	}

	cpu_data_bus = mintf->read_arg(adr);
	cpu_external_bus = cpu_data_bus;
		
	// Cache the real returned values so the double-read returns the SAME byte later
	if (adr == 0x4016)
		last_4016_val = cpu_data_bus;
	else if (adr == 0x4017)
		last_4017_val = cpu_data_bus;
		
	return cpu_data_bus;
}

	
uint8_t m6502_device::read_sync(uint16_t adr)
{
	if ((adr == 0xfffa || adr == 0xfffb) && !m_mmc5_reset_scanline_irq.isnull()) {
		m_mmc5_reset_scanline_irq();
	}
		
	adr_bus = adr;
	prevReadAddress = adr;
	set_var_read();

	if (adr == 0x4016 || adr == 0x4017)
	{
		int64_t tc = suspended() ? (total_cycles() - 1) : total_cycles();

		if (adr == 0x4016)
		{
			if (tc - prev_4016_read == 1)
			{
				prev_4016_read = tc;
				return last_4016_val;
			}
			prev_4016_read = tc;
		}
		else
		{
			if (tc - prev_4017_read == 1)
			{
				prev_4017_read = tc;
				return last_4017_val;
			}
			prev_4017_read = tc;
		}
	}

	if (is_open_bus_address(adr))
	{
		logerror("OPENBUS OPCODE: PC=%04X IR=%02X address=%04X value=%02X\n", PC, IR, adr, cpu_external_bus);
		cpu_data_bus = cpu_external_bus;
		return cpu_data_bus;
	}

	if (adr == 0x4015) {
		cpu_data_bus = mintf->read_sync(adr);
		return cpu_data_bus;
	}

	cpu_data_bus = mintf->read_sync(adr);
	cpu_external_bus = cpu_data_bus;
		
	// Cache the real returned values so the double-read returns the SAME byte later
	if (adr == 0x4016)
		last_4016_val = cpu_data_bus;
	else if (adr == 0x4017)
		last_4017_val = cpu_data_bus;
		
	return cpu_data_bus;
}
	
void m6502_device::set_var_read() { 
	rmw_1 = false; 
	cpu_is_reading = true; 
}

// Called by the NES DMA scheduler in nes_apu.cpp while the CPU is suspended.
// The generated 6502 core only exposes the bus cycle after do_exec_partial(),
// so we execute one partial step, then roll back inst_substate to retry it later.
void m6502_device::run_suspended_cpu_dma_cycle() { 	
	if(inst_substate) {
		do_exec_partial();
		//if(suspended())
			inst_substate--;
	}
	if(sync) {
		next_IR=IR;
		IR=prev_IR;
	}
}
	
void m6502_device::do_halt() {
	// Fast path: no DMA halt pending.
	if (!dmc_halt && !oam_halt)
		return;
	if(dmc_halt) {
		if(!get_is_pal()) {
			if(dmc_dma_reload) {
				if(!apu_clk1_is_high && write_cycles_since_dma_halt_request == 0 ) {
					suspend(SUSPEND_REASON_HALT,1);
					return;
				}
				if (write_cycles_since_dma_halt_request > 0) {
					suspend(SUSPEND_REASON_HALT,1);
					return;
				}
			} else {
				if (apu_clk1_is_high && write_cycles_since_dma_halt_request == 0) {
					suspend(SUSPEND_REASON_HALT,1);
					return;
				}
				if (write_cycles_since_dma_halt_request > 0) {
					suspend(SUSPEND_REASON_HALT,1);
					return;
				}
			}
		} else {
			if (sync) {
				suspend(SUSPEND_REASON_HALT,1);
				return;
			}			
		} 
	}
	if(oam_halt) {
		suspend(SUSPEND_REASON_HALT,1);
		return;
	}
}

void m6502_device::dmc_halt_next_read(bool a, bool b) {
	dmc_halt = true; 
	dmc_dma_reload = a; 
	write_cycles_since_dma_halt_request = 0; 
	dmc_dma_explicit_stop = b; 
}

uint16_t m6502_device::get_prevReadAddress () { 
	return prevReadAddress; 
}

void m6502_device::mark_oam_dma_halt_cycle() { 
	oam_dma_halt_cycle = total_cycles(); 
}
	
void m6502_device::oam_halt_next_read() { 
	oam_halt = true; 
}

void m6502_device::dmc_clear_halt() { 
	dmc_halt = false; 
	dmc_dma_explicit_stop = false; 
}

void m6502_device::oam_clear_halt() { 
	oam_halt = false; 
}

uint8_t m6502_device::read_4016_4017(uint16_t adr) {
	return read(adr); 
}

uint16_t m6502_device::get_adr_bus() { 
	return adr_bus; 
}

uint8_t m6502_device::get_open_bus() {
	return cpu_external_bus;
}

uint8_t m6502_device::get_data_bus()
{
	return cpu_data_bus;
}

void m6502_device::set_open_bus(uint8_t x) {
	cpu_external_bus = x;
}

void m6502_device::set_data_bus(uint8_t x)
{
	cpu_data_bus = x;
}

void m6502_device::set_m_exram_control(int x) { 
	m_exram_control = x; 
}

//void m6502_device::set_is_mmc5(bool x)
//{
//	is_mmc5 = x;
//}

bool m6502_device::get_apu_clk1_is_high() { 
	return apu_clk1_is_high; 
}

bool m6502_device::get_cpu_is_reading() { 
	return cpu_is_reading; 
}

bool m6502_device::get_rmw_1() { 
	return rmw_1; 
}

void m6502_device::set_open_bus_ranges(const uint32_t *ranges, int count)	{
	m_ob_count = (count > MAX_OPEN_BUS_RANGES) ? MAX_OPEN_BUS_RANGES : count;
	for (int i = 0; i < m_ob_count; i++)
		m_open_bus_ranges[i] = ranges[i];
}

bool m6502_device::is_open_bus_address(uint16_t adr) const
{
	if (m_mmc5_reset_scanline_irq.isnull() && m_ob_count == 0)
		return false;
	
	// MMC5: open bus only in ExRAM modes 0 and 1
	if(!m_mmc5_reset_scanline_irq.isnull()) {
		if (adr >= 0x5C00 && adr <= 0x5FFF) {
			if (m_exram_control == 0 || m_exram_control == 1) {
				logerror("open-bus MMC5 read @ %04x\n", adr);
				return true;
			}
		}
	}
		
	// ignore real APU registers
	if (adr == 0x4015 || adr == 0x4016 || adr == 0x4017)
		return false;

	for (int i = 0; i < m_ob_count; i++) {
		uint32_t r = m_open_bus_ranges[i];
		uint16_t start = r >> 16;
		uint16_t end   = r & 0xFFFF;

		if (adr >= start && adr <= end) {
			//logerror("open-bus read @ %04x\n", adr);
			return true;
		}
	}
	return false;
}

void m6502_device::set_nz(uint8_t v)
{
	P &= ~(F_Z|F_N);
	if(v & 0x80)
		P |= F_N;
	if(!v)
		P |= F_Z;
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
	return program.read_byte(adr);
}

uint8_t m6502_device::mi_default::read_sync(uint16_t adr)
{
	//return csprogram.read_byte(adr);
	return program.read_byte(adr);
}

uint8_t m6502_device::mi_default::read_arg(uint16_t adr)
{
	//return cprogram.read_byte(adr);
	return program.read_byte(adr);
}

void m6502_device::mi_default::write(uint16_t adr, uint8_t val)
{
	program.write_byte(adr, val);
}

uint8_t m6502_device::mi_default14::read(uint16_t adr)
{
	return program14.read_byte(adr);
}

void m6502_device::mi_default14::write(uint16_t adr, uint8_t val)
{
	program14.write_byte(adr, val);
}



#include "cpu/m6502/m6502.hxx"
