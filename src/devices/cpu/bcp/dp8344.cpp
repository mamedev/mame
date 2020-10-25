// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    National Semiconductor DP8344 Biphase Communications Processor

    The BCP employs a true Harvard architecture, with entirely
    separate 64k address spaces for instruction words and data bytes
    (the data bus is address-latched). Wait states can be internally
    configured and/or externally requested. The sophisticated Remote
    Interface and Arbitration System (RIAS) allows for remote access
    to both data and instruction memory.

    Though instruction words are 16 bits wide, the ALU is only 8-bit.
    Since there are no instructions that perform 16-bit data transfers
    (not even in immediate mode), the 16-bit index registers must be
    loaded 8 bits at a time. The RIAS interface is also 8-bit.

    Assuming no wait states are involved, all instructions accessing
    data memory take 3 T-states. All other instructions use 2 T-states
    per instruction word except for JRMK, which takes two additional
    T-states to calculate the jump destination.

    Data can be popped from and pushed to the internal 16-byte LIFO
    data stack by reading and writing the special Data Stack register
    (R31). The entries in the internal 12-word address stack are not
    addressable by the BCP, though its pointer can be manipulated.

    The 44 internal registers are mapped to 32 directly addressable
    locations (some instructions are limited to the first 16). One
    flag switches R0-R3 between Main Bank A (CCR, NCF, ICR, ACR) and
    Alternate Bank A, and another flag switches R4-R11, including
    accumulator R8, between Main Bank B (GP0-GP7) and Alternate Bank
    B (RTR, TSR, TCR, TMR, GP4'-GP7'). These two flags must be set
    together by the EXX function, and while they cannot be read back,
    they are saved on the address stack along with the ALU flags and
    Global Interrupt Enable bit.

**********************************************************************/

#include "emu.h"
#include "cpu/bcp/dp8344.h"
#include "cpu/bcp/bcpdasm.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definitions
DEFINE_DEVICE_TYPE(DP8344A, dp8344a_device, "dp8344a", "National Semiconductor DP8344A BCP")
DEFINE_DEVICE_TYPE(DP8344B, dp8344b_device, "dp8344b", "National Semiconductor DP8344B BCP")


//**************************************************************************
//  DEVICE CONSTRUCTION AND INITIALIZATION
//**************************************************************************

ALLOW_SAVE_TYPE(dp8344_device::inst_state);

//-------------------------------------------------
//  dp8344_device - constructor
//-------------------------------------------------

dp8344_device::dp8344_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_inst_config("instruction", ENDIANNESS_LITTLE, 16, 16, -1)
	, m_data_config("data", ENDIANNESS_LITTLE, 8, 16, 0)
	, m_birq_out_cb(*this)
	, m_data_out_cb(*this)
	, m_data_dly_cb(*this)
	, m_tx_act_cb(*this)
	, m_pc(0)
	, m_icount(0)
	, m_nmi_pending(false)
	, m_wait_states(0)
	, m_source_data(0)
	, m_data_address(0)
	, m_ccr(0)
	, m_ncf(0)
	, m_icr(0)
	, m_acr(0)
	, m_dcr(0)
	, m_ibr(0)
	, m_atr(0)
	, m_fbr(0)
	, m_ecr(0)
	, m_tsr(0)
	, m_tcr(0)
	, m_tmr(0)
	, m_gp_main{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
	, m_gp_alt{0, 0, 0, 0}
	, m_ir{0, 0, 0, 0}
	, m_tr(0)
	, m_tclk(0)
	, m_tcount(0)
	, m_as{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
	, m_ds{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
	, m_asp(0)
	, m_dsp(0)
	, m_ba(false)
	, m_bb(false)
	, m_ric(0)
	, m_hib(false)
	, m_latched_instr(0)
	, m_auto_start(true)
	, m_nmi_state(false)
	, m_rfifo{0, 0, 0}
	, m_tfifo{0, 0, 0}
	, m_rfifo_head(0)
	, m_tfifo_head(0)
	, m_receiver_interrupt(false)
{
}


//-------------------------------------------------
//  dp8344a_device - constructor
//-------------------------------------------------

dp8344a_device::dp8344a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: dp8344_device(mconfig, DP8344A, tag, owner, clock)
{
}


//-------------------------------------------------
//  dp8344b_device - constructor
//-------------------------------------------------

dp8344b_device::dp8344b_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: dp8344_device(mconfig, DP8344B, tag, owner, clock)
{
	// TODO: emulate differences between DP8344A and DP8344B
}


//-------------------------------------------------
//  memory_space_config - return a vector of
//  configuration structures for memory spaces
//-------------------------------------------------

device_memory_interface::space_config_vector dp8344_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_inst_config),
		std::make_pair(AS_DATA, &m_data_config)
	};
}


//-------------------------------------------------
//  device_resolve_objects - resolve objects that
//  may be needed for other devices to set
//  initial conditions at start time
//-------------------------------------------------

