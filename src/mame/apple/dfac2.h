// license:BSD-3-Clause
// copyright-holders:R. Belmont, Tim Douglas
#ifndef MAME_APPLE_DFAC2_H
#define MAME_APPLE_DFAC2_H

#pragma once

#include "machine/i2chle.h"
#include "sound/flt_biquad.h"
#include "sound/flt_vol.h"

class dfac2_device : public device_t, public device_mixer_interface, public i2c_hle_interface
{
public:
	dfac2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	required_device<filter_volume_device> const &dac(u32 channel) const { return m_dac[channel]; }
	required_device<filter_volume_device> const &rec(u32 input) const { return m_rec[input]; }

protected:
	// device_r overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	// i2c_hle_interface overrides
	virtual u8 read_data(u16 offset) override;
	virtual bool one_byte_transfers() const override;
	virtual bool subaddress_valid(u8 subaddress) const override;
	virtual void write_data(u16 offset, u8 data) override;
	virtual const char *get_tag() override { return tag(); }

private:
	void update_controls();

	required_device_array<filter_volume_device, 2> m_dac;
	required_device_array<filter_biquad_device, 2> m_filter;
	required_device_array<filter_volume_device, 4> m_rec;
	u8 m_regs[0x11];
};

DECLARE_DEVICE_TYPE(APPLE_DFAC2, dfac2_device)

#endif // MAME_APPLE_DFAC2_H
