// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    TI-99 RS232 and Parallel interface card

    TI99 RS232 card ('rs232')
    TMS9902 ('rs232:tms9902_0')
    TMS9902 ('rs232:tms9902_1')
    TI99 RS232 attached serial device ('rs232:serdev0')
    TI99 RS232 attached serial device ('rs232:serdev1')
    TI99 PIO attached parallel device ('rs232:piodev')

    Currently this emulation does not directly interact with the serial
    interface on the host computer. However, using a socket connection it is
    possible to attach an external bridge which interacts with a real UART.

    TI RS232 card wiring
    --------------------
    The card uses this wiring (inverters not included)

     +-----+         Pins of the connector
     | 9902|          (common naming)
     | RIN |---<-------  2  (TD)
     | XOUT|--->-------  3  (RD)
     | RTS |--->-------  8  (DCD)
     | CTS |-<-+------- 20  (DTR)
     | DSR |-<-+    H--> 6  (DSR)
     +-----+     +-----> 5  (CTS)
                /
     +-----+   /
     | CRU |--+
     +-----+

    This wiring is typical for a DCE, not a DTE. The TI RS232 was obviously
    designed to look like a modem. The advantage is that you can use the same
    cables for connecting a modem to the RS232 interface or for connecting
    a second TI via its interface. To connect to a DTE you can use a 1-1
    wiring cable (1 on 1, 2 on 2 ...)

    The TI manual for the RS232 card suggests the following cables:

    TI RS232   -    Modem or other TI RS232
      2 -----<----- 3
      3 ----->----- 2
      6 ----->---- 20         (crossover cable)
     20 -----<----- 6

    TI RS232   -    Terminal (DTE)
      2 ----<------ 2
      3 ---->------ 3
      5 ---->------ 5
      6 ---->------ 6         (1-1 cable)
      8 ---->------ 8
     20 ----<------20

    If we want to use a PC serial interface to play the role of the TI
    interface we have to map the TI wiring to a suitable wiring for PC
    interfaces which are designed as DTEs. This is achieved by the functions
    map_lines_in, map_lines_out.

    Note that we now have to swap the cable types: Use a 1-1 cable to connect
    another TI or a modem on the other end, and use a crossover cable for
    another PC (the usual way of connecting).

    RS232 Over IP protocol
    ----------------------
    This implementation can make use of such an external bridge. Normal data
    are forwarded to the bridge and back, while line control is organized via
    special byte sequences. These sequences are introduced by a 0x1B byte (ESC).

    The protocol has two modes: normal and escape

    normal mode: transmit byte (!= 0x1b) unchanged
    escape mode: entered by ESC, bytes following:
       ESC = plain ESC byte
       length byte[length] = control sequence (length != 0x1b)

       byte[]:
          All configuration settings are related to a specified UART; UARTs may
          differ in their capabilities and may require specific settings
          (e.g. the TMS9902 specifies the line speed by a clock ratio, while
          others may have indexed, fixed rates or use integers)

          (x=unused)

          1ccc xaaa = configuration of parameter ccc; UART type aaa
             1111 xaaa rrrr rrrr rrrr 0000     = config receive rate on aaa
             1110 xaaa rrrr rrrr rrrr 0000     = config transmit rate on aaa
             1101 xaaa xxxx xxbb               = config databits bb (00=5 ... 11=8)
             1100 xaaa xxxx xxss               = config stop bits ss (00=1.5, 01=2, 1x=1)
             1011 xaaa xxxx xxpp               = config parity pp (1x=enable, x1=odd)

          00ab cdef = line state of RTS=a, CTS=b, DSR=c, DCD=d, DTR=e, RI=f
          01gh i000 = exception g=BRK, h=FRMERR, i=PARERR

    The protocol changes back to normal mode after transmitting the control
    sequence.

    Michael Zapf
    February 2012: Rewritten as class

*****************************************************************************/

#include "emu.h"
#include "ti_rs232.h"

#define LOG_WARN        (1U<<1)    // Warnings
#define LOG_CONFIG      (1U<<2)
#define LOG_LINES       (1U<<3)
#define LOG_SETTING     (1U<<4)
#define LOG_STATE       (1U<<5)
#define LOG_MAP         (1U<<6)
#define LOG_IN          (1U<<7)
#define LOG_OUT         (1U<<8)
#define LOG_ILA         (1U<<9)

#define VERBOSE ( LOG_CONFIG | LOG_WARN )
#include "logmacro.h"

DEFINE_DEVICE_TYPE_NS(TI99_RS232,     bus::ti99::peb, ti_rs232_pio_device,      "ti99_rs232",           "TI-99 RS232/PIO interface")
DEFINE_DEVICE_TYPE_NS(TI99_RS232_DEV, bus::ti99::peb, ti_rs232_attached_device, "ti99_rs232_atttached", "TI-99 Serial attached device")
DEFINE_DEVICE_TYPE_NS(TI99_PIO_DEV,   bus::ti99::peb, ti_pio_attached_device,   "ti99_pio_attached",    "TI-99 Parallel attached device")

