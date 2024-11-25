// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    Fujifilm Microdevices MD8412B IEEE-1394 Link Layer Controller

    Skeleton device

***************************************************************************/

#include "emu.h"
#include "md8412b.h"

#define VERBOSE (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(MD8412B, md8412b_device, "md8412b", "Fujifilm MD8412B IEEE-1394 LLC")

md8412b_device::md8412b_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MD8412B, tag, owner, clock)
	, m_ctrl(0)
	, m_node_id(0)
	, m_async_bufsize(0)
	, m_sync_bufsize(0)
	, m_packet_ctrl(0)
	, m_phy_ctrl(0)
	, m_at_retries_ctrl(0)
	, m_cycle_timer(0)
	, m_sync_packet_len(0)
	, m_buf_status_ctrl(0)
	, m_interrupt(0)
	, m_interrupt_mask(0)
	, m_tgo(0)
	, m_bus_time(0)
	, m_at_retries(0)
{
	memset(m_phy_regs, 0, sizeof(m_phy_regs));
	memset(m_sync_config, 0, sizeof(m_sync_config));
}

void md8412b_device::device_start()
{
	save_item(NAME(m_ctrl));
	save_item(NAME(m_node_id));
	save_item(NAME(m_async_bufsize));
	save_item(NAME(m_sync_bufsize));
	save_item(NAME(m_packet_ctrl));
	save_item(NAME(m_phy_ctrl));
	save_item(NAME(m_phy_regs));
	save_item(NAME(m_at_retries_ctrl));
	save_item(NAME(m_cycle_timer));
	save_item(NAME(m_sync_packet_len));
	save_item(NAME(m_sync_config));
	save_item(NAME(m_buf_status_ctrl));
	save_item(NAME(m_interrupt));
	save_item(NAME(m_interrupt_mask));
	save_item(NAME(m_tgo));
	save_item(NAME(m_bus_time));
	save_item(NAME(m_at_retries));
}

void md8412b_device::device_reset()
{
	m_ctrl = 0x00030001;
	m_node_id = 0x0000ffff;
	m_async_bufsize = 0x007f00ff;
	m_sync_bufsize = 0x007f00ff;
	m_packet_ctrl = 0x00001020;
	m_phy_ctrl = 0;
	m_phy_regs[0] = 0x00;
	m_phy_regs[1] = 0x3f;
	m_phy_regs[2] = 0x43;
	m_phy_regs[3] = 0x00;
	m_phy_regs[4] = 0x00;
	m_phy_regs[5] = 0x00;
	m_phy_regs[6] = 0x00;
	m_at_retries_ctrl = 0x00000001;
	m_cycle_timer = 0x00000000;
	m_sync_packet_len = 0x00040000;
	m_sync_config[0] = 0x00000000;
	m_sync_config[1] = 0x00000000;
	m_sync_config[2] = 0x00000000;
	m_sync_config[3] = 0x00000000;
	m_buf_status_ctrl = 0x00000055;
	m_interrupt = 0x00000000;
	m_interrupt_mask = 0x00000000;
	m_tgo = 0x00000000;
	m_bus_time = 0x00000000;
	m_at_retries = 0x00c80000;
}

