// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "wpc_lamp.h"

DEFINE_DEVICE_TYPE(WPC_LAMP, wpc_lamp_device, "wpc_lamp", "Williams Pinball Controller Lamp Control")

wpc_lamp_device::wpc_lamp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, WPC_LAMP, tag, owner, clock),
	outputs(),
	timer(nullptr),
	col(0),
	row(0)
{
}

wpc_lamp_device::~wpc_lamp_device()
{
}

void wpc_lamp_device::set_names(char const *const (&lamp_names)[8][8])
{
	if (outputs)
		throw emu_fatalerror("wpc_lamp(%s): lamp output names may only be set once", tag());
	outputs.emplace(*this, lamp_names);
}

void wpc_lamp_device::update()
{
	for (int i = 0; i < 8; i++)
	{
		if (BIT(row, i))
		{
			for (int j = 0; j < 8; j++)
			{
				if (BIT(col, j))
				{
					state[(j << 3) | i] |= 0x80;
				}
			}
		}
	}
}

void wpc_lamp_device::row_w(uint8_t data)
{
	row = data;
	update();
}

void wpc_lamp_device::col_w(uint8_t data)
{
	col = data;
	update();
}

void wpc_lamp_device::device_config_complete()
{
	if (!outputs)
		outputs.emplace(*this, "l:%u%u", 0U, 0U);
}

void wpc_lamp_device::device_start()
{
	outputs->resolve();

	save_item(NAME(state));
	save_item(NAME(col));
	save_item(NAME(row));

	timer = timer_alloc(FUNC(wpc_lamp_device::update_lamps), this);
}

void wpc_lamp_device::device_reset()
{
	std::fill(std::begin(state), std::end(state), 0);
	timer->adjust(attotime::from_hz(60), 0, attotime::from_hz(60));
}

TIMER_CALLBACK_MEMBER(wpc_lamp_device::update_lamps)
{
	for (int i = 0; i < 64; i++)
	{
		uint8_t s = state[i];
		state[i] = s >> 1;
		if ((s & 0xc0) == 0x40 || (s & 0xc0) == 0x80)
			(*outputs)[i >> 3][i & 7] = (s & 0xc0) == 0x80;
	}
}
