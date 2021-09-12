// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_MACHINE_VT1682_TIMER_H
#define MAME_MACHINE_VT1682_TIMER_H

#pragma once

DECLARE_DEVICE_TYPE(VT_VT1682_TIMER, vrt_vt1682_timer_device)

class vrt_vt1682_timer_device : public device_t
{
public:
	// construction/destruction
	vrt_vt1682_timer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto write_irq_callback() { return m_irq_cb.bind(); }

	// so that we can filter logging, sound timer gets used hundreds of times a frame, so logging it is unwise
	void set_sound_timer() { m_is_sound_timer = true; }

	uint8_t vt1682_timer_preload_7_0_r();
	void vt1682_timer_preload_7_0_w(uint8_t data);

	uint8_t vt1682_timer_preload_15_8_r();
	void vt1682_timer_preload_15_8_w(uint8_t data);

	uint8_t vt1682_timer_enable_r();
	void vt1682_timer_enable_w(uint8_t data);

	void vt1682_timer_irqclear_w(uint8_t data);

	void change_clock();

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	bool m_is_sound_timer;

	void update_timer(void);

	uint8_t m_timer_preload_7_0;
	uint8_t m_timer_preload_15_8;
	uint8_t m_timer_enable;

	void timer_expired();

	devcb_write_line m_irq_cb;
	persistent_timer m_timer;
};

#endif // MAME_MACHINE_VT1682_TIMER_H
