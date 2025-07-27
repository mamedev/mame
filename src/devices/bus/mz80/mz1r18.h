// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_BUS_MZ80_MZ1R18_H
#define MAME_BUS_MZ80_MZ1R18_H

#pragma once

#include "mz80_exp.h"

class mz1r18_device : public device_t, public device_mz80_exp_interface
{
public:
	// device type constructor
	mz1r18_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// device_mz80_exp_interface implementation
	virtual void io_map(address_map &map) override ATTR_COLD;

private:
	u8 ram_data_r();
	void ram_data_w(u8 data);
	void ram_address_w(offs_t offset, u8 data);

	std::unique_ptr<u8 []> m_ram;
	u16 m_ram_address;
};

// device type declaration
DECLARE_DEVICE_TYPE(MZ1R18, mz1r18_device)

#endif // MAME_BUS_MZ80_MZ1R18_H
