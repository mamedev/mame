// license:BSD-3-Clause
// copyright-holders:smf, Carl
/**********************************************************************

    National Semiconductor 8250 UART interface and emulation

   More information on the different models can be found in
   section 1.6 at this location:
     http://www.freebsd.org/doc/en_US.ISO8859-1/articles/serial-uart/

Model overview (from page above):

INS8250
This part was used in the original IBM PC and IBM PC/XT. The original name
for this part was the INS8250 ACE (Asynchronous Communications Element) and
it is made from NMOS technology.

The 8250 uses eight I/O ports and has a one-byte send and a one-byte receive
buffer. This original UART has several race conditions and other flaws. The
original IBM BIOS includes code to work around these flaws, but this made
the BIOS dependent on the flaws being present, so subsequent parts like the
8250A, 16450 or 16550 could not be used in the original IBM PC or IBM PC/XT.

The original 8250 pulses the interrupt line if a higher priority interrupt is
cleared but a lower priority one is still active.  It also clears the tsre bit
for a moment before loading the tsr from the thr.  These may be the bugs the
PC and XT depend on as the 8250A and up fix them.

INS8250-B
This is the slower speed of the INS8250 made from NMOS technology. It contains
the same problems as the original INS8250.

INS8250A
An improved version of the INS8250 using XMOS technology with various functional
flaws corrected. The INS8250A was used initially in PC clone computers by vendors
who used "clean" BIOS designs. Because of the corrections in the chip, this part
could not be used with a BIOS compatible with the INS8250 or INS8250B.

INS82C50A
This is a CMOS version (low power consumption) of the INS8250A and has similar
functional characteristics.

NS16450
Same as NS8250A with improvements so it can be used with faster CPU bus designs.
IBM used this part in the IBM AT and updated the IBM BIOS to no longer rely on
the bugs in the INS8250.

NS16C450
This is a CMOS version (low power consumption) of the NS16450.

NS16550
Same as NS16450 with a 16-byte send and receive buffer but the buffer design
was flawed and could not be reliably be used.

The 16550 sometimes will send more then one character over the bus from the fifo
when the rbr is read making the rx fifo useless.  It's unlikely anything depends
on this behavior.

NS16550A
Same as NS16550 with the buffer flaws corrected. The 16550A and its successors
have become the most popular UART design in the PC industry, mainly due to
its ability to reliably handle higher data rates on operating systems with
sluggish interrupt response times.

NS16C552
This component consists of two NS16C550A CMOS UARTs in a single package.

PC16550D
Same as NS16550A with subtle flaws corrected. This is revision D of the
16550 family and is the latest design available from National Semiconductor.

Intel 82050
Essentially a NS16450 squeezed into a 28-pin package with some minor functions
eliminated. Can be strapped for either a 18.432 MHz XTAL (divided by 10 to
produce the BRG clock) or an externally generated 9.216 or 18.432 MHz clock.

Intel 82510
A functional expansion of the 82050 with dozens of extra registers. Adds 4-byte
Tx/Rx FIFOs with programmable thresholds, MCS-51 compatible 9-bit protocol,
ASCII/EBCDIC control character recognition, timed interrupts and more.


Known issues:
- MAME does currently not handle all these model specific features.


History:
    KT - 14-Jun-2000 - Improved Interrupt setting/clearing
    KT - moved into separate file so it can be used in Super I/O emulation and
        any other system which uses a PC type COM port
    KT - 24-Jun-2000 - removed pc specific input port tests. More compatible
        with PCW16 and PCW16 doesn't requre the PC input port definitions
        which are not required by the PCW16 hardware

**********************************************************************/

#include "emu.h"
#include "ins8250.h"

#include <algorithm>

//#define VERBOSE 1
#include "logmacro.h"

DEFINE_DEVICE_TYPE(INS8250,  ins8250_device, "ins8250",  "National Semiconductor INS8250 UART")
DEFINE_DEVICE_TYPE(NS16450,  ns16450_device, "ns16450",  "National Semiconductor NS16450 UART")
DEFINE_DEVICE_TYPE(NS16550,  ns16550_device, "ns16550",  "National Semiconductor NS16550 UART")
DEFINE_DEVICE_TYPE(PC16552D, pc16552_device, "pc16552d", "National Semiconductor PC16552D UART")

