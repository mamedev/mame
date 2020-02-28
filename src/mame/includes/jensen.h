// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_INCLUDES_JENSEN_H
#define MAME_INCLUDES_JENSEN_H

#pragma once

#include "cpu/alpha/alpha.h"

// memory
#include "machine/ram.h"
#include "machine/xc1700e.h"
#include "machine/intelfsh.h"

// various hardware
#include "machine/i82357.h"

// busses and connectors
#include "bus/rs232/rs232.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "bus/pc_kbd/keyboards.h"

class jensen_state : public driver_device
{
public:
	jensen_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "cpu")
		, m_ram(*this, "ram")
		, m_srom(*this, "srom")
		, m_feprom(*this, "feprom%u", 0)
		, m_isp(*this, "isp")
	{
	}

	// machine config
	void jensen(machine_config &config);

	void d2k300axp(machine_config &config);
	void d2k500axp(machine_config &config);
	void dpcaxp150(machine_config &config);

	void init_common();

protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	// address maps
	void local_memory(address_map &map);
	void local_io(address_map &map);
	void eisa_memory(address_map &map);
	void eisa_io(address_map &map);

private:
	// devices
	required_device<alpha_device> m_cpu;
	required_device<ram_device> m_ram;
	required_device<xc1765e_device> m_srom;
	required_device_array<intel_e28f008sa_device, 2> m_feprom;

	required_device<i82357_device> m_isp;

	// machine state
	u8 m_hae;
	u8 m_sysctl;
	u8 m_spare;
};

#endif // MAME_INCLUDES_JENSEN_H