namespace bus { namespace ti99 { namespace peb {

#define SENILA_0_BIT 0x80
#define SENILA_1_BIT 0x40

#define RECV_MODE_NORMAL 1
#define RECV_MODE_ESC 2
#define RECV_MODE_ESC_LINES 3

#define ESC 0x1b

#define UART0 "uart0"
#define UART1 "uart1"

#define SERDEV0 "serdev0"
#define SERDEV1 "serdev1"
#define PIODEV "piodev"

ti_rs232_pio_device::ti_rs232_pio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, TI99_RS232, tag, owner, clock),
	device_ti99_peribox_card_interface(mconfig, *this),
	m_crulatch(*this, "crulatch"),
	m_uart0(*this, UART0),
	m_uart1(*this, UART1),
	m_serdev0(*this, SERDEV0),
	m_serdev1(*this, SERDEV1),
	m_piodev(*this, PIODEV),
	m_dsrrom(nullptr),
	m_pio_direction_in(false),
	m_pio_handshakeout(false),
	m_pio_handshakein(false),
	m_pio_spareout(false),
	m_pio_sparein(false),
	m_flag0(false),
	m_led(false),
	m_pio_out_buffer(0),
	m_pio_in_buffer(0),
	m_pio_readable(false),
	m_pio_writable(false),
	m_pio_write(false),
	m_ila(0)
{
}


/**************************************************************************/
/* Ports */

ti_rs232_attached_device::ti_rs232_attached_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TI99_RS232_DEV, tag, owner, clock),
	device_image_interface(mconfig, *this),
	m_uart(nullptr)
{
}

ti_pio_attached_device::ti_pio_attached_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TI99_PIO_DEV, tag, owner, clock),
	device_image_interface(mconfig, *this)
{
}

/*
    Initialize rs232 unit and open image
*/
image_init_result ti_rs232_attached_device::call_load()
{
	m_uart->set_clock(true);

	// The following line may cause trouble in the init phase
	// card->incoming_dtr(devnumber, (m_file!=nullptr)? ASSERT_LINE : CLEAR_LINE);

	return image_init_result::PASS;  // OK
}

void ti_rs232_attached_device::call_unload()
{
	m_uart->set_clock(false);
}

/*
    Initialize pio unit and open image
*/
image_init_result ti_pio_attached_device::call_load()
{
	ti_rs232_pio_device* card = static_cast<ti_rs232_pio_device*>(owner());

	// tell whether the image is readable
	card->m_pio_readable = true;
	// tell whether the image is writable
	card->m_pio_writable = !is_readonly();

	if (card->m_pio_write && card->m_pio_writable)
		card->m_pio_handshakein = false;    // receiver ready
	else
		card->m_pio_handshakein = true;

	return image_init_result::PASS;  // OK
}

/*
    close a pio image
*/
void ti_pio_attached_device::call_unload()
{
	ti_rs232_pio_device* card = static_cast<ti_rs232_pio_device*>(owner());

	card->m_pio_writable = false;
	card->m_pio_handshakein = true; /* receiver not ready */
	card->m_pio_sparein = false;
}

/****************************************************************************/

/*
    CRU read
*/
READ8Z_MEMBER(ti_rs232_pio_device::crureadz)
{
	if ((offset & 0xff00)==m_cru_base)
	{
		if ((offset & 0x00c0)==0x0000)
		{
			uint8_t reply = 0x00;
			if (m_pio_direction_in)         reply |= 0x02;
			if (m_pio_handshakein)          reply |= 0x04;
			if (m_pio_sparein)              reply |= 0x08;
			if (m_flag0)                    reply |= 0x10;
			// The CTS line is realized as CRU bits
			// Mind that this line is handled as an output going to the remote CTS
			if ((m_signals[0] & tms9902_device::CTS)!=0)    reply |= 0x20;
			if ((m_signals[1] & tms9902_device::CTS)!=0)    reply |= 0x40;
			if (m_led)                      reply |= 0x80;
			*value = BIT(reply, (offset>>1) & 7);
			return;
		}
		if ((offset & 0x00c0)==0x0040)
		{
			*value = m_uart0->cruread(offset>>1);
			return;
		}
		if ((offset & 0x00c0)==0x0080)
		{
			*value = m_uart1->cruread(offset>>1);
			return;
		}
	}
}

/*
    CRU write
*/
void ti_rs232_pio_device::cruwrite(offs_t offset, uint8_t data)
{
	if ((offset & 0xff00)==m_cru_base)
	{
		if ((offset & 0x00c0)==0x0040)
		{
			m_uart0->cruwrite(offset>>1, data);
			return;
		}
		if ((offset & 0x00c0)==0x0080)
		{
			m_uart1->cruwrite(offset>>1, data);
			return;
		}

		int bit = (offset & 0x00ff)>>1;
		m_crulatch->write_bit(bit, data);
	}
}

WRITE_LINE_MEMBER(ti_rs232_pio_device::selected_w)
{
	m_selected = state;
}

WRITE_LINE_MEMBER(ti_rs232_pio_device::pio_direction_in_w)
{
	m_pio_direction_in = state;
}