ins8250_uart_device::ins8250_uart_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, dev_type device_type)
	: device_t(mconfig, type, tag, owner, clock)
	, device_serial_interface(mconfig, *this)
	, m_device_type(device_type)
	, m_regs{0}
	, m_out_tx_cb(*this)
	, m_out_dtr_cb(*this)
	, m_out_rts_cb(*this)
	, m_out_int_cb(*this)
	, m_out_out1_cb(*this)
	, m_out_out2_cb(*this)
	, m_rxd(1)
	, m_dcd(1)
	, m_dsr(1)
	, m_ri(1)
	, m_cts(1)
{
	m_regs.ier = 0;
}

ins8250_device::ins8250_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: ins8250_uart_device(mconfig, INS8250, tag, owner, clock, dev_type::INS8250)
{
}

ns16450_device::ns16450_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: ins8250_uart_device(mconfig, NS16450, tag, owner, clock, dev_type::NS16450)
{
}

ns16550_device::ns16550_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: ins8250_uart_device(mconfig, NS16550, tag, owner, clock, dev_type::NS16550)
{
}

pc16552_device::pc16552_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, PC16552D, tag, owner, clock)
{
}

void pc16552_device::device_start()
{
	m_chan0 = subdevice<ns16550_device>("chan0");
	m_chan1 = subdevice<ns16550_device>("chan1");
}

/* int's pending */
#define COM_INT_PENDING_RECEIVED_DATA_AVAILABLE 0x0001
#define COM_INT_PENDING_TRANSMITTER_HOLDING_REGISTER_EMPTY 0x0002
#define COM_INT_PENDING_RECEIVER_LINE_STATUS 0x0004
#define COM_INT_PENDING_MODEM_STATUS_REGISTER 0x0008
#define COM_INT_PENDING_CHAR_TIMEOUT 0x0011

static constexpr u8 INS8250_LSR_TSRE = 0x40;
static constexpr u8 INS8250_LSR_THRE = 0x20;
static constexpr u8 INS8250_LSR_BI = 0x10;
static constexpr u8 INS8250_LSR_FE = 0x08;
static constexpr u8 INS8250_LSR_PE = 0x04;
static constexpr u8 INS8250_LSR_OE = 0x02;
static constexpr u8 INS8250_LSR_DR = 0x01;

static constexpr u8 INS8250_MCR_DTR = 0x01;
static constexpr u8 INS8250_MCR_RTS = 0x02;
static constexpr u8 INS8250_MCR_OUT1 = 0x04;
static constexpr u8 INS8250_MCR_OUT2 = 0x08;
static constexpr u8 INS8250_MCR_LOOPBACK = 0x10;

static constexpr u8 INS8250_LCR_BITCOUNT_MASK= 0x03;
static constexpr u8 INS8250_LCR_2STOP_BITS = 0x04;
//static constexpr u8 INS8250_LCR_PEN = 0x08;
//static constexpr u8 INS8250_LCR_EVEN_PAR = 0x10;
//static constexpr u8 INS8250_LCR_PARITY = 0x20;
static constexpr u8 INS8250_LCR_BREAK = 0x40;
static constexpr u8 INS8250_LCR_DLAB = 0x80;

/* ints will continue to be set for as long as there are ints pending */
void ins8250_uart_device::update_interrupt()
{
	int state;

	/* if any bits are set and are enabled */
	if (((m_int_pending & m_regs.ier) & 0x0f) != 0)
	{
		/* trigger next highest priority int */

		/* set int */
		state = 1;
		m_regs.iir &= ~(0x08|0x04|0x02);

		/* highest to lowest */
		if (m_regs.ier & m_int_pending & COM_INT_PENDING_RECEIVER_LINE_STATUS)
			m_regs.iir |=0x04|0x02;
		else if (m_regs.ier & m_int_pending & COM_INT_PENDING_RECEIVED_DATA_AVAILABLE)
		{
			m_regs.iir |=0x04;
			if ((m_int_pending & COM_INT_PENDING_CHAR_TIMEOUT) == 0x11)
				m_regs.iir |= 0x08;
		}
		else if (m_regs.ier & m_int_pending & COM_INT_PENDING_TRANSMITTER_HOLDING_REGISTER_EMPTY)
			m_regs.iir |=0x02;

		/* int pending */
		m_regs.iir &= ~0x01;
	}
	else
	{
		/* clear int */
		state = 0;

		/* no ints pending */
		m_regs.iir |= 0x01;
		/* priority level */
		m_regs.iir &= ~(0x08|0x04|0x02);
	}

	/* set or clear the int */
	m_out_int_cb(state);
}

