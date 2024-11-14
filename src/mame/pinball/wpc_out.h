// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Williams Pinball Controller outputs control (solenoids, flashers, generic logic, global illumination, coin counter, cpu led)

#ifndef MAME_PINBALL_WPC_OUT_H
#define MAME_PINBALL_WPC_OUT_H

#pragma once

class wpc_out_device : public device_t
{
public:
	typedef delegate<bool (int, bool)> handler_t;

	wpc_out_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, int gi_count)
		: wpc_out_device(mconfig, tag, owner, clock)
	{
		set_gi_count(gi_count);
	}

	wpc_out_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~wpc_out_device();

	void out_w(offs_t offset, uint8_t data);
	void out4_w(uint8_t data); // fixed offset 4
	void gi_w(uint8_t data);
	void led_w(uint8_t data);

	void set_names(const char *const *names);
	void set_handler(handler_t cb);
	void set_gi_count(int _count);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(update_outputs);

	void gi_update();

	void send_output(int sid, int state);

	uint8_t state[6]{}, gi = 0;
	bool first_after_led = false;
	attotime previous_gi_update;
	int gi_count = 0;
	uint32_t gi_time[5];
	emu_timer *timer = nullptr;
	const char *const *names;
	handler_t handler_cb;
};

DECLARE_DEVICE_TYPE(WPC_OUT, wpc_out_device)

#endif // MAME_PINBALL_WPC_OUT_H
