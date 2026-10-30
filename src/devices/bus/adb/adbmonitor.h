// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_BUS_ADB_ADBMONITOR_H
#define MAME_BUS_ADB_ADBMONITOR_H

#pragma once

#include "adb.h"

class adb_monitor_device : public adb_device_interface, public adb_slot_card_interface
{
public:
	// construction/destruction
	adb_monitor_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);


protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void adb_w(int state) override;

private:
	u8 m_buffer[16];
	u64 m_last_adb_time;
	s32 m_linein;
	s32 m_stream_ptr, m_linestate;
	u8 m_command;
	bool m_to_host;
};

// device type definition
DECLARE_DEVICE_TYPE(ADB_MONITOR, adb_monitor_device)

#endif // MAME_BUS_ADB_ADBMONITOR_H
