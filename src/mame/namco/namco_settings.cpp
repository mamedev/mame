// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "namco_settings.h"

#define VERBOSE ( 0 )
#include "logmacro.h"

DEFINE_DEVICE_TYPE(NAMCO_SETTINGS, namco_settings_device, "namco_settings", "Namco Settings")

namco_settings_device::namco_settings_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NAMCO_SETTINGS, tag, owner, clock)
{
}

void namco_settings_device::ce_w(int state)
{
	if(ce != state) {
		ce = state;
		if(ce)
			cur_bit = 0;
	}
}

void namco_settings_device::clk_w(int state)
{
	if(clk != state) {
		clk = state;
		if(clk) {
			if(cur_bit < 8)
				adr = (adr >> 1) | (data << 7);
			else
				value = (value << 1) | data;
			cur_bit++;
			if(cur_bit == 16) {
				cur_bit = 0;
				LOG("%s: %02x = %02x\n", tag(), adr, value);
			}
		}
	}
}

void namco_settings_device::data_w(int state)
{
	data = state;
}

void namco_settings_device::device_start()
{
	save_item(NAME(adr));
	save_item(NAME(value));
	save_item(NAME(ce));
	save_item(NAME(clk));
	save_item(NAME(data));
	save_item(NAME(cur_bit));
}

void namco_settings_device::device_reset()
{
	adr = value = 0x00;
	ce = false;
	clk = true;
	data = false;
	cur_bit = 0;
}
