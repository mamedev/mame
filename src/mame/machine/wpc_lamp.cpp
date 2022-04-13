// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "wpc_lamp.h"

DEFINE_DEVICE_TYPE(WPC_LAMP, wpc_lamp_device, "wpc_lamp", "Williams Pinball Controller Lamp Control")

wpc_lamp_device::wpc_lamp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, WPC_LAMP, tag, owner, clock)
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

void wpc_lamp_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	for(int i=0; i<64; i++) {
		uint8_t s = state[i];
		state[i] = s >> 1;
		if((s & 0xc0) == 0x40 || (s & 0xc0) == 0x80) {
			char buffer[256];
			if(names && names[i])
				sprintf(buffer, "l:%s", names[i]);
			else
				sprintf(buffer, "l:%d%d", 1+(i >> 3), 1 + (i & 7));
			machine().output().set_value(buffer, (s & 0xc0) == 0x80);
		}
	}
}
