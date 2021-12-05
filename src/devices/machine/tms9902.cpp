// license:BSD-3-Clause
// copyright-holders:Michael Zapf
/****************************************************************************

    TMS9902 Asynchronous Communication Controller

    TMS9902 is an asynchronous serial controller for use with the TI990 and
    TMS9900 family.  It provides serial I/O, three extra I/O pins (namely RTS,
    DSR and CTS), and a timer.  It communicates with the CPU through the CRU
    I/O bus, and one interrupt pin.

               +----+--+----+
     <-   /INT |1   \--/  18| VCC
     <-   XOUT |2         17| /CE     <-
     ->    RIN |3         16| /PHI    <-
     <-  CRUIN |4         15| CRUCLK  <-
     <-   /RTS |5         14| S0      <-
     ->   /CTS |6         13| S1      <-
     ->   /DSR |7         12| S2      <-
     -> CRUOUT |8         11| S3      <-
           VSS |9         10| S4      <-
               +------------+

     The CRUIN line borrows its name from the connector of the connected CPU
     where it is an input, so CRUIN is an output of this chip. The same is true
     for CRUOUT.

     /PHI is a TTL clock input with 4 MHz maximum rate.

    IMPORTANT NOTE: The previous versions of TMS9902 attempted to write their
    output to a file. This implementation is able to communicate with an external
    UART via a socket connection and an external bridge. However, the work is
    not done yet, and until then the file writing is disabled.

    Raphael Nabet, 2003
    Michael Zapf, 2011
    February 2012: Rewritten as class

*****************************************************************************/

#include "emu.h"
#include "tms9902.h"

#include <cmath>

#define LOG_GENERAL (1U << 0)
#define LOG_LINES   (1U << 1)
#define LOG_CRU     (1U << 2)
#define LOG_DETAIL  (1U << 3)
#define LOG_BUFFER  (1U << 4)
#define LOG_ERROR   (1U << 5)
#define LOG_SETTING (1U << 6)

#define VERBOSE (LOG_ERROR)
#include "logmacro.h"

#define LOGGENERAL(...)     LOGMASKED(LOG_GENERAL, __VA_ARGS__)
#define LOGLINES(...)       LOGMASKED(LOG_LINES, __VA_ARGS__)
#define LOGCRU(...)         LOGMASKED(LOG_CRU, __VA_ARGS__)
#define LOGDETAIL(...)      LOGMASKED(LOG_DETAIL, __VA_ARGS__)
#define LOGBUFFER(...)      LOGMASKED(LOG_BUFFER, __VA_ARGS__)
#define LOGERROR(...)       LOGMASKED(LOG_ERROR, __VA_ARGS__)
#define LOGSETTING(...)     LOGMASKED(LOG_SETTING, __VA_ARGS__)


enum
{
	DECTIMER,
	RECVTIMER,
	SENDTIMER
};

// Polling frequency. We use a much higher value to allow for line state changes
// happening between character transmissions (which happen in parallel in real
// communications but which must be serialized here)
#define POLLING_FREQ 20000


/*
    Constructor
*/
tms9902_device::tms9902_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TMS9902, tag, owner, clock)
	, m_int_cb(*this)
	, m_rcv_cb(*this)
	, m_xmit_cb(*this)
	, m_ctrl_cb(*this)
{
}

/*
    should be called after any change to int_state or enabled_ints.
*/
void tms9902_device::field_interrupts()
{
	bool const new_int = (m_DSCH && m_DSCENB)
							|| (m_RBRL && m_RIENB)
							|| (m_XBRE && m_XBIENB)
							|| (m_TIMELP && m_TIMENB);
	LOGDETAIL("interrupt flags (DSCH = %02x, DSCENB = %02x), (RBRL = %02x, RIENB = %02x), (XBRE = %02x, XBIENB = %02x), (TIMELP = %02x, TIMENB = %02x)\n",
			m_DSCH, m_DSCENB, m_RBRL, m_RIENB, m_XBRE, m_XBIENB, m_TIMELP, m_TIMENB);

	if (new_int != m_INT)
	{
		// Only consider edges
		m_INT = new_int;
		LOGLINES("/INT = %s\n", m_INT ? "asserted" : "cleared");
		m_int_cb(m_INT ? ASSERT_LINE : CLEAR_LINE);
	}
}

