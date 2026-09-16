// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_BUS_ISA_WSS_H
#define MAME_BUS_ISA_WSS_H

#pragma once

#include "isa.h"
#include "sound/ad1848.h"
#include "sound/ymopl.h"

class isa16_wss_device : public device_t,
						 public device_isa16_card_interface
{
public:
	isa16_wss_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type imperfect_features() { return feature::SOUND; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	// virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void remap(int space_id, offs_t start, offs_t end) override;

private:
	required_device<ymf262_device> m_opl3;
	required_device<ad1848_device> m_soundport;

	void host_io(address_map &map);
};


DECLARE_DEVICE_TYPE(ISA16_WSS, isa16_wss_device)

#endif // MAME_BUS_ISA_WSS_H