WRITE_LINE_MEMBER(ti_rs232_pio_device::pio_handshake_out_w)
{
	m_pio_handshakeout = state;
	if (m_pio_write && m_pio_writable && (!m_pio_direction_in))
	{   /* PIO in output mode */
		if (!m_pio_handshakeout)
		{   /* write data strobe */
			/* write data and acknowledge */
			uint8_t buf = m_pio_out_buffer;
			int ret = m_piodev->fwrite(&buf, 1);
			if (ret)
				m_pio_handshakein = 1;
		}
		else
		{
			/* end strobe */
			/* we can write some data: set receiver ready */
			m_pio_handshakein = 0;
		}
	}
	if ((!m_pio_write) && m_pio_readable /*&& pio_direction_in*/)
	{   /* PIO in input mode */
		if (!m_pio_handshakeout)
		{   /* receiver ready */
			/* send data and strobe */
			uint8_t buf;
			if (m_piodev->fread(&buf, 1))
				m_pio_in_buffer = buf;
			m_pio_handshakein = 0;
		}
		else
		{
			/* data acknowledge */
			/* we can send some data: set transmitter ready */
			m_pio_handshakein = 1;
		}
	}
}

WRITE_LINE_MEMBER(ti_rs232_pio_device::pio_spareout_w)
{
	m_pio_spareout = state;
}

WRITE_LINE_MEMBER(ti_rs232_pio_device::flag0_w)
{
	m_flag0 = state;
}

WRITE_LINE_MEMBER(ti_rs232_pio_device::cts0_w)
{
	// Set the CTS line for RS232/1
	LOGMASKED(LOG_LINES, "(1/3) Setting CTS* via CRU to %d\n", state);
	output_line_state(0, tms9902_device::CTS, state ? 0 : tms9902_device::CTS);
}

WRITE_LINE_MEMBER(ti_rs232_pio_device::cts1_w)
{
	// Set the CTS line for RS232/2
	LOGMASKED(LOG_LINES, "(2/4) Setting CTS* via CRU to %d\n", state);
	output_line_state(1, tms9902_device::CTS, state ? 0 : tms9902_device::CTS);
}

WRITE_LINE_MEMBER(ti_rs232_pio_device::led_w)
{
	m_led = state;
}

/*
    Memory read
*/
READ8Z_MEMBER( ti_rs232_pio_device::readz )
{
	if (m_senila==ASSERT_LINE)
	{
		LOGMASKED(LOG_ILA, "Sensing ILA\n");
		*value = m_ila;
		// The card ROM must be unselected, or we get two values
		// on the data bus

		// Not sure whether this is correct; there is no software that makes
		// use of it
		m_ila = 0;
	}
	if (((offset & m_select_mask)==m_select_value) && m_selected)
	{
		if ((offset & 0x1000)==0x0000)
		{
			*value = m_dsrrom[offset&0x0fff];
		}
		else
		{
			*value = m_pio_direction_in ? m_pio_in_buffer : m_pio_out_buffer;
		}
	}
}

/*
    Memory write
*/
void ti_rs232_pio_device::write(offs_t offset, uint8_t data)
{
	if (((offset & m_select_mask)==m_select_value) && m_selected)
	{
		if ((offset & 0x1001)==0x1000)
		{
			m_pio_out_buffer = data;
		}
	}
}

/**************************************************************************/

/*
    The DTR line of the interface card is wired to the CTS and DSR
    of the UART.
*/
void ti_rs232_pio_device::incoming_dtr(int uartind, line_state value)
{
	LOGMASKED(LOG_LINES, "(RS232/%d) Incoming DTR = %d\n", uartind+1, (value==ASSERT_LINE)? 1:0);

	if (uartind==0)
	{
		m_uart0->rcv_cts(value);
		m_uart0->rcv_dsr(value);
	}
	else
	{
		m_uart1->rcv_cts(value);
		m_uart1->rcv_dsr(value);
	}
}

/*
    Data transmission
*/
void ti_rs232_pio_device::transmit_data(int uartind, uint8_t value)
{
	uint8_t buf = value;
	ti_rs232_attached_device *serial = (uartind==0)? m_serdev0 : m_serdev1;

	if (!serial->exists())
	{
		LOGMASKED(LOG_CONFIG, "(RS232/%d) No serial output attached\n", uartind+1);
		return;
	}

	// Send a double ESC if this is not a control operation
	if (buf==0x1b)
	{
		LOGMASKED(LOG_OUT, "(RS232/%d) send ESC (requires another ESC)\n", uartind+1);
		serial->fwrite(&buf, 1);
	}
	char cbuf = (buf < 0x20 || buf > 0x7e)? '.' : (char)buf;
	LOGMASKED(LOG_OUT, "(RS232/%d) send %c <%02x>\n", uartind+1, cbuf, buf);
	serial->fwrite(&buf, 1);
}