/*
    Called whenever the incoming CTS* line changes. This should be called by
    the device that contains the UART.
*/
void tms9902_device::rcv_cts(line_state state)
{
	bool previous = m_CTSin;

	// CTSin is an internal register of the TMS9902 with positive logic
	m_CTSin = (state==ASSERT_LINE);

	LOGLINES("CTS* = %s\n", (state==ASSERT_LINE)? "asserted" : "cleared");

	if (m_CTSin != previous)
	{
		m_DSCH = true;
		field_interrupts();

		// If CTS becomes asserted and we have been sending
		if (state==ASSERT_LINE && m_RTSout)
		{
			// and if the byte buffer is empty
			if (m_XBRE)
			{
				// and we want to have a BRK, send it
				if (m_BRKON) send_break(true);
			}
			else
			{
				// Buffer is not empty, we can send it
				// If the shift register is empty, transfer the data
				if (m_XSRE && !m_BRKout)
				{
					initiate_transmit();
				}
			}
		}
	}
	else
	{
		m_DSCH = false;
		LOGLINES("no change in CTS line, no interrupt.\n");
	}
}

void tms9902_device::set_clock(bool state)
{
	if (state)
		m_recvtimer->adjust(attotime::from_msec(1), 0, attotime::from_hz(POLLING_FREQ));
	else
		m_recvtimer->reset();
}

/*
    Called whenever the incoming DSR* line changes. This should be called by
    the device that contains the UART.
*/
void tms9902_device::rcv_dsr(line_state state)
{
	bool previous = m_DSRin;
	LOGLINES("DSR* = %s\n", (state==ASSERT_LINE)? "asserted" : "cleared");
	m_DSRin = (state==ASSERT_LINE);

	if (m_DSRin != previous)
	{
		m_DSCH = true;
		field_interrupts();
	}
	else
	{
		m_DSCH = false;
		LOGLINES("no change in DSR line, no interrupt.\n");
	}
}

/*
    Called whenever the incoming RIN line changes. This should be called by
    the device that contains the UART. Unlike the real thing, we deliver
    complete bytes in one go.
*/
void tms9902_device::rcv_data(uint8_t data)
{
	// Put the received byte into the 1-byte receive buffer
	m_RBR = data;

	// Clear last errors
	m_RFER = false;
	m_RPER = false;

	if (!m_RBRL)
	{
		// Receive buffer was empty
		m_RBRL = true;
		m_ROVER = false;
		LOGBUFFER("Receive buffer loaded with byte %02x; RIENB=%d\n", data, m_RIENB);
		field_interrupts();
	}
	else
	{
		// Receive buffer was full
		m_ROVER = true;
		LOGERROR("Receive buffer still loaded; overflow error\n");
	}
}

//------------------------------------------------

/*
    Framing error. This can only be detected by a remotely attached real UART;
    if we get a report on a framing error we use it to announce the framing error
    as if it occurred here.
    The flag is reset by the next correctly received byte.
*/
void tms9902_device::rcv_framing_error()
{
	LOGERROR("Detected framing error\n");
	m_RFER = true;
}

/*
    Parity error. This can only be detected by a remotely attached real UART;
    if we get a report on a parity error we use it to announce the parity error
    as if it occurred here.
    The flag is reset by the next correctly received byte.
*/
void tms9902_device::rcv_parity_error()
{
	LOGERROR("Detected parity error\n");
	m_RPER = true;
}

/*
    Incoming BREAK condition. The TMS9902 does not show any directly visible
    reactions on a BREAK (no interrupt, no flag set). A BREAK is a time period
    of low level on the RIN pin which makes the chip re-synchronize on the
    next rising edge.
*/
void tms9902_device::rcv_break(bool value)
{
	LOGERROR("Receive BREAK=%d (no effect)\n", value? 1:0);
}

//------------------------------------------------

