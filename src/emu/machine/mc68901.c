/**********************************************************************

    Motorola MC68901 Multi Function Peripheral emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

/*

    TODO:

    - daisy chaining
    - disable GPIO3/4 interrupts when timer A/B in pulse mode
    - spurious interrupt

        If you look at the MFP datasheet it is obvious that it can generate the conditions for a spurious interrupt.
        However the fact that they indeed happen in the ST is quite interesting.

        The MFP will generate a spurious interrupt if interrupts are disabled (by changing the IERA/IERB registers)
        at the 'precise point'. The precise point would be after the system (but not necessarily the CPU, see below)
        triggered an MFP interrupt, and before the CPU drives the interrupt acknowledge cycle.

        If the MFP was connected directly to the CPU, spurious interrupts probably couldn't happen. However in the
        ST, GLUE seats in the middle and handles all the interrupt timing. It is possible that GLUE introduces a
        delay between detecting a change in the MFP interrupt request signal and actually propagating the change to
        the CPU IPL signals (it is even possible that GLUE make some kind of latching). This would create a window
        long enough for the 'precise point' described above.

        "yes, the spurious interrupt occurs when i mask a timer. i did not notice an occurance of the SPI when changing data and control registers.
        if i kill interrupts with the status reg before masking the timer interrupt, then the SPI occurs as soon as the status register is set to re-enable interrupts."

        Well, more experiments show that it's somewhat incorrect, and
        the GLUE is essentially invisible w.r.t IPL.  The CPU and the
        MFP manage to add the delays all by themselves.

    - divide serial clock by 16
    - synchronous mode
    - 1.5/2 stop bits
    - interrupt on receiver break end
    - interrupt on character boundaries during break transmission
    - loopback mode

*/

#include "emu.h"
#include "mc68901.h"
#include "cpu/m68000/m68000.h"


// device type definition
const device_type MC68901 = &device_creator<mc68901_device>;


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0


enum
{
	REGISTER_GPIP = 0,
	REGISTER_AER,
	REGISTER_DDR,
	REGISTER_IERA,
	REGISTER_IERB,
	REGISTER_IPRA,
	REGISTER_IPRB,
	REGISTER_ISRA,
	REGISTER_ISRB,
	REGISTER_IMRA,
	REGISTER_IMRB,
	REGISTER_VR,
	REGISTER_TACR,
	REGISTER_TBCR,
	REGISTER_TCDCR,
	REGISTER_TADR,
	REGISTER_TBDR,
	REGISTER_TCDR,
	REGISTER_TDDR,
	REGISTER_SCR,
	REGISTER_UCR,
	REGISTER_RSR,
	REGISTER_TSR,
	REGISTER_UDR
};


enum
{
	INT_GPI0 = 0,
	INT_GPI1,
	INT_GPI2,
	INT_GPI3,
	INT_TIMER_D,
	INT_TIMER_C,
	INT_GPI4,
	INT_GPI5,
	INT_TIMER_B,
	INT_XMIT_ERROR,
	INT_XMIT_BUFFER_EMPTY,
	INT_RCV_ERROR,
	INT_RCV_BUFFER_FULL,
	INT_TIMER_A,
	INT_GPI6,
	INT_GPI7
};


enum
{
	GPIP_0 = 0,
	GPIP_1,
	GPIP_2,
	GPIP_3,
	GPIP_4,
	GPIP_5,
	GPIP_6,
	GPIP_7
};


enum
{
	TIMER_A = 0,
	TIMER_B,
	TIMER_C,
	TIMER_D,
	MAX_TIMERS
};


enum
{
	SERIAL_START = 0,
	SERIAL_DATA,
	SERIAL_PARITY,
	SERIAL_STOP
};


enum
{
	XMIT_OFF = 0,
	XMIT_STARTING,
	XMIT_ON,
	XMIT_BREAK,
	XMIT_STOPPING
};


#define AER_GPIP_0              0x01
#define AER_GPIP_1              0x02
#define AER_GPIP_2              0x04
#define AER_GPIP_3              0x08
#define AER_GPIP_4              0x10
#define AER_GPIP_5              0x20
#define AER_GPIP_6              0x40
#define AER_GPIP_7              0x80


#define VR_S                    0x08


#define IR_GPIP_0               0x0001
#define IR_GPIP_1               0x0002
#define IR_GPIP_2               0x0004
#define IR_GPIP_3               0x0008
#define IR_TIMER_D              0x0010
#define IR_TIMER_C              0x0020
#define IR_GPIP_4               0x0040
#define IR_GPIP_5               0x0080
#define IR_TIMER_B              0x0100
#define IR_XMIT_ERROR           0x0200
#define IR_XMIT_BUFFER_EMPTY    0x0400
#define IR_RCV_ERROR            0x0800
#define IR_RCV_BUFFER_FULL      0x1000
#define IR_TIMER_A              0x2000
#define IR_GPIP_6               0x4000
#define IR_GPIP_7               0x8000


#define TCR_TIMER_STOPPED       0x00
#define TCR_TIMER_DELAY_4       0x01
#define TCR_TIMER_DELAY_10      0x02
#define TCR_TIMER_DELAY_16      0x03
#define TCR_TIMER_DELAY_50      0x04
#define TCR_TIMER_DELAY_64      0x05
#define TCR_TIMER_DELAY_100     0x06
#define TCR_TIMER_DELAY_200     0x07
#define TCR_TIMER_EVENT         0x08
#define TCR_TIMER_PULSE_4       0x09
#define TCR_TIMER_PULSE_10      0x0a
#define TCR_TIMER_PULSE_16      0x0b
#define TCR_TIMER_PULSE_50      0x0c
#define TCR_TIMER_PULSE_64      0x0d
#define TCR_TIMER_PULSE_100     0x0e
#define TCR_TIMER_PULSE_200     0x0f
#define TCR_TIMER_RESET         0x10


