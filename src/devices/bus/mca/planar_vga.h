// license:BSD-3-Clause
// copyright-holders:Katherine Rohl
/***************************************************************************

    IBM PS/2 Planar VGA.

***************************************************************************/

#ifndef MAME_BUS_MCA_PLANAR_VGA_H
#define MAME_BUS_MCA_PLANAR_VGA_H

#pragma once

#include "mca.h"
#include "video/pc_vga.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mca16_planar_vga_device

class mca16_planar_vga_device :
		public device_t,
		public device_mca16_card_interface
{
public:
	// construction/destruction
	mca16_planar_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

    virtual void enable();
    virtual void disable();

	virtual void unmap() override {};
	virtual void remap() override {};

    virtual void planar_remap(int space_id, offs_t start, offs_t end);
	virtual void planar_remap_irq(uint8_t line);

	uint8_t port_03b0_r(offs_t offset);
    uint8_t port_03c0_r(offs_t offset);
    uint8_t port_03d0_r(offs_t offset);

    void port_03b0_w(offs_t offset, uint8_t data);
    void port_03c0_w(offs_t offset, uint8_t data);
    void port_03d0_w(offs_t offset, uint8_t data);

	void sleep_w(uint8_t data);
	uint8_t sleep_r() { return m_sleep; }

protected:
	mca16_planar_vga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;

    required_device<vga_device> m_vga;
    required_device<mca16_device> m_mcabus;

private:
	bool m_is_mapped;
	bool m_sleep;		// VGA does not respond to any data when cleared.

	DECLARE_WRITE_LINE_MEMBER(retrace_interrupt_w) { m_mcabus->ireq_w<2>(state); }

};

// device type definition
DECLARE_DEVICE_TYPE(MCA16_PLANAR_VGA, mca16_planar_vga_device)

#endif