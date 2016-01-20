// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    AMD AM9517A
    Intel 8237A
    NEC uPD71037

    NEC uPD71071 (extended version of above)

    a variant is used in the V53 CPU which offers subsets of both the
    uPD71071 and uPD71037 functionality depending on a mode bit.

    Multimode DMA Controller emulation

***************************************************************************/

/*

    TODO:

    - external EOP

*/

/*

    When the V53 operates in uPD71071 compatible mode there are the following
    differences from a real uPD71071

                               V53     Real uPD71071
    Software Reqs              No      Yes
    Memory-to-Memory DMA       No      Yes
    DMARQ active level         High    programmable
    DMAAK active level         Low     programmable
    Bus Cycle                  4       4 or 3

    we don't currently handle the differences

*/

#include "am9517a.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type AM9517A = &device_creator<am9517a_device>;
const device_type V53_DMAU = &device_creator<upd71071_v53_device>;


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0


enum
{
	REGISTER_ADDRESS = 0,
	REGISTER_WORD_COUNT,
	REGISTER_STATUS = 8,
	REGISTER_COMMAND = REGISTER_STATUS,
	REGISTER_REQUEST,
	REGISTER_SINGLE_MASK,
	REGISTER_MODE,
	REGISTER_BYTE_POINTER,
	REGISTER_TEMPORARY,
	REGISTER_MASTER_CLEAR = REGISTER_TEMPORARY,
	REGISTER_CLEAR_MASK,
	REGISTER_MASK
};


#define COMMAND_MEM_TO_MEM          BIT(m_command, 0)
#define COMMAND_CH0_ADDRESS_HOLD    BIT(m_command, 1)
#define COMMAND_DISABLE             BIT(m_command, 2)
#define COMMAND_COMPRESSED_TIMING   BIT(m_command, 3)
#define COMMAND_ROTATING_PRIORITY   BIT(m_command, 4)
#define COMMAND_EXTENDED_WRITE      BIT(m_command, 5)
#define COMMAND_DREQ_ACTIVE_LOW     BIT(m_command, 6)
#define COMMAND_DACK_ACTIVE_HIGH    BIT(m_command, 7)


#define MODE_TRANSFER_MASK          (m_channel[m_current_channel].m_mode & 0x0c)
#define MODE_TRANSFER_VERIFY        0x00
#define MODE_TRANSFER_WRITE         0x04
#define MODE_TRANSFER_READ          0x08
#define MODE_TRANSFER_ILLEGAL       0x0c
#define MODE_AUTOINITIALIZE         BIT(m_channel[m_current_channel].m_mode, 4)
#define MODE_ADDRESS_DECREMENT      BIT(m_channel[m_current_channel].m_mode, 5)
#define MODE_MASK                   (m_channel[m_current_channel].m_mode & 0xc0)
#define MODE_DEMAND                 0x00
#define MODE_SINGLE                 0x40
#define MODE_BLOCK                  0x80
#define MODE_CASCADE                0xc0


enum
{
	STATE_SI,
	STATE_S0,
	STATE_SC,
	STATE_S1,
	STATE_S2,
	STATE_S3,
	STATE_SW,
	STATE_S4,
	STATE_S11,
	STATE_S12,
	STATE_S13,
	STATE_S14,
	STATE_S21,
	STATE_S22,
	STATE_S23,
	STATE_S24
};



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  dma_request -
//-------------------------------------------------

inline void am9517a_device::dma_request(int channel, int state)
{
	if (LOG) logerror("AM9517A '%s' Channel %u DMA Request: %u\n", tag().c_str(), channel, state);

	if (state ^ COMMAND_DREQ_ACTIVE_LOW)
	{
		m_status |= (1 << (channel + 4));
	}
	else
	{
		m_status &= ~(1 << (channel + 4));
	}
	trigger(1);
}


//-------------------------------------------------
//  is_request_active -
//-------------------------------------------------

inline bool am9517a_device::is_request_active(int channel)
{
	return (BIT(m_status, channel + 4) & ~BIT(m_mask, channel)) ? true : false;
}


//-------------------------------------------------
//  is_software_request_active -
//-------------------------------------------------

