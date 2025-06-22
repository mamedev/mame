// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Acho A. Tang, R. Belmont
#ifndef MAME_KONAMI_K051733_H
#define MAME_KONAMI_K051733_H

#pragma once


class k051733_device : public device_t
{
public:
	k051733_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto set_nmi_cb() { return m_nmi_cb.bind(); }

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	void nmiclock_w(int state); // called FIRQ in schematics

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// internal state
	uint8_t m_ram[0x20];

	uint16_t m_lfsr;
	uint8_t m_nmi_clock;
	uint16_t m_nmi_timer;
	emu_timer *m_nmi_clear;

	devcb_write_line m_nmi_cb;

	TIMER_CALLBACK_MEMBER(nmi_clear) { m_nmi_cb(0); }

	void clock_lfsr();
	uint32_t uint_sqrt(uint32_t op);
};

DECLARE_DEVICE_TYPE(K051733, k051733_device)

#endif // MAME_KONAMI_K051733_H
