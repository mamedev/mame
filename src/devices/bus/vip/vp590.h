// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    RCA VIP Color Board VP590 emulation

**********************************************************************/

#ifndef MAME_BUS_VIP_VP590_H
#define MAME_BUS_VIP_VP590_H

#pragma once

#include "machine/rescap.h"
#include "exp.h"
#include "video/cdp1862.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vp590_device

class vp590_device : public device_t,
						public device_vip_expansion_card_interface
{
public:
	// construction/destruction
	vp590_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_vip_expansion_card_interface overrides
	virtual void vip_program_w(offs_t offset, uint8_t data, int cdef, int *minh) override;
	virtual void vip_io_w(offs_t offset, uint8_t data) override;
	virtual void vip_dma_w(offs_t offset, uint8_t data) override;
	virtual uint32_t vip_screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) override;
	virtual int vip_ef3_r() override;
	virtual int vip_ef4_r() override;

private:
	int rd_r();
	int bd_r();
	int gd_r();

	required_device<cdp1862_device> m_cgc;
	memory_share_creator<uint8_t> m_color_ram;
	required_ioport m_j1;
	required_ioport m_j2;

	int m_a12;
	uint8_t m_color;
	uint8_t m_keylatch;
};


// device type definition
DECLARE_DEVICE_TYPE(VP590, vp590_device)

#endif // MAME_BUS_VIP_VP590_H
