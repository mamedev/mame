/***************************************************************************

    pc_vga.h

    PC standard VGA adaptor

***************************************************************************/

#ifndef PC_VGA_H
#define PC_VGA_H

MACHINE_CONFIG_EXTERN( pcvideo_vga );
MACHINE_CONFIG_EXTERN( pcvideo_trident_vga );
MACHINE_CONFIG_EXTERN( pcvideo_gamtor_vga );
MACHINE_CONFIG_EXTERN( pcvideo_cirrus_vga );

// ======================> vga_device

class vga_device :  public device_t
{
public:
    // construction/destruction
    vga_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	vga_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);


	virtual UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	
	virtual READ8_MEMBER(port_03b0_r);
	virtual WRITE8_MEMBER(port_03b0_w);
	virtual READ8_MEMBER(port_03c0_r);
	virtual WRITE8_MEMBER(port_03c0_w);
	virtual READ8_MEMBER(port_03d0_r);
	virtual WRITE8_MEMBER(port_03d0_w);
	virtual READ8_MEMBER(mem_r);
	virtual WRITE8_MEMBER(mem_w);
	virtual READ8_MEMBER(mem_linear_r);
	virtual WRITE8_MEMBER(mem_linear_w);
protected:
    // device-level overrides
    virtual void device_start();
    virtual void device_reset();

	void vga_vh_text(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void vga_vh_ega(bitmap_rgb32 &bitmap,  const rectangle &cliprect);
	void vga_vh_vga(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void vga_vh_cga(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void vga_vh_mono(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	virtual UINT8 pc_vga_choosevideomode();	
	void recompute_params_clock(int divisor, int xtal);
	UINT8 crtc_reg_read(UINT8 index);
	void recompute_params();
	void crtc_reg_write(UINT8 index, UINT8 data);
	void seq_reg_write(UINT8 index, UINT8 data);
	UINT8 vga_vblank();
	READ8_MEMBER(vga_crtc_r);
	WRITE8_MEMBER(vga_crtc_w);
	UINT8 gc_reg_read(UINT8 index);
	void attribute_reg_write(UINT8 index, UINT8 data);
	void gc_reg_write(UINT8 index,UINT8 data);
private:
	inline UINT8 rotate_right(UINT8 val);
	inline UINT8 vga_logical_op(UINT8 data, UINT8 plane, UINT8 mask);
	inline UINT8 vga_latch_write(int offs, UINT8 data);
};


// device type definition
extern const device_type VGA;

// ======================> svga_device

class svga_device :  public vga_device
{
public:
    // construction/destruction
	svga_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);	

	virtual UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
protected:
	void svga_vh_rgb8(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void svga_vh_rgb15(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void svga_vh_rgb16(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void svga_vh_rgb24(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void svga_vh_rgb32(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	virtual UINT8 pc_vga_choosevideomode();
private:
};

// ======================> tseng_vga_device

class tseng_vga_device :  public svga_device
{
public:
    // construction/destruction
    tseng_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual READ8_MEMBER(port_03b0_r);
	virtual WRITE8_MEMBER(port_03b0_w);
	virtual READ8_MEMBER(port_03c0_r);
	virtual WRITE8_MEMBER(port_03c0_w);
	virtual READ8_MEMBER(port_03d0_r);
	virtual WRITE8_MEMBER(port_03d0_w);
	virtual READ8_MEMBER(mem_r);
	virtual WRITE8_MEMBER(mem_w);

protected:

private:
	void tseng_define_video_mode();
	UINT8 tseng_crtc_reg_read(UINT8 index);
	void tseng_crtc_reg_write(UINT8 index, UINT8 data);
	UINT8 tseng_seq_reg_read(UINT8 index);
	void tseng_seq_reg_write(UINT8 index, UINT8 data);

};


// device type definition
extern const device_type TSENG_VGA;

// ======================> trident_vga_device

class trident_vga_device :  public svga_device
{
public:
    // construction/destruction
    trident_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual READ8_MEMBER(port_03c0_r);
	virtual WRITE8_MEMBER(port_03c0_w);
	virtual READ8_MEMBER(port_03d0_r);
	virtual WRITE8_MEMBER(port_03d0_w);
	virtual READ8_MEMBER(mem_r);
	virtual WRITE8_MEMBER(mem_w);

protected:

private:
	UINT8 trident_seq_reg_read(UINT8 index);
	void trident_seq_reg_write(UINT8 index, UINT8 data);

};


// device type definition
extern const device_type TRIDENT_VGA;


// ======================> s3_vga_device

class s3_vga_device :  public svga_device
{
public:
    // construction/destruction
    s3_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	s3_vga_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);

	virtual READ8_MEMBER(port_03b0_r);
	virtual WRITE8_MEMBER(port_03b0_w);
	virtual READ8_MEMBER(port_03c0_r);
	virtual WRITE8_MEMBER(port_03c0_w);
	virtual READ8_MEMBER(port_03d0_r);
	virtual WRITE8_MEMBER(port_03d0_w);
	virtual READ8_MEMBER(mem_r);
	virtual WRITE8_MEMBER(mem_w);
	
	virtual UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	
	READ16_MEMBER(s3_line_error_r);
	WRITE16_MEMBER(s3_line_error_w);
	READ16_MEMBER(ibm8514_gpstatus_r);
	READ16_MEMBER(s3_gpstatus_r);
	WRITE16_MEMBER(ibm8514_cmd_w);
	WRITE16_MEMBER(s3_cmd_w);
	READ16_MEMBER(ibm8514_desty_r);
	WRITE16_MEMBER(ibm8514_desty_w);
	READ16_MEMBER(ibm8514_destx_r);
	WRITE16_MEMBER(ibm8514_destx_w);
	READ16_MEMBER(ibm8514_ssv_r);
	WRITE16_MEMBER(ibm8514_ssv_w);
	READ16_MEMBER(s3_width_r);
	WRITE16_MEMBER(s3_width_w);
	READ16_MEMBER(ibm8514_currentx_r);
	WRITE16_MEMBER(ibm8514_currentx_w);
	READ16_MEMBER(ibm8514_currenty_r);
	WRITE16_MEMBER(ibm8514_currenty_w);
	READ16_MEMBER(s3_fgcolour_r);
	WRITE16_MEMBER(s3_fgcolour_w);
	READ16_MEMBER(s3_bgcolour_r);
	WRITE16_MEMBER(s3_bgcolour_w);
	READ16_MEMBER(s3_multifunc_r);
	WRITE16_MEMBER(s3_multifunc_w);
	READ16_MEMBER(s3_backmix_r);
	WRITE16_MEMBER(s3_backmix_w);
	READ16_MEMBER(s3_foremix_r);
	WRITE16_MEMBER(s3_foremix_w);
	READ16_MEMBER(s3_pixel_xfer_r);
	WRITE16_MEMBER(s3_pixel_xfer_w);
	
protected:
    // device-level overrides
    virtual void device_start();

private:
	UINT8 s3_crtc_reg_read(UINT8 index);
	void s3_define_video_mode(void);
	void s3_crtc_reg_write(UINT8 index, UINT8 data);
	void s3_write_fg(UINT32 offset);
	void s3_write_bg(UINT32 offset);
	void s3_write(UINT32 offset, UINT32 src);
	void ibm8514_draw_vector(UINT8 len, UINT8 dir, bool draw);
	void ibm8514_wait_draw_ssv();
	void ibm8514_draw_ssv(UINT8 data);
	void ibm8514_wait_draw_vector();
	void s3_wait_draw();
};


// device type definition
extern const device_type S3_VGA;


// ======================> gamtor_vga_device

class gamtor_vga_device :  public svga_device
{
public:
    // construction/destruction
    gamtor_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);


	virtual READ8_MEMBER(port_03b0_r);
	virtual WRITE8_MEMBER(port_03b0_w);
	virtual READ8_MEMBER(port_03c0_r);
	virtual WRITE8_MEMBER(port_03c0_w);
	virtual READ8_MEMBER(port_03d0_r);
	virtual WRITE8_MEMBER(port_03d0_w);
	virtual READ8_MEMBER(mem_r);
	virtual WRITE8_MEMBER(mem_w);
	
protected:
private:
};


// device type definition
extern const device_type GAMTOR_VGA;

// ======================> ati_vga_device

class ati_vga_device :  public s3_vga_device
{
public:
    // construction/destruction
    ati_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual READ8_MEMBER(mem_r);
	virtual WRITE8_MEMBER(mem_w);
	virtual READ8_MEMBER(port_03c0_r);
	READ8_MEMBER(ati_port_ext_r);
	WRITE8_MEMBER(ati_port_ext_w);
	READ16_MEMBER(ibm8514_status_r);
	WRITE16_MEMBER(ibm8514_htotal_w);
	READ16_MEMBER(ibm8514_substatus_r);
	WRITE16_MEMBER(ibm8514_subcontrol_w);
	READ16_MEMBER(ibm8514_subcontrol_r);
	READ16_MEMBER(ibm8514_htotal_r);
	READ16_MEMBER(ibm8514_vtotal_r);
	WRITE16_MEMBER(ibm8514_vtotal_w);
	READ16_MEMBER(ibm8514_vdisp_r);
	WRITE16_MEMBER(ibm8514_vdisp_w);
	READ16_MEMBER(ibm8514_vsync_r);
	WRITE16_MEMBER(ibm8514_vsync_w);
	READ16_MEMBER(mach8_ec0_r);
	WRITE16_MEMBER(mach8_ec0_w);
	READ16_MEMBER(mach8_ec1_r);
	WRITE16_MEMBER(mach8_ec1_w);
	READ16_MEMBER(mach8_ec2_r);
	WRITE16_MEMBER(mach8_ec2_w);
	READ16_MEMBER(mach8_ec3_r);
	WRITE16_MEMBER(mach8_ec3_w);
	READ16_MEMBER(mach8_ext_fifo_r);
	WRITE16_MEMBER(mach8_linedraw_index_w);
	READ16_MEMBER(mach8_bresenham_count_r);
	WRITE16_MEMBER(mach8_bresenham_count_w);
	WRITE16_MEMBER(mach8_linedraw_w);
	READ16_MEMBER(mach8_scratch0_r);
	WRITE16_MEMBER(mach8_scratch0_w);
	READ16_MEMBER(mach8_scratch1_r);
	WRITE16_MEMBER(mach8_scratch1_w);
	READ16_MEMBER(mach8_config1_r);
	READ16_MEMBER(mach8_config2_r);	
protected:
   virtual machine_config_constructor device_mconfig_additions() const;
private:
	void ati_define_video_mode();

};


// device type definition
extern const device_type ATI_VGA;

// ======================> cirrus_vga_device

class cirrus_vga_device :  public svga_device
{
public:
    // construction/destruction
    cirrus_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual READ8_MEMBER(port_03c0_r);
	virtual WRITE8_MEMBER(port_03c0_w);
protected:
	// device-level overrides
    virtual void device_start();
private:
	void cirrus_define_video_mode();
	UINT8 cirrus_seq_reg_read(UINT8 index);
	void cirrus_seq_reg_write(UINT8 index, UINT8 data);
};

// device type definition
extern const device_type CIRRUS_VGA;
/*
  pega notes (paradise)
  build in amstrad pc1640

  ROM_LOAD("40100", 0xc0000, 0x8000, CRC(d2d1f1ae))

  4 additional dipswitches
  seems to have emulation modes at register level
  (mda/hgc lines bit 8 not identical to ega/vga)

  standard ega/vga dipswitches
  00000000  320x200
  00000001  640x200 hanging
  00000010  640x200 hanging
  00000011  640x200 hanging

  00000100  640x350 hanging
  00000101  640x350 hanging EGA mono
  00000110  320x200
  00000111  640x200

  00001000  640x200
  00001001  640x200
  00001010  720x350 partial visible
  00001011  720x350 partial visible

  00001100  320x200
  00001101  320x200
  00001110  320x200
  00001111  320x200

*/

/*
  oak vga (oti 037 chip)
  (below bios patch needed for running)

  ROM_LOAD("oakvga.bin", 0xc0000, 0x8000, CRC(318c5f43))
*/

#endif /* PC_VGA_H */

