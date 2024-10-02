// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Novell Disk Coprocessor Board (DCB)

***************************************************************************/

#ifndef MAME_BUS_ISA_DCB_H
#define MAME_BUS_ISA_DCB_H

#pragma once

#include "isa.h"
#include "cpu/i86/i186.h"
#include "machine/eepromser.h"

class novell_dcb_device : public device_t, public device_isa16_card_interface
{
public:
	novell_dcb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type unemulated_features() { return feature::DISK; }

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	void eeprom_w(u8 data);
	u8 misc_r();

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	required_device<i80188_cpu_device> m_localcpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
};

DECLARE_DEVICE_TYPE(NOVELL_DCB, novell_dcb_device)

#endif // MAME_BUS_ISA_DCB_H
