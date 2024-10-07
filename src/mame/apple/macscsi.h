// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_APPLE_MACSCSI_H
#define MAME_APPLE_MACSCSI_H

#pragma once

class mac_scsi_helper_device : public device_t
{
public:
	// device type constructor
	mac_scsi_helper_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// callback configuration
	auto scsi_read_callback() { return m_scsi_read_callback.bind(); }
	auto scsi_write_callback() { return m_scsi_write_callback.bind(); }
	auto scsi_dma_read_callback() { return m_scsi_dma_read_callback.bind(); }
	auto scsi_dma_write_callback() { return m_scsi_dma_write_callback.bind(); }
	auto cpu_halt_callback() { return m_cpu_halt_callback.bind(); }
	auto timeout_error_callback() { return m_timeout_error_callback.bind(); }

	// miscellaneous configuration
	void set_timeout(attotime timeout) { m_timeout = timeout; }

	// read/write handlers
	u8 read_wrapper(bool pseudo_dma, offs_t offset);
	void write_wrapper(bool pseudo_dma, offs_t offset, u8 data);

	void drq_w(int state);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	enum class mode : u8 { NON_DMA, READ_WAIT_DRQ, READ_DMA, WRITE_DMA, BAD_DMA };

	// internal helpers
	void read_fifo_process();
	void write_fifo_process();
	TIMER_CALLBACK_MEMBER(timer_callback);
	void dma_stop();

	// callback objects
	devcb_read8 m_scsi_read_callback;
	devcb_write8 m_scsi_write_callback;
	devcb_read8 m_scsi_dma_read_callback;
	devcb_write8 m_scsi_dma_write_callback;
	devcb_write_line m_cpu_halt_callback;
	devcb_write8 m_timeout_error_callback;

	// misc. parameters
	attotime m_timeout;

	// internal state
	emu_timer *m_pseudo_dma_timer;
	mode m_mode;
	u8 m_read_fifo_bytes;
	u8 m_write_fifo_bytes;
	u32 m_read_fifo_data;
	u32 m_write_fifo_data;
};

// device type declaration
DECLARE_DEVICE_TYPE(MAC_SCSI_HELPER, mac_scsi_helper_device)

#endif // MAME_APPLE_MACSCSI_H
