// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen

#ifndef MAME_VIDEO_CAT6613_H
#define MAME_VIDEO_CAT6613_H

#pragma once

#include <span>

class cat6613_device : public device_t
{
public:
	cat6613_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	void set_edid(std::span<u8 const> data) { m_edid = data; }
	auto hpd_callback() { return m_hpd.bind(); }
	// Normalized interrupt assertion, independent of the configured INT# polarity.
	auto irq_callback() { return m_irq.bind(); }

	// Byte-level I2C interface. Bit 8 marks an address byte.
	u8 transmit(offs_t data);
	u8 receive();
	void stop(int state) { m_selected = false; }
	void acknowledge(int state);
	void set_connected(int state);
	void set_video_signal(int state);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	u8 reg_read(u8 address);
	void reg_write(u8 address, u8 data);
	void reset_registers();
	bool interrupt_active() const;
	bool video_stable() const;
	void update_irq();
	void ddc_command(u8 command);
	TIMER_CALLBACK_MEMBER(ddc_complete);

	devcb_read_line m_hpd;
	devcb_write_line m_irq;
	std::span<u8 const> m_edid;
	emu_timer *m_ddc_timer;
	u8 m_regs[2][256];
	u8 m_fifo[32];
	u8 m_fifo_head;
	u8 m_fifo_count;
	u8 m_pointer;
	u8 m_ddc_command;
	u8 m_ddc_header;
	u8 m_ddc_offset;
	u8 m_ddc_count;
	u8 m_ddc_segment;
	bool m_selected;
	bool m_read;
	bool m_subaddress;
	bool m_connected;
	bool m_input_video;
	bool m_irq_state;
};

DECLARE_DEVICE_TYPE(CAT6613, cat6613_device)

#endif // MAME_VIDEO_CAT6613_H