void md8412b_device::map(address_map &map)
{
	map(0x00, 0x03).r(FUNC(md8412b_device::version_r));
	map(0x04, 0x07).rw(FUNC(md8412b_device::ctrl_r), FUNC(md8412b_device::ctrl_w));
	map(0x08, 0x0b).rw(FUNC(md8412b_device::node_id_r), FUNC(md8412b_device::node_id_w));
	map(0x0c, 0x0f).rw(FUNC(md8412b_device::reset_r), FUNC(md8412b_device::reset_w));
	map(0x10, 0x13).rw(FUNC(md8412b_device::async_bufsize_r), FUNC(md8412b_device::async_bufsize_w));
	map(0x14, 0x17).rw(FUNC(md8412b_device::sync_bufsize_r), FUNC(md8412b_device::sync_bufsize_w));
	map(0x18, 0x1b).rw(FUNC(md8412b_device::packet_ctrl_r), FUNC(md8412b_device::packet_ctrl_w));
	map(0x1c, 0x1f).r(FUNC(md8412b_device::diag_status_r));
	map(0x20, 0x23).rw(FUNC(md8412b_device::phy_ctrl_r), FUNC(md8412b_device::phy_ctrl_w));
	map(0x24, 0x27).rw(FUNC(md8412b_device::at_retries_ctrl_r), FUNC(md8412b_device::at_retries_ctrl_w));
	map(0x28, 0x2b).rw(FUNC(md8412b_device::cycle_timer_r), FUNC(md8412b_device::cycle_timer_w));
	map(0x2c, 0x2f).rw(FUNC(md8412b_device::sync_packet_len_r), FUNC(md8412b_device::sync_packet_len_w));
	map(0x30, 0x3f).rw(FUNC(md8412b_device::sync_config_r), FUNC(md8412b_device::sync_config_w));
	map(0x40, 0x43).w(FUNC(md8412b_device::atf_data_w));
	map(0x44, 0x47).r(FUNC(md8412b_device::arf_data_r));
	map(0x48, 0x4b).rw(FUNC(md8412b_device::sync_data_r), FUNC(md8412b_device::sync_data_w));
	map(0x4c, 0x4f).r(FUNC(md8412b_device::irf_data_r));
	map(0x50, 0x53).rw(FUNC(md8412b_device::buf_status_ctrl_r), FUNC(md8412b_device::buf_status_ctrl_w));
	map(0x54, 0x57).rw(FUNC(md8412b_device::interrupt_r), FUNC(md8412b_device::interrupt_w));
	map(0x58, 0x5b).rw(FUNC(md8412b_device::interrupt_mask_r), FUNC(md8412b_device::interrupt_mask_w));
	map(0x5c, 0x5f).rw(FUNC(md8412b_device::tgo_r), FUNC(md8412b_device::tgo_w));
	map(0x60, 0x63).rw(FUNC(md8412b_device::bus_time_r), FUNC(md8412b_device::bus_time_w));
	map(0x68, 0x6b).rw(FUNC(md8412b_device::at_retries_r), FUNC(md8412b_device::at_retries_w));
}

u32 md8412b_device::version_r()
{
	const u32 data = 0x00010002;
	LOG("%s: version_r: %08x\n", machine().describe_context(), data);
	return data;
}

u32 md8412b_device::ctrl_r()
{
	const u32 data = m_ctrl;
	LOG("%s: ctrl_r: %08x\n", machine().describe_context(), data);
	return data;
}

