// license:BSD-3-Clause
// copyright-holders:Curt Coder,Carl
/***************************************************************************

    Intel 8257 DMA Controller emulation

***************************************************************************/

#include "emu.h"
#include "i8257.h"

#define LOG_SETUP     (1U << 1)
#define LOG_TFR       (1U << 2)

//#define VERBOSE (LOG_GENERAL | LOG_SETUP | LOG_TFR)
//#define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

#define LOGSETUP(...) LOGMASKED(LOG_SETUP, __VA_ARGS__)
#define LOGTFR(...)   LOGMASKED(LOG_TFR, __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(I8257, i8257_device, "i8257", "Intel 8257 DMA Controller")



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

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
	LOG("I8257 Channel %u DMA Request: %u (%sabled)\n", channel, state, MODE_CHAN_ENABLE(channel) ? "en" : "dis");

	if (state)
		m_request |= 1 << channel;
	else
		m_request &= ~(1 << channel);

	trigger(1);
}


//-------------------------------------------------
//  is_request_active -
//-------------------------------------------------

inline bool i8257_device::is_request_active(int channel) const
{
	LOG("%s Channel %d: %02x && MODE_CHAN_ENABLE:%02x\n", FUNCNAME, channel, m_request, MODE_CHAN_ENABLE(channel));
	return BIT(m_request, channel) && MODE_CHAN_ENABLE(channel);
}

//-------------------------------------------------
//  set_hreq
//-------------------------------------------------

inline void i8257_device::set_hreq(int state)
{
	LOG("%s\n", FUNCNAME);
	if (m_hreq != state)
	{
		m_out_hrq_cb(state);
		m_hreq = state;
		abort_timeslice();
	}
}


//-------------------------------------------------
//  set_tc -
//-------------------------------------------------

inline void i8257_device::set_tc(int state)
{
	LOG("%s: %d\n", FUNCNAME, state);
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
	LOG("%s\n", FUNCNAME);
	for (int ch = 0; ch < 4; ch++)
		m_out_dack_cb[ch](m_current_channel != ch);
}


//-------------------------------------------------
//  dma_read -
//-------------------------------------------------

inline void i8257_device::dma_read()
{
	LOG("%s\n", FUNCNAME);
	offs_t offset = m_channel[m_current_channel].m_address;

	switch (MODE_TRANSFER_MASK)
	{
	case MODE_TRANSFER_VERIFY:
		break;
	case MODE_TRANSFER_WRITE:
		LOGTFR(" - MODE TRANSFER VERIFY/WRITE");
		m_temp = m_in_ior_cb[m_current_channel](offset);
		break;

	case MODE_TRANSFER_READ:
		LOGTFR(" - MODE TRANSFER READ");
		m_temp = m_in_memr_cb(offset);
		break;
	}
	LOGTFR(" channel %d Offset %04x: %02x\n", m_current_channel, offset, m_temp);
}


//-------------------------------------------------
//  dma_write -
//-------------------------------------------------

inline void i8257_device::dma_write()
{
	LOG("%s\n", FUNCNAME);
	offs_t offset = m_channel[m_current_channel].m_address;

	switch (MODE_TRANSFER_MASK)
	{
	case MODE_TRANSFER_VERIFY: {
		LOGTFR(" - MODE TRANSFER VERIFY");
		m_verify_cb[m_current_channel](offset);
		break;
	}

	case MODE_TRANSFER_WRITE:
		LOGTFR(" - MODE TRANSFER WRITE");
		m_out_memw_cb(offset, m_temp);
		break;

	case MODE_TRANSFER_READ:
		LOGTFR(" - MODE TRANSFER READ");
		m_out_iow_cb[m_current_channel](offset, m_temp);
		break;
	}
	LOGTFR(" channel %d Offset %04x: %02x %04x\n", m_current_channel, offset, m_temp, m_channel[m_current_channel].m_count);
}


//-------------------------------------------------
//  end_of_process -
//-------------------------------------------------