inline bool am9517a_device::is_software_request_active(int channel)
{
	return BIT(m_request, channel) && ((m_channel[channel].m_mode & 0xc0) == MODE_BLOCK);
}


//-------------------------------------------------
//  set_hreq
//-------------------------------------------------

inline void am9517a_device::set_hreq(int state)
{
	if (m_hreq != state)
	{
		m_out_hreq_cb(state);

		m_hreq = state;
	}
}


//-------------------------------------------------
//  set_dack -
//-------------------------------------------------

inline void am9517a_device::set_dack()
{
	for (int channel = 0; channel < 4; channel++)
	{
		if (channel == 0)
		{
			if ((channel == m_current_channel) && !COMMAND_MEM_TO_MEM)
			{
				m_out_dack_0_cb(COMMAND_DACK_ACTIVE_HIGH);
			}
			else
			{
				m_out_dack_0_cb(!COMMAND_DACK_ACTIVE_HIGH);
			}
		}
		else if (channel == 1)
		{
			if ((channel == m_current_channel) && !COMMAND_MEM_TO_MEM)
			{
				m_out_dack_1_cb(COMMAND_DACK_ACTIVE_HIGH);
			}
			else
			{
				m_out_dack_1_cb(!COMMAND_DACK_ACTIVE_HIGH);
			}
		}
		else if (channel == 2)
		{
			if ((channel == m_current_channel) && !COMMAND_MEM_TO_MEM)
			{
				m_out_dack_2_cb(COMMAND_DACK_ACTIVE_HIGH);
			}
			else
			{
				m_out_dack_2_cb(!COMMAND_DACK_ACTIVE_HIGH);
			}
		}
		else if (channel == 3)
		{
			if ((channel == m_current_channel) && !COMMAND_MEM_TO_MEM)
			{
				m_out_dack_3_cb(COMMAND_DACK_ACTIVE_HIGH);
			}
			else
			{
				m_out_dack_3_cb(!COMMAND_DACK_ACTIVE_HIGH);
			}
		}
	}
}


//-------------------------------------------------
//  set_eop -
//-------------------------------------------------

inline void am9517a_device::set_eop(int state)
{
	if (m_eop != state)
	{
		m_out_eop_cb(state);

		m_eop = state;
	}
}


//-------------------------------------------------
//  dma_read -
//-------------------------------------------------

inline int am9517a_device::get_state1(bool msb_changed)
{
	if (COMMAND_MEM_TO_MEM)
	{
		return msb_changed ? STATE_S11 : STATE_S12;
	}
	else
	{
		return msb_changed ? STATE_S1 : STATE_S2;
	}
}


//-------------------------------------------------
//  dma_read -
//-------------------------------------------------

inline void am9517a_device::dma_read()
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