#define UCR_PARITY_ENABLED      0x04
#define UCR_PARITY_EVEN         0x02
#define UCR_PARITY_ODD          0x00
#define UCR_WORD_LENGTH_8       0x00
#define UCR_WORD_LENGTH_7       0x20
#define UCR_WORD_LENGTH_6       0x40
#define UCR_WORD_LENGTH_5       0x60
#define UCR_START_STOP_0_0      0x00
#define UCR_START_STOP_1_1      0x08
#define UCR_START_STOP_1_15     0x10
#define UCR_START_STOP_1_2      0x18
#define UCR_CLOCK_DIVIDE_16     0x80
#define UCR_CLOCK_DIVIDE_1      0x00


#define RSR_RCV_ENABLE          0x01
#define RSR_SYNC_STRIP_ENABLE   0x02
#define RSR_MATCH               0x04
#define RSR_CHAR_IN_PROGRESS    0x04
#define RSR_FOUND_SEARCH        0x08
#define RSR_BREAK               0x08
#define RSR_FRAME_ERROR         0x10
#define RSR_PARITY_ERROR        0x20
#define RSR_OVERRUN_ERROR       0x40
#define RSR_BUFFER_FULL         0x80

#define TSR_XMIT_ENABLE         0x01
#define TSR_OUTPUT_HI_Z         0x00
#define TSR_OUTPUT_LOW          0x02
#define TSR_OUTPUT_HIGH         0x04
#define TSR_OUTPUT_LOOP         0x06
#define TSR_OUTPUT_MASK         0x06
#define TSR_BREAK               0x08
#define TSR_END_OF_XMIT         0x10
#define TSR_AUTO_TURNAROUND     0x20
#define TSR_UNDERRUN_ERROR      0x40
#define TSR_BUFFER_EMPTY        0x80


static const int INT_MASK_GPIO[] =
{
	IR_GPIP_0, IR_GPIP_1, IR_GPIP_2, IR_GPIP_3,
	IR_GPIP_4, IR_GPIP_5, IR_GPIP_6, IR_GPIP_7
};


static const int INT_MASK_TIMER[] =
{
	IR_TIMER_A, IR_TIMER_B, IR_TIMER_C, IR_TIMER_D
};


static const int GPIO_TIMER[] =
{
	GPIP_4, GPIP_3
};


static const int PRESCALER[] = { 0, 4, 10, 16, 50, 64, 100, 200 };


#define TXD(_data) m_out_so_func(_data);


//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

inline void mc68901_device::check_interrupts()
{
	if (m_ipr & m_imr)
	{
		m_out_irq_func(ASSERT_LINE);
	}
	else
	{
		m_out_irq_func(CLEAR_LINE);
	}
}

inline void mc68901_device::take_interrupt(UINT16 mask)
{
	m_ipr |= mask;

	check_interrupts();
}

inline void mc68901_device::rx_buffer_full()
{
	if (m_ier & IR_RCV_BUFFER_FULL)
	{
		take_interrupt(IR_RCV_BUFFER_FULL);
	}
}

inline void mc68901_device::rx_error()
{
	if (m_ier & IR_RCV_ERROR)
	{
		take_interrupt(IR_RCV_ERROR);
	}
	else
	{
		rx_buffer_full();
	}
}

inline void mc68901_device::tx_buffer_empty()
{
	if (m_ier & IR_XMIT_BUFFER_EMPTY)
	{
		take_interrupt(IR_XMIT_BUFFER_EMPTY);
	}
}

inline void mc68901_device::tx_error()
{
	if (m_ier & IR_XMIT_ERROR)
	{
		take_interrupt(IR_XMIT_ERROR);
	}
	else
	{
		tx_buffer_empty();
	}
}

inline int mc68901_device::get_parity_bit(UINT8 b)
{
	b ^= b >> 4;
	b ^= b >> 2;
	b ^= b >> 1;
	return b & 1;
}

inline void mc68901_device::serial_receive()
{
	int rxd;

	if (!(m_rsr & RSR_RCV_ENABLE)) return;

	rxd = m_in_si_func();

	switch (m_rx_state)
	{
	case SERIAL_START:
		if (!rxd)
		{
			m_rsr |= RSR_CHAR_IN_PROGRESS;
			m_rx_bits = 0;
			m_rx_buffer = 0;
			m_rx_state = SERIAL_DATA;
			m_next_rsr = RSR_BREAK;
		}
		break;

	case SERIAL_DATA:
		if ((m_next_rsr & RSR_BREAK) && (rxd == 1) && m_rsr_read)
		{
			m_next_rsr &= ~RSR_BREAK;
		}

		m_rx_buffer >>= 1;
		m_rx_buffer |= (rxd << 7);
		m_rx_bits++;

		if (m_rx_bits == m_rxtx_word)
		{
			if (m_rxtx_word < 8)
			{
				m_rx_buffer >>= (8 - m_rxtx_word);
			}

			m_rsr &= ~RSR_CHAR_IN_PROGRESS;

			if (m_ucr & UCR_PARITY_ENABLED)
			{
				m_rx_state = SERIAL_PARITY;
			}
			else
			{
				m_rx_state = SERIAL_STOP;
			}
		}
		break;

	case SERIAL_PARITY:
		m_rx_parity = rxd;

		if (m_rx_parity != (get_parity_bit(m_rx_buffer) ^ ((m_ucr & UCR_PARITY_EVEN) >> 1)))
		{
			m_next_rsr |= RSR_PARITY_ERROR;
		}

		m_rx_state = SERIAL_STOP;
		break;

	case SERIAL_STOP:
		if (rxd == 1)
		{
			if (!((m_rsr & RSR_SYNC_STRIP_ENABLE) && (m_rx_buffer == m_scr)))
			{
				if (!(m_rsr & RSR_OVERRUN_ERROR))
				{
					if (m_rsr & RSR_BUFFER_FULL)
					{
						// incoming word received but last word in receive buffer has not been read
						m_next_rsr |= RSR_OVERRUN_ERROR;
					}
					else
					{
						// incoming word received and receive buffer is empty
						m_rsr |= RSR_BUFFER_FULL;
						m_udr = m_rx_buffer;
						rx_buffer_full();
					}
				}
			}
		}
		else
		{
			if (m_rx_buffer)
			{
				// non-zero data word not followed by a stop bit
				m_next_rsr |= RSR_FRAME_ERROR;
			}
		}

		m_rx_state = SERIAL_START;
		break;
	}
}