void dp8344_device::device_resolve_objects()
{
	// resolve output callbacks
	m_birq_out_cb.resolve_safe();
	m_data_out_cb.resolve_safe();
	m_data_dly_cb.resolve_safe();
	m_tx_act_cb.resolve_safe();
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dp8344_device::device_start()
{
	// get memory spaces
	space(AS_PROGRAM).cache(m_inst_cache);
	space(AS_PROGRAM).specific(m_inst_space);
	space(AS_DATA).specific(m_data_space);

	set_icountptr(m_icount);

	// debug state registration
	state_add(BCP_PC, "PC", m_pc, [this](u16 data) { m_pc = m_ppc = data; prefetch_instruction(); });
	state_add(STATE_GENPC, "GENPC", m_pc).noshow();
	state_add(STATE_GENPCBASE, "GENPCBASE", m_pc).noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_ccr).formatstr("%09s").noshow();
	state_add(BCP_BA, "BA", m_ba);
	state_add(BCP_BB, "BB", m_bb);
	state_add(BCP_CCR, "CCR", m_ccr);
	state_add(BCP_NCF, "NCF", m_ncf);
	state_add(BCP_ICR, "ICR", m_icr);
	state_add(BCP_ACR, "ACR", m_acr);
	state_add(BCP_DCR, "DCR", m_dcr);
	state_add(BCP_IBR, "IBR", m_ibr);
	state_add(BCP_ATR, "ATR", m_atr);
	state_add(BCP_FBR, "FBR", m_fbr);
	for (int i = 0; i < 8; i++)
		state_add(BCP_GP0 + i, string_format("GP%d", i).c_str(), m_gp_main[i]);
	state_add(BCP_ECR, "ECR", m_ecr).mask(0x1f);
	state_add(BCP_TSR, "TSR", m_tsr);
	state_add(BCP_TCR, "TCR", m_tcr);
	state_add(BCP_TMR, "TMR", m_tmr);
	for (int i = 0; i < 4; i++)
		state_add(BCP_GP4_ALT + i, string_format("GP%da", 4 + i).c_str(), m_gp_alt[i]);
	for (int i = 0; i < 4; i++)
	{
		state_add(BCP_IW + i, string_format("I%c", 'W' + i).c_str(), m_ir[i]);
		state_add<u8>(BCP_IWLO + i, string_format("I%cLO", 'W' + i).c_str(),
			[this, i]() -> u8 { return m_ir[i] & 0x00ff; },
			[this, i](u8 data) { m_ir[i] = (m_ir[i] & 0xff00) | data; }).noshow();
		state_add<u8>(BCP_IWHI + i, string_format("I%cHI", 'W' + i).c_str(),
			[this, i]() -> u8 { return (m_ir[i] & 0xff00) >> 8; },
			[this, i](u8 data) { m_ir[i] = (m_ir[i] & 0x00ff) | u16(data) << 8; }).noshow();
	}
	for (int i = 8; i < 16; i++)
		state_add(BCP_GP8 + i - 8, string_format("GP%d", i).c_str(), m_gp_main[i]);
	state_add(BCP_TR, "TR", m_tr);
	state_add(BCP_COUNT, "COUNT", m_tcount);
	state_add(BCP_ASP, "ASP", m_asp).mask(0xf);
	state_add(BCP_DSP, "DSP", m_dsp).mask(0xf);

	// save states
	save_item(NAME(m_pc));
	save_item(NAME(m_ppc));
	save_item(NAME(m_nmi_state));
	save_item(NAME(m_ccr));
	save_item(NAME(m_ncf));
	save_item(NAME(m_icr));
	save_item(NAME(m_acr));
	save_item(NAME(m_dcr));
	save_item(NAME(m_ibr));
	save_item(NAME(m_atr));
	save_item(NAME(m_fbr));
	save_item(NAME(m_ecr));
	save_item(NAME(m_tsr));
	save_item(NAME(m_tcr));
	save_item(NAME(m_tmr));
	save_item(NAME(m_gp_main));
	save_item(NAME(m_gp_alt));
	save_item(NAME(m_ir));
	save_item(NAME(m_tr));
	save_item(NAME(m_tclk));
	save_item(NAME(m_tcount));
	save_item(NAME(m_asp));
	save_item(NAME(m_dsp));
	save_item(NAME(m_as)); // 12-word Address Stack
	save_item(NAME(m_ds)); // 16-byte Data Stack
	save_item(NAME(m_ba));
	save_item(NAME(m_bb));
	save_item(NAME(m_ric));
	save_item(NAME(m_hib));
	save_item(NAME(m_latched_instr));
	save_item(NAME(m_nmi_pending));
	save_item(NAME(m_inst_state));
	save_item(NAME(m_wait_states));
	save_item(NAME(m_source_data));
	save_item(NAME(m_data_address));
	save_item(NAME(m_rfifo)); // 3-frame Receive FIFO
	save_item(NAME(m_tfifo)); // 3-frame Transmit FIFO
	save_item(NAME(m_rfifo_head));
	save_item(NAME(m_tfifo_head));
	save_item(NAME(m_receiver_interrupt));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void dp8344_device::device_reset()
{
	// Reset Program Counter
	m_pc = m_ppc = 0x0000;

	// Reset Condition Code Register
	m_ccr &= 0x10;

	// Reset Interrupt Control Register (all interrupts masked out)
	m_icr = 0xff;

	// Reset Auxiliary Control Register and timer interrupt
	m_acr = 0x00;
	set_timer_interrupt(false);

	// Reset Device Control Register (maximum number of wait states)
	m_dcr = 0xdf;

	// Reset Interrupt Base Register
	m_ibr = 0x00;

	// Reset Transceiver Command Register (3 line quiesce pulses)
	m_tcr = 0x80;

	// Reset Transceiver Mode Register (default to 3270 protocol)
	m_tmr = 0x00;

	// Reset Address and Data Stack Pointers
	m_asp = 0x0;
	m_dsp = 0x0;

	// Select Alternate Bank A and Alternate Bank B
	m_ba = true;
	m_bb = true;

	// Reset Remote Interface Configuration Register
	m_ric = m_auto_start ? 0x03 : 0x01;
	m_hib = false;
	if (!m_auto_start)
		suspend(SUSPEND_REASON_HALT, true);

	// Reset execution state
	m_nmi_pending = false;
	m_inst_state = T1_START;

	transceiver_reset();

	// Undefined on reset: ATR, FBR, TRH, TRL, data in transceiver FIFOs and stacks
	// Also likely undefined on reset: all general-purpose registers
}


//-------------------------------------------------
//  create_disassembler - factory method for
//  disassembling program code
//-------------------------------------------------

std::unique_ptr<util::disasm_interface> dp8344_device::create_disassembler()
{
	return std::make_unique<dp8344_disassembler>();
}


//**************************************************************************
//  INTERRUPTS AND CONTROL REGISTERS
//**************************************************************************

//-------------------------------------------------
//  set_receiver_interrupt - update receiver
//  interrupt request status
//-------------------------------------------------

void dp8344_device::set_receiver_interrupt(bool state)
{
	m_receiver_interrupt = state;
}


//-------------------------------------------------
//  set_transmitter_interrupt - update transmitter
//  interrupt request status
//-------------------------------------------------

void dp8344_device::set_transmitter_interrupt(bool state)
{
	// TODO
}


//-------------------------------------------------
//  set_line_turn_around_interrupt - update line
//  turn around interrupt request status
//-------------------------------------------------

void dp8344_device::set_line_turn_around_interrupt(bool state)
{
	// TODO
}


//-------------------------------------------------
//  set_birq_interrupt - update internal
//  bi-directional interrupt request status
//-------------------------------------------------

void dp8344_device::set_birq_interrupt(bool state)
{
	if (state)
		m_ccr |= 0x10;
	else
		m_ccr &= 0xef;
}


//-------------------------------------------------
//  set_timer_interrupt - update TO flag and timer
//  interrupt request status
//-------------------------------------------------

void dp8344_device::set_timer_interrupt(bool state)
{
	if (state)
		m_ccr |= 0x80;
	else
		m_ccr &= 0x7f;
}


//-------------------------------------------------
//  set_gie - set the global interrupt enable bit
//-------------------------------------------------

void dp8344_device::set_gie(bool state)
{
	if (state)
		m_acr |= 0x01;
	else
		m_acr &= 0xfe;
}


//-------------------------------------------------
//  set_condition_code - write to CCR
//-------------------------------------------------

void dp8344_device::set_condition_code(u8 data)
{
	// ALU flags normally writeable; TO, RR, RW cleared by writing 1s; BIRQ is read-only
	m_ccr = (m_ccr & (~data & 0xe0)) | (m_ccr & 0x10) | (data & 0x0f);

	if (BIT(data, 7))
		set_timer_interrupt(false);
}


//-------------------------------------------------
//  set_interrupt_control - write to ICR
//-------------------------------------------------

void dp8344_device::set_interrupt_control(u8 data)
{
	// IM0: Receiver
	// IM1: Transmitter
	// IM2: Line Turn-Around
	// IM3: BIRQ
	// IM4: Timer

	if ((m_icr & 0xc0) != (data & 0xc0))
	{
		switch (data & 0xc0)
		{
		case 0x00:
			// Interrupt on RFF + RE
			set_receiver_interrupt(BIT(m_ncf, 6) || BIT(m_tsr, 5));
			break;

		case 0x40:
			// Interrupt on DAV + RE
			set_receiver_interrupt((m_tsr & 0x28) != 0);
			break;

		case 0xc0:
			// Interrupt on RA
			set_receiver_interrupt(BIT(m_tsr, 4));
			break;

		default:
			set_receiver_interrupt(false);
			break;
		}
	}

	// Update BIRQ output
	if (BIT(m_acr, 4) && BIT(data ^ m_icr, 3))
		m_birq_out_cb(BIT(data, 3));

	// Bit 5 is reserved
	m_icr = data | 0x20;
}


//-------------------------------------------------
//  set_auxiliary_control - write to ACR
//-------------------------------------------------

void dp8344_device::set_auxiliary_control(u8 data)
{
	// Configure BIRQ for output when BIC is set
	if (BIT(data, 4) && !BIT(m_acr, 4))
	{
		set_birq_interrupt(false);
		m_birq_out_cb(BIT(m_icr, 3));
	}

	// Clearing Timer StarT disables the timer and its interrupt
	if (!BIT(data, 7) && BIT(m_acr, 7))
	{
		logerror("%04X: Timer Stop\n", m_ppc);
		set_timer_interrupt(false);
	}
	else if (BIT(data, 7) && !BIT(m_acr, 7))
	{
		unsigned prescale = (BIT(m_dcr, 7) ? 2 : 1) * (BIT(data, 5) ? 2 : 16);
		logerror("%04X: Timer Start (TO Period = %d Hz)\n",
			m_ppc,
			int(clocks_to_attotime(prescale * (m_tr == 0 ? 65536 : m_tr)).as_hz()));
	}

	if (BIT(data, 6))
	{
		logerror("%04X: Timer Load (TR = %d)\n", m_ppc, m_tr);
		m_tcount = m_tr;

		// TODO: only cleared when load pulse finishes
		data &= 0xbf;
	}

	// Bit 3 is reserved (TODO: DP8344B uses this to extend read cycles)
	m_acr = data & 0xf7;
}


//-------------------------------------------------
//  set_device_control - write to DCR
//-------------------------------------------------

void dp8344_device::set_device_control(u8 data)
{
	if ((data & 0xe0) != (m_dcr & 0xe0))
	{
		if ((data & 0x60) == 0x60)
			logerror("%04X: CPU Clock = OCLK%s (%.4f MHz); Transceiver Clock = X-TCLK\n",
				m_ppc,
				BIT(data, 7) ? "/2" : "",
				clocks_to_attotime(BIT(data, 7) ? 2 : 1).as_mhz());
		else
			logerror("%04X: CPU Clock = OCLK%s (%.4f MHz); Transceiver Clock = OCLK%s (%.4f MHz)\n",
				m_ppc,
				BIT(data, 7) ? "/2" : "",
				clocks_to_attotime(BIT(data, 7) ? 2 : 1).as_mhz(),
				BIT(data, 6) ? "/4" : BIT(data, 5) ? "/2" : "",
				clocks_to_attotime(BIT(data, 6) ? 4 : BIT(data, 5) ? 2 : 1).as_mhz());
	}

	if ((data & 0x1f) != (m_dcr & 0x1f))
		logerror("%04X: Wait States = %d (Instruction Memory), %d (Data Memory)\n",
			m_ppc,
			(data & 0x18) >> 3,
			data & 0x07);

	m_dcr = data;
}


//-------------------------------------------------
//  interrupt_active - determine if any of the
//  five maskable interrupts is active
//-------------------------------------------------

bool dp8344_device::interrupt_active() const
{
	// TODO: non-receiver, non-timer interrupts
	return (!BIT(m_icr, 0) && m_receiver_interrupt) || (!BIT(m_icr, 4) && BIT(m_ccr, 7));
}


//-------------------------------------------------
//  get_interrupt_vector - return the vector for
//  the highest priority maskable interrupt
//-------------------------------------------------

u8 dp8344_device::get_interrupt_vector() const
{
	// TODO: non-receiver, non-timer interrupts
	if (!BIT(m_icr, 0) && m_receiver_interrupt)
		return 0x04;
	else
		return 0x14;
}


//**************************************************************************
//  CONDITION CODES AND ALU HELPERS
//**************************************************************************

//-------------------------------------------------
//  get_flag - retrieve condition from CCR or TSR
//-------------------------------------------------

bool dp8344_device::get_flag(unsigned f) const
{
	switch (f)
	{
	case 0: // Zero/Equal
		return BIT(m_ccr, 0);

	case 1: // Carry
		return BIT(m_ccr, 1);

	case 2: // Overflow
		return BIT(m_ccr, 2);

	case 3: // Negative (or else Positive)
		return BIT(m_ccr, 3);

	case 4: // Receiver Active
		return BIT(m_tsr, 4);

	case 5: // Receiver Error
		return BIT(m_tsr, 5);

	case 6: // Data Available (differs from DAV bit in TSR)
		return m_rfifo_head < 3;

	case 7: // Transmitter FIFO Full (or else Not Full)
		return BIT(m_tsr, 7);

	default:
		logerror("Unknown flag %d\n", f);
		return false;
	}
}


//-------------------------------------------------
//  set_nz - set the N and Z flags based on the
//  result of an ALU operation
//-------------------------------------------------

void dp8344_device::set_nz(u8 result)
{
	m_ccr &= 0xf6;
	if (result == 0)
		m_ccr |= 0x01;
	else if (BIT(result, 7))
		m_ccr |= 0x08;
}


//-------------------------------------------------
//  set_carry - set or clear the C flag
//-------------------------------------------------

void dp8344_device::set_carry(bool state)
{
	if (state)
		m_ccr |= 0x02;
	else
		m_ccr &= 0xfd;
}


//-------------------------------------------------
//  rotate_right - perform right rotation for ROT
//  and JRMK instructions
//-------------------------------------------------

u8 dp8344_device::rotate_right(u8 data, u8 b)
{
	return (data >> b) | (data << (8 - b));
}


//-------------------------------------------------
//  add_nzcv - add and set N, Z, C, V flags for
//  ADCA, ADD and ADDA instructions
//-------------------------------------------------

u8 dp8344_device::add_nzcv(u8 s1, u8 s2, bool carry_in)
{
	s16 result = s8(s1) + s8(s2) + (carry_in ? 1 : 0);
	if (result >= 128 || result < -128)
		m_ccr |= 0x04;
	else
		m_ccr &= 0xfb;

	set_carry(s1 + s2 + (carry_in ? 1 : 0) >= 0x100);
	set_nz(u8(result));
	return u8(result);
}


//-------------------------------------------------
//  sub_nzcv - add and set N, Z, C, V flags for
//  CMP, SBCA, SUB and SUBA instructions
//-------------------------------------------------

u8 dp8344_device::sub_nzcv(u8 s1, u8 s2, bool carry_in)
{
	s16 result = s8(s1) - s8(s2) - (carry_in ? 1 : 0);
	if (result >= 128 || result < -128)
		m_ccr |= 0x04;
	else
		m_ccr &= 0xfb;

	set_carry(s1 < s2 + (carry_in ? 1 : 0));
	set_nz(u8(result));
	return u8(result);
}


void dp8344_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
	case STATE_GENFLAGS:
		str = string_format("%c%c%c%c %c%c%c%c",
			BIT(m_ccr, 7) ? 'T' : '.', // TO
			BIT(m_ccr, 6) ? 'R' : '.', // RR
			BIT(m_ccr, 5) ? 'W' : '.', // RW
			BIT(m_ccr, 4) ? 'B' : '.', // BIRQ
			BIT(m_ccr, 3) ? 'N' : '.',
			BIT(m_ccr, 2) ? 'V' : '.',
			BIT(m_ccr, 1) ? 'C' : '.',
			BIT(m_ccr, 0) ? 'Z' : '.');
		break;
	}
}


