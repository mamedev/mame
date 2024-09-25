// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_MACHINE_AIC6250_H
#define MAME_MACHINE_AIC6250_H

#pragma once

#include "machine/nscsi_bus.h"

class aic6250_device : public nscsi_device, public nscsi_slot_card_interface
{
public:
	aic6250_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void map(address_map &map) ATTR_COLD;

	auto int_cb() { return m_int_cb.bind(); }
	auto breq_cb() { return m_breq_cb.bind(); }

	auto port_a_r_cb() { return m_port_a_r_cb.bind(); }
	auto port_a_w_cb() { return m_port_a_w_cb.bind(); }
	auto port_b_r_cb() { return m_port_b_r_cb.bind(); }
	auto port_b_w_cb() { return m_port_b_w_cb.bind(); }

	void back_w(int state);

	u8 read(address_space &space, offs_t offset);
	void write(offs_t offset, u8 data);

	u8 dma_r();
	u16 dma16_r();
	void dma_w(u8 data);
	void dma16_w(u16 data);

protected:
	aic6250_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// standard device_interface overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// ncsci_device overrides
	virtual void scsi_ctrl_changed() override;

	void scsi_reset();

	u8 dma_count_l_r() { return m_dma_count >> 0; }
	u8 dma_count_m_r() { return m_dma_count >> 8; }
	u8 dma_count_h_r() { return m_dma_count >> 16; }
	u8 fifo_status_r();
	u8 rev_cntrl_r() { return m_rev_cntrl; }
	u8 status_reg_0_r();
	u8 status_reg_1_r();
	u8 scsi_signal_reg_r();
	u8 scsi_id_data_r();
	u8 source_dest_id_r() { return m_source_dest_id; }
	u8 memory_data_r();
	u8 port_a_r();
	u8 port_b_r();
	u8 scsi_latch_data_r() { return m_scsi_latch_data; }

	void dma_count_l_w(u8 data) { m_dma_count &= ~0x0000ff; m_dma_count |= (data << 0); }
	void dma_count_m_w(u8 data) { m_dma_count &= ~0x00ff00; m_dma_count |= (data << 8); }
	void dma_count_h_w(u8 data) { m_dma_count &= ~0xff0000; m_dma_count |= (data << 16); }
	void int_msk_reg_0_w(u8 data);
	void offset_cntrl_w(u8 data);
	void dma_cntrl_w(u8 data);
	void int_msk_reg_1_w(u8 data);
	void control_reg_0_w(u8 data);
	void control_reg_1_w(u8 data);
	void scsi_signal_reg_w(u8 data);
	void scsi_id_data_w(u8 data);
	void memory_data_w(u8 data);
	void port_a_w(u8 data);
	void port_b_w(u8 data);
	void scsi_bsy_rst_w(u8 data) { /* TODO: target mode, reset /BSY out and enter bus free */ }

private:
	TIMER_CALLBACK_MEMBER(state_loop);
	int state_step();

	bool int_check();
	void set_int_state(bool asserted);

	bool phase_match(u8 aic6250_phase, u32 nscsi_phase)
	{
		return (bool(aic6250_phase & R09W_SCSI_CD_OUT) == bool(nscsi_phase & S_CTL))
			&& (bool(aic6250_phase & R09W_SCSI_IO_OUT) == bool(nscsi_phase & S_INP))
			&& (bool(aic6250_phase & R09W_SCSI_MSG_OUT) == bool(nscsi_phase & S_MSG));
	}

	enum int_msk_reg_0_mask : u8
	{
		R03W_EN_SELECT_INT   = 0x01,
		R03W_EN_RESEL_INT    = 0x02,
		R03W_EN_SEL_OUT_INT  = 0x04,
		R03W_EN_CMD_DONE_INT = 0x08,
		R03W_EN_ERROR_INT    = 0x10,
		R03W_EN_AUTO_ATN     = 0x20,
		R03W_ARB_SEL_START   = 0x40,
		R03W_RESERVED        = 0x80,

		R03W_INT_MASK        = 0x0f
	};
	enum offset_cntrl_mask : u8
	{
		R04W_OFFSET         = 0x0f,
		R04W_SYNC_XFER_RATE = 0x70,
		R04W_RESERVED       = 0x80
	};
	enum fifo_status_mask : u8
	{
		R05R_FIFO_COUNTER      = 0x07,
		R05R_FIFO_FULL         = 0x08,
		R05R_FIFO_EMPTY        = 0x10,
		R05R_OFFSET_COUNT_ZERO = 0x20,
		R05R_TEST_SIGNAL       = 0xc0
	};
	enum dma_cntrl_mask : u8
	{
		R05W_DMA_XFER_EN    = 0x01,
		R05W_TRANSFER_DIR   = 0x02,
		R05W_ODD_XFER_START = 0x04,
		R05W_RESERVED       = 0xf8
	};
	enum rev_cntrl_mask : u8
	{
		R06R_REVISION = 0x03,
		R06R_RESERVED = 0xfc
	};
	enum int_msk_reg_1_mask : u8
	{
		R06W_EN_ATN_ON_INT_TGT        = 0x01,
		R06W_EN_PHASE_CHANGE_INT_INIT = 0x01,
		R06W_EN_SCSI_PARITY_ERR_INT   = 0x02,
		R06W_EN_BUS_FREE_DETECT_INT   = 0x04,
		R06W_EN_PHASE_MISMATCH_INT    = 0x08,
		R06W_EN_MEM_PARITY_ERROR_INT  = 0x10,
		R06W_EN_SCSI_RST_INT          = 0x20,
		R06W_EN_SCSI_REQ_ON_INT       = 0x40,
		R06W_RESERVED                 = 0x80,

