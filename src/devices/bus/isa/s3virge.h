// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * s3virge.h
 *
 * S3 ViRGE 2D/3D video card
 *
 */

#ifndef S3VIRGE_H_
#define S3VIRGE_H_

#include "emu.h"
#include "video/pc_vga.h"

// ======================> s3virge_vga_device

class s3virge_vga_device :  public s3_vga_device
{
public:
	// construction/destruction
	s3virge_vga_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	s3virge_vga_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);

	virtual READ8_MEMBER(port_03b0_r) override;
	virtual WRITE8_MEMBER(port_03b0_w) override;
	virtual READ8_MEMBER(port_03c0_r) override;
	virtual WRITE8_MEMBER(port_03c0_w) override;
	virtual READ8_MEMBER(port_03d0_r) override;
	virtual WRITE8_MEMBER(port_03d0_w) override;
	virtual READ8_MEMBER(mem_r) override;
	virtual WRITE8_MEMBER(mem_w) override;

	ibm8514a_device* get_8514() { fatalerror("s3virge requested non-existant 8514/A device\n"); return nullptr; }
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	virtual UINT8 s3_crtc_reg_read(UINT8 index);
	virtual void s3_define_video_mode(void);
	virtual void s3_crtc_reg_write(UINT8 index, UINT8 data);
	// has no 8514/A device
};


// ======================> s3virgedx_vga_device

class s3virgedx_vga_device :  public s3virge_vga_device
{
public:
	// construction/destruction
	s3virgedx_vga_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	s3virgedx_vga_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
};

// ======================> s3virgedx_vga_device

class s3virgedx_rev1_vga_device :  public s3virgedx_vga_device
{
public:
	// construction/destruction
	s3virgedx_rev1_vga_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
};

// device type definition
extern const device_type S3VIRGE;
extern const device_type S3VIRGEDX;
extern const device_type S3VIRGEDX1;

#endif /* S3VIRGE_H_ */
