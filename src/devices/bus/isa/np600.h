// license:BSD-3-Clause
// copyright-holders:AJR
/*********************************************************************

    InterLan NP600 Intelligent Protocol Processor

*********************************************************************/

#ifndef MAME_BUS_ISA_NP600_H
#define MAME_BUS_ISA_NP600_H

#pragma once

#include "isa.h"
#include "machine/i82586.h"

class np600a3_device : public device_t, public device_isa16_card_interface
{
public:
	np600a3_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type unemulated_features() { return feature::LAN; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	void lcc_ca_w(u16 data);
	void int1_ack_w(u8 data);
	void host_int_w(int state);
	u16 status_r();
	u8 enetaddr_r(offs_t offset);

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void lcc_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_npcpu;
	required_device<i82586_device> m_lcc;
	required_region_ptr<u8> m_enetaddr;
};

DECLARE_DEVICE_TYPE(NP600A3, np600a3_device)

#endif // MAME_BUS_ISA_NP600_H