//**************************************************************************
//  16-BIT TIMER
//**************************************************************************

//-------------------------------------------------
//  timer_count - count down on timer
//-------------------------------------------------

void dp8344_device::timer_count()
{
	// Count down to zero
	if (--m_tcount == 0)
	{
		set_timer_interrupt(true);

		// Count automatically reloads
		m_tcount = m_tr;
	}
}


//**************************************************************************
//  INTERNAL STACKS
//**************************************************************************

//-------------------------------------------------
//  address_stack_push - push address frame upon
//  performing a call
//-------------------------------------------------

void dp8344_device::address_stack_push()
{
	m_as[m_asp++] = u32(m_pc)
		| (u32(m_ccr & 0x0f) << 16)
		| (m_ba << 21)
		| (m_bb << 20)
		| (u32(m_acr & 0x01) << 22);
	if (m_asp >= 12)
		m_asp = 0;
}


//-------------------------------------------------
//  address_stack_pop - pop address frame upon
//  executing a RET or RETF instruction
//-------------------------------------------------

void dp8344_device::address_stack_pop(u8 g, bool rf)
{
	m_asp = (m_asp == 0) ? 11 : m_asp - 1;
	m_pc = m_as[m_asp] & 0xffff;

	// Optionally restore, set or clear GIE
	if (BIT(g, 1))
		set_gie(!BIT(g, 0));
	else if (BIT(g, 0))
		set_gie(BIT(m_as[m_asp], 22));

	// Optionally restore ALU flags and register banks
	if (rf)
	{
		m_ccr = (m_ccr & 0xf0) | (m_as[m_asp] & 0xf0000) >> 16;
		m_ba = BIT(m_as[m_asp], 21);
		m_bb = BIT(m_as[m_asp], 20);
	}
}


