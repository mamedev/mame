// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_APPLE_MESH_H
#define MAME_APPLE_MESH_H

#pragma once

#include "machine/nscsi_bus.h"


class mesh_device : public device_t, public nscsi_device_interface
{
public:
	mesh_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto irq_handler_cb() { return m_irq_handler.bind(); }
	auto cmd_done_handler_cb() { return m_cmd_done_handler.bind(); }
	auto error_handler_cb() { return m_error_handler.bind(); }
	auto exception_handler_cb() { return m_exception_handler.bind(); }
	auto drq_handler_cb() { return m_drq_handler.bind(); }

	void set_mesh_id(u8 mesh_id) { m_mesh_id = mesh_id; }

	void map(address_map &map) ATTR_COLD;

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	u8 xfer_count_0_r();
	void xfer_count_0_w(u8 data);
	u8 xfer_count_1_r();
	void xfer_count_1_w(u8 data);
	u8 fifo_r();
	void fifo_w(u8 data);
	u8 sequence_r();
	void sequence_w(u8 data);
	u8 bus_status_0_r();
	void bus_status_0_w(u8 data);
	u8 bus_status_1_r();
	void bus_status_1_w(u8 data);
	u8 fifo_count_r();
	u8 exception_r();
	void exception_w(u8 data);
	u8 error_r();
	void error_w(u8 data);
	u8 interrupt_mask_r();
	void interrupt_mask_w(u8 data);
	u8 interrupt_r();
	void interrupt_w(u8 data);
	u8 source_id_r();
	void source_id_w(u8 data);
	u8 dest_id_r();
	void dest_id_w(u8 data);
	u8 sync_params_r();
	void sync_params_w(u8 data);
	u8 mesh_id_r();
	u8 selection_timeout_r();
	void selection_timeout_w(u8 data);

	u16 dma16_r();
	void dma16_w(u16 data);
	u8 dma8_r();
	void dma8_w(u8 data);

	u16 dma16_swap_r() { return swapendian_int16(dma16_r()); }
	void dma16_swap_w(u16 data) { dma16_w(swapendian_int16(data)); }

	virtual void scsi_ctrl_changed() override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	enum reg_t : u8
	{
		REG_XFER_COUNT_0 = 0,
		REG_XFER_COUNT_1,
		REG_FIFO,
		REG_SEQUENCE,
		REG_BUS_STATUS_0,
		REG_BUS_STATUS_1,
		REG_FIFO_COUNT,
		REG_EXCEPTION,
		REG_ERROR,
		REG_INT_MASK,
		REG_INTERRUPT,
		REG_SOURCE_ID,
		REG_DEST_ID,
		REG_SYNC_PARAMS,
		REG_MESH_ID,
		REG_SEL_TIMEOUT
	};

	enum sequence_mask : u8
	{
		SEQ_DMA         = 0x80,
		SEQ_TARGET      = 0x40,
		SEQ_ATN         = 0x20,
		SEQ_ACT_NEG     = 0x10,
		SEQ_CMD_MASK    = 0x0f
	};

	enum sequence_cmd : u8
	{
		SEQ_ARBITRATE = 1,
		SEQ_SELECT,
		SEQ_COMMAND,
		SEQ_STATUS,
		SEQ_DATA_OUT,
		SEQ_DATA_IN,
		SEQ_MSG_OUT,
		SEQ_MSG_IN,
		SEQ_BUS_FREE,
		SEQ_EN_PARITY,
		SEQ_DIS_PARITY,
		SEQ_EN_RESEL,
		SEQ_DIS_RESEL,
		SEQ_RESET,
		SEQ_FLUSH_FIFO
	};

	enum interrupt_mask : u8
	{
		INT_CMD_DONE    = 0x01,
		INT_EXCEPTION   = 0x02,
		INT_ERROR       = 0x04
	};

	enum exception_mask : u8
	{
		EXC_SEL_TO      = 0x01,
		EXC_PHASE_MM    = 0x02,
		EXC_ARB_LOST    = 0x04,
		EXC_RESELECT    = 0x08,
		EXC_SELECTED    = 0x10,
		EXC_SEL_ATN     = 0x20
	};

	enum error_mask : u8
	{
		ERR_PARITY_0    = 0x01,
		ERR_PARITY_1    = 0x02,
		ERR_PARITY_2    = 0x04,
		ERR_PARITY_3    = 0x08,
		ERR_SEQ         = 0x10,
		ERR_SCSI_RST    = 0x20,
		ERR_UNEXP_DISC  = 0x40
	};

	enum bus_status_0_mask : u8
	{
		BUS0_REQ32      = 0x80,
		BUS0_ACK32      = 0x40,
		BUS0_REQ        = 0x20,
		BUS0_ACK        = 0x10,
		BUS0_ATN        = 0x08,
		BUS0_MSG        = 0x04,
		BUS0_CD         = 0x02,
		BUS0_IO         = 0x01
	};

	enum bus_status_1_mask : u8
	{
		BUS1_RST        = 0x80,
		BUS1_BSY        = 0x40,
		BUS1_SEL        = 0x20
	};

	enum class sequence_state : u8
	{
		IDLE,
		ARB_WAIT_FREE,
		ARB_ASSERT_BUS,
		ARB_CHECK_WIN,
		SELECT_WAIT_BSY,
		BUS_FREE_WAIT,
		XFER_WAIT_REQ,
		XFER_WAIT_REQ_FALSE
	};

	TIMER_CALLBACK_MEMBER(sequence_timer);

	u8 interrupt_value() const;
	void update_lines();
	void update_drq();
	u8 bus_status_0_value() const;
	u8 bus_status_1_value() const;
	void write_bus_status_0(u8 data);
	void write_bus_status_1(u8 data);
	void reset_sequence_state();
	void start_sequence(u8 data);
	void step(bool timeout);
	void schedule(attotime delay);
	void command_done();
	void finish_exception(u8 mask);
	void finish_error(u8 mask);
	void do_information_transfer();
	bool xfer_count_done() const;
	u32 phase_for_sequence() const;
	u32 active_selection_timeout() const;
	void fifo_clear();
	void fifo_push(u8 data);
	void fifo_push_from_scsi(u8 data);
	u8 fifo_pop();
	void decrement_xfer_count(u32 count);
	bool is_data_xfer() const;
	bool transfer_to_scsi() const;
	bool transfer_from_scsi() const;
	bool data_in_fifo_pending() const;
	bool sync_data_in() const;

	u32 m_xfer_count;
	bool m_xfer_started;
	u8 m_fifo[16];
	u8 m_fifo_count;
	u8 m_sequence;
	u8 m_exception;
	u8 m_error;
	u8 m_interrupt_mask;
	bool m_cmd_done;
	u8 m_source_id;
	u8 m_dest_id;
	u8 m_sync_params;
	u8 m_mesh_id;
	u8 m_selection_timeout;

	emu_timer *m_timer;
	sequence_state m_state;
	u32 m_expected_phase;
	u8 m_stepping;

	int m_irq;
	int m_cmd_done_line;
	int m_error_line;
	int m_exception_line;
	int m_drq;

	devcb_write_line m_irq_handler;
	devcb_write_line m_cmd_done_handler;
	devcb_write_line m_error_handler;
	devcb_write_line m_exception_handler;
	devcb_write_line m_drq_handler;
};

DECLARE_DEVICE_TYPE(APPLE_MESH_SCSI, mesh_device)

#endif // MAME_APPLE_MESH_H
