// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    Fujifilm Microdevices MD8412B IEEE-1394 Link Layer Controller

    Customized device specifically for Namco System 23

***************************************************************************/

#ifndef MAME_NAMCO_MD8412B_S23_H
#define MAME_NAMCO_MD8412B_S23_H

#pragma once

class md8412b_s23_device : public device_t
{
public:
	md8412b_s23_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void map(address_map &map);

	auto int_callback() { return m_int_cb.bind(); }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	u32 version_r();
	u32 ctrl_r();
	void ctrl_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 node_id_r();
	void node_id_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 reset_r();
	void reset_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 async_bufsize_r();
	void async_bufsize_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 sync_bufsize_r();
	void sync_bufsize_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 packet_ctrl_r();
	void packet_ctrl_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 diag_status_r();
	u32 phy_ctrl_r();
	void phy_ctrl_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 at_retries_ctrl_r();
	void at_retries_ctrl_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 cycle_timer_r();
	void cycle_timer_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 sync_packet_len_r();
	void sync_packet_len_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 sync_config_r(offs_t offset, u32 mem_mask = ~0);
	void sync_config_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void atf_data_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 arf_data_r();
	u32 itrf_data_r();
	void itf_data_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 irf_data_r();
	u32 buf_status_ctrl_r();
	void buf_status_ctrl_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 interrupt_r();
	void interrupt_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 interrupt_mask_r();
	void interrupt_mask_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 tgo_r();
	void tgo_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 bus_time_r();
	void bus_time_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 at_retries_r();
	void at_retries_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	enum : u32 {
		CTRL_MASK                       = 0x31770093,
		CTRL_TX_EN                      = 0,
		CTRL_RX_EN                      = 1,
		CTRL_LPS_ON                     = 4,
		CTRL_PHY_IF_RST                 = 7,
		CTRL_CYCLE_TMR_EN               = 16,
		CTRL_CYCLE_MASTER               = 17,
		CTRL_CYCLE_SOURCE               = 18,
		CTRL_ISOMODE                    = 20,
		CTRL_ISOMODE_WIDTH              = 3,
		CTRL_LITTLE                     = 24,
		CTRL_DMASIZE                    = 28,
		CTRL_DMASIZE_WIDTH              = 2,

		NODE_ID_MASK                    = 0x8000ffff,
		NODE_ID_NUM                     = 0,
		NODE_ID_NUM_WIDTH               = 6,
		NODE_ID_BUS                     = 6,
		NODE_ID_BUS_WIDTH               = 10,
		NODE_ID_VALID                   = 31,

		RESET_ATF                       = 0,
		RESET_ITRF                      = 1,
		RESET_ARF                       = 2,
		RESET_IRF                       = 3,
		RESET_TX                        = 4,
		RESET_LINK                      = 5,
		RESET_DMA                       = 6,

		ASYNC_BUFSIZE_MASK              = 0x01ff01ff,
		ASYNC_BUFSIZE_TOTALSIZE         = 0,
		ASYNC_BUFSIZE_TOTALSIZE_WIDTH   = 9,
		ASYNC_BUFSIZE_RXSIZE            = 16,
		ASYNC_BUFSIZE_RXSIZE_WIDTH      = 9,

		SYNC_BUFSIZE_MASK               = 0x01ff01ff,
		SYNC_BUFSIZE_TOTALSIZE          = 0,
		SYNC_BUFSIZE_TOTALSIZE_WIDTH    = 9,
		SYNC_BUFSIZE_RXSIZE             = 16,
		SYNC_BUFSIZE_RXSIZE_WIDTH       = 9,

		PACKET_CTRL_MASK                = 0x00001776,
		PACKET_CTRL_ACC_EN              = 1,
		PACKET_CTRL_MULTI               = 2,
		PACKET_CTRL_SNOOP_EN            = 4,
		PACKET_CTRL_RX_SELF_ID          = 5,
		PACKET_CTRL_RX_PHY_PKT          = 6,
		PACKET_CTRL_BUSYCTRL            = 8,
		PACKET_CTRL_BUSYCTRL_WIDTH      = 3,
		PACKET_CTRL_WRITE_PEND          = 12,

		DIAG_STATUS_BUSY_STATE			= 2,
		DIAG_STATUS_AT_ACK				= 8,
		DIAG_STATUS_AT_ACK_MASK			= 0x00000f00,
		DIAG_STATUS_ACK_STATUS			= 12,
		DIAG_STATUS_ACK_STATUS_MASK		= 0x00003000,
		DIAG_STATUS_RETRY_TIME_MAX		= 15,
		DIAG_STATUS_AT_BUSY				= 24,
		DIAG_STATUS_IT_BUSY				= 25,

		PHY_CTRL_MASK                   = 0x00000fff,
		PHY_CTRL_REG_DATA               = 0,
		PHY_CTRL_REG_DATA_WIDTH         = 8,
		PHY_CTRL_REG_DATA_MASK          = 0x000000ff,
		PHY_CTRL_REG_ADDR               = 8,
		PHY_CTRL_REG_ADDR_WIDTH         = 4,
		PHY_CTRL_WR_REG                 = 12,
		PHY_CTRL_RD_REG                 = 13,
		PHY_CTRL_REG_RCVD               = 14,

		AT_RETRIES_CTRL_MASK            = 0x000001ff,
		AT_RETRIES_CTRL_MAX_RETRY       = 0,
		AT_RETRIES_CTRL_MAX_RETRY_WIDTH = 4,
		AT_RETRIES_CTRL_RETRY           = 4,
		AT_RETRIES_CTRL_RETRY_WIDTH     = 4,
		AT_RETRIES_CTRL_RETRY_STOP      = 8,

		CYCLE_TIMER_OFFSET              = 0,
		CYCLE_TIMER_OFFSET_WIDTH        = 12,
		CYCLE_TIMER_COUNT               = 12,
		CYCLE_TIMER_COUNT_WIDTH         = 13,
		CYCLE_TIMER_COUNT_MASK          = 0x01fff000,
		CYCLE_TIMER_SECONDS             = 25,
		CYCLE_TIMER_SECONDS_WIDTH       = 7,
		CYCLE_TIMER_SECONDS_MASK		= 0xfe000000,

		SYNC_LENGTH_MASK                = 0x0fff0000,
		SYNC_LENGTH_VAL                 = 16,
		SYNC_LENGTH_VAL_WIDTH           = 12,

		SYNC_CONFIG_MASK                = 0xff03fff3,
		SYNC_CONFIG_SYNC_EN             = 0,
		SYNC_CONFIG_ISO_RX_EN           = 1,
		SYNC_CONFIG_STOP_SYNC           = 4,
		SYNC_CONFIG_STOP_SYNC_WIDTH     = 4,
		SYNC_CONFIG_START_SYNC          = 8,
		SYNC_CONFIG_START_SYNC_WIDTH    = 4,
		SYNC_CONFIG_SYNC                = 12,
		SYNC_CONFIG_SYNC_WIDTH          = 4,
		SYNC_CONFIG_SPEED               = 16,
		SYNC_CONFIG_SPEED_WIDTH         = 2,
		SYNC_CONFIG_CHANNEL             = 24,
		SYNC_CONFIG_CHANNEL_WIDTH       = 6,
		SYNC_CONFIG_TAG                 = 30,
		SYNC_CONFIG_TAG_WIDTH           = 2,

		BUF_STAT_CTRL_MASK              = 0x01ff7077,
		BUF_STAT_CTRL_ATF_EMPTY         = 0,
		BUF_STAT_CTRL_ATF_FULL          = 1,
		BUF_STAT_CTRL_ARF_EMPTY         = 2,
		BUF_STAT_CTRL_ITRF_EMPTY        = 4,
		BUF_STAT_CTRL_ITRF_FULL         = 5,
		BUF_STAT_CTRL_IRF_EMPTY         = 6,
		BUF_STAT_CTRL_DREQ_EN           = 12,
		BUF_STAT_CTRL_SEL_DREQ          = 13,
		BUF_STAT_CTRL_SEL_DREQ_WIDTH    = 2,
		BUF_STAT_CTRL_IRF_COUNT         = 16,
		BUF_STAT_CTRL_IRF_COUNT_WIDTH   = 9,

		INTERRUPT_MASK                  = 0x01fffff8,
		INTERRUPT_CMD_RESET             = 3,
		INTERRUPT_CYCLE_LOST            = 4,
		INTERRUPT_CYCLE_DONE            = 5,
		INTERRUPT_CYCLE_START           = 6,
		INTERRUPT_CYCLE_SECONDS         = 7,
		INTERRUPT_SENT_REJECT           = 8,
		INTERRUPT_HEADER_ERR            = 9,
		INTERRUPT_TCODE_ERR             = 10,
		INTERRUPT_ACK_ERR               = 11,
		INTERRUPT_PHY_REG_RCVD          = 12,
		INTERRUPT_BUS_RESET_FIN         = 13,
		INTERRUPT_BUS_RESET             = 14,
		INTERRUPT_PHY_INT               = 15,
		INTERRUPT_ITRF_FLUSH            = 16,
		INTERRUPT_IRF_FLUSH             = 17,
		INTERRUPT_ITF_NO_TX             = 18,
		INTERRUPT_ITX_END               = 19,
		INTERRUPT_ARX_END               = 20,
		INTERRUPT_ITRF_RX_END           = 21,
		INTERRUPT_IRF_RX_END            = 22,
		INTERRUPT_ATX_END               = 23,
		INTERRUPT_ARF_FLUSH             = 24,

		TGO_MASK                        = 0x00000007,
		TGO_AT_GO                       = 0,
		TGO_IT_GO                       = 1,
		TGO_IT_START                    = 2,

		BUS_TIME_SECONDS_MASK           = 0xffffff80,
		BUS_TIME_SECONDS_LO             = 0,
		BUS_TIME_SECONDS_LO_WIDTH       = 7,
		BUS_TIME_SECONDS_HI             = 7,
		BUS_TIME_SECONDS_HI_WIDTH       = 25,

		AT_RETRIES_MASK                 = 0xffff0000,
		AT_RETRIES_CYCLE_LIM            = 0,
		AT_RETRIES_CYCLE_LIM_WIDTH      = 13,
		AT_RETRIES_SECOND_LIM           = 13,
		AT_RETRIES_SECOND_LIM_WIDTH     = 3,
		AT_RETRIES_MAX_CYCLE_LIM        = 16,
		AT_RETRIES_MAX_CYCLE_LIM_WIDTH  = 13,
		AT_RETRIES_MAX_SECOND_LIM       = 29,
		AT_RETRIES_MAX_SECOND_LIM_WIDTH = 3
	};

private:
	void update_interrupt();