inline void am9517a_device::dma_write()
{
	offs_t offset = m_channel[m_current_channel].m_address;

	switch (MODE_TRANSFER_MASK)
	{
	case MODE_TRANSFER_VERIFY: {
		UINT8 v1 = m_in_memr_cb(offset);
		if(0 && m_temp != v1)
			logerror("%s: verify error %02x vs. %02x\n", tag().c_str(), m_temp, v1);
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
//  dma_advance -
//-------------------------------------------------

inline void am9517a_device::dma_advance()
{
	bool msb_changed = false;

	m_channel[m_current_channel].m_count--;

	if (m_current_channel || !COMMAND_MEM_TO_MEM || !COMMAND_CH0_ADDRESS_HOLD)
	{
		if (MODE_ADDRESS_DECREMENT)
		{
			m_channel[m_current_channel].m_address--;
			m_channel[m_current_channel].m_address &= m_address_mask;

			if ((m_channel[m_current_channel].m_address & 0xff) == 0xff)
			{
				msb_changed = true;
			}
		}
		else
		{
			m_channel[m_current_channel].m_address++;
			m_channel[m_current_channel].m_address &= m_address_mask;

			if ((m_channel[m_current_channel].m_address & 0xff) == 0x00)
			{
				msb_changed = true;
			}
		}
	}

	if (m_channel[m_current_channel].m_count == 0xffff)
	{
		end_of_process();
	}
	else
	{
		switch (MODE_MASK)
		{
		case MODE_DEMAND:
			if (!is_request_active(m_current_channel))
			{
				set_hreq(0);
				set_dack();
				m_state = STATE_SI;
			}
			else
			{
				m_state = get_state1(msb_changed);
			}
			break;

		case MODE_SINGLE:
			set_hreq(0);
			set_dack();
			m_state = STATE_SI;
			break;

		case MODE_BLOCK:
			m_state = get_state1(msb_changed);
			break;

		case MODE_CASCADE:
			break;
		}
	}
}


//-------------------------------------------------
//  end_of_process -
//-------------------------------------------------

inline void am9517a_device::end_of_process()
{
	// terminal count
	if (COMMAND_MEM_TO_MEM)
	{
		m_status |= 1 << 0;
		m_status |= 1 << 1;
		m_request &= ~(1 << 0);
		m_request &= ~(1 << 1);
	}
	else
	{
		m_status |= 1 << m_current_channel;
		m_request &= ~(1 << m_current_channel);
	}

	if (MODE_AUTOINITIALIZE)
	{
		// autoinitialize
		m_channel[m_current_channel].m_address = m_channel[m_current_channel].m_base_address;
		m_channel[m_current_channel].m_count = m_channel[m_current_channel].m_base_count;
	}
	else
	{
		// mask out channel
		m_mask |= 1 << m_current_channel;
	}

	// signal end of process
	set_eop(ASSERT_LINE);
	set_hreq(0);

	m_current_channel = -1;
	set_dack();

	m_state = STATE_SI;
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  am9517a_device - constructor
//-------------------------------------------------


am9517a_device::am9517a_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, __FILE__),
		device_execute_interface(mconfig, *this),
		m_icount(0),
		m_hack(0),
		m_ready(1),
		m_command(0),
		m_out_hreq_cb(*this),
		m_out_eop_cb(*this),
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


am9517a_device::am9517a_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, AM9517A, "AM9517A", tag, owner, clock, "am9517a", __FILE__),
		device_execute_interface(mconfig, *this),
		m_icount(0),
		m_hack(0),
		m_ready(1),
		m_command(0),
		m_out_hreq_cb(*this),
		m_out_eop_cb(*this),
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

upd71071_v53_device::upd71071_v53_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: am9517a_device(mconfig, V53_DMAU, "V53 DMAU", tag, owner, clock, "v53_dmau")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void am9517a_device::device_start()
{
	// set our instruction counter
	m_icountptr = &m_icount;

	// resolve callbacks
	m_out_hreq_cb.resolve_safe();
	m_out_eop_cb.resolve_safe();
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

	for (auto & elem : m_channel)
	{
		elem.m_address = 0;
		elem.m_count = 0;
		elem.m_base_address = 0;
		elem.m_base_count = 0;
		elem.m_mode = 0;
	}

	// state saving
	save_item(NAME(m_msb));
	save_item(NAME(m_hreq));
	save_item(NAME(m_hack));
	save_item(NAME(m_ready));
	save_item(NAME(m_eop));
	save_item(NAME(m_state));
	save_item(NAME(m_current_channel));
	save_item(NAME(m_last_channel));
	save_item(NAME(m_command));
	save_item(NAME(m_mask));
	save_item(NAME(m_status));
	save_item(NAME(m_temp));
	save_item(NAME(m_request));

	for (int i = 0; i < 4; i++)
	{
		save_item(NAME(m_channel[i].m_address), i);
		save_item(NAME(m_channel[i].m_count), i);
		save_item(NAME(m_channel[i].m_base_address), i);
		save_item(NAME(m_channel[i].m_base_count), i);
		save_item(NAME(m_channel[i].m_mode), i);
	}

	m_address_mask = 0xffff;

}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void am9517a_device::device_reset()
{
	m_state = STATE_SI;
	m_command = 0;
	m_status = 0;
	m_request = 0;
	m_mask = 0x0f;
	m_temp = 0;
	m_msb = 0;
	m_current_channel = -1;
	m_last_channel = 3;
	m_hreq = -1;
	m_eop = 0;

	set_hreq(0);
	set_eop(ASSERT_LINE);

	set_dack();
}


//-------------------------------------------------
//  execute_run -
//-------------------------------------------------

void am9517a_device::execute_run()
{
	do
	{
		switch (m_state)
		{
		case STATE_SI:
			set_eop(CLEAR_LINE);

			if (!COMMAND_DISABLE)
			{
				int priority[] = { 0, 1, 2, 3 };

				if (COMMAND_ROTATING_PRIORITY)
				{
					int last_channel = m_last_channel;

					for (int channel = 3; channel >= 0; channel--)
					{
						priority[channel] = last_channel;
						last_channel--;
						if (last_channel < 0) last_channel = 3;
					}
				}

				for (int channel = 0; channel < 4; channel++)
				{
					if (is_request_active(priority[channel]) || is_software_request_active(priority[channel]))
					{
						m_current_channel = m_last_channel = priority[channel];
						m_state = STATE_S0;
						break;
					}
					else if (COMMAND_MEM_TO_MEM && BIT(m_request, channel) && ((m_channel[channel].m_mode & 0xc0) == MODE_SINGLE))
					{
						m_current_channel = m_last_channel = priority[channel];
						m_state = STATE_S0;
						break;
					}
				}
			}
			if(m_state == STATE_SI)
			{
				suspend_until_trigger(1, true);
				m_icount = 0;
			}
			break;

		case STATE_S0:
			set_hreq(1);

			if (m_hack)
			{
				m_state = (MODE_MASK == MODE_CASCADE) ? STATE_SC : get_state1(true);
			}
			else
			{
				suspend_until_trigger(1, true);
				m_icount = 0;
			}
			break;

		case STATE_SC:
			if (!is_request_active(m_current_channel))
			{
				set_hreq(0);
				m_current_channel = -1;
				m_state = STATE_SI;
			}
			else
			{
				suspend_until_trigger(1, true);
				m_icount = 0;
			}

			set_dack();
			break;

		case STATE_S1:
			m_state = STATE_S2;
			break;

		case STATE_S2:
			set_dack();
			m_state = COMMAND_COMPRESSED_TIMING ? STATE_S4 : STATE_S3;
			break;

		case STATE_S3:
			dma_read();

			if (COMMAND_EXTENDED_WRITE)
			{
				dma_write();
			}

			m_state = m_ready ? STATE_S4 : STATE_SW;
			break;

		case STATE_SW:
			m_state = m_ready ? STATE_S4 : STATE_SW;
			break;

		case STATE_S4:
			if (COMMAND_COMPRESSED_TIMING)
			{
				dma_read();
				dma_write();
			}
			else if (!COMMAND_EXTENDED_WRITE)
			{
				dma_write();
			}

			dma_advance();
			break;

		case STATE_S11:
			m_current_channel = 0;

			m_state = STATE_S12;
			break;

		case STATE_S12:
			m_state = STATE_S13;
			break;

		case STATE_S13:
			m_state = STATE_S14;
			break;

		case STATE_S14:
			dma_read();

			m_state = STATE_S21;
			break;

		case STATE_S21:
			m_current_channel = 1;

			m_state = STATE_S22;
			break;

		case STATE_S22:
			m_state = STATE_S23;
			break;

		case STATE_S23:
			m_state = STATE_S24;
			break;

		case STATE_S24:
			dma_write();
			dma_advance();

			m_current_channel = 0;
			m_channel[m_current_channel].m_count--;
			if (MODE_ADDRESS_DECREMENT)
			{
				m_channel[m_current_channel].m_address--;
				m_channel[m_current_channel].m_address &= m_address_mask;
			}
			else
			{
				m_channel[m_current_channel].m_address++;
				m_channel[m_current_channel].m_address &= m_address_mask;
			}

			break;
		}

		m_icount--;
	} while (m_icount > 0);
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

READ8_MEMBER( am9517a_device::read )
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
				data = m_channel[channel].m_count >> 8;
			}
			else
			{
				data = m_channel[channel].m_count & 0xff;
			}
			break;
		}

		m_msb = !m_msb;
	}
	else
	{
		switch (offset & 0x0f)
		{
		case REGISTER_STATUS:
			data = m_status;

			// clear TC bits
			m_status &= 0xf0;
			break;

		case REGISTER_TEMPORARY:
			data = m_temp;
			break;

		case REGISTER_MASK:
			data = m_mask;
			break;
		}
	}

	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