//-------------------------------------------------
//  set_stack_pointer - write to ISP register
//-------------------------------------------------

void dp8344_device::set_stack_pointer(u8 data)
{
	// TODO: one-instruction latency
	m_asp = data >> 4;
	m_dsp = data & 0xf;
}


//-------------------------------------------------
//  data_stack_push - write to DS register
//-------------------------------------------------

void dp8344_device::data_stack_push(u8 data)
{
	m_ds[m_dsp] = data;
	m_dsp = (m_dsp + 1) & 0xf;
}


//-------------------------------------------------
//  data_stack_pop - read from DS register
//-------------------------------------------------

u8 dp8344_device::data_stack_pop()
{
	// TBD: do test instructions (BIT, CMP, JRMK, etc.) suppress side effects as with RTR and ECR?
	m_dsp = (m_dsp - 1) & 0xf;
	return m_ds[m_dsp];
}


//**************************************************************************
//  TRANSCEIVER OPERATION
//**************************************************************************

const char *const dp8344_device::s_protocol_names[8] =
{
	"IBM 3270",
	"IBM 3299 Multiplexer",
	"IBM 3299 Controller",
	"IBM 3299 Repeater",
	"IBM 5250",
	"IBM 5250 Promiscuous",
	"8-bit",
	"8-bit Promiscuous"
};


//-------------------------------------------------
//  transceiver_reset - reset the receiver and
//  transmitter
//-------------------------------------------------

void dp8344_device::transceiver_reset()
{
	// Clear transmitter and receiver FIFOs
	m_rfifo_head = 3;
	m_tfifo_head = 3;

	// Clear all transmitter and receiver status flags except for Transmit FIFO Empty
	m_ncf = 0x80;
	m_tsr &= 0x07;

	// Clear Error Code Register
	m_ecr = 0x00;

	// Clear receiver and line turn around interrupts
	set_receiver_interrupt(false);
	set_line_turn_around_interrupt(false);

	// Set transmitter interrupt (due to TFE)
	set_transmitter_interrupt(true);

	// Reset transmitter outputs
	m_tx_act_cb(0);
	m_data_out_cb(1);
	m_data_dly_cb(!BIT(m_tmr, 3));
}


//-------------------------------------------------
//  transmit_fifo_push - write to RTR
//-------------------------------------------------

void dp8344_device::transmit_fifo_push(u8 data)
{
	// HACK: loopback data
	if (BIT(m_tmr, 6))
	{
		u16 frame = data | u16(m_tcr & 0x07) << 8;
		if ((m_tmr & 0x07) == 0x00)
		{
			// Calculate odd parity on 3270 data frames
			if (!BIT(m_tcr, 2))
			{
				if ((population_count_32(data) & 1) == 0)
					frame |= 0x100;
				else
					frame &= 0x2ff;
			}

			// Calculate word parity
			frame &= 0x3ff;
			if ((population_count_32(frame) & 1) != BIT(m_tcr, 3))
				frame |= 0x400;
		}
		receive_fifo_push(frame);
		return;
	}

	// Push OWP, TF10-8 and RTF7-0 onto transmit FIFO
	m_tfifo[0] = data | u16(m_tcr & 0x0f) << 8;

	if (m_tfifo_head != 0)
	{
		// Asynchronously propagate data through FIFO (actually takes 40 ns)
		m_tfifo_head--;
		for (int i = 0; i < m_tfifo_head; i++)
			m_tfifo[i + 1] = m_tfifo[i];

		// Set flag when FIFO is full
		if (m_tfifo_head == 0)
			m_tsr |= 0x80;
	}

	// Clear Transmit FIFO Empty and Line Turn Around bits in NCF and associated interrupts
	m_ncf &= 0x6f;
	set_transmitter_interrupt(false);
	set_line_turn_around_interrupt(false);

	// Make sure to disable receiver before transmitting in half-duplex mode
	if (BIT(m_tsr, 4) && !BIT(m_tmr, 5))
	{
		// Receiver Disabled While Active error
		set_receiver_error(0x01);
	}
}


//-------------------------------------------------
//  transmit_fifo_pop -
//-------------------------------------------------

u16 dp8344_device::transmit_fifo_pop()
{
	assert(m_tfifo_head < 3);
	u16 data = m_tfifo[m_tfifo_head++];

	if (m_tfifo_head == 3)
	{
		// Set Transmit FIFO Empty bit
		m_ncf |= 0x80;
		set_transmitter_interrupt(true);
	}

	// Clear FIFO full flag
	m_tsr &= 0x7f;

	return data;
}


//-------------------------------------------------
//  transmitter_idle -
//-------------------------------------------------

void dp8344_device::transmitter_idle()
{
	// Deassert TX-ACT
	m_tx_act_cb(0);

	// Clear Transmitter Active flag
	m_tsr &= 0xbf;
}


//-------------------------------------------------
//  receiver_active - update status and interrupt
//  flags after receiving a valid sync sequence
//-------------------------------------------------

void dp8344_device::receiver_active()
{
	m_tsr |= 0x10;
	if ((m_icr & 0xc0) == 0xc0)
		set_receiver_interrupt(true);
}


//-------------------------------------------------
//  receive_fifo_push - push a data frame onto
//  the receive FIFO
//-------------------------------------------------

void dp8344_device::receive_fifo_push(u16 data)
{
	// Push word onto receive FIFO
	m_rfifo[0] = data;

	if (m_rfifo_head != 0)
	{
		// Asynchronously propagate data through FIFO (actually takes 40 ns)
		m_rfifo_head--;
		for (int i = 0; i < m_rfifo_head; i++)
			m_rfifo[i + 1] = m_rfifo[i];

		// Set data available flag
		m_tsr |= 0x08;
		if ((m_icr & 0xc0) == 0x40)
			set_receiver_interrupt(true);

		// Copy upper bits to TSR
		m_tsr = (m_tsr & 0xf8) | (data & 0x700) >> 8;

		// Set flag when FIFO is full
		if (m_rfifo_head == 0)
		{
			m_tsr |= 0x40;
			if ((m_icr & 0xc0) == 0x00)
				set_receiver_interrupt(true);
		}
	}
	else
	{
		// Overflow error if the receive FIFO is already full
		set_receiver_error(0x10);
	}
}


//-------------------------------------------------
//  receive_fifo_pop - read from RTR
//-------------------------------------------------

u8 dp8344_device::receive_fifo_pop(bool test)
{
	u8 data = m_rfifo[m_rfifo_head < 3 ? m_rfifo_head : 2] & 0x0ff;

	if (!test)
	{
		if (m_rfifo_head < 3)
		{
			// Advance the FIFO
			m_rfifo_head++;

			// Copy upper bits to TSR
			if (m_rfifo_head < 3)
				m_tsr = (m_tsr & 0xf8) | (m_rfifo[m_rfifo_head] & 0x700) >> 8;
		}

		// Clear Data Available flag in TSR
		m_tsr &= 0xf7;

		// Clear Receive FIFO Full, DEME, Auto-Response and Poll/Acknowledge bits in NCF
		m_ncf &= 0xb0;

		// Clear receiver interrupt unless error interrupt exists
		if (BIT(m_icr, 7) || !BIT(m_tsr, 5))
			set_receiver_interrupt(false);
	}

	return data;
}


//-------------------------------------------------
//  set_receiver_error - raise error code and
//  disable receiver
//-------------------------------------------------

void dp8344_device::set_receiver_error(u8 code)
{
	assert(code != 0 && (code & 0xe0) == 0);

	// Set one of five flags in ECR
	m_ecr |= code;

	// Clear Data Available and Receiver Active flags in TSR
	m_tsr &= 0xe7;

	// Set Receiver Error flag in TSR
	m_tsr |= 0x20;

	// Interrupt on RE if enabled; no interrupt on RA
	set_receiver_interrupt(!BIT(m_icr, 7));
}


//-------------------------------------------------
//  get_error_code - read from ECR
//-------------------------------------------------

