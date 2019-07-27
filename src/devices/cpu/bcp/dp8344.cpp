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

DEFINE_DEVICE_TYPE(DP8344, dp8344_device, "dp8344", "DP8344 BCP")


//**************************************************************************
//  DEVICE CONSTRUCTION AND INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  dp8344_device - constructor
//-------------------------------------------------

dp8344_device::dp8344_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: cpu_device(mconfig, DP8344, tag, owner, clock)
	, m_inst_config("instruction", ENDIANNESS_LITTLE, 16, 16, -1)
	, m_data_config("data", ENDIANNESS_LITTLE, 8, 16, 0)
	, m_inst_space(nullptr)
	, m_data_space(nullptr)
	, m_inst_cache(nullptr)
	, m_birq_out_cb(*this)
	, m_data_out_cb(*this)
	, m_data_dly_cb(*this)
	, m_tx_act_cb(*this)
	, m_pc(0)
	, m_icount(0)
	, m_nmi_pending(false)
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
	, m_ir{{0}, {0}, {0}, {0}}
	, m_tr(0)
	, m_as{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
	, m_ds{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
	, m_asp(0)
	, m_dsp(0)
	, m_ba(false)
	, m_bb(false)
	, m_ric(0)
	, m_hib(false)
	, m_auto_start(true)
	, m_nmi_state(false)
	, m_rfifo{0, 0, 0}
	, m_tfifo{0, 0, 0}
	, m_rfifo_head(0)
	, m_tfifo_head(0)
{
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
	m_inst_space = &space(AS_PROGRAM);
	m_data_space = &space(AS_DATA);
	m_inst_cache = m_inst_space->cache<1, -1, ENDIANNESS_LITTLE>();

	set_icountptr(m_icount);

	// debug state registration
	state_add(BCP_PC, "PC", m_pc);
	state_add(STATE_GENPC, "GENPC", m_pc).noshow();
	state_add(STATE_GENPCBASE, "GENPCBASE", m_pc).noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_ccr).noshow();
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
		state_add(BCP_IW + i, string_format("I%c", 'W' + i).c_str(), m_ir[i].w);
		state_add(BCP_IWLO + i, string_format("I%cLO", 'W' + i).c_str(), m_ir[i].b.l).noshow();
		state_add(BCP_IWHI + i, string_format("I%cHI", 'W' + i).c_str(), m_ir[i].b.h).noshow();
	}
	for (int i = 8; i < 16; i++)
		state_add(BCP_GP8 + i - 8, string_format("GP%d", i).c_str(), m_gp_main[i]);
	state_add(BCP_TR, "TR", m_tr);
	state_add(BCP_ASP, "ASP", m_asp).mask(0xf);
	state_add(BCP_DSP, "DSP", m_dsp).mask(0xf);

	// save states
	save_item(NAME(m_pc));
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
	for (int i = 0; i < 4; i++)
		save_item(NAME(m_ir[i].w), i);
	save_item(NAME(m_tr));
	save_item(NAME(m_asp));
	save_item(NAME(m_dsp));
	save_item(NAME(m_as)); // 12-word Address Stack
	save_item(NAME(m_ds)); // 16-byte Data Stack
	save_item(NAME(m_ba));
	save_item(NAME(m_bb));
	save_item(NAME(m_ric));
	save_item(NAME(m_hib));
	save_item(NAME(m_nmi_pending));
	save_item(NAME(m_rfifo)); // 3-frame Receive FIFO
	save_item(NAME(m_tfifo)); // 3-frame Transmit FIFO
	save_item(NAME(m_rfifo_head));
	save_item(NAME(m_tfifo_head));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void dp8344_device::device_reset()
{
	// Reset Program Counter
	m_pc = 0x0000;

	// Reset Condition Code Register
	m_ccr &= 0x10;

	// Reset Interrupt Control Register
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

	m_nmi_pending = false;

	transceiver_reset();

	// Undefined on reset: ATR, FBR, TRH, TRL, data in transceiver FIFOs and stacks
	// Also likely undefined on reset: all general-purpose registers
}


//**************************************************************************
//  CONTROL REGISTERS AND CONDITION CODES
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

	case 6: // Data Available
		return BIT(m_tsr, 3); // FIXME: differs from numeric value!

	case 7: // Transmitter FIFO Full (or else Not Full)
		return BIT(m_tsr, 7);

	default:
		logerror("Unknown flag %d\n", f);
		return false;
	}
}


//-------------------------------------------------
//  set_receiver_interrupt - update receiver
//  interrupt request status
//-------------------------------------------------

void dp8344_device::set_receiver_interrupt(bool state)
{
	// TODO
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
		set_timer_interrupt(false);

	// Bit 3 is reserved
	m_acr = data & 0xf7;
}


//**************************************************************************
//  16-BIT TIMER
//**************************************************************************

//-------------------------------------------------
//  get_timer_count - return current count status
//  of the timer
//-------------------------------------------------

u16 dp8344_device::get_timer_count()
{
	// TODO
	return 0;
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
		| (m_ba << 20)
		| (m_bb << 21)
		| (u32(m_acr & 0x01) << 22);
	if (m_asp >= 12)
		m_asp = 0;
}


//-------------------------------------------------
//  address_stack_pop - pop address frame upon
//  executing a RET or RETF instruction
//-------------------------------------------------

