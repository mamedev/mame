// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn ANC13 ARM Evaluation System

**********************************************************************/


#ifndef MAME_BUS_BBC_TUBE_ARM_H
#define MAME_BUS_BBC_TUBE_ARM_H

#include "tube.h"
#include "cpu/arm/arm.h"
#include "machine/ram.h"
#include "machine/tube.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bbc_tube_arm_device

class bbc_tube_arm_device :
	public device_t,
	public device_bbc_tube_interface
{
public:
	// construction/destruction
	bbc_tube_arm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual uint8_t host_r(offs_t offset) override;
	virtual void host_w(offs_t offset, uint8_t data) override;

private:
	required_device<arm_cpu_device> m_maincpu;
	required_device<tube_device> m_ula;
	required_device<ram_device> m_ram;
	required_memory_region m_bootstrap;

	memory_passthrough_handler m_rom_shadow_tap;

	void tube_arm_mem(address_map &map) ATTR_COLD;

	void prst_w(int state);
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_TUBE_ARM, bbc_tube_arm_device)


#endif /* MAME_BUS_BBC_TUBE_ARM_H */