u8 dp8344_device::get_error_code(bool test)
{
	u8 code = m_ecr;

	// Upper bits come from receive FIFO
	code |= m_rfifo[m_rfifo_head < 3 ? m_rfifo_head : 2] & 0x0e0;

	if (!test)
	{
		// Clear Error Code Register and Receiver Error flag in NCF
		m_ecr = 0x00;
		m_ncf &= 0xdf;

		// Reassert DAV if FIFO is not empty
		if (m_rfifo_head < 3)
			m_tsr |= 0x08;

		// Update interrupt status if RE interrupt selected
		switch (m_icr & 0xc0)
		{
		case 0x00:
			// Interrupt on RFF
			set_receiver_interrupt(BIT(m_ncf, 6));
			break;

		case 0x40:
			// Interrupt on DAV
			set_receiver_interrupt(BIT(m_tsr, 3));
			break;

		default:
			break;
		}
	}

	return code;
}


//-------------------------------------------------
//  set_transceiver_command - write to TCR
//-------------------------------------------------

void dp8344_device::set_transceiver_command(u8 data)
{
	if ((data & 0xb0) != (m_tcr & 0xb0))
		logerror("%04X: Receiver Selected as %s with %d Quiesces; TX-ACT %sAdvanced\n",
			m_ppc,
			BIT(data, 5) ? "ALG" : "DATA-IN",
			BIT(data, 7) ? 3 : 2,
			BIT(data, 4) ? "" : "Not ");

	m_tcr = data;
}


//-------------------------------------------------
//  set_transceiver_mode - write to TMR
//-------------------------------------------------

void dp8344_device::set_transceiver_mode(u8 data)
{
	// Programmed transceiver reset
	if (!BIT(m_tmr, 7) && BIT(data, 7))
		transceiver_reset();

	// TX-ACT is disabled when internal loopback mode is selected
	if (!BIT(m_tmr, 6) && BIT(data, 6))
		m_tx_act_cb(0);

	if ((m_tmr & 0x07) != (data & 0x07))
		logerror("%04X: Protocol Select: %s\n", m_ppc, s_protocol_names[data & 0x07]);

	m_tmr = data;
}


//-------------------------------------------------
//  clear_network_command_flag - write to NCF
//-------------------------------------------------

void dp8344_device::clear_network_command_flag(u8 data)
{
	// Clear Line Turn Around flag by writing a 1
	if (BIT(data, 4))
	{
		m_ncf &= 0xef;
		set_line_turn_around_interrupt(false);
	}
}


//**************************************************************************
//  REGISTER ARRAY
//**************************************************************************

//-------------------------------------------------
//  read_register - read from source register
//-------------------------------------------------

u8 dp8344_device::read_register(unsigned reg, bool test)
{
	switch (reg)
	{
	case 0: // Condition Code Register (main) or Device Control Register (alternate)
		if (m_ba)
			return m_dcr;
		else
			return m_ccr;

	case 1: // Network Command Flag Register (main) or Interrupt Base Register (alternate)
		if (m_ba)
			return m_ibr;
		else
			return m_ncf;

	case 2: // Interrupt Control Register (main) or Auxiliary Transceiver Register (alternate)
		if (m_ba)
			return m_atr;
		else
			return m_icr;

	case 3: // Auxiliary Control Register (main) or Fill-Bit Register (alternate)
		if (m_ba)
			return m_fbr;
		else
			return m_acr;

	case 4: // GP0 (main) or Receive/Transmit Register and/or Error Code Register (alternate)
		if (m_bb)
			return BIT(m_tcr, 6) ? get_error_code(test) : receive_fifo_pop(test);
		else
			return m_gp_main[0];

	case 5: // GP1 (main) or Transceiver Status Register (alternate)
		if (m_bb)
			return m_tsr;
		else
			return m_gp_main[1];

	case 6: // GP2 (main) or Transceiver Command Register (alternate)
		if (m_bb)
			return m_tcr;
		else
			return m_gp_main[2];

	case 7: // GP3 (main) or Transceiver Mode Register (alternate)
		if (m_bb)
			return m_tmr;
		else
			return m_gp_main[3];

	case 8: // GP4/accumulator (main) or GP4' (alternate)
		if (m_bb)
			return m_gp_alt[0];
		else
			return m_gp_main[4];

	case 9: // GP5 (main) or GP5' (alternate)
		if (m_bb)
			return m_gp_alt[1];
		else
			return m_gp_main[5];

	case 10: // GP6 (main) or GP6' (alternate)
		if (m_bb)
			return m_gp_alt[2];
		else
			return m_gp_main[6];

	case 11: // GP7 (main) or GP7' (alternate)
		if (m_bb)
			return m_gp_alt[3];
		else
			return m_gp_main[7];

	case 12: // IW (low byte)
		return m_ir[0] & 0x00ff;

	case 13: // IW (high byte)
		return (m_ir[0] & 0xff00) >> 8;

	case 14: // IX (low byte)
		return m_ir[1] & 0x00ff;

	case 15: // IX (high byte)
		return (m_ir[1] & 0xff00) >> 8;

	case 16: // IY (low byte)
		return m_ir[2] & 0x00ff;

	case 17: // IY (high byte)
		return (m_ir[2] & 0xff00) >> 8;

	case 18: // IZ (low byte)
		return m_ir[3] & 0x00ff;

	case 19: // IZ (high byte)
		return (m_ir[3] & 0xff00) >> 8;

	case 20: // GP8
		return m_gp_main[8];

	case 21: // GP9
		return m_gp_main[9];

	case 22: // GP10
		return m_gp_main[10];

	case 23: // GP11
		return m_gp_main[11];

	case 24: // GP12
		return m_gp_main[12];

	case 25: // GP13
		return m_gp_main[13];

	case 26: // GP14
		return m_gp_main[14];

	case 27: // GP15
		return m_gp_main[15];

	case 28: // TRL count status
		return m_tcount & 0x00ff;

	case 29: // TRH count status
		return (m_tcount & 0xff00) >> 8;

	case 30: // Internal Stack Pointer
		return (m_asp << 4) | m_dsp;

	case 31: // Data Stack
		return data_stack_pop();

	default: // should never happen
		logerror("Invalid source register R%d\n", reg);
		return 0;
	}
}


//-------------------------------------------------
//  read_accumulator - source ALU data from
//  accumulator register (GP4)
//-------------------------------------------------

u8 dp8344_device::read_accumulator() const
{
	return m_bb ? m_gp_alt[0] : m_gp_main[4];
}


//-------------------------------------------------
//  write_register - write to destination register
//-------------------------------------------------

