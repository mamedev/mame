// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_MACHINE_DIMM_SPD_H
#define MAME_MACHINE_DIMM_SPD_H

#pragma once

#include "machine/i2chle.h"
class dimm_spd_device :  public device_t, public i2c_hle_interface
{
public:
	// construction/destruction
	dimm_spd_device(const machine_config &mconfig, const char *tag, device_t *owner)
		: dimm_spd_device(mconfig, tag, owner, (uint32_t)0)
	{
	}

	dimm_spd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	typedef enum
	{
		SIZE_SLOT_EMPTY = 0,
		SIZE_4_MIB,
		SIZE_8_MIB,
		SIZE_16_MIB,
		SIZE_32_MIB,
		SIZE_64_MIB,
		SIZE_128_MIB,
		SIZE_256_MIB
	} dimm_size_t;

	void set_dimm_size(dimm_size_t size);

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	// i2c_hle_interface overrides
	virtual u8 read_data(u16 offset) override;
	virtual const char *get_tag() override { return tag(); }

private:
	u8 m_data[256];
	dimm_size_t m_size;
};

// device type definition
DECLARE_DEVICE_TYPE(DIMM_SPD, dimm_spd_device)

#endif // MAME_MACHINE_DIMM_SPD_H
