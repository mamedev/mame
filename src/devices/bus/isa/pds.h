// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * isa_pds.h
 *
 *  Created on: 31/01/2014
 */

#ifndef ISA_PDS_H_
#define ISA_PDS_H_

#include "emu.h"
#include "isa.h"
#include "machine/i8255.h"

class isa8_pds_device :
		public device_t,
		public device_isa8_card_interface
{
public:
		isa8_pds_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

		DECLARE_READ8_MEMBER(ppi_r);
		DECLARE_WRITE8_MEMBER(ppi_w);

		// optional information overrides
		virtual machine_config_constructor device_mconfig_additions() const override;

		required_device<i8255_device> m_ppi;
protected:
		// device-level overrides
		virtual void device_start() override;
		virtual void device_reset() override;
		virtual void device_stop() override;

};

extern const device_type ISA8_PDS;

#endif /* ISA_PDS_H_ */
