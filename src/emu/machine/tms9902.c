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

#include <math.h>
#include "tms9902.h"

#define VERBOSE 1
#define LOG logerror

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
tms9902_device::tms9902_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TMS9902, "TMS9902 ACC", tag, owner, clock, "tms9902", __FILE__),
		m_int_cb(*this),
		m_rcv_cb(*this),
		m_xmit_cb(*this),
		m_ctrl_cb(*this)
{
}

/*
    should be called after any change to int_state or enabled_ints.
*/
void tms9902_device::field_interrupts()
{
	bool new_int = (m_DSCH && m_DSCENB)
							|| (m_RBRL && m_RIENB)
							|| (m_XBRE && m_XBIENB)
							|| (m_TIMELP && m_TIMENB);
	if (VERBOSE>8) LOG("TMS9902: interrupt flags (DSCH = %02x, DSCENB = %02x), (RBRL = %02x, RIENB = %02x), (XBRE = %02x, XBIENB = %02x), (TIMELP = %02x, TIMENB = %02x)\n",
		m_DSCH, m_DSCENB, m_RBRL, m_RIENB, m_XBRE, m_XBIENB, m_TIMELP, m_TIMENB);

	if (new_int != m_INT)
	{
		// Only consider edges
		m_INT = new_int;
		if (VERBOSE>3) LOG("TMS9902: /INT = %s\n", (m_INT)? "asserted" : "cleared");
		m_int_cb(m_INT? ASSERT_LINE : CLEAR_LINE);
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

	if (VERBOSE>3) LOG("TMS9902: CTS* = %s\n", (state==ASSERT_LINE)? "asserted" : "cleared");

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
		if (VERBOSE>4) LOG("TMS9902: no change in CTS line, no interrupt.");
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
	if (VERBOSE>3) LOG("TMS9902: DSR* = %s\n", (state==ASSERT_LINE)? "asserted" : "cleared");
	m_DSRin = (state==ASSERT_LINE);

	if (m_DSRin != previous)
	{
		m_DSCH = true;
		field_interrupts();
	}
	else
	{
		m_DSCH = false;
		if (VERBOSE>4) LOG("TMS9902: no change in DSR line, no interrupt.");
	}
}

/*
    Called whenever the incoming RIN line changes. This should be called by
    the device that contains the UART. Unlike the real thing, we deliver
    complete bytes in one go.
*/
void tms9902_device::rcv_data(UINT8 data)
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
		if (VERBOSE>3) LOG("TMS9902: Receive buffer loaded with byte %02x\n", data);
		field_interrupts();
	}
	else
	{
		// Receive buffer was full
		m_ROVER = true;
		if (VERBOSE>1) LOG("TMS9902: Receive buffer still loaded; overflow error\n");
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
	if (VERBOSE>2) LOG("TMS9902: Detected framing error\n");
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
	if (VERBOSE>2) LOG("TMS9902: Detected parity error\n");
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
	if (VERBOSE>2) LOG("TMS9902: Receive BREAK=%d (no effect)\n", value? 1:0);
}

//------------------------------------------------

/*
    Timer callback
*/
void tms9902_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	// This call-back is called by the MESS timer system when the decrementer
	// reaches 0.
	case DECTIMER:
		m_TIMERR = m_TIMELP;
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
		if (VERBOSE>2) LOG("TMS9902: Sending BREAK=%d\n", state? 1:0);

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
	if (VERBOSE>3) LOG("TMS9902: receive rate = %04x\n", value);

	// Calculate the ratio between receive baud rate and polling frequency
	double fint = m_clock_rate / ((m_CLK4M) ? 4.0 : 3.0);
	double baud = fint / (2.0 * ((m_RDV8)? 8:1) * m_RDR);

	// We assume 10 bit per character (7 data usually add 1 parity; 1 start, 1 stop)
	// This value represents the ratio of data inputs of one poll.
	// Thus the callback function should add up this value on each poll
	// and deliver a data input not before it sums up to 1.
	m_baudpoll = (double)(baud / (10*POLLING_FREQ));
	if (VERBOSE>3) LOG ("TMS9902: baudpoll = %f\n", m_baudpoll);

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
	if (VERBOSE>3) LOG("TMS9902: set transmit rate = %04x\n", value);
	m_last_config_value = value;
	m_ctrl_cb((offs_t)CONFIG, RATEXMIT);
}

void tms9902_device::set_stop_bits()
{
	int value = m_STOPB;
	if (VERBOSE>3) LOG("TMS9902: set stop bits = %02x\n", value);
	m_last_config_value = value;
	m_ctrl_cb((offs_t)CONFIG, STOPBITS);
}

void tms9902_device::set_data_bits()
{
	int value = m_RCL;
	if (VERBOSE>3) LOG("TMS9902: set data bits = %02x\n", value);
	m_last_config_value = value;
	m_ctrl_cb((offs_t)CONFIG, DATABITS);
}

void tms9902_device::set_parity()
{
	int value = (m_PENB? 2:0) | (m_ODDP? 1:0);
	if (VERBOSE>3) LOG("TMS9902: set parity = %02x\n", value);
	m_last_config_value = value;
	m_ctrl_cb((offs_t)CONFIG, PARITY);
}

