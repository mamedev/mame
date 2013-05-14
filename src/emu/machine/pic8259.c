/**********************************************************************

    8259 PIC interface and emulation

    The 8259 is a programmable interrupt controller used to multiplex
    interrupts for x86 and other computers.  The chip is set up by
    writing a series of Initialization Command Words (ICWs) after which
    the chip is operational and capable of dispatching interrupts.  After
    this, Operation Command Words (OCWs) can be written to control further
    behavior.

**********************************************************************/

#include "emu.h"
#include "machine/pic8259.h"

#define IRQ_COUNT   8

#define LOG_ICW     0
#define LOG_OCW     0
#define LOG_GENERAL  0


TIMER_CALLBACK_MEMBER( pic8259_device::timerproc )
{
	int irq;
	UINT8 mask;

	/* check the various IRQs */
	for (irq = 0; irq < IRQ_COUNT; irq++)
	{
		mask = 1 << irq;

		/* is this IRQ in service? */
		if (m_isr & mask)
		{
			if (LOG_GENERAL)
			{
				logerror("pic8259_timerproc(): PIC IRQ #%d still in service\n", irq);
			}
			break;
		}

		/* is this IRQ pending and enabled? */
		if ((m_state == STATE_READY) && (m_irr & mask) && !(m_imr & mask))
		{
			if (LOG_GENERAL)
			{
				logerror("pic8259_timerproc(): PIC triggering IRQ #%d\n", irq);
			}
			if (!BIT(m_ocw3, 2))
			{
				m_out_int_func(1);
			}
			return;
		}
	}
	if (!BIT(m_ocw3, 2))
	{
		m_out_int_func(0);
	}
}


void pic8259_device::set_irq_line(int irq, int state)
{
	UINT8 mask = (1 << irq);

	if (state)
	{
		/* setting IRQ line */
		if (LOG_GENERAL)
			logerror("pic8259_set_irq_line(): PIC set IRQ line #%d\n", irq);

		if(m_level_trig_mode || (!m_level_trig_mode && !(m_irq_lines & mask)))
		{
			m_irr |= mask;
		}
		m_irq_lines |= mask;
	}
	else
	{
		/* clearing IRQ line */
		if (LOG_GENERAL)
		{
			logerror("pic8259_device::set_irq_line(): PIC cleared IRQ line #%d\n", irq);
		}

		m_irq_lines &= ~mask;
		m_irr &= ~mask;
	}
	set_timer();
}


UINT32 pic8259_device::acknowledge()
{
	UINT8 mask;
	int irq;

	for (irq = 0; irq < IRQ_COUNT; irq++)
	{
		mask = 1 << irq;

		/* is this IRQ pending and enabled? */
		if ((m_irr & mask) && !(m_imr & mask))
		{
			if (LOG_GENERAL)
			{
				logerror("pic8259_acknowledge(): PIC acknowledge IRQ #%d\n", irq);
			}
			if (!m_level_trig_mode)
			{
				m_irr &= ~mask;
			}

			if (!m_auto_eoi)
			{
				m_isr |= mask;
			}

			set_timer();

			if ((m_cascade!=0) && (m_master!=0) && (mask & m_slave))
			{
				// it's from slave device
				return m_read_slave_ack_func(irq);
			}
			else
			{
				if (m_is_x86)
				{
					/* For x86 mode*/
					return irq + m_base;
				}
				else
				{
					/* in case of 8080/85) */
					return 0xcd0000 + (m_vector_addr_high << 8) + m_vector_addr_low + (irq << (3-m_vector_size));
				}
			}
		}
	}
	return 0;
}


UINT8 pic8259_device::inta_r()
{
	return acknowledge();
}


READ8_MEMBER( pic8259_device::read )
{
	/* NPW 18-May-2003 - Changing 0xFF to 0x00 as per Ruslan */
	UINT8 data = 0x00;

	switch(offset)
	{
		case 0: /* PIC acknowledge IRQ */
			if ( m_ocw3 & 0x04 )
			{
				/* Polling mode */
				if ( m_isr & ~m_imr )
				{
					acknowledge();
				}

				if ( m_irr & ~m_imr )
				{
					int irq;
					for ( irq = 0; irq < IRQ_COUNT; irq++ )
					{
						if ( ( 1 << irq ) & m_irr & ~m_imr )
						{
							data = 0x80 | irq;
							break;
						}
					}
				}
			}
			else
			{
				switch ( m_ocw3 & 0x03 )
				{
				case 2:
					data = m_irr;
					break;
				case 3:
					data = m_isr & ~m_imr;
					break;
				default:
					data = 0x00;
					break;
				}
			}
			break;

		case 1: /* PIC mask register */
			data = m_imr;
			break;
	}
	return data;
}


