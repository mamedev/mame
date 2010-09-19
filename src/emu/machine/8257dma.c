/**********************************************************************

    8257 DMA interface and emulation

    For datasheet http://www.threedee.com/jcm/library/index.html

    2008/05     Miodrag Milanovic

        - added support for autoload mode
        - fixed bug in calculating count

    2007/11     couriersud

        - architecture copied from 8237 DMA
        - significant changes to implementation

    The DMA works like this:

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
#include "memconv.h"
#include "8257dma.h"
#include "devhelpr.h"

#define I8257_STATUS_UPDATE		0x10
#define I8257_STATUS_TC_CH3		0x08
#define I8257_STATUS_TC_CH2		0x04
#define I8257_STATUS_TC_CH1		0x02
#define I8257_STATUS_TC_CH0		0x01

#define DMA_MODE_AUTOLOAD(mode)		((mode) & 0x80)
#define DMA_MODE_TCSTOP(mode)		((mode) & 0x40)
#define DMA_MODE_EXWRITE(mode)		((mode) & 0x20)
#define DMA_MODE_ROTPRIO(mode)		((mode) & 0x10)
#define DMA_MODE_CH_EN(mode, chan)	((mode) & (1 << (chan)))


//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

GENERIC_DEVICE_CONFIG_SETUP(i8257, "DMA8257")

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void i8257_device_config::device_config_complete()
{
	// inherit a copy of the static data
	const i8257_interface *intf = reinterpret_cast<const i8257_interface *>(static_config());
	if (intf != NULL)
	{
		*static_cast<i8257_interface *>(this) = *intf;
	}

	// or initialize to defaults if none provided
	else
	{
    	memset(&m_out_hrq_func, 0, sizeof(m_out_hrq_func));
    	memset(&m_out_tc_func, 0, sizeof(m_out_tc_func));
    	memset(&m_out_mark_func, 0, sizeof(m_out_mark_func));
    	memset(&m_in_memr_func, 0, sizeof(m_in_memr_func));
    	memset(&m_out_memw_func, 0, sizeof(m_out_memw_func));
    	memset(&m_in_ior_func[0], 0, sizeof(m_in_ior_func[0]));
    	memset(&m_in_ior_func[1], 0, sizeof(m_in_ior_func[1]));
    	memset(&m_in_ior_func[2], 0, sizeof(m_in_ior_func[2]));
    	memset(&m_in_ior_func[3], 0, sizeof(m_in_ior_func[3]));
    	memset(&m_out_iow_func[0], 0, sizeof(m_out_iow_func[0]));
    	memset(&m_out_iow_func[1], 0, sizeof(m_out_iow_func[1]));
    	memset(&m_out_iow_func[2], 0, sizeof(m_out_iow_func[2]));
    	memset(&m_out_iow_func[3], 0, sizeof(m_out_iow_func[3]));
	}
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

const device_type I8257 = i8257_device_config::static_alloc_device_config;

//-------------------------------------------------
//  i8257_device - constructor
//-------------------------------------------------

i8257_device::i8257_device(running_machine &_machine, const i8257_device_config &config)
    : device_t(_machine, config),
      m_mode(0),
      m_rr(0),
      m_msb(0),
      m_drq(0),
      m_status(0x0f),
      m_config(config)
{
	memset(m_registers, 0, sizeof(m_registers));
	memset(m_address, 0, sizeof(m_address));
	memset(m_count, 0, sizeof(m_count));
	memset(m_rwmode, 0, sizeof(m_rwmode));
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void i8257_device::device_start()
{
	/* validate arguments */
	assert(this != NULL);

	/* resolve callbacks */
	devcb_resolve_write_line(&m_out_hrq_func, &m_config.m_out_hrq_func, this);
	devcb_resolve_write_line(&m_out_tc_func, &m_config.m_out_tc_func, this);
	devcb_resolve_write_line(&m_out_mark_func, &m_config.m_out_mark_func, this);
	devcb_resolve_read8(&m_in_memr_func, &m_config.m_in_memr_func, this);
	devcb_resolve_write8(&m_out_memw_func, &m_config.m_out_memw_func, this);

	for (int i = 0; i < I8257_NUM_CHANNELS; i++)
	{
		devcb_resolve_read8(&m_in_ior_func[i], &m_config.m_in_ior_func[i], this);
		devcb_resolve_write8(&m_out_iow_func[i], &m_config.m_out_iow_func[i], this);
	}

	/* set initial values */
	m_timer = device_timer_alloc(*this, TIMER_OPERATION);
	m_msbflip_timer = device_timer_alloc(*this, TIMER_MSBFLIP);

	/* register for state saving */
	state_save_register_device_item_array(this, 0, m_address);
	state_save_register_device_item_array(this, 0, m_count);
	state_save_register_device_item_array(this, 0, m_rwmode);
	state_save_register_device_item_array(this, 0, m_registers);
	state_save_register_device_item(this, 0, m_mode);
	state_save_register_device_item(this, 0, m_rr);
	state_save_register_device_item(this, 0, m_msb);
	state_save_register_device_item(this, 0, m_drq);
	state_save_register_device_item(this, 0, m_status);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void i8257_device::device_reset()
{
	m_status &= 0xf0;
	m_mode = 0;
	i8257_update_status();
}