void tms9902_device::transmit_line_state()
{
	// 00ab cdef = setting line RTS=a, CTS=b, DSR=c, DCD=d, DTR=e, RI=f
	// The 9902 only outputs RTS and BRK
	if (VERBOSE>3) LOG("TMS9902: transmitting line state (only RTS) = %02x\n", (m_RTSout)? 1:0);
	m_last_config_value = (m_RTSout)? RTS : 0;
	m_ctrl_cb((offs_t)LINES, RTS);
}

void tms9902_device::set_rts(line_state state)
{
	bool lstate = (state==ASSERT_LINE);

	if (lstate != m_RTSout)
	{
		// Signal RTS to the modem
		if (VERBOSE>3) LOG("TMS9902: Set RTS=%d\n", lstate? 1:0);
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
			if (VERBOSE>5) LOG("TMS9902: transferring XBR to XSR; XSRE=false, XBRE=true\n");
			m_XSR = m_XBR;
			m_XSRE = false;
			m_XBRE = true;

			field_interrupts();

			if (VERBOSE>4) LOG("TMS9902: transmit XSR=%02x, RCL=%02x\n", m_XSR, m_RCL);

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
READ8_MEMBER( tms9902_device::cruread )
{
	UINT8 answer = 0;

	offset &= 0x0003;

	switch (offset)
	{
	case 3: // Bits 31-24
		if (m_INT) answer |= 0x80;
		if (m_LDCTRL || m_LDIR || m_LRDR || m_LXDR || m_BRKON) answer |= 0x40;
		if (m_DSCH) answer |= 0x20;
		if (m_CTSin) answer |= 0x10;
		if (m_DSRin) answer |= 0x08;
		if (m_RTSout) answer |= 0x04;
		if (m_TIMELP)  answer |= 0x02;
		if (m_TIMERR)  answer |= 0x01;
		break;

	case 2: // Bits 23-16
		if (m_XSRE) answer |= 0x80;
		if (m_XBRE) answer |= 0x40;
		if (m_RBRL) answer |= 0x20;
		if (m_DSCH && m_DSCENB) answer |= 0x10;
		if (m_TIMELP && m_TIMENB) answer |= 0x08;
		if (m_XBRE && m_XBIENB) answer |= 0x02;
		if (m_RBRL && m_RIENB) answer |= 0x01;
		break;

	case 1: // Bits 15-8
		if (m_RIN) answer |= 0x80;
		if (m_RSBD) answer |= 0x40;
		if (m_RFBD) answer |= 0x20;
		if (m_RFER) answer |= 0x10;
		if (m_ROVER) answer |= 0x08;
		if (m_RPER) answer |= 0x04;
		if (m_RPER || m_RFER || m_ROVER) answer |= 0x02;
		break;

	case 0: // Bits 7-0
		answer = m_RBR;
		break;
	}
	if (VERBOSE>7) LOG("TMS9902: Reading flag bits %d - %d = %02x\n", ((offset+1)*8-1), offset*8, answer);
	return answer;
}

static inline void set_bits8(UINT8 *reg, UINT8 bits, bool set)
{
	if (set)
		*reg |= bits;
	else
		*reg &= ~bits;
}

static inline void set_bits16(UINT16 *reg, UINT16 bits, bool set)
{
	if (set)
		*reg |= bits;
	else
		*reg &= ~bits;
}

void tms9902_device::reset_uart()
{
	if (VERBOSE>1) LOG("TMS9902: resetting\n");

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
	m_INT = false;
	m_CTSin = false;

	m_TMR = 0;
	m_STOPB = 0;
	m_RCL = 0;
	m_XDR = 0;
	m_RDR = 0;
	m_RBR = 0;
	m_XBR = 0;
	m_XSR = 0;

	// m_INT will be cleared in field_interrupts
	field_interrupts();
}

/*
    TMS9902 CRU write
*/
WRITE8_MEMBER( tms9902_device::cruwrite )
{
	data &= 1;  /* clear extra bits */

	offset &= 0x1F;
	if (VERBOSE>5) LOG("TMS9902: Setting bit %d = %02x\n", offset, data);

	if (offset <= 10)
	{
		UINT16 mask = (1 << offset);

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
				if (VERBOSE>1) LOG("tms9902: Invalid control register address %d\n", offset);
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
			if (VERBOSE>3) LOG("TMS9902: set BRKON=%d; BRK=%d\n", data, m_BRKout? 1:0);
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
			if (VERBOSE>4) LOG("TMS9902: set RBRL=0, set RIENB=%d\n", data);
			field_interrupts();
			return;
		case 19:
			/* Transmit Buffer Interrupt Enable */
			m_XBIENB = (data!=0);
			if (VERBOSE>4) LOG("TMS9902: set XBIENB=%d\n", data);
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
			if (VERBOSE>4) LOG("TMS9902: set DSCH=0, set DSCENB=%d\n", data);
			field_interrupts();
			return;
		case 31:
			/* RESET */
			reset_uart();
			return;
		default:
			if (VERBOSE>1) LOG("TMS9902: Writing to undefined flag bit position %d = %01x\n", offset, data);
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
		m_dectimer = NULL;
	}
}

/*-------------------------------------------------
    device_reset - device-specific reset
-------------------------------------------------*/

void tms9902_device::device_reset()
{
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
}

const device_type TMS9902 = &device_creator<tms9902_device>;
