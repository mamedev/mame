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

/*
 * The EISA_DMA device represents the 82C37A-compatible DMA devices present in
 * EISA bus systems, in particular those embedded within the i82357 Integrated
 * System Peripheral. The device supports 32 bit addressing, 32 bit data sizes,
 * and 24 bit transfer counts, allowing DMA across 64k boundaries. It also adds
 * stop registers, supporting ring-buffer memory arrangements.
 *
 * TODO
 *   - stop registers
 *   - 16/32-bit transfer sizes
 */

#include "emu.h"
#include "am9517a.h"

//#define VERBOSE 1
#include "logmacro.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(AM9517A,      am9517a_device,      "am9517a",  "AM9517A")
DEFINE_DEVICE_TYPE(V5X_DMAU,     v5x_dmau_device,     "v5x_dmau", "V5X DMAU")
DEFINE_DEVICE_TYPE(PCXPORT_DMAC, pcxport_dmac_device, "pcx_dmac", "PC Transporter DMAC")
DEFINE_DEVICE_TYPE(EISA_DMA,     eisa_dma_device,     "eisa_dma", "EISA DMA")


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************


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

void am9517a_device::dma_request(int channel, int state)
{
	LOG("AM9517A Channel %u DMA Request: %u\n", channel, state);

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
		if ((channel == m_current_channel) && !COMMAND_MEM_TO_MEM)
			m_out_dack_cb[channel](COMMAND_DACK_ACTIVE_HIGH);
		else
			m_out_dack_cb[channel](!COMMAND_DACK_ACTIVE_HIGH);
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
//  get_state1 -
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

void am9517a_device::dma_read()
{
	offs_t offset = m_channel[m_current_channel].m_address;

	switch (MODE_TRANSFER_MASK)
	{
	case MODE_TRANSFER_VERIFY:
	case MODE_TRANSFER_WRITE:
		m_temp = m_in_ior_cb[m_current_channel](offset);
		break;

	case MODE_TRANSFER_READ:
		m_temp = m_in_memr_cb(offset);
		break;
	}
}


//-------------------------------------------------
//  dma_write -
//-------------------------------------------------

void am9517a_device::dma_write()
{
	offs_t offset = m_channel[m_current_channel].m_address;

	switch (MODE_TRANSFER_MASK)
	{
	case MODE_TRANSFER_VERIFY:
		{
			uint8_t v1 = m_in_memr_cb(offset);
			if(0 && m_temp != v1)
				logerror("verify error %02x vs. %02x\n", m_temp, v1);
		}
		break;

	case MODE_TRANSFER_WRITE:
		m_out_memw_cb(offset, m_temp);
		break;

	case MODE_TRANSFER_READ:
		m_out_iow_cb[m_current_channel](offset, m_temp);
		break;
	}
}


//-------------------------------------------------
//  dma_advance -
//-------------------------------------------------

inline void am9517a_device::dma_advance()
{
	bool msb_changed = false;

	if (m_current_channel || !COMMAND_MEM_TO_MEM || !COMMAND_CH0_ADDRESS_HOLD)
	{
		if (MODE_ADDRESS_DECREMENT)
		{
			m_channel[m_current_channel].m_address -= transfer_size(m_current_channel);
			m_channel[m_current_channel].m_address &= m_address_mask;

			if ((m_channel[m_current_channel].m_address & 0xff) == 0xff)
			{
				msb_changed = true;
			}
		}
		else
		{
			m_channel[m_current_channel].m_address += transfer_size(m_current_channel);
			m_channel[m_current_channel].m_address &= m_address_mask;

			if ((m_channel[m_current_channel].m_address & 0xff) == 0x00)
			{
				msb_changed = true;
			}
		}
	}

	if (m_channel[m_current_channel].m_count-- == 0)
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

void am9517a_device::end_of_process()
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

	set_eop(CLEAR_LINE);
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


am9517a_device::am9517a_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock),
		device_execute_interface(mconfig, *this),
		m_icount(0),
		m_hack(0),
		m_ready(1),
		m_command(0),
		m_out_hreq_cb(*this),
		m_out_eop_cb(*this),
		m_in_memr_cb(*this),
		m_out_memw_cb(*this),
		m_in_ior_cb(*this),
		m_out_iow_cb(*this),
		m_out_dack_cb(*this)
{
}


am9517a_device::am9517a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: am9517a_device(mconfig, AM9517A, tag, owner, clock)
{
}

v5x_dmau_device::v5x_dmau_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: am9517a_device(mconfig, V5X_DMAU, tag, owner, clock)
	, m_in_mem16r_cb(*this)
	, m_out_mem16w_cb(*this)
	, m_in_io16r_cb(*this)
	, m_out_io16w_cb(*this)

{
}

pcxport_dmac_device::pcxport_dmac_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: am9517a_device(mconfig, PCXPORT_DMAC, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void am9517a_device::device_start()
{
	// set our instruction counter
	set_icountptr(m_icount);

	// resolve callbacks
	m_out_hreq_cb.resolve_safe();
	m_out_eop_cb.resolve_safe();
	m_in_memr_cb.resolve_safe(0);
	m_out_memw_cb.resolve_safe();
	m_in_ior_cb.resolve_all_safe(0);
	m_out_iow_cb.resolve_all_safe();
	m_out_dack_cb.resolve_all_safe();

	for(auto &elem : m_channel)
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

	save_item(STRUCT_MEMBER(m_channel, m_address));
	save_item(STRUCT_MEMBER(m_channel, m_count));
	save_item(STRUCT_MEMBER(m_channel, m_base_address));
	save_item(STRUCT_MEMBER(m_channel, m_base_count));
	save_item(STRUCT_MEMBER(m_channel, m_mode));

	m_address_mask = 0xffff;

	// force clear upon initial reset
	m_eop = ASSERT_LINE;
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

	set_hreq(0);
	set_eop(CLEAR_LINE);

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
			if (COMMAND_COMPRESSED_TIMING)
			{
				// signal end of process during last cycle
				if (m_channel[m_current_channel].m_count == 0)
					set_eop(ASSERT_LINE);

				m_state = STATE_S4;
			}
			else
				m_state = STATE_S3;
			break;

		case STATE_S3:
			// signal end of process during last cycle
			if (m_channel[m_current_channel].m_count == 0)
				set_eop(ASSERT_LINE);

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
			// signal end of process during last cycle
			if (m_channel[m_current_channel].m_count == 0)
				set_eop(ASSERT_LINE);

			m_state = STATE_S24;
			break;

		case STATE_S24:
			dma_write();
			dma_advance();

			m_current_channel = 0;
			m_channel[m_current_channel].m_count--;
			if (MODE_ADDRESS_DECREMENT)
			{
				m_channel[m_current_channel].m_address -= transfer_size(m_current_channel);
				m_channel[m_current_channel].m_address &= m_address_mask;
			}
			else
			{
				m_channel[m_current_channel].m_address += transfer_size(m_current_channel);
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

uint8_t am9517a_device::read(offs_t offset)
{
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

void am9517a_device::write(offs_t offset, uint8_t data)
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

			LOG("AM9517A Channel %u Base Address: %04x\n", channel, m_channel[channel].m_base_address);
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

			LOG("AM9517A Channel %u Base Word Count: %04x\n", channel, m_channel[channel].m_base_count);
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

			LOG("AM9517A Command Register: %02x\n", m_command);
			break;

		case REGISTER_REQUEST:
			{
				int channel = data & 0x03;

				if (BIT(data, 2))
				{
					m_request |= (1 << channel);
				}
				else
				{
					m_request &= ~(1 << channel);
				}

				LOG("AM9517A Request Register: %01x\n", m_request);
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

				LOG("AM9517A Mask Register: %01x\n", m_mask);
			}
			break;

		case REGISTER_MODE:
			{
				int channel = data & 0x03;

				m_channel[channel].m_mode = data & 0xfc;

				// clear terminal count
				m_status &= ~(1 << channel);

				LOG("AM9517A Channel %u Mode: %02x\n", channel, data & 0xfc);
			}
			break;

		case REGISTER_BYTE_POINTER:
			LOG("AM9517A Clear Byte Pointer Flip-Flop\n");

			m_msb = 0;
			break;

		case REGISTER_MASTER_CLEAR:
			LOG("AM9517A Master Clear\n");

			device_reset();
			break;

		case REGISTER_CLEAR_MASK:
			LOG("AM9517A Clear Mask Register\n");

			m_mask = 0;
			break;

		case REGISTER_MASK:
			m_mask = data & 0x0f;

			LOG("AM9517A Mask Register: %01x\n", m_mask);

			// tek4132 firmware indicates setting mask bits also sets status
			m_status |= (data & 0x0f) << 4;
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
	LOG("AM9517A Hold Acknowledge: %u\n", state);

	m_hack = state;
	trigger(1);
}


//-------------------------------------------------
//  ready_w - ready
//-------------------------------------------------

WRITE_LINE_MEMBER( am9517a_device::ready_w )
{
	LOG("AM9517A Ready: %u\n", state);

	m_ready = state;
}


//-------------------------------------------------
//  eop_w - end of process
//-------------------------------------------------

WRITE_LINE_MEMBER( am9517a_device::eop_w )
{
	LOG("AM9517A End of Process: %u\n", state);
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

void v5x_dmau_device::device_start()
{
	am9517a_device::device_start();
	m_address_mask = 0x00ffffff;

	m_in_mem16r_cb.resolve_safe(0);
	m_out_mem16w_cb.resolve_safe();
	m_in_io16r_cb.resolve_all_safe(0);
	m_out_io16w_cb.resolve_all_safe();

	m_selected_channel = 0;
	m_base = 0;

	save_item(NAME(m_selected_channel));
	save_item(NAME(m_base));
}

void v5x_dmau_device::device_reset()
{
	am9517a_device::device_reset();

	m_selected_channel = 0;
	m_base = 0;
}


uint8_t v5x_dmau_device::read(offs_t offset)
{
	uint8_t ret = 0;
	int channel = m_selected_channel;

	LOG("DMA: read from register %02x\n",offset);

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

void v5x_dmau_device::write(offs_t offset, uint8_t data)
{
	int channel = m_selected_channel;

	switch (offset)
	{
		case 0x00:  // Initialise
			// TODO: reset (bit 0)
			//m_buswidth = data & 0x02;
			//if (data & 0x01)
			//  soft_reset();
			LOG("DMA: Initialise [%02x]\n", data);
			break;
		case 0x01:  // Channel
			m_selected_channel = data & 0x03;
			m_base = data & 0x04;
			LOG("DMA: Channel selected [%02x]\n", data);
			break;
		case 0x02:  // Count (low)
			m_channel[channel].m_base_count =
				(m_channel[channel].m_base_count & 0xff00) | data;
			if (m_base == 0)
				m_channel[channel].m_count =
				(m_channel[channel].m_count & 0xff00) | data;
			LOG("DMA: Channel %i Counter set [%04x]\n", m_selected_channel, m_channel[channel].m_base_count);
			break;
		case 0x03:  // Count (high)
			m_channel[channel].m_base_count =
				(m_channel[channel].m_base_count & 0x00ff) | (data << 8);
			if (m_base == 0)
				m_channel[channel].m_count =
				(m_channel[channel].m_count & 0x00ff) | (data << 8);
			LOG("DMA: Channel %i Counter set [%04x]\n", m_selected_channel, m_channel[channel].m_base_count);
			break;
		case 0x04:  // Address (low)
			m_channel[channel].m_base_address =
				(m_channel[channel].m_base_address & 0xffffff00) | data;
			if (m_base == 0)
				m_channel[channel].m_address =
				(m_channel[channel].m_address & 0xffffff00) | data;
			LOG("DMA: Channel %i Address set [%08x]\n", m_selected_channel, m_channel[channel].m_base_address);
			break;
		case 0x05:  // Address (mid)
			m_channel[channel].m_base_address =
				(m_channel[channel].m_base_address & 0xffff00ff) | (data << 8);
			if (m_base == 0)
				m_channel[channel].m_address =
				(m_channel[channel].m_address & 0xffff00ff) | (data << 8);
			LOG("DMA: Channel %i Address set [%08x]\n", m_selected_channel, m_channel[channel].m_base_address);
			break;
		case 0x06:  // Address (high)
			m_channel[channel].m_base_address =
				(m_channel[channel].m_base_address & 0xff00ffff) | (data << 16);
			if (m_base == 0)
				m_channel[channel].m_address =
				(m_channel[channel].m_address & 0xff00ffff) | (data << 16);
			LOG("DMA: Channel %i Address set [%08x]\n", m_selected_channel, m_channel[channel].m_base_address);
			break;
		case 0x07:  // Address (highest)
			m_channel[channel].m_base_address =
				(m_channel[channel].m_base_address & 0x00ffffff) | (data << 24);
			if (m_base == 0)
				m_channel[channel].m_address =
				(m_channel[channel].m_address & 0x00ffffff) | (data << 24);
			LOG("DMA: Channel %i Address set [%08x]\n", m_selected_channel, m_channel[channel].m_base_address);
			break;
		case 0x0a:  // Mode control
			m_channel[channel].m_mode = data;
			// clear terminal count
			m_status &= ~(1 << channel);

			LOG("DMA: Channel %i Mode control set [%02x]\n",m_selected_channel,m_channel[channel].m_mode);
			break;

		case 0x08:  // Device control (low)
			m_command = data;
			LOG("DMA: Device control low set [%02x]\n",data);
			break;
		case 0x09:  // Device control (high)
			m_command_high = data;
			LOG("DMA: Device control high set [%02x]\n",data);
			break;
		case 0x0e:  // Request
			//m_reg.request = data;
			LOG("(invalid) DMA: Request set [%02x]\n",data); // no software requests on the v53 integrated version
			break;
		case 0x0f:  // Mask
			m_mask = data & 0x0f;
			LOG("DMA: Mask set [%02x]\n",data);
			break;


	}
	trigger(1);

}

void v5x_dmau_device::dma_read()
{
	if (m_channel[m_current_channel].m_mode & 0x1)
	{
		offs_t const offset = m_channel[m_current_channel].m_address >> 1;

		switch (MODE_TRANSFER_MASK)
		{
		case MODE_TRANSFER_VERIFY:
		case MODE_TRANSFER_WRITE:
			m_temp = m_in_io16r_cb[m_current_channel](offset);
			break;

		case MODE_TRANSFER_READ:
			m_temp = m_in_mem16r_cb(offset);
			break;
		}
	}
	else
		am9517a_device::dma_read();
}

void v5x_dmau_device::dma_write()
{
	if (m_channel[m_current_channel].m_mode & 0x1)
	{
		offs_t const offset = m_channel[m_current_channel].m_address >> 1;

		switch (MODE_TRANSFER_MASK)
		{
		case MODE_TRANSFER_VERIFY:
		{
			u16 const v1 = m_in_mem16r_cb(offset);
			if (0 && m_temp != v1)
				logerror("verify error %04x vs. %04x\n", m_temp, v1);
		}
		break;

		case MODE_TRANSFER_WRITE:
			m_out_mem16w_cb(offset, m_temp);
			break;

		case MODE_TRANSFER_READ:
			m_out_io16w_cb[m_current_channel](offset, m_temp);
			break;
		}
	}
	else
		am9517a_device::dma_write();
}

void pcxport_dmac_device::device_reset()
{
	m_state = STATE_SI;
	m_command = 0;
	m_status = 0;
	m_request = 0;
	m_mask = 0;
	m_temp = 0;
	m_msb = 0;
	m_current_channel = -1;
	m_last_channel = 3;
	m_hreq = -1;

	set_hreq(0);
	set_eop(CLEAR_LINE);

	set_dack();
}

void pcxport_dmac_device::end_of_process()
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
	// don't mask out channel if not autoinitialize

	set_eop(CLEAR_LINE);
	set_hreq(0);

	m_current_channel = -1;
	set_dack();

	m_state = STATE_SI;
}

eisa_dma_device::eisa_dma_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: am9517a_device(mconfig, EISA_DMA, tag, owner, clock)
{
}

void eisa_dma_device::device_start()
{
	am9517a_device::device_start();

	m_address_mask = 0xffffffffU;

	save_item(NAME(m_stop));
	save_item(NAME(m_ext_mode));
}
