// license:BSD-3-Clause
// copyright-holders:Curt Coder,Carl
/***************************************************************************

    Intel 8257 DMA Controller emulation

***************************************************************************/

#include "i8257.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type I8257 = &device_creator<i8257_device>;



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0


enum
{
	REGISTER_ADDRESS = 0,
	REGISTER_WORD_COUNT,
	REGISTER_STATUS = 8,
	REGISTER_MODE = REGISTER_STATUS
};


#define MODE_CHAN_ENABLE(x)      BIT(m_transfer_mode, x)
#define MODE_ROTATING_PRIORITY   BIT(m_transfer_mode, 4)
#define MODE_EXTENDED_WRITE      BIT(m_transfer_mode, 5)
#define MODE_TC_STOP             BIT(m_transfer_mode, 6)
#define MODE_AUTOLOAD            BIT(m_transfer_mode, 7)
#define MODE_TRANSFER_MASK       (m_channel[m_current_channel].m_mode)
#define MODE_TRANSFER_VERIFY     0
#define MODE_TRANSFER_WRITE      1
#define MODE_TRANSFER_READ       2


enum
{
	STATE_SI,
	STATE_S0,
	STATE_S1,
	STATE_S2,
	STATE_S3,
	STATE_SW,
	STATE_S4
};



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  dma_request -
//-------------------------------------------------

inline void i8257_device::dma_request(int channel, int state)
{
	if (LOG) logerror("I8257 '%s' Channel %u DMA Request: %u\n", tag(), channel, state);

	if (state)
	{
		m_request |= 1 << channel;
	}
	else
	{
			m_request &= ~(1 << channel);
	}
	trigger(1);
}


//-------------------------------------------------
//  is_request_active -
//-------------------------------------------------

inline bool i8257_device::is_request_active(int channel)
{
	return (BIT(m_request, channel) && MODE_CHAN_ENABLE(channel)) ? true : false;
}

//-------------------------------------------------
//  set_hreq
//-------------------------------------------------

inline void i8257_device::set_hreq(int state)
{
	if (m_hreq != state)
	{
		m_out_hrq_cb(state);
		m_hreq = state;
	}
}


//-------------------------------------------------
//  set_tc -
//-------------------------------------------------

inline void i8257_device::set_tc(int state)
{
	if (m_tc != state)
	{
		m_out_tc_cb(state);

		m_tc = state;
	}
}


//-------------------------------------------------
//  set_dack - dack is active low
//-------------------------------------------------

inline void i8257_device::set_dack()
{
	m_out_dack_0_cb(m_current_channel != 0);
	m_out_dack_1_cb(m_current_channel != 1);
	m_out_dack_2_cb(m_current_channel != 2);
	m_out_dack_3_cb(m_current_channel != 3);
}


//-------------------------------------------------
//  dma_read -
//-------------------------------------------------

inline void i8257_device::dma_read()
{
	offs_t offset = m_channel[m_current_channel].m_address;

	switch (MODE_TRANSFER_MASK)
	{
	case MODE_TRANSFER_VERIFY:
	case MODE_TRANSFER_WRITE:
		switch(m_current_channel)
		{
			case 0:
				m_temp = m_in_ior_0_cb(offset);
				break;
			case 1:
				m_temp = m_in_ior_1_cb(offset);
				break;
			case 2:
				m_temp = m_in_ior_2_cb(offset);
				break;
			case 3:
				m_temp = m_in_ior_3_cb(offset);
				break;
		}
		break;

	case MODE_TRANSFER_READ:
		m_temp = m_in_memr_cb(offset);
		break;
	}

}


//-------------------------------------------------
//  dma_write -
//-------------------------------------------------