WRITE8_MEMBER( am9517a_device::write )
{
	if (!BIT(offset, 3))
	{
		int channel = (offset >> 1) & 0x03;

		switch (offset & 0x01)
		{
		case REGISTER_ADDRESS:
			if (m_msb)
			{
				m_channel[channel].m_base_address = (data << 8) | (m_channel[channel].m_base_address & 0xff);
				m_channel[channel].m_address = (data << 8) | (m_channel[channel].m_address & 0xff);
			}
			else
			{
				m_channel[channel].m_base_address = (m_channel[channel].m_base_address & 0xff00) | data;
				m_channel[channel].m_address = (m_channel[channel].m_address & 0xff00) | data;
			}

			if (LOG) logerror("AM9517A '%s' Channel %u Base Address: %04x\n", tag().c_str(), channel, m_channel[channel].m_base_address);
			break;

		case REGISTER_WORD_COUNT:
			if (m_msb)
			{
				m_channel[channel].m_base_count = (data << 8) | (m_channel[channel].m_base_count & 0xff);
				m_channel[channel].m_count = (data << 8) | (m_channel[channel].m_count & 0xff);
			}
			else
			{
				m_channel[channel].m_base_count = (m_channel[channel].m_base_count & 0xff00) | data;
				m_channel[channel].m_count = (m_channel[channel].m_count & 0xff00) | data;
			}

			if (LOG) logerror("AM9517A '%s' Channel %u Base Word Count: %04x\n", tag().c_str(), channel, m_channel[channel].m_base_count);
			break;
		}

		m_msb = !m_msb;
	}
	else
	{
		switch (offset & 0x0f)
		{
		case REGISTER_COMMAND:
			m_command = data;

			if (LOG) logerror("AM9517A '%s' Command Register: %02x\n", tag().c_str(), m_command);
			break;

		case REGISTER_REQUEST:
			{
				int channel = data & 0x03;

				if (BIT(data, 2))
				{
					m_request |= (1 << (channel + 4));
					if (COMMAND_MEM_TO_MEM)
					{
						m_request |= (1 << channel);
					}
				}
				else
				{
					m_request &= ~(1 << (channel + 4));
				}

				if (LOG) logerror("AM9517A '%s' Request Register: %01x\n", tag().c_str(), m_request);
			}
			break;

		case REGISTER_SINGLE_MASK:
			{
				int channel = data & 0x03;

				if (BIT(data, 2))
				{
					m_mask |= (1 << channel);
				}
				else
				{
					m_mask &= ~(1 << channel);
				}

				if (LOG) logerror("AM9517A '%s' Mask Register: %01x\n", tag().c_str(), m_mask);
			}
			break;

		case REGISTER_MODE:
			{
				int channel = data & 0x03;

				m_channel[channel].m_mode = data & 0xfc;

				// clear terminal count
				m_status &= ~(1 << channel);

				if (LOG) logerror("AM9517A '%s' Channel %u Mode: %02x\n", tag().c_str(), channel, data & 0xfc);
			}
			break;

		case REGISTER_BYTE_POINTER:
			if (LOG) logerror("AM9517A '%s' Clear Byte Pointer Flip-Flop\n", tag().c_str());

			m_msb = 0;
			break;

		case REGISTER_MASTER_CLEAR:
			if (LOG) logerror("AM9517A '%s' Master Clear\n", tag().c_str());

			device_reset();
			break;

		case REGISTER_CLEAR_MASK:
			if (LOG) logerror("AM9517A '%s' Clear Mask Register\n", tag().c_str());

			m_mask = 0;
			break;

		case REGISTER_MASK:
			m_mask = data & 0x0f;

			if (LOG) logerror("AM9517A '%s' Mask Register: %01x\n", tag().c_str(), m_mask);
			break;
		}
	}
	trigger(1);
}


