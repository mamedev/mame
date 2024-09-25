// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_BUS_NSCSI_CFP1080S_H
#define MAME_BUS_NSCSI_CFP1080S_H

#pragma once

#include "machine/nscsi_bus.h"

class cfp1080s_device : public nscsi_device, public nscsi_slot_card_interface
{
public:
	cfp1080s_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type unemulated_features() { return feature::DISK; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	u8 ff081_r();
	u8 ff082_r();
	void data_port_w(u8 data);
	void addrptrl_w(u8 data);
	void addrptrm_w(u8 data);
	void addrptrh_w(u8 data);

	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_hdcpu;

	u32 m_addrptr;
};

DECLARE_DEVICE_TYPE(CFP1080S, cfp1080s_device)

#endif // MAME_BUS_NSCSI_CFP1080S_H