/*
    Map the DCE-like wiring to a DTE-like wiring and vice versa (mapping==0)
    No handshake

       Emulated           PC serial
       TI RS232           interface
     XOUT  2 TXD ----->-----( 3) ---> TXD
      RIN  3 RXD -----<-----( 2) <--- RXD
      CRU  5 CTS -|       |-( 8) <--- DCD  (cable)
     +12V  6 DSR ----->-----(20) ---> DTR
      RTS  8 DCD ----->-----( 4) ---> RTS
  DSR+CTS 20 DTR -----<-----( 6) <--- DSR
                          |-( 5) <--- CTS

    Alternative mapping: (mapping==1)
    RTS/CTS handshake

       Emulated           PC serial
       TI RS232           interface
     XOUT  2 TXD ----->-----( 3) ---> TXD
      RIN  3 RXD -----<-----( 2) <--- RXD
      CRU  5 CTS ----->-----( 4) ---> RTS
      +12V 6 DSR -|       |-( 6) <--- DSR
      RTS  8 DCD ----->-----(20) ---> DTR
  DSR+CTS 20 DTR -----<-----( 8) <--- DCD
                          |-( 5) <--- CTS

    Yet another mapping: (mapping==2)
    CRU-based handshake

       Emulated           PC serial
       TI RS232           interface
     XOUT  2 TXD ----->-----( 3) ---> TXD
      RIN  3 RXD -----<-----( 2) <--- RXD
      CRU  5 CTS ----->-----(20) ---> DTR
      +12V 6 DSR -|       |-( 6) <--- DSR
      RTS  8 DCD ----->-----( 4) ---> RTS
  DSR+CTS 20 DTR -----<-----( 5) <--- CTS  (cable)
                          |-( 8) <--- DCD

*/
uint8_t ti_rs232_pio_device::map_lines_out(int uartind, uint8_t value)
{
	uint8_t ret = 0;
	int mapping = ioport("SERIALMAP")->read();

	//    00ab cdef = setting line RTS=a, CTS=b, DSR=c, DCD=d, DTR=e, RI=f

	LOGMASKED(LOG_LINES, "(RS232/%d) out connector pins = 0x%02x; translate for DTE\n", uartind+1, value);

	if (value & tms9902_device::BRK)
	{
		LOGMASKED(LOG_MAP, "(RS232/%d) Sending BRK\n", uartind+1);
		ret |= tms9902_device::EXCEPT | tms9902_device::BRK;
	}

	if (mapping==0)
	{
		// V1
		if (value & tms9902_device::CTS)
		{
			LOGMASKED(LOG_MAP, "(RS232/%d) Cannot map CTS line, ignoring\n", uartind+1);
		}
		if (value & tms9902_device::DSR)
		{
			ret |= tms9902_device::DTR;
			LOGMASKED(LOG_MAP, "(RS232/%d) Setting DTR line\n", uartind+1);
		}
		if (value & tms9902_device::DCD)
		{
			ret |= tms9902_device::RTS;
			LOGMASKED(LOG_MAP, "(RS232/%d) Setting RTS line\n", uartind+1);
		}
	}
	else
	{
		if (mapping==1)
		{
			// V2
			if (value & tms9902_device::CTS)
			{
				ret |= tms9902_device::RTS;
				LOGMASKED(LOG_MAP, "(RS232/%d) Setting RTS line\n", uartind+1);
			}
			if (value & tms9902_device::DCD)
			{
				ret |= tms9902_device::DTR;
				LOGMASKED(LOG_MAP, "(RS232/%d) Setting DTR line\n", uartind+1);
			}
		}
		else
		{
			// v3
			if (value & tms9902_device::CTS)
			{
				ret |= tms9902_device::DTR;
				LOGMASKED(LOG_MAP, "(RS232/%d) Setting DTR line\n", uartind+1);
			}
			if (value & tms9902_device::DSR)
			{
				LOGMASKED(LOG_MAP, "(RS232/%d) Cannot map DSR line, ignoring\n", uartind+1);
			}
			if (value & tms9902_device::DCD)
			{
				ret |= tms9902_device::RTS;
				LOGMASKED(LOG_MAP, "(RS232/%d) Setting RTS line\n", uartind+1);
			}
		}
	}

	return ret;
}

