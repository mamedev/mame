// license:LGPL-2.1+
// copyright-holders:David Haywood, Angelo Salese, Olivier Galibert, Mariusz Wojcieszek, R. Belmont
/**************************************************************************************************

Sega Saturn / ST-V - VDP1

**************************************************************************************************/

#ifndef MAME_VIDEO_SATURN_VDP1_H
#define MAME_VIDEO_SATURN_VDP1_H

#pragma once


#define RGB_R(_color)   (_color & 0x1f)
#define RGB_G(_color)   ((_color >> 5) & 0x1f)
#define RGB_B(_color)   ((_color >> 10) & 0x1f)

#define SWAP_INT32(_a,_b) \
	{ \
		int32_t t; \
		t = _a; \
		_a = _b; \
		_b = t; \
	}

#define SWAP_INT32PTR(_p1, _p2) \
	{ \
		int32_t *p; \
		p = _p1; \
		_p1 = _p2; \
		_p2 = p; \
	}

/*TV Mode Selection Register */
/*
   xxxx xxxx xxxx ---- | UNUSED
   ---- ---- ---- x--- | VBlank Erase/Write (VBE)
   ---- ---- ---- -xxx | TV Mode (TVM)
   TV-Mode:
   This sets the Frame Buffer size,the rotation of the Frame Buffer & the bit width.
   bit 2 HDTV disable(0)/enable(1)
   bit 1 non-rotation/rotation(1)
   bit 0 16(0)/8(1) bits per pixel
   Size of the Frame Buffer:
   7 invalid
   6 invalid
   5 invalid
   4 512x256
   3 512x512
   2 512x256
   1 1024x256
   0 512x256
*/
#define STV_VDP1_TVMR ((m_vdp1_regs[0x000/2])&0xffff)
#define STV_VDP1_VBE  ((STV_VDP1_TVMR & 0x0008) >> 3)
#define STV_VDP1_TVM  ((STV_VDP1_TVMR & 0x0007) >> 0)

/*Frame Buffer Change Mode Register*/
/*
   xxxx xxxx xxx- ---- | UNUSED
   ---- ---- ---x ---- | Even/Odd Coordinate Select Bit (EOS)
   ---- ---- ---- x--- | Double Interlace Mode (DIE)
   ---- ---- ---- -x-- | Double Interlace Draw Line (DIL)
   ---- ---- ---- --x- | Frame Buffer Change Trigger (FCM)
   ---- ---- ---- ---x | Frame Buffer Change Mode (FCT)
*/
#define STV_VDP1_FBCR ((m_vdp1_regs[0x002/2] >> 0)&0xffff)
#define STV_VDP1_EOS ((STV_VDP1_FBCR & 0x0010) >> 4)
#define STV_VDP1_DIE ((STV_VDP1_FBCR & 0x0008) >> 3)
#define STV_VDP1_DIL ((STV_VDP1_FBCR & 0x0004) >> 2)
#define STV_VDP1_FCM ((STV_VDP1_FBCR & 0x0002) >> 1)
#define STV_VDP1_FCT ((STV_VDP1_FBCR & 0x0001) >> 0)

/*Plot Trigger Register*/
/*
   xxxx xxxx xxxx xx-- | UNUSED
   ---- ---- ---- --xx | Plot Trigger Mode (PTM)

   Plot Trigger Mode:
   3 Invalid
   2 Automatic draw
   1 VDP1 draw by request
   0 VDP1 Idle (no access)
*/
#define STV_VDP1_PTMR ((m_vdp1_regs[0x004/2])&0xffff)
#define STV_VDP1_PTM  ((STV_VDP1_PTMR & 0x0003) >> 0)
#define PTM_0         m_vdp1_regs[0x004/2]&=~0x0001

/*
    Erase/Write Data Register
    16 bpp = data
    8 bpp = erase/write data for even/odd X coordinates
*/
#define STV_VDP1_EWDR ((m_vdp1_regs[0x006/2])&0xffff)

