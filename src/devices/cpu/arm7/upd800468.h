// license:BSD-3-Clause
// copyright-holders:Devin Acker

#ifndef MAME_CPU_ARM7_UPD800468_H
#define MAME_CPU_ARM7_UPD800468_H

#pragma once

#include "arm7.h"
#include "arm7core.h"
#include "machine/gt913_kbd.h"
#include "machine/vic_pl192.h"

/***************************************************************************
	DEVICE CONFIGURATION MACROS
***************************************************************************/


/***************************************************************************
	TYPE DEFINITIONS
***************************************************************************/

class upd800468_device : public arm7_cpu_device
{
public:
	upd800468_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t);
	
	void upd800468_map(address_map &map);

	u32 ram_enable_r();
	void ram_enable_w(u32 data);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual space_config_vector memory_space_config() const override;

private:
	address_space_config m_program_config;

	required_device<gt913_kbd_hle_device> m_kbd;
	required_device<vic_upd800468_device> m_vic;

	memory_view m_ram_view;
	u32 m_ram_enable;
};


// device type definition
DECLARE_DEVICE_TYPE(UPD800468, upd800468_device)

#endif // MAME_CPU_ARM7_UPD800468_H
