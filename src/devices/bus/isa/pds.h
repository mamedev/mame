// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * isa_pds.h
 *
 *  Created on: 31/01/2014
 */

#ifndef MAME_BUS_ISA_PDS_H
#define MAME_BUS_ISA_PDS_H

#include "isa.h"
#include "machine/i8255.h"

class isa8_pds_device :
		public device_t,
		public device_isa8_card_interface
{
public:
	isa8_pds_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t ppi_r(offs_t offset);
	void ppi_w(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<i8255_device> m_ppi;
};

DECLARE_DEVICE_TYPE(ISA8_PDS, isa8_pds_device)

#endif // MAME_BUS_ISA_PDS_H
