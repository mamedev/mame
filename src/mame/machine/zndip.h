// license:BSD-3-Clause
// copyright-holders:smf
#pragma once

#ifndef __ZNDIP_H__
#define __ZNDIP_H__

#include "emu.h"

extern const device_type ZNDIP;

#define MCFG_ZNDIP_DATAOUT_HANDLER(_devcb) \
	devcb = &zndip_device::set_dataout_handler(*device, DEVCB_##_devcb);

#define MCFG_ZNDIP_DSR_HANDLER(_devcb) \
	devcb = &zndip_device::set_dsr_handler(*device, DEVCB_##_devcb);

#define MCFG_ZNDIP_DATA_HANDLER(_devcb) \
	devcb = &zndip_device::set_data_handler(*device, DEVCB_##_devcb);

class zndip_device : public device_t
{
public:
	zndip_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	template<class _Object> static devcb_base &set_data_handler(device_t &device, _Object object) { return downcast<zndip_device &>(device).m_data_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_dataout_handler(device_t &device, _Object object) { return downcast<zndip_device &>(device).m_dataout_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_dsr_handler(device_t &device, _Object object) { return downcast<zndip_device &>(device).m_dsr_handler.set_callback(object); }

	WRITE_LINE_MEMBER(write_select);
	WRITE_LINE_MEMBER(write_clock);

protected:
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	devcb_read8 m_data_handler;
	devcb_write_line m_dataout_handler;
	devcb_write_line m_dsr_handler;

	int m_select;
	int m_clock;

	UINT8 m_bit;
	emu_timer *m_dip_timer;
};

#endif
