// license:BSD-3-Clause
// copyright-holders:hap
#ifndef MAME_KONAMI_K005849_H
#define MAME_KONAMI_K005849_H

#pragma once


class k005849_device : public device_t, public device_video_interface
{
public:
	k005849_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto set_irq_cb() { return m_irq_cb.bind(); }
	auto set_firq_cb() { return m_firq_cb.bind(); }
	auto set_nmi_cb() { return m_nmi_cb.bind(); }
	auto set_flipscreen_cb() { return m_flipscreen_cb.bind(); }

	bool flipscreen() { return m_flipscreen; }
	uint8_t ctrl_r(offs_t offset) { return m_ctrlram[offset & 7]; } // not from addressmap
	void ctrl_w(offs_t offset, uint8_t data);

	// scroll RAM
	uint8_t scroll_r(offs_t offset) { return m_scrollram[offset & 0x3f]; }
	void scroll_w(offs_t offset, uint8_t data) { m_scrollram[offset & 0x3f] = data; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// internal state
	uint8_t m_ctrlram[8];
	uint8_t m_scrollram[0x40];
	bool m_flipscreen;
	emu_timer *m_scanline_timer;

	devcb_write_line m_flipscreen_cb;
	devcb_write_line m_irq_cb;
	devcb_write_line m_firq_cb;
	devcb_write_line m_nmi_cb;

	TIMER_CALLBACK_MEMBER(scanline);
};

DECLARE_DEVICE_TYPE(K005849, k005849_device)

#endif // MAME_KONAMI_K005849_H
