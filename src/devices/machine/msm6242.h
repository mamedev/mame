// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    MSM6242 Real Time Clock

***************************************************************************/

#pragma once

#ifndef __MSM6242DEV_H__
#define __MSM6242DEV_H__

#include "emu.h"
#include "dirtc.h"


#define MCFG_MSM6242_OUT_INT_HANDLER(_devcb) \
	devcb = &msm6242_device::set_out_int_handler(*device, DEVCB_##_devcb);


// ======================> msm6242_device

class msm6242_device :  public device_t,
								public device_rtc_interface
{
public:
	// construction/destruction
	msm6242_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);


	template<class _Object> static devcb_base &set_out_int_handler(device_t &device, _Object object) { return downcast<msm6242_device &>(device).m_out_int_handler.set_callback(object); }

	// I/O operations
	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_pre_save() override;
	virtual void device_post_load() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// rtc overrides
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;

private:
	static const int RTC_TICKS = ~0;

	static const uint8_t IRQ_64THSECOND = 0;
	static const uint8_t IRQ_SECOND = 1;
	static const uint8_t IRQ_MINUTE = 2;
	static const uint8_t IRQ_HOUR = 3;

	// state
	uint8_t                       m_reg[3];
	uint8_t                       m_irq_flag;
	uint8_t                       m_irq_type;
	uint16_t                      m_tick;

	// incidentals
	devcb_write_line m_out_int_handler;
	emu_timer *                 m_timer;
	uint64_t                      m_last_update_time; // last update time, in clock cycles

	// methods
	void rtc_timer_callback();
	uint64_t current_time();
	void irq(uint8_t irq_type);
	uint64_t bump(int rtc_register, uint64_t delta, uint64_t register_min, uint64_t register_range);
	void update_rtc_registers();
	void update_timer();
	uint8_t get_clock_nibble(int rtc_register, bool high);
	static const char *irq_type_string(uint8_t irq_type);
};


// device type definition
extern const device_type MSM6242;


#endif /* __MSM6242DEV_H__ */
