#pragma once

#ifndef __ZNDIP_H__
#define __ZNDIP_H__

#include "cpu/psx/siodev.h"

extern const device_type ZNDIP;

#define MCFG_ZNDIP_DATA_HANDLER(_devcb) \
	devcb = &zndip_device::set_data_handler(*device, DEVCB2_##_devcb); \

class zndip_device : public psxsiodev_device
{
public:
	zndip_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	template<class _Object> static devcb2_base &set_data_handler(device_t &device, _Object object) { return downcast<zndip_device &>(device).m_data_handler.set_callback(object); }

	void select(int select);

protected:
	void device_start();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	virtual void data_in( int data, int mask );

	devcb2_read8 m_data_handler;

	int m_select;
	UINT8 m_bit;
	emu_timer *m_dip_timer;
};

#endif