uint8_t ti_rs232_pio_device::map_lines_in(int uartind, uint8_t value)
{
	uint8_t ret = 0;
	int mapping = ioport("SERIALMAP")->read();

	//    00ab cdef = setting line RTS=a, CTS=b, DSR=c, DCD=d, DTR=e, RI=f

	LOGMASKED(LOG_LINES, "(RS232/%d) in connector pins = 0x%02x; translate from DTE\n", uartind+1, value);

	if (value & tms9902_device::BRK)
	{
		LOGMASKED(LOG_MAP, "(RS232/%d) Getting BRK\n", uartind+1);
		ret |= tms9902_device::EXCEPT | tms9902_device::BRK;
	}

	if (mapping==0)
	{
		// V1
		if (value & tms9902_device::CTS)
		{
			LOGMASKED(LOG_MAP, "(RS232/%d) Cannot map CTS line, ignoring\n", uartind+1);
		}
		if (value & tms9902_device::DSR)
		{
			ret |= tms9902_device::DTR;
			LOGMASKED(LOG_MAP, "(RS232/%d) Setting DTR line\n", uartind+1);
		}
		if (value & tms9902_device::DCD)
		{
			LOGMASKED(LOG_MAP, "(RS232/%d) Cannot map DCD line, ignoring\n", uartind+1);
		}
	}
	else
	{
		if (mapping==1)
		{
			if (value & tms9902_device::DCD)
			{
				ret |= tms9902_device::DTR;
				LOGMASKED(LOG_MAP, "(RS232/%d) Setting DTR line\n", uartind+1);
			}
			if (value & tms9902_device::DSR)
			{
				LOGMASKED(LOG_MAP, "(RS232/%d) Cannot map DSR line, ignoring\n", uartind+1);
			}
			if (value & tms9902_device::CTS)
			{
				LOGMASKED(LOG_MAP, "(RS232/%d) Cannot map CTS line, ignoring\n", uartind+1);
			}
		}
		else
		{
			if (value & tms9902_device::CTS)
			{
				ret |= tms9902_device::DTR;
				LOGMASKED(LOG_MAP, "(RS232/%d) Setting DTR line\n", uartind+1);
			}
			if (value & tms9902_device::DSR)
			{
				LOGMASKED(LOG_MAP, "(RS232/%d) Cannot map DSR line, ignoring\n", uartind+1);
			}
			if (value & tms9902_device::DCD)
			{
				LOGMASKED(LOG_MAP, "(RS232/%d) Cannot map DCD line, ignoring\n", uartind+1);
			}
		}
	}

	return ret;
}
/*
    Receive a character or a line state from the remote site. This method
    is called by a timer with some sufficiently high polling frequency. Note
    that the control lines are not subject to baud rates.
    The higher polling frequency will cause overloads in the TMS9902 which has
    a one-byte buffer only: Since the data source (e.g. the PC UART and the
    socket connection) may be buffered, we may get a cluster of bytes in rapid
    succession. In order to avoid this, this function uses the parameter
    "baudpoll" which is the ratio of the characters/second and the polling
    frequency. (char/sec is baud rate divided by 10.)
    Whenever we receive a character that is passed to the UART, we have to
    pause for 1/baudpoll iterations before getting the next byte from the
    data source.

    FIXME: This may fail when the emulated system tries to stop the remote
    system by deactivating RTS or DTR, but there are still incoming
    bytes in the socket or PC UART buffer. The buffered bytes may then cause
    an overflow in the emulated UART, since the application program expects
    the remote system to stop sending instantly.
    The only way to handle this is to mirror the activity within the serial
    bridge: Whenever a RTS=0 or DTR=0 is transmitted to the remote site, the
    serial bridge must stop delivering data bytes until the handshake opens the
    channel again.
*/
void ti_rs232_pio_device::receive_data_or_line_state(int uartind)
{
	uint8_t buffer;

	ti_rs232_attached_device *serial = (uartind==0)? m_serdev0 : m_serdev1;
	tms9902_device *uart = (uartind==0)? m_uart0 : m_uart1;

	if (!serial->exists())
	{
		LOGMASKED(LOG_CONFIG, "(RS232/%d) No serial input attached\n", uartind+1);
		return;
	}

	double baudpoll = uart->get_baudpoll();

	// If more than the minimum waiting time since the last data byte has
	// elapsed, we can get a new value.
	if (m_time_hold[uartind] > 1.0)
	{
		// Buffer empty?
		if (m_bufpos[uartind] == m_buflen[uartind])
		{
			// Get all out of sdlsocket
			m_buflen[uartind] = serial->fread(m_recvbuf[uartind].get(), 512);
			m_bufpos[uartind] = 0;
			if (m_buflen[uartind]==0) return;
		}
		buffer = m_recvbuf[uartind][m_bufpos[uartind]++];
	}
	else
	{
		// number of polls has not yet elapsed; we have to wait.
		m_time_hold[uartind] += baudpoll;
		return;
	}

	char cbuf = (buffer < 0x20 || buffer > 0x7e)? '.' : (char)buffer;

	// No config parameters here, only data or line setting
	switch (m_recv_mode[uartind])
	{
	case RECV_MODE_NORMAL:
		if (buffer==0x1b)
		{
			LOGMASKED(LOG_IN, "(RS232/%d) Received: %c <%02x>, switch to ESC mode\n", uartind+1, cbuf, buffer);
			m_recv_mode[uartind] = RECV_MODE_ESC;
		}
		else
		{
			LOGMASKED(LOG_IN, "(RS232/%d) Received: %c <%02x>, pass to UART\n", uartind+1, cbuf, buffer);
			uart->rcv_data(buffer);
			m_time_hold[uartind] = 0.0;
		}
		break;
	case RECV_MODE_ESC:
		if (buffer==0x1b)
		{
			m_recv_mode[uartind] = RECV_MODE_NORMAL;
			LOGMASKED(LOG_STATE, "(RS232/%d) Received another ESC, passing to UART, leaving ESC mode\n", uartind+1);
			uart->rcv_data(buffer);
			m_time_hold[uartind] = 0.0;
		}
		else
		{
			// the byte in buffer is the length byte
			LOGMASKED(LOG_STATE, "(RS232/%d) Received length byte <%02x> in ESC mode\n", uartind+1, buffer);
			if (buffer != 1)
			{
				LOGMASKED(LOG_WARN, "(RS232/%d) ** ERROR: Expected length byte 1 but got 0x%02x, leaving ESC mode.\n", uartind+1, buffer);
				m_recv_mode[uartind] = RECV_MODE_NORMAL;
			}
			else
				m_recv_mode[uartind] = RECV_MODE_ESC_LINES;
		}
		break;

	case RECV_MODE_ESC_LINES:
		// Map the real serial interface lines to our emulated connector
		// The mapping is the same for both directions, so we use the same function
		if (buffer & tms9902_device::EXCEPT)
		{
			// Exception states: BRK, FRMERR, PARERR
			LOGMASKED(LOG_LINES, "(RS232/%d) Received BRK or ERROR <%02x>\n", uartind+1, buffer);
			uart->rcv_break(((buffer & tms9902_device::BRK)!=0));

			if (buffer & tms9902_device::FRMERR)
				uart->rcv_framing_error();
			if (buffer & tms9902_device::PARERR)
				uart->rcv_parity_error();
		}
		else
		{
			buffer = map_lines_in(uartind, buffer);
			LOGMASKED(LOG_LINES, "(RS232/%d) Received (remapped) <%02x> in ESC mode\n", uartind+1, buffer);

			// The DTR line on the RS232 connector of the board is wired to both the
			// CTS and the DSR pin of the TMS9902
			// Apart from the data line, DTR is the only input line
			incoming_dtr(uartind,  (buffer & tms9902_device::DTR)? ASSERT_LINE : CLEAR_LINE);
		}

		m_recv_mode[uartind] = RECV_MODE_NORMAL;
		break;

	default:
		LOGMASKED(LOG_WARN, "(RS232/%d) Unknown mode: %d\n", uartind+1, m_recv_mode[uartind]);
	}
}

