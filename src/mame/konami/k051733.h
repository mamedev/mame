// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Acho A. Tang, R. Belmont
#ifndef MAME_KONAMI_K051733_H
#define MAME_KONAMI_K051733_H

#pragma once


class k051733_device : public device_t
{
public:
	k051733_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto set_nmi_cb() { return m_nmi_cb.bind(); }

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	void nmiclock_w(int state); // called FIRQ in schematics

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// internal state
	u8 m_ram[0x20];

	u16 m_lfsr;
	u8 m_nmi_clock;
	u16 m_nmi_timer;
	emu_timer *m_nmi_clear;

	devcb_write_line m_nmi_cb;

	TIMER_CALLBACK_MEMBER(nmi_clear) { m_nmi_cb(0); }

	void clock_lfsr();
	u32 u32_sqrt(u32 op);
};

DECLARE_DEVICE_TYPE(K051733, k051733_device)

#endif // MAME_KONAMI_K051733_H
