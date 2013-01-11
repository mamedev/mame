/**********************************************************************

    8237 DMA interface and emulation

    The DMA works like this:
    (summarized from http://www.infran.ru/TechInfo/BSD/handbook258.html#410)

    1.  The device asserts the DRQn line
    2.  The DMA clears the TC (terminal count) line
    3.  The DMA asserts the CPU's HRQ (halt request) line
    4.  Upon acknowledgement of the halt, the DMA will let the device
        know that it needs to send information by asserting the DACKn
        line
    5.  The DMA will read the byte from the device
    6.  The device clears the DRQn line
    7.  The DMA clears the CPU's HRQ line
    8.  (steps 3-7 are repeated for every byte in the chain)

**********************************************************************/

#include "emu.h"
#include "8237dma.h"
#include "devhelpr.h"

/***************************************************************************
    MACROS
***************************************************************************/

#define DMA_MODE_CHANNEL(mode)      ((mode) & 0x03)
#define DMA_MODE_OPERATION(mode)    ((mode) & 0x0c)
#define DMA_MODE_AUTO_INIT(mode)    ((mode) & 0x10)
#define DMA_MODE_DIRECTION(mode)    ((mode) & 0x20)
#define DMA_MODE_TRANSFERMODE(mode) ((mode) & 0xc0)

#define DMA8237_VERIFY_TRANSFER     0x00
#define DMA8237_WRITE_TRANSFER      0x04
#define DMA8237_READ_TRANSFER       0x08
#define DMA8237_ILLEGAL_TRANSFER    0x0c

#define DMA8237_DEMAND_MODE     0x00
#define DMA8237_SINGLE_MODE     0x40
#define DMA8237_BLOCK_MODE      0x80
#define DMA8237_CASCADE_MODE    0xc0


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
const device_type I8237 = &device_creator<i8237_device>;

//-------------------------------------------------
//  i8237_device - constructor
//-------------------------------------------------