/* set pending bit and trigger int */
void ins8250_uart_device::trigger_int(int flag)
{
	m_int_pending |= flag;
	update_interrupt();
}

/* clear pending bit, if any ints are pending, then int will be triggered, otherwise it
will be cleared */
void ins8250_uart_device::clear_int(int flag)
{
	m_int_pending &= ~flag;
	update_interrupt();
}

int ins8250_uart_device::intrpt_r()
{
	return !BIT(m_regs.iir, 0);
}

// Baud rate generator is reset after writing to either byte of divisor latch
void ins8250_uart_device::update_baud_rate()
{
	LOG("%.1f baud selected (divisor = %d)\n", double(clock()) / (m_regs.dl * 16), m_regs.dl);
	set_rate(clock(), m_regs.dl * 16);

	// FIXME: Baud rate generator should not affect transmitter or receiver, but device_serial_interface resets them regardless.
	// If the transmitter is still running at this time and we don't flush it, the shift register will never be emptied!
	if (!(m_regs.lsr & INS8250_LSR_TSRE))
		tra_complete();
}

void ins8250_uart_device::ins8250_w(offs_t offset, u8 data)
{
	int tmp;

	switch (offset)
	{
		case 0:
			if (m_regs.lcr & INS8250_LCR_DLAB)
			{
				m_regs.dl = (m_regs.dl & 0xff00) | data;
				update_baud_rate();
			}
			else
			{
				m_regs.thr = data;
				m_regs.lsr &= ~INS8250_LSR_THRE;
				if((m_device_type >= dev_type::NS16550) && (m_regs.fcr & 1))
					push_tx(data);
				clear_int(COM_INT_PENDING_TRANSMITTER_HOLDING_REGISTER_EMPTY);
				if(m_regs.lsr & INS8250_LSR_TSRE)
					tra_complete();
			}
			break;
		case 1:
			if (m_regs.lcr & INS8250_LCR_DLAB)
			{
				m_regs.dl = (m_regs.dl & 0xff) | (data << 8);
				update_baud_rate();
			}
			else
			{
				if ((m_regs.lsr & INS8250_LSR_THRE) && (data & COM_INT_PENDING_TRANSMITTER_HOLDING_REGISTER_EMPTY))
					trigger_int(COM_INT_PENDING_TRANSMITTER_HOLDING_REGISTER_EMPTY);
				m_regs.ier = data;
				update_interrupt();
			}
			break;
		case 2:
			set_fcr(data);
			break;
		case 3:
			{
			bool break_state_changed = bool((m_regs.lcr ^ data) & INS8250_LCR_BREAK);
			bool data_frame_changed = bool((m_regs.lcr ^ data) & 0x3f);

			m_regs.lcr = data;

			int data_bit_count = (m_regs.lcr & INS8250_LCR_BITCOUNT_MASK) + 5;
			parity_t parity;
			stop_bits_t stop_bits;

			switch ((m_regs.lcr>>3) & 7)
			{
			case 1:
				parity = PARITY_ODD;
				break;

			case 3:
				parity = PARITY_EVEN;
				break;

			case 5:
				parity = PARITY_MARK;
				break;

			case 7:
				parity = PARITY_SPACE;
				break;

			default:
				parity = PARITY_NONE;
				break;
			}

			if (!(m_regs.lcr & INS8250_LCR_2STOP_BITS))
				stop_bits = STOP_BITS_1;
			else if (data_bit_count == 5)
				stop_bits = STOP_BITS_1_5;
			else
				stop_bits = STOP_BITS_2;

			if (break_state_changed)
			{
				int new_out_val = (m_regs.lcr & INS8250_LCR_BREAK) ? 0 : m_txd;

				if (m_regs.mcr & INS8250_MCR_LOOPBACK)
				{
					device_serial_interface::rx_w(new_out_val);
				}
				else
				{
					m_out_tx_cb(new_out_val);
				}
			}
			if (data_frame_changed)
			{
				set_data_frame(1, data_bit_count, parity, stop_bits);
			}
			}
			break;
		case 4:
			if ( ( m_regs.mcr & 0x1f ) != ( data & 0x1f ) )
			{
				m_regs.mcr = data & 0x1f;

				update_msr();

				if (m_regs.mcr & INS8250_MCR_LOOPBACK)
				{
					m_out_tx_cb(1);
					if ((m_regs.lcr & INS8250_LCR_BREAK) == 0)
					{
						device_serial_interface::rx_w(m_txd);
					}
					m_out_dtr_cb(1);
					m_out_rts_cb(1);
					m_out_out1_cb(1);
					m_out_out2_cb(1);
				}
				else
				{
					if ((m_regs.lcr & INS8250_LCR_BREAK) == 0)
					{
						m_out_tx_cb(m_txd);
					}
					device_serial_interface::rx_w(m_rxd);
					m_out_dtr_cb((m_regs.mcr & INS8250_MCR_DTR) ? 0 : 1);
					m_out_rts_cb((m_regs.mcr & INS8250_MCR_RTS) ? 0 : 1);
					m_out_out1_cb((m_regs.mcr & INS8250_MCR_OUT1) ? 0 : 1);
					m_out_out2_cb((m_regs.mcr & INS8250_MCR_OUT2) ? 0 : 1);
				}
			}
			break;
		case 5:
			/*
			  This register can be written, but if you write a 1 bit into any of
			  bits 5 - 0, you could cause an interrupt if the appropriate IER bit
			  is set.
			*/
			m_regs.lsr = (m_regs.lsr & (INS8250_LSR_TSRE|INS8250_LSR_THRE)) | (data & ~(INS8250_LSR_TSRE|INS8250_LSR_THRE));

			tmp = 0;
			tmp |= ( m_regs.lsr & INS8250_LSR_DR ) ? COM_INT_PENDING_RECEIVED_DATA_AVAILABLE : 0;
			tmp |= ( m_regs.lsr & 0x1e ) ? COM_INT_PENDING_RECEIVER_LINE_STATUS : 0;
			tmp |= ( m_regs.lsr & INS8250_LSR_THRE ) ? COM_INT_PENDING_TRANSMITTER_HOLDING_REGISTER_EMPTY : 0;
			trigger_int(tmp);

			break;
		case 6:
			/*
			  This register can be written, but if you write a 1 bit into any of
			  bits 3 - 0, you could cause an interrupt if the appropriate IER bit
			  is set.
			  Bits 7 - 4 are read-only.
			 */
			m_regs.msr = (m_regs.msr & 0xf0) | (data & 0x0f);

			if (m_regs.msr & 0x0f)
				trigger_int(COM_INT_PENDING_MODEM_STATUS_REGISTER);
			else
				clear_int(COM_INT_PENDING_MODEM_STATUS_REGISTER);
			break;
		case 7:
			if (m_device_type >= dev_type::INS8250A)
			{
				m_regs.scr = data;
			}
			break;
	}
}

