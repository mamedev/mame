// license:BSD-3-Clause
// copyright-holders:Katherine Rohl
/***************************************************************************

    IBM PS/2 Planar LPT.

***************************************************************************/

#ifndef MAME_BUS_MCA_PLANAR_LPT_H
#define MAME_BUS_MCA_PLANAR_LPT_H

#pragma once

#include "mca.h"
#include "machine/pc_lpt.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mca16_planar_lpt_device

class mca16_planar_lpt_device :
		public device_t,
		public device_mca16_card_interface
{
public:
	// construction/destruction
	mca16_planar_lpt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

    void enable();
    void disable();

	virtual void unmap() override {};
	virtual void remap() override {};

    void planar_remap(int space_id, offs_t start, offs_t end);
	void planar_remap_irq(uint8_t line);

	virtual uint8_t io8_r(offs_t offset) override;
	virtual void io8_w(offs_t offset, uint8_t data) override;

protected:
	mca16_planar_lpt_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;

	required_device<pc_lpt_device> m_lpt;

private:
	bool 	m_is_mapped;
    offs_t  m_cur_io_start, m_cur_io_end;
    uint8_t m_cur_irq;
};

// device type definition
DECLARE_DEVICE_TYPE(MCA16_PLANAR_LPT, mca16_planar_lpt_device)

#endif