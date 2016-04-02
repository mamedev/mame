// license:BSD-3-Clause
// copyright-holders:David Haywood
/* */

/* todo: remove code from header, linker starts throwing silly messages when I try due to the templates.. make sense of them */

#pragma once
#ifndef __K05324x_H__
#define __K05324x_H__

#define NORMAL_PLANE_ORDER 4

typedef device_delegate<void (int *code, int *color, int *priority_mask)> k053247_cb_delegate;
#define K053246_CB_MEMBER(_name)   void _name(int *code, int *color, int *priority_mask)
#define K055673_CB_MEMBER(_name)   void _name(int *code, int *color, int *priority_mask)

#define MCFG_K053246_CB(_class, _method) \
	k053247_device::set_k053247_callback(*device, k053247_cb_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#define MCFG_K053246_CONFIG(_gfx_reg, _order, _dx, _dy) \
	k053247_device::set_config(*device, _gfx_reg, _order, _dx, _dy);

#define MCFG_K055673_CB(_class, _method) \
	k053247_device::set_k053247_callback(*device, k053247_cb_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#define MCFG_K055673_CONFIG(_gfx_reg, _order, _dx, _dy) \
	k053247_device::set_config(*device, _gfx_reg, _order, _dx, _dy);


/**  Konami 053246 / 053247 / 055673  **/
#define K055673_LAYOUT_GX  5
#define K055673_LAYOUT_RNG 4
#define K055673_LAYOUT_LE2 8
#define K055673_LAYOUT_GX6 6


/*
Callback procedures for non-standard shadows:

1) translate shadow code to the correct 2-bit form (0=off, 1-3=style)
2) shift shadow code left by K053247_SHDSHIFT and add the K053247_CUSTOMSHADOW flag
3) combine the result with sprite color
*/
#define K053247_CUSTOMSHADOW    0x20000000
#define K053247_SHDSHIFT        20

#ifdef GX_DEBUG
	#define GX_ZBUFW     512
	#define GX_ZBUFH     384
	#define GX_ZPAGESIZE 0x300000
	#define GX_ZBUFSIZE  0x600000
#else
	#define GX_ZBUFW     576
	#define GX_ZBUFH     224
	#define GX_ZPAGESIZE (GX_ZBUFW*GX_ZBUFH)
	#define GX_ZBUFSIZE  ((GX_ZBUFW*GX_ZBUFH)*2)
#endif


class k053247_device : public device_t,
						public device_video_interface,
						public device_gfx_interface
{
public:
	k053247_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	k053247_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	~k053247_device() { }

	// static configuration
	static void set_k053247_callback(device_t &device, k053247_cb_delegate callback) { downcast<k053247_device &>(device).m_k053247_cb = callback; }
	static void set_config(device_t &device, const char *gfx_reg, int bpp, int dx, int dy)
	{
		k053247_device &dev = downcast<k053247_device &>(device);
		dev.m_memory_region = gfx_reg;
		dev.m_bpp = bpp;
		dev.m_dx = dx;
		dev.m_dy = dy;
	}

	void clear_all();

	DECLARE_READ16_MEMBER( k055673_rom_word_r );
	DECLARE_READ16_MEMBER( k055673_5bpp_rom_word_r );

	DECLARE_READ8_MEMBER( k053247_r );
	DECLARE_WRITE8_MEMBER( k053247_w );
	DECLARE_READ16_MEMBER( k053247_word_r );
	DECLARE_WRITE16_MEMBER( k053247_word_w );
	DECLARE_WRITE16_MEMBER( k055673_reg_word_w ); // "OBJSET2" registers

	void k053247_sprites_draw( bitmap_ind16 &bitmap,const rectangle &cliprect);
	void k053247_sprites_draw( bitmap_rgb32 &bitmap,const rectangle &cliprect);
	int k053247_read_register( int regnum);
	void k053247_set_z_rejection( int zcode); // common to k053246/7
	void k053247_get_ram( UINT16 **ram);
	int k053247_get_dx( void );
	int k053247_get_dy( void );

	DECLARE_READ8_MEMBER( k053246_r );
	DECLARE_WRITE8_MEMBER( k053246_w );
	DECLARE_READ16_MEMBER( k053246_word_r );
	DECLARE_WRITE16_MEMBER( k053246_word_w );

	void k053246_set_objcha_line( int state);
	int k053246_is_irq_enabled(void);
	int k053246_read_register( int regnum);

	DECLARE_READ16_MEMBER( k053246_reg_word_r );    // OBJSET1

	std::unique_ptr<UINT16[]>    m_ram;

	gfx_element *m_gfx;

	UINT8    m_kx46_regs[8];
	UINT16   m_kx47_regs[16];
	int      m_dx, m_dy;
	UINT8    m_objcha_line;
	int      m_z_rejection;

	k053247_cb_delegate m_k053247_cb;

	const char *m_memory_region;
	int m_gfx_num;
	int m_bpp;

	/* alt implementation - to be collapsed */
	void zdrawgfxzoom32GP(
		bitmap_rgb32 &bitmap, const rectangle &cliprect,
		UINT32 code, UINT32 color, int flipx, int flipy, int sx, int sy,
		int scalex, int scaley, int alpha, int drawmode, int zcode, int pri, UINT8* gx_objzbuf, UINT8* gx_shdzbuf);

	void zdrawgfxzoom32GP(
			bitmap_ind16 &bitmap, const rectangle &cliprect,
			UINT32 code, UINT32 color, int flipx, int flipy, int sx, int sy,
			int scalex, int scaley, int alpha, int drawmode, int zcode, int pri, UINT8* gx_objzbuf, UINT8* gx_shdzbuf);

	template<class _BitmapClass>
	inline void k053247_draw_single_sprite_gxcore(_BitmapClass &bitmap , rectangle const &cliprect,
		unsigned char*gx_objzbuf, unsigned char*gx_shdzbuf, int code, unsigned short*gx_spriteram, int offs,
		int color, int alpha, int drawmode, int zcode, int pri,
		int primask, int shadow, unsigned char*drawmode_table, unsigned char*shadowmode_table, int shdmask)
	{
		int xa,ya,ox,oy,flipx,flipy,mirrorx,mirrory,zoomx,zoomy,scalex,scaley,nozoom;
		int temp, temp4;
		int flipscreenx = m_kx46_regs[5] & 0x01;
		int flipscreeny = m_kx46_regs[5] & 0x02;

		xa = ya = 0;
		if (code & 0x01) xa += 1;
		if (code & 0x02) ya += 1;
		if (code & 0x04) xa += 2;
		if (code & 0x08) ya += 2;
		if (code & 0x10) xa += 4;
		if (code & 0x20) ya += 4;
		code &= ~0x3f;

		temp4 = gx_spriteram[offs];

		// mask off the upper 6 bits of coordinate and zoom registers
		oy = gx_spriteram[offs+2] & 0x3ff;
		ox = gx_spriteram[offs+3] & 0x3ff;

		scaley = zoomy = gx_spriteram[offs+4] & 0x3ff;
		if (zoomy) zoomy = (0x400000+(zoomy>>1)) / zoomy;
		else zoomy = 0x800000;
		if (!(temp4 & 0x4000))
		{
			scalex = zoomx = gx_spriteram[offs+5] & 0x3ff;
			if (zoomx) zoomx = (0x400000+(zoomx>>1)) / zoomx;
			else zoomx = 0x800000;
		}
		else { zoomx = zoomy; scalex = scaley; }

		nozoom = (scalex == 0x40 && scaley == 0x40);

		flipx = temp4 & 0x1000;
		flipy = temp4 & 0x2000;

		temp = gx_spriteram[offs+6];
		mirrorx = temp & 0x4000;
		if (mirrorx) flipx = 0; // only applies to x mirror, proven
		mirrory = temp & 0x8000;

		int objset1 = k053246_read_register(5);
		// for Escape Kids (GX975)
		if ( objset1 & 8 ) // Check only "Bit #3 is '1'?"
		{
			int screenwidth = m_screen->width();

			zoomx = zoomx>>1; // Fix sprite width to HALF size
			ox = (ox>>1) + 1; // Fix sprite draw position

			if (flipscreenx) ox += screenwidth;
			nozoom = 0;
		}

		if (flipscreenx) { ox = -ox; if (!mirrorx) flipx = !flipx; }
		if (flipscreeny) { oy = -oy; if (!mirrory) flipy = !flipy; }

		int k053247_opset = k053247_read_register(0xc/2);
		int wrapsize, xwraplim, ywraplim;
		if (k053247_opset & 0x40)
		{
			wrapsize = 512;
			xwraplim = 512 - 64;
			ywraplim = 512 - 128;
		}
		else
		{
			wrapsize  = 1024;
			xwraplim  = 1024 - 384;
			ywraplim  = 1024 - 512;
		}


		// get "display window" offsets
		int offx = (short)((m_kx46_regs[0] << 8) | m_kx46_regs[1]);
		int offy = (short)((m_kx46_regs[2] << 8) | m_kx46_regs[3]);


		// apply wrapping and global offsets
		temp = wrapsize-1;

		if (gx_objzbuf && gx_shdzbuf) // our GX drivers assume the offsets are applied here, where is the best place, figure it out and make consistent
		{
			ox += m_dx;
			oy -= m_dy;
		}

		ox = ( ox - offx) & temp;
		oy = (-oy - offy) & temp;
		if (ox >= xwraplim) ox -= wrapsize;
		if (oy >= ywraplim) oy -= wrapsize;


		temp = temp4>>8 & 0x0f;
		int width = 1 << (temp & 3);
		int height = 1 << (temp>>2 & 3);

		if (!(gx_objzbuf && gx_shdzbuf)) // the non-GX ones assume the offsets are applied here
		{
			ox += m_dx;
			oy -= m_dy;
		}

		ox -= (zoomx * width) >> 13;
		oy -= (zoomy * height) >> 13;

		if (gx_objzbuf && gx_shdzbuf) /* GX  */
		{
			k053247_draw_yxloop_gx(bitmap, cliprect,
				code,
				color,
				height, width,
				zoomx, zoomy, flipx, flipy,
				ox, oy,
				xa, ya,
				mirrorx, mirrory,
				nozoom,
				pri,
				zcode, alpha, drawmode,
				gx_objzbuf, gx_shdzbuf,
				0,nullptr
				);

		}
		else /* non-GX */
		{
			UINT8* whichtable = drawmode_table;
			if (color == -1)
			{
				// drop the entire sprite to shadow unconditionally
				if (shdmask < 0) return;
				color = 0;
				shadow = -1;
				whichtable = shadowmode_table;
				palette().set_shadow_mode(0);
			}
			else
			{
				if (shdmask >= 0)
				{
					shadow = (color & K053247_CUSTOMSHADOW) ? (color >> K053247_SHDSHIFT) : (shadow >> 10);
					if (shadow &= 3) palette().set_shadow_mode((shadow - 1) & shdmask);
				}
				else
					shadow = 0;
			}

			color &= 0xffff; // strip attribute flags

			drawmode_table[m_gfx->granularity() - 1] = shadow ? DRAWMODE_SHADOW : DRAWMODE_SOURCE;

			k053247_draw_yxloop_gx(bitmap, cliprect,
				code,
				color,
				height, width,
				zoomx, zoomy, flipx, flipy,
				ox, oy,
				xa, ya,
				mirrorx, mirrory,
				nozoom,
				0,
				0, 0, 0,
				nullptr, nullptr,
				primask,whichtable
				);


		}
	}



	template<class _BitmapClass>
	void k053247_draw_yxloop_gx(_BitmapClass &bitmap, const rectangle &cliprect,
		int code,
		int color,
		int height, int width,
		int zoomx, int zoomy, int flipx, int flipy,
		int ox, int oy,
		int xa, int ya,
		int mirrorx, int mirrory,
		int nozoom,
		/* gx specifics */
		int pri,
		int zcode, int alpha, int drawmode,
		UINT8* gx_objzbuf, UINT8* gx_shdzbuf,
		/* non-gx specifics */
		int primask,
		UINT8* whichtable
		)
	{
		static const int xoffset[8] = { 0, 1, 4, 5, 16, 17, 20, 21 };
		static const int yoffset[8] = { 0, 2, 8, 10, 32, 34, 40, 42 };
		int zw,zh;
		int  fx, fy, sx, sy;
		int tempcode;

		for (int y=0; y<height; y++)
		{
			sy = oy + ((zoomy * y + (1<<11)) >> 12);
			zh = (oy + ((zoomy * (y+1) + (1<<11)) >> 12)) - sy;

			for (int x=0; x<width; x++)
			{
				sx = ox + ((zoomx * x + (1<<11)) >> 12);
				zw = (ox + ((zoomx * (x+1) + (1<<11)) >> 12)) - sx;
				tempcode = code;

				if (mirrorx)
				{
					if ((!flipx)^((x<<1)<width))
					{
						/* mirror left/right */
						tempcode += xoffset[(width-1-x+xa)&7];
						fx = 1;
					}
					else
					{
						tempcode += xoffset[(x+xa)&7];
						fx = 0;
					}
				}
				else
				{
					if (flipx) tempcode += xoffset[(width-1-x+xa)&7];
					else tempcode += xoffset[(x+xa)&7];
					fx = flipx;
				}

				if (mirrory)
				{
					if ((!flipy)^((y<<1)>=height))
					{
						/* mirror top/bottom */
						tempcode += yoffset[(height-1-y+ya)&7];
						fy = 1;
					}
					else
					{
						tempcode += yoffset[(y+ya)&7];
						fy = 0;
					}
				}
				else
				{
					if (flipy) tempcode += yoffset[(height-1-y+ya)&7];
					else tempcode += yoffset[(y+ya)&7];
					fy = flipy;
				}

				if (gx_objzbuf && gx_shdzbuf) /* GX uses a custom draw function */
				{
					if (nozoom) { zw = zh = 0x10; }

					zdrawgfxzoom32GP(
							bitmap, cliprect,
							tempcode,
							color,
							fx,fy,
							sx,sy,
							zw << 12, zh << 12, alpha, drawmode, zcode, pri,
							gx_objzbuf, gx_shdzbuf
							);

				}
				else /* Non-GX using regular pdrawgfx */
				{
					if (nozoom)
					{
						m_gfx->prio_transtable(bitmap,cliprect,
								tempcode,
								color,
								fx,fy,
								sx,sy,
								m_screen->priority(),primask,
								whichtable);
					}
					else
					{
						m_gfx->prio_zoom_transtable(bitmap,cliprect,
								tempcode,
								color,
								fx,fy,
								sx,sy,
								(zw << 16) >> 4,(zh << 16) >> 4,
								m_screen->priority(),primask,
								whichtable);
					}

					if (mirrory && height == 1)  /* Simpsons shadows */
					{
						if (nozoom)
						{
							m_gfx->prio_transtable(bitmap,cliprect,
									tempcode,
									color,
									fx,!fy,
									sx,sy,
									m_screen->priority(),primask,
									whichtable);
						}
						else
						{
							m_gfx->prio_zoom_transtable(bitmap,cliprect,
									tempcode,
									color,
									fx,!fy,
									sx,sy,
									(zw << 16) >> 4,(zh << 16) >> 4,
									m_screen->priority(),primask,
									whichtable);
						}
					}
				}
			} // end of X loop
		} // end of Y loop
	}





	template<class _BitmapClass>
	void k053247_sprites_draw_common( _BitmapClass &bitmap, const rectangle &cliprect );


protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
private:


};

extern const device_type K053246;

class k055673_device : public k053247_device
{
public:
	k055673_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k055673_device() { }

protected:
	// device-level overrides
	virtual void device_start() override;
//  virtual void device_reset();
private:

};

extern const device_type K055673;


#define MCFG_K053246_SET_SCREEN MCFG_VIDEO_SET_SCREEN

#define MCFG_K053246_PALETTE MCFG_GFX_PALETTE


#define MCFG_K055673_SET_SCREEN MCFG_VIDEO_SET_SCREEN

#define MCFG_K055673_PALETTE MCFG_GFX_PALETTE


#endif