void md8412b_device::ctrl_w(offs_t offset, u32 data, u32 mem_mask)
{
	static const char *const ISO_NAMES[8] =
	{
		"000 (Tx Normal, Rx Normal)",
		"001 (Tx Normal, Rx Auto)",
		"010 (Tx Auto, Rx Normal)",
		"011 (Tx -, Rx Auto)",
		"100 (Tx Auto, Rx Auto)",
		"101 (Tx -, Rx Normal)",
		"110 (Reserved)",
		"111 (Reserved)"
	};
	static const char *const DMA_NAMES[4] =
	{
		"00 (8-bit)",
		"01 (16-bit)",
		"10 (32-bit)",
		"11 (Reserved)"
	};
	LOG("%s: ctrl_w: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOG("%s:     Transmitter Enable:   %d\n", machine().describe_context(), BIT(data, CTRL_TX_EN));
	LOG("%s:     Receiver Enable:      %d\n", machine().describe_context(), BIT(data, CTRL_RX_EN));
	LOG("%s:     Link Power Status On: %d\n", machine().describe_context(), BIT(data, CTRL_LPS_ON));
	LOG("%s:     PHY I/F Reset Timing: %s\n", machine().describe_context(), BIT(data, CTRL_PHY_IF_RST) ? "Defined by P1394a" : "Default");
	LOG("%s:     Cycle Timer Enable:   %d\n", machine().describe_context(), BIT(data, CTRL_CYCLE_TMR_EN));
	LOG("%s:     Cycle Master:         %d\n", machine().describe_context(), BIT(data, CTRL_CYCLE_MASTER));
	LOG("%s:     Cycle Source:         %s\n", machine().describe_context(), BIT(data, CTRL_CYCLE_SOURCE) ? "CYCLEIN pin" : "Master Clock");
	LOG("%s:     Isochronous Mode:     %s\n", machine().describe_context(), ISO_NAMES[BIT(data, CTRL_ISOMODE, CTRL_ISOMODE_WIDTH)]);
	LOG("%s:     Endianness:           %s\n", machine().describe_context(), BIT(data, CTRL_LITTLE) ? "Little" : "Big");
	LOG("%s:     DMA Szie:             %s\n", machine().describe_context(), DMA_NAMES[BIT(data, CTRL_DMASIZE, CTRL_DMASIZE_WIDTH)]);

	mem_mask &= CTRL_MASK;
	COMBINE_DATA(&m_ctrl);
}

u32 md8412b_device::node_id_r()
{
	const u32 data = m_node_id;
	LOG("%s: node_id_r: %08x\n", machine().describe_context(), data);
	return data;
}

void md8412b_device::node_id_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOG("%s: node_id_w: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOG("%s:     Node Number: %02x\n", machine().describe_context(), BIT(data, NODE_ID_NUM, NODE_ID_NUM_WIDTH));
	LOG("%s:     Bus Number:  %03x\n", machine().describe_context(), BIT(data, NODE_ID_BUS, NODE_ID_BUS_WIDTH));
	LOG("%s:     ID Valid:    %d\n", machine().describe_context(), BIT(data, NODE_ID_VALID));

	mem_mask &= NODE_ID_MASK;
	COMBINE_DATA(&m_node_id);
}

u32 md8412b_device::reset_r()
{
	const u32 data = 0x00000000;
	LOG("%s: reset_r: %08x\n", machine().describe_context(), data);
	return data;
}

void md8412b_device::reset_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOG("%s: reset_w: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOG("%s:     Reset ATF:     %d\n", machine().describe_context(), BIT(data, RESET_ATF));
	LOG("%s:     Reset ITF/IRF: %d\n", machine().describe_context(), BIT(data, RESET_ITRF));
	LOG("%s:     Reset ARF:     %d\n", machine().describe_context(), BIT(data, RESET_ARF));
	LOG("%s:     Reset IRF:     %d\n", machine().describe_context(), BIT(data, RESET_IRF));
	LOG("%s:     Reset Link:    %d\n", machine().describe_context(), BIT(data, RESET_LINK));
	LOG("%s:     Reset DMA:     %d\n", machine().describe_context(), BIT(data, RESET_DMA));
}

u32 md8412b_device::async_bufsize_r()
{
	const u32 data = m_async_bufsize;
	LOG("%s: async_bufsize_r: %08x\n", machine().describe_context(), data);
	return data;
}

void md8412b_device::async_bufsize_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOG("%s: async_bufsize_w: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOG("%s:     Async Total Size:     %03x\n", machine().describe_context(), BIT(data, ASYNC_BUFSIZE_TOTALSIZE, ASYNC_BUFSIZE_TOTALSIZE_WIDTH));
	LOG("%s:     Async Rx Buffer Size: %03x\n", machine().describe_context(), BIT(data, ASYNC_BUFSIZE_RXSIZE, ASYNC_BUFSIZE_RXSIZE_WIDTH));

	mem_mask &= ASYNC_BUFSIZE_MASK;
	COMBINE_DATA(&m_async_bufsize);
}

u32 md8412b_device::sync_bufsize_r()
{
	const u32 data = m_sync_bufsize;
	LOG("%s: sync_bufsize_r: %08x\n", machine().describe_context(), data);
	return data;
}

void md8412b_device::sync_bufsize_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOG("%s: sync_bufsize_w: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOG("%s:     Iso Total Size:     %03x\n", machine().describe_context(), BIT(data, SYNC_BUFSIZE_TOTALSIZE, SYNC_BUFSIZE_TOTALSIZE_WIDTH));
	LOG("%s:     Iso Rx Buffer Size: %03x\n", machine().describe_context(), BIT(data, SYNC_BUFSIZE_RXSIZE, SYNC_BUFSIZE_RXSIZE_WIDTH));
	mem_mask &= SYNC_BUFSIZE_MASK;
	COMBINE_DATA(&m_sync_bufsize);
}

u32 md8412b_device::packet_ctrl_r()
{
	const u32 data = m_packet_ctrl;
	LOG("%s: packet_ctrl_r: %08x\n", machine().describe_context(), data);
	return data;
}

void md8412b_device::packet_ctrl_w(offs_t offset, u32 data, u32 mem_mask)
{
	static const char *const MODE_NAMES[8] =
	{
		"000 (Busy-ack via dual-phase retry if no vacancy)",
		"001 (BusyA-ack if no vacancy)",
		"010 (BusyB-ack if no vacancy)",
		"011 (BusyX-ack if no vacancy)",
		"100 (Busy-ack via dual-phase retry regardless of vacancy)",
		"101 (BusyA-ack regardless of vacancy)",
		"110 (BusyB-ack regardless of vacancy)",
		"111 (BusyX-ack regardless of vacancy)"
	};
	LOG("%s: packet_ctrl_w: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOG("%s:     Ack Acceleration Enable:   %d\n", machine().describe_context(), BIT(data, PACKET_CTRL_ACC_EN));
	LOG("%s:     Multi-Speed Concat Enable: %d\n", machine().describe_context(), BIT(data, PACKET_CTRL_MULTI));
	LOG("%s:     Snoop Enable:              %d\n", machine().describe_context(), BIT(data, PACKET_CTRL_SNOOP_EN));
	LOG("%s:     Receive Self ID:           %d\n", machine().describe_context(), BIT(data, PACKET_CTRL_RX_SELF_ID));
	LOG("%s:     Receive PHY Packet:        %d\n", machine().describe_context(), BIT(data, PACKET_CTRL_RX_PHY_PKT));
	LOG("%s:     Busy Control:              %s\n", machine().describe_context(), MODE_NAMES[BIT(data, PACKET_CTRL_BUSYCTRL, PACKET_CTRL_BUSYCTRL_WIDTH)]);
	LOG("%s:     Write Request Ack-Pending: %d\n", machine().describe_context(), BIT(data, PACKET_CTRL_WRITE_PEND));

	mem_mask &= PACKET_CTRL_MASK;
	COMBINE_DATA(&m_packet_ctrl);
}

u32 md8412b_device::diag_status_r()
{
	const u32 data = 0x00000000;
	LOG("%s: diag_status_r: %08x\n", machine().describe_context(), data);
	return data;
}

u32 md8412b_device::phy_ctrl_r()
{
	const u32 data = m_phy_ctrl;
	m_phy_ctrl &= ~(1 << PHY_CTRL_REG_RCVD);
	LOG("%s: phy_ctrl_r: %08x\n", machine().describe_context(), data);
	return data;
}

void md8412b_device::phy_ctrl_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOG("%s: phy_ctrl_w: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOG("%s:     PHY Register Write: %d\n", machine().describe_context(), BIT(data, PHY_CTRL_WR_REG));
	LOG("%s:      PHY Register Read: %d\n", machine().describe_context(), BIT(data, PHY_CTRL_RD_REG));
	mem_mask &= PHY_CTRL_MASK;
	COMBINE_DATA(&m_phy_ctrl);
	if (BIT(data, PHY_CTRL_WR_REG))
	{
		switch (BIT(m_phy_ctrl, PHY_CTRL_REG_ADDR, PHY_CTRL_REG_ADDR_WIDTH))
		{
		case 1:
			m_phy_regs[1] = BIT(data, PHY_CTRL_REG_DATA, PHY_CTRL_REG_DATA_WIDTH);
			LOG("%s:     Register 1:\n", machine().describe_context());
			LOG("%s:         Gap Count: %02x\n", machine().describe_context(), m_phy_regs[1] & 0x3f);
			LOG("%s:         Bus Reset: %d\n", machine().describe_context(), BIT(m_phy_regs[1], 6));
			LOG("%s:         Root Hold: %d\n", machine().describe_context(), BIT(m_phy_regs[1], 7));
			break;
		case 6:
			m_phy_regs[6] = BIT(data, PHY_CTRL_REG_DATA, PHY_CTRL_REG_DATA_WIDTH) & ~(1 << 5);
			LOG("%s:     Register 6:\n", machine().describe_context());
			LOG("%s:         Bus Reset Initiated:          %d\n", machine().describe_context(), BIT(m_phy_regs[6], 4));
			LOG("%s:         Cable Power Status:           %d\n", machine().describe_context(), BIT(m_phy_regs[6], 5));
			LOG("%s:         Cable Power Status Interrupt: %d\n", machine().describe_context(), BIT(m_phy_regs[6], 6));
			LOG("%s:         Loop Interrupt:               %d\n", machine().describe_context(), BIT(m_phy_regs[6], 7));
			break;
		}
	}
	else if (BIT(data, PHY_CTRL_RD_REG))
	{
		const u8 addr = BIT(m_phy_ctrl, PHY_CTRL_REG_ADDR, PHY_CTRL_REG_ADDR_WIDTH);
		u8 phy_data = 0;
		if (addr < 7)
		{
			phy_data = m_phy_regs[addr];
		}
		LOG("%s:     Register %2d:        %02x\n", machine().describe_context(), phy_data);
		m_phy_ctrl &= ~PHY_CTRL_REG_DATA_MASK;
		m_phy_ctrl |= phy_data;
	}
	m_phy_ctrl |= (1 << PHY_CTRL_REG_RCVD);
	m_interrupt |= (1 << INTERRUPT_PHY_REG_RCVD);
	m_interrupt &= m_interrupt_mask;
}

u32 md8412b_device::at_retries_ctrl_r()
{
	const u32 data = m_at_retries_ctrl;
	LOG("%s: at_retries_ctrl_r: %08x\n", machine().describe_context(), data);
	return data;
}

void md8412b_device::at_retries_ctrl_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOG("%s: at_retries_ctrl_w: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOG("%s:     Max Retry Count: %d\n", machine().describe_context(), BIT(data, AT_RETRIES_CTRL_MAX_RETRY, AT_RETRIES_CTRL_MAX_RETRY_WIDTH));
	LOG("%s:     Retry Count:     %d\n", machine().describe_context(), BIT(data, AT_RETRIES_CTRL_RETRY, AT_RETRIES_CTRL_RETRY_WIDTH));
	LOG("%s:     Retry Stop:      %d\n", machine().describe_context(), BIT(data, AT_RETRIES_CTRL_RETRY_STOP));

	mem_mask &= AT_RETRIES_CTRL_MASK;
	COMBINE_DATA(&m_at_retries_ctrl);
}

u32 md8412b_device::cycle_timer_r()
{
	const u32 data = machine().rand();//m_cycle_timer;
	LOG("%s: cycle_timer_r: %08x\n", machine().describe_context(), data);
	return data;
}

void md8412b_device::cycle_timer_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOG("%s: cycle_timer_w: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOG("%s:     Cycle Offset:  %03x\n", machine().describe_context(), BIT(data, CYCLE_TIMER_OFFSET, CYCLE_TIMER_OFFSET_WIDTH));
	LOG("%s:     Cycle Count:   %03x\n", machine().describe_context(), BIT(data, CYCLE_TIMER_COUNT, CYCLE_TIMER_COUNT_WIDTH));
	LOG("%s:     Cycle Seconds: %02x\n", machine().describe_context(), BIT(data, CYCLE_TIMER_SECONDS, CYCLE_TIMER_SECONDS_WIDTH));

	COMBINE_DATA(&m_cycle_timer);
}

u32 md8412b_device::sync_packet_len_r()
{
	const u32 data = m_sync_packet_len;
	LOG("%s: sync_packet_len_r: %08x\n", machine().describe_context(), data);
	return data;
}

void md8412b_device::sync_packet_len_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOG("%s: sync_packet_len_w: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOG("%s:     Packet Length: %03x\n", machine().describe_context(), BIT(data, SYNC_LENGTH_VAL, SYNC_LENGTH_VAL_WIDTH));

	mem_mask &= SYNC_LENGTH_MASK;
	COMBINE_DATA(&m_sync_packet_len);
}

u32 md8412b_device::sync_config_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_sync_config[offset];
	LOG("%s: sync_config_r: %08x\n", machine().describe_context(), data);
	return data;
}

void md8412b_device::sync_config_w(offs_t offset, u32 data, u32 mem_mask)
{
	static const char *const SPEED_NAMES[4] =
	{
		"00 (100Mbps)",
		"01 (200Mbps)",
		"10 (400Mbps)",
		"11 (Reserved)"
	};

	LOG("%s: sync_config_w: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOG("%s:     Sync Enable:           %d\n", machine().describe_context(), BIT(data, SYNC_CONFIG_SYNC_EN));
	LOG("%s:     Isochronous Rx Enable: %d\n", machine().describe_context(), BIT(data, SYNC_CONFIG_ISO_RX_EN));
	LOG("%s:     Stop Sync:             %02x\n", machine().describe_context(), BIT(data, SYNC_CONFIG_STOP_SYNC, SYNC_CONFIG_STOP_SYNC_WIDTH));
	LOG("%s:     Start Sync:            %02x\n", machine().describe_context(), BIT(data, SYNC_CONFIG_START_SYNC, SYNC_CONFIG_START_SYNC_WIDTH));
	LOG("%s:     Sync:                  %02x\n", machine().describe_context(), BIT(data, SYNC_CONFIG_SYNC, SYNC_CONFIG_SYNC_WIDTH));
	LOG("%s:     Speed:                 %02x\n", machine().describe_context(), SPEED_NAMES[BIT(data, SYNC_CONFIG_SPEED, SYNC_CONFIG_SPEED_WIDTH)]);
	LOG("%s:     Channel:               %02x\n", machine().describe_context(), BIT(data, SYNC_CONFIG_CHANNEL, SYNC_CONFIG_CHANNEL_WIDTH));
	LOG("%s:     Tag:                   %d\n", machine().describe_context(), BIT(data, SYNC_CONFIG_TAG, SYNC_CONFIG_TAG_WIDTH));

	mem_mask &= SYNC_CONFIG_MASK;
	COMBINE_DATA(&m_sync_config[offset]);
}

void md8412b_device::atf_data_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOG("%s: atf_data_w: %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 md8412b_device::arf_data_r()
{
	const u32 data = 0x00000000;
	LOG("%s: arf_data_r: %08x\n", machine().describe_context(), data);
	return data;
}

u32 md8412b_device::sync_data_r()
{
	const u32 data = 0x00000000;
	LOG("%s: sync_data_r: %08x\n", machine().describe_context(), data);
	return data;
}

void md8412b_device::sync_data_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOG("%s: sync_data_w: %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 md8412b_device::irf_data_r()
{
	const u32 data = 0x00000000;
	LOG("%s: irf_data_r: %08x\n", machine().describe_context(), data);
	return data;
}

u32 md8412b_device::buf_status_ctrl_r()
{
	const u32 data = m_buf_status_ctrl;
	LOG("%s: buf_status_ctrl_r: %08x\n", machine().describe_context(), data);
	return data;
}

void md8412b_device::buf_status_ctrl_w(offs_t offset, u32 data, u32 mem_mask)
{
	static const char *const DREQ_NAMES[4] =
	{
		"00 (DREQ indicates ATF Full)",
		"01 (DREQ indicates ATF Empty)", // Possibly a typo in the datasheet? Doesn't reflect the choice of IRF Empty for '11'
		"10 (DREQ indicates ITF/IRF Full)",
		"11 (DREQ indicates IRF Empty)"
	};

	LOG("%s: buf_status_ctrl_w: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOG("%s:     ATF Empty:     %d\n", machine().describe_context(), BIT(data, BUF_STAT_CTRL_ATF_EMPTY));
	LOG("%s:     ATF Full:      %d\n", machine().describe_context(), BIT(data, BUF_STAT_CTRL_ATF_FULL));
	LOG("%s:     ARF Empty:     %d\n", machine().describe_context(), BIT(data, BUF_STAT_CTRL_ARF_EMPTY));
	LOG("%s:     ITF/IRF Empty: %d\n", machine().describe_context(), BIT(data, BUF_STAT_CTRL_ITRF_EMPTY));
	LOG("%s:     ITF/IRF Full:  %d\n", machine().describe_context(), BIT(data, BUF_STAT_CTRL_ITRF_FULL));
	LOG("%s:     IRF Empty:     %d\n", machine().describe_context(), BIT(data, BUF_STAT_CTRL_IRF_EMPTY));
	LOG("%s:     DREQ Enable:   %d\n", machine().describe_context(), BIT(data, BUF_STAT_CTRL_DREQ_EN));
	LOG("%s:     DREQ Select:   %s\n", machine().describe_context(), DREQ_NAMES[BIT(data, BUF_STAT_CTRL_SEL_DREQ, BUF_STAT_CTRL_SEL_DREQ_WIDTH)]);
	LOG("%s:     IRF Count:     %03x\n", machine().describe_context(), BIT(data, BUF_STAT_CTRL_IRF_COUNT, BUF_STAT_CTRL_IRF_COUNT_WIDTH));

	mem_mask &= BUF_STAT_CTRL_MASK;
	COMBINE_DATA(&m_buf_status_ctrl);
}

u32 md8412b_device::interrupt_r()
{
	const u32 data = m_interrupt;
	LOG("%s: interrupt_r: %08x\n", machine().describe_context(), data);
	LOG("%s:     Command Reset:        %d\n", machine().describe_context(), BIT(data, INTERRUPT_CMD_RESET));
	LOG("%s:     Cycle Lost:           %d\n", machine().describe_context(), BIT(data, INTERRUPT_CYCLE_LOST));
	LOG("%s:     Cycle Done:           %d\n", machine().describe_context(), BIT(data, INTERRUPT_CYCLE_DONE));
	LOG("%s:     Cycle Start:          %d\n", machine().describe_context(), BIT(data, INTERRUPT_CYCLE_START));
	LOG("%s:     Cycle Seconds:        %d\n", machine().describe_context(), BIT(data, INTERRUPT_CYCLE_SECONDS));
	LOG("%s:     Sent Reject:          %d\n", machine().describe_context(), BIT(data, INTERRUPT_SENT_REJECT));
	LOG("%s:     Header Error:         %d\n", machine().describe_context(), BIT(data, INTERRUPT_HEADER_ERR));
	LOG("%s:     TCode Error:          %d\n", machine().describe_context(), BIT(data, INTERRUPT_TCODE_ERR));
	LOG("%s:     Ack Error:            %d\n", machine().describe_context(), BIT(data, INTERRUPT_ACK_ERR));
	LOG("%s:     PHY Reg Received:     %d\n", machine().describe_context(), BIT(data, INTERRUPT_PHY_REG_RCVD));
	LOG("%s:     Bus Reset Finish:     %d\n", machine().describe_context(), BIT(data, INTERRUPT_BUS_RESET_FIN));
	LOG("%s:     Bus Reset:            %d\n", machine().describe_context(), BIT(data, INTERRUPT_BUS_RESET));
	LOG("%s:     PHY Interrupt:        %d\n", machine().describe_context(), BIT(data, INTERRUPT_PHY_INT));
	LOG("%s:     ITF/IRF Flush:        %d\n", machine().describe_context(), BIT(data, INTERRUPT_ITRF_FLUSH));
	LOG("%s:     IRF Flush:            %d\n", machine().describe_context(), BIT(data, INTERRUPT_IRF_FLUSH));
	LOG("%s:     Iso No-Tx:            %d\n", machine().describe_context(), BIT(data, INTERRUPT_ITF_NO_TX));
	LOG("%s:     Iso Tx End:           %d\n", machine().describe_context(), BIT(data, INTERRUPT_ITX_END));
	LOG("%s:     Async Rx End:         %d\n", machine().describe_context(), BIT(data, INTERRUPT_ARX_END));
	LOG("%s:     Iso Rx End (ITF/IRF): %d\n", machine().describe_context(), BIT(data, INTERRUPT_ITRF_RX_END));
	LOG("%s:     Iso Rx End (IRF):     %d\n", machine().describe_context(), BIT(data, INTERRUPT_IRF_RX_END));
	LOG("%s:     Async Tx End:         %d\n", machine().describe_context(), BIT(data, INTERRUPT_ATX_END));
	LOG("%s:     ARF Flush:            %d\n", machine().describe_context(), BIT(data, INTERRUPT_ARF_FLUSH));
	return data;
}

void md8412b_device::interrupt_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOG("%s: interrupt_w: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	mem_mask &= ~data;
	mem_mask &= INTERRUPT_MASK;
	COMBINE_DATA(&m_interrupt);
}

u32 md8412b_device::interrupt_mask_r()
{
	const u32 data = m_interrupt_mask;
	LOG("%s: interrupt_mask_r: %08x\n", machine().describe_context(), data);
	return data;
}

void md8412b_device::interrupt_mask_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOG("%s: interrupt_mask_w: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOG("%s:     Command Reset:        %d\n", machine().describe_context(), BIT(data, INTERRUPT_CMD_RESET));
	LOG("%s:     Cycle Lost:           %d\n", machine().describe_context(), BIT(data, INTERRUPT_CYCLE_LOST));
	LOG("%s:     Cycle Done:           %d\n", machine().describe_context(), BIT(data, INTERRUPT_CYCLE_DONE));
	LOG("%s:     Cycle Start:          %d\n", machine().describe_context(), BIT(data, INTERRUPT_CYCLE_START));
	LOG("%s:     Cycle Seconds:        %d\n", machine().describe_context(), BIT(data, INTERRUPT_CYCLE_SECONDS));
	LOG("%s:     Sent Reject:          %d\n", machine().describe_context(), BIT(data, INTERRUPT_SENT_REJECT));
	LOG("%s:     Header Error:         %d\n", machine().describe_context(), BIT(data, INTERRUPT_HEADER_ERR));
	LOG("%s:     TCode Error:          %d\n", machine().describe_context(), BIT(data, INTERRUPT_TCODE_ERR));
	LOG("%s:     Ack Error:            %d\n", machine().describe_context(), BIT(data, INTERRUPT_ACK_ERR));
	LOG("%s:     PHY Reg Received:     %d\n", machine().describe_context(), BIT(data, INTERRUPT_PHY_REG_RCVD));
	LOG("%s:     Bus Reset Finish:     %d\n", machine().describe_context(), BIT(data, INTERRUPT_BUS_RESET_FIN));
	LOG("%s:     Bus Reset:            %d\n", machine().describe_context(), BIT(data, INTERRUPT_BUS_RESET));
	LOG("%s:     PHY Interrupt:        %d\n", machine().describe_context(), BIT(data, INTERRUPT_PHY_INT));
	LOG("%s:     ITF/IRF Flush:        %d\n", machine().describe_context(), BIT(data, INTERRUPT_ITRF_FLUSH));
	LOG("%s:     IRF Flush:            %d\n", machine().describe_context(), BIT(data, INTERRUPT_IRF_FLUSH));
	LOG("%s:     Iso No-Tx:            %d\n", machine().describe_context(), BIT(data, INTERRUPT_ITF_NO_TX));
	LOG("%s:     Iso Tx End:           %d\n", machine().describe_context(), BIT(data, INTERRUPT_ITX_END));
	LOG("%s:     Async Rx End:         %d\n", machine().describe_context(), BIT(data, INTERRUPT_ARX_END));
	LOG("%s:     Iso Rx End (ITF/IRF): %d\n", machine().describe_context(), BIT(data, INTERRUPT_ITRF_RX_END));
	LOG("%s:     Iso Rx End (IRF):     %d\n", machine().describe_context(), BIT(data, INTERRUPT_IRF_RX_END));
	LOG("%s:     Async Tx End:         %d\n", machine().describe_context(), BIT(data, INTERRUPT_ATX_END));
	LOG("%s:     ARF Flush:            %d\n", machine().describe_context(), BIT(data, INTERRUPT_ARF_FLUSH));
	mem_mask &= INTERRUPT_MASK;
	COMBINE_DATA(&m_interrupt_mask);
}

u32 md8412b_device::tgo_r()
{
	const u32 data = m_tgo;
	LOG("%s: tgo_r: %08x\n", machine().describe_context(), data);
	return data;
}

void md8412b_device::tgo_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOG("%s: tgo_w: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOG("%s:     AT Go:    %d\n", machine().describe_context(), BIT(data, TGO_AT_GO));
	LOG("%s:     IT Go:    %d\n", machine().describe_context(), BIT(data, TGO_IT_GO));
	LOG("%s:     IT Start: %d\n", machine().describe_context(), BIT(data, TGO_IT_START));
	mem_mask &= TGO_MASK;
	COMBINE_DATA(&m_tgo);
}

u32 md8412b_device::bus_time_r()
{
	const u32 data = m_bus_time;
	LOG("%s: bus_time_r: %08x\n", machine().describe_context(), data);
	LOG("%s:     Seconds Lo: %02x\n", machine().describe_context(), BIT(data, BUS_TIME_SECONDS_LO, BUS_TIME_SECONDS_LO_WIDTH));
	LOG("%s:     Seconds Hi: %07x\n", machine().describe_context(), BIT(data, BUS_TIME_SECONDS_HI, BUS_TIME_SECONDS_HI_WIDTH));
	return data;
}

void md8412b_device::bus_time_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOG("%s: bus_time_w: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOG("%s:     Seconds Lo: %02x\n", machine().describe_context(), BIT(data, BUS_TIME_SECONDS_LO, BUS_TIME_SECONDS_LO_WIDTH));
	LOG("%s:     Seconds Hi: %07x\n", machine().describe_context(), BIT(data, BUS_TIME_SECONDS_HI, BUS_TIME_SECONDS_HI_WIDTH));

	mem_mask &= BUS_TIME_SECONDS_MASK;
	COMBINE_DATA(&m_bus_time);
}

u32 md8412b_device::at_retries_r()
{
	const u32 data = m_at_retries;
	LOG("%s: at_retries_r: %08x\n", machine().describe_context(), data);
	return data;
}

void md8412b_device::at_retries_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOG("%s: at_retries_w: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOG("%s:     Retry Cycle Limit:      %03x\n", machine().describe_context(), BIT(data, AT_RETRIES_CYCLE_LIM, AT_RETRIES_CYCLE_LIM_WIDTH));
	LOG("%s:     Retry Second Limit:     %d\n", machine().describe_context(), BIT(data, AT_RETRIES_SECOND_LIM, AT_RETRIES_SECOND_LIM_WIDTH));
	LOG("%s:     Max Retry Cycle Limit:  %03x\n", machine().describe_context(), BIT(data, AT_RETRIES_MAX_CYCLE_LIM, AT_RETRIES_MAX_CYCLE_LIM_WIDTH));
	LOG("%s:     Max Retry Second Limit: %d\n", machine().describe_context(), BIT(data, AT_RETRIES_MAX_SECOND_LIM, AT_RETRIES_MAX_SECOND_LIM_WIDTH));
	mem_mask &= AT_RETRIES_MASK;
	COMBINE_DATA(&m_at_retries);
}