int i8257_device::i8257_do_operation(int channel)
{
	int done;
	UINT8 data;

	UINT8 mode = m_rwmode[channel];
	if (m_count[channel] == 0x0000)
	{
		m_status |=  (0x01 << channel);

		devcb_call_write_line(&m_out_tc_func, ASSERT_LINE);
	}
	switch(mode) {
	case 1:
		if (&m_in_memr_func.target != NULL)
		{
			data = devcb_call_read8(&m_in_memr_func, m_address[channel]);
		}
		else
		{
			data = 0;
			logerror("8257: No memory read function defined.\n");
		}
		if (&m_out_iow_func[channel].target != NULL)
		{
			devcb_call_write8(&m_out_iow_func[channel], 0, data);
		}
		else
		{
			logerror("8257: No channel write function for channel %d defined.\n",channel);
		}

		m_address[channel]++;
		m_count[channel]--;
		done = (m_count[channel] == 0xFFFF);
		break;

	case 2:
		if (&m_in_ior_func[channel].target != NULL)
		{
			data = devcb_call_read8(&m_in_ior_func[channel], 0);
		}
		else
		{
			data = 0;
			logerror("8257: No channel read function for channel %d defined.\n",channel);
		}

		if (&m_out_memw_func.target != NULL)
		{
			devcb_call_write8(&m_out_memw_func, m_address[channel], data);
		}
		else
		{
			logerror("8257: No memory write function defined.\n");
		}
		m_address[channel]++;
		m_count[channel]--;
		done = (m_count[channel] == 0xFFFF);
		break;
	case 0: /* verify */
		m_address[channel]++;
		m_count[channel]--;
		done = (m_count[channel] == 0xFFFF);
		break;
	default:
		fatalerror("i8257_do_operation: invalid mode!\n");
		break;
	}
	if (done)
	{
		if ((channel==2) && DMA_MODE_AUTOLOAD(m_mode))
		{
			/* in case of autoload at the end channel 3 info is */
			/* copied to channel 2 info                         */
			m_registers[4] = m_registers[6];
			m_registers[5] = m_registers[7];
		}

		devcb_call_write_line(&m_out_tc_func, CLEAR_LINE);
	}
	return done;
}


void i8257_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		case TIMER_OPERATION:
		{
			int i, channel = 0, rr;
			int done;

			rr = DMA_MODE_ROTPRIO(m_mode) ? m_rr : 0;
			for (i = 0; i < I8257_NUM_CHANNELS; i++)
			{
				channel = (i + rr) % I8257_NUM_CHANNELS;
				if ((m_status & (1 << channel)) == 0)
				{
					if (m_mode & m_drq & (1 << channel))
					{
						break;
					}
				}
			}
			done = i8257_do_operation(channel);

			m_rr = (channel + 1) & 0x03;

			if (done)
			{
				m_drq &= ~(0x01 << channel);
				i8257_update_status();
				if (!(DMA_MODE_AUTOLOAD(m_mode) && channel==2))
				{
					if (DMA_MODE_TCSTOP(m_mode))
					{
						m_mode &= ~(0x01 << channel);
					}
				}
			}
			break;
		}

		case TIMER_MSBFLIP:
			m_msb ^= 1;
			break;

		case TIMER_DRQ_SYNC:
		{
			int channel = param >> 1;
			int state = param & 0x01;

			/* normalize state */
			if (state)
			{
				m_drq |= 0x01 << channel;
				m_address[channel] =  m_registers[channel * 2];
				m_count[channel] =  m_registers[channel * 2 + 1] & 0x3FFF;
				m_rwmode[channel] =  m_registers[channel * 2 + 1] >> 14;
				/* clear channel TC */
				m_status &= ~(0x01 << channel);
			}
			else
				m_drq &= ~(0x01 << channel);

			i8257_update_status();
			break;
		}
	}
}


