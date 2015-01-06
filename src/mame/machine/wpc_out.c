// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "wpc_out.h"

const device_type WPC_OUT = &device_creator<wpc_out_device>;

wpc_out_device::wpc_out_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, WPC_OUT, "Williams Pinball Controller Output Control", tag, owner, clock, "wpc_out", __FILE__)
{
	names = NULL;
}

wpc_out_device::~wpc_out_device()
{
}

void wpc_out_device::set_names(const char *const *_names)
{
	names = _names;
}

void wpc_out_device::set_handler(handler_t cb)
{
	handler_cb = cb;
}

void wpc_out_device::set_gi_count(int _count)
{
	gi_count = _count;
}

void wpc_out_device::gi_update()
{
	attotime now = machine().time();
	attotime delta = now - previous_gi_update;
	UINT32 delta_us = delta.as_ticks(1e6);
	for(int i=0; i<gi_count; i++)
		if(gi & (1 <<i))
			gi_time[i] += delta_us;
	previous_gi_update = now;
}

void wpc_out_device::send_output(int sid, int state)
{
	if(!handler_cb.isnull() && handler_cb(sid, state))
		return;

	char buffer[32];
	const char *name;
	if(names && names[sid-1] && strcmp(names[sid-1], "s:"))
		name = names[sid-1];
	else {
		sprintf(buffer, "u:output %02d", sid);
		name = buffer;
	}
	output_set_value(name, state);

	if(sid == 41)
		coin_counter_w(machine(), 0, state);
}

WRITE8_MEMBER(wpc_out_device::out_w)
{
	first_after_led = false;
	UINT8 diff = state[offset] ^ data;
	state[offset] = data;
	if(diff)
		for(int i=0; i<8; i++)
			if(diff & (1 << i)) {
				int id = (offset << 3) | i;
				int sid;
				if(id <= 3)
					sid = id + 25;
				else if(id <= 7)
					sid = id + 33;
				else if(id <= 15)
					sid = id-7;
				else if(id <= 23)
					sid = id+1;
				else if(id <= 31)
					sid = id-15;
				else if(id <= 39)
					sid = id-3;
				else
					sid = id+2;

				send_output(sid, (data & (1<<i)) != 0);
			}
}

WRITE8_MEMBER(wpc_out_device::out4_w)
{
	// This is gross, probably wrong, but also the best I could find.
	// Test case is No Good Gofers (ngg_13).
	out_w(space, first_after_led ? 5 : 4, data, mem_mask);
}

WRITE8_MEMBER(wpc_out_device::gi_w)
{
	gi_update();
	if((gi^data) & 0x80)
		send_output(41, data & 0x80 ? 1 : 0);
	gi = data;
}

WRITE8_MEMBER(wpc_out_device::led_w)
{
	first_after_led = true;
	output_set_value("L:cpu led", data & 0x80 ? 1 : 0);
}

void wpc_out_device::device_start()
{
	save_item(NAME(state));
	save_item(NAME(gi));
	save_item(NAME(first_after_led));
	save_item(NAME(previous_gi_update));
	save_item(NAME(gi_time));

	timer = timer_alloc(0);
}

void wpc_out_device::device_reset()
{
	memset(state, 0x00, 6);
	first_after_led = false;
	gi = 0x00;
	previous_gi_update = attotime::zero;
	memset(gi_time, 0, sizeof(gi_time));
	timer->adjust(attotime::from_hz(10), 0, attotime::from_hz(10));
}

void wpc_out_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	gi_update();
	for(int i=0; i<gi_count; i++) {
		//      fprintf(stderr, "gi[%d] = %d\n", i, gi_time[i]);
		gi_time[i] = 0;
	}
}
