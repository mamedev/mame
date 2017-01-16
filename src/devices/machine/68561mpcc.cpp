// license:BSD-3-Clause copyright-holders: Joakim Larsson Edstrom
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
#define LOG_GENERAL 0x001
#define LOG_SETUP   0x002
#define LOG_PRINTF  0x004
#define LOG_READ    0x008
#define LOG_INT     0x010
#define LOG_CMD     0x020
#define LOG_TX      0x040
#define LOG_RCV     0x080
#define LOG_CTS     0x100
#define LOG_DCD     0x200
#define LOG_SYNC    0x400
#define LOG_CHAR    0x800

#define VERBOSE 0 // (LOG_PRINTF | LOG_SETUP  | LOG_GENERAL)

#define LOGMASK(mask, ...)   do { if (VERBOSE & mask) logerror(__VA_ARGS__); } while (0)
#define LOGLEVEL(mask, level, ...) do { if ((VERBOSE & mask) >= level) logerror(__VA_ARGS__); } while (0)

#define LOG(...)      LOGMASK(LOG_GENERAL, __VA_ARGS__)
#define LOGSETUP(...) LOGMASK(LOG_SETUP,   __VA_ARGS__)
#define LOGR(...)     LOGMASK(LOG_READ,    __VA_ARGS__)
#define LOGINT(...)   LOGMASK(LOG_INT,     __VA_ARGS__)
#define LOGCMD(...)   LOGMASK(LOG_CMD,     __VA_ARGS__)
#define LOGTX(...)    LOGMASK(LOG_TX,      __VA_ARGS__)
#define LOGRCV(...)   LOGMASK(LOG_RCV,     __VA_ARGS__)
#define LOGCTS(...)   LOGMASK(LOG_CTS,     __VA_ARGS__)
#define LOGDCD(...)   LOGMASK(LOG_DCD,     __VA_ARGS__)
#define LOGSYNC(...)  LOGMASK(LOG_SYNC,    __VA_ARGS__)
#define LOGCHAR(...)  LOGMASK(LOG_CHAR,    __VA_ARGS__)

#if VERBOSE & LOG_PRINTF
#define logerror printf
#endif

#ifdef _MSC_VER
#define FUNCNAME __func__
#define LLFORMAT "%I64d"
#else
#define FUNCNAME __PRETTY_FUNCTION__
#define LLFORMAT "%lld"
#endif

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************
// device type definition
const device_type MPCC       = &device_creator<mpcc_device>;
const device_type MPCC68560  = &device_creator<mpcc68560_device>;
const device_type MPCC68560A = &device_creator<mpcc68560A_device>;
const device_type MPCC68561  = &device_creator<mpcc68561_device>;
const device_type MPCC68561A = &device_creator<mpcc68561A_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

mpcc_device::mpcc_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, uint32_t variant, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	  device_serial_interface(mconfig, *this),
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
	: device_t(mconfig, MPCC, "Rockwell MPCC", tag, owner, clock, "mpcc", __FILE__),
	  device_serial_interface(mconfig, *this),
	  m_variant(TYPE_MPCC),
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

mpcc68560_device::mpcc68560_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mpcc_device(mconfig, MPCC68560, "MPCC 68560", tag, owner, clock, TYPE_MPCC68560, "mpcc68560", __FILE__){ }

mpcc68560A_device::mpcc68560A_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mpcc_device(mconfig, MPCC68560A, "MPCC 68560A", tag, owner, clock, TYPE_MPCC68560A, "mpcc68560A", __FILE__){ }

mpcc68561_device::mpcc68561_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mpcc_device(mconfig, MPCC68561, "MPCC 68561", tag, owner, clock, TYPE_MPCC68561, "mpcc68561", __FILE__){ }