void dp8344_device::address_stack_pop(u8 grf)
{
	m_asp = (m_asp == 0) ? 11 : m_asp - 1;
	m_pc = m_as[m_asp] & 0xffff;

	// Optionally restore, set or clear GIE
	if (BIT(grf, 6))
		set_gie(BIT(grf, 5));
	else if (BIT(grf, 5))
		set_gie(BIT(m_as[m_asp], 22));

	// Optionally restore ALU flags and register banks
	if (BIT(grf, 4))
	{
		m_ccr = (m_ccr & 0xf0) | (m_as[m_asp] & 0xf0000) >> 20;
		m_ba = BIT(m_as[m_asp], 20);
		m_bb = BIT(m_as[m_asp], 21);
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
	m_dsp = (m_dsp - 1) & 0xf;
	return m_ds[m_dsp];
}




//**************************************************************************
//  TRANSCEIVER OPERATION
//**************************************************************************

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
//  receive_fifo_push -
//-------------------------------------------------

void dp8344_device::receive_fifo_push(u8 data)
{
	// Overflow error if the receive FIFO is already full
	if (BIT(m_ncf, 6))
		set_receiver_error(0x10);
}


//-------------------------------------------------
//  receive_fifo_pop - read from RTR
//-------------------------------------------------

u8 dp8344_device::receive_fifo_pop()
{
	// Clear Receive FIFO Full, DEME, Auto-Response and Poll/Acknowledge bits in NCF
	m_ncf &= 0xb0;

	// Clear Data Available flag in TSR
	m_tsr &= 0xf7;
	if ((m_icr & 0xc0) == 0xc0)
		set_receiver_interrupt(false);

	return 0;
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

u8 dp8344_device::get_error_code()
{
	u8 code = m_ecr;

	// Clear Error Code Register and Receiver Error flag in NCF
	m_ecr = 0x00;
	m_ncf &= 0xdf;

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

	return code;
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

	switch (data & 0x07)
	{
	case 0:
		logerror("Protocol Select: IBM 3270\n");
		break;

	case 1:
		logerror("Protocol Select: IBM 3299 Multiplexer\n");
		break;

	case 2:
		logerror("Protocol Select: IBM 3299 Controller\n");
		break;

	case 3:
		logerror("Protocol Select: IBM 3299 Repeater\n");
		break;

	case 4:
		logerror("Protocol Select: IBM 5250\n");
		break;

	case 5:
		logerror("Protocol Select: IBM 5250 Promiscuous\n");
		break;

	case 6:
		logerror("Protocol Select: 8-bit\n");
		break;

	case 7:
		logerror("Protocol Select: 8-bit Promiscuous\n");
		break;
	}

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

u8 dp8344_device::read_register(unsigned reg)
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
			return BIT(m_tcr, 6) ? get_error_code() : receive_fifo_pop();
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
		return m_ir[0].b.l;

	case 13: // IW (high byte)
		return m_ir[0].b.h;

	case 14: // IX (low byte)
		return m_ir[1].b.l;

	case 15: // IX (high byte)
		return m_ir[1].b.h;

	case 16: // IY (low byte)
		return m_ir[2].b.l;

	case 17: // IY (high byte)
		return m_ir[2].b.h;

	case 18: // IZ (low byte)
		return m_ir[3].b.l;

	case 19: // IZ (high byte)
		return m_ir[3].b.h;

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
		return get_timer_count() & 0x00ff;

	case 29: // TRH count status
		return get_timer_count() >> 8;

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
			m_dcr = data;
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

	case 6: // GP2 (main) or Transceiver Command Register (alternate)
		if (m_bb)
			m_tcr = data;
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
		m_ir[0].b.l = data;
		break;

	case 13: // IW (high byte)
		m_ir[0].b.h = data;
		break;

	case 14: // IX (low byte)
		m_ir[1].b.l = data;
		break;

	case 15: // IX (high byte)
		m_ir[1].b.h = data;
		break;

	case 16: // IY (low byte)
		m_ir[2].b.l = data;
		break;

	case 17: // IY (high byte)
		m_ir[2].b.h = data;
		break;

	case 18: // IZ (low byte)
		m_ir[3].b.l = data;
		break;

	case 19: // IZ (high byte)
		m_ir[3].b.h = data;
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
//  create_disassembler -
//-------------------------------------------------

std::unique_ptr<util::disasm_interface> dp8344_device::create_disassembler()
{
	return std::make_unique<dp8344_disassembler>();
}


//-------------------------------------------------
//  execute_run -
//-------------------------------------------------

void dp8344_device::execute_run()
{
	debugger_instruction_hook(m_pc);

	// TODO: everything
	m_icount = 0;
}


//**************************************************************************
//  REMOTE INTERFACE AND ARBITRATION SYSTEM
//**************************************************************************

//-------------------------------------------------
//  ric_w - write to Remote Interface Control
//  Register from off-chip
//-------------------------------------------------

void dp8344_device::ric_w(u8 data)
{
	// Bit 7 = Bidirectional Interrupt Status
	// Bit 6 = Single-Step
	// Bit 5 = Fast Write
	// Bit 4 = Latched Read
	// Bit 3 = Latched Write
	// Bit 2 = STaRT
	// Bits 1-0 = Memory Select

	m_ric = data;
}


//-------------------------------------------------
//  remote_read -
//-------------------------------------------------

void dp8344_device::remote_read()
{
	// TODO: stop CPU if not accessing DMEM or RIC

	// set Remote Read bit in CCR
	m_ccr |= 0x40;
}


//-------------------------------------------------
//  remote_write -
//-------------------------------------------------

void dp8344_device::remote_write()
{
	// TODO: stop CPU if not accessing DMEM or RIC

	// set Remote Write bit in CCR
	m_ccr |= 0x20;
}