void dp8344_device::write_register(unsigned reg, u8 data)
{
	switch (reg)
	{
	case 0: // Condition Code Register (main) or Device Control Register (alternate)
		if (m_ba)
			set_device_control(data);
		else
			set_condition_code(data);
		break;

	case 1: // Network Command Flag Register (main) or Interrupt Base Register (alternate)
		if (m_ba)
			m_ibr = data;
		else
			clear_network_command_flag(data);
		break;

	case 2: // Interrupt Control Register (main) or Auxiliary Transceiver Register (alternate)
		if (m_ba)
			m_atr = data;
		else
			set_interrupt_control(data);
		break;

	case 3: // Auxiliary Control Register (main) or Fill-Bit Register (alternate)
		if (m_ba)
			m_fbr = data;
		else
			set_auxiliary_control(data);
		break;

	case 4: // GP0 (main) or Receive/Transmit Register (alternate)
		if (m_bb)
			transmit_fifo_push(data);
		else
			m_gp_main[0] = data;
		break;

	case 5: // GP1 (main) or Transceiver Status Register (alternate)
		if (m_bb)
			logerror("Transceiver Status Register is read-only\n");
		else
			m_gp_main[1] = data;
		break;

	case 6: // GP2 (main) or Transceiver Command Register (alternate)
		if (m_bb)
			set_transceiver_command(data);
		else
			m_gp_main[2] = data;
		break;

	case 7: // GP3 (main) or Transceiver Mode Register (alternate)
		if (m_bb)
			set_transceiver_mode(data);
		else
			m_gp_main[3] = data;
		break;

	case 8: // GP4/accumulator (main) or GP4' (alternate)
		if (m_bb)
			m_gp_alt[0] = data;
		else
			m_gp_main[4] = data;
		break;

	case 9: // GP5 (main) or GP5' (alternate)
		if (m_bb)
			m_gp_alt[1] = data;
		else
			m_gp_main[5] = data;
		break;

	case 10: // GP6 (main) or GP6' (alternate)
		if (m_bb)
			m_gp_alt[2] = data;
		else
			m_gp_main[6] = data;
		break;

	case 11: // GP7 (main) or GP7' (alternate)
		if (m_bb)
			m_gp_alt[3] = data;
		else
			m_gp_main[7] = data;
		break;

	case 12: // IW (low byte)
		m_ir[0] = (m_ir[0] & 0xff00) | data;
		break;

	case 13: // IW (high byte)
		m_ir[0] = (m_ir[0] & 0x00ff) | u16(data) << 8;
		break;

	case 14: // IX (low byte)
		m_ir[1] = (m_ir[1] & 0xff00) | data;
		break;

	case 15: // IX (high byte)
		m_ir[1] = (m_ir[1] & 0x00ff) | u16(data) << 8;
		break;

	case 16: // IY (low byte)
		m_ir[2] = (m_ir[2] & 0xff00) | data;
		break;

	case 17: // IY (high byte)
		m_ir[2] = (m_ir[2] & 0x00ff) | u16(data) << 8;
		break;

	case 18: // IZ (low byte)
		m_ir[3] = (m_ir[3] & 0xff00) | data;
		break;

	case 19: // IZ (high byte)
		m_ir[3] = (m_ir[3] & 0x00ff) | u16(data) << 8;
		break;

	case 20: // GP8
		m_gp_main[8] = data;
		break;

	case 21: // GP9
		m_gp_main[9] = data;
		break;

	case 22: // GP10
		m_gp_main[10] = data;
		break;

	case 23: // GP11
		m_gp_main[11] = data;
		break;

	case 24: // GP12
		m_gp_main[12] = data;
		break;

	case 25: // GP13
		m_gp_main[13] = data;
		break;

	case 26: // GP14
		m_gp_main[14] = data;
		break;

	case 27: // GP15
		m_gp_main[15] = data;
		break;

	case 28: // TRL holding register
		m_tr = (m_tr & 0xff00) | data;
		break;

	case 29: // TRH holding register
		m_tr = (m_tr & 0x00ff) | u16(data) << 8;
		break;

	case 30: // Internal Stack Pointer
		set_stack_pointer(data);
		break;

	case 31: // Data Stack
		data_stack_push(data);
		break;

	default: // should never happen
		logerror("Invalid destination register R%d = %02X\n", reg, data);
		break;
	}
}


//**************************************************************************
//  INPUT LINE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  execute_set_input -
//-------------------------------------------------

void dp8344_device::execute_set_input(int irqline, int state)
{
	switch (irqline)
	{
	case BIRQ_LINE:
		if (!BIT(m_acr, 4))
			set_birq_interrupt(state == ASSERT_LINE);
		break;

	case NMI_LINE:
		if ((state == ASSERT_LINE) && !m_nmi_state)
			m_nmi_pending = true;
		m_nmi_state = state == ASSERT_LINE;
		break;

	case DATA_IN_LINE:
		break;

	case X_TCLK_LINE:
		break;
	}
}


//**************************************************************************
//  PROGRAM EXECUTION
//**************************************************************************

//-------------------------------------------------
//  prefetch_instruction - fetch the next
//  instruction word from program memory
//-------------------------------------------------

void dp8344_device::prefetch_instruction()
{
	m_latched_instr = m_inst_cache.read_word(m_pc);
}


//-------------------------------------------------
//  latch_address - handle ALE cycle and set T2
//  wait states for instructions referencing data
//  memory
//-------------------------------------------------

void dp8344_device::latch_address(bool rw)
{
	//logerror("Latching data memory address %04Xh for %sing (PC = %04Xh)\n", m_data_address, rw ? "read" : "writ", m_pc);
	if ((m_dcr & 0x18) == 0)
		m_wait_states = m_dcr & 0x07;
	else
		m_wait_states = std::max(m_dcr & 0x07, ((m_dcr & 0x18) >> 3) - 1);
}


//-------------------------------------------------
//  instruction_wait - set T2 wait states for
//  non-memory instructions
//-------------------------------------------------

void dp8344_device::instruction_wait()
{
	m_wait_states = (m_dcr & 0x18) >> 3;
}


//-------------------------------------------------
//  decode_instruction - begin execution of an
//  opcode
//-------------------------------------------------

dp8344_device::inst_state dp8344_device::decode_instruction()
{
	if (m_latched_instr < 0x8000)
	{
		m_source_data = read_register(m_latched_instr & 0x000f, (m_latched_instr & 0x7000) == 0x3000);
		if ((m_latched_instr & 0xf000) == 0x1000)
		{
			// MOVE to indexed data memory
			m_data_address = m_ir[3] + ((m_latched_instr & 0x0ff0) >> 4);
			return TX_WRITE;
		}
		else
		{
			instruction_wait();
			return T2_STORE;
		}
	}
	else if ((m_latched_instr & 0xf800) == 0x8000)
	{
		// JRMK
		m_source_data = read_register(m_latched_instr & 0x001f, true);
		return TX1_JRMK;
	}
	else if ((m_latched_instr & 0xfc00) == 0x8800)
	{
		// MOVE immediate to data memory
		m_source_data = (m_latched_instr & 0x0380) >> 2 | (m_latched_instr & 0x001f);
		m_data_address = m_ir[(m_latched_instr & 0x0060) >> 5];
		return TX_WRITE;
	}
	else if ((m_latched_instr & 0xfc00) == 0x8c00)
	{
		// LJMP or LCALL, conditional
		m_source_data = read_register(m_latched_instr & 0x001f, true);
		instruction_wait();
		return T2_ABSOLUTE;
	}
	else if ((m_latched_instr & 0xf000) == 0x9000)
	{
		// MOVE from indexed data memory
		m_data_address = m_ir[3] + ((m_latched_instr & 0x0ff0) >> 4);
		return TX_READ;
	}
	else if ((m_latched_instr >= 0xa000 && m_latched_instr < 0xae00) || (m_latched_instr & 0xfc00) == 0xc000)
	{
		if ((m_latched_instr & 0xfe00) != 0xc000)
			m_source_data = read_register(m_latched_instr & 0x001f, false);
		switch (m_latched_instr & 0x0180)
		{
		case 0x0000:
			m_data_address = m_ir[(m_latched_instr & 0x0060) >> 5]--;
			break;
		case 0x0080:
			m_data_address = m_ir[(m_latched_instr & 0x0060) >> 5];
			break;
		case 0x0100:
			m_data_address = m_ir[(m_latched_instr & 0x0060) >> 5]++;
			break;
		case 0x0180:
			m_data_address = ++m_ir[(m_latched_instr & 0x0060) >> 5];
			break;
		}
		return (m_latched_instr & 0xfe00) == 0xc000 ? TX_READ : TX_WRITE;
	}
	else if ((m_latched_instr & 0xff80) == 0xae80)
	{
		// EXX
		m_ba = BIT(m_latched_instr, 4);
		m_bb = BIT(m_latched_instr, 3);
		if (BIT(m_latched_instr, 6))
			set_gie(!BIT(m_latched_instr, 5));
		instruction_wait();
		return T2_NEXT;
	}
	else if ((m_latched_instr & 0xff00) == 0xaf00)
	{
		// RET or RETF
		if (BIT(m_latched_instr, 7))
		{
			address_stack_pop((m_latched_instr & 0x0060) >> 5, BIT(m_latched_instr, 4));
			instruction_wait();
			return T2_NEXT;
		}
		else if (get_flag(m_latched_instr & 0x0007) == BIT(m_latched_instr, 3))
			return TX_RETF;
		else
		{
			instruction_wait();
			return T2_NEXT;
		}
	}
	else if ((m_latched_instr & 0xf000) == 0xb000)
	{
		// MOVE immediate to register
		instruction_wait();
		return T2_STORE;
	}
	else if ((m_latched_instr & 0xfc00) == 0xc400)
	{
		// MOVE to or from accumulator-indexed data memory
		if (BIT(m_latched_instr, 7))
			m_source_data = read_register(m_latched_instr & 0x001f, false);
		m_data_address = m_ir[(m_latched_instr & 0x0060) >> 5] + read_accumulator();
		return BIT(m_latched_instr, 7) ? TX_WRITE : TX_READ;
	}
	else if ((m_latched_instr & 0xff00) == 0xcb00)
	{
		// JMP to relative destination
		m_source_data = m_latched_instr & 0x00ff;
		return TX2_JMP;
	}
	else if ((m_latched_instr & 0xff00) == 0xcc00)
	{
		// CALL to relative destination
		m_source_data = m_latched_instr & 0x00ff;
		return TX_CALL;
	}
	else if ((m_latched_instr & 0xff80) == 0xcd00)
	{
		// LJMP to index register
		m_pc = m_ir[(m_latched_instr & 0x0060) >> 5];
		instruction_wait();
		return T2_NEXT;
	}
	else if ((m_latched_instr & 0xff80) == 0xcd80)
	{
		// JMP, register-based
		m_source_data = read_register(m_latched_instr & 0x001f, true);
		return TX1_JMP;
	}
	else if ((m_latched_instr & 0xff00) == 0xce00)
	{
		// LJMP or LCALL, unconditional
		instruction_wait();
		return T2_ABSOLUTE;
	}
	else if ((m_latched_instr & 0xff80) == 0xcf80)
	{
		// TRAP or interrupt
		address_stack_push();
		m_pc = u16(m_ibr) << 8 | (m_latched_instr & 0x003f);
		if (BIT(m_latched_instr, 6))
			set_gie(false);
		instruction_wait();
		return T2_NEXT;
	}
	else if ((m_latched_instr & 0xf000) == 0xd000)
	{
		// JMP, conditional
		m_source_data = m_latched_instr & 0x00ff;
		if (get_flag((m_latched_instr & 0x0700) >> 8) == BIT(m_latched_instr, 11))
			return TX2_JMP;
		else
		{
			instruction_wait();
			return T2_NEXT;
		}
	}
	else
	{
		m_source_data = read_register(m_latched_instr & 0x001f, false);
		instruction_wait();
		return T2_STORE;
	}
}


