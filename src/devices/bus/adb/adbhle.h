// license:BSD-3-Clause
// copyright-holders: Olivier Galibert

// ADB - Apple Desktop Bus
//
// Generic HLE

#ifndef MAME_BUS_ADB_ADBHLE_H
#define MAME_BUS_ADB_ADBHLE_H

#pragma once

#include "adb.h"

class adb_hle_device : public adb_device, public adb_slot_card_interface
{
public:
	adb_hle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void adb_w(int state) override;

private:
	bool m_last_state;
	attotime m_last_state_time;
};

DECLARE_DEVICE_TYPE(ADB_HLE, adb_hle_device)

#endif
