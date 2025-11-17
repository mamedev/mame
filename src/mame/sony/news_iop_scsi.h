// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_SONY_NEWS_IOP_SCSI_H
#define MAME_SONY_NEWS_IOP_SCSI_H

#pragma once

class news_iop_scsi_helper_device : public device_t
{
public:
	// device type constructor
	news_iop_scsi_helper_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// callback configuration
	auto scsi_read_callback() { return m_scsi_read_callback.bind(); }
	auto scsi_write_callback() { return m_scsi_write_callback.bind(); }
	auto scsi_dma_read_callback() { return m_scsi_dma_read_callback.bind(); }
	auto scsi_dma_write_callback() { return m_scsi_dma_write_callback.bind(); }
	auto iop_halt_callback() { return m_iop_halt_callback.bind(); }
	auto bus_error_callback() { return m_bus_error_callback.bind(); }
	auto irq_out_callback() { return m_irq_out_callback.bind(); }

	// miscellaneous configuration
	void set_timeout(attotime timeout) { m_timeout = timeout; }

	// read/write handlers
	u8 read_wrapper(bool pseudo_dma, offs_t offset);
	void write_wrapper(bool pseudo_dma, offs_t offset, u8 data);

	void drq_w(int state);
	void irq_w(int state);

	// Constants for consumers of the bus error details
	static inline constexpr u8 READ_ERROR = 1;
	static inline constexpr u8 WRITE_ERROR = 0;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	enum class mode : u8
	{
		NON_DMA,
		READ_WAIT_DRQ,
		READ_DMA,
		WRITE_DMA,
		IRQ_FIFO_DRAIN,
		BAD_DMA
	};

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
	devcb_write_line m_iop_halt_callback;
	devcb_write8 m_bus_error_callback;
	devcb_write_line m_irq_out_callback;

	// misc. parameters
	attotime m_timeout;

	// internal state
	emu_timer *m_pseudo_dma_timer;
	mode m_mode;
	u8 m_read_fifo_bytes;
	u8 m_write_fifo_bytes;
	u32 m_read_fifo_data;
	u32 m_write_fifo_data;
	bool m_irq;
};

DECLARE_DEVICE_TYPE(NEWS_IOP_SCSI_HELPER, news_iop_scsi_helper_device)

#endif // MAME_SONY_NEWS_IOP_SCSI_H