inline void mc68901_device::tx_disabled()
{
	switch (m_tsr & TSR_OUTPUT_MASK)
	{
	case TSR_OUTPUT_HI_Z:
		/* indeterminate state */
	case TSR_OUTPUT_LOW:
		TXD(0);
		break;

	case TSR_OUTPUT_HIGH:
	case TSR_OUTPUT_LOOP:
		TXD(1);
		break;
	}
}


inline void mc68901_device::tx_starting()
{
	if (m_tsr & TSR_XMIT_ENABLE)
	{
		/* enable transmitter */
		TXD(1);
		m_xmit_state = XMIT_ON;
	}
	else
	{
		/* disable transmitter */
		m_tsr |= TSR_END_OF_XMIT;
		m_xmit_state = XMIT_OFF;
	}
}


inline void mc68901_device::tx_break()
{
	if (m_tsr & TSR_XMIT_ENABLE)
	{
		if (m_tsr & TSR_BREAK)
		{
			/* transmit break */
			TXD(1);
		}
		else
		{
			/* enable transmitter */
			m_xmit_state = XMIT_ON;
		}
	}
	else
	{
		/* disable transmitter */
		m_tsr |= TSR_END_OF_XMIT;
		m_xmit_state = XMIT_OFF;
	}
}


inline void mc68901_device::tx_enabled()
{
	switch (m_tx_state)
	{
	case SERIAL_START:
		if (m_tsr & TSR_UNDERRUN_ERROR)
		{
			/* buffer underrun condition */
			if (m_tsr & TSR_XMIT_ENABLE)
			{
				/* transmit break */
				TXD(1);
			}
			else
			{
				/* transmitter disabled */
				tx_disabled();
			}
		}
		else
		{
			if (m_tsr & TSR_BUFFER_EMPTY)
			{
				/* transmit buffer is empty, signal underrun error */
				m_tsr |= TSR_UNDERRUN_ERROR;

				if (m_tsr & TSR_XMIT_ENABLE)
				{
					/* transmit break */
					TXD(1);
				}
				else
				{
					/* transmitter disabled */
					tx_disabled();
				}
			}
			else
			{
				/* transmit start bit */
				TXD(0);

				/* load transmit buffer */
				m_tx_buffer = m_udr;
				m_tx_bits = 0;

				/* signal transmit buffer empty */
				m_tsr |= TSR_BUFFER_EMPTY;
				tx_buffer_empty();

				/* calculate parity */
				m_tx_parity = get_parity_bit(m_tx_buffer);

				/* next bit is data */
				m_tx_state = SERIAL_DATA;
			}
		}
		break;

	case SERIAL_DATA:
		/* transmit data bit */
		TXD(m_tx_buffer & 0x01);

		/* shift transmit buffer */
		m_tx_buffer >>= 1;
		m_tx_bits++;

		if (m_tx_bits == m_rxtx_word)
		{
			/* all data bits transferred */
			if (m_ucr & UCR_PARITY_ENABLED)
			{
				/* next bit is parity */
				m_tx_state = SERIAL_PARITY;
			}
			else
			{
				/* next bit is stop */
				m_tx_state = SERIAL_STOP;
			}
		}
		break;

	case SERIAL_PARITY:
		if (m_rxtx_word < 8)
		{
			/* transmit user-defined parity bit from buffer */
			TXD(m_tx_buffer & 0x01);
		}
		else
		{
			/* transmit calculated parity bit */
			TXD(m_tx_parity ^ ((m_ucr & UCR_PARITY_EVEN) >> 1));
		}

		/* next bit is stop */
		m_tx_state = SERIAL_STOP;
		break;

	case SERIAL_STOP:
		/* transmit stop bit */
		TXD(1);

		if (m_tsr & TSR_XMIT_ENABLE)
		{
			/* next bit is start */
			m_tx_state = SERIAL_START;
		}
		else
		{
			if (m_tsr & TSR_AUTO_TURNAROUND)
			{
				/* enable transmitter */
				m_tsr |= TSR_XMIT_ENABLE;

				/* next bit is start */
				m_tx_state = SERIAL_START;
			}
			else
			{
				/* disable transmitter */
				m_xmit_state = XMIT_OFF;
				m_tsr |= TSR_END_OF_XMIT;

				/* signal transmit error */
				tx_error();
			}
		}
		break;
	}
}

inline void mc68901_device::serial_transmit()
{
	switch (m_xmit_state)
	{
	case XMIT_OFF:      tx_disabled();  break;
	case XMIT_STARTING: tx_starting();  break;
	case XMIT_BREAK:    tx_break();     break;
	case XMIT_ON:       tx_enabled();   break;
	}
}