inline void i8257_device::dma_write()
{
	offs_t offset = m_channel[m_current_channel].m_address;

	switch (MODE_TRANSFER_MASK)
	{
	case MODE_TRANSFER_VERIFY: {
		UINT8 v1 = m_in_memr_cb(offset);
		if(0 && m_temp != v1)
			logerror("%s: verify error %02x vs. %02x\n", tag(), m_temp, v1);
		break;
	}

	case MODE_TRANSFER_WRITE:
		m_out_memw_cb(offset, m_temp);
		break;

	case MODE_TRANSFER_READ:
		switch(m_current_channel)
		{
			case 0:
				m_out_iow_0_cb(offset, m_temp);
				break;
			case 1:
				m_out_iow_1_cb(offset, m_temp);
				break;
			case 2:
				m_out_iow_2_cb(offset, m_temp);
				break;
			case 3:
				m_out_iow_3_cb(offset, m_temp);
				break;
		}
		break;
	}
}


//-------------------------------------------------
//  end_of_process -
//-------------------------------------------------

inline void i8257_device::advance()
{
	bool tc = (m_channel[m_current_channel].m_count == 0);
	bool al = (MODE_AUTOLOAD && (m_current_channel == 2));

	if(tc)
	{
		m_status |= 1 << m_current_channel;
		m_request &= ~(1 << m_current_channel); // docs imply this isn't right but pc-8001 works better with it
		set_tc(1);

		if(al)
		{
			// autoinitialize
			m_channel[2].m_address = m_channel[3].m_address;
			m_channel[2].m_count = m_channel[3].m_count;
			m_channel[2].m_mode = m_channel[3].m_mode;
		}
		else if(MODE_TC_STOP)
			// disable channel
			m_transfer_mode &= ~(1 << m_current_channel);
	}

	if(!(al && tc))
	{
		m_channel[m_current_channel].m_count--;
		m_channel[m_current_channel].m_count &= 0x3fff;
		m_channel[m_current_channel].m_address++;
	}
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  i8257_device - constructor
//-------------------------------------------------

i8257_device::i8257_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, I8257, "Intel 8257", tag, owner, clock, "i8257", __FILE__),
		device_execute_interface(mconfig, *this),
		m_icount(0),
		m_reverse_rw(0),
		m_tc(false),
		m_msb(0),
		m_hreq(CLEAR_LINE),
		m_hack(0),
		m_ready(1),
		m_state(0),
		m_current_channel(0),
		m_last_channel(0),
		m_transfer_mode(0),
		m_status(0),
		m_request(0),
		m_temp(0),
		m_out_hrq_cb(*this),
		m_out_tc_cb(*this),
		m_in_memr_cb(*this),
		m_out_memw_cb(*this),
		m_in_ior_0_cb(*this),
		m_in_ior_1_cb(*this),
		m_in_ior_2_cb(*this),
		m_in_ior_3_cb(*this),
		m_out_iow_0_cb(*this),
		m_out_iow_1_cb(*this),
		m_out_iow_2_cb(*this),
		m_out_iow_3_cb(*this),
		m_out_dack_0_cb(*this),
		m_out_dack_1_cb(*this),
		m_out_dack_2_cb(*this),
		m_out_dack_3_cb(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void i8257_device::device_start()
{
	// set our instruction counter
	m_icountptr = &m_icount;

	// resolve callbacks
	m_out_hrq_cb.resolve_safe();
	m_out_tc_cb.resolve_safe();
	m_in_memr_cb.resolve_safe(0);
	m_out_memw_cb.resolve_safe();
	m_in_ior_0_cb.resolve_safe(0);
	m_in_ior_1_cb.resolve_safe(0);
	m_in_ior_2_cb.resolve_safe(0);
	m_in_ior_3_cb.resolve_safe(0);
	m_out_iow_0_cb.resolve_safe();
	m_out_iow_1_cb.resolve_safe();
	m_out_iow_2_cb.resolve_safe();
	m_out_iow_3_cb.resolve_safe();
	m_out_dack_0_cb.resolve_safe();
	m_out_dack_1_cb.resolve_safe();
	m_out_dack_2_cb.resolve_safe();
	m_out_dack_3_cb.resolve_safe();

	// state saving
	save_item(NAME(m_msb));
	save_item(NAME(m_hreq));
	save_item(NAME(m_hack));
	save_item(NAME(m_ready));
	save_item(NAME(m_state));
	save_item(NAME(m_current_channel));
	save_item(NAME(m_last_channel));
	save_item(NAME(m_transfer_mode));
	save_item(NAME(m_status));
	save_item(NAME(m_request));

	save_item(NAME(m_channel[0].m_address));
	save_item(NAME(m_channel[0].m_count));
	save_item(NAME(m_channel[0].m_mode));
	save_item(NAME(m_channel[1].m_address));
	save_item(NAME(m_channel[1].m_count));
	save_item(NAME(m_channel[1].m_mode));
	save_item(NAME(m_channel[2].m_address));
	save_item(NAME(m_channel[2].m_count));
	save_item(NAME(m_channel[2].m_mode));
	save_item(NAME(m_channel[3].m_address));
	save_item(NAME(m_channel[3].m_count));
	save_item(NAME(m_channel[3].m_mode));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void i8257_device::device_reset()
{
	m_state = STATE_SI;
	m_transfer_mode = 0;
	m_status = 0;
	m_request = 0;
	m_msb = 0;
	m_current_channel = -1;
	m_last_channel = 3;
	m_hreq = -1;
	m_tc = 0;

	for (auto & elem : m_channel)
	{
		elem.m_address = 0;
		elem.m_count = 0;
		elem.m_mode = 0;
	}
	set_hreq(0);
	set_dack();
}

bool i8257_device::next_channel()
{
	int priority[] = { 0, 1, 2, 3 };

	if (MODE_ROTATING_PRIORITY)
	{
		int last_channel = m_last_channel;

		for (int channel = 3; channel >= 0; channel--)
		{
			priority[channel] = last_channel;
			last_channel--;
			if (last_channel < 0) last_channel = 3;
		}
	}

	for (auto & elem : priority)
	{
		if (is_request_active(elem))
		{
			m_current_channel = m_last_channel = elem;
			return true;
		}
	}
	return false;
}


//-------------------------------------------------
//  execute_run -
//-------------------------------------------------

void i8257_device::execute_run()
{
	do
	{
		switch (m_state)
		{
		case STATE_SI:
			set_tc(0);
			if(next_channel())
				m_state = STATE_S0;
			else
			{
				suspend_until_trigger(1, true);
				m_icount = 0;
			}
			break;

		case STATE_S0:
			set_hreq(1);

			if (m_hack)
			{
				m_state = STATE_S1;
			}
			else
			{
				suspend_until_trigger(1, true);
				m_icount = 0;
			}
			break;

		case STATE_S1:
			set_tc(0);
			m_state = STATE_S2;
			break;

		case STATE_S2:
			set_dack();
			m_state = STATE_S3;
			break;

		case STATE_S3:
			dma_read();

			if (MODE_EXTENDED_WRITE)
			{
				dma_write();
			}

			m_state = m_ready ? STATE_S4 : STATE_SW;
			break;

		case STATE_SW:
			m_state = m_ready ? STATE_S4 : STATE_SW;
			break;

		case STATE_S4:
			if (!MODE_EXTENDED_WRITE)
			{
				dma_write();
			}
			advance();

			if(next_channel())
				m_state = STATE_S1;
			else
			{
				set_hreq(0);
				m_current_channel = -1;
				m_state = STATE_SI;
				set_dack();
			}
			break;
		}
		m_icount--;
	} while (m_icount > 0);
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

READ8_MEMBER( i8257_device::read )
{
	UINT8 data = 0;

	if (!BIT(offset, 3))
	{
		int channel = (offset >> 1) & 0x03;

		switch (offset & 0x01)
		{
		case REGISTER_ADDRESS:
			if (m_msb)
			{
				data = m_channel[channel].m_address >> 8;
			}
			else
			{
				data = m_channel[channel].m_address & 0xff;
			}
			break;

		case REGISTER_WORD_COUNT:
			if (m_msb)
			{
				data = (m_channel[channel].m_count >> 8);
				if(m_reverse_rw && m_channel[channel].m_mode)
					data |= (m_channel[channel].m_mode == 1) ? 0x80 : 0x40;
				else
					data |= (m_channel[channel].m_mode << 6);
			}
			else
			{
				data = m_channel[channel].m_count & 0xff;
			}
			break;
		}

		m_msb = !m_msb;
	}
	else if(offset == REGISTER_STATUS)
	{
		data = m_status;

		// clear TC bits
		m_status &= 0xf0;
	}

	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

WRITE8_MEMBER( i8257_device::write )
{
	if (!BIT(offset, 3))
	{
		int channel = (offset >> 1) & 0x03;

		switch (offset & 0x01)
		{
		case REGISTER_ADDRESS:
			if (m_msb)
			{
				m_channel[channel].m_address = (data << 8) | (m_channel[channel].m_address & 0xff);
				if(MODE_AUTOLOAD && (channel == 2))
					m_channel[3].m_address = (data << 8) | (m_channel[3].m_address & 0xff);
			}
			else
			{
				m_channel[channel].m_address = (m_channel[channel].m_address & 0xff00) | data;
				if(MODE_AUTOLOAD && (channel == 2))
					m_channel[3].m_address = (m_channel[3].m_address & 0xff00) | data;
			}
			break;

		case REGISTER_WORD_COUNT:
			if (m_msb)
			{
				m_channel[channel].m_count = ((data & 0x3f) << 8) | (m_channel[channel].m_count & 0xff);
				m_channel[channel].m_mode = (data >> 6);

				if(m_reverse_rw && m_channel[channel].m_mode)
					m_channel[channel].m_mode = (m_channel[channel].m_mode == 1) ? 2 : 1;

				if(MODE_AUTOLOAD && (channel == 2))
				{
					m_channel[3].m_count = ((data & 0x3f) << 8) | (m_channel[3].m_count & 0xff);
					m_channel[3].m_mode = m_channel[2].m_mode;
				}
			}
			else
			{
				m_channel[channel].m_count = (m_channel[channel].m_count & 0xff00) | data;
				if(MODE_AUTOLOAD && (channel == 2))
					m_channel[3].m_count = (m_channel[3].m_count & 0xff00) | data;
			}
			break;
		}

		m_msb = !m_msb;
	}
	else if(offset == REGISTER_MODE)
	{
		m_transfer_mode = data;

		if (LOG) logerror("I8257 '%s' Command Register: %02x\n", tag(), m_transfer_mode);
	}
	trigger(1);
}


//-------------------------------------------------
//  hlda_w - hold acknowledge
//-------------------------------------------------

WRITE_LINE_MEMBER( i8257_device::hlda_w )
{
	if (LOG) logerror("I8257 '%s' Hold Acknowledge: %u\n", tag(), state);

	m_hack = state;
	trigger(1);
}


//-------------------------------------------------
//  ready_w - ready
//-------------------------------------------------

WRITE_LINE_MEMBER( i8257_device::ready_w )
{
	if (LOG) logerror("I8257 '%s' Ready: %u\n", tag(), state);

	m_ready = state;
}


//-------------------------------------------------
//  dreq0_w - DMA request for channel 0
//-------------------------------------------------

WRITE_LINE_MEMBER( i8257_device::dreq0_w )
{
	dma_request(0, state);
}


//-------------------------------------------------
//  dreq0_w - DMA request for channel 1
//-------------------------------------------------

WRITE_LINE_MEMBER( i8257_device::dreq1_w )
{
	dma_request(1, state);
}


//-------------------------------------------------
//  dreq1_w - DMA request for channel 2
//-------------------------------------------------

WRITE_LINE_MEMBER( i8257_device::dreq2_w )
{
	dma_request(2, state);
}


//-------------------------------------------------
//  dreq3_w - DMA request for channel 3
//-------------------------------------------------

WRITE_LINE_MEMBER( i8257_device::dreq3_w )
{
	dma_request(3, state);
}
