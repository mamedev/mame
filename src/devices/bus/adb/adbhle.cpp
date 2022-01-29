// license:BSD-3-Clause
// copyright-holders: Olivier Galibert

// ADB - Apple Desktop Bus
//
// Generic HLE

#include "emu.h"
#include "adbhle.h"

DEFINE_DEVICE_TYPE(ADB_HLE, adb_hle_device, "adbhle", "ADB HLE")

adb_hle_device::adb_hle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	adb_device(mconfig, ADB_HLE, tag, owner, clock),
	adb_slot_card_interface(mconfig, *this, DEVICE_SELF)
{
}

void adb_hle_device::device_start()
{
	adb_device::device_start();

	save_item(NAME(m_last_state));
	save_item(NAME(m_last_state_time));
}

void adb_hle_device::device_reset()
{
	adb_device::device_reset();
	m_last_state = true;
	m_last_state_time = machine().time();
}

void adb_hle_device::adb_w(int state)
{
	if(m_last_state != state) {
		attotime delta = machine().time() - m_last_state_time;
		u32 dt = delta.as_ticks(1000000);

		logerror("level %d duration %6d us\n", m_last_state, dt);

		m_last_state = state;
		m_last_state_time = machine().time();
	}
}