//-------------------------------------------------
//  store_result - calculate and store the result
//  of a register operation
//-------------------------------------------------

void dp8344_device::store_result()
{
	switch (m_latched_instr & 0xf000)
	{
	case 0x0000:
		m_source_data = add_nzcv(m_source_data, (m_latched_instr & 0x0ff0) >> 4, false);
		write_register(m_latched_instr & 0x000f, m_source_data);
		break;

	case 0x2000:
		m_source_data = sub_nzcv(m_source_data, (m_latched_instr & 0x0ff0) >> 4, false);
		write_register(m_latched_instr & 0x000f, m_source_data);
		break;

	case 0x3000:
		(void)sub_nzcv(m_source_data, (m_latched_instr & 0x0ff0) >> 4, false);
		break;

	case 0x4000:
		m_source_data &= (m_latched_instr & 0x0ff0) >> 4;
		set_nz(m_source_data);
		write_register(m_latched_instr & 0x000f, m_source_data);
		break;

	case 0x5000:
		m_source_data |= (m_latched_instr & 0x0ff0) >> 4;
		set_nz(m_source_data);
		write_register(m_latched_instr & 0x000f, m_source_data);
		break;

	case 0x6000:
		m_source_data ^= (m_latched_instr & 0x0ff0) >> 4;
		set_nz(m_source_data);
		write_register(m_latched_instr & 0x000f, m_source_data);
		break;

	case 0x7000:
		m_source_data &= (m_latched_instr & 0x0ff0) >> 4;
		set_nz(m_source_data);
		break;

	case 0xa000:
		assert((m_latched_instr & 0x0f80) == 0x0e00);
		m_source_data ^= 0xff;
		set_nz(m_source_data);
		write_register(m_latched_instr & 0x001f, m_source_data);
		break;

	case 0xb000:
		write_register(m_latched_instr & 0x000f, (m_latched_instr & 0x0ff0) >> 4);
		break;

	case 0xc000:
		switch (m_latched_instr & 0x0f00)
		{
		case 0x800:
			set_carry(BIT(u16(m_source_data) << 1, (m_latched_instr & 0x00e0) >> 5));
			m_source_data >>= (m_latched_instr & 0x00e0) >> 5;
			set_nz(m_source_data);
			write_register(m_latched_instr & 0x001f, m_source_data);
			break;

		case 0x900:
			set_carry(BIT(m_source_data, (m_latched_instr & 0x00e0) >> 5));
			if ((m_latched_instr & 0x00e0) != 0)
				m_source_data <<= 8 - ((m_latched_instr & 0x00e0) >> 5);
			set_nz(m_source_data);
			write_register(m_latched_instr & 0x001f, m_source_data);
			break;

		case 0xa00:
			set_carry(BIT(u16(m_source_data) << 1, (m_latched_instr & 0x00e0) >> 5));
			m_source_data = rotate_right(m_source_data, (m_latched_instr & 0x00e0) >> 5);
			set_nz(m_source_data);
			write_register(m_latched_instr & 0x001f, m_source_data);
			break;
		}

	case 0xe000:
		if (BIT(m_latched_instr, 11))
			m_source_data = sub_nzcv(m_source_data, read_accumulator(), BIT(m_latched_instr, 10) && BIT(m_ccr, 1));
		else
			m_source_data = add_nzcv(m_source_data, read_accumulator(), BIT(m_latched_instr, 10) && BIT(m_ccr, 1));
		write_register((m_latched_instr & 0x03e0) >> 5, m_source_data);
		break;

	case 0xf000:
		switch (m_latched_instr & 0x0c00)
		{
		case 0x000:
			m_source_data &= read_accumulator();
			set_nz(m_source_data);
			break;

		case 0x400:
			m_source_data |= read_accumulator();
			set_nz(m_source_data);
			break;

		case 0x800:
			m_source_data ^= read_accumulator();
			set_nz(m_source_data);
			break;

		case 0xc00:
			break;
		}
		write_register((m_latched_instr & 0x03e0) >> 5, m_source_data);
		break;
	}
}


//-------------------------------------------------
//  data_write - write one byte to data memory to
//  finish instruction
//-------------------------------------------------

void dp8344_device::data_write()
{
	switch (m_latched_instr & 0xfe00)
	{
	case 0xa000: case 0xa200:
		m_source_data = add_nzcv(m_source_data, read_accumulator(), BIT(m_latched_instr, 9) && BIT(m_ccr, 1));
		break;

	case 0xa400: case 0xa600:
		m_source_data = sub_nzcv(m_source_data, read_accumulator(), BIT(m_latched_instr, 9) && BIT(m_ccr, 1));
		break;

	case 0xa800:
		m_source_data &= read_accumulator();
		set_nz(m_source_data);
		break;

	case 0xaa00:
		m_source_data |= read_accumulator();
		set_nz(m_source_data);
		break;

	case 0xac00:
		m_source_data ^= read_accumulator();
		set_nz(m_source_data);
		break;
	}

	m_data_space.write_byte(m_data_address, m_source_data);
}


//-------------------------------------------------
//  data_read - read one byte from data memory
//  into register to finish instruction
//-------------------------------------------------

void dp8344_device::data_read()
{
	write_register(m_latched_instr & ((m_latched_instr & 0xf000) == 0x9000 ? 0x000f : 0x001f), m_data_space.read_byte(m_data_address));
}