/*Erase/Write Upper-Left register*/
/*
   x--- ---- ---- ---- | UNUSED
   -xxx xxx- ---- ---- | X1 register
   ---- ---x xxxx xxxx | Y1 register

*/
#define STV_VDP1_EWLR ((m_vdp1_regs[0x008/2])&0xffff)
#define STV_VDP1_EWLR_X1 ((STV_VDP1_EWLR & 0x7e00) >> 9)
#define STV_VDP1_EWLR_Y1 ((STV_VDP1_EWLR & 0x01ff) >> 0)
/*Erase/Write Lower-Right register*/
/*
   xxxx xxx- ---- ---- | X3 register
   ---- ---x xxxx xxxx | Y3 register

*/
#define STV_VDP1_EWRR ((m_vdp1_regs[0x00a/2])&0xffff)
#define STV_VDP1_EWRR_X3 ((STV_VDP1_EWRR & 0xfe00) >> 9)
#define STV_VDP1_EWRR_Y3 ((STV_VDP1_EWRR & 0x01ff) >> 0)
/*Transfer End Status Register*/
/*
   xxxx xxxx xxxx xx-- | UNUSED
   ---- ---- ---- --x- | CEF
   ---- ---- ---- ---x | BEF

*/
#define STV_VDP1_EDSR ((m_vdp1_regs[0x010/2])&0xffff)
#define STV_VDP1_CEF  (STV_VDP1_EDSR & 2)
#define STV_VDP1_BEF  (STV_VDP1_EDSR & 1)
/**/
#define CEF_1   m_vdp1_regs[0x010/2]|=0x0002
#define CEF_0   m_vdp1_regs[0x010/2]&=~0x0002
#define BEF_1   m_vdp1_regs[0x010/2]|=0x0001
#define BEF_0   m_vdp1_regs[0x010/2]&=~0x0001

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class saturn_vdp1_device : public device_t
{
public:
	// construction/destruction
	saturn_vdp1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template<class T> void set_hostcpu(T &&tag) { m_host_cpu.set_tag(std::forward<T>(tag)); }

	// I/O operations
	void regs_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 regs_r(offs_t offset);
	void vram_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 vram_r(offs_t offset);
	u32 framebuffer0_r(offs_t offset, u32 mem_mask = ~0);
	void framebuffer0_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	// public helpers for VDP2 interface
	uint16_t *read_fb_display_lines(int y);
	bool read_fb_double_interlace_mode();
	void video_update( void );
	bool is_tvm_zero();

	void vram_clear();
	void check_fb_clear();

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_device <cpu_device> m_host_cpu;

	std::unique_ptr<uint16_t * []> framebuffer_display_lines;
	int       framebuffer_mode;
	int       framebuffer_double_interlace;
	int       fbcr_accessed;
	int       framebuffer_width;
	int       framebuffer_height;
	int       framebuffer_current_display;
	int       framebuffer_current_draw;
	int       framebuffer_clear_on_next_frame;
	rectangle system_cliprect;
	rectangle user_cliprect;
	std::unique_ptr<uint16_t []> framebuffer[2];
	std::unique_ptr<uint16_t * []> framebuffer_draw_lines;
	std::unique_ptr<uint8_t []> gfx_decode;
	uint16_t    lopr;
	uint16_t    copr;
	uint16_t    ewdr;

	int       local_x;
	int       local_y;

	std::unique_ptr<uint32_t[]>    m_vdp1_vram;
	std::unique_ptr<uint16_t[]>    m_vdp1_regs;
	
	void stv_set_framebuffer_config( void );
	void stv_prepare_framebuffers( void );
	void stv_vdp1_change_framebuffers( void );
	void stv_vdp1_process_list( void );
	void stv_vdp1_set_drawpixel( void );

	void stv_vdp1_draw_normal_sprite(const rectangle &cliprect, int sprite_type);
	void stv_vdp1_draw_scaled_sprite(const rectangle &cliprect);
	void stv_vdp1_draw_distorted_sprite(const rectangle &cliprect);
	void stv_vdp1_draw_poly_line(const rectangle &cliprect);
	void stv_vdp1_draw_line(const rectangle &cliprect);
	int x2s(int v);
	int y2s(int v);
	void vdp1_fill_quad(const rectangle &cliprect, int patterndata, int xsize, const struct spoint *q);
	void vdp1_fill_line(const rectangle &cliprect, int patterndata, int xsize, int32_t y, int32_t x1, int32_t x2, int32_t u1, int32_t u2, int32_t v1, int32_t v2);
	void (saturn_vdp1_device::*drawpixel)(int x, int y, int patterndata, int offsetcnt);
	void drawpixel_poly(int x, int y, int patterndata, int offsetcnt);
	void drawpixel_8bpp_trans(int x, int y, int patterndata, int offsetcnt);
	void drawpixel_4bpp_notrans(int x, int y, int patterndata, int offsetcnt);
	void drawpixel_4bpp_trans(int x, int y, int patterndata, int offsetcnt);
	void drawpixel_generic(int x, int y, int patterndata, int offsetcnt);
	void vdp1_fill_slope(const rectangle &cliprect, int patterndata, int xsize,
							int32_t x1, int32_t x2, int32_t sl1, int32_t sl2, int32_t *nx1, int32_t *nx2,
							int32_t u1, int32_t u2, int32_t slu1, int32_t slu2, int32_t *nu1, int32_t *nu2,
							int32_t v1, int32_t v2, int32_t slv1, int32_t slv2, int32_t *nv1, int32_t *nv2,
							int32_t _y1, int32_t y2);
	void stv_vdp1_setup_shading_for_line(int32_t y, int32_t x1, int32_t x2,
												int32_t r1, int32_t g1, int32_t b1,
												int32_t r2, int32_t g2, int32_t b2);
	void stv_vdp1_setup_shading_for_slope(
							int32_t x1, int32_t x2, int32_t sl1, int32_t sl2, int32_t *nx1, int32_t *nx2,
							int32_t r1, int32_t r2, int32_t slr1, int32_t slr2, int32_t *nr1, int32_t *nr2,
							int32_t g1, int32_t g2, int32_t slg1, int32_t slg2, int32_t *ng1, int32_t *ng2,
							int32_t b1, int32_t b2, int32_t slb1, int32_t slb2, int32_t *nb1, int32_t *nb2,
							int32_t _y1, int32_t y2);
	uint16_t stv_vdp1_apply_gouraud_shading( int x, int y, uint16_t pix );
	void stv_vdp1_setup_shading(const struct spoint* q, const rectangle &cliprect);
	uint8_t stv_read_gouraud_table( void );
	void stv_clear_gouraud_shading(void);

	void stv_clear_framebuffer( int which_framebuffer );
	void stv_vdp1_state_save_postload( void );

	struct stv_vdp1_poly_scanline
	{
		int32_t   x[2];
		int32_t   b[2];
		int32_t   g[2];
		int32_t   r[2];
		int32_t   db;
		int32_t   dg;
		int32_t   dr;
	};

	struct stv_vdp1_poly_scanline_data
	{
		int32_t   sy, ey;
		struct  stv_vdp1_poly_scanline scanline[512];
	};

	std::unique_ptr<struct stv_vdp1_poly_scanline_data> stv_vdp1_shading_data;
	
	struct vdp1_sprite_list
	{
		int CMDCTRL, CMDLINK, CMDPMOD, CMDCOLR, CMDSRCA, CMDSIZE, CMDGRDA;
		int CMDXA, CMDYA;
		int CMDXB, CMDYB;
		int CMDXC, CMDYC;
		int CMDXD, CMDYD;

		int ispoly;

	} current_sprite;

	/* Gouraud shading */

	struct _stv_gouraud_shading
	{
		/* Gouraud shading table */
		uint16_t  GA;
		uint16_t  GB;
		uint16_t  GC;
		uint16_t  GD;
	} stv_gouraud_shading;
	
	uint16_t m_sprite_colorbank;

	TIMER_CALLBACK_MEMBER(vdp1_draw_end);
};


// device type definition
DECLARE_DEVICE_TYPE(SATURN_VDP1, saturn_vdp1_device)

#endif // MAME_VIDEO_SATURN_VDP1_H