inline void i8257_device::advance()
{
	LOG("%s\n", FUNCNAME);
	bool tc = m_tc;
	bool al = (MODE_AUTOLOAD && (m_current_channel == 2));

	set_tc(0);
	if(tc)
	{
		m_status |= 1 << m_current_channel;

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

i8257_device::i8257_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, I8257, tag, owner, clock)
	, device_execute_interface(mconfig, *this)
	, m_icount(0)
	, m_reverse_rw(0)
	, m_tc(false)
	, m_msb(0)
	, m_hreq(CLEAR_LINE)
	, m_hack(0)
	, m_ready(1)
	, m_state(0)
	, m_current_channel(0)
	, m_last_channel(0)
	, m_transfer_mode(0)
	, m_status(0)
	, m_request(0)
	, m_temp(0)
	, m_out_hrq_cb(*this)
	, m_out_tc_cb(*this)
	, m_in_memr_cb(*this, 0)
	, m_out_memw_cb(*this)
	, m_in_ior_cb(*this, 0)
	, m_out_iow_cb(*this)
	, m_verify_cb(*this, 0)
	, m_out_dack_cb(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void i8257_device::device_start()
{
	LOG("%s\n", FUNCNAME);
	// set our instruction counter
	set_icountptr(m_icount);

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

	save_item(STRUCT_MEMBER(m_channel, m_address));
	save_item(STRUCT_MEMBER(m_channel, m_count));
	save_item(STRUCT_MEMBER(m_channel, m_mode));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void i8257_device::device_reset()
{
	LOG("%s\n", FUNCNAME);
	m_state = STATE_SI;
	m_transfer_mode = 0;
	m_status = 0;
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
	LOG("%s\n", FUNCNAME);
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
	LOG("%s\n", FUNCNAME);
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
				m_icount = 0;
				suspend_until_trigger(1, true);
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
				m_icount = 0;
				suspend_until_trigger(1, true);
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

			if (m_ready)
			{
				m_state = STATE_S4;
				if ((m_channel[m_current_channel].m_count == 0) && (MODE_TRANSFER_MASK != MODE_TRANSFER_READ))
					set_tc(1);
			}
			else
				m_state = STATE_SW;
			break;

		case STATE_SW:
			if (m_ready)
			{
				m_state = STATE_S4;
				if ((m_channel[m_current_channel].m_count == 0) && (MODE_TRANSFER_MASK != MODE_TRANSFER_READ))
					set_tc(1);
			}
			break;

		case STATE_S4:
			if (!MODE_EXTENDED_WRITE)
			{
				dma_write();
			}
			if ((m_channel[m_current_channel].m_count == 0) && (MODE_TRANSFER_MASK == MODE_TRANSFER_READ))
				set_tc(1);
			advance();

			if(m_hack && next_channel())
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

uint8_t i8257_device::read(offs_t offset)
{
	LOG("%s\n", FUNCNAME);
	uint8_t data = 0;

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

		if (!machine().side_effects_disabled())
			m_msb = !m_msb;
	}
	else if(offset == REGISTER_STATUS)
	{
		data = m_status;

		// clear TC bits
		if (!machine().side_effects_disabled())
			m_status &= 0xf0;
	}

	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void i8257_device::write(offs_t offset, uint8_t data)
{
	LOG("%s \n", FUNCNAME);
	if (!BIT(offset, 3))
	{
		int channel = (offset >> 1) & 0x03;

		switch (offset & 0x01)
		{
		case REGISTER_ADDRESS:
			LOGSETUP(" * Channel %d Register Address <- %02x\n", channel, data);
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
			LOGSETUP(" * Channel %d Register Word Count <- %02x\n", channel, data);
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

		LOGSETUP("I8257 Command Register: %02x\n", m_transfer_mode);
	}

	if ((m_transfer_mode & m_request & 0x0f) != 0)
	{
		machine().scheduler().eat_all_cycles();
		trigger(1);
	}
}


//-------------------------------------------------
//  hlda_w - hold acknowledge
//-------------------------------------------------

void i8257_device::hlda_w(int state)
{
	LOG("I8257 Hold Acknowledge: %u\n", state);

	m_hack = state;
	trigger(1);
}


//-------------------------------------------------
//  ready_w - ready
//-------------------------------------------------

void i8257_device::ready_w(int state)
{
	LOG("I8257 Ready: %u\n", state);

	m_ready = state;
}


//-------------------------------------------------
//  dreq0_w - DMA request for channel 0
//-------------------------------------------------

void i8257_device::dreq0_w(int state)
{
	LOG("%s\n", FUNCNAME);
	dma_request(0, state);
}


//-------------------------------------------------
//  dreq0_w - DMA request for channel 1
//-------------------------------------------------

void i8257_device::dreq1_w(int state)
{
	LOG("%s\n", FUNCNAME);
	dma_request(1, state);
}


//-------------------------------------------------
//  dreq1_w - DMA request for channel 2
//-------------------------------------------------

void i8257_device::dreq2_w(int state)
{
	LOG("%s\n", FUNCNAME);
	dma_request(2, state);
}


//-------------------------------------------------
//  dreq3_w - DMA request for channel 3
//-------------------------------------------------

void i8257_device::dreq3_w(int state)
{
	LOG("%s\n", FUNCNAME);
	dma_request(3, state);
}