//-------------------------------------------------
//  hack_w - hold acknowledge
//-------------------------------------------------

WRITE_LINE_MEMBER( am9517a_device::hack_w )
{
	if (LOG) logerror("AM9517A '%s' Hold Acknowledge: %u\n", tag().c_str(), state);

	m_hack = state;
	trigger(1);
}


//-------------------------------------------------
//  ready_w - ready
//-------------------------------------------------

WRITE_LINE_MEMBER( am9517a_device::ready_w )
{
	if (LOG) logerror("AM9517A '%s' Ready: %u\n", tag().c_str(), state);

	m_ready = state;
}


//-------------------------------------------------
//  eop_w - end of process
//-------------------------------------------------

WRITE_LINE_MEMBER( am9517a_device::eop_w )
{
	if (LOG) logerror("AM9517A '%s' End of Process: %u\n", tag().c_str(), state);
}


//-------------------------------------------------
//  dreq0_w - DMA request for channel 0
//-------------------------------------------------

WRITE_LINE_MEMBER( am9517a_device::dreq0_w )
{
	dma_request(0, state);
}


//-------------------------------------------------
//  dreq0_w - DMA request for channel 1
//-------------------------------------------------

WRITE_LINE_MEMBER( am9517a_device::dreq1_w )
{
	dma_request(1, state);
}


