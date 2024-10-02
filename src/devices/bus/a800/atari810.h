// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Atari 810 Disk Drive

***************************************************************************/

#ifndef MAME_BUS_A800_ATARI810
#define MAME_BUS_A800_ATARI810 1

#pragma once

#include "a8sio.h"
#include "machine/mos6530.h"
#include "machine/wd_fdc.h"


class atari810_device : public device_t, public device_a8sio_card_interface
{
public:
	atari810_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type unemulated_features() { return feature::DISK; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void data_out_w(int state) override;
	virtual void command_w(int state) override;
	virtual void ready_w(int state) override;

private:
	void step_w(u8 data);

	void mem_map(address_map &map) ATTR_COLD;

	required_device<mos6532_device> m_pia;
	required_device<fd1771_device> m_fdc;
};

DECLARE_DEVICE_TYPE(ATARI810, atari810_device)

#endif // MAME_BUS_A800_ATARI810