u8 ins8250_uart_device::ins8250_r(offs_t offset)
{
	int data = 0x0ff;

	switch (offset)
	{
		case 0:
			if (m_regs.lcr & INS8250_LCR_DLAB)
				data = (m_regs.dl & 0xff);
			else
			{
				if (!machine().side_effects_disabled())
				{
					if ((m_device_type >= dev_type::NS16550) && (m_regs.fcr & 1))
						m_regs.rbr = pop_rx();
					else
					{
						clear_int(COM_INT_PENDING_RECEIVED_DATA_AVAILABLE);
						if (m_regs.lsr & INS8250_LSR_DR)
							m_regs.lsr &= ~INS8250_LSR_DR;
					}
				}
				data = m_regs.rbr;
			}
			break;
		case 1:
			if (m_regs.lcr & INS8250_LCR_DLAB)
				data = (m_regs.dl >> 8);
			else
				data = m_regs.ier & 0x0f;
			break;
		case 2:
			data = m_regs.iir;
			/* The documentation says that reading this register will
			clear the int if this is the source of the int */
			if (!machine().side_effects_disabled())
			{
				if ((m_regs.iir & 0x0f) == 0x02)
					clear_int(COM_INT_PENDING_TRANSMITTER_HOLDING_REGISTER_EMPTY);
			}
			break;
		case 3:
			data = m_regs.lcr;
			break;
		case 4:
			data = m_regs.mcr;
			break;
		case 5:
			data = m_regs.lsr;
			if (!machine().side_effects_disabled() && (m_regs.lsr & (INS8250_LSR_BI | INS8250_LSR_FE | INS8250_LSR_PE | INS8250_LSR_OE)) != 0)
			{
				m_regs.lsr &= 0xe1; /* clear FE, PE and OE and BREAK bits */

				/* reading line status register clears int */
				clear_int(COM_INT_PENDING_RECEIVER_LINE_STATUS);
			}
			break;
		case 6:
			data = m_regs.msr;
			if (!machine().side_effects_disabled())
			{
				m_regs.msr &= 0xf0; /* reset delta values */

				/* reading msr clears int */
				clear_int(COM_INT_PENDING_MODEM_STATUS_REGISTER);
			}
			break;
		case 7:
			if (m_device_type >= dev_type::INS8250A)
			{
				data = m_regs.scr;
			}
			break;
	}
	return data;
}

