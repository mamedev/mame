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


Known issues:
- MESS does currently not handle all these model specific features.


History:
    KT - 14-Jun-2000 - Improved Interrupt setting/clearing
    KT - moved into separate file so it can be used in Super I/O emulation and
        any other system which uses a PC type COM port
    KT - 24-Jun-2000 - removed pc specific input port tests. More compatible
        with PCW16 and PCW16 doesn't requre the PC input port definitions
        which are not required by the PCW16 hardware

**********************************************************************/

#include "machine/ins8250.h"

const device_type INS8250 = &device_creator<ins8250_device>;
const device_type NS16450 = &device_creator<ns16450_device>;
const device_type NS16550 = &device_creator<ns16550_device>;

ins8250_uart_device::ins8250_uart_device(const machine_config &mconfig, device_type type, const char* name, const char *tag, device_t *owner, UINT32 clock)
		: device_t(mconfig, type, name, tag, owner, clock),
		  device_serial_interface(mconfig, *this)
{
}

ins8250_device::ins8250_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: ins8250_uart_device(mconfig, INS8250, "ins8250", tag, owner, clock)
{
	m_device_type = TYPE_INS8250;
}

ns16450_device::ns16450_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: ins8250_uart_device(mconfig, NS16450, "ns16450", tag, owner, clock)
{
	m_device_type = TYPE_NS16450;
}

ns16550_device::ns16550_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: ins8250_uart_device(mconfig, NS16550, "ns16550", tag, owner, clock)
{
	m_device_type = TYPE_NS16550;
}

/* int's pending */
#define COM_INT_PENDING_RECEIVED_DATA_AVAILABLE	0x0001
#define COM_INT_PENDING_TRANSMITTER_HOLDING_REGISTER_EMPTY 0x0002
#define COM_INT_PENDING_RECEIVER_LINE_STATUS 0x0004
#define COM_INT_PENDING_MODEM_STATUS_REGISTER 0x0008

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
		m_regs.iir &= ~(0x04|0x02);

		/* highest to lowest */
		if (m_regs.ier & m_int_pending & COM_INT_PENDING_RECEIVER_LINE_STATUS)
			m_regs.iir |=0x04|0x02;
		else if (m_regs.ier & m_int_pending & COM_INT_PENDING_RECEIVED_DATA_AVAILABLE)
			m_regs.iir |=0x04;
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
		m_regs.iir &= ~(0x04|0x02);
	}

	/* set or clear the int */
	m_out_int_func(state);
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

void ins8250_uart_device::update_clock()
{
	int baud;
	if(m_regs.dl == 0)
	{
		set_tra_rate(0);
		set_rcv_rate(0);
		return;
	}
	baud = clock()/(m_regs.dl*16);
	set_tra_rate(baud);
	set_rcv_rate(baud);
}