void i8257_device::i8257_update_status()
{
	UINT16 pending_transfer;
	attotime next;

	/* no transfer is active right now; is there a transfer pending right now? */
	pending_transfer = m_drq & (m_mode & 0x0F);

	if (pending_transfer)
	{
		next = ATTOTIME_IN_HZ(clock() / 4 );
		timer_adjust_periodic(m_timer,
			attotime_zero,
			0,
			/* 1 byte transferred in 4 clock cycles */
			next);
	}
	else
	{
		/* no transfers active right now */
		timer_reset(m_timer, attotime_never);
	}

	/* set the halt line */
	devcb_call_write_line(&m_out_hrq_func, pending_transfer ? ASSERT_LINE : CLEAR_LINE);
}


void i8257_device::i8257_prepare_msb_flip()
{
	timer_adjust_oneshot(m_msbflip_timer, attotime_zero, 0);
}


READ8_DEVICE_HANDLER_TRAMPOLINE(i8257, i8257_r)
{
	UINT8 data = 0xFF;

	switch(offset)
	{
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
		/* DMA address/count register */
		data = ( m_registers[offset] >> (m_msb ? 8 : 0) ) & 0xFF;
		i8257_prepare_msb_flip();
		break;

	case 8:
		/* DMA status register */
		data = (UINT8) m_status;
		/* read resets status ! */
		m_status &= 0xF0;

		break;

	default:
		logerror("8257: Read from register %d.\n", offset);
		data = 0xFF;
		break;
	}
	return data;
}


WRITE8_DEVICE_HANDLER_TRAMPOLINE(i8257, i8257_w)
{
	switch(offset)
	{
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
		/* DMA address/count register */
		if (m_msb)
		{
			m_registers[offset] |= ((UINT16) data) << 8;
		}
		else
		{
			m_registers[offset] = data;
		}

		if (DMA_MODE_AUTOLOAD(m_mode))
		{
			/* in case of autoload when inserting channel 2 info */
			/* it is automaticaly copied to channel 3 info       */
			switch(offset)
			{
				case 4:
				case 5:
					if (m_msb)
					{
						m_registers[offset+2] |= ((UINT16) data) << 8;
					}
					else
					{
						m_registers[offset+2] = data;
					}
			}
		}

		i8257_prepare_msb_flip();
		break;

	case 8:
		/* DMA mode register */
		m_mode = data;
		break;

	default:
		logerror("8257: Write to register %d.\n", offset);
		break;
	}
}


void i8257_device::i8257_drq_w(int channel, int state)
{
	int param = (channel << 1) | (state ? 1 : 0);

	device_timer_call_after_resynch(*this, TIMER_DRQ_SYNC, param);
}

WRITE_LINE_DEVICE_HANDLER( i8257_hlda_w ) { }
WRITE_LINE_DEVICE_HANDLER( i8257_ready_w ) { }
WRITE_LINE_DEVICE_HANDLER( i8257_drq0_w ) { downcast<i8257_device*>(device)->i8257_drq_w(0, state); }
WRITE_LINE_DEVICE_HANDLER( i8257_drq1_w ) { downcast<i8257_device*>(device)->i8257_drq_w(1, state); }
WRITE_LINE_DEVICE_HANDLER( i8257_drq2_w ) { downcast<i8257_device*>(device)->i8257_drq_w(2, state); }
WRITE_LINE_DEVICE_HANDLER( i8257_drq3_w ) { downcast<i8257_device*>(device)->i8257_drq_w(3, state); }