/*
    Timer callback
*/
void tms9902_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	// This call-back is called by the MAME timer system when the decrementer
	// reaches 0.
	case DECTIMER:
		m_TIMERR = m_TIMELP;
		m_TIMELP = true;
		field_interrupts();
		break;

	//  Callback for the autonomous operations of the chip. This is normally
	//  controlled by an external clock of 3-4 MHz, internally divided by 3 or 4,
	//  depending on CLK4M. With this timer, reception of characters becomes
	//  possible.
	case RECVTIMER:
		m_rcv_cb(ASSERT_LINE);
		break;

	case SENDTIMER:
		// Byte has been sent
		m_XSRE = true;

		// In the meantime, the CPU may have pushed a new byte into the XBR
		// so we loop until all data are transferred
		if (!m_XBRE && m_CTSin)
		{
			initiate_transmit();
		}
		break;
	}
}

/*
    load the content of clockinvl into the decrementer
*/
void tms9902_device::reload_interval_timer()
{
	if (m_TMR)
	{   /* reset clock interval */
		m_dectimer->adjust(
						attotime::from_double((double) m_TMR / (m_clock_rate / ((m_CLK4M) ? 4. : 3.) / 64.)),
						0,
						attotime::from_double((double) m_TMR / (m_clock_rate / ((m_CLK4M) ? 4. : 3.) / 64.)));
	}
	else
	{   /* clock interval == 0 -> no timer */
		m_dectimer->enable(0);
	}
}

void tms9902_device::send_break(bool state)
{
	if (state != m_BRKout)
	{
		m_BRKout = state;
		LOGLINES("Sending BREAK=%d\n", state? 1:0);

		// Signal BRK (on/off) to the remote site
		m_ctrl_cb((offs_t)(EXCEPT | BRK), state? 1:0);
	}
}

/*
    Baudpoll value allows the callback function to know when the next data byte shall be delivered.
*/
double tms9902_device::get_baudpoll()
{
	return m_baudpoll;
}

// ==========================================================================

/*
    Sets the data rate for the receiver part. If a remote UART is attached,
    propagate this setting.
    The TMS9902 calculates the baud rate from the external clock, and the result
    does not match the known baud rates precisely (e.g. for 9600 baud the
    closest value is 9615). Other UARTs may have a different way to set baud
    rates. Thus we transmit the bit pattern and leave it up to the remote UART
    to calculate its own baud rate from it. Apart from that, the callback
    function should add information about the UART.

    CLK4M RDV8 RDR9 RDR8 | RDR7 RDR6 RDR5 RDR4 | RDR3 RDR2 RDR1 RDR0
*/
void tms9902_device::set_receive_data_rate()
{
	int value = (m_CLK4M? 0x800 : 0) | (m_RDV8? 0x400 : 0) | m_RDR;
	LOGSETTING("receive rate = %04x\n", value);

	// Calculate the ratio between receive baud rate and polling frequency
	double fint = m_clock_rate / ((m_CLK4M) ? 4.0 : 3.0);
	double baud = fint / (2.0 * ((m_RDV8)? 8:1) * m_RDR);

	// We assume 10 bit per character (7 data usually add 1 parity; 1 start, 1 stop)
	// This value represents the ratio of data inputs of one poll.
	// Thus the callback function should add up this value on each poll
	// and deliver a data input not before it sums up to 1.
	m_baudpoll = (double)(baud / (10*POLLING_FREQ));
	LOGSETTING("baudpoll = %f\n", m_baudpoll);

	m_last_config_value = value;
	m_ctrl_cb((offs_t)CONFIG, RATERECV);
}

/*
    Sets the data rate for the sender part. If a remote UART is attached,
    propagate this setting.
*/
void tms9902_device::set_transmit_data_rate()
{
	int value = (m_CLK4M? 0x800 : 0) | (m_XDV8? 0x400 : 0) | m_XDR;
	LOGSETTING("set transmit rate = %04x\n", value);
	m_last_config_value = value;
	m_ctrl_cb((offs_t)CONFIG, RATEXMIT);
}

void tms9902_device::set_stop_bits()
{
	int value = m_STOPB;
	LOGSETTING("set stop bits = %02x\n", value);
	m_last_config_value = value;
	m_ctrl_cb((offs_t)CONFIG, STOPBITS);
}

