// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * mach32.h
 *
 *  Created on: 16/05/2014
 */

#ifndef MAME_BUS_ISA_MACH32_H
#define MAME_BUS_ISA_MACH32_H

#pragma once

#include "video/pc_vga.h"
#include "machine/eepromser.h"

// 8514/A module of the Mach32
class mach32_8514a_device : public mach8_device
{
public:
	// construction/destruction
	mach32_8514a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ16_MEMBER(mach32_chipid_r) { return m_chip_ID; }
	DECLARE_WRITE16_MEMBER(mach32_clksel_w) { mach8.clksel = data; }  // read only on the mach8
	DECLARE_READ16_MEMBER(mach32_mem_boundary_r) { return m_membounds; }
	DECLARE_WRITE16_MEMBER(mach32_mem_boundary_w) { m_membounds = data; if(data & 0x10) logerror("ATI: Unimplemented memory boundary activated."); }

protected:
	mach32_8514a_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_config_complete() override;

	uint16_t m_chip_ID;
	uint16_t m_membounds;
};

// main SVGA device
class mach32_device : public ati_vga_device
{
public:
	// construction/destruction
	mach32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	required_device<mach32_8514a_device> m_8514a;  // provides accelerated 2D drawing, derived from the Mach8 device

	// map 8514/A functions to 8514/A module
	DECLARE_READ16_MEMBER(mach8_ec0_r) { return m_8514a->mach8_ec0_r(space,offset,mem_mask); }
	DECLARE_WRITE16_MEMBER(mach8_ec0_w) { m_8514a->mach8_ec0_w(space,offset,data,mem_mask); }
	DECLARE_READ16_MEMBER(mach8_ec1_r) { return m_8514a->mach8_ec1_r(space,offset,mem_mask); }
	DECLARE_WRITE16_MEMBER(mach8_ec1_w) { m_8514a->mach8_ec1_w(space,offset,data,mem_mask); }
	DECLARE_READ16_MEMBER(mach8_ec2_r) { return m_8514a->mach8_ec2_r(space,offset,mem_mask); }
	DECLARE_WRITE16_MEMBER(mach8_ec2_w) { m_8514a->mach8_ec2_w(space,offset,data,mem_mask); }
	DECLARE_READ16_MEMBER(mach8_ec3_r) { return m_8514a->mach8_ec3_r(space,offset,mem_mask); }
	DECLARE_WRITE16_MEMBER(mach8_ec3_w) { m_8514a->mach8_ec3_w(space,offset,data,mem_mask); }
	DECLARE_READ16_MEMBER(mach8_ext_fifo_r) { return m_8514a->mach8_ext_fifo_r(space,offset,mem_mask); }
	DECLARE_WRITE16_MEMBER(mach8_linedraw_index_w) { m_8514a->mach8_linedraw_index_w(space,offset,data,mem_mask); }
	DECLARE_READ16_MEMBER(mach8_bresenham_count_r) { return m_8514a->mach8_bresenham_count_r(space,offset,mem_mask); }
	DECLARE_WRITE16_MEMBER(mach8_bresenham_count_w) { m_8514a->mach8_bresenham_count_w(space,offset,data,mem_mask); }
	DECLARE_WRITE16_MEMBER(mach8_linedraw_w) { m_8514a->mach8_linedraw_w(space,offset,data,mem_mask); }
	DECLARE_READ16_MEMBER(mach8_linedraw_r) { return m_8514a->mach8_linedraw_r(space,offset,mem_mask); }
	DECLARE_READ16_MEMBER(mach8_scratch0_r) { return m_8514a->mach8_scratch0_r(space,offset,mem_mask); }
	DECLARE_WRITE16_MEMBER(mach8_scratch0_w) { m_8514a->mach8_scratch0_w(space,offset,data,mem_mask); }
	DECLARE_READ16_MEMBER(mach8_scratch1_r) { return m_8514a->mach8_scratch1_r(space,offset,mem_mask); }
	DECLARE_WRITE16_MEMBER(mach8_scratch1_w) { m_8514a->mach8_scratch1_w(space,offset,data,mem_mask); }
	DECLARE_READ16_MEMBER(mach8_config1_r) { return m_8514a->mach8_config1_r(space,offset,mem_mask); }
	DECLARE_READ16_MEMBER(mach8_config2_r) { return m_8514a->mach8_config2_r(space,offset,mem_mask); }
	DECLARE_READ16_MEMBER(mach8_sourcex_r) { return m_8514a->mach8_sourcex_r(space,offset,mem_mask); }
	DECLARE_READ16_MEMBER(mach8_sourcey_r) { return m_8514a->mach8_sourcey_r(space,offset,mem_mask); }
	DECLARE_WRITE16_MEMBER(mach8_ext_leftscissor_w) { m_8514a->mach8_ext_leftscissor_w(space,offset,data,mem_mask); }
	DECLARE_WRITE16_MEMBER(mach8_ext_topscissor_w) { m_8514a->mach8_ext_topscissor_w(space,offset,data,mem_mask); }