/*
    Control operations like configuration or line changes
*/
void ti_rs232_pio_device::configure_interface(int uartind, int type, int value)
{
	uint8_t bufctrl[4];
	ti_rs232_attached_device *serial = (uartind==0)? m_serdev0 : m_serdev1;
	uint8_t esc = ESC;

	if (!serial->exists())
	{
		LOGMASKED(LOG_CONFIG, "(RS232/%d) No serial output attached\n", uartind+1);
		return;
	}

	serial->fwrite(&esc, 1);
	bufctrl[0] = 0x02;
	bufctrl[1] = tms9902_device::CONFIG | tms9902_device::TYPE_TMS9902;

	switch (type) {
	case tms9902_device::RATERECV:
		LOGMASKED(LOG_SETTING, "(RS232/%d) Send receive rate %04x\n", uartind+1, value);
		// value has 12 bits
		// 1ccc xaaa                         = config adapter type a
		// 1111 xaaa rrrr rrrr rrrr 0000     = config receive rate on a
		// 1110 xaaa rrrr rrrr rrrr 0000     = config transmit rate on a
		bufctrl[0] = 0x03; // length
		bufctrl[1] |= tms9902_device::RATERECV;
		bufctrl[2] = (value & 0x0ff0)>>4;
		bufctrl[3] = (value & 0x0f)<<4;
		break;
	case tms9902_device::RATEXMIT:
		LOGMASKED(LOG_SETTING, "(RS232/%d) Send transmit rate %04x\n", uartind+1, value);
		bufctrl[0] = 0x03; // length
		bufctrl[1] |= tms9902_device::RATEXMIT;
		bufctrl[2] = (value & 0x0ff0)>>4;
		bufctrl[3] = (value & 0x0f)<<4;
		break;
	case tms9902_device::STOPBITS:
		LOGMASKED(LOG_SETTING, "(RS232/%d) Send stop bit config %02x\n", uartind+1, value&0x03);
		bufctrl[1] |= tms9902_device::STOPBITS;
		bufctrl[2] = (value & 0x03);
		break;
	case tms9902_device::DATABITS:
		LOGMASKED(LOG_SETTING, "(RS232/%d) Send data bit config %02x\n", uartind+1, value&0x03);
		bufctrl[1] |= tms9902_device::DATABITS;
		bufctrl[2] = (value & 0x03);
		break;
	case tms9902_device::PARITY:
		LOGMASKED(LOG_SETTING, "(RS232/%d) Send parity config %02x\n", uartind+1, value&0x03);
		bufctrl[1] |= tms9902_device::PARITY;
		bufctrl[2] = (value & 0x03);
		break;
	default:
		LOGMASKED(LOG_WARN, "(RS232/%d) Error - unknown config type %02x\n", uartind+1, type);
	}

	serial->fwrite(bufctrl, bufctrl[0]+1);
}

void ti_rs232_pio_device::set_bit(int uartind, int line, int value)
{
	switch (line)
	{
	case tms9902_device::CTS:
		LOGMASKED(LOG_LINES, "(RS232/%d) Set CTS(out)=%s\n", uartind+1, (value!=0)? "asserted" : "cleared");
		break;
	case tms9902_device::DCD:
		LOGMASKED(LOG_LINES, "(RS232/%d) Set DCD(out)=%s\n", uartind+1, (value!=0)? "asserted" : "cleared");
		break;
	case tms9902_device::BRK:
		LOGMASKED(LOG_LINES, "(RS232/%d) Set BRK(out)=%s\n", uartind+1, (value!=0)? "asserted" : "cleared");
		break;
	}

	if (value!=0)
		m_signals[uartind] |= line;
	else
		m_signals[uartind] &= ~line;
}

