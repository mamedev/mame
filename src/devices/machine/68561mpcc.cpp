// license:BSD-3-Clause
// copyright-holders: Joakim Larsson Edstrom
/***************************************************************************

    MPCC Multi-Protocol Communications Controller emulation

    The MPCC was introduced in the late 80:ies by Rockwell

    The variants in the MPCC family are as follows:

    - 68560  with an 8 bit data bus
    - 68560A with an 8 bit data bus and some enhancements
    - 68561  with a 16 bit data bus
    - 68561A with a 16 bit data bus and some enhancements

FEATURES
------------------------------------------------------------------
 *   Full duplex synchronous/asynchronous receiver and transmitter
 *   Implements IBM Binary Synchronous Communications (BSC) in two coding formats: ASCII and EBCDIC
 *   Supports other synchronous character -oriented protocols (COP), such as six -bit BSC, X3.28k. ISO IS1745, ECMA-16, etc.
 *   Supports synchronous bit oriented protocols (BOP), such as SDLC, HDLC, X.25, etc.
 *   Asynchronous and isochronous modes
 *   Modem handshake interface
 *   High speed serial data rate (DC to 4 MHz)
 *   Internal oscillator and baud rate generator with programmable data rate
 *   Crystal or TTL level clock input and buffered clock output (8 MHz)
 *   Direct interface to 68008/68000 asynchronous bus
 *   Eight -character receiver and transmitter buffer registers
 *   22 directly addressable registers for flexible option selection, complete status reporting, and data transfer
 *   Three separate programmable interrupt vector numbers for receiver, transmitter and serial interface
 *   Maskable interrupt conditions for receiver, transmitter and serial interface
 *   Programmable microprocessor bus data transfer; polled, interrupt and two -channel DMA transfer compatible with MC68440/MC68450
 *   Clock control register for receiver clock divisor and receiver and transmitter clock routing
 *   Selectable full/half duplex, autoecho and local loop -back modes
 *   Selectable parity (enable, odd, even) and CRC (control field enable, CRC -16, CCITT V.41, VRC/LRC)
 *-------------------------------------------------------------------------------------------
 *       x = Features that has been implemented  p = partly n = features that will not
 *-------------------------------------------------------------------------------------------
*/

#include "emu.h"
#include "68561mpcc.h"

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************
//#define LOG_GENERAL (1U <<  0)
#define LOG_SETUP   (1U <<  1)
#define LOG_INT     (1U <<  2)
#define LOG_READ    (1U <<  4)
#define LOG_CMD     (1U <<  5)
#define LOG_TX      (1U <<  6)
#define LOG_RCV     (1U <<  7)
#define LOG_CTS     (1U <<  8)
#define LOG_DCD     (1U <<  9)
#define LOG_SYNC    (1U <<  10)
#define LOG_CHAR    (1U <<  11)
#define LOG_RX      (1U <<  12)

//#define VERBOSE ( LOG_SETUP | LOG_GENERAL | LOG_INT)
//#define LOG_OUTPUT_FUNC printf

#include "logmacro.h"

//#define LOG(...)      LOGMASKED(LOG_GENERAL, __VA_ARGS__)
#define LOGSETUP(...) LOGMASKED(LOG_SETUP,   __VA_ARGS__)
#define LOGR(...)     LOGMASKED(LOG_READ,    __VA_ARGS__)
#define LOGINT(...)   LOGMASKED(LOG_INT,     __VA_ARGS__)
#define LOGCMD(...)   LOGMASKED(LOG_CMD,     __VA_ARGS__)
#define LOGTX(...)    LOGMASKED(LOG_TX,      __VA_ARGS__)
#define LOGRCV(...)   LOGMASKED(LOG_RCV,     __VA_ARGS__)
#define LOGCTS(...)   LOGMASKED(LOG_CTS,     __VA_ARGS__)
#define LOGDCD(...)   LOGMASKED(LOG_DCD,     __VA_ARGS__)
#define LOGSYNC(...)  LOGMASKED(LOG_SYNC,    __VA_ARGS__)
#define LOGCHAR(...)  LOGMASKED(LOG_CHAR,    __VA_ARGS__)
#define LOGRX(...)    LOGMASKED(LOG_RX,      __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************
// device type definition
DEFINE_DEVICE_TYPE(MPCC,       mpcc_device,       "mpcc",       "Rockwell MPCC")
DEFINE_DEVICE_TYPE(MPCC68560,  mpcc68560_device,  "mpcc68560",  "MPCC 68560")
DEFINE_DEVICE_TYPE(MPCC68560A, mpcc68560a_device, "mpcc68560a", "MPCC 68560A")
DEFINE_DEVICE_TYPE(MPCC68561,  mpcc68561_device,  "mpcc68561",  "MPCC 68561")
DEFINE_DEVICE_TYPE(MPCC68561A, mpcc68561a_device, "mpcc68561a", "MPCC 68561A")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

mpcc_device::mpcc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t variant)
	: device_t(mconfig, type, tag, owner, clock),
	  device_serial_interface(mconfig, *this),
	  m_irq(CLEAR_LINE),
	  m_variant(variant),
	  m_rxc(0),
	  m_txc(0),
	  m_brg_rate(0),
	  m_rcv(0),
	  m_rxd(0),
	  m_tra(0),
	  m_out_txd_cb(*this),
	  m_out_dtr_cb(*this),
	  m_out_rts_cb(*this),
	  m_out_rtxc_cb(*this),
	  m_out_trxc_cb(*this),
	  m_out_int_cb(*this),
	  m_rsr(0),
	  m_rcr(0),
	  m_rdr(0),
	  m_rivnr(0),
	  m_rier(0),
	  m_tsr(0),
	  m_tcr(0),
	  m_tdr(0),
	  m_tivnr(0),
	  m_tier(0),
	  m_sisr(0),
	  m_sicr(0),
	  m_sivnr(0),
	  m_sier(0),
	  m_psr1(0),
	  m_psr2(0),
	  m_ar1(0),
	  m_ar2(0),
	  m_brdr1(0),
	  m_brdr2(0),
	  m_ccr(0),
	  m_ecr(0)
{
	for (auto & elem : m_int_state)
		elem = 0;
}

