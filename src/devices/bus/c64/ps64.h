// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Koulun erityispalvelu PS-64 speech cartridge emulation

**********************************************************************/

#ifndef MAME_BUS_C64_PS64_H
#define MAME_BUS_C64_PS64_H

#pragma once


#include "exp.h"
#include "sound/ssi263hle.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_ps64_cartridge_device

class c64_ps64_cartridge_device : public device_t,
								  public device_c64_expansion_card_interface,
								  public device_memory_interface
{
public:
	// construction/destruction
	c64_ps64_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_c64_expansion_card_interface overrides
	virtual uint8_t c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;
	virtual void c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;
	virtual int c64_exrom_r(offs_t offset, int sphi2, int ba, int rw) override;

private:
	void io_map(address_map &map) ATTR_COLD;

	required_device<ssi263hle_device> m_ssi263;
	required_ioport m_sw1;
	address_space_config m_io_space_config;

	bool m_enabled;
};


// device type definition
DECLARE_DEVICE_TYPE(C64_PS64, c64_ps64_cartridge_device)


#endif // MAME_BUS_C64_PS64_H
