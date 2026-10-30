// license:BSD-3-Clause
// copyright-holders:Myrtle Shah


#ifndef MAME_MACHINE_LH79524_TIMER_H
#define MAME_MACHINE_LH79524_TIMER_H

#pragma once

class lh79524_timer_device : public device_t
{
public:
	lh79524_timer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_timer_index(int index);
	auto irq_cb() { return m_irq_cb.bind(); }

	void write(offs_t offset, uint32_t data, uint32_t mem_mask = ~0U);
	uint32_t read(offs_t offset, uint32_t mem_mask = ~0U);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_clock_changed() override;

	TIMER_CALLBACK_MEMBER(timer_update);

private:
	void update_interrupt();

	devcb_write_line m_irq_cb;

	emu_timer *m_tick_timer;

	int m_timer_index;

	uint32_t m_control;
	uint32_t m_cap_control;
	uint32_t m_inten;
	uint32_t m_status;
	uint32_t m_cnt;
	uint32_t m_cmp0;
	uint32_t m_cmp1;

	// TODO: compare
};

DECLARE_DEVICE_TYPE(LH79524_TIMER, lh79524_timer_device)

#endif // MAME_MACHINE_LH79524_TIMER_H
