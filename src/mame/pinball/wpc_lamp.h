// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Williams Pinball Controller lamp control

#ifndef MAME_PINBALL_WPC_LAMP_H
#define MAME_PINBALL_WPC_LAMP_H

#pragma once


class wpc_lamp_device : public device_t
{
public:
	wpc_lamp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~wpc_lamp_device();

	void set_names(char const *const (&names)[8][8]);

	void row_w(uint8_t data);
	void col_w(uint8_t data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(update_lamps);

	void update();

	output_finder<8, 8> outputs;
	emu_timer *timer;
	uint8_t state[64];
	uint8_t col;
	uint8_t row;
};

DECLARE_DEVICE_TYPE(WPC_LAMP, wpc_lamp_device)

#endif // MAME_PINBALL_WPC_LAMP_H
