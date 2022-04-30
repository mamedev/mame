// license:BSD-3-Clause
// copyright-holders: Olivier Galibert

// ADB - Apple Desktop Bus
//
// The serial desktop device bus from before USB was cool.
//
// Single data wire + poweron line, open collector

#include "emu.h"
#include "adb.h"

#include "adbhle.h"
#include "a9m0330.h"
#include "a9m0331.h"

DEFINE_DEVICE_TYPE(ADB_CONNECTOR, adb_connector, "adbslot", "ADB connector")

adb_connector::adb_connector(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ADB_CONNECTOR, tag, owner, clock),
	device_single_card_slot_interface<adb_slot_card_interface>(mconfig, *this)
{
}

void adb_connector::device_start()
{
}

adb_device *adb_connector::get_device()
{
	adb_slot_card_interface *const connected = get_card_device();
	if (connected)
		return connected->device().subdevice<adb_device>(connected->m_adb.finder_tag());
	else
		return nullptr;
}

adb_slot_card_interface::adb_slot_card_interface(const machine_config &mconfig, device_t &device, const char *adb_tag) :
	device_interface(device, "adb"),
	m_adb(*this, adb_tag)
{
}



adb_device::adb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_adb_cb(*this),
	m_poweron_cb(*this)
{
}

void adb_device::device_start()
{
	m_adb_cb.resolve_safe();
	m_poweron_cb.resolve_safe();
}

void adb_device::device_reset()
{
	m_adb_istate = true;
	m_adb_ostate = true;
}

void adb_device::default_devices(device_slot_interface &device)
{
	device.option_add("hle", ADB_HLE);
	device.option_add("a9m0330", ADB_A9M0330);
	device.option_add("a9m0331", ADB_A9M0331);
}