void tms9902_device::set_data_bits()
{
	int value = m_RCL;
	LOGSETTING("set data bits = %02x\n", value);
	m_last_config_value = value;
	m_ctrl_cb((offs_t)CONFIG, DATABITS);
}

void tms9902_device::set_parity()
{
	int value = (m_PENB? 2:0) | (m_ODDP? 1:0);
	LOGSETTING("set parity = %02x\n", value);
	m_last_config_value = value;
	m_ctrl_cb((offs_t)CONFIG, PARITY);
}

void tms9902_device::transmit_line_state()
{
	// 00ab cdef = setting line RTS=a, CTS=b, DSR=c, DCD=d, DTR=e, RI=f
	// The 9902 only outputs RTS and BRK
	LOGSETTING("transmitting line state (only RTS) = %02x\n", (m_RTSout)? 1:0);
	m_last_config_value = (m_RTSout)? RTS : 0;
	m_ctrl_cb((offs_t)(LINES | RTS), RTS);
}

void tms9902_device::set_rts(line_state state)
{
	bool lstate = (state==ASSERT_LINE);

	if (lstate != m_RTSout)
	{
		// Signal RTS to the modem
		LOGSETTING("Set RTS=%d\n", lstate? 1:0);
		m_RTSout = lstate;
		transmit_line_state();
	}
}

int tms9902_device::get_config_value()
{
	return m_last_config_value;
}

// ==========================================================================

void tms9902_device::initiate_transmit()
{
	if (m_BRKON && m_CTSin)
		/* enter break mode */
		send_break(true);
	else
	{
		if (!m_RTSON && (!m_CTSin || (m_XBRE && !m_BRKout)))
			/* clear RTS output */
			set_rts(CLEAR_LINE);
		else
		{
			m_XSR = m_XBR;
			m_XSRE = false;
			m_XBRE = true;

			field_interrupts();

			LOGBUFFER("transmit XSR=%02x, RCL=%02x\n", m_XSR, m_RCL);

			m_xmit_cb((offs_t)0, m_XSR & (0xff >> (3-m_RCL)));

			// Should store that somewhere (but the CPU is fast enough, can afford to recalc :-) )
			double fint = m_clock_rate / ((m_CLK4M) ? 4.0 : 3.0);
			double baud = fint / (2.0 * ((m_RDV8)? 8:1) * m_RDR);

			// Time for transmitting 10 bit (8 bit + start + stop)
			m_sendtimer->adjust(attotime::from_hz(baud/10.0));
		}
	}
}



/*----------------------------------------------------------------
    TMS9902 CRU interface.
----------------------------------------------------------------*/

/*
    Read a 8 bit chunk from tms9902.

    signification:
    bit 0-7: RBR0-7 Receive Buffer register
    bit 8: not used (always 0)
    bit 9: RCVERR Receive Error (RFER | ROVER | RPER)
    bit 10: RPER Receive Parity Error
    bit 11: ROVER Receive Overrun Error
    bit 12: RFER Receive Framing Error
    bit 13-15: not emulated, normally used for diagnostics
    bit 16: RBINT (RBRL&RIENB)
*/
uint8_t tms9902_device::cruread(offs_t offset)
{
	uint8_t answer = 0;

	switch (offset & 31)
	{
	case 31:
		answer = m_INT;
		break;

	case 30:
		answer = (m_LDCTRL || m_LDIR || m_LRDR || m_LXDR || m_BRKON);
		break;

	case 29:
		answer = m_DSCH;
		break;

	case 28:
		answer = m_CTSin;
		break;

	case 27:
		answer = m_DSRin;
		break;

	case 26:
		answer = m_RTSout;
		break;

	case 25:
		answer = m_TIMELP;
		break;

	case 24:
		answer = m_TIMERR;
		break;

	case 23:
		answer = m_XSRE;
		break;

	case 22:
		answer = m_XBRE;
		break;

	case 21:
		answer = m_RBRL;
		break;

	case 20:
		answer = (m_DSCH && m_DSCENB);
		break;

	case 19:
		answer = (m_TIMELP && m_TIMENB);
		break;

	case 17:
		answer = (m_XBRE && m_XBIENB);
		break;

	case 16:
		answer = (m_RBRL && m_RIENB);
		break;

	case 15:
		answer = m_RIN;
		break;

	case 14:
		answer = m_RSBD;
		break;

	case 13:
		answer = m_RFBD;
		break;

	case 12:
		answer = m_RFER;
		break;

	case 11:
		answer = m_ROVER;
		break;

	case 10:
		answer = m_RPER;
		break;

	case 9:
		answer = (m_RPER || m_RFER || m_ROVER);
		break;

	case 7:
	case 6:
	case 5:
	case 4:
	case 3:
	case 2:
	case 1:
	case 0:
		answer = BIT(m_RBR, offset & 31);
		break;
	}
	if (VERBOSE & LOG_DETAIL) LOGCRU("Reading flag bits %d - %d = %02x\n", ((offset+1)*8-1), offset*8, answer);
	return answer;
}