	DECLARE_READ16_MEMBER(ibm8514_vtotal_r) { return m_8514a->ibm8514_vtotal_r(space,offset,mem_mask); }
	DECLARE_WRITE16_MEMBER(ibm8514_vtotal_w) { m_8514a->ibm8514_vtotal_w(space,offset,data,mem_mask); }
	DECLARE_READ16_MEMBER(ibm8514_htotal_r) { return m_8514a->ibm8514_htotal_r(space,offset,mem_mask); }
	DECLARE_WRITE16_MEMBER(ibm8514_htotal_w) { m_8514a->ibm8514_htotal_w(space,offset,data,mem_mask); }
	DECLARE_READ16_MEMBER(ibm8514_vdisp_r) { return m_8514a->ibm8514_vdisp_r(space,offset,mem_mask); }
	DECLARE_WRITE16_MEMBER(ibm8514_vdisp_w) { m_8514a->ibm8514_vdisp_w(space,offset,data,mem_mask); }
	DECLARE_READ16_MEMBER(ibm8514_vsync_r) { return m_8514a->ibm8514_vsync_r(space,offset,mem_mask); }
	DECLARE_WRITE16_MEMBER(ibm8514_vsync_w) { m_8514a->ibm8514_vsync_w(space,offset,data,mem_mask); }
	DECLARE_READ16_MEMBER(ibm8514_substatus_r) { return m_8514a->ibm8514_substatus_r(space,offset,mem_mask); }
	DECLARE_WRITE16_MEMBER(ibm8514_subcontrol_w) { m_8514a->ibm8514_subcontrol_w(space,offset,data,mem_mask); }
	DECLARE_READ16_MEMBER(ibm8514_subcontrol_r) { return m_8514a->ibm8514_subcontrol_r(space,offset,mem_mask); }
	DECLARE_READ16_MEMBER(ibm8514_currentx_r) { return m_8514a->ibm8514_currentx_r(space,offset,mem_mask); }
	DECLARE_WRITE16_MEMBER(ibm8514_currentx_w) { m_8514a->ibm8514_currentx_w(space,offset,data,mem_mask); }
	DECLARE_READ16_MEMBER(ibm8514_currenty_r) { return m_8514a->ibm8514_currenty_r(space,offset,mem_mask); }
	DECLARE_WRITE16_MEMBER(ibm8514_currenty_w) { m_8514a->ibm8514_currenty_w(space,offset,data,mem_mask); }
	DECLARE_READ16_MEMBER(ibm8514_desty_r) { return m_8514a->ibm8514_desty_r(space,offset,mem_mask); }
	DECLARE_WRITE16_MEMBER(ibm8514_desty_w) { m_8514a->ibm8514_desty_w(space,offset,data,mem_mask); }
	DECLARE_READ16_MEMBER(ibm8514_destx_r) { return m_8514a->ibm8514_destx_r(space,offset,mem_mask); }
	DECLARE_WRITE16_MEMBER(ibm8514_destx_w) { m_8514a->ibm8514_destx_w(space,offset,data,mem_mask); }
	DECLARE_READ16_MEMBER(ibm8514_line_error_r) { return m_8514a->ibm8514_line_error_r(space,offset,mem_mask); }
	DECLARE_WRITE16_MEMBER(ibm8514_line_error_w) { m_8514a->ibm8514_line_error_w(space,offset,data,mem_mask); }
	DECLARE_READ16_MEMBER(ibm8514_width_r) { return m_8514a->ibm8514_width_r(space,offset,mem_mask); }
	DECLARE_WRITE16_MEMBER(ibm8514_width_w) { m_8514a->ibm8514_width_w(space,offset,data,mem_mask); }
	DECLARE_READ16_MEMBER(ibm8514_gpstatus_r) { return m_8514a->ibm8514_gpstatus_r(space,offset,mem_mask); }
	DECLARE_WRITE16_MEMBER(ibm8514_cmd_w) { m_8514a->ibm8514_cmd_w(space,offset,data,mem_mask); }
	DECLARE_READ16_MEMBER(ibm8514_ssv_r) { return m_8514a->ibm8514_ssv_r(space,offset,mem_mask); }
	DECLARE_WRITE16_MEMBER(ibm8514_ssv_w) { m_8514a->ibm8514_ssv_w(space,offset,data,mem_mask); }
	DECLARE_READ16_MEMBER(ibm8514_fgcolour_r) { return m_8514a->ibm8514_fgcolour_r(space,offset,mem_mask); }
	DECLARE_WRITE16_MEMBER(ibm8514_fgcolour_w) { m_8514a->ibm8514_fgcolour_w(space,offset,data,mem_mask); }
	DECLARE_READ16_MEMBER(ibm8514_bgcolour_r) { return m_8514a->ibm8514_bgcolour_r(space,offset,mem_mask); }
	DECLARE_WRITE16_MEMBER(ibm8514_bgcolour_w) { m_8514a->ibm8514_bgcolour_w(space,offset,data,mem_mask); }
	DECLARE_READ16_MEMBER(ibm8514_read_mask_r) { return m_8514a->ibm8514_read_mask_r(space,offset,mem_mask); }
	DECLARE_WRITE16_MEMBER(ibm8514_read_mask_w) { m_8514a->ibm8514_read_mask_w(space,offset,data,mem_mask); }
	DECLARE_READ16_MEMBER(ibm8514_write_mask_r) { return m_8514a->ibm8514_write_mask_r(space,offset,mem_mask); }
	DECLARE_WRITE16_MEMBER(ibm8514_write_mask_w) { m_8514a->ibm8514_write_mask_w(space,offset,data,mem_mask); }
	DECLARE_READ16_MEMBER(ibm8514_backmix_r) { return m_8514a->ibm8514_backmix_r(space,offset,mem_mask); }
	DECLARE_WRITE16_MEMBER(ibm8514_backmix_w) { m_8514a->ibm8514_backmix_w(space,offset,data,mem_mask); }
	DECLARE_READ16_MEMBER(ibm8514_foremix_r) { return m_8514a->ibm8514_foremix_r(space,offset,mem_mask); }
	DECLARE_WRITE16_MEMBER(ibm8514_foremix_w) { m_8514a->ibm8514_foremix_w(space,offset,data,mem_mask); }
	DECLARE_READ16_MEMBER(ibm8514_multifunc_r) { return m_8514a->ibm8514_multifunc_r(space,offset,mem_mask); }
	DECLARE_WRITE16_MEMBER(ibm8514_multifunc_w) { m_8514a->ibm8514_multifunc_w(space,offset,data,mem_mask); }
	DECLARE_READ16_MEMBER(ibm8514_pixel_xfer_r) { return m_8514a->ibm8514_pixel_xfer_r(space,offset,mem_mask); }
	DECLARE_WRITE16_MEMBER(ibm8514_pixel_xfer_w) { m_8514a->ibm8514_pixel_xfer_w(space,offset,data,mem_mask); }