		R06W_ERROR_INT_MASK           = 0x3f
	};
	enum status_reg_0_mask : u8
	{
		R07R_DMA_BYTE_CNT_ZERO  = 0x01,
		R07R_SCSI_PHASE_CHG_ATN = 0x02,
		R07R_SCSI_REQ_ON        = 0x04,
		R07R_SCSI_PARITY_ERR    = 0x08,
		R07R_BUS_FREE_DETECT    = 0x10,
		R07R_PHASE_MISMATCH_ERR = 0x20,
		R07R_MEMORY_PARITY_ERR  = 0x40,
		R07R_SCSI_RST_OCCURRED  = 0x80,

		R07R_ERROR_MASK         = 0xfa
	};
	enum control_reg_0_mask : u8
	{
		R07W_SCSI_ID              = 0x07,
		R07W_SCSI_INTERFACE_MODE  = 0x08,
		R07W_EN_PORT_A_INP_OR_OUT = 0x10,
		R07W_TARGET_MODE          = 0x20,
		R07W_P_MEM_RW             = 0x40,
		R07W_P_MEM_CYCLE_REQ      = 0x80
	};
	enum status_reg_1_mask : u8
	{
		R08R_SELECTED       = 0x01,
		R08R_RESELECTED     = 0x02,
		R08R_SEL_OUT        = 0x04,
		R08R_CMD_DONE       = 0x08,
		R08R_ERROR          = 0x10,
		R08R_SCSI_RST_IN    = 0x20,
		R08R_RESERVED       = 0x40,
		R08R_MEM_CYCLE_CMPL = 0x80
	};
	enum control_reg_1_mask : u8
	{
		R08W_CHIP_SW_RESET        = 0x01,
		R08W_SCSI_RST_OUT         = 0x02,
		R08W_CLK_FREQ_MODE        = 0x04,
		R08W_PHASE_CHANGE_MODE    = 0x08,
		R08W_EN_PORT_B_INP_OR_OUT = 0x10,
		R08W_RESERVED             = 0x20,
		R08W_EN_16_BIT_MEM_BUS    = 0x40,
		R08W_AUTO_SCSI_PIO_REQ    = 0x80
	};
	enum scsi_signal_reg_mask : u8
	{
		R09R_SCSI_ACK_IN  = 0x01,
		R09R_SCSI_REQ_IN  = 0x02,
		R09R_SCSI_BSY_IN  = 0x04,
		R09R_SCSI_SEL_IN  = 0x08,
		R09R_SCSI_ATN_IN  = 0x10,
		R09R_SCSI_MSG_IN  = 0x20,
		R09R_SCSI_IO_IN   = 0x40,
		R09R_SCSI_CD_IN   = 0x80,

		R09W_RESERVED     = 0x01,
		R09W_SCSI_REQ_OUT = 0x02, // target mode
		R09W_SCSI_ACK_OUT = 0x02, // initiator mode
		R09W_SCSI_BSY_OUT = 0x04,
		R09W_SCSI_SEL_OUT = 0x08,
		R09W_SCSI_ATN_OUT = 0x10,
		R09W_SCSI_MSG_OUT = 0x20,
		R09W_SCSI_IO_OUT  = 0x40,
		R09W_SCSI_CD_OUT  = 0x80,

		R09R_PHASE_MASK   = 0xe0
	};

	devcb_write_line m_int_cb;
	devcb_write_line m_breq_cb;

	devcb_read8 m_port_a_r_cb;
	devcb_write8 m_port_a_w_cb;
	devcb_read8 m_port_b_r_cb;
	devcb_write8 m_port_b_w_cb;

	enum state_t
	{
		IDLE,
		ARB_BUS_FREE,
		ARB_START,
		ARB_EVALUATE,
		SEL_START,
		SEL_DELAY,
		SEL_WAIT_BSY,
		SEL_COMPLETE,

		DMA_IN,
		DMA_IN_NEXT,
		DMA_IN_REQ,
		DMA_IN_DRAIN,
		DMA_IN_DONE,
		DMA_OUT,
		DMA_OUT_NEXT,
		DMA_OUT_REQ,
		DMA_OUT_DONE,

		AUTO_PIO,
		AUTO_PIO_IN,
		AUTO_PIO_OUT,
		AUTO_PIO_DONE,
	};

	// internal state
	emu_timer *m_state_timer;
	state_t m_state;
	bool m_int_asserted;

	u32 m_scsi_ctrl_state;

	u8 m_address_reg;

	// registers
	u32 m_dma_count;
	bool m_offset_count_zero;
	u8 m_rev_cntrl;
	u8 m_status_reg_0;
	u8 m_status_reg_1;
	u8 m_scsi_signal_reg;
	u8 m_scsi_id_data;
	u8 m_source_dest_id;
	u8 m_memory_data;
	u8 m_port_a_latch;
	u8 m_port_b_latch;
	u8 m_scsi_latch_data;

	u8 m_int_msk_reg_0;
	u8 m_offset_cntrl;
	u8 m_dma_cntrl;
	u8 m_int_msk_reg_1;
	u8 m_control_reg_0;
	u8 m_control_reg_1;

	util::fifo <u8, 8> m_fifo;
};

class aic6251a_device : public aic6250_device
{
public:
	aic6251a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(AIC6250, aic6250_device)
DECLARE_DEVICE_TYPE(AIC6251A, aic6251a_device)

#endif // MAME_MACHINE_AIC6250_H
