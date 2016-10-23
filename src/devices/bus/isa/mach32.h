// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * mach32.h
 *
 *  Created on: 16/05/2014
 */

#ifndef MACH32_H_
#define MACH32_H_

#include "emu.h"
#include "video/pc_vga.h"
#include "machine/eepromser.h"

// 8514/A module of the Mach32
class mach32_8514a_device : public mach8_device
{
public:
	// construction/destruction
	mach32_8514a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	mach32_8514a_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

	uint16_t mach32_chipid_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) { return m_chip_ID; }
	void mach32_clksel_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) { mach8.clksel = data; }  // read only on the mach8
	uint16_t mach32_mem_boundary_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) { return m_membounds; }
	void mach32_mem_boundary_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) { m_membounds = data; if(data & 0x10) logerror("ATI: Unimplemented memory boundary activated."); }

protected:
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
	mach32_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

	required_device<mach32_8514a_device> m_8514a;  // provides accelerated 2D drawing, derived from the Mach8 device

	// map 8514/A functions to 8514/A module
	uint16_t mach8_ec0_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) { return m_8514a->mach8_ec0_r(space,offset,mem_mask); }
	void mach8_ec0_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) { m_8514a->mach8_ec0_w(space,offset,data,mem_mask); }
	uint16_t mach8_ec1_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) { return m_8514a->mach8_ec1_r(space,offset,mem_mask); }
	void mach8_ec1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) { m_8514a->mach8_ec1_w(space,offset,data,mem_mask); }
	uint16_t mach8_ec2_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) { return m_8514a->mach8_ec2_r(space,offset,mem_mask); }
	void mach8_ec2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) { m_8514a->mach8_ec2_w(space,offset,data,mem_mask); }
	uint16_t mach8_ec3_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) { return m_8514a->mach8_ec3_r(space,offset,mem_mask); }
	void mach8_ec3_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) { m_8514a->mach8_ec3_w(space,offset,data,mem_mask); }
	uint16_t mach8_ext_fifo_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) { return m_8514a->mach8_ext_fifo_r(space,offset,mem_mask); }
	void mach8_linedraw_index_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) { m_8514a->mach8_linedraw_index_w(space,offset,data,mem_mask); }
	uint16_t mach8_bresenham_count_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) { return m_8514a->mach8_bresenham_count_r(space,offset,mem_mask); }
	void mach8_bresenham_count_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) { m_8514a->mach8_bresenham_count_w(space,offset,data,mem_mask); }
	void mach8_linedraw_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) { m_8514a->mach8_linedraw_w(space,offset,data,mem_mask); }
	uint16_t mach8_linedraw_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) { return m_8514a->mach8_linedraw_r(space,offset,mem_mask); }
	uint16_t mach8_scratch0_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) { return m_8514a->mach8_scratch0_r(space,offset,mem_mask); }
	void mach8_scratch0_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) { m_8514a->mach8_scratch0_w(space,offset,data,mem_mask); }
	uint16_t mach8_scratch1_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) { return m_8514a->mach8_scratch1_r(space,offset,mem_mask); }
	void mach8_scratch1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) { m_8514a->mach8_scratch1_w(space,offset,data,mem_mask); }
	uint16_t mach8_config1_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) { return m_8514a->mach8_config1_r(space,offset,mem_mask); }
	uint16_t mach8_config2_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) { return m_8514a->mach8_config2_r(space,offset,mem_mask); }
	uint16_t mach8_sourcex_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) { return m_8514a->mach8_sourcex_r(space,offset,mem_mask); }
	uint16_t mach8_sourcey_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) { return m_8514a->mach8_sourcey_r(space,offset,mem_mask); }
	void mach8_ext_leftscissor_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) { m_8514a->mach8_ext_leftscissor_w(space,offset,data,mem_mask); }
	void mach8_ext_topscissor_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) { m_8514a->mach8_ext_topscissor_w(space,offset,data,mem_mask); }

	uint16_t ibm8514_vtotal_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) { return m_8514a->ibm8514_vtotal_r(space,offset,mem_mask); }
	void ibm8514_vtotal_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) { m_8514a->ibm8514_vtotal_w(space,offset,data,mem_mask); }
	uint16_t ibm8514_htotal_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) { return m_8514a->ibm8514_htotal_r(space,offset,mem_mask); }
	void ibm8514_htotal_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) { m_8514a->ibm8514_htotal_w(space,offset,data,mem_mask); }
	uint16_t ibm8514_vdisp_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) { return m_8514a->ibm8514_vdisp_r(space,offset,mem_mask); }
	void ibm8514_vdisp_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) { m_8514a->ibm8514_vdisp_w(space,offset,data,mem_mask); }
	uint16_t ibm8514_vsync_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) { return m_8514a->ibm8514_vsync_r(space,offset,mem_mask); }
	void ibm8514_vsync_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) { m_8514a->ibm8514_vsync_w(space,offset,data,mem_mask); }
	uint16_t ibm8514_substatus_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) { return m_8514a->ibm8514_substatus_r(space,offset,mem_mask); }
	void ibm8514_subcontrol_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) { m_8514a->ibm8514_subcontrol_w(space,offset,data,mem_mask); }
	uint16_t ibm8514_subcontrol_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) { return m_8514a->ibm8514_subcontrol_r(space,offset,mem_mask); }
	uint16_t ibm8514_currentx_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) { return m_8514a->ibm8514_currentx_r(space,offset,mem_mask); }
	void ibm8514_currentx_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) { m_8514a->ibm8514_currentx_w(space,offset,data,mem_mask); }
	uint16_t ibm8514_currenty_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) { return m_8514a->ibm8514_currenty_r(space,offset,mem_mask); }
	void ibm8514_currenty_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) { m_8514a->ibm8514_currenty_w(space,offset,data,mem_mask); }
	uint16_t ibm8514_desty_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) { return m_8514a->ibm8514_desty_r(space,offset,mem_mask); }
	void ibm8514_desty_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) { m_8514a->ibm8514_desty_w(space,offset,data,mem_mask); }
	uint16_t ibm8514_destx_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) { return m_8514a->ibm8514_destx_r(space,offset,mem_mask); }
	void ibm8514_destx_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) { m_8514a->ibm8514_destx_w(space,offset,data,mem_mask); }
	uint16_t ibm8514_line_error_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) { return m_8514a->ibm8514_line_error_r(space,offset,mem_mask); }
	void ibm8514_line_error_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) { m_8514a->ibm8514_line_error_w(space,offset,data,mem_mask); }
	uint16_t ibm8514_width_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) { return m_8514a->ibm8514_width_r(space,offset,mem_mask); }
	void ibm8514_width_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) { m_8514a->ibm8514_width_w(space,offset,data,mem_mask); }
	uint16_t ibm8514_gpstatus_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) { return m_8514a->ibm8514_gpstatus_r(space,offset,mem_mask); }
	void ibm8514_cmd_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) { m_8514a->ibm8514_cmd_w(space,offset,data,mem_mask); }
	uint16_t ibm8514_ssv_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) { return m_8514a->ibm8514_ssv_r(space,offset,mem_mask); }
	void ibm8514_ssv_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) { m_8514a->ibm8514_ssv_w(space,offset,data,mem_mask); }
	uint16_t ibm8514_fgcolour_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) { return m_8514a->ibm8514_fgcolour_r(space,offset,mem_mask); }
	void ibm8514_fgcolour_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) { m_8514a->ibm8514_fgcolour_w(space,offset,data,mem_mask); }
	uint16_t ibm8514_bgcolour_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) { return m_8514a->ibm8514_bgcolour_r(space,offset,mem_mask); }
	void ibm8514_bgcolour_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) { m_8514a->ibm8514_bgcolour_w(space,offset,data,mem_mask); }
	uint16_t ibm8514_read_mask_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) { return m_8514a->ibm8514_read_mask_r(space,offset,mem_mask); }
	void ibm8514_read_mask_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) { m_8514a->ibm8514_read_mask_w(space,offset,data,mem_mask); }
	uint16_t ibm8514_write_mask_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) { return m_8514a->ibm8514_write_mask_r(space,offset,mem_mask); }
	void ibm8514_write_mask_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) { m_8514a->ibm8514_write_mask_w(space,offset,data,mem_mask); }
	uint16_t ibm8514_backmix_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) { return m_8514a->ibm8514_backmix_r(space,offset,mem_mask); }
	void ibm8514_backmix_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) { m_8514a->ibm8514_backmix_w(space,offset,data,mem_mask); }
	uint16_t ibm8514_foremix_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) { return m_8514a->ibm8514_foremix_r(space,offset,mem_mask); }
	void ibm8514_foremix_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) { m_8514a->ibm8514_foremix_w(space,offset,data,mem_mask); }
	uint16_t ibm8514_multifunc_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) { return m_8514a->ibm8514_multifunc_r(space,offset,mem_mask); }
	void ibm8514_multifunc_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) { m_8514a->ibm8514_multifunc_w(space,offset,data,mem_mask); }
	uint16_t ibm8514_pixel_xfer_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) { return m_8514a->ibm8514_pixel_xfer_r(space,offset,mem_mask); }
	void ibm8514_pixel_xfer_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) { m_8514a->ibm8514_pixel_xfer_w(space,offset,data,mem_mask); }

	uint16_t mach32_chipid_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) { return m_8514a->mach32_chipid_r(space,offset,mem_mask);  }
	uint16_t mach8_clksel_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) { return m_8514a->mach8_clksel_r(space,offset,mem_mask); }
	void mach32_clksel_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) { m_8514a->mach32_clksel_w(space,offset,data,mem_mask); }  // read only on the mach8
	uint16_t mach32_mem_boundary_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) { return m_8514a->mach32_mem_boundary_r(space,offset,mem_mask); }
	void mach32_mem_boundary_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) { m_8514a->mach32_mem_boundary_w(space,offset,data,mem_mask); }  // read only on the mach8
	uint16_t mach32_status_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) { return vga_vblank() << 1; }
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;
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
	mach64_8514a_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

protected:
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
	mach64_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

	required_device<mach64_8514a_device> m_8514a;  // provides accelerated 2D drawing, derived from the Mach8 device

	void mach64_config1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) {  }  // why does the mach64 BIOS write to these, they are read only on the mach32 and earlier
	void mach64_config2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) {  }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;
};
// device type definition
extern const device_type ATIMACH32;
extern const device_type ATIMACH32_8514A;
extern const device_type ATIMACH64;
extern const device_type ATIMACH64_8514A;

#endif /* MACH32_H_ */
