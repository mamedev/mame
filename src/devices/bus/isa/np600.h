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
	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	void lcc_ca_w(u16 data);
	DECLARE_WRITE_LINE_MEMBER(lcc_reset_w);
	u16 status_r();

	void mem_map(address_map &map);
	void io_map(address_map &map);
	void lcc_map(address_map &map);

	required_device<cpu_device> m_npcpu;
	required_device<i82586_device> m_lcc;
};

DECLARE_DEVICE_TYPE(NP600A3, np600a3_device)

#endif // MAME_BUS_ISA_NP600_H