inline void mc68901_device::timer_count(int index)
{
	if (m_tmc[index] == 0x01)
	{
		/* toggle timer output signal */
		m_to[index] = !m_to[index];

		switch (index)
		{
		case TIMER_A:   m_out_tao_func(m_to[index]);    break;
		case TIMER_B:   m_out_tbo_func(m_to[index]);    break;
		case TIMER_C:   m_out_tco_func(m_to[index]);    break;
		case TIMER_D:   m_out_tdo_func(m_to[index]);    break;
		}

		if (m_ier & INT_MASK_TIMER[index])
		{
			/* signal timer elapsed interrupt */
			take_interrupt(INT_MASK_TIMER[index]);
		}

		/* load main counter */
		m_tmc[index] = m_tdr[index];
	}
	else
	{
		/* count down */
		m_tmc[index]--;
	}
}


inline void mc68901_device::timer_input(int index, int value)
{
	int bit = GPIO_TIMER[index];
	int aer = BIT(m_aer, bit);
	int cr = index ? m_tbcr : m_tacr;

	switch (cr & 0x0f)
	{
	case TCR_TIMER_EVENT:
		if (((m_ti[index] ^ aer) == 1) && ((value ^ aer) == 0))
		{
			timer_count(index);
		}

		m_ti[index] = value;
		break;

	case TCR_TIMER_PULSE_4:
	case TCR_TIMER_PULSE_10:
	case TCR_TIMER_PULSE_16:
	case TCR_TIMER_PULSE_50:
	case TCR_TIMER_PULSE_64:
	case TCR_TIMER_PULSE_100:
	case TCR_TIMER_PULSE_200:
		m_timer[index]->enable((value == aer));

		if (((m_ti[index] ^ aer) == 0) && ((value ^ aer) == 1))
		{
			if (m_ier & INT_MASK_GPIO[bit])
			{
				take_interrupt(INT_MASK_GPIO[bit]);
			}
		}

		m_ti[index] = value;
		break;
	}
}


