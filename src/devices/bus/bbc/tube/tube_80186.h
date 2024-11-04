// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn ADC08 Intel 80186 Co-processor

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Acorn_ADC08_80186Copro.html

**********************************************************************/


#ifndef MAME_BUS_BBC_TUBE_80186_H
#define MAME_BUS_BBC_TUBE_80186_H

#include "tube.h"
#include "cpu/i86/i186.h"
#include "machine/ram.h"
#include "machine/tube.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bbc_tube_80186_device

class bbc_tube_80186_device :
	public device_t,
	public device_bbc_tube_interface
{
public:
	// construction/destruction
	bbc_tube_80186_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	bbc_tube_80186_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual uint8_t host_r(offs_t offset) override;
	virtual void host_w(offs_t offset, uint8_t data) override;

	required_device<i80186_cpu_device> m_i80186;
	required_device<tube_device> m_ula;
	required_device<ram_device> m_ram;
	required_memory_region m_bootstrap;

	void tube_80186_io(address_map &map) ATTR_COLD;
	void tube_80186_mem(address_map &map) ATTR_COLD;

	void prst_w(int state);
};


class bbc_tube_pcplus_device : public bbc_tube_80186_device
{
public:
	bbc_tube_pcplus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_TUBE_80186, bbc_tube_80186_device)
DECLARE_DEVICE_TYPE(BBC_TUBE_PCPLUS, bbc_tube_pcplus_device)


#endif /* MAME_BUS_BBC_TUBE_80186_H */
