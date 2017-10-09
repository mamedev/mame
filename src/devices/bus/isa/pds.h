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

	DECLARE_READ8_MEMBER(ppi_r);
	DECLARE_WRITE8_MEMBER(ppi_w);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_stop() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;

	required_device<i8255_device> m_ppi;
};

DECLARE_DEVICE_TYPE(ISA8_PDS, isa8_pds_device)

#endif // MAME_BUS_ISA_PDS_H