inline void mc68901_device::gpio_input(int bit, int state)
{
	if (BIT(m_gpip, bit) && !state) // if transition from 1 to 0 is detected...
	{
		if (LOG) logerror("MC68901 '%s' Edge Transition Detected on GPIO%u\n", tag(), bit);

		if (m_ier & INT_MASK_GPIO[bit]) // AND interrupt enabled bit is set...
		{
			if (LOG) logerror("MC68901 '%s' Interrupt Pending for GPIO%u\n", tag(), bit);

			take_interrupt(INT_MASK_GPIO[bit]); // set interrupt pending bit
		}
	}

	m_gpip &= ((1 << bit) ^ 0xff);
	m_gpip |= (state << bit);
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mc68901_device - constructor
//-------------------------------------------------

mc68901_device::mc68901_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MC68901, "Motorola MC68901", tag, owner, clock),
		m_gpip(0),
		m_tsr(TSR_BUFFER_EMPTY)
{
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void mc68901_device::device_config_complete()
{
	// inherit a copy of the static data
	const mc68901_interface *intf = reinterpret_cast<const mc68901_interface *>(static_config());
	if (intf != NULL)
		*static_cast<mc68901_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
//      memset(&in_pa_cb, 0, sizeof(in_pa_cb));
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mc68901_device::device_start()
{
	/* resolve callbacks */
	m_in_gpio_func.resolve(m_in_gpio_cb, *this);
	m_out_gpio_func.resolve(m_out_gpio_cb, *this);
	m_in_si_func.resolve(m_in_si_cb, *this);
	m_out_so_func.resolve(m_out_so_cb, *this);
	m_out_tao_func.resolve(m_out_tao_cb, *this);
	m_out_tbo_func.resolve(m_out_tbo_cb, *this);
	m_out_tco_func.resolve(m_out_tco_cb, *this);
	m_out_tdo_func.resolve(m_out_tdo_cb, *this);
	m_out_irq_func.resolve(m_out_irq_cb, *this);

	/* create the timers */
	m_timer[TIMER_A] = timer_alloc(TIMER_A);
	m_timer[TIMER_B] = timer_alloc(TIMER_B);
	m_timer[TIMER_C] = timer_alloc(TIMER_C);
	m_timer[TIMER_D] = timer_alloc(TIMER_D);

	if (m_rx_clock > 0)
	{
		m_rx_timer = timer_alloc(TIMER_RX);
		m_rx_timer->adjust(attotime::zero, 0, attotime::from_hz(m_rx_clock));
	}

	if (m_tx_clock > 0)
	{
		m_tx_timer = timer_alloc(TIMER_TX);
		m_tx_timer->adjust(attotime::zero, 0, attotime::from_hz(m_tx_clock));
	}

	/* register for state saving */
	save_item(NAME(m_gpip));
	save_item(NAME(m_aer));
	save_item(NAME(m_ddr));
	save_item(NAME(m_ier));
	save_item(NAME(m_ipr));
	save_item(NAME(m_isr));
	save_item(NAME(m_imr));
	save_item(NAME(m_vr));
	save_item(NAME(m_tacr));
	save_item(NAME(m_tbcr));
	save_item(NAME(m_tcdcr));
	save_item(NAME(m_tdr));
	save_item(NAME(m_tmc));
	save_item(NAME(m_to));
	save_item(NAME(m_ti));
	save_item(NAME(m_scr));
	save_item(NAME(m_ucr));
	save_item(NAME(m_rsr));
	save_item(NAME(m_tsr));
	save_item(NAME(m_udr));
	save_item(NAME(m_rx_bits));
	save_item(NAME(m_tx_bits));
	save_item(NAME(m_rx_parity));
	save_item(NAME(m_tx_parity));
	save_item(NAME(m_rx_state));
	save_item(NAME(m_tx_state));
	save_item(NAME(m_rx_buffer));
	save_item(NAME(m_tx_buffer));
	save_item(NAME(m_xmit_state));
	save_item(NAME(m_rxtx_word));
	save_item(NAME(m_rxtx_start));
	save_item(NAME(m_rxtx_stop));
	save_item(NAME(m_rsr_read));
	save_item(NAME(m_next_rsr));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mc68901_device::device_reset()
{
	m_xmit_state = XMIT_OFF;
	m_rx_state = SERIAL_STOP;
	m_rx_buffer = 0;
	m_tx_buffer = 0;

	// Avoid read-before-write
	m_ipr = m_imr = 0;

	register_w(REGISTER_GPIP, 0);
	register_w(REGISTER_AER, 0);
	register_w(REGISTER_DDR, 0);
	register_w(REGISTER_IERA, 0);
	register_w(REGISTER_IERB, 0);
	register_w(REGISTER_IPRA, 0);
	register_w(REGISTER_IPRB, 0);
	register_w(REGISTER_ISRA, 0);
	register_w(REGISTER_ISRB, 0);
	register_w(REGISTER_IMRA, 0);
	register_w(REGISTER_IMRB, 0);
	register_w(REGISTER_VR, 0);
	register_w(REGISTER_TACR, 0);
	register_w(REGISTER_TBCR, 0);
	register_w(REGISTER_TCDCR, 0);
	register_w(REGISTER_SCR, 0);
	register_w(REGISTER_UCR, 0);
	register_w(REGISTER_RSR, 0);
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void mc68901_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_RX:
		serial_receive();
		break;

	case TIMER_TX:
		serial_transmit();
		break;

	default:
		timer_count(id);
		break;
	}
}



READ8_MEMBER( mc68901_device::read )
{
	switch (offset)
	{
	case REGISTER_GPIP:
		m_gpip = m_in_gpio_func(0);
		return m_gpip;

	case REGISTER_AER:   return m_aer;
	case REGISTER_DDR:   return m_ddr;

	case REGISTER_IERA:  return m_ier >> 8;
	case REGISTER_IERB:  return m_ier & 0xff;
	case REGISTER_IPRA:  return m_ipr >> 8;
	case REGISTER_IPRB:  return m_ipr & 0xff;
	case REGISTER_ISRA:  return m_isr >> 8;
	case REGISTER_ISRB:  return m_isr & 0xff;
	case REGISTER_IMRA:  return m_imr >> 8;
	case REGISTER_IMRB:  return m_imr & 0xff;
	case REGISTER_VR:    return m_vr;

	case REGISTER_TACR:  return m_tacr;
	case REGISTER_TBCR:  return m_tbcr;
	case REGISTER_TCDCR: return m_tcdcr;
	case REGISTER_TADR:  return m_tmc[TIMER_A];
	case REGISTER_TBDR:  return m_tmc[TIMER_B];
	case REGISTER_TCDR:  return m_tmc[TIMER_C];
	case REGISTER_TDDR:  return m_tmc[TIMER_D];

	case REGISTER_SCR:   return m_scr;
	case REGISTER_UCR:   return m_ucr;
	case REGISTER_RSR:
		m_rsr_read = 1;
		return m_rsr;

	case REGISTER_TSR:
		{
			/* clear UE bit (in reality, this won't be cleared until one full clock cycle of the transmitter has passed since the bit was set) */
			UINT8 tsr = m_tsr;
			m_tsr &= 0xbf;

			return tsr;
		}

	case REGISTER_UDR:
		/* load RSR with latched value */
		m_rsr = (m_next_rsr & 0x7c) | (m_rsr & 0x03);
		m_next_rsr = 0;

		if (m_rsr & 0x78)
		{
			/* signal receiver error interrupt */
			rx_error();
		}

		return m_udr;

	default:                      return 0;
	}
}


#define DIVISOR PRESCALER[data & 0x07]

void mc68901_device::register_w(offs_t offset, UINT8 data)
{
	switch (offset)
	{
	case REGISTER_GPIP:
		if (LOG) logerror("MC68901 '%s' General Purpose I/O : %x\n", tag(), data);
		m_gpip = data & m_ddr;

		m_out_gpio_func(0, m_gpip);
		break;

	case REGISTER_AER:
		if (LOG) logerror("MC68901 '%s' Active Edge Register : %x\n", tag(), data);
		m_aer = data;
		break;

	case REGISTER_DDR:
		if (LOG) logerror("MC68901 '%s' Data Direction Register : %x\n", tag(), data);
		m_ddr = data;
		break;

	case REGISTER_IERA:
		if (LOG) logerror("MC68901 '%s' Interrupt Enable Register A : %x\n", tag(), data);
		m_ier = (data << 8) | (m_ier & 0xff);
		m_ipr &= m_ier;
		check_interrupts();
		break;

	case REGISTER_IERB:
		if (LOG) logerror("MC68901 '%s' Interrupt Enable Register B : %x\n", tag(), data);
		m_ier = (m_ier & 0xff00) | data;
		m_ipr &= m_ier;
		check_interrupts();
		break;

	case REGISTER_IPRA:
		if (LOG) logerror("MC68901 '%s' Interrupt Pending Register A : %x\n", tag(), data);
		m_ipr &= (data << 8) | (m_ipr & 0xff);
		check_interrupts();
		break;

	case REGISTER_IPRB:
		if (LOG) logerror("MC68901 '%s' Interrupt Pending Register B : %x\n", tag(), data);
		m_ipr &= (m_ipr & 0xff00) | data;
		check_interrupts();
		break;

	case REGISTER_ISRA:
		if (LOG) logerror("MC68901 '%s' Interrupt In-Service Register A : %x\n", tag(), data);
		m_isr &= (data << 8) | (m_isr & 0xff);
		break;

	case REGISTER_ISRB:
		if (LOG) logerror("MC68901 '%s' Interrupt In-Service Register B : %x\n", tag(), data);
		m_isr &= (m_isr & 0xff00) | data;
		break;

	case REGISTER_IMRA:
		if (LOG) logerror("MC68901 '%s' Interrupt Mask Register A : %x\n", tag(), data);
		m_imr = (data << 8) | (m_imr & 0xff);
		m_isr &= m_imr;
		check_interrupts();
		break;

	case REGISTER_IMRB:
		if (LOG) logerror("MC68901 '%s' Interrupt Mask Register B : %x\n", tag(), data);
		m_imr = (m_imr & 0xff00) | data;
		m_isr &= m_imr;
		check_interrupts();
		break;

	case REGISTER_VR:
		if (LOG) logerror("MC68901 '%s' Interrupt Vector : %x\n", tag(), data & 0xf0);

		m_vr = data & 0xf8;

		if (m_vr & VR_S)
		{
			if (LOG) logerror("MC68901 '%s' Software End-Of-Interrupt Mode\n", tag());
		}
		else
		{
			if (LOG) logerror("MC68901 '%s' Automatic End-Of-Interrupt Mode\n", tag());

			m_isr = 0;
		}
		break;

	case REGISTER_TACR:
		m_tacr = data & 0x1f;

		switch (m_tacr & 0x0f)
		{
		case TCR_TIMER_STOPPED:
			if (LOG) logerror("MC68901 '%s' Timer A Stopped\n", tag());
			m_timer[TIMER_A]->enable(false);
			break;

		case TCR_TIMER_DELAY_4:
		case TCR_TIMER_DELAY_10:
		case TCR_TIMER_DELAY_16:
		case TCR_TIMER_DELAY_50:
		case TCR_TIMER_DELAY_64:
		case TCR_TIMER_DELAY_100:
		case TCR_TIMER_DELAY_200:
			{
			int divisor = PRESCALER[m_tacr & 0x07];
			if (LOG) logerror("MC68901 '%s' Timer A Delay Mode : %u Prescale\n", tag(), divisor);
			m_timer[TIMER_A]->adjust(attotime::from_hz(m_timer_clock / divisor), 0, attotime::from_hz(m_timer_clock / divisor));
			}
			break;

		case TCR_TIMER_EVENT:
			if (LOG) logerror("MC68901 '%s' Timer A Event Count Mode\n", tag());
			m_timer[TIMER_A]->enable(false);
			break;

		case TCR_TIMER_PULSE_4:
		case TCR_TIMER_PULSE_10:
		case TCR_TIMER_PULSE_16:
		case TCR_TIMER_PULSE_50:
		case TCR_TIMER_PULSE_64:
		case TCR_TIMER_PULSE_100:
		case TCR_TIMER_PULSE_200:
			{
			int divisor = PRESCALER[m_tacr & 0x07];
			if (LOG) logerror("MC68901 '%s' Timer A Pulse Width Mode : %u Prescale\n", tag(), divisor);
			m_timer[TIMER_A]->adjust(attotime::never, 0, attotime::from_hz(m_timer_clock / divisor));
			m_timer[TIMER_A]->enable(false);
			}
			break;
		}

		if (m_tacr & TCR_TIMER_RESET)
		{
			if (LOG) logerror("MC68901 '%s' Timer A Reset\n", tag());

			m_to[TIMER_A] = 0;

			m_out_tao_func(m_to[TIMER_A]);
		}
		break;

	case REGISTER_TBCR:
		m_tbcr = data & 0x1f;

		switch (m_tbcr & 0x0f)
		{
		case TCR_TIMER_STOPPED:
			if (LOG) logerror("MC68901 '%s' Timer B Stopped\n", tag());
			m_timer[TIMER_B]->enable(false);
			break;

		case TCR_TIMER_DELAY_4:
		case TCR_TIMER_DELAY_10:
		case TCR_TIMER_DELAY_16:
		case TCR_TIMER_DELAY_50:
		case TCR_TIMER_DELAY_64:
		case TCR_TIMER_DELAY_100:
		case TCR_TIMER_DELAY_200:
			{
			int divisor = PRESCALER[m_tbcr & 0x07];
			if (LOG) logerror("MC68901 '%s' Timer B Delay Mode : %u Prescale\n", tag(), divisor);
			m_timer[TIMER_B]->adjust(attotime::from_hz(m_timer_clock / divisor), 0, attotime::from_hz(m_timer_clock / divisor));
			}
			break;

		case TCR_TIMER_EVENT:
			if (LOG) logerror("MC68901 '%s' Timer B Event Count Mode\n", tag());
			m_timer[TIMER_B]->enable(false);
			break;

		case TCR_TIMER_PULSE_4:
		case TCR_TIMER_PULSE_10:
		case TCR_TIMER_PULSE_16:
		case TCR_TIMER_PULSE_50:
		case TCR_TIMER_PULSE_64:
		case TCR_TIMER_PULSE_100:
		case TCR_TIMER_PULSE_200:
			{
			int divisor = PRESCALER[m_tbcr & 0x07];
			if (LOG) logerror("MC68901 '%s' Timer B Pulse Width Mode : %u Prescale\n", tag(), DIVISOR);
			m_timer[TIMER_B]->adjust(attotime::never, 0, attotime::from_hz(m_timer_clock / divisor));
			m_timer[TIMER_B]->enable(false);
			}
			break;
		}

		if (m_tacr & TCR_TIMER_RESET)
		{
			if (LOG) logerror("MC68901 '%s' Timer B Reset\n", tag());

			m_to[TIMER_B] = 0;

			m_out_tbo_func(m_to[TIMER_B]);
		}
		break;

	case REGISTER_TCDCR:
		m_tcdcr = data & 0x6f;

		switch (m_tcdcr & 0x07)
		{
		case TCR_TIMER_STOPPED:
			if (LOG) logerror("MC68901 '%s' Timer D Stopped\n", tag());
			m_timer[TIMER_D]->enable(false);
			break;

		case TCR_TIMER_DELAY_4:
		case TCR_TIMER_DELAY_10:
		case TCR_TIMER_DELAY_16:
		case TCR_TIMER_DELAY_50:
		case TCR_TIMER_DELAY_64:
		case TCR_TIMER_DELAY_100:
		case TCR_TIMER_DELAY_200:
			{
			int divisor = PRESCALER[m_tcdcr & 0x07];
			if (LOG) logerror("MC68901 '%s' Timer D Delay Mode : %u Prescale\n", tag(), divisor);
			m_timer[TIMER_D]->adjust(attotime::from_hz(m_timer_clock / divisor), 0, attotime::from_hz(m_timer_clock / divisor));
			}
			break;
		}

		switch ((m_tcdcr >> 4) & 0x07)
		{
		case TCR_TIMER_STOPPED:
			if (LOG) logerror("MC68901 '%s' Timer C Stopped\n", tag());
			m_timer[TIMER_C]->enable(false);
			break;

		case TCR_TIMER_DELAY_4:
		case TCR_TIMER_DELAY_10:
		case TCR_TIMER_DELAY_16:
		case TCR_TIMER_DELAY_50:
		case TCR_TIMER_DELAY_64:
		case TCR_TIMER_DELAY_100:
		case TCR_TIMER_DELAY_200:
			{
			int divisor = PRESCALER[(m_tcdcr >> 4) & 0x07];
			if (LOG) logerror("MC68901 '%s' Timer C Delay Mode : %u Prescale\n", tag(), divisor);
			m_timer[TIMER_C]->adjust(attotime::from_hz(m_timer_clock / divisor), 0, attotime::from_hz(m_timer_clock / divisor));
			}
			break;
		}
		break;

	case REGISTER_TADR:
		if (LOG) logerror("MC68901 '%s' Timer A Data Register : %x\n", tag(), data);

		m_tdr[TIMER_A] = data;

		if (!m_timer[TIMER_A]->enabled())
		{
			m_tmc[TIMER_A] = data;
		}
		break;

	case REGISTER_TBDR:
		if (LOG) logerror("MC68901 '%s' Timer B Data Register : %x\n", tag(), data);

		m_tdr[TIMER_B] = data;

		if (!m_timer[TIMER_B]->enabled())
		{
			m_tmc[TIMER_B] = data;
		}
		break;

	case REGISTER_TCDR:
		if (LOG) logerror("MC68901 '%s' Timer C Data Register : %x\n", tag(), data);

		m_tdr[TIMER_C] = data;

		if (!m_timer[TIMER_C]->enabled())
		{
			m_tmc[TIMER_C] = data;
		}
		break;

	case REGISTER_TDDR:
		if (LOG) logerror("MC68901 '%s' Timer D Data Register : %x\n", tag(), data);

		m_tdr[TIMER_D] = data;

		if (!m_timer[TIMER_D]->enabled())
		{
			m_tmc[TIMER_D] = data;
		}
		break;

	case REGISTER_SCR:
		if (LOG) logerror("MC68901 '%s' Sync Character : %x\n", tag(), data);

		m_scr = data;
		break;

	case REGISTER_UCR:
		if (data & UCR_PARITY_ENABLED)
		{
			if (data & UCR_PARITY_EVEN)
			{
				if (LOG) logerror("MC68901 '%s' Parity : Even\n", tag());
			}
			else
			{
				if (LOG) logerror("MC68901 '%s' Parity : Odd\n", tag());
			}
		}
		else
		{
			if (LOG) logerror("MC68901 '%s' Parity : Disabled\n", tag());
		}

		switch (data & 0x60)
		{
		case UCR_WORD_LENGTH_8: m_rxtx_word = 8; break;
		case UCR_WORD_LENGTH_7: m_rxtx_word = 7; break;
		case UCR_WORD_LENGTH_6: m_rxtx_word = 6; break;
		case UCR_WORD_LENGTH_5: m_rxtx_word = 5; break;
		}

		if (LOG) logerror("MC68901 '%s' Word Length : %u bits\n", tag(), m_rxtx_word);

		switch (data & 0x18)
		{
		case UCR_START_STOP_0_0:
			m_rxtx_start = 0;
			m_rxtx_stop = 0;
			if (LOG) logerror("MC68901 '%s' Start Bits : 0, Stop Bits : 0, Format : synchronous\n", tag());
			break;
		case UCR_START_STOP_1_1:
			m_rxtx_start = 1;
			m_rxtx_stop = 1;
			if (LOG) logerror("MC68901 '%s' Start Bits : 1, Stop Bits : 1, Format : asynchronous\n", tag());
			break;
		case UCR_START_STOP_1_15:
			m_rxtx_start = 1;
			m_rxtx_stop = 1;
			if (LOG) logerror("MC68901 '%s' Start Bits : 1, Stop Bits : 1??, Format : asynchronous\n", tag());
			break;
		case UCR_START_STOP_1_2:
			m_rxtx_start = 1;
			m_rxtx_stop = 2;
			if (LOG) logerror("MC68901 '%s' Start Bits : 1, Stop Bits : 2, Format : asynchronous\n", tag());
			break;
		}

		if (data & UCR_CLOCK_DIVIDE_16)
		{
			if (LOG) logerror("MC68901 '%s' Rx/Tx Clock Divisor : 16\n", tag());
		}
		else
		{
			if (LOG) logerror("MC68901 '%s' Rx/Tx Clock Divisor : 1\n", tag());
		}

		m_ucr = data;
		break;

	case REGISTER_RSR:
		if ((data & RSR_RCV_ENABLE) == 0)
		{
			if (LOG) logerror("MC68901 '%s' Receiver Disabled\n", tag());
			m_rsr = 0;
		}
		else
		{
			if (LOG) logerror("MC68901 '%s' Receiver Enabled\n", tag());

			if (data & RSR_SYNC_STRIP_ENABLE)
			{
				if (LOG) logerror("MC68901 '%s' Sync Strip Enabled\n", tag());
			}
			else
			{
				if (LOG) logerror("MC68901 '%s' Sync Strip Disabled\n", tag());
			}

			if (data & RSR_FOUND_SEARCH)
				if (LOG) logerror("MC68901 '%s' Receiver Search Mode Enabled\n", tag());

			m_rsr = data & 0x0b;
		}
		break;

	case REGISTER_TSR:
		if ((data & TSR_XMIT_ENABLE) == 0)
		{
			if (LOG) logerror("MC68901 '%s' Transmitter Disabled\n", tag());

			m_tsr = data & 0x27;
		}
		else
		{
			if (LOG) logerror("MC68901 '%s' Transmitter Enabled\n", tag());

			switch (data & 0x06)
			{
			case TSR_OUTPUT_HI_Z:
				if (LOG) logerror("MC68901 '%s' Transmitter Disabled Output State : Hi-Z\n", tag());
				break;
			case TSR_OUTPUT_LOW:
				if (LOG) logerror("MC68901 '%s' Transmitter Disabled Output State : 0\n", tag());
				break;
			case TSR_OUTPUT_HIGH:
				if (LOG) logerror("MC68901 '%s' Transmitter Disabled Output State : 1\n", tag());
				break;
			case TSR_OUTPUT_LOOP:
				if (LOG) logerror("MC68901 '%s' Transmitter Disabled Output State : Loop\n", tag());
				break;
			}

			if (data & TSR_BREAK)
			{
				if (LOG) logerror("MC68901 '%s' Transmitter Break Enabled\n", tag());
			}
			else
			{
				if (LOG) logerror("MC68901 '%s' Transmitter Break Disabled\n", tag());
			}

			if (data & TSR_AUTO_TURNAROUND)
			{
				if (LOG) logerror("MC68901 '%s' Transmitter Auto Turnaround Enabled\n", tag());
			}
			else
			{
				if (LOG) logerror("MC68901 '%s' Transmitter Auto Turnaround Disabled\n", tag());
			}

			m_tsr = data & 0x2f;
			m_tsr |= TSR_BUFFER_EMPTY;  // x68000 expects the buffer to be empty, so this will do for now
		}
		break;

	case REGISTER_UDR:
		if (LOG) logerror("MC68901 '%s' UDR %x\n", tag(), data);
		m_udr = data;
		//m_tsr &= ~TSR_BUFFER_EMPTY;
		break;
	}
}

WRITE8_MEMBER( mc68901_device::write )
{
	register_w(offset, data);
}


int mc68901_device::get_vector()
{
	int ch;

	for (ch = 15; ch >= 0; ch--)
	{
		if (BIT(m_imr, ch) && BIT(m_ipr, ch))
		{
			if (m_vr & VR_S)
			{
				/* set interrupt-in-service bit */
				m_isr |= (1 << ch);
			}

			/* clear interrupt pending bit */
			m_ipr &= ~(1 << ch);

			check_interrupts();

			return (m_vr & 0xf0) | ch;
		}
	}

	return M68K_INT_ACK_SPURIOUS;
}

WRITE_LINE_MEMBER( mc68901_device::i0_w ) { gpio_input(0, state); }
WRITE_LINE_MEMBER( mc68901_device::i1_w ) { gpio_input(1, state); }
WRITE_LINE_MEMBER( mc68901_device::i2_w ) { gpio_input(2, state); }
WRITE_LINE_MEMBER( mc68901_device::i3_w ) { gpio_input(3, state); }
WRITE_LINE_MEMBER( mc68901_device::i4_w ) { gpio_input(4, state); }
WRITE_LINE_MEMBER( mc68901_device::i5_w ) { gpio_input(5, state); }
WRITE_LINE_MEMBER( mc68901_device::i6_w ) { gpio_input(6, state); }
WRITE_LINE_MEMBER( mc68901_device::i7_w ) { gpio_input(7, state); }


WRITE_LINE_MEMBER( mc68901_device::tai_w )
{
	timer_input(TIMER_A, state);
}


WRITE_LINE_MEMBER( mc68901_device::tbi_w )
{
	timer_input(TIMER_B, state);
}


WRITE_LINE_MEMBER( mc68901_device::rc_w )
{
	if (state)
	{
		serial_receive();
	}
}


WRITE_LINE_MEMBER( mc68901_device::tc_w )
{
	if (state)
	{
		serial_transmit();
	}
}