WRITE8_MEMBER( ins8250_uart_device::ins8250_w )
{
	int tmp;

	switch (offset)
	{
		case 0:
			if (m_regs.lcr & 0x80)
			{
				m_regs.dl = (m_regs.dl & 0xff00) | data;
				update_clock();
			}
			else
			{
				m_regs.thr = data;
				m_regs.lsr &= ~0x20;
				if ( m_regs.mcr & 0x10 )
				{
					m_regs.lsr &= ~0x40;
					m_regs.lsr |= 1;
					m_regs.rbr = data;
					trigger_int(COM_INT_PENDING_RECEIVED_DATA_AVAILABLE);
				}
				else
					if(m_regs.lsr & 0x40)
						tra_complete();
			}
			break;
		case 1:
			if (m_regs.lcr & 0x80)
			{
				m_regs.dl = (m_regs.dl & 0xff) | (data << 8);
				update_clock();
			}
			else
			{
				m_regs.ier = data;
				update_interrupt();
			}
            break;
		case 2:
            break;
		case 3:
			m_regs.lcr = data;
			switch ((m_regs.lcr>>3) & 7)
			{
			case 1:
				tmp = SERIAL_PARITY_ODD;
				break;
			case 3:
				tmp = SERIAL_PARITY_EVEN;
				break;
			case 5:
				tmp = SERIAL_PARITY_MARK;
				break;
			case 7:
				tmp = SERIAL_PARITY_SPACE;
				break;
			default:
				tmp = SERIAL_PARITY_NONE;
				break;
			}
			// if 5 data bits and stb = 1, stop bits is supposed to be 1.5
			set_data_frame((m_regs.lcr & 3) + 5, (m_regs.lcr & 4)?2:1, tmp);
			break;
		case 4:
			if ( ( m_regs.mcr & 0x1f ) != ( data & 0x1f ) )
			{
				m_regs.mcr = data & 0x1f;

				if ( m_regs.mcr & 0x10 )		/* loopback test */
				{
					data = ( ( m_regs.mcr & 0x0c ) << 4 ) | ( ( m_regs.mcr & 0x01 ) << 5 ) | ( ( m_regs.mcr & 0x02 ) << 3 );
					if ( ( m_regs.msr & 0x20 ) != ( data & 0x20 ) )
						data |= 0x02;
					if ( ( m_regs.msr & 0x10 ) != ( data & 0x10 ) )
						data |= 0x01;
					if ( ( m_regs.msr & 0x40 ) && ! ( data & 0x40 ) )
						data |= 0x04;
					if ( ( m_regs.msr & 0x80 ) != ( data & 0x80 ) )
						data |= 0x08;
					m_regs.msr = data;
				}
				else
				{
					m_out_dtr_func(m_regs.mcr & 1);
					m_out_rts_func((m_regs.mcr & 2) && 1);
					m_out_out1_func((m_regs.mcr & 4) && 1);
					m_out_out2_func((m_regs.mcr & 8) && 1);
				}
			}
            break;
		case 5:
			/*
              This register can be written, but if you write a 1 bit into any of
              bits 5 - 0, you could cause an interrupt if the appropriate IER bit
              is set.
            */
			m_regs.lsr = data;

			tmp = 0;
			tmp |= ( m_regs.lsr & 0x01 ) ? COM_INT_PENDING_RECEIVED_DATA_AVAILABLE : 0;
			tmp |= ( m_regs.lsr & 0x1e ) ? COM_INT_PENDING_RECEIVER_LINE_STATUS : 0;
			tmp |= ( m_regs.lsr & 0x20 ) ? COM_INT_PENDING_TRANSMITTER_HOLDING_REGISTER_EMPTY : 0;
			trigger_int(tmp);

			break;
		case 6:
			/*
              This register can be written, but if you write a 1 bit into any of
              bits 3 - 0, you could cause an interrupt if the appropriate IER bit
              is set.
             */
			m_regs.msr = data;

			if ( m_regs.msr & 0x0f )
				trigger_int(COM_INT_PENDING_MODEM_STATUS_REGISTER);
			break;
		case 7:
			m_regs.scr = data;
            break;
	}
}

READ8_MEMBER( ins8250_uart_device::ins8250_r )
{
	int data = 0x0ff;

	switch (offset)
	{
		case 0:
			if (m_regs.lcr & 0x80)
				data = (m_regs.dl & 0xff);
			else
			{
				data = m_regs.rbr;
				if( m_regs.lsr & 0x01 )
					m_regs.lsr &= ~0x01;		/* clear data ready status */

				clear_int(COM_INT_PENDING_RECEIVED_DATA_AVAILABLE);
			}
			break;
		case 1:
			if (m_regs.lcr & 0x80)
				data = (m_regs.dl >> 8);
			else
				data = m_regs.ier & 0x0f;
            break;
		case 2:
			data = m_regs.iir;
			/* The documentation says that reading this register will
            clear the int if this is the source of the int */
			if ( m_regs.ier & COM_INT_PENDING_TRANSMITTER_HOLDING_REGISTER_EMPTY )
				clear_int(COM_INT_PENDING_TRANSMITTER_HOLDING_REGISTER_EMPTY);
            break;
		case 3:
			data = m_regs.lcr;
            break;
		case 4:
			data = m_regs.mcr;
            break;
		case 5:
			data = m_regs.lsr;
			if( m_regs.lsr & 0x1f )
				m_regs.lsr &= 0xe1; /* clear FE, PE and OE and BREAK bits */

			/* reading line status register clears int */
			clear_int(COM_INT_PENDING_RECEIVER_LINE_STATUS);
            break;
		case 6:
			data = m_regs.msr;
			m_regs.msr &= 0xf0; /* reset delta values */

			/* reading msr clears int */
			clear_int(COM_INT_PENDING_MODEM_STATUS_REGISTER);

			break;
		case 7:
			data = m_regs.scr;
            break;
	}
    return data;
}