	DECLARE_READ16_MEMBER(mach32_chipid_r) { return m_8514a->mach32_chipid_r(space,offset,mem_mask);  }
	DECLARE_READ16_MEMBER(mach8_clksel_r) { return m_8514a->mach8_clksel_r(space,offset,mem_mask); }
	DECLARE_WRITE16_MEMBER(mach32_clksel_w) { m_8514a->mach32_clksel_w(space,offset,data,mem_mask); }  // read only on the mach8
	DECLARE_READ16_MEMBER(mach32_mem_boundary_r) { return m_8514a->mach32_mem_boundary_r(space,offset,mem_mask); }
	DECLARE_WRITE16_MEMBER(mach32_mem_boundary_w) { m_8514a->mach32_mem_boundary_w(space,offset,data,mem_mask); }  // read only on the mach8
	DECLARE_READ16_MEMBER(mach32_status_r) { return vga_vblank() << 1; }

protected:
	mach32_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
};

/*
 *   ATi mach64
 */

// 8514/A module of the Mach64
class mach64_8514a_device : public mach32_8514a_device
{
public:
	// construction/destruction
	mach64_8514a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	mach64_8514a_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_config_complete() override;
};

// main SVGA device
class mach64_device : public mach32_device
{
public:
	// construction/destruction
	mach64_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_WRITE16_MEMBER(mach64_config1_w) { }  // why does the mach64 BIOS write to these, they are read only on the mach32 and earlier
	DECLARE_WRITE16_MEMBER(mach64_config2_w) { }

protected:
	mach64_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	required_device<mach64_8514a_device> m_8514a;  // provides accelerated 2D drawing, derived from the Mach8 device
};

// device type definition
DECLARE_DEVICE_TYPE(ATIMACH32,       mach32_device)
DECLARE_DEVICE_TYPE(ATIMACH32_8514A, mach32_8514a_device)
DECLARE_DEVICE_TYPE(ATIMACH64,       mach64_device)
DECLARE_DEVICE_TYPE(ATIMACH64_8514A, mach64_8514a_device)

#endif // MAME_BUS_ISA_MACH32_H
