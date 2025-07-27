// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_BUS_MZ80_MZ1E05_H
#define MAME_BUS_MZ80_MZ1E05_H

#pragma once

#include "mz80_exp.h"

#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"

class mz1e05_device : public device_t, public device_mz80_exp_interface
{
public:
	// device type constructor
	mz1e05_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_mz80_exp_interface implementation
	virtual void io_map(address_map &map) override ATTR_COLD;

private:
	void dm_w(u8 data);
	void hs_w(u8 data);
	void fm_w(u8 data);

	required_device<wd_fdc_device_base> m_fdc;
	required_device_array<floppy_connector, 4> m_fdd;

	u8 m_dm;
	u8 m_hs;
};

// device type declaration
DECLARE_DEVICE_TYPE(MZ1E05, mz1e05_device)

#endif // MAME_BUS_MZ80_MZ1E05_H