WRITE8_MEMBER( pic8259_device::write )
{
	switch(offset)
	{
		case 0:    /* PIC acknowledge IRQ */
			if (data & 0x10)
			{
				/* write ICW1 - this pretty much resets the chip */
				if (LOG_ICW)
				{
					logerror("pic8259_device::write(): ICW1; data=0x%02X\n", data);
				}

				m_imr                = 0x00;
				m_isr                = 0x00;
				m_irr                = 0x00;
				m_level_trig_mode    = (data & 0x08) ? 1 : 0;
				m_vector_size        = (data & 0x04) ? 1 : 0;
				m_cascade            = (data & 0x02) ? 0 : 1;
				m_icw4_needed        = (data & 0x01) ? 1 : 0;
				m_vector_addr_low    = (data & 0xe0);
				m_state          = STATE_ICW2;
				m_out_int_func(0);
			}
			else if (m_state == STATE_READY)
			{
				if ((data & 0x98) == 0x08)
				{
					/* write OCW3 */
					if (LOG_OCW)
					{
						logerror("pic8259_device::write(): OCW3; data=0x%02X\n", data);
					}

					m_ocw3 = data;
				}
				else if ((data & 0x18) == 0x00)
				{
					int n = data & 7;
					UINT8 mask = 1 << n;

					/* write OCW2 */
					if (LOG_OCW)
					{
						logerror("pic8259_device::write(): OCW2; data=0x%02X\n", data);
					}

					switch (data & 0xe0)
					{
						case 0x00:
							m_prio = 0;
							break;
						case 0x20:
							for (n = 0, mask = 1<<m_prio; n < 8; n++, mask = (mask<<1) | (mask>>7))
							{
								if (m_isr & mask)
								{
									m_isr &= ~mask;
									break;
								}
							}
							break;
						case 0x40:
							break;
						case 0x60:
							if( m_isr & mask )
							{
								m_isr &= ~mask;
							}
							break;
						case 0x80:
							m_prio = (m_prio + 1) & 7;
							break;
						case 0xa0:
							for (n = 0, mask = 1<<m_prio; n < 8; n++, mask = (mask<<1) | (mask>>7))
							{
								if( m_isr & mask )
								{
									m_isr &= ~mask;
									m_prio = (m_prio + 1) & 7;
									break;
								}
							}
							break;
						case 0xc0:
							m_prio = n & 7;
							break;
						case 0xe0:
							if( m_isr & mask )
							{
								m_isr &= ~mask;
								m_prio = (m_prio + 1) & 7;
							}
							break;
					}
				}
			}
			break;

		case 1:
			switch(m_state)
			{
				case STATE_ICW1:
					break;

				case STATE_ICW2:
					/* write ICW2 */
					if (LOG_ICW)
					{
						logerror("pic8259_device::write(): ICW2; data=0x%02X\n", data);
					}

					m_base = data & 0xf8;
					m_vector_addr_high = data ;
					if (m_cascade)
					{
						m_state = STATE_ICW3;
					}
					else
					{
						m_state = m_icw4_needed ? STATE_ICW4 : STATE_READY;
					}
					break;

				case STATE_ICW3:
					/* write ICW3 */
					if (LOG_ICW)
					{
						logerror("pic8259_device::write(): ICW3; data=0x%02X\n", data);
					}

					m_slave = data;
					m_state = m_icw4_needed ? STATE_ICW4 : STATE_READY;
					break;

				case STATE_ICW4:
					/* write ICW4 */
					if (LOG_ICW)
					{
						logerror("pic8259_device::write(): ICW4; data=0x%02X\n", data);
					}

					m_nested = (data & 0x10) ? 1 : 0;
					m_mode = (data >> 2) & 3;
					m_auto_eoi = (data & 0x02) ? 1 : 0;
					m_is_x86 = (data & 0x01) ? 1 : 0;
					m_state = STATE_READY;
					break;

				case STATE_READY:
					/* write OCW1 - set interrupt mask register */
					if (LOG_OCW)
					{
						logerror("pic8259_device::write(): OCW1; data=0x%02X\n", data);
					}

					//printf("%s %02x\n",m_master ? "master pic8259 mask" : "slave pic8259 mask",data);
					m_imr = data;
					break;
			}
			break;
	}
	set_timer();
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pic8259_device::device_start()
{
	const struct pic8259_interface *intf = (const struct pic8259_interface *)this->static_config();

	assert(intf != NULL);

	m_timer = machine().scheduler().timer_alloc( timer_expired_delegate( FUNC(pic8259_device::timerproc), this) );

	/* resolve callbacks */
	m_out_int_func.resolve(intf->out_int_func, *this);
	m_sp_en_func.resolve(intf->sp_en_func, *this);
	m_read_slave_ack_func.resolve(intf->read_slave_ack_func, *this);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void pic8259_device::device_reset()
{
	m_state = STATE_READY;
	m_isr = 0;
	m_irr = 0;
	m_irq_lines = 0;
	m_prio = 0;
	m_imr = 0;
	m_input = 0;
	m_ocw3 = 2;
	m_level_trig_mode = 0;
	m_vector_size = 0;
	m_cascade = 0;
	m_icw4_needed = 0;
	m_base = 0;
	m_slave = 0;
	m_nested = 0;
	m_mode = 0;
	m_auto_eoi = 0;
	m_is_x86 = 0;
	m_vector_addr_low = 0;
	m_vector_addr_high = 0;

	m_master = m_sp_en_func();
}

const device_type PIC8259 = &device_creator<pic8259_device>;

pic8259_device::pic8259_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, PIC8259, "Intel PIC8259", tag, owner, clock)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void pic8259_device::device_config_complete()
{
}

