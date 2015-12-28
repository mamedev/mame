// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "wpc_lamp.h"

const device_type WPC_LAMP = &device_creator<wpc_lamp_device>;

wpc_lamp_device::wpc_lamp_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, WPC_LAMP, "Williams Pinball Controller Lamp Control", tag, owner, clock, "wpc_lamp", __FILE__)
{
	names = nullptr;
}

wpc_lamp_device::~wpc_lamp_device()
{
}

void wpc_lamp_device::set_names(const char *const *_names)
{
	names = _names;
}

void wpc_lamp_device::update()
{
	for(int i=0; i<8; i++)
		if(row & (1 << i))
			for(int j=0; j<8; j++)
				if(col & (1 << j))
					state[(j<<3)|i] |= 0x80;
}

WRITE8_MEMBER(wpc_lamp_device::row_w)
{
	row = data;
	update();
}

WRITE8_MEMBER(wpc_lamp_device::col_w)
{
	col = data;
	update();
}

void wpc_lamp_device::device_start()
{
	save_item(NAME(state));
	save_item(NAME(col));
	save_item(NAME(row));

	timer = timer_alloc(0);
}

void wpc_lamp_device::device_reset()
{
	memset(state, 0x00, 64);
	timer->adjust(attotime::from_hz(60), 0, attotime::from_hz(60));
}

void wpc_lamp_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	for(int i=0; i<64; i++) {
		UINT8 s = state[i];
		state[i] = s >> 1;
		if((s & 0xc0) == 0x40 || (s & 0xc0) == 0x80) {
			char buffer[256];
			if(names && names[i])
				sprintf(buffer, "l:%s", names[i]);
			else
				sprintf(buffer, "l:%d%d", 1+(i >> 3), 1 + (i & 7));
			output_set_value(buffer, (s & 0xc0) == 0x80);
		}
	}
}