void ns16550_device::rcv_complete()
{
	if(!(m_regs.fcr & 1))
		return ins8250_uart_device::rcv_complete();

	receive_register_extract();

	if(m_rnum == 16)
	{
		m_regs.lsr |= INS8250_LSR_OE; //overrun
		trigger_int(COM_INT_PENDING_RECEIVER_LINE_STATUS);
		return;
	}

	u8 errors = 0;
	if (is_receive_framing_error())
		errors |= INS8250_LSR_FE;
	if (is_receive_parity_error())
		errors |= INS8250_LSR_PE;
	if (m_rnum == 0 && errors != 0)
	{
		m_regs.lsr |= errors;
		trigger_int(COM_INT_PENDING_RECEIVER_LINE_STATUS);
	}

	m_regs.lsr |= INS8250_LSR_DR;
	m_rfifo[m_rhead] = get_received_char();
	m_efifo[m_rhead] = errors;
	++m_rhead &= 0x0f;
	m_rnum++;
	if(m_rnum >= m_rintlvl)
		trigger_int(COM_INT_PENDING_RECEIVED_DATA_AVAILABLE);
	set_timer();
}

void ns16550_device::tra_complete()
{
	if(!(m_regs.fcr & 1))
		return ins8250_uart_device::tra_complete();

	if(m_ttail != m_thead)
	{
		transmit_register_setup(m_tfifo[m_ttail]);
		++m_ttail &= 0x0f;
		m_regs.lsr &= ~INS8250_LSR_TSRE;
		if(m_ttail == m_thead)
		{
			m_regs.lsr |= INS8250_LSR_THRE;
			trigger_int(COM_INT_PENDING_TRANSMITTER_HOLDING_REGISTER_EMPTY);
		}
	}
	else
		m_regs.lsr |= 0x40;
}