i8237_device::i8237_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, I8237, "Intel 8237", tag, owner, clock)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void i8237_device::device_config_complete()
{
	// inherit a copy of the static data
	const i8237_interface *intf = reinterpret_cast<const i8237_interface *>(static_config());
	if (intf != NULL)
	{
		*static_cast<i8237_interface *>(this) = *intf;
	}

	// or initialize to defaults if none provided
	else
	{
		memset(&m_out_hrq_cb, 0, sizeof(m_out_hrq_cb));
		memset(&m_out_eop_cb, 0, sizeof(m_out_eop_cb));
		memset(&m_in_memr_cb, 0, sizeof(m_in_memr_cb));
		memset(&m_out_memw_cb, 0, sizeof(m_out_memw_cb));
		memset(&m_in_ior_cb[0], 0, sizeof(m_in_ior_cb[0]));
		memset(&m_in_ior_cb[1], 0, sizeof(m_in_ior_cb[1]));
		memset(&m_in_ior_cb[2], 0, sizeof(m_in_ior_cb[2]));
		memset(&m_in_ior_cb[3], 0, sizeof(m_in_ior_cb[3]));
		memset(&m_out_iow_cb[0], 0, sizeof(m_out_iow_cb[0]));
		memset(&m_out_iow_cb[1], 0, sizeof(m_out_iow_cb[1]));
		memset(&m_out_iow_cb[2], 0, sizeof(m_out_iow_cb[2]));
		memset(&m_out_iow_cb[3], 0, sizeof(m_out_iow_cb[3]));
		memset(&m_out_dack_cb[0], 0, sizeof(m_out_dack_cb[0]));
		memset(&m_out_dack_cb[1], 0, sizeof(m_out_dack_cb[1]));
		memset(&m_out_dack_cb[2], 0, sizeof(m_out_dack_cb[2]));
		memset(&m_out_dack_cb[3], 0, sizeof(m_out_dack_cb[3]));
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void i8237_device::device_start()
{
	/* resolve callbacks */
	m_out_hrq_func.resolve(m_out_hrq_cb, *this);
	m_out_eop_func.resolve(m_out_eop_cb, *this);
	m_in_memr_func.resolve(m_in_memr_cb, *this);
	m_out_memw_func.resolve(m_out_memw_cb, *this);

	for (int i = 0; i < 4; i++)
	{
		m_chan[i].m_in_ior_func.resolve(m_in_ior_cb[i], *this);
		m_chan[i].m_out_iow_func.resolve(m_out_iow_cb[i], *this);
		m_chan[i].m_out_dack_func.resolve(m_out_dack_cb[i], *this);
	}

	m_timer = machine().scheduler().timer_alloc(FUNC(i8237_timerproc_callback), (void *)this);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void i8237_device::device_reset()
{
	m_status = 0x0F;
	m_eop = ASSERT_LINE;
	m_state = DMA8237_SI;
	m_last_service_channel = 3;
	m_service_channel = 0;

	m_command = 0;
	m_drq = 0;
	m_mask = 0x00;
	m_hrq = 0;
	m_hlda = 0;
	m_chan[0].m_mode = 0;
	m_chan[1].m_mode = 0;
	m_chan[2].m_mode = 0;
	m_chan[3].m_mode = 0;

	m_timer->adjust(attotime::from_hz(clock()), 0, attotime::from_hz(clock()));
}


void i8237_device::i8237_do_read()
{
	int channel = m_service_channel;

	switch( DMA_MODE_OPERATION( m_chan[ channel ].m_mode ) )
	{
	case DMA8237_WRITE_TRANSFER:
		m_temporary_data = m_chan[channel].m_in_ior_func(0);
		break;
	case DMA8237_READ_TRANSFER:
		m_temporary_data = m_in_memr_func(m_chan[ channel ].m_address);
		break;
	case DMA8237_VERIFY_TRANSFER:
	case DMA8237_ILLEGAL_TRANSFER:
		break;
	}
}


void i8237_device::i8237_do_write()
{
	int channel = m_service_channel;

	switch( DMA_MODE_OPERATION( m_chan[ channel ].m_mode ) )
	{
	case DMA8237_WRITE_TRANSFER:
		m_out_memw_func(m_chan[ channel ].m_address, m_temporary_data);
		break;
	case DMA8237_READ_TRANSFER:
		m_chan[channel].m_out_iow_func(0, m_temporary_data);
		break;
	case DMA8237_VERIFY_TRANSFER:
	case DMA8237_ILLEGAL_TRANSFER:
		break;
	}
}


void i8237_device::i8237_advance()
{
	int channel = m_service_channel;
	int mode = m_chan[channel].m_mode;

	switch ( DMA_MODE_OPERATION( mode ) )
	{
	case DMA8237_VERIFY_TRANSFER:
	case DMA8237_WRITE_TRANSFER:
	case DMA8237_READ_TRANSFER:
		m_chan[channel].m_high_address_changed = 0;

		if ( DMA_MODE_DIRECTION( mode ) )
		{
			m_chan[channel].m_address -= 1;
			if ( ( m_chan[channel].m_address & 0xFF ) == 0xFF )
			{
				m_chan[channel].m_high_address_changed  = 1;
			}
		}
		else
		{
			m_chan[channel].m_address += 1;
			if ( ( m_chan[channel].m_address & 0xFF ) == 0x00 )
			{
				m_chan[channel].m_high_address_changed = 1;
			}
		}

		m_chan[channel].m_count--;

		if ( m_chan[channel].m_count == 0xFFFF )
		{
			/* Set TC bit for this channel */
			m_status |= ( 0x01 << channel );

			if ( DMA_MODE_AUTO_INIT( mode ) )
			{
				m_chan[channel].m_address = m_chan[channel].m_base_address;
				m_chan[channel].m_count = m_chan[channel].m_base_count;
				m_chan[channel].m_high_address_changed = 1;
			}
			else
			{
				m_mask |= ( 0x01 << channel );
			}
		}
		break;
	case DMA8237_ILLEGAL_TRANSFER:
		break;
	}
}


void i8237_device::i8327_set_dack(int channel)
{
	for (int i = 0; i < 4; i++)
	{
		int state = (i == channel) ^ !BIT(m_command, 7);

		m_chan[i].m_out_dack_func(state);
	}
}


TIMER_CALLBACK( i8237_device::i8237_timerproc_callback )
{
	reinterpret_cast<i8237_device*>(ptr)->i8237_timerproc();
}


void i8237_device::i8237_timerproc()
{
	/* Check if operation is disabled */
	if ( m_command & 0x04 )
	{
		return;
	}

	switch ( m_state )
	{

	case DMA8237_SI:
	{
		/* Make sure EOP is high */
		if ( m_eop == CLEAR_LINE )
		{
			m_eop = ASSERT_LINE;
			m_out_eop_func(m_eop);
		}

		/* Check if a new DMA request has been received. */
		/* Bit 6 of the command register determines whether the DREQ signals are active
		  high or active low. */
		UINT16 pending_request = ( ( m_command & 0x40 ) ? ~m_drq : m_drq ) & ~m_mask;

		if ( pending_request & 0x0f )
		{
			int prio_channel = 0;

			/* Determine the channel that should be serviced */
			int channel = ( m_command & 0x10 ) ? m_last_service_channel : 3;
			for ( int i = 0; i < 4; i++ )
			{
				if ( pending_request & ( 1 << channel ) )
				{
					prio_channel = channel;
				}
				channel = ( channel - 1 ) & 0x03;
			}

			/* Store the channel we will be servicing and go to the next state */
			m_service_channel = prio_channel;
			m_last_service_channel = prio_channel;

			m_hrq = 1;
			m_out_hrq_func(m_hrq);
			m_state = DMA8237_S0;

			m_timer->enable( true );
		}
		else if (m_command == 3 && (m_drq & 1))
		{
			/* Memory-to-memory transfers */
			m_hlda = 1;
			m_state = DMA8237_S0;
		}
		else
		{
			m_timer->enable( false );
		}
		break;
	}

	case DMA8237_S0:
		/* S0 is the first of the DMA service. We have requested a hold but are waiting
		  for confirmation. */
		if ( m_hlda )
		{
			if ( DMA_MODE_TRANSFERMODE( m_chan[m_service_channel].m_mode ) == DMA8237_CASCADE_MODE )
			{
				/* Cascade Mode, set DACK */
				i8327_set_dack(m_service_channel);

				/* Wait until peripheral is done */
				m_state = DMA8237_SC;
			}
			else
			{
				if ( m_command & 0x01 )
				{
					/* Memory-to-memory transfers */
					m_state = DMA8237_S11;
				}
				else
				{
					/* Regular transfers */
					m_state = DMA8237_S1;
				}
			}
		}
		break;

	case DMA8237_SC:    /* Cascade mode, waiting until peripheral is done */
		if ( ! ( m_drq & ( 0x01 << m_service_channel ) ) )
		{
			m_hrq = 0;
			m_hlda = 0;
			m_out_hrq_func(m_hrq);
			m_state = DMA8237_SI;

			/* Clear DACK */
			i8327_set_dack(-1);
		}

		/* Not sure if this is correct, documentation is not clear */
		/* Check if EOP output needs to be asserted  */
		if ( m_status & ( 0x01 << m_service_channel ) )
		{
			m_eop = CLEAR_LINE;
			m_out_eop_func(m_eop);
		}
		break;

	case DMA8237_S1:    /* Output A8-A15 */
		m_state = DMA8237_S2;
		break;

	case DMA8237_S2:    /* Output A7-A0 */
		/* set DACK */
		i8327_set_dack(m_service_channel);

		/* Check for compressed timing */
		if ( m_command & 0x08 )
		{
			m_state = DMA8237_S4;
		}
		else
		{
			m_state = DMA8237_S3;
		}
		break;

	case DMA8237_S3:    /* Initiate read */
		i8237_do_read();
		m_state = DMA8237_S4;
		break;

	case DMA8237_S4:    /* Perform read/write */
		/* Perform read when in compressed timing mode */
		if ( m_command & 0x08 )
		{
			i8237_do_read();
		}

		/* Perform write */
		i8237_do_write();


		/* Advance */
		i8237_advance();

		{
			int channel = m_service_channel;

			switch( DMA_MODE_TRANSFERMODE( m_chan[channel].m_mode ) )
			{
			case DMA8237_DEMAND_MODE:
				/* Check for terminal count or EOP signal or DREQ begin de-asserted */
				if ( ( m_status & ( 0x01 << channel ) ) || m_eop == CLEAR_LINE || !( m_drq & ( 0x01 << channel ) ) )
				{
					m_hrq = 0;
					m_hlda = 0;
					m_out_hrq_func(m_hrq);
					m_state = DMA8237_SI;
				}
				else
				{
					m_state = m_chan[channel].m_high_address_changed ? DMA8237_S1 : DMA8237_S2;
				}
				break;

			case DMA8237_SINGLE_MODE:
				m_hrq = 0;
				m_hlda = 0;
				m_out_hrq_func(m_hrq);
				m_state = DMA8237_SI;
				break;

			case DMA8237_BLOCK_MODE:
				/* Check for terminal count or EOP signal */
				if ( ( m_status & ( 0x01 << channel ) ) || m_eop == CLEAR_LINE )
				{
					m_hrq = 0;
					m_hlda = 0;
					m_out_hrq_func(m_hrq);
					m_state = DMA8237_SI;
				}
				else
				{
					m_state = m_chan[channel].m_high_address_changed ? DMA8237_S1 : DMA8237_S2;
				}
				break;
			}

			/* Check if EOP output needs to be asserted */
			if ( m_status & ( 0x01 << channel ) )
			{
				m_eop = CLEAR_LINE;
				m_out_eop_func(m_eop);
			}
		}

		/* clear DACK */
		if ( m_state == DMA8237_SI )
		{
			i8327_set_dack(-1);
		}
		break;

	case DMA8237_S11: /* Output A8-A15 */

//      logerror("###### dma8237_timerproc %s: from %04x count=%x to %04x count=%x\n", tag(),
//              m_chan[0].m_address, m_chan[0].m_count,
//              m_chan[1].m_address, m_chan[1].m_count);

		// FIXME: this will copy bytes correct, but not 16 bit words
		m_temporary_data = m_in_memr_func(m_chan[0].m_address);
		m_out_memw_func(m_chan[1].m_address, m_temporary_data);

		m_service_channel = 0;

		/* Advance */
		i8237_advance();

		// advance destination channel as well
		m_chan[1].m_count--;
		m_chan[1].m_address++;

		if (m_chan[0].m_count == 0xFFFF || m_chan[1].m_count == 0xFFFF) {
			m_hrq = 0;
			m_hlda = 0;
			m_out_hrq_func(m_hrq);
			m_state = DMA8237_SI;
			m_status |= 3; // set TC for channel 0 and 1
			m_drq &= ~3; // clear drq for channel 0 and 1

		//  logerror("!!! dma8237_timerproc DMA8237_S11 %s: m_drq=%x m_command=%x\n", tag(), m_drq, m_command);
		}
		break;
	}
}


READ8_DEVICE_HANDLER_TRAMPOLINE(i8237, i8237_r)
{
	UINT8 data = 0xFF;

	offset &= 0x0F;

	switch(offset) {
	case 0:
	case 2:
	case 4:
	case 6:
		/* DMA address register */
		data = m_chan[offset / 2].m_address >> (m_msb ? 8 : 0);
		m_msb ^= 1;
		break;

	case 1:
	case 3:
	case 5:
	case 7:
		/* DMA count register */
		data = m_chan[offset / 2].m_count >> (m_msb ? 8 : 0);
		m_msb ^= 1;
		break;

	case 8:
		/* DMA status register */
		data = (UINT8) m_status;

		/* TC bits are cleared on a status read */
		m_status &= 0xF0;
		break;

	case 10:
		/* DMA mask register */
		data = m_mask;
		break;

	case 13:
		/* DMA master clear */
		data = m_temp;
		break;

	case 9:     /* DMA write request register */
	case 11:    /* DMA mode register */
	case 12:    /* DMA clear byte pointer flip-flop */
	case 14:    /* DMA clear mask register */
	case 15:    /* DMA write mask register */
		data = 0xFF;
		break;
	}

	return data;
}


WRITE8_DEVICE_HANDLER_TRAMPOLINE(i8237, i8237_w)
{
	offset &= 0x0F;

//  logerror("i8237_w: offset = %02x, data = %02x\n", offset, data );

	switch(offset) {
	case 0:
	case 2:
	case 4:
	case 6:
	{
		/* DMA address register */
		int channel = offset / 2;
		if (m_msb)
		{
			m_chan[channel].m_base_address = ( m_chan[channel].m_base_address & 0x00FF ) | ( data << 8 );
			m_chan[channel].m_address = ( m_chan[channel].m_address & 0x00FF ) | ( data << 8 );
		}
		else
		{
			m_chan[channel].m_base_address = ( m_chan[channel].m_base_address & 0xFF00 ) | data;
			m_chan[channel].m_address = ( m_chan[channel].m_address & 0xFF00 ) | data;
		}
		m_msb ^= 1;
		break;
	}

	case 1:
	case 3:
	case 5:
	case 7:
	{
		/* DMA count register */
		int channel = offset / 2;
		if (m_msb)
		{
			m_chan[channel].m_base_count = ( m_chan[channel].m_base_count & 0x00FF ) | ( data << 8 );
			m_chan[channel].m_count = ( m_chan[channel].m_count & 0x00FF ) | ( data << 8 );
		}
		else
		{
			m_chan[channel].m_base_count = ( m_chan[channel].m_base_count & 0xFF00 ) | data;
			m_chan[channel].m_count = ( m_chan[channel].m_count & 0xFF00 ) | data;
		}
		m_msb ^= 1;
		break;
	}

	case 8:
		/* DMA command register */
		m_command = data;
		m_timer->enable( ( m_command & 0x04 ) ? 0 : 1 );
		break;

	case 9:
	{
		/* DMA request register */
		int channel = DMA_MODE_CHANNEL(data);
		if ( data & 0x04 )
		{
			m_drq |= 0x01 << channel;
			m_timer->enable( ( m_command & 0x04 ) ? 0 : 1 );
		}
		else
		{
			m_status &= ~ ( 0x10 << channel );
			m_drq &= ~ ( 0x01 << channel );
		}
		break;
	}

	case 10:
	{
		/* DMA mask register */
		int channel = DMA_MODE_CHANNEL(data);
		if (data & 0x04)
		{
			m_mask |= 0x11 << channel;
		}
		else
		{
			m_mask &= ~(0x11 << channel);
		}
		break;
	}

	case 11:
	{
		/* DMA mode register */
		int channel = DMA_MODE_CHANNEL(data);
		m_chan[channel].m_mode = data;
		/* Apparently mode writes also clear the TC bit(?) */
		m_status &= ~ ( 1 << channel );
		break;
	}

	case 12:
		/* DMA clear byte pointer flip-flop */
		m_temp = data;
		m_msb = 0;
		break;

	case 13:
		/* DMA master clear */
		m_msb = 0;
		m_mask = 0x0f;
		m_state = DMA8237_SI;
		m_status &= 0xF0;
		m_temp = 0;
		break;

	case 14:
		/* DMA clear mask register */
		m_mask &= ~data;
		m_mask = 0;
		break;

	case 15:
		/* DMA write mask register */
		m_mask = data & 0x0f;
		break;
	}
}


void i8237_device::i8237_drq_write(int channel, int state)
{
	if (state)
	{
		m_drq |= ( 0x01 << channel );
	}
	else
	{
		m_drq &= ~( 0x01 << channel );
	}

	m_timer->enable( ( m_command & 0x04 ) ? 0 : 1 );
}



/***************************************************************************
    TRAMPOLINES
***************************************************************************/

WRITE_LINE_DEVICE_HANDLER( i8237_hlda_w ) { downcast<i8237_device*>(device)->i8237_hlda_w(state); }
WRITE_LINE_DEVICE_HANDLER( i8237_ready_w ) { }
WRITE_LINE_DEVICE_HANDLER( i8237_dreq0_w ) { downcast<i8237_device*>(device)->i8237_drq_write(0, state); }
WRITE_LINE_DEVICE_HANDLER( i8237_dreq1_w ) { downcast<i8237_device*>(device)->i8237_drq_write(1, state); }
WRITE_LINE_DEVICE_HANDLER( i8237_dreq2_w ) { downcast<i8237_device*>(device)->i8237_drq_write(2, state); }
WRITE_LINE_DEVICE_HANDLER( i8237_dreq3_w ) { downcast<i8237_device*>(device)->i8237_drq_write(3, state); }
WRITE_LINE_DEVICE_HANDLER( i8237_eop_w ) { }
