// license:GPL2+
// copyright-holders:Felipe Sanches

#include "emu.h"
#include "kn6000_expansion.h"

#include "hdsx3.h"

DEFINE_DEVICE_TYPE(KN6000_EXPANSION, kn6000_expansion_connector, "kn6000_expansion", "SX-KN6000 expansion connector")

kn6000_expansion_connector::kn6000_expansion_connector(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, KN6000_EXPANSION, tag, owner, clock)
	, device_single_card_slot_interface<device_kn6000_expansion_interface>(mconfig, *this)
{
}

void kn6000_expansion_connector::device_start()
{
}

device_kn6000_expansion_interface::device_kn6000_expansion_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "kn6000exp")
{
}

void kn6000_expansion_intf(device_slot_interface &device)
{
	device.option_add("hdsx3", HDSX3);
}
