// license:BSD-3-Clause
// copyright-holders:Sandro Ronco


#ifndef MAME_MACHINE_SPG290_TIMER_H
#define MAME_MACHINE_SPG290_TIMER_H

#pragma once

class spg290_timer_device : public device_t
{
public:
	spg290_timer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void write(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t read(offs_t offset, uint32_t mem_mask);
	void control_w(uint32_t data);
	auto irq_cb() { return m_irq_cb.bind(); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_clock_changed() override;

	TIMER_CALLBACK_MEMBER(timer_update);

private:
	devcb_write_line m_irq_cb;

	emu_timer *m_tick_timer;

	bool     m_enabled;
	uint16_t m_counter;
	uint32_t m_control;
	uint32_t m_control2;
	uint32_t m_preload;
	uint32_t m_ccp;
	uint32_t m_upcount;
};

DECLARE_DEVICE_TYPE(SPG290_TIMER, spg290_timer_device)

#endif // MAME_MACHINE_SPG290_TIMER_H
