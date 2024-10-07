// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn ANC05 32016 2nd processor

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Acorn_ANC05_320162ndproc.html

    Acorn ANC06 Cambridge Co-Processor

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Acorn_ANC06_CamCoPro.html

**********************************************************************/


#ifndef MAME_BUS_BBC_TUBE_32016_H
#define MAME_BUS_BBC_TUBE_32016_H

#include "tube.h"
#include "cpu/ns32000/ns32000.h"
#include "machine/ns32081.h"
#include "machine/ram.h"
#include "machine/tube.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bbc_tube_32016_device

class bbc_tube_32016_device :
	public device_t,
	public device_bbc_tube_interface
{
public:
	// construction/destruction
	bbc_tube_32016_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	bbc_tube_32016_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual uint8_t host_r(offs_t offset) override;
	virtual void host_w(offs_t offset, uint8_t data) override;

	required_device<ns32016_device> m_maincpu;
	required_device<ns32081_device> m_ns32081;
	required_device<tube_device> m_ula;
	required_device<ram_device> m_ram;
	required_memory_region m_rom;

	memory_passthrough_handler m_rom_shadow_tap;

	void tube_32016_mem(address_map &map) ATTR_COLD;

	void prst_w(int state);
};


class bbc_tube_16032_device : public bbc_tube_32016_device
{
public:
	bbc_tube_16032_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


class bbc_tube_32016l_device : public bbc_tube_32016_device
{
public:
	bbc_tube_32016l_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};



// device type definition
DECLARE_DEVICE_TYPE(BBC_TUBE_32016, bbc_tube_32016_device)
DECLARE_DEVICE_TYPE(BBC_TUBE_16032, bbc_tube_16032_device)
DECLARE_DEVICE_TYPE(BBC_TUBE_32016L, bbc_tube_32016l_device)


#endif /* MAME_BUS_BBC_TUBE_32016_H */
