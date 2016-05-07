// license:BSD-3-Clause
// copyright-holders:Darius Goad

#ifndef NV4_H_
#define NV4_H_

#include "emu.h"
#include "video/pc_vga.h"

// ======================> nv4_vga_device

class nv4_vga_device :  public svga_device
{
public:
	// construction/destruction
	nv4_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	nv4_vga_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	virtual READ8_MEMBER(port_03b0_r) override;
	virtual WRITE8_MEMBER(port_03b0_w) override;
	virtual READ8_MEMBER(port_03d0_r) override;
	virtual WRITE8_MEMBER(port_03d0_w) override;
	virtual READ8_MEMBER(mem_r) override;
	virtual WRITE8_MEMBER(mem_w) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	virtual UINT8 nv4_crtc_reg_read(UINT8 index);
	virtual void nv4_define_video_mode(void);
	virtual void nv4_crtc_reg_write(UINT8 index, UINT8 data);

	virtual UINT32 nv4_pramdac_read(UINT32 addr);
	virtual void nv4_pramdac_write(UINT32 addr, UINT32 data);

	virtual UINT32 nv4_mmio_read(UINT32 addr);
	virtual void nv4_mmio_write(UINT32 addr, UINT32 data);

	virtual UINT8 nv4_rma_read(int index);
	virtual void nv4_rma_write(int index, UINT8 data);

	struct
	{
		struct
		{
			UINT32 nvpll, mpll, vpll;
		} pramdac;
		struct
		{
			bool scl, sda;
		} i2c;
		struct
		{
			UINT8 mode;
			UINT32 addr;
			UINT32 data;
		} rma;
	} nv4;
};

// device type definition
extern const device_type NV4;

#endif /* S3VIRGE_H_ */
