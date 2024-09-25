// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Williams Pinball Controller lamp control

#ifndef MAME_PINBALL_WPC_LAMP_H
#define MAME_PINBALL_WPC_LAMP_H

#pragma once

class wpc_lamp_device : public device_t
{
public:
	wpc_lamp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~wpc_lamp_device();

	void row_w(uint8_t data);
	void col_w(uint8_t data);

	void set_names(const char *const *lamp_names);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(update_lamps);

	void update();

	uint8_t state[64]{};
	uint8_t col = 0;
	uint8_t row = 0;
	emu_timer *timer = nullptr;
	const char *const *names;
};

DECLARE_DEVICE_TYPE(WPC_LAMP, wpc_lamp_device)

#endif // MAME_PINBALL_WPC_LAMP_H
