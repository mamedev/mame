// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
 * ncr5394/5396.h SCSI controller
 *
 */

#ifndef MAME_MACHINE_NCR539X_H
#define MAME_MACHINE_NCR539X_H

#pragma once

#include "legscsi.h"

// device stuff


class ncr539x_device : public legacy_scsi_host_adapter
{
public:
	// construction/destruction
	ncr539x_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto irq_callback() { return m_out_irq_cb.bind(); }
	auto drq_callback() { return m_out_drq_cb.bind(); }

	// our API
	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

	void dma_read_data(int bytes, uint8_t *pData);
	void dma_write_data(int bytes, uint8_t *pData);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	void fifo_write(uint8_t data);
	void check_fifo_executable();
	void exec_fifo();
	void update_fifo_internal_state(int bytes);

	uint32_t m_xfer_count;
	uint32_t m_dma_size;
	uint8_t m_command;
	uint8_t m_last_id;
	uint8_t m_timeout;
	uint8_t m_sync_xfer_period;
	uint8_t m_sync_offset;
	uint8_t m_control1, m_control2, m_control3, m_control4;
	uint8_t m_clock_factor;
	uint8_t m_forced_test;
	uint8_t m_data_alignment;

	bool m_selected;
	bool m_chipid_available, m_chipid_lock;

	static const int m_fifo_size = 16;
	uint8_t m_fifo_ptr, m_fifo_read_ptr, m_fifo[m_fifo_size];

	//int m_xfer_remaining;   // amount in the FIFO when we're in data in phase

	// read-only registers
	uint8_t m_status, m_irq_status, m_internal_state, m_fifo_internal_state;

	static const int m_buffer_size = 2048;

	uint8_t m_buffer[m_buffer_size];
	int m_buffer_offset, m_buffer_remaining, m_total_data;

	emu_timer *m_operation_timer;

	devcb_write_line m_out_irq_cb;          /* IRQ line */
	devcb_write_line m_out_drq_cb;          /* DRQ line */
};

// device type definition
DECLARE_DEVICE_TYPE(NCR539X, ncr539x_device)

#endif // MAME_MACHINE_NCR539X_H
