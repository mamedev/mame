// license:BSD-3-Clause
// copyright-holders:Katherine Rohl
/***************************************************************************

    MCA card template

***************************************************************************/

#ifndef MAME_BUS_MCA_IBM_SVGA_H
#define MAME_BUS_MCA_IBM_SVGA_H

#pragma once

#include "mca.h"
#include "video/clgd542x.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mca16_ibm_svga_device

class mca16_ibm_svga_device :
		public device_t,
		public device_mca16_card_interface
{
public:
	// construction/destruction
	mca16_ibm_svga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void unmap() override;
	virtual void remap() override;

protected:
	mca16_ibm_svga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual uint8_t pos_r(offs_t offset) override;
	virtual void pos_w(offs_t offset, uint8_t data) override;

    void map_main(address_map &map);

	uint8_t port_03b0_r(offs_t offset);
    uint8_t port_03c0_r(offs_t offset);
    uint8_t port_03d0_r(offs_t offset);

    void port_03b0_w(offs_t offset, uint8_t data);
    void port_03c0_w(offs_t offset, uint8_t data);
    void port_03d0_w(offs_t offset, uint8_t data);

private:
	bool m_is_mapped;

	required_device<cirrus_gd5428_device> m_svga;
};

// device type definition
DECLARE_DEVICE_TYPE(MCA16_IBM_SVGA, mca16_ibm_svga_device)

#endif