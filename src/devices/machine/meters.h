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

#ifndef MAME_MACHINE_METERS_H
#define MAME_MACHINE_METERS_H

#pragma once


class meters_device : public device_t
{
public:
	static constexpr unsigned MAXMECHMETERS = 8;

	static constexpr double METERREACTTIME = 0.025; // number of seconds meter has to be active to tick

	meters_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_number(int number) { m_number_mtr = number; }

	int update(int id, int state);
	int GetActivity(int id);

	int GetNumberMeters(void);  // currently unused
	void Setcount(int id, int32_t count); // currently unused
	int32_t Getcount(int id); // currently unused
	void ReactTime(int id, int32_t cycles); // currently unused

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;

private:
	// internal state
	struct meter_info
	{
		bool on;    // Activity of reel
		int32_t reacttime;
		int32_t count;      // mechmeter value
		bool state;     // state 0/1
		emu_timer *meter_timer;
	};

	meter_info m_meter_info[MAXMECHMETERS];

	int m_number_mtr;
};

DECLARE_DEVICE_TYPE(METERS, meters_device)

#endif // MAME_MACHINE_METERS_H