mpcc_device::mpcc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mpcc_device(mconfig, MPCC, tag, owner, clock, TYPE_MPCC)
{
}

mpcc68560_device::mpcc68560_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mpcc_device(mconfig, MPCC68560, tag, owner, clock, TYPE_MPCC68560)
{
}

mpcc68560a_device::mpcc68560a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mpcc_device(mconfig, MPCC68560A, tag, owner, clock, TYPE_MPCC68560A)
{
}

mpcc68561_device::mpcc68561_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mpcc_device(mconfig, MPCC68561, tag, owner, clock, TYPE_MPCC68561)
{
}

mpcc68561a_device::mpcc68561a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mpcc_device(mconfig, MPCC68561A, tag, owner, clock, TYPE_MPCC68561A)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void mpcc_device::device_start()
{
	LOGSETUP("%s\n", FUNCNAME);

	// resolve callbacks
	m_out_txd_cb.resolve_safe();
	m_out_dtr_cb.resolve_safe();
	m_out_rts_cb.resolve_safe();
	m_out_rtxc_cb.resolve_safe();
	m_out_trxc_cb.resolve_safe();
	m_out_int_cb.resolve_safe();

	// state saving
	save_item(NAME(m_int_state));
	save_item(NAME(m_rsr));
	save_item(NAME(m_rcr));
	save_item(NAME(m_rdr));
	save_item(NAME(m_rivnr));
	save_item(NAME(m_rier));
	save_item(NAME(m_tsr));
	save_item(NAME(m_tcr));
	save_item(NAME(m_tdr));
	save_item(NAME(m_tivnr));
	save_item(NAME(m_tier));
	save_item(NAME(m_sisr));
	save_item(NAME(m_sicr));
	save_item(NAME(m_sivnr));
	save_item(NAME(m_sier));
	save_item(NAME(m_psr1));
	save_item(NAME(m_psr2));
	save_item(NAME(m_ar1));
	save_item(NAME(m_ar2));
	save_item(NAME(m_brdr1));
	save_item(NAME(m_brdr2));
	save_item(NAME(m_ccr));
	save_item(NAME(m_ecr));
	LOG(" - MPCC variant %02x\n", m_variant);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------
void mpcc_device::device_reset()
{
	LOGSETUP("%s %s \n",tag(), FUNCNAME);

	// Reset RS232 emulation
	receive_register_reset();
	transmit_register_reset();

	// Device reset values
	m_rsr   = 0x00;
	m_rcr   = 0x01;
	m_rivnr = 0x0f;
	m_rier  = 0x00;
	m_tsr   = 0x80;
	m_tcr   = 0x01;
	m_tivnr = 0x0f;
	m_tier  = 0x00;
	m_sisr  = 0x00;
	m_sicr  = 0x00;
	m_sivnr = 0x0f;
	m_sier  = 0x00;
	m_psr1  = 0x00;
	m_psr2  = 0x00;
	m_ar1   = 0x00;
	m_ar2   = 0x00;
	m_brdr1 = 0x01;
	m_brdr2 = 0x00;
	m_ccr   = 0x00;
	m_ecr   = 0x04;

	// Clear fifos
	m_tx_data_fifo.clear();
	m_rx_data_fifo.clear();

	// Init out callbacks to known inactive state
	m_out_txd_cb(1);
	m_out_dtr_cb(CLEAR_LINE);
	m_out_rts_cb(CLEAR_LINE);
	m_out_rtxc_cb(CLEAR_LINE);
	m_out_trxc_cb(CLEAR_LINE);
	m_out_int_cb(CLEAR_LINE);
	m_irq = CLEAR_LINE;
}

/*
 * Serial device implementation
 */
WRITE_LINE_MEMBER(mpcc_device::cts_w)
{
	if (state == CLEAR_LINE)
	{
		uint8_t old_sisr = m_sisr;

		m_sisr &= ~REG_SISR_CTSLVL;
		if ( (old_sisr & REG_SISR_CTSLVL) &&
			 (m_sicr & REG_SICR_RTSLVL) &&
			 (m_tcr  & REG_TCR_TEN))
		{
			m_sisr |= REG_SISR_CTST;
			if (m_sier & REG_SIER_CTS)
			{
				// TODO: make sure interrupt is issued with the next negative transition of TxC
				trigger_interrupt(INT_SR_CTS);
				// TODO: Make sure TxC has negative transition after CTS goes inactive before INT can be reset in SISR7
			}
		}
	}
	else
		m_sisr |= REG_SISR_CTSLVL;
}

WRITE_LINE_MEMBER(mpcc_device::dsr_w)
{
	if (state == ASSERT_LINE)
	{
		uint8_t old_sisr = m_sisr;

		m_sisr |= REG_SISR_DSRLVL;
		if ( !(old_sisr & REG_SISR_DSRLVL) &&
			 !(m_rcr  & REG_RCR_RRES))
		{
			m_sisr |= REG_SISR_DSRT;
			if (m_sier & REG_SIER_DSR)
			{
				// TODO: make sure interrupt is issued with the next negative transition of RxC
				trigger_interrupt(INT_SR_DSR);
				// TODO: Make sure RxC has negative transition after DSR goes inactive before INT can be reset in SISR6
			}
		}
	}
	else
		m_sisr &= ~REG_SISR_DSRLVL;
}

WRITE_LINE_MEMBER(mpcc_device::dcd_w)
{
	if (state == CLEAR_LINE)
	{
		uint8_t old_sisr = m_sisr;

		m_sisr &= ~REG_SISR_DCDLVL;
		if ( (old_sisr & REG_SISR_DCDLVL) &&
			 !(m_rcr & REG_RCR_RRES))
		{
			m_sisr |= REG_SISR_DCDT;
			if (m_sier & REG_SIER_DCD)
			{
				// TODO: make sure interrupt is issued with the next negative transition of RxC
				trigger_interrupt(INT_SR_DCD);
				// TODO: Make sure RxC has negative transition before INT can be reset in SISR5
			}
		}
	}
	else
		m_sisr |= REG_SISR_DCDLVL;
}

//-------------------------------------------------
// get_brg_rate - helper function
//-------------------------------------------------
uint32_t mpcc_device::get_brg_rate()
{
	uint32_t rate;
	uint32_t brg_const;

	brg_const = (m_brdr1 | m_brdr2 << 8); // Baud rate divider
	brg_const += (m_ccr & REG_CCR_PSCDIV) ? 3 : 2; // Add prescaler factor
	brg_const += (m_psr2 & REG_PSR2_PSEL_MSK) == REG_PSR2_PSEL_ASCII ? 2 : 1; // Add K factor
	rate = clock() / brg_const;

	return rate;
}

//-------------------------------------------------
// get_tx_rate - helper function
//-------------------------------------------------
uint32_t mpcc_device::get_tx_rate()
{
	uint32_t rate;

	// Check if TxC is an input and use it instead of the BRG
	if ((m_ccr & REG_CCR_TCLO) == 0)
	{
		rate = m_txc;
	}
	else
	{
		rate = get_brg_rate();
	}

	return rate;
}

//-------------------------------------------------
// get_clock_div - helper function
//-------------------------------------------------
uint32_t mpcc_device::get_clock_div()
{
	uint32_t clk_div = 1;

	switch (m_ccr & REG_CCR_CLKDIV_MSK)
	{
	case REG_CCR_CLKDIV_X1 : clk_div =  1; break;
	case REG_CCR_CLKDIV_X16: clk_div = 16; break;
	case REG_CCR_CLKDIV_X32: clk_div = 32; break;
	case REG_CCR_CLKDIV_X64: clk_div = 64; break;
	}

	return clk_div;
}

//-------------------------------------------------
// get_rx_rate - helper function
//-------------------------------------------------
uint32_t mpcc_device::get_rx_rate()
{
	uint32_t rate;

	// Check if TxC is an input and use it instead of the BRG
	if ((m_ccr & REG_CCR_RCLKIN) == 0)
	{
		rate = m_rxc / get_clock_div();
	}
	else
	{
		rate = get_brg_rate();
	}

	return rate;
}

//-------------------------------------------------
//  get_word_length - get word length
//-------------------------------------------------
uint32_t mpcc_device::get_word_length()
{
	int bits = 5;

	switch (m_psr2 & REG_PSR2_CHLN_MSK)
	{
	case REG_PSR2_CHLN_5:  bits = 5;   break;
	case REG_PSR2_CHLN_6:  bits = 6;   break;
	case REG_PSR2_CHLN_7:  bits = 7;   break;
	case REG_PSR2_CHLN_8:  bits = 8;   break;
	}

	return bits;
}

//-------------------------------------------------
//  get_stop_bits - translate stop bit settings for serial interface
//-------------------------------------------------
device_serial_interface::stop_bits_t mpcc_device::get_stop_bits()
{
	switch (m_psr2 & REG_PSR2_STP_MSK)
	{
	case REG_PSR2_STP_1:   return STOP_BITS_1;
	case REG_PSR2_STP_1_5: return STOP_BITS_1_5;
	case REG_PSR2_STP_2:   return STOP_BITS_2;
	}

	return STOP_BITS_0;
}

//-------------------------------------------------
//  get_parity - translate parity settings for serial interface
//-------------------------------------------------
device_serial_interface::parity_t mpcc_device::get_parity()
{
	parity_t parity;

	if (m_ecr & REG_ECR_PAREN)
	{
		if (m_ecr & REG_ECR_ODDPAR)
			parity = PARITY_ODD;
		else
			parity = PARITY_EVEN;
	}
	else
	{
		parity = PARITY_NONE;
	}
	return parity;
}

//-------------------------------------------------
//  update_serial -
//-------------------------------------------------
void mpcc_device::update_serial()
{
	int         data_bits = get_word_length();
	stop_bits_t stop_bits = get_stop_bits();
	parity_t    parity    = get_parity();

	LOGSETUP(" %s() %s Setting data frame %d+%d%c%s\n", FUNCNAME, owner()->tag(), 1,
		 data_bits, parity == PARITY_NONE ? 'N' : parity == PARITY_EVEN ? 'E' : 'O',
		stop_bits == STOP_BITS_1 ? "1" : (stop_bits == STOP_BITS_2 ? "2" : "1.5"));

	set_data_frame(1, data_bits, parity, stop_bits);

	// Setup the Receiver
	//  check if the receiver is in reset mode
	if  (m_rcr & REG_RCR_RRES)
	{
		LOGSETUP("- Rx in reset\n");
		set_rcv_rate(0);
	}
	// Rx is running
	else
	{
		LOGSETUP("- Rx enabled\n");
		m_brg_rate = get_rx_rate();

		LOGSETUP("- BRG rate %d\n", m_brg_rate);
		set_rcv_rate(m_brg_rate);
	}

	// Setup the Transmitter
	//  check if Rx is in reset
	if  (m_tcr & REG_TCR_TRES)
	{
		LOGSETUP("- Tx in reset\n");
		set_tra_rate(0);
	}
	// Tx is running
	else
	{
		// Check that Tx is enabled
		if (m_tcr & REG_TCR_TEN)
		{
			LOGSETUP("- Tx enabled\n");
			m_brg_rate = get_tx_rate();

			LOGSETUP("- BRG rate %d\n", m_brg_rate);
			set_tra_rate(m_brg_rate);
		}
		else
		{
			LOGSETUP("- Tx disabled\n");
			set_tra_rate(0);
		}
	}
}

//-------------------------------------------------
//  tra_callback - is called for each bit that needs to be transmitted
//-------------------------------------------------
void mpcc_device::tra_callback()
{
	// Check if transmitter is idle as in disabled
	if (!(m_tcr & REG_TCR_TEN))
	{
		// transmit idle TODO: Support TCR TICS bit
		LOGTX("%s idle bit\n", FUNCNAME);
		m_out_txd_cb(1);
	}
#if 0
	// Check if we are transmitting break TODO: Figure out Break support
	else if (...)
	{
		// transmit break
		m_out_txd_cb(0);
	}
#endif
	// Check if there is more bits to send
	else if (!is_transmit_register_empty())
	{
		// Get the next bit
		int db = transmit_register_get_data_bit();
		LOGTX("%s bit: %d\n", FUNCNAME, db ? 1 : 0);

		// transmit data
		m_out_txd_cb(db);
	}
	// Otherwise we don't know why we are called...
	else
	{
		logerror("%s %s Failed to transmit\n", FUNCNAME, owner()->tag());
	}
}

//-------------------------------------------------
//  tra_complete - is called when the transmitter shift register has sent the last bit
//-------------------------------------------------
void mpcc_device::tra_complete()
{
	// check if transmitter is enabled and we are not sending BREAK level
	if ((m_tcr & REG_TCR_TEN) && !(m_tcr & REG_TCR_TICS))
	{   // check if there are more data in the fifo
		if (!m_tx_data_fifo.empty())
		{
			transmit_register_setup(m_tx_data_fifo.dequeue()); // Reload the shift register
			m_tsr |= REG_TSR_TDRA; // Mark fifo as having room for more data
		}
		else
		{
			m_out_rts_cb(CLEAR_LINE); // TODO: respect the RTSLV bit
			m_sicr &= ~REG_SICR_RTSLVL;
		}

		// Check if Tx interrupts are enabled
		if (m_tier & REG_TIER_TDRA)
		{
			trigger_interrupt(INT_TX_TDRA);
		}
	}       // Check if sending BREAK
	else if (m_tcr & REG_TCR_TICS)
	{
		// TODO: Should transmit content of AR2, needs investigation
		m_out_txd_cb(0);
	}
	else
	{
		// transmit mark
		m_out_txd_cb(1);
	}
}

//-------------------------------------------------
//  rcv_callback - called when it is time to sample incoming data bit
//-------------------------------------------------
void mpcc_device::rcv_callback()
{
	// Check if the Receiver is enabled
	if (!(m_rcr & REG_RCR_RRES))
	{
		receive_register_update_bit(m_rxd);
	}
}

//-------------------------------------------------
//  rcv_complete -
//-------------------------------------------------
void mpcc_device::rcv_complete()
{
	uint8_t data;

	receive_register_extract();
	data = get_received_char();
	LOGRX("%s %02x [%c]\n", FUNCNAME, isascii(data) ? data : ' ', data);

	uint8_t errors = 0;
	if (is_receive_parity_error())
		errors |= REG_RSR_CPERR;
	if (is_receive_framing_error())
		errors |= REG_RSR_FRERR;

	//  receive_data(data);
	if (m_rx_data_fifo.full())
	{
		// receive overrun error detected, new data is lost
		m_rsr |= REG_RSR_ROVRN;
		// interrupt if rx overrun interrupt is enabled
		if (m_rier & REG_RIER_ROVRN)
		{
			trigger_interrupt(INT_RX_ROVRN);
		}
	}
	else
	{
		if (m_rx_data_fifo.empty())
		{
			m_rsr |= errors;
			update_interrupts(INT_RX);
		}
		m_rx_data_fifo.enqueue(data | errors << 8);
		m_rsr |= REG_RSR_RDA;
		// interrupt if rx data availble is enabled
		if (m_rier & REG_RIER_RDA)
		{
			trigger_interrupt(INT_RX_RDA);
		}
	}
}

//-------------------------------------------------
//  write_rx - called by terminal through rs232/diserial
//         when character is sent to board
//-------------------------------------------------
WRITE_LINE_MEMBER(mpcc_device::write_rx)
{
	LOGRCV("%s(%d)\n", FUNCNAME, state);
	m_rxd = state;

	//only use rx_w when self-clocked
	if(m_rxc != 0 || m_brg_rate != 0)
		device_serial_interface::rx_w(state);
}


/*
 * Interrupts
 */
//-------------------------------------------------
//  check_interrupts -
//-------------------------------------------------
void mpcc_device::check_interrupts()
{
	int state = 0;
	LOGINT("%s %s \n",tag(), FUNCNAME);

	// loop over all interrupt sources
	for (auto & elem : m_int_state)
	{
		state |= elem;
	}

	// update IRQ line
	// If we are not serving any interrupt we need to check for a new interrupt or
	//   otherwise the IRQ line is asserted already and we need to do nothing
	if ((state & INT_ACK) == 0)
	{
		// If there is a new interrupt not yet acknowledged IRQ needs to be asserted
		if (state & INT_REQ)
		{
			if (m_irq != ASSERT_LINE)
			{
				m_out_int_cb(ASSERT_LINE);
				m_irq = ASSERT_LINE;
			}
		}
		// Otherwise we just clear the IRQ line allowing other devices to interrupt
		else
		{
			if (m_irq != CLEAR_LINE)
			{
				m_out_int_cb(CLEAR_LINE);
				m_irq = CLEAR_LINE;
			}
		}
	}
}

//-------------------------------------------------
//  reset_interrupts -
//-------------------------------------------------

void mpcc_device::reset_interrupts()
{
	LOGINT("%s %s \n",tag(), FUNCNAME);
	// reset internal interrupt sources
	for (auto & elem : m_int_state)
	{
		elem = 0;
	}

	// check external interrupt sources
	check_interrupts();
}

//-----------------------------------------------------------------------
//  trigger_interrupt - called when a potential interrupt condition occurs
//-------------------------------------------------
void mpcc_device::trigger_interrupt(int source)
{
	LOGINT("%s %s: %02x\n",FUNCNAME, tag(), source);
	switch(source)
	{
	case INT_TX_TDRA:
	case INT_TX_TFC:
	case INT_TX_TUNRN:
	case INT_TX_TFERR:
		m_int_state[TX_INT_PRIO] = INT_REQ;
		break;
	case INT_RX_RDA:
	case INT_RX_EOF:
	case INT_RX_CPERR:
	case INT_RX_FRERR:
	case INT_RX_ROVRN:
	case INT_RX_RAB:
		m_int_state[RX_INT_PRIO] = INT_REQ;
		break;
	case INT_SR_CTS:
	case INT_SR_DSR:
	case INT_SR_DCD:
		m_int_state[SR_INT_PRIO] = INT_REQ;
		break;
	}
	check_interrupts();
}

//-------------------------------------------------------------------------
//  update_interrupt - called when an interrupt condition has been cleared
//-------------------------------------------------------------------------
void mpcc_device::update_interrupts(int source)
{
	LOGINT("%s %s \n",FUNCNAME, tag());

	switch(source)
	{
	case INT_TX:
	case INT_TX_TDRA:
	case INT_TX_TFC:
	case INT_TX_TUNRN:
	case INT_TX_TFERR:
		if (m_tsr & m_tier & (REG_TSR_TDRA | REG_TSR_TFC | REG_TSR_TUNRN | REG_TSR_TFERR))
		{
			LOGINT(" - Found unserved TX interrupt %02x\n", m_tsr & m_tier);
			m_int_state[TX_INT_PRIO] = INT_REQ; // Still TX interrupts to serve
		}
		else
		{
			m_int_state[TX_INT_PRIO] = INT_NONE; // No more TX interrupts to serve
		}
		break;
	case INT_RX:
	case INT_RX_RDA:
	case INT_RX_EOF:
	case INT_RX_CPERR:
	case INT_RX_FRERR:
	case INT_RX_ROVRN:
	case INT_RX_RAB:
		if (m_rsr & m_rier & (REG_RSR_RDA | REG_RSR_EOF | REG_RSR_CPERR | REG_RSR_FRERR | REG_RSR_ROVRN | REG_RSR_RAB))
		{
			LOGINT(" - Found unserved RX interrupt %02x\n", m_rsr & m_rier);
			m_int_state[RX_INT_PRIO] = INT_REQ; // Still RX interrupts to serve
		}
		else
		{
			m_int_state[RX_INT_PRIO] = INT_NONE; // No more RX interrupts to serve
		}
		break;
	case INT_SR:
	case INT_SR_CTS:
	case INT_SR_DSR:
	case INT_SR_DCD:
		if (m_sisr & m_sier & (REG_SISR_CTST | REG_SISR_DSRT | REG_SISR_DCDT))
		{
			LOGINT(" - Found unserved SR interrupt %02x\n", m_sisr & m_sier);
			m_int_state[SR_INT_PRIO] = INT_REQ; // Still SR interrupts to serve
		}
		else
		{
			m_int_state[SR_INT_PRIO] = INT_NONE; // No more SR interrupts to serve
		}
		break;
	}
	check_interrupts();
}

//-------------------------------------------------
//  Read register
//-------------------------------------------------
READ8_MEMBER( mpcc_device::read )
{
	uint8_t data = 0;

	switch(offset)
	{
	case 0x00: data = do_rsr(); break;
	case 0x01: data = do_rcr(); break;
	case 0x02: data = do_rdr(); break;
	case 0x04: data = do_rivnr(); break;
	case 0x05: data = do_rier(); break;
	case 0x08: data = do_tsr(); break;
	case 0x09: data = do_tcr(); break;
	//case 0x0a: data = m_tdr; break; // TDR is a write only register
	case 0x0c: data = do_tivnr(); break;
	case 0x0d: data = do_tier(); break;
	case 0x10: data = do_sisr(); break;
	case 0x11: data = do_sicr(); break;
	case 0x14: data = m_sivnr; logerror("MPCC: Reg SIVNR not implemented\n"); break;
	case 0x15: data = do_sier(); break;
	case 0x18: data = do_psr1(); break;
	case 0x19: data = do_psr2(); break;
	case 0x1a: data = m_ar1; logerror("MPCC: Reg AR1 not implemented\n"); break;
	case 0x1b: data = m_ar2; logerror("MPCC: Reg AR2 not implemented\n"); break;
	case 0x1c: data = do_brdr1(); break;
	case 0x1d: data = do_brdr2(); break;
	case 0x1e: data = do_ccr(); break;
	case 0x1f: data = do_ecr(); break;
	default: logerror("%s:%s invalid register accessed: %02x\n", owner()->tag(), tag(), offset);
	}
	LOGR(" * %s Reg %02x -> %02x  \n", tag(), offset, data);
	return data;
}

//-------------------------------------------------
//  Write register
//-------------------------------------------------
WRITE8_MEMBER( mpcc_device::write )
{
	LOGSETUP(" * %s Reg %02x <- %02x  \n", tag(), offset, data);
	switch(offset)
	{
	case 0x00: do_rsr(data); break;
	case 0x01: do_rcr(data); break;
	//case 0x02: m_rdr = data; break; // RDR is a read only register
	case 0x04: do_rivnr(data); break;
	case 0x05: do_rier(data); break;
	case 0x08: do_tsr(data); break;
	case 0x09: do_tcr(data); break;
	case 0x0a: m_tdr = data; LOGCHAR("*%c", data); do_tdr(data); break;
	case 0x0c: do_tivnr(data); break;
	case 0x0d: do_tier(data); break;
	case 0x10: do_sisr(data); break;
	case 0x11: do_sicr(data); break;
	case 0x14: m_sivnr = data; logerror("MPCC: Reg SIVNR not implemented\n"); break;
	case 0x15: do_sier(data); break;
	case 0x18: do_psr1(data); break;
	case 0x19: do_psr2(data); break;
	case 0x1a: m_ar1 = data; logerror("MPCC: Reg AR1 not implemented\n"); break;
	case 0x1b: m_ar2 = data; logerror("MPCC: Reg AR2 not implemented\n"); break;
	case 0x1c: do_brdr1(data); break;
	case 0x1d: do_brdr2(data); break;
	case 0x1e: do_ccr(data); break;
	case 0x1f: do_ecr(data); break;
	default: logerror("%s:%s invalid register accessed: %02x\n", owner()->tag(), tag(), offset);
	}
}

// TODO: implement Idle bit
void mpcc_device::do_rsr(uint8_t data)
{
	LOG("%s -> %02x\n", FUNCNAME, data);
	// writing 1 resets status bits except for RDA which is read-only
	m_rsr &= ~data | REG_RSR_RDA;
	// status belonging to data at the head of the FIFO cannot be cleared
	if (!m_rx_data_fifo.empty())
		m_rsr |= m_rx_data_fifo.peek() >> 8;
	update_interrupts(INT_RX);
}

uint8_t mpcc_device::do_rsr()
{
	uint8_t data = m_rsr;
	LOG("%s <- %02x\n", FUNCNAME, data);
	return data;
}

void mpcc_device::do_rcr(uint8_t data)
{
	LOG("%s -> %02x\n", FUNCNAME, data);
	m_rcr = data;
	LOGSETUP(" - Rx DMA      : %s\n", (m_rcr & REG_RCR_RDSREN) ? "enabled" : "disabled");
	LOGSETUP(" - Rx DONE out : %s\n", (m_rcr & REG_RCR_DONEEN) ? "enabled" : "disabled");
	LOGSETUP(" - Rx RSYN out : %s\n", (m_rcr & REG_RCR_RSYNEN) ? "enabled" : "disabled");
	LOGSETUP(" - Rx strip SYN: %s\n", (m_rcr & REG_RCR_STRSYN) ? "enabled" : "disabled");
	LOGSETUP(" - Rx Abort    : %s\n", (m_rcr & REG_RCR_RABTEN) ? "enabled" : "disabled");
	LOGSETUP(" - Rx Mode     : %s\n", (m_rcr & REG_RCR_RRES  ) ? "reset" : "normal");

	update_serial();
}

uint8_t mpcc_device::do_rcr()
{
	uint8_t data = m_rcr;
	LOG("%s <- %02x\n", FUNCNAME, data);
	return data;
}

uint8_t mpcc_device::do_rdr()
{
	uint8_t data = 0;

	if (!m_rx_data_fifo.empty())
	{
		// load data from the FIFO
		data = m_rx_data_fifo.dequeue() & 0xff;

		// Check if this was the last data and reset the interrupt and status register accordingly
		if (m_rx_data_fifo.empty())
		{
			m_rsr &= ~REG_RSR_RDA;
			update_interrupts(INT_RX_RDA);
		}
		else
		{
			m_rsr |= m_rx_data_fifo.peek() >> 8;
			update_interrupts(INT_RX);
		}
	}
	else
	{
		LOGRX("data_read: Attempt to read out character from empty FIFO\n");
		logerror("data_read: Attempt to read out character from empty FIFO\n");
	}

	LOGRX("%s <- %02x [%c]\n", FUNCNAME, isascii(data) ? data : ' ', data);
	return data;
}

void mpcc_device::do_rivnr(uint8_t data)
{
	LOG("%s -> %02x\n", FUNCNAME, data);
	m_rivnr = data;
	LOGSETUP(" - Rx Int vector: %02x\n", m_tivnr);
}

uint8_t mpcc_device::do_rivnr()
{
	uint8_t data = m_rivnr;
	LOG("%s <- %02x\n", FUNCNAME, data);
	return data;
}

void mpcc_device::do_rier(uint8_t data)
{
	LOG("%s -> %02x\n", FUNCNAME, data);
	m_rier = data;
	LOGSETUP(" - Rx INT on Rx data available    : %s\n", (m_rier & REG_RIER_RDA)   ? "enabled" : "disabled");
	LOGSETUP(" - Rx INT on End of Frame         : %s\n", (m_rier & REG_RIER_EOF)   ? "enabled" : "disabled");
	LOGSETUP(" - Rx INT on CRC/Parity error     : %s\n", (m_rier & REG_RIER_CPERR) ? "enabled" : "disabled");
	LOGSETUP(" - Rx INT on Frame error          : %s\n", (m_rier & REG_RIER_FRERR) ? "enabled" : "disabled");
	LOGSETUP(" - Rx INT on Receiver overrun     : %s\n", (m_rier & REG_RIER_ROVRN) ? "enabled" : "disabled");
	LOGSETUP(" - Rx INT on Abort/Break          : %s\n", (m_rier & REG_RIER_RAB)   ? "enabled" : "disabled");
	update_interrupts(INT_RX);
}

uint8_t mpcc_device::do_rier()
{
	uint8_t data = m_rier;
	LOG("%s <- %02x\n", FUNCNAME, data);
	return data;
}

void mpcc_device::do_tdr(uint8_t data)
{
	LOG("%s -> %d [%c]\n", FUNCNAME, data, isprint(data) ? data : ' ');
	// Check of Tx fifo has room
	if (m_tx_data_fifo.full())
	{
		logerror("- TX FIFO is full, discarding data\n");
		LOGTX("- TX FIFO is full, discarding data\n");
	}
	else // ..there is still room
	{
		m_tx_data_fifo.enqueue(data);
		if (m_tx_data_fifo.full())
		{
			m_tsr &= ~REG_TSR_TDRA; // Mark fifo as full
			update_interrupts(INT_TX_TDRA);
		}
	}

	// Check if Tx is enabled
	if (m_tcr & REG_TCR_TEN)
	{
		LOGTX("- TX is enabled\n");
		if (is_transmit_register_empty()) // Is the shift register loaded?
		{
			LOGTX("- Setting up transmitter\n");
			transmit_register_setup(m_tx_data_fifo.dequeue()); // Load the shift register, reload is done in tra_complete()
			m_tsr |= REG_TSR_TDRA; // Now there is a slot in the FIFO available again
			if (m_tier & REG_TIER_TDRA)
			{
				trigger_interrupt(INT_TX_TDRA);
			}
		}
		else
		{
			LOGTX("- Transmitter not empty\n");
		}
	}
}

void mpcc_device::do_tsr(uint8_t data)
{
	LOGINT("%s -> %02x\n", FUNCNAME, data);
	// writing 1 resets status bits except for TDRA which is read-only
	m_tsr &= ~data | REG_TSR_TDRA;
	update_interrupts(INT_TX);
}

uint8_t mpcc_device::do_tsr()
{
	uint8_t data = m_tsr;
	LOGR("%s <- %02x\n", FUNCNAME, data);
	return data;
}

void mpcc_device::do_tcr(uint8_t data)
{
	LOG("%s -> %02x\n", FUNCNAME, data);
	m_tcr = data;
	LOGSETUP(" - Tx                : %s\n", (m_tcr & REG_TCR_TEN)    ? "enabled" : "disabled");
	LOGSETUP(" - Tx DMA            : %s\n", (m_tcr & REG_TCR_TDSREN) ? "enabled" : "disabled");
	LOGSETUP(" - Tx Idle character : %s\n", (m_tcr & REG_TCR_TICS)   ? "AR2"     : "high");
	LOGSETUP(" - Tx Half Word next : %s\n", (m_tcr & REG_TCR_THW)    ? "yes"     : "no");
	LOGSETUP(" - Tx Last character : %s\n", (m_tcr & REG_TCR_TLAST)  ? "yes"     : "no");
	LOGSETUP(" - Tx SYN            : %s\n", (m_tcr & REG_TCR_TSYN)   ? "enabled" : "disabled");
	LOGSETUP(" - Tx Abort command  : %s\n", (m_tcr & REG_TCR_TABT)   ? "active"  : "inactive");
	LOGSETUP(" - Tx Mode           : %s\n", (m_tcr & REG_TCR_TRES)   ? "reset"   : "normal");

	update_serial();
}

uint8_t mpcc_device::do_tcr()
{
	uint8_t data = m_tcr;
	LOG("%s <- %02x\n", FUNCNAME, data);
	return data;
}

void mpcc_device::do_tivnr(uint8_t data)
{
	LOG("%s -> %02x\n", FUNCNAME, data);
	m_tivnr = data;
	LOGSETUP(" - Tx Int vector: %02x\n", m_tivnr);
}

uint8_t mpcc_device::do_tivnr()
{
	uint8_t data = m_tivnr;
	LOG("%s <- %02x\n", FUNCNAME, data);
	return data;
}

void mpcc_device::do_tier(uint8_t data)
{
	LOG("%s -> %02x\n", FUNCNAME, data);
	m_tier = data;
	LOGSETUP(" - Tx INT on FIFO slot available  : %s\n", (m_tier & REG_TIER_TDRA) ? "enabled" : "disabled");
	LOGSETUP(" - Tx INT on Frame complete       : %s\n", (m_tier & REG_TIER_TFC ) ? "enabled" : "disabled");
	LOGSETUP(" - Tx INT on Underrun             : %s\n", (m_tier & REG_TIER_TUNRN) ? "enabled" : "disabled");
	LOGSETUP(" - Tx INT on Frame error          : %s\n", (m_tier & REG_TIER_TFERR) ? "enabled" : "disabled");
	update_interrupts(INT_TX);
}

uint8_t mpcc_device::do_tier()
{
	uint8_t data = m_tier;
	LOG("%s <- %02x\n", FUNCNAME, data);
	return data;
}

void mpcc_device::do_sisr(uint8_t data)
{
	LOG("%s -> %02x\n", FUNCNAME, data);
	if (data & REG_SISR_CTST) m_sisr &= ~REG_SISR_CTST;
	if (data & REG_SISR_DSRT) m_sisr &= ~REG_SISR_DSRT;
	if (data & REG_SISR_DCDT) m_sisr &= ~REG_SISR_DCDT;
	update_interrupts(INT_SR);

	LOGSETUP(" - CTS %d transitioned: %d\n", (m_sisr & REG_SISR_CTSLVL) ? 1 :0, (m_sisr & REG_SISR_CTST) ? 1 : 0);
	LOGSETUP(" - DSR %d transitioned: %d\n", (m_sisr & REG_SISR_DSRLVL) ? 1 :0, (m_sisr & REG_SISR_DSRT) ? 1 : 0);
	LOGSETUP(" - DCD %d transitioned: %d\n", (m_sisr & REG_SISR_DCDLVL) ? 1 :0, (m_sisr & REG_SISR_DCDT) ? 1 : 0);
}

uint8_t mpcc_device::do_sisr()
{
	uint8_t data = m_sisr;
	LOG("%s <- %02x\n", FUNCNAME, data);
	return data;
}

void mpcc_device::do_sicr(uint8_t data)
{
	LOG("%s -> %02x\n", FUNCNAME, data);

	// If RTS is activated the RTS output latch can only be reset by an empty FIFO.
	if ( !(m_sicr & REG_SICR_RTSLVL) &&
		 (data & REG_SICR_RTSLVL))
	{
		m_out_rts_cb(ASSERT_LINE); // TODO: respect the RTSLV bit
	}

	m_sicr = data;

	if (m_sicr & REG_SICR_DTRLVL)
	{
		m_out_dtr_cb(ASSERT_LINE);
	}
	else
	{
		m_out_dtr_cb(CLEAR_LINE);
	}

	LOGSETUP(" - RTS level : %s\n", (m_sicr & REG_SICR_RTSLVL) ? "high" : "low");
	LOGSETUP(" - DTR level : %s\n", (m_sicr & REG_SICR_DTRLVL) ? "high" : "low");
	LOGSETUP(" - Echo Mode : %s\n", (m_sicr & REG_SICR_ECHO)   ? "enabled" : "disabled");
	LOGSETUP(" - Test Mode : %s\n", (m_sicr & REG_SICR_TEST)   ? "enabled" : "disabled");
}

uint8_t mpcc_device::do_sicr()
{
	uint8_t data = m_sicr;
	LOG("%s <- %02x\n", FUNCNAME, data);
	return data;
}

void mpcc_device::do_sier(uint8_t data)
{
	LOG("%s -> %02x\n", FUNCNAME, data);
	m_sier = data;
	LOGSETUP(" - Serial interface INT on CTS: %s\n", (m_sier & REG_SIER_CTS) ? "enabled" : "disabled");
	LOGSETUP(" - Serial interface INT on DSR: %s\n", (m_sier & REG_SIER_DSR) ? "enabled" : "disabled");
	LOGSETUP(" - Serial interface INT on DCD: %s\n", (m_sier & REG_SIER_DCD) ? "enabled" : "disabled");
	update_interrupts(INT_SR);
}

uint8_t mpcc_device::do_sier()
{
	uint8_t data = m_sier;
	LOG("%s <- %02x\n", FUNCNAME, data);
	return data;
}

void mpcc_device::do_psr1(uint8_t data)
{
	LOG("%s -> %02x\n", FUNCNAME, data);
	m_psr1 = data;
	LOGSETUP(" - Zero Address option: %s\n", (m_psr1 & REG_PSR1_ADRZ)  ? "enabled" : "disabled" );
	LOGSETUP(" - IPARS option       : %s\n", (m_psr1 & REG_PSR1_IPARS) ? "enabled" : "disabled" );
	LOGSETUP(" - Control Field Width: %s\n", (m_psr1 & REG_PSR1_CTLEX) ? "16 bit"  : "8 bit" );
	LOGSETUP(" - Address Extend     : %s\n", (m_psr1 & REG_PSR1_ADDEX) ? "enabled" : "disabled" );

	update_serial();
}

uint8_t mpcc_device::do_psr1()
{
	uint8_t data = m_psr1;
	LOG("%s <- %02x\n", FUNCNAME, data);
	return data;
}

void mpcc_device::do_psr2(uint8_t data)
{
	LOG("%s -> %02x\n", FUNCNAME, data);
	m_psr2 = data;
	LOGSETUP(" - %s data bus\n", (m_psr2 & REG_PSR2_WDBYT) ? "16 bit (not implemented)" : "8 bit" );
	LOGSETUP(" - %s stop bits\n",(m_psr2 & REG_PSR2_STP_MSK) == REG_PSR2_STP_1 ? "1" :
		( (m_psr2 & REG_PSR2_STP_MSK) == REG_PSR2_STP_1_5 ? "1.5" :
		  ( (m_psr2 & REG_PSR2_STP_MSK) == REG_PSR2_STP_2 ? "2" : "Unknown")));
	LOGSETUP(" - %d bit characters\n", 5 + ((m_psr2 & REG_PSR2_CHLN_MSK) >> 3));
	LOGSETUP(" - Protocol %d %s\n", m_psr2 & REG_PSR2_PSEL_MSK, (m_psr2 & REG_PSR2_PSEL_MSK) != REG_PSR2_PSEL_ASCII ? "(not implemented)" : "");

	update_serial();
}

uint8_t mpcc_device::do_psr2()
{
	uint8_t data = m_psr2;
	LOG("%s <- %02x\n", FUNCNAME, data);
	return data;
}

/*
 * Clocks and Baud Rates
 */
void mpcc_device::do_brdr1(uint8_t data)
{
	LOG("%s -> %02x\n", FUNCNAME, data);
	m_brdr1 = data;
	LOGSETUP(" - Baudrate Divider 1: %02x\n", m_brdr1);

	update_serial();
}

uint8_t mpcc_device::do_brdr1()
{
	uint8_t data = m_brdr1;
	LOG("%s <- %02x\n", FUNCNAME, data);
	return data;
}

void mpcc_device::do_brdr2(uint8_t data)
{
	LOG("%s -> %02x\n", FUNCNAME, data);
	m_brdr2 = data;
	LOGSETUP(" - Baudrate Divider 2: %02x\n", m_brdr2);

	update_serial();
}

uint8_t mpcc_device::do_brdr2()
{
	uint8_t data = m_brdr2;
	LOG("%s <- %02x\n", FUNCNAME, data);
	return data;
}

void mpcc_device::do_ccr(uint8_t data)
{
	LOG("%s -> %02x\n", FUNCNAME, data);
	m_ccr = data;
	LOGSETUP(" - Prescaler: x%d\n", (m_ccr & REG_CCR_PSCDIV) ? 3 : 2);
	LOGSETUP(" - TxC used as: %s\n", (m_ccr & REG_CCR_TCLO) ? "output" : "input");
	LOGSETUP(" - RxC taken from: %s source, (ASYNC mode only)\n", (m_ccr & REG_CCR_TCLO) ? "internal" : "external");
	LOGSETUP(" - External RxC divisor: x%d\n",(m_ccr & REG_CCR_CLKDIV_MSK) == REG_CCR_CLKDIV_X1 ? 1 :
			 ( (m_ccr & REG_CCR_CLKDIV_MSK) == REG_CCR_CLKDIV_X16 ? 16 :
			   ( (m_ccr & REG_CCR_CLKDIV_MSK) == REG_CCR_CLKDIV_X32 ? 32 : 64)));

	update_serial();
}

uint8_t mpcc_device::do_ccr()
{
	uint8_t data = m_ccr;
	LOG("%s <- %02x\n", FUNCNAME, data);
	return data;
}

void mpcc_device::do_ecr(uint8_t data)
{
	LOG("%s -> %02x\n", FUNCNAME, data);
	m_ecr = data;
	LOGSETUP(" - Parity         : %s\n", (m_ecr & REG_ECR_PAREN) ? "enabled" : "disabled");
	LOGSETUP(" - Parity         : %s\n", (m_ecr & REG_ECR_ODDPAR) ? "odd" : "even");
	LOGSETUP(" - CRC            : %s\n", (m_ecr & REG_ECR_CFCRC) ? "enabled" : "disabled");
	LOGSETUP(" - CRC Polynominal: %s\n", (m_ecr & REG_ECR_CRCSEL_MSK) == REG_ECR_CRCSEL_V41 ? "CCITT V.41 (BOP)" :
			 ( (m_ecr & REG_ECR_CRCSEL_MSK) == REG_ECR_CRCSEL_C16 ? "CRC-16 (BSC)" :
			   ( (m_ecr & REG_ECR_CRCSEL_MSK) == REG_ECR_CRCSEL_VRC ? "VRC/LRC (BSC, ASCII, non-transp)" :
				 "Not used")));

	update_serial();
}

uint8_t mpcc_device::do_ecr()
{
	uint8_t data = m_ecr;
	LOG("%s <- %02x\n", FUNCNAME, data);
	return data;
}