mpcc68561A_device::mpcc68561A_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mpcc_device(mconfig, MPCC68561A, "MPCC 68561A", tag, owner, clock, TYPE_MPCC68561A, "mpcc68561A", __FILE__){ }

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

	m_rsr 	= 0x00;
	m_rcr 	= 0x01;
	m_rivnr = 0x0f;
	m_rier 	= 0x00;
	m_tsr 	= 0x80;
	m_tcr 	= 0x01;
	m_tivnr = 0x0f;
	m_tier 	= 0x00;
	m_sisr 	= 0x00;
	m_sicr 	= 0x00;
	m_sivnr = 0x0f;
	m_sier 	= 0x00;
	m_psr1 	= 0x00;
	m_psr2 	= 0x00;
	m_ar1 	= 0x00;
	m_ar2 	= 0x00;
	m_brdr1 = 0x01;
	m_brdr2 = 0x00;
	m_ccr 	= 0x00;
	m_ecr 	= 0x04;

	// Init out callbacks to known inactive state
	m_out_txd_cb(1);
	m_out_dtr_cb(1);
	m_out_rts_cb(1);
	m_out_rtxc_cb(1);
	m_out_trxc_cb(1);
	m_out_int_cb(1);
}

/*
 * Serial device implementation
 */
//-------------------------------------------------
//  tra_callback - is called for each bit that needs to be transmitted
//-------------------------------------------------
void mpcc_device::tra_callback()
{
	// Check if transmitter is idle as in disabled
	if (!(m_tcr & REG_TCR_TEN))
	{
		// transmit idle TODO: Support TCR TICS bit
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

		// transmit data
		m_out_txd_cb(db);
	}
	// Otherwise we don't know why we are called...
	else
	{
		logerror("%s %s Failed to transmit\n", FUNCNAME, m_owner->tag());
	}
}

