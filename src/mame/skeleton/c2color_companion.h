// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_SKELETON_C2COLOR_COMPANION_H
#define MAME_SKELETON_C2COLOR_COMPANION_H

#pragma once

#include "machine/i2chle.h"

DECLARE_DEVICE_TYPE(C2_COLOR_COMPANION, c2_color_companion_device)

// High-level model of the unidentified companion at I2C address 0x53.
// Only the observed three-byte challenge and two-byte response are understood.
// TODO: Identify the device and the remaining commands.
class c2_color_companion_device : public device_t, public i2c_hle_interface
{
public:
	c2_color_companion_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual u8 read_data(u16 offset) override;
	virtual void write_data(u16 offset, u8 data) override;
	virtual const char *get_tag() override { return tag(); }
	void clear_state();

private:
	u8 m_challenge[3];
	u8 m_received;
	u8 m_response;
};

#endif // MAME_SKELETON_C2COLOR_COMPANION_H