//-------------------------------------------------
//  dreq1_w - DMA request for channel 2
//-------------------------------------------------

WRITE_LINE_MEMBER( am9517a_device::dreq2_w )
{
	dma_request(2, state);
}


//-------------------------------------------------
//  dreq3_w - DMA request for channel 3
//-------------------------------------------------

WRITE_LINE_MEMBER( am9517a_device::dreq3_w )
{
	dma_request(3, state);
}

//-------------------------------------------------
//  upd71071 register layouts
//-------------------------------------------------

void upd71071_v53_device::device_start()
{
	am9517a_device::device_start();
	m_address_mask = 0x00ffffff;

	m_selected_channel = 0;
	m_base = 0;

	save_item(NAME(m_selected_channel));
	save_item(NAME(m_base));
}

void upd71071_v53_device::device_reset()
{
	am9517a_device::device_reset();

	m_selected_channel = 0;
	m_base = 0;
}


READ8_MEMBER(upd71071_v53_device::read)
{
	UINT8 ret = 0;
	int channel = m_selected_channel;

	if (LOG) logerror("DMA: read from register %02x\n",offset);

	switch (offset)
	{
		case 0x01:  // Channel
			ret = (1 << m_selected_channel);
			if (m_base != 0)
				ret |= 0x10;
			break;
		case 0x02:  // Count (low)
			if (m_base != 0)
				ret = m_channel[channel].m_base_count & 0xff;
			else
				ret = m_channel[channel].m_count & 0xff;
			break;
		case 0x03:  // Count (high)
			if (m_base != 0)
				ret = (m_channel[channel].m_base_count >> 8) & 0xff;
			else
				ret = (m_channel[channel].m_count >> 8) & 0xff;
			break;
		case 0x04:  // Address (low)
			if (m_base != 0)
				ret = m_channel[channel].m_base_address & 0xff;
			else
				ret = m_channel[channel].m_address & 0xff;
			break;
		case 0x05:  // Address (mid)
			if (m_base != 0)
				ret = (m_channel[channel].m_base_address >> 8) & 0xff;
			else
				ret = (m_channel[channel].m_address >> 8) & 0xff;
			break;
		case 0x06:  // Address (high)
			if (m_base != 0)
				ret = (m_channel[channel].m_base_address >> 16) & 0xff;
			else
				ret = (m_channel[channel].m_address >> 16) & 0xff;
			break;
		case 0x07:  // Address (highest)
			if (m_base != 0)
				ret = (m_channel[channel].m_base_address >> 24) & 0xff;
			else
				ret = (m_channel[channel].m_address >> 24) & 0xff;
			break;
		case 0x0a:  // Mode control
				ret = (m_channel[channel].m_mode);
			break;

		case 0x08:  // Device control (low)
			ret = m_command & 0xff;
			break;
		case 0x09:  // Device control (high) // UPD71071 only?
			ret = m_command_high & 0xff;
			break;
		case 0x0b:  // Status
			ret = m_status;
			// clear TC bits
			m_status &= 0xf0;
			break;
		case 0x0c:  // Temporary (low)
			ret = m_temp & 0xff;
			break;
		case 0x0d:  // Temporary (high) // UPD71071 only? (other doesn't do 16-bit?)
			ret = (m_temp >> 8 ) & 0xff;
			break;
		case 0x0e:  // Request
			//ret = m_reg.request;
			ret = 0; // invalid?
			break;
		case 0x0f:  // Mask
			ret = m_mask;
			break;

	}

	return ret;
}

