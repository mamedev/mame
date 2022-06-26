// license:BSD-3-Clause
// copyright-holders:Katherine Rohl
/***************************************************************************

    IBM PS/2 Planar FDC.

***************************************************************************/

#ifndef MAME_BUS_MCA_PLANARFDC_H
#define MAME_BUS_MCA_PLANARFDC_H

#pragma once

#include "mca.h"
#include "imagedev/floppy.h"
#include "machine/upd765.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mca16_planar_fdc_device

class mca16_planar_fdc_device :
		public device_t,
		public device_mca16_card_interface
{
public:
	// construction/destruction
	mca16_planar_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_WRITE_LINE_MEMBER( irq_w );
	DECLARE_WRITE_LINE_MEMBER( drq_w );
	static void floppy_formats(format_registration &fr);

	virtual uint8_t io8_r(offs_t offset) override;
	virtual void io8_w(offs_t offset, uint8_t data) override;

	void enable();
	void disable();

	virtual void unmap() override {};
	virtual void remap() override {};

protected:
	mca16_planar_fdc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;

	virtual uint8_t dack_r(int line) override;
	virtual void dack_w(int line, uint8_t data) override;
	virtual void eop_w(int state) override;

	uint8_t dor_r();
	void dor_w(uint8_t data);
	uint8_t dir_r();
	void ccr_w(uint8_t data);

	required_device<n82077aa_device> m_fdc;

	DECLARE_WRITE_LINE_MEMBER( fdc_irq_w );
	DECLARE_WRITE_LINE_MEMBER( fdc_drq_w );
};

// device type definition
DECLARE_DEVICE_TYPE(MCA16_PLANAR_FDC, mca16_planar_fdc_device)

#endif