/*
   Line changes
*/
void ti_rs232_pio_device::output_exception(int uartind, int param, uint8_t value)
{
	ti_rs232_attached_device *serial = (uartind==0)? m_serdev0 : m_serdev1;
	uint8_t bufctrl[2];
	uint8_t esc = ESC;

	if (!serial->exists())
	{
		LOGMASKED(LOG_CONFIG, "(RS232/%d) No serial output attached\n", uartind+1);
		return;
	}

	serial->fwrite(&esc, 1);

	bufctrl[0] = 1;
	// 0100 0xxv = exception xx: 02=BRK, 04=FRMERR, 06=PARERR; v=0,1 (only for BRK)
	// BRK is the only output exception
	bufctrl[1] = tms9902_device::EXCEPT | param | (value&1);
	serial->fwrite(bufctrl, 2);
}

/*
   Line changes
*/
void ti_rs232_pio_device::output_line_state(int uartind, int mask, uint8_t value)
{
	ti_rs232_attached_device *serial = (uartind==0)? m_serdev0 : m_serdev1;
	uint8_t bufctrl[2];
	uint8_t esc = ESC;

	if (!serial->exists())
	{
		LOGMASKED(LOG_CONFIG, "(RS232/%d) No serial output attached\n", uartind+1);
		return;
	}

	// Send ESC to serial bridge
	// FIXME: When the socket cannot be set up, MAME crashes here
	serial->fwrite(&esc, 1);

	// Length 1
	bufctrl[0] = 1;

	// 01ab cdef = setting line RTS=a, CTS=b, DSR=c, DCD=d, DTR=e, RI=f

	// The CTS line (coming from a CRU bit) is connected to the CTS pin
	if (mask & tms9902_device::CTS) set_bit(uartind, tms9902_device::CTS, value & tms9902_device::CTS);

	// The RTS line (from 9902) is connected to the DCD pin
	if (mask & tms9902_device::RTS) set_bit(uartind, tms9902_device::DCD, value & tms9902_device::RTS);

	// The DSR pin is hardwired to +5V
	set_bit(uartind, tms9902_device::DSR, 1);

	// As of here, the lines are set according to the schematics of the
	// serial interface.

	// Now translate the signals of the board to those of a DTE-like device
	// so that we can pass the signal to the real PC serial interface
	// (can be imagined as if we emulated the cable)
	bufctrl[1] = map_lines_out(uartind, m_signals[uartind]);
	serial->fwrite(bufctrl, 2);
}

/***********************************************************************
    callbacks
************************************************************************/
/*
    Propagates the /INT signal of the UARTs to the /INT line of the pbox.
*/
WRITE_LINE_MEMBER( ti_rs232_pio_device::int0_callback )
{
	int senila_bit = SENILA_0_BIT;

	if (state==ASSERT_LINE) m_ila |= senila_bit;
	else m_ila &= ~senila_bit;

	m_slot->set_inta(state);
}

WRITE_LINE_MEMBER( ti_rs232_pio_device::int1_callback )
{
	int senila_bit = SENILA_1_BIT;

	if (state==ASSERT_LINE) m_ila |= senila_bit;
	else m_ila &= ~senila_bit;

	m_slot->set_inta(state);
}

/*
    Called from the UART when it wants to receive a character
    However, characters are not passed to it at this point
    Instead, we check for signal line change or data transmission
    and call the respective function
*/
WRITE_LINE_MEMBER( ti_rs232_pio_device::rcv0_callback )
{
	receive_data_or_line_state(0);
}

WRITE_LINE_MEMBER( ti_rs232_pio_device::rcv1_callback )
{
	receive_data_or_line_state(1);
}

void ti_rs232_pio_device::xmit0_callback(uint8_t data)
{
	transmit_data(0, data);
}

void ti_rs232_pio_device::xmit1_callback(uint8_t data)
{
	transmit_data(1, data);
}

void ti_rs232_pio_device::ctrl_callback(int uartind, int offset, uint8_t data)
{
	if ((offset & tms9902_device::CONFIG)!=0)
	{
		// We cannot pass the configuration data as they need more than 8 bits.
		// Could be done by a write16 function as well.
		configure_interface(uartind, data, (uartind==0)? m_uart0->get_config_value() : m_uart1->get_config_value());
	}
	else
	{
		if ((offset & tms9902_device::EXCEPT)!=0)
		{
			output_exception(uartind, offset & ~tms9902_device::EXCEPT, data);
		}
		else
		{
			output_line_state(uartind, offset, data);
		}
	}
}

void ti_rs232_pio_device::ctrl0_callback(offs_t offset, uint8_t data)
{
	ctrl_callback(0, offset, data);
}

void ti_rs232_pio_device::ctrl1_callback(offs_t offset, uint8_t data)
{
	ctrl_callback(1, offset, data);
}

void ti_rs232_pio_device::device_start()
{
	m_dsrrom = memregion(TI99_DSRROM)->base();

	// Prepare the receive buffers
	m_recvbuf[0] = std::make_unique<uint8_t[]>(512);
	m_recvbuf[1] = std::make_unique<uint8_t[]>(512);

	m_pio_write = true; // required for call_load of pio_attached_device
	m_pio_writable = false;
	m_pio_handshakein = false;

	// We don't save the receive buffers for persistent state
	save_pointer(NAME(m_signals),2);
	save_pointer(NAME(m_recv_mode),2);
	save_pointer(NAME(m_time_hold),2);
	save_item(NAME(m_pio_direction_in));
	save_item(NAME(m_pio_handshakeout));
	save_item(NAME(m_pio_handshakein));
	save_item(NAME(m_pio_spareout));
	save_item(NAME(m_pio_sparein));
	save_item(NAME(m_flag0));
	save_item(NAME(m_led));
	save_item(NAME(m_pio_out_buffer));
	save_item(NAME(m_pio_in_buffer));
	save_item(NAME(m_pio_readable));
	save_item(NAME(m_pio_writable));
	save_item(NAME(m_pio_write));
	save_item(NAME(m_ila));
}