static inline void set_bits8(uint8_t *reg, uint8_t bits, bool set)
{
	if (set)
		*reg |= bits;
	else
		*reg &= ~bits;
}

static inline void set_bits16(uint16_t *reg, uint16_t bits, bool set)
{
	if (set)
		*reg |= bits;
	else
		*reg &= ~bits;
}

void tms9902_device::reset_uart()
{
	logerror("resetting UART\n");

	/*  disable all interrupts */
	m_DSCENB = false;   // Data Set Change Interrupt Enable
	m_TIMENB = false;   // Timer Interrupt Enable
	m_XBIENB = false;   // Transmit Buffer Interrupt Enable
	m_RIENB = false;        // Read Buffer Interrupt Enable

	/* initialize transmitter */
	m_XBRE = true;      // Transmit Buffer Register Empty
	m_XSRE = true;      // Transmit Shift Register Empty

	/* initialize receiver */
	m_RBRL = false;     // Read Buffer Register Loaded

	/* clear RTS */
	m_RTSON = false;        // Request-to-send on (flag)
	m_RTSout = true;        // Note we are doing this to ensure the state is sent to the interface
	set_rts(CLEAR_LINE);
	m_RTSout = false;   // what we actually want

	/* set all register load flags to 1 */
	m_LDCTRL = true;
	m_LDIR = true;
	m_LRDR = true;
	m_LXDR = true;

	/* clear break condition */
	m_BRKON = false;
	m_BRKout = false;

	m_DSCH = false;
	m_TIMELP = false;
//  m_CTSin = false;   // not a good idea - this is the latch of an incoming line

	m_TMR = 0;
	m_STOPB = 0;
	m_RCL = 0;
	m_XDR = 0;
	m_RDR = 0;
	m_RBR = 0;
	m_XBR = 0;
	m_XSR = 0;

	// m_INT will be cleared in field_interrupts; setting to true is required
	// to trigger the INT line update
	m_INT = true;
	field_interrupts();
}

