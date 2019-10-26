// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * s3virge.h
 *
 * S3 ViRGE 2D/3D video card
 *
 */

#ifndef MAME_BUS_ISA_S3VIRGE_H
#define MAME_BUS_ISA_S3VIRGE_H

#pragma once

#include "video/pc_vga.h"

// ======================> s3virge_vga_device

class s3virge_vga_device :  public s3_vga_device
{
public:
	// construction/destruction
	s3virge_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual READ8_MEMBER(port_03b0_r) override;
	virtual WRITE8_MEMBER(port_03b0_w) override;
	virtual READ8_MEMBER(port_03c0_r) override;
	virtual WRITE8_MEMBER(port_03c0_w) override;
	virtual READ8_MEMBER(port_03d0_r) override;
	virtual WRITE8_MEMBER(port_03d0_w) override;
	virtual READ8_MEMBER(mem_r) override;
	virtual WRITE8_MEMBER(mem_w) override;

	DECLARE_READ8_MEMBER(fb_r);
	DECLARE_WRITE8_MEMBER(fb_w);
	DECLARE_READ32_MEMBER(s3d_sub_status_r);
	DECLARE_WRITE32_MEMBER(s3d_sub_control_w);

	DECLARE_READ32_MEMBER(s3d_register_r);
	DECLARE_WRITE32_MEMBER(s3d_register_w);

	uint16_t get_crtc_port() { return (vga.miscellaneous_output&1)?0x3d0:0x3b0; }
	uint32_t get_linear_address() { return s3virge.linear_address; }
	void set_linear_address(uint32_t addr) { s3virge.linear_address = addr; }
	uint8_t get_linear_address_size() { return s3virge.linear_address_size; }
	bool is_linear_address_active() { return s3virge.linear_address_enable; }
	bool is_new_mmio_active() { return s3.cr53 & 0x08; }

	ibm8514a_device* get_8514() { fatalerror("s3virge requested non-existent 8514/A device\n"); return nullptr; }

protected:
	s3virge_vga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	enum
	{
		LAW_64K = 0,
		LAW_1MB,
		LAW_2MB,
		LAW_4MB
	};

	enum
	{
		OP_BITBLT = 0,
		OP_2DLINE,
		OP_2DPOLY,
		OP_3DLINE,
		OP_3DTRI
	};

	struct
	{
		uint32_t linear_address;
		uint8_t linear_address_size;
		uint32_t linear_address_size_full;
		bool linear_address_enable;
		uint32_t interrupt_enable;

		struct
		{
			uint32_t src_base[5];
			uint32_t dest_base[5];
			uint32_t command[5];
			uint16_t source_x[5];
			uint16_t source_y[5];
			uint16_t dest_x[5];
			uint16_t dest_y[5];
			uint16_t rect_width[5];
			uint16_t rect_height[5];
		} s3d;
	} s3virge;

private:
	virtual uint8_t s3_crtc_reg_read(uint8_t index);
	virtual void s3_define_video_mode(void);
	virtual void s3_crtc_reg_write(uint8_t index, uint8_t data);
	// has no 8514/A device
};


// ======================> s3virgedx_vga_device

class s3virgedx_vga_device :  public s3virge_vga_device
{
public:
	// construction/destruction
	s3virgedx_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	s3virgedx_vga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
};

// ======================> s3virgedx_vga_device

class s3virgedx_rev1_vga_device :  public s3virgedx_vga_device
{
public:
	// construction/destruction
	s3virgedx_rev1_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
};

// device type definition
DECLARE_DEVICE_TYPE(S3VIRGE,    s3virge_vga_device)
DECLARE_DEVICE_TYPE(S3VIRGEDX,  s3virgedx_vga_device)
DECLARE_DEVICE_TYPE(S3VIRGEDX1, s3virgedx_rev1_vga_device)

#endif // MAME_BUS_ISA_S3VIRGE_H