void ins8250_uart_device::rcv_complete()
{
	// According to datasheet (and HP82939 self-test) the received character is always
	// extracted and stored in RBR even in case of overrun
	receive_register_extract();
	if(m_regs.lsr & INS8250_LSR_DR)
	{
		m_regs.lsr |= INS8250_LSR_OE; //overrun
	}
	m_regs.lsr |= INS8250_LSR_DR;

	if (is_receive_framing_error())
		m_regs.lsr |= INS8250_LSR_FE;
	if (is_receive_parity_error())
		m_regs.lsr |= INS8250_LSR_PE;
	if ((m_regs.lsr & (INS8250_LSR_BI | INS8250_LSR_PE | INS8250_LSR_FE | INS8250_LSR_OE)) != 0)
		trigger_int(COM_INT_PENDING_RECEIVER_LINE_STATUS);

	m_regs.rbr = get_received_char();
	trigger_int(COM_INT_PENDING_RECEIVED_DATA_AVAILABLE);
}

void ins8250_uart_device::tra_complete()
{
	if(!(m_regs.lsr & INS8250_LSR_THRE))
	{
		transmit_register_setup(m_regs.thr);
		m_regs.lsr &= ~INS8250_LSR_TSRE;
		m_regs.lsr |= INS8250_LSR_THRE;
		trigger_int(COM_INT_PENDING_TRANSMITTER_HOLDING_REGISTER_EMPTY);
	}
	else
		m_regs.lsr |= INS8250_LSR_TSRE;
}

void ins8250_uart_device::tra_callback()
{
	m_txd = transmit_register_get_data_bit();

	if (m_regs.lcr & INS8250_LCR_BREAK)
	{
		// in break mode, don't change transmitted bit
		return;
	}

	if (m_regs.mcr & INS8250_MCR_LOOPBACK)
	{
		device_serial_interface::rx_w(m_txd);
	}
	else
	{
		m_out_tx_cb(m_txd);
	}
}

void ins8250_uart_device::update_msr()
{
	u8 data;
	int change;

	if (m_regs.mcr & INS8250_MCR_LOOPBACK)
	{
		data = (((m_regs.mcr & (INS8250_MCR_OUT1|INS8250_MCR_OUT2)) << 4) |
			((m_regs.mcr & INS8250_MCR_DTR) << 5) | ((m_regs.mcr & INS8250_MCR_RTS) << 3));
		change = (m_regs.msr ^ data) >> 4;
		if(!(m_regs.msr & 0x40) && (data & 0x40))
			change &= ~4;
	}
	else
	{
		data = (!m_dcd << 7) | (!m_ri << 6) | (!m_dsr << 5) | (!m_cts << 4);
		change = (m_regs.msr ^ data) >> 4;
	}

	m_regs.msr = data | change;

	if(change)
		trigger_int(COM_INT_PENDING_MODEM_STATUS_REGISTER);
}

void ins8250_uart_device::dcd_w(int state)
{
	m_dcd = state;
	update_msr();
}

void ins8250_uart_device::dsr_w(int state)
{
	m_dsr = state;
	update_msr();
}

void ins8250_uart_device::ri_w(int state)
{
	m_ri = state;
	update_msr();
}

void ins8250_uart_device::cts_w(int state)
{
	m_cts = state;
	update_msr();
}

void ins8250_uart_device::rx_w(int state)
{
	m_rxd = state;

	if (!(m_regs.mcr & INS8250_MCR_LOOPBACK))
		device_serial_interface::rx_w(m_rxd);
}

void ins8250_uart_device::device_start()
{
	set_tra_rate(0);
	set_rcv_rate(0);

	save_item(NAME(m_regs.thr));
	save_item(NAME(m_regs.rbr));
	save_item(NAME(m_regs.ier));
	save_item(NAME(m_regs.dl));
	save_item(NAME(m_regs.iir));
	save_item(NAME(m_regs.fcr));
	save_item(NAME(m_regs.lcr));
	save_item(NAME(m_regs.mcr));
	save_item(NAME(m_regs.lsr));
	save_item(NAME(m_regs.msr));
	save_item(NAME(m_regs.scr));
	save_item(NAME(m_int_pending));
	save_item(NAME(m_txd));
	save_item(NAME(m_rxd));
	save_item(NAME(m_dcd));
	save_item(NAME(m_dsr));
	save_item(NAME(m_ri));
	save_item(NAME(m_cts));
}