/*
    TMS9902 CRU write
*/
void tms9902_device::cruwrite(offs_t offset, uint8_t data)
{
	data &= 1;  /* clear extra bits */

	offset &= 0x1F;
	if (VERBOSE & LOG_DETAIL) LOGCRU("Setting bit %d = %02x\n", offset, data);

	if (offset <= 10)
	{
		uint16_t mask = (1 << offset);

		if (m_LDCTRL)
		{   // Control Register mode. Values written to bits 0-7 are copied
			// into the control register.
			switch (offset)
			{
			case 0:
				set_bits8(&m_RCL, 0x01, (data!=0));
				// we assume that bits are written in increasing order
				// so we do not transmit the data bits twice
				// (will fail when bit 1 is written first)
				break;
			case 1:
				set_bits8(&m_RCL, 0x02, (data!=0));
				set_data_bits();
				break;
			case 2:
				break;
			case 3:
				m_CLK4M = (data!=0);
				break;
			case 4:
				m_ODDP = (data!=0);
				// we also assume that the parity type is set before the parity enable
				break;
			case 5:
				m_PENB = (data!=0);
				set_parity();
				break;
			case 6:
				set_bits8(&m_STOPB, 0x01, (data!=0));
				break;
			case 7:
				set_bits8(&m_STOPB, 0x02, (data!=0));
				// When bit 7 is written the control register mode is automatically terminated.
				m_LDCTRL = false;
				set_stop_bits();
				break;
			default:
				logerror("tms9902: Invalid control register address %d\n", offset);
			}
		}
		else if (m_LDIR)
		{   // Interval Register mode. Values written to bits 0-7 are copied
			// into the interval register.
			if (offset <= 7)
			{
				set_bits8(&m_TMR, mask, (data!=0));

				if (offset == 7)
				{
					reload_interval_timer();
					// When bit 7 is written the interval register mode is automatically terminated.
					m_LDIR = false;
				}
			}
		}
		else if (m_LRDR || m_LXDR)
		{
			if (m_LRDR)
			{   // Receive rate register mode. Values written to bits 0-10 are copied
				// into the receive rate register.
				if (offset < 10)
				{
					set_bits16(&m_RDR, mask, (data!=0));
				}
				else
				{
					// When bit 10 is written the receive register mode is automatically terminated.
					m_RDV8 = (data!=0);
					m_LRDR = false;
					set_receive_data_rate();
				}
			}
			if (m_LXDR)
			{
				// The transmit rate register can be set together with the receive rate register.
				if (offset < 10)
				{
					set_bits16(&m_XDR, mask, (data!=0));
				}
				else
				{
					// Note that the transmit rate register is NOT terminated when
					// writing bit 10. This must be done by unsetting bit 11.
					m_XDV8 = (data!=0);
					set_transmit_data_rate();
				}
			}
		}
		else
		{   // LDCTRL=LDIR=LRDR=LXRD=0: Transmit buffer register mode. Values
			// written to bits 0-7 are transferred into the transmit buffer register.
			if (offset <= 7)
			{
				set_bits8(&m_XBR, mask, (data!=0));

				if (offset == 7)
				{   /* transmit */
					m_XBRE = false;
					// Spec: When the transmitter is active, the contents of the Transmit
					// Buffer Register are transferred to the Transmit Shift Register
					// each time the previous character has been completely transmitted
					// We need to check XSRE=true as well, as the implementation
					// makes use of a timed transmission, during which XSRE=false
					if (m_XSRE && m_RTSout && m_CTSin && !m_BRKout)
					{
						initiate_transmit();
					}
				}
			}
		}
		return;
	}
	switch (offset)
	{
		case 11:
			m_LXDR = (data!=0);
			break;
		case 12:
			m_LRDR = (data!=0);
			break;
		case 13:
			m_LDIR = (data!=0);
			// Spec: Each time LDIR is reset the contents of the Interval
			// Register are loaded into the Interval Timer, thus restarting
			// the timer.
			if (data==0)
				reload_interval_timer();
			break;
		case 14:
			m_LDCTRL = (data!=0);
			break;
		case 15:
			m_TSTMD = (data!=0); // Test mode not implemented
			break;
		case 16:
			if (data!=0)
			{
				m_RTSON = true;
				set_rts(ASSERT_LINE);
				if (m_CTSin)
				{
					if (m_XSRE && !m_XBRE && !m_BRKout)
						initiate_transmit();
					else if (m_BRKON)
						send_break(true);
				}
			}
			else
			{
				m_RTSON = false;
				if (m_XBRE && m_XSRE && !m_BRKout)
				{
					set_rts(CLEAR_LINE);
				}
			}
			return;
		case 17:
			LOGCRU("set BRKON=%d; BRK=%d\n", data, m_BRKout? 1:0);
			m_BRKON = (data!=0);
			if (m_BRKout && data==0)
			{
				// clear BRK
				m_BRKout = false;
				if ((!m_XBRE) && m_CTSin)
				{
					/* transmit next byte */
					initiate_transmit();
				}
				else if (!m_RTSON)
				{
					/* clear RTS */
					set_rts(CLEAR_LINE);
				}
			}
			else if (m_XBRE && m_XSRE && m_RTSout && m_CTSin)
			{
				send_break(data!=0);
			}
			return;
		case 18:
			// Receiver Interrupt Enable
			// According to spec, (re)setting this flag clears the RBRL flag
			// (the only way to clear the flag!)
			m_RIENB = (data!=0);
			m_RBRL = false;
			LOGCRU("Set RBRL=0, set RIENB=%d\n", data);
			field_interrupts();
			return;
		case 19:
			/* Transmit Buffer Interrupt Enable */
			m_XBIENB = (data!=0);
			LOGCRU("set XBIENB=%d\n", data);
			field_interrupts();
			return;
		case 20:
			/* Timer Interrupt Enable */
			m_TIMENB = (data!=0);
			m_TIMELP = false;
			m_TIMERR = false;
			field_interrupts();
			return;
		case 21:
			/* Data Set Change Interrupt Enable */
			m_DSCENB = (data!=0);
			m_DSCH = false;
			LOGCRU("set DSCH=0, set DSCENB=%d\n", data);
			field_interrupts();
			return;
		case 31:
			/* RESET */
			reset_uart();
			return;
		default:
			logerror("Writing to undefined flag bit position %d = %01x\n", offset, data);
	}
}

