// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_APPLE_MACSCSI_H
#define MAME_APPLE_MACSCSI_H

#pragma once

#include "cpu/m68000/m68kcommon.h"

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
	auto timeout_error_callback() { return m_timeout_error_callback.bind(); }

	// miscellaneous configuration
	template <typename T> void set_maincpu(T &&tag) { m_maincpu.set_tag(std::forward<T>(tag)); }

	// read/write handlers
	u8 read_wrapper(bool pseudo_dma, offs_t offset);
	void write_wrapper(bool pseudo_dma, offs_t offset, u8 data);
	u32 pseudo_dma_r(offs_t offset, u32 mem_mask = ~0);
	void pseudo_dma_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	void drq_w(int state);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// internal helpers
	u32 dma_read(u8 bytes);
	void dma_write(u32 data, u8 bytes);
	bool timed_out(u8 error);
	void wait(bool progress);
	void done();
	void wake();
	TIMER_CALLBACK_MEMBER(timeout);

	required_device<m68000_base_device> m_maincpu;

	// callback objects
	devcb_read8 m_scsi_read_callback;
	devcb_write8 m_scsi_write_callback;
	devcb_read8 m_scsi_dma_read_callback;
	devcb_write8 m_scsi_dma_write_callback;
	devcb_write8 m_timeout_error_callback;

	// internal state
	emu_timer *m_timeout_timer;
	bool m_drq;
	bool m_waiting;
	bool m_woken;
	bool m_timed_out;
	u8 m_count;
	u32 m_data;
};

// device type declaration
DECLARE_DEVICE_TYPE(MAC_SCSI_HELPER, mac_scsi_helper_device)

#endif // MAME_APPLE_MACSCSI_H