	void cycle_tick(s32 param);

	void handle_async_tx();
	void handle_iso_tx();

	void push_async_rx_word(const u32 data);
	u32 pop_async_rx_word();
	void push_async_tx_word(const u32 data);
	u32 pop_async_tx_word();
	void push_iso_rx_word(const u32 data);
	u32 pop_iso_rx_word();
	void push_iso_tx_word(const u32 data);
	u32 pop_iso_tx_word();
	void push_iso_flight_word(const u32 data);
	u32 pop_iso_flight_word();

	void recv_self_id();

	devcb_write_line m_int_cb;
	bool m_int_active;

	emu_timer *m_cycle_timer;
	bool m_async_active;
	bool m_cycle_started;
	bool m_iso_going;

	struct fifo_t
	{
		void init(const u32 _limit)
		{
			std::fill_n(buf, std::size(buf), 0);
			wr_idx = 0;
			rd_idx = 0;
			count = 0;
			limit = _limit;
		}

		void push(const u32 data)
		{
			buf[wr_idx] = data;
			wr_idx = (wr_idx + 1) % limit;
			count++;
		}

		u32 pop()
		{
			if (count == 0)
				return 0;

			const u32 data = buf[rd_idx];
			rd_idx = (rd_idx + 1) % limit;
			count--;
			return data;
		}

		u32 peek()
		{
			return buf[rd_idx];
		}

		u32 buf[512];
		u32 wr_idx;
		u32 rd_idx;
		u32 count;
		u32 limit;
	};

	fifo_t m_async_rx;
	fifo_t m_async_tx;
	fifo_t m_iso_rx;
	fifo_t m_iso_tx;

	u32 m_ctrl;
	u32 m_node_id;
	u32 m_async_bufsize;
	u32 m_sync_bufsize;
	u32 m_packet_ctrl;
	u32 m_diag_status;
	u32 m_phy_ctrl;
	u8 m_phy_regs[7];
	u32 m_at_retries_ctrl;
	u32 m_cycle_timer_reg;
	u32 m_sync_packet_len;
	u32 m_sync_config[4];
	u32 m_buf_status_ctrl;
	u32 m_interrupt;
	u32 m_interrupt_mask;
	u32 m_tgo;
	u32 m_bus_time;
	u32 m_at_retries;
};

DECLARE_DEVICE_TYPE(MD8412B_S23, md8412b_s23_device)

#endif // MAME_NAMCO_MD8412B_S23_H