void ins8250_uart_device::rcv_complete()
{
	if(m_regs.lsr & 0x01)
	{
		m_regs.lsr |= 0x02; //overrun
		trigger_int(COM_INT_PENDING_RECEIVER_LINE_STATUS);
		receive_register_reset();
	}
	else
	{
		m_regs.lsr |= 0x01;
		receive_register_extract();
		m_regs.rbr = get_received_char();
		trigger_int(COM_INT_PENDING_RECEIVED_DATA_AVAILABLE);
	}
}

void ins8250_uart_device::tra_complete()
{
	if(!(m_regs.lsr & 0x20))
	{
		transmit_register_setup(m_regs.thr);
		m_regs.lsr &= ~0x40;
		m_regs.lsr |= 0x20;
		trigger_int(COM_INT_PENDING_TRANSMITTER_HOLDING_REGISTER_EMPTY);
	}
	else
		m_regs.lsr |= 0x40;
}

void ins8250_uart_device::tra_callback()
{
	m_out_tx_func(transmit_register_get_data_bit());
}	

void ins8250_uart_device::update_msr(int bit, UINT8 state)
{
	UINT8 mask = (1<<bit);
	if((m_regs.msr & mask) == (state<<bit))
		return;
	m_regs.msr |= mask;
	m_regs.msr = (m_regs.msr & ~(mask << 4)) | (state<<(bit+4));
	trigger_int(COM_INT_PENDING_MODEM_STATUS_REGISTER);
}

WRITE_LINE_MEMBER(ins8250_uart_device::dcd_w)
{
	update_msr(3, (state&&1));
}

WRITE_LINE_MEMBER(ins8250_uart_device::dsr_w)
{
	update_msr(1, (state&&1));
}

WRITE_LINE_MEMBER(ins8250_uart_device::ri_w)
{
	update_msr(2, (state&&1));
}

WRITE_LINE_MEMBER(ins8250_uart_device::cts_w)
{
	update_msr(0, (state&&1));
}

void ins8250_uart_device::device_start()
{
	m_out_tx_func.resolve(m_out_tx_cb, *this);
	m_out_dtr_func.resolve(m_out_dtr_cb, *this);
	m_out_rts_func.resolve(m_out_rts_cb, *this);
	m_out_int_func.resolve(m_out_int_cb, *this);
	m_out_out1_func.resolve(m_out_out1_cb, *this);
	m_out_out2_func.resolve(m_out_out2_cb, *this);
	set_tra_rate(0);
	set_rcv_rate(0);
}

void ins8250_uart_device::device_reset()
{
	memset(&m_regs, '\0', sizeof(m_regs));
	m_regs.ier = 0;
	m_regs.iir = 1;
	m_regs.lcr = 0;
	m_regs.mcr = 0;
	m_regs.lsr = (1<<5) | (1<<6);
	m_int_pending = 0;
	receive_register_reset();
	transmit_register_reset();
	m_out_rts_func(0);
	m_out_dtr_func(0);
	m_out_out1_func(0);
	m_out_out2_func(0);
	m_out_tx_func(1);
}

void ins8250_uart_device::device_config_complete()
{
	const ins8250_interface *intf = reinterpret_cast<const ins8250_interface *>(static_config());
	if(intf != NULL)
	{
		*static_cast<ins8250_interface *>(this) = *intf;
	}
	else
	{
		memset(&m_out_tx_cb, 0, sizeof(m_out_tx_cb));
		memset(&m_out_dtr_cb, 0, sizeof(m_out_dtr_cb));
		memset(&m_out_rts_cb, 0, sizeof(m_out_rts_cb));
		memset(&m_out_int_cb, 0, sizeof(m_out_int_cb));
		memset(&m_out_out1_cb, 0, sizeof(m_out_out1_cb));
		memset(&m_out_out2_cb, 0, sizeof(m_out_out2_cb));
	}
}