//-------------------------------------------------
//  tra_complete - is called when the transmitter shift register has sent the last bit
//-------------------------------------------------
void mpcc_device::tra_complete()
{
	// check if transmitter is enabled and we are not sending BREAK level
	if ((m_tcr & REG_TCR_TEN) && !(m_tcr & REG_TCR_TICS))
	{	// check if there are more data in the fifo
		if (!m_tx_data_fifo.empty())
		{
			transmit_register_setup(m_tx_data_fifo.dequeue()); // Reload the shift register
			m_tsr |= REG_TSR_TDRA; // Mark fifo as having room for more data
		}
		else
		{
			m_out_rts_cb(CLEAR_LINE); // TODO: respect the RTSLV bit
		}

		// Check if Tx interrupts are enabled
		if (m_tier & REG_TIER_TDRA)
		{
			// TODO: Check circumstances, eg int on first or every character etc
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
//  rcv_callback - called when it is time to sample incomming data bit
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

	//	receive_data(data);
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
		m_rx_data_fifo.enqueue(data);
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
	// If we are not serving any interrupt the IRQ is asserted already and we need to do nothing
	if ((state & INT_ACK) == 0)
	{
		// If there is a new interrupt not yet acknowledged IRQ needs to be asserted
		if (state & INT_REQ)
		{
			m_out_int_cb(ASSERT_LINE);
		}
		// Otherwise we just clear the IRQ line allowing other devices to interrupt
		else
		{
			m_out_int_cb(CLEAR_LINE);
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

//-------------------------------------------------
//  Read register
//-------------------------------------------------
READ8_MEMBER( mpcc_device::read )
{
	uint8_t data = 0;

	switch(offset)
	{
	case 0x00: data = m_rsr; logerror("MPCC: Reg RSR not implemented\n"); break;
	case 0x01: data = m_rcr; logerror("MPCC: Reg RCR not implemented\n"); break;
	case 0x02: data = m_rdr; logerror("MPCC: Reg RDR not implemented\n"); break;
	case 0x04: data = m_rivnr; logerror("MPCC: Reg RIVNR not implemented\n"); break;
	case 0x05: data = m_rier; logerror("MPCC: Reg RIER not implemented\n"); break;
	case 0x08: data = m_tsr; break; logerror("MPCC: Reg TSR not implemented\n"); break;
	case 0x09: data = m_tcr; logerror("MPCC: Reg TCR not implemented\n"); break;
	//case 0x0a: data = m_tdr; break;
	case 0x0c: data = m_tivnr; logerror("MPCC: Reg TIVNR not implemented\n"); break;
	case 0x0d: data = m_tier; logerror("MPCC: Reg TIER not implemented\n"); break;
	case 0x10: data = m_sisr; logerror("MPCC: Reg SISR not implemented\n"); break;
	case 0x11: data = m_sicr; logerror("MPCC: Reg SICR not implemented\n"); break;
	case 0x14: data = m_sivnr; logerror("MPCC: Reg SIVNR not implemented\n"); break;
	case 0x15: data = m_sier; logerror("MPCC: Reg SIER not implemented\n"); break;
	case 0x18: data = m_psr1; logerror("MPCC: Reg PSR1 not implemented\n"); break;
	case 0x19: data = m_psr2; logerror("MPCC: Reg PSR2 not implemented\n"); break;
	case 0x1a: data = m_ar1; logerror("MPCC: Reg AR1 not implemented\n"); break;
	case 0x1b: data = m_ar2; logerror("MPCC: Reg AR2 not implemented\n"); break;
	case 0x1c: data = m_brdr1; logerror("MPCC: Reg BRDR1 not implemented\n"); break;
	case 0x1d: data = m_brdr2; logerror("MPCC: Reg BRDR2 not implemented\n"); break;
	case 0x1e: data = m_ccr; logerror("MPCC: Reg CCR not implemented\n"); break;
	case 0x1f: data = m_ecr; logerror("MPCC: Reg ECR not implemented\n"); break;
	default: logerror("%s invalid register accessed: %02x\n", m_owner->tag(), offset);
	}
	LOGSETUP(" * %s Reg %02x -> %02x  \n", m_owner->tag(), offset, data);
	return data;
}

//-------------------------------------------------
//  Write register
//-------------------------------------------------
WRITE8_MEMBER( mpcc_device::write )
{
	LOGSETUP(" * %s Reg %02x <- %02x  \n", m_owner->tag(), offset, data);
	switch(offset)
	{
	case 0x00: m_rsr = data; logerror("MPCC: Reg RSR not implemented\n"); break;
	case 0x01: m_rcr = data; logerror("MPCC: Reg RCR not implemented\n"); break;
	//case 0x02: m_rdr = data; break;
	case 0x04: m_rivnr = data; logerror("MPCC: Reg RIVNR not implemented\n"); break;
	case 0x05: m_rier = data; logerror("MPCC: Reg RIER not implemented\n"); break;
	case 0x08: m_tsr = data; logerror("MPCC: Reg TSR not implemented\n"); break;
	case 0x09: m_tcr = data; logerror("MPCC: Reg TCR not implemented\n"); break;
	case 0x0a: m_tdr = data; LOGCHAR("*%c", data); do_tdr_w(data); break;
	case 0x0c: m_tivnr = data; logerror("MPCC: Reg TIVNR not implemented\n"); break;
	case 0x0d: m_tier = data; logerror("MPCC: Reg TIER not implemented\n"); break;
	case 0x10: m_sisr = data; logerror("MPCC: Reg SISR not implemented\n"); break;
	case 0x11: m_sicr = data; logerror("MPCC: Reg SICR not implemented\n"); break;
	case 0x14: m_sivnr = data; logerror("MPCC: Reg SIVNR not implemented\n"); break;
	case 0x15: m_sier = data; logerror("MPCC: Reg SIER not implemented\n"); break;
	case 0x18: m_psr1 = data; logerror("MPCC: Reg PSR1 not implemented\n"); break;
	case 0x19: m_psr2 = data; logerror("MPCC: Reg PSR2 not implemented\n"); break;
	case 0x1a: m_ar1 = data; logerror("MPCC: Reg AR1 not implemented\n"); break;
	case 0x1b: m_ar2 = data; logerror("MPCC: Reg AR2 not implemented\n"); break;
	case 0x1c: m_brdr1 = data; logerror("MPCC: Reg BRDR1 not implemented\n"); break;
	case 0x1d: m_brdr2 = data; logerror("MPCC: Reg BRDR2 not implemented\n"); break;
	case 0x1e: m_ccr = data; logerror("MPCC: Reg CCR not implemented\n"); break;
	case 0x1f: m_ecr = data; logerror("MPCC: Reg ECR not implemented\n"); break;
	default: logerror("%s invalid register accessed: %02x\n", m_owner->tag(), offset);
	}
}

void mpcc_device::do_tdr_w(uint8_t data)
{
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
		}
	}
}
