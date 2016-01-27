// license:BSD-3-Clause
// copyright-holders:James Wallace
///////////////////////////////////////////////////////////////////////////
//                                                                       //
// Electro mechanical meters                                             //
//                                                                       //
// 23-07-2004: Re-Animator                                               //
//                                                                       //
//                                                                       //
///////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __METERS_H__
#define __METERS_H__

#include "emu.h"


#define MCFG_METERS_NUMBER(_number) \
	meters_device::static_set_number_meters(*device, _number);
#define MAXMECHMETERS 8

#define METERREACTTIME 0.025 // number of seconds meter has to be active to tick

class meters_device : public device_t
{
public:
	meters_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~meters_device() {}

	static void static_set_number_meters(device_t &device, int number) { downcast<meters_device &>(device).m_number_mtr = number; }

	int update(int id, int state);
	int GetActivity(int id);

	int GetNumberMeters(void);  // currently unused
	void Setcount(int id, INT32 count); // currently unused
	INT32 Getcount(int id); // currently unused
	void ReactTime(int id, INT32 cycles); // currently unused

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	// internal state
	struct meter_info
	{
		bool on;    // Activity of reel
		INT32 reacttime;
		INT32 count;      // mechmeter value
		bool state;     // state 0/1
		emu_timer *meter_timer;
	};

	meter_info m_meter_info[MAXMECHMETERS];

	int m_number_mtr;
};

extern const device_type METERS;

#endif