/*-------------------------------------------------
    device_stop - device-specific stop
-------------------------------------------------*/

void tms9902_device::device_stop()
{
	if (m_dectimer)
	{
		m_dectimer->reset();
		m_dectimer = nullptr;
	}
}

/*-------------------------------------------------
    device_reset - device-specific reset
-------------------------------------------------*/

void tms9902_device::device_reset()
{
	// This must be true because we may have missed a CTS* assertion
	// on startup, and the whole implementation relies on pushing
	m_CTSin = true;
	reset_uart();
}

/*-------------------------------------------------
    device_start - device-specific startup
-------------------------------------------------*/

void tms9902_device::device_start()
{
	m_clock_rate = clock();

	m_int_cb.resolve_safe();
	m_rcv_cb.resolve_safe();
	m_xmit_cb.resolve_safe();
	m_ctrl_cb.resolve_safe();

	m_dectimer = timer_alloc(DECTIMER);
	m_recvtimer = timer_alloc(RECVTIMER);
	m_sendtimer = timer_alloc(SENDTIMER);

	save_item(NAME(m_LDCTRL));
	save_item(NAME(m_LDIR));
	save_item(NAME(m_LRDR));
	save_item(NAME(m_LXDR));
	save_item(NAME(m_TSTMD));
	save_item(NAME(m_RTSON));
	save_item(NAME(m_BRKON));
	save_item(NAME(m_BRKout));
	save_item(NAME(m_XBR));
	save_item(NAME(m_XSR));
	save_item(NAME(m_RBR));
	save_item(NAME(m_DSCENB));
	save_item(NAME(m_RIENB));
	save_item(NAME(m_XBIENB));
	save_item(NAME(m_TIMENB));
	save_item(NAME(m_RDR));
	save_item(NAME(m_RDV8));
	save_item(NAME(m_XDR));
	save_item(NAME(m_XDV8));
	save_item(NAME(m_INT));
	save_item(NAME(m_DSCH));
	save_item(NAME(m_CTSin));
	save_item(NAME(m_DSRin));
	save_item(NAME(m_RTSout));
	save_item(NAME(m_TIMELP));
	save_item(NAME(m_TIMERR));
	save_item(NAME(m_XSRE));
	save_item(NAME(m_XBRE));
	save_item(NAME(m_RBRL));
	save_item(NAME(m_RIN));
	save_item(NAME(m_RSBD));
	save_item(NAME(m_RFBD));
	save_item(NAME(m_RFER));
	save_item(NAME(m_ROVER));
	save_item(NAME(m_RPER));
	save_item(NAME(m_RCL));
	save_item(NAME(m_ODDP));
	save_item(NAME(m_PENB));
	save_item(NAME(m_STOPB));
	save_item(NAME(m_CLK4M));
	save_item(NAME(m_TMR));
}

DEFINE_DEVICE_TYPE(TMS9902, tms9902_device, "tms9902", "TMS9902 ACC")