//-------------------------------------------------
//  execute_run -
//-------------------------------------------------

void dp8344_device::execute_run()
{
	while (m_icount > 0)
	{
		switch (m_inst_state)
		{
		case T1_DECODE:
			m_ppc = m_pc;
			debugger_instruction_hook(m_pc);

			if (m_nmi_pending)
			{
				m_nmi_pending = false;

				// TRAP to interrupt vector
				m_latched_instr = 0xcfdc;
			}
			else if (BIT(m_acr, 0) && interrupt_active())
			{
				// TRAP to interrupt vector
				m_latched_instr = 0xcfc0 | get_interrupt_vector();
			}
			else
				m_pc++;

			m_inst_state = decode_instruction();
			break;

		case T1_START:
			instruction_wait();
			m_inst_state = T2_NEXT;
			break;

		case T1_SKIP:
			m_pc++;
			instruction_wait();
			m_inst_state = T2_NEXT;
			break;

		case T1_LJMP:
			m_pc = m_latched_instr;
			instruction_wait();
			m_inst_state = T2_NEXT;
			break;

		case T1_LCALL:
			m_pc++;
			address_stack_push();
			m_pc = m_latched_instr;
			instruction_wait();
			m_inst_state = T2_NEXT;
			break;

		case TX_READ:
			latch_address(true);
			m_inst_state = T2_READ;
			break;

		case TX_WRITE:
			latch_address(false);
			m_inst_state = T2_WRITE;
			break;

		case TX1_JRMK:
			m_source_data = rotate_right(m_source_data, (m_latched_instr & 0x00e0) >> 5);
			m_source_data &= (0xff >> ((m_latched_instr & 0x0700) >> 8)) & 0xfe;
			m_inst_state = TX2_JMP;
			break;

		case TX1_JMP:
			m_inst_state = TX2_JMP;
			break;

		case TX2_JMP:
			m_pc += s8(m_source_data);
			instruction_wait();
			m_inst_state = T2_NEXT;
			break;

		case TX_CALL:
			address_stack_push();
			m_pc += s8(m_source_data);
			instruction_wait();
			m_inst_state = T2_NEXT;
			break;

		case TX_RETF:
			address_stack_pop((m_latched_instr & 0x0060) >> 5, BIT(m_latched_instr, 4));
			instruction_wait();
			m_inst_state = T2_NEXT;
			break;

		case T2_NEXT:
			if (m_wait_states != 0)
				m_wait_states--;
			else
			{
				prefetch_instruction();
				m_inst_state = T1_DECODE;
			}
			break;

		case T2_STORE:
			if (m_wait_states != 0)
				m_wait_states--;
			else
			{
				store_result();
				prefetch_instruction();
				m_inst_state = T1_DECODE;
			}
			break;

		case T2_READ:
			if (m_wait_states != 0)
				m_wait_states--;
			else
			{
				data_read();
				prefetch_instruction();
				m_inst_state = T1_DECODE;
			}
			break;

		case T2_WRITE:
			if (m_wait_states != 0)
				m_wait_states--;
			else
			{
				data_write();
				prefetch_instruction();
				m_inst_state = T1_DECODE;
			}
			break;

		case T2_ABSOLUTE:
			if (m_wait_states != 0)
				m_wait_states--;
			else
			{
				if ((m_latched_instr & 0xfc00) == 0x8c00)
				{
					if (BIT(m_source_data, (m_latched_instr & 0x00e0) >> 5) == BIT(m_latched_instr, 8))
						m_inst_state = BIT(m_latched_instr, 9) ? T1_LCALL : T1_LJMP;
					else
						m_inst_state = T1_SKIP;
				}
				else
					m_inst_state = BIT(m_latched_instr, 7) ? T1_LCALL : T1_LJMP;
				prefetch_instruction();
			}
			break;
		}

		// If the timer is started, run it
		if (BIT(m_acr, 7))
		{
			if ((m_tclk++ & (BIT(m_acr, 5) ? 1 : 15)) == 0)
				timer_count();
		}

		m_icount--;
	}
}


//**************************************************************************
//  REMOTE INTERFACE AND ARBITRATION SYSTEM
//**************************************************************************

//-------------------------------------------------
//  cmd_r - read from Remote Interface Control
//  Register (off-chip access only)
//-------------------------------------------------

u8 dp8344_device::cmd_r()
{
	return m_ric;
}


//-------------------------------------------------
//  cmd_w - write to Remote Interface Control
//  Register (off-chip access only)
//-------------------------------------------------

void dp8344_device::cmd_w(u8 data)
{
	// Bit 7 = Bidirectional Interrupt Status
	// Bit 6 = Single-Step
	// Bit 5 = Fast Write
	// Bit 4 = Latched Read
	// Bit 3 = Latched Write
	// Bit 2 = STaRT
	// Bits 1-0 = Memory Select

	logerror("%s: %s %s read, %s write; CPU %s\n", machine().describe_context(),
		(data & 0x03) == 0x00 ? "Data memory" :
			(data & 0x03) == 0x01 ? "Instruction memory" :
			(data & 0x03) == 0x02 ? "PC low byte" : "PC high byte",
		BIT(data, 4) ? "latched" : "buffered",
		BIT(data, 3) ? "latched" : BIT(data, 5) ? "fast buffered" : "slow buffered",
		BIT(data, 6) ? "single step" : BIT(data, 2) ? "start" : "stop");

	// Make sure not to restart in the middle of an instruction
	if (!BIT(m_ric, 2) && BIT(data, 2))
		m_inst_state = T1_START;

	m_ric = data;
	set_input_line(INPUT_LINE_HALT, BIT(data, 2) ? CLEAR_LINE : ASSERT_LINE);
}


//-------------------------------------------------
//  remote_read - off-chip memory read
//-------------------------------------------------

u8 dp8344_device::remote_read(offs_t offset)
{
	u8 data = 0;

	// TODO: stop CPU if not accessing DMEM or RIC

	switch (m_ric & 0x03)
	{
	case 0x00:
		// Read from data memory
		data = m_data_space.read_byte(offset);
		break;

	case 0x01:
		// Read from instruction memory
		if (!m_hib && !machine().side_effects_disabled())
			m_latched_instr = m_inst_space.read_word(m_pc);
		data = m_hib ? m_latched_instr >> 8 : m_latched_instr & 0xff;
		if (!machine().side_effects_disabled())
		{
			m_hib = !m_hib;
			if (!m_hib)
				m_pc++;
		}
		break;

	case 0x02:
		// Read PC low byte
		data = m_pc & 0x00ff;
		break;

	case 0x03:
		// Read PC high byte
		data = m_pc >> 8;
		break;
	}

	// set Remote Read bit in CCR
	if (!machine().side_effects_disabled())
		m_ccr |= 0x40;

	return data;
}


//-------------------------------------------------
//  remote_write - off-chip memory write
//-------------------------------------------------

void dp8344_device::remote_write(offs_t offset, u8 data)
{
	// TODO: stop CPU if not accessing DMEM or RIC
	switch (m_ric & 0x03)
	{
	case 0x00:
		// Write to data memory
		m_data_space.write_byte(offset, data);
		break;

	case 0x01:
		// Write to instruction memory
		if (m_hib)
			m_latched_instr = (data << 8) | (m_latched_instr & 0x00ff);
		else
			m_latched_instr = data | (m_latched_instr & 0xff00);
		if (!machine().side_effects_disabled())
		{
			m_hib = !m_hib;
			if (!m_hib)
				m_inst_space.write_word(m_pc++, m_latched_instr);
		}
		break;

	case 0x02:
		// Write PC low byte
		m_pc = data | (m_pc & 0xff00);
		break;

	case 0x03:
		// Write PC high byte
		m_pc = data << 8 | (m_pc & 0x00ff);
		break;
	}

	// set Remote Write bit in CCR
	m_ccr |= 0x20;
}
