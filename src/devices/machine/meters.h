// license:BSD-3-Clause
// copyright-holders:James Wallace
/**********************************************************************

    Electromechanical Meter device

**********************************************************************/

#ifndef MAME_MACHINE_METERS_H
#define MAME_MACHINE_METERS_H

#pragma once


class meters_device : public device_t
{
public:
	meters_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_number(int number) { m_number_mtr = number; }

	int update(int id, int state);
	int get_activity(int id);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(count_tick);

private:
	static constexpr unsigned MAXMECHMETERS = 8;
	static constexpr double METERREACTTIME = 0.025; // number of seconds meter has to be active to tick

	// internal state
	struct meter_info
	{
		bool on;    // Reel active
		int32_t reacttime;
		int32_t count;      // Meter value
		bool state;
		emu_timer *meter_timer;
	};

	meter_info m_meter_info[MAXMECHMETERS];

	int m_number_mtr;
};

DECLARE_DEVICE_TYPE(METERS, meters_device)

#endif // MAME_MACHINE_METERS_H