void ins8250_uart_device::device_reset()
{
	m_regs.ier = 0;
	m_regs.iir = 1;
	m_regs.lcr = 0;
	m_regs.mcr = 0;
	m_regs.lsr = INS8250_LSR_THRE | INS8250_LSR_TSRE;
	update_msr();
	m_regs.msr &= 0xf0;
	m_int_pending = 0;
	update_interrupt();
	receive_register_reset();
	transmit_register_reset();
	m_txd = 1;
	m_out_tx_cb(1);
	m_out_rts_cb(1);
	m_out_dtr_cb(1);
	m_out_out1_cb(1);
	m_out_out2_cb(1);
}

void ns16550_device::device_start()
{
	m_timeout = timer_alloc(FUNC(ns16550_device::timeout_expired), this);
	ins8250_uart_device::device_start();
	save_item(NAME(m_rintlvl));
	save_item(NAME(m_rfifo));
	save_item(NAME(m_efifo));
	save_item(NAME(m_tfifo));
	save_item(NAME(m_rhead));
	save_item(NAME(m_rtail));
	save_item(NAME(m_rnum));
	save_item(NAME(m_thead));
	save_item(NAME(m_ttail));
}

void ns16550_device::device_reset()
{
	std::fill(std::begin(m_rfifo), std::end(m_rfifo), 0);
	std::fill(std::begin(m_efifo), std::end(m_efifo), 0);
	std::fill(std::begin(m_tfifo), std::end(m_tfifo), 0);
	m_rhead = m_rtail = m_rnum = 0;
	m_thead = m_ttail = 0;
	m_timeout->adjust(attotime::never);
	ins8250_uart_device::device_reset();
}

TIMER_CALLBACK_MEMBER(ns16550_device::timeout_expired)
{
	trigger_int(COM_INT_PENDING_CHAR_TIMEOUT);
	m_timeout->adjust(attotime::never);
}

void ns16550_device::push_tx(u8 data)
{
	m_tfifo[m_thead] = data;
	++m_thead &= 0x0f;
}

u8 ns16550_device::pop_rx()
{
	u8 data = m_rfifo[m_rtail];
	clear_int(COM_INT_PENDING_CHAR_TIMEOUT & ~1); // don't clear bit 1 yet

	if(m_rnum)
	{
		++m_rtail &= 0x0f;
		m_rnum--;
		if (m_rnum > 0 && m_efifo[m_rtail] != 0)
		{
			m_regs.lsr |= m_efifo[m_rtail];
			trigger_int(COM_INT_PENDING_RECEIVER_LINE_STATUS);
		}
	}
	else
		data = 0;

	if(m_rnum < m_rintlvl)
		clear_int(COM_INT_PENDING_RECEIVED_DATA_AVAILABLE);

	if(m_rnum)
		set_timer();
	else
	{
		m_timeout->adjust(attotime::never);
		m_regs.lsr &= ~INS8250_LSR_DR;
	}

	return data;
}

void ns16550_device::set_fcr(u8 data)
{
	const int bytes_per_int[] = {1, 4, 8, 14};
	if(!(data & 1))
	{
		m_regs.fcr = 0;
		m_regs.iir &= ~0xc8;
		return;
	}
	if(!(m_regs.fcr & 1) && (data & 1))
		data |= 0x06;
	if(data & 2)
	{
		std::fill(std::begin(m_rfifo), std::end(m_rfifo), 0);
		std::fill(std::begin(m_efifo), std::end(m_efifo), 0);
		m_rhead = m_rtail = m_rnum = 0;
		clear_int(COM_INT_PENDING_CHAR_TIMEOUT | COM_INT_PENDING_RECEIVED_DATA_AVAILABLE);
		m_timeout->adjust(attotime::never);
	}
	if(data & 4)
	{
		std::fill(std::begin(m_tfifo), std::end(m_tfifo), 0);
		m_thead = m_ttail = 0;
		m_regs.lsr |= INS8250_LSR_THRE;
		trigger_int(COM_INT_PENDING_TRANSMITTER_HOLDING_REGISTER_EMPTY);
	}
	m_rintlvl = bytes_per_int[(data>>6)&3];
	m_regs.iir |= 0xc0;
	m_regs.fcr = data & 0xc9;
}