void ti_rs232_pio_device::device_stop()
{
	m_recvbuf[0] = nullptr;
	m_recvbuf[1] = nullptr;
}

void ti_rs232_pio_device::device_reset()
{
	m_pio_direction_in = false;
	m_pio_handshakeout = false;
	m_pio_spareout = false;
	m_flag0 = false;

	set_bit(0, tms9902_device::CTS, 0);
	set_bit(1, tms9902_device::CTS, 0);

	m_led = false;
	m_recv_mode[0] = RECV_MODE_NORMAL;
	m_recv_mode[1] = RECV_MODE_NORMAL;

	m_bufpos[0] = m_bufpos[1] = m_buflen[0] = m_buflen[1] = 0;

	if (m_genmod)
	{
		m_select_mask = 0x1fe000;
		m_select_value = 0x174000;
	}
	else
	{
		m_select_mask = 0x7e000;
		m_select_value = 0x74000;
	}

	m_selected = false;

	m_cru_base = (ioport("CRURS232")->read()==0)? 0x1300 : 0x1500;

	m_time_hold[0] = m_time_hold[1] = 0.0;

	// Both DTRs are pulled up
	incoming_dtr(0, ASSERT_LINE);
	incoming_dtr(1, ASSERT_LINE);
}

ROM_START( ti_rs232 )
	ROM_REGION(0x1000, TI99_DSRROM, 0)
	ROM_LOAD("rs232pio_dsr.u1", 0x0000, 0x1000, CRC(eab382fb) SHA1(ee609a18a21f1a3ddab334e8798d5f2a0fcefa91)) /* TI rs232 DSR ROM */
ROM_END

INPUT_PORTS_START( ti_rs232 )
	PORT_START( "CRURS232"  )
	PORT_DIPNAME( 0x01, 0x00, "TI-RS232 CRU base" )
		PORT_DIPSETTING(    0x00, "1300" )
		PORT_DIPSETTING(    0x01, "1500" )

	PORT_START( "SERIALMAP" )
	PORT_CONFNAME( 0x03, 0x00, "Serial cable pin mapping" )
		PORT_CONFSETTING(    0x00, "6-20" )
		PORT_CONFSETTING(    0x01, "8-20" )
		PORT_CONFSETTING(    0x02, "5-20" )
INPUT_PORTS_END

void ti_rs232_pio_device::device_add_mconfig(machine_config &config)
{
	TMS9902(config, m_uart0, 3000000);
	m_uart0->int_cb().set(FUNC(ti_rs232_pio_device::int0_callback));
	m_uart0->rcv_cb().set(FUNC(ti_rs232_pio_device::rcv0_callback));
	m_uart0->xmit_cb().set(FUNC(ti_rs232_pio_device::xmit0_callback));
	m_uart0->ctrl_cb().set(FUNC(ti_rs232_pio_device::ctrl0_callback));
	TMS9902(config, m_uart1, 3000000);
	m_uart1->int_cb().set(FUNC(ti_rs232_pio_device::int1_callback));
	m_uart1->rcv_cb().set(FUNC(ti_rs232_pio_device::rcv1_callback));
	m_uart1->xmit_cb().set(FUNC(ti_rs232_pio_device::xmit1_callback));
	m_uart1->ctrl_cb().set(FUNC(ti_rs232_pio_device::ctrl1_callback));
	TI99_RS232_DEV(config, m_serdev0, 0);
	m_serdev0->connect(m_uart0);
	TI99_RS232_DEV(config, m_serdev1, 0);
	m_serdev1->connect(m_uart1);

	TI99_PIO_DEV(config, m_piodev, 0);

	LS259(config, m_crulatch); // U12
	m_crulatch->q_out_cb<0>().set(FUNC(ti_rs232_pio_device::selected_w));
	m_crulatch->q_out_cb<1>().set(FUNC(ti_rs232_pio_device::pio_direction_in_w));
	m_crulatch->q_out_cb<2>().set(FUNC(ti_rs232_pio_device::pio_handshake_out_w));
	m_crulatch->q_out_cb<3>().set(FUNC(ti_rs232_pio_device::pio_spareout_w));
	m_crulatch->q_out_cb<4>().set(FUNC(ti_rs232_pio_device::flag0_w));
	m_crulatch->q_out_cb<5>().set(FUNC(ti_rs232_pio_device::cts0_w));
	m_crulatch->q_out_cb<6>().set(FUNC(ti_rs232_pio_device::cts1_w));
	m_crulatch->q_out_cb<7>().set(FUNC(ti_rs232_pio_device::led_w));
}

const tiny_rom_entry *ti_rs232_pio_device::device_rom_region() const
{
	return ROM_NAME( ti_rs232 );
}

ioport_constructor ti_rs232_pio_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(ti_rs232);
}

} } } // end namespace bus::ti99::peb