WRITE8_MEMBER(upd71071_v53_device::write)
{
	int channel = m_selected_channel;

	switch (offset)
	{
		case 0x00:  // Initialise
			// TODO: reset (bit 0)
			//m_buswidth = data & 0x02;
			//if (data & 0x01)
			//  soft_reset();
			logerror("DMA: Initialise [%02x]\n", data);
			break;
		case 0x01:  // Channel
			m_selected_channel = data & 0x03;
			m_base = data & 0x04;
			logerror("DMA: Channel selected [%02x]\n", data);
			break;
		case 0x02:  // Count (low)
			m_channel[channel].m_base_count =
				(m_channel[channel].m_base_count & 0xff00) | data;
			if (m_base == 0)
				m_channel[channel].m_count =
				(m_channel[channel].m_count & 0xff00) | data;
			logerror("DMA: Channel %i Counter set [%04x]\n", m_selected_channel, m_channel[channel].m_base_count);
			break;
		case 0x03:  // Count (high)
			m_channel[channel].m_base_count =
				(m_channel[channel].m_base_count & 0x00ff) | (data << 8);
			if (m_base == 0)
				m_channel[channel].m_count =
				(m_channel[channel].m_count & 0x00ff) | (data << 8);
			logerror("DMA: Channel %i Counter set [%04x]\n", m_selected_channel, m_channel[channel].m_base_count);
			break;
		case 0x04:  // Address (low)
			m_channel[channel].m_base_address =
				(m_channel[channel].m_base_address & 0xffffff00) | data;
			if (m_base == 0)
				m_channel[channel].m_address =
				(m_channel[channel].m_address & 0xffffff00) | data;
			logerror("DMA: Channel %i Address set [%08x]\n", m_selected_channel, m_channel[channel].m_base_address);
			break;
		case 0x05:  // Address (mid)
			m_channel[channel].m_base_address =
				(m_channel[channel].m_base_address & 0xffff00ff) | (data << 8);
			if (m_base == 0)
				m_channel[channel].m_address =
				(m_channel[channel].m_address & 0xffff00ff) | (data << 8);
			logerror("DMA: Channel %i Address set [%08x]\n", m_selected_channel, m_channel[channel].m_base_address);
			break;
		case 0x06:  // Address (high)
			m_channel[channel].m_base_address =
				(m_channel[channel].m_base_address & 0xff00ffff) | (data << 16);
			if (m_base == 0)
				m_channel[channel].m_address =
				(m_channel[channel].m_address & 0xff00ffff) | (data << 16);
			logerror("DMA: Channel %i Address set [%08x]\n", m_selected_channel, m_channel[channel].m_base_address);
			break;
		case 0x07:  // Address (highest)
			m_channel[channel].m_base_address =
				(m_channel[channel].m_base_address & 0x00ffffff) | (data << 24);
			if (m_base == 0)
				m_channel[channel].m_address =
				(m_channel[channel].m_address & 0x00ffffff) | (data << 24);
			logerror("DMA: Channel %i Address set [%08x]\n", m_selected_channel, m_channel[channel].m_base_address);
			break;
		case 0x0a:  // Mode control
			m_channel[channel].m_mode = data;
			// clear terminal count
			m_status &= ~(1 << channel);

			logerror("DMA: Channel %i Mode control set [%02x]\n",m_selected_channel,m_channel[channel].m_mode);
			break;

		case 0x08:  // Device control (low)
			m_command = data;
			logerror("DMA: Device control low set [%02x]\n",data);
			break;
		case 0x09:  // Device control (high)
			m_command_high = data;
			logerror("DMA: Device control high set [%02x]\n",data);
			break;
		case 0x0e:  // Request
			//m_reg.request = data;
			logerror("(invalid) DMA: Request set [%02x]\n",data); // no software requests on the v53 integrated version
			break;
		case 0x0f:  // Mask
			m_mask = data & 0x0f;
			logerror("DMA: Mask set [%02x]\n",data);
			break;


	}
	trigger(1);

}
