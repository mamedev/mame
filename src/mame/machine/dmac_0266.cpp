// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Sony 0266 DMA Controller gate array.
 *
 * This device is a single-channel DMA controller for the CXD1180 SCSI chip
 * (NCR5380 derivative) in Sony NEWS NWS-1[2457]x0 workstations.
 *
 * Sources:
 *  - https://github.com/NetBSD/src/blob/trunk/sys/arch/news68k/dev/dmac_0266.h
 *  - https://github.com/NetBSD/src/blob/trunk/sys/arch/news68k/dev/si.c
 *
 * TODO:
 *  - tczero vs interrupt status
 *  - verify if eop output exists
 *  - verify map count/width
 */

#include "emu.h"
#include "dmac_0266.h"

#define LOG_GENERAL (1U << 0)
#define LOG_DATA    (1U << 1)

//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(DMAC_0266, dmac_0266_device, "dmac_0266", "Sony 0266 DMA Controller")

dmac_0266_device::dmac_0266_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, DMAC_0266, tag, owner, clock)
	, m_bus(*this, finder_base::DUMMY_TAG, -1, 32)
	, m_int(*this)
	, m_dma_r(*this)
	, m_dma_w(*this)
{
}

void dmac_0266_device::map(address_map &map)
{
	map(0x00, 0x03).rw(FUNC(dmac_0266_device::control_r), FUNC(dmac_0266_device::control_w));
	map(0x04, 0x07).r(FUNC(dmac_0266_device::status_r));
	map(0x08, 0x0b).rw(FUNC(dmac_0266_device::tcount_r), FUNC(dmac_0266_device::tcount_w));
	map(0x0c, 0x0f).w(FUNC(dmac_0266_device::tag_w));
	map(0x10, 0x13).w(FUNC(dmac_0266_device::offset_w));
	map(0x14, 0x17).w(FUNC(dmac_0266_device::entry_w));
}

void dmac_0266_device::device_start()
{
	m_int.resolve_safe();

	m_dma_r.resolve_safe(0);
	m_dma_w.resolve_safe();

	save_item(NAME(m_control));
	save_item(NAME(m_status));
	save_item(NAME(m_tcount));
	save_item(NAME(m_tag));
	save_item(NAME(m_offset));
	save_item(NAME(m_map));

	save_item(NAME(m_int_state));
	save_item(NAME(m_drq_state));

	m_dma_check = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(dmac_0266_device::dma_check), this));

	m_int_state = false;
	m_drq_state = false;
}

void dmac_0266_device::device_reset()
{
	for (u32 &entry : m_map)
		entry = 0;

	soft_reset();
}

void dmac_0266_device::soft_reset()
{
	// soft reset does not clear map entries
	m_control = 0;
	m_status &= INTERRUPT;
	m_tcount = 0;
	m_tag = 0;
	m_offset = 0;

	m_dma_check->enable(false);
}

void dmac_0266_device::irq_w(int state)
{
	if (state)
		m_status |= INTERRUPT;
	else
		m_status &= ~INTERRUPT;

	set_int(state);
}

void dmac_0266_device::drq_w(int state)
{
	m_drq_state = bool(state);

	if (m_drq_state)
		m_dma_check->adjust(attotime::zero);
}

void dmac_0266_device::set_int(bool int_state)
{
	if (int_state != m_int_state)
	{
		m_int_state = int_state;
		m_int(int_state);
	}
}

void dmac_0266_device::control_w(u32 data)
{
	LOG("control_w 0x%08x (%s)\n", data, machine().describe_context());

	if (!(data & RESET))
	{
		if ((data ^ m_control) & ENABLE)
		{
			if (data & ENABLE)
			{
				LOG("transfer started address 0x%08x count 0x%x\n",
					(m_map[m_tag & 0x7f] << 12) | (m_offset & 0xfff), m_tcount);

				m_dma_check->adjust(attotime::zero);
			}
			else
				m_dma_check->enable(false);
		}

		m_control = data;
	}
	else
		soft_reset();
}

void dmac_0266_device::dma_check(void *ptr, s32 param)
{
	// check drq active
	if (!m_drq_state)
		return;

	// check enabled
	if (!(m_control & ENABLE))
		return;

	// check transfer count
	if (!m_tcount)
		return;

	u32 const address = (m_map[m_tag & 0x7f] << 12) | (m_offset & 0xfff);

	// perform dma transfer
	if (m_control & DIRECTION)
	{
		// device to memory
		u8 const data = m_dma_r();

		LOGMASKED(LOG_DATA, "dma_r data 0x%02x address 0x%08x\n", data, address);

		m_bus->write_byte(address, data);
	}
	else
	{
		// memory to device
		u8 const data = m_bus->read_byte(address);

		LOGMASKED(LOG_DATA, "dma_w data 0x%02x address 0x%08x\n", data, address);

		m_dma_w(data);
	}

	// increment offset
	if ((m_offset & 0xfff) == 0xfff)
	{
		// advance to next page
		m_tag++;
		m_offset = 0;
	}
	else
		m_offset++;

	// decrement count
	m_tcount--;

	// set terminal count flag
	if (!m_tcount)
	{
		LOG("transfer complete\n");
		m_control &= ~ENABLE;
		m_status |= TCZERO;
	}
	else
		m_dma_check->adjust(attotime::zero);
}
