/*************************************************************************

    taitoic.h

    Implementation of various Taito custom video & input ICs

**************************************************************************/

#ifndef _TAITOIC_H_
#define _TAITOIC_H_

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct pc080sn_interface
{
	int                m_gfxnum;

	int                m_x_offset, m_y_offset;
	int                m_y_invert;
	int                m_dblwidth;
};


struct pc090oj_interface
{
	int                m_gfxnum;

	int                m_x_offset, m_y_offset;
	int                m_use_buffer;
};


struct tc0080vco_interface
{
	int                m_gfxnum;
	int                m_txnum;

	int                m_bg_xoffs, m_bg_yoffs;
	int                m_bg_flip_yoffs;

	int                m_has_fg0; /* for debug */
};

struct tc0100scn_interface
{
	const char         *m_screen_tag;

	int                m_gfxnum;
	int                m_txnum;

	int                m_x_offset, m_y_offset;
	int                m_flip_xoffs, m_flip_yoffs;
	int                m_flip_text_xoffs, m_flip_text_yoffs;

	int                m_multiscrn_xoffs;
	int                m_multiscrn_hack;
};


struct tc0280grd_interface
{
	int                m_gfxnum;
};


struct tc0480scp_interface
{
	int                m_gfxnum;
	int                m_txnum;

	int                m_pixels;

	int                m_x_offset, m_y_offset;
	int                m_text_xoffs, m_text_yoffs;
	int                m_flip_xoffs, m_flip_yoffs;

	int                m_col_base;
};


struct tc0150rod_interface
{
	const char      *m_gfx_region;    /* gfx region for the road */
};


struct tc0110pcr_interface
{
	int               m_pal_offs;
};

struct tc0180vcu_interface
{
	int            m_bg_color_base;
	int            m_fg_color_base;
	int            m_tx_color_base;
};

class pc080sn_device : public device_t,
						public pc080sn_interface
{
public:
	pc080sn_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~pc080sn_device() {}
	
	DECLARE_READ16_MEMBER( word_r );
	DECLARE_WRITE16_MEMBER( word_w );
	DECLARE_WRITE16_MEMBER( xscroll_word_w );
	DECLARE_WRITE16_MEMBER( yscroll_word_w );
	DECLARE_WRITE16_MEMBER( ctrl_word_w );
	DECLARE_WRITE16_MEMBER( scrollram_w );
	
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	
	void common_get_pc080sn_bg_tile_info( tile_data &tileinfo, int tile_index, UINT16 *ram, int gfxnum );
	void common_get_pc080sn_fg_tile_info( tile_data &tileinfo, int tile_index, UINT16 *ram, int gfxnum );

	void set_scroll(int tilemap_num, int scrollx, int scrolly);
	void set_trans_pen(int tilemap_num, int pen);
	void tilemap_update();
	void tilemap_draw(bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, UINT32 priority);
	void tilemap_draw_offset(bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, UINT32 priority, int xoffs, int yoffs);
	void topspeed_custom_draw(bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, UINT32 priority, UINT16 *color_ctrl_ram);

	/* For Topspeed */
	void tilemap_draw_special(bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, UINT32 priority, UINT16 *ram);
	
	void restore_scroll();

	protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	
	private:
	// internal state
	UINT16         m_ctrl[8];

	UINT16 		   *m_ram;
	UINT16 		   *m_bg_ram[2];
	UINT16		   *m_bgscroll_ram[2];

	int            m_bgscrollx[2], m_bgscrolly[2];
		
	tilemap_t      *m_tilemap[2];
			
};

extern const device_type PC080SN;

class pc090oj_device : public device_t,
						public pc090oj_interface
{
public:
	pc090oj_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~pc090oj_device() {}

	DECLARE_READ16_MEMBER( word_r );
	DECLARE_WRITE16_MEMBER( word_w );

	void set_sprite_ctrl(UINT16 sprctrl);
	void eof_callback();
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int pri_type);
	
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

private:
	/* NB: pc090oj_ctrl is the internal register controlling flipping

   pc090oj_sprite_ctrl is a representation of the hardware OUTSIDE the pc090oj
   which impacts on sprite plotting, and which varies between games. It
   includes color banking and (optionally) priority. It allows each game to
   control these aspects of the sprites in different ways, while keeping the
   routines here modular.

*/

	UINT16     m_ctrl;
	UINT16     m_sprite_ctrl;

	UINT16 *   m_ram;
	UINT16 *   m_ram_buffered;
};

extern const device_type PC090OJ;

class tc0080vco_device : public device_t,
							public tc0080vco_interface
{
public:
	tc0080vco_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~tc0080vco_device() {}

	DECLARE_READ16_MEMBER( word_r );
	DECLARE_WRITE16_MEMBER( word_w );

	void tilemap_update();
	void tilemap_draw(bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, UINT32 priority);

	DECLARE_READ16_MEMBER( cram_0_r );
	DECLARE_READ16_MEMBER( cram_1_r );
	DECLARE_READ16_MEMBER( sprram_r );
	DECLARE_READ16_MEMBER( scrram_r );
	DECLARE_WRITE16_MEMBER( scrollram_w );
	READ_LINE_MEMBER( flipscreen_r );
	void postload();

	protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();

	private:
	// internal state
	UINT16 *       m_ram;
	UINT16 *       m_bg0_ram_0;
	UINT16 *       m_bg0_ram_1;
	UINT16 *       m_bg1_ram_0;
	UINT16 *       m_bg1_ram_1;
	UINT16 *       m_tx_ram_0;
	UINT16 *       m_tx_ram_1;
	UINT16 *       m_char_ram;
	UINT16 *       m_bgscroll_ram;

/* FIXME: This sprite related stuff still needs to be accessed in
   video/taito_h */
	UINT16 *       m_chain_ram_0;
	UINT16 *       m_chain_ram_1;
	UINT16 *       m_spriteram;
	UINT16 *       m_scroll_ram;

	UINT16         m_bg0_scrollx;
	UINT16         m_bg0_scrolly;
	UINT16         m_bg1_scrollx;
	UINT16         m_bg1_scrolly;

	tilemap_t        *m_tilemap[3];

	INT32          m_flipscreen;
	
	TILE_GET_INFO_MEMBER(get_bg0_tile_info);
	TILE_GET_INFO_MEMBER(get_bg1_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	void bg0_tilemap_draw( bitmap_ind16 &bitmap, const rectangle &cliprect, int flags, UINT32 priority );
	void bg1_tilemap_draw( bitmap_ind16 &bitmap, const rectangle &cliprect, int flags, UINT32 priority );
};

extern const device_type TC0080VCO;

class tc0100scn_device : public device_t,
							public tc0100scn_interface
{
public:
	tc0100scn_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~tc0100scn_device() {}
	
	#define TC0100SCN_SINGLE_VDU    1024

	/* Function to set separate color banks for the three tilemapped layers.
	To change from the default (0,0,0) use after calling TC0100SCN_vh_start */
	void set_colbanks(int bg0, int bg1, int tx);

	/* Function to set separate color banks for each TC0100SCN.
	To change from the default (0,0,0) use after calling TC0100SCN_vh_start */
	void set_colbank(int colbank);

	/* Function to set bg tilemask < 0xffff */
	void set_bg_tilemask(int mask);

	/* Function to for Mjnquest to select gfx bank */
	DECLARE_WRITE16_MEMBER(gfxbank_w);

	DECLARE_READ16_MEMBER(word_r);
	DECLARE_WRITE16_MEMBER(word_w);
	DECLARE_READ16_MEMBER(ctrl_word_r);
	DECLARE_WRITE16_MEMBER(ctrl_word_w);

	/* Functions for use with 68020 (Under Fire) */
	DECLARE_READ32_MEMBER(long_r);
	DECLARE_WRITE32_MEMBER(long_w);
	DECLARE_READ32_MEMBER(ctrl_long_r);
	DECLARE_WRITE32_MEMBER(ctrl_long_w);

	void tilemap_update();
	int tilemap_draw(bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, UINT32 priority);

	/* returns 0 or 1 depending on the lowest priority tilemap set in the internal
	register. Use this function to draw tilemaps in the correct order. */
	int bottomlayer();
	
	void postload();

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

private:
	// internal state
	UINT16       m_ctrl[8];

	UINT16 *     m_ram;
	UINT16 *     m_bg_ram;
	UINT16 *     m_fg_ram;
	UINT16 *     m_tx_ram;
	UINT16 *     m_char_ram;
	UINT16 *     m_bgscroll_ram;
	UINT16 *     m_fgscroll_ram;
	UINT16 *     m_colscroll_ram;

	int          m_bgscrollx, m_bgscrolly, m_fgscrollx, m_fgscrolly;

	/* We keep two tilemaps for each of the 3 actual tilemaps: one at standard width, one double */
	tilemap_t      *m_tilemap[3][2];
	rectangle    m_cliprect;

	int          m_bg_col_mult, m_bg_tilemask, m_tx_col_mult;
	INT32        m_gfxbank, m_colbank;
	INT32        m_bg0_colbank, m_bg1_colbank, m_tx_colbank;
	int          m_dblwidth;

	screen_device *m_screen;

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	
	void common_get_bg0_tile_info(tile_data &tileinfo, int tile_index, UINT16 *ram, int gfxnum, int colbank, int dblwidth);
	void common_get_bg1_tile_info(tile_data &tileinfo, int tile_index, UINT16 *ram, int gfxnum, int colbank, int dblwidth);
	void common_get_tx_tile_info(tile_data &tileinfo, int tile_index, UINT16 *ram, int gfxnum, int colbank, int dblwidth);
	
	void tilemap_draw_fg(bitmap_ind16 &bitmap, const rectangle &cliprect, tilemap_t* tmap, int flags, UINT32 priority);
	void set_layer_ptrs();
	void dirty_tilemaps();
	void restore_scroll();
};

extern const device_type TC0100SCN;

class tc0280grd_device : public device_t,
							public tc0280grd_interface
{
public:
	tc0280grd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~tc0280grd_device() {}
	
	DECLARE_READ16_MEMBER( tc0280grd_word_r );
	DECLARE_WRITE16_MEMBER( tc0280grd_word_w );
	DECLARE_WRITE16_MEMBER( tc0280grd_ctrl_word_w );
	void tc0280grd_tilemap_update(int base_color);
	void tc0280grd_zoom_draw(bitmap_ind16 &bitmap, const rectangle &cliprect, int xoffset, int yoffset, UINT32 priority);

	DECLARE_READ16_MEMBER( tc0430grw_word_r );
	DECLARE_WRITE16_MEMBER( tc0430grw_word_w );
	DECLARE_WRITE16_MEMBER( tc0430grw_ctrl_word_w );
	void tc0430grw_tilemap_update(int base_color);
	void tc0430grw_zoom_draw(bitmap_ind16 &bitmap, const rectangle &cliprect, int xoffset, int yoffset, UINT32 priority);

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
	
private:
	// internal state
	UINT16 *       m_ram;

	tilemap_t      *m_tilemap;

	UINT16         m_ctrl[8];
	int            m_base_color;

	TILE_GET_INFO_MEMBER(tc0280grd_get_tile_info);
	void zoom_draw( bitmap_ind16 &bitmap, const rectangle &cliprect, int xoffset, int yoffset, UINT32 priority, int xmultiply );
};

extern const device_type TC0280GRD;

#define TC0430GRW TC0280GRD

class tc0360pri_device : public device_t
{
public:
	tc0360pri_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~tc0360pri_device() {}

	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( read );
	
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

private:
	// internal state
	UINT8   m_regs[16];
};

extern const device_type TC0360PRI;

class tc0480scp_device : public device_t,
											public tc0480scp_interface
{
public:
	tc0480scp_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~tc0480scp_device() {}

	/* When writing a driver, pass zero for the text and flip offsets initially:
	then tweak them once you have the 4 bg layer positions correct. Col_base
	may be needed when tilemaps use a palette area from sprites. */

	DECLARE_READ16_MEMBER( word_r );
	DECLARE_WRITE16_MEMBER( word_w );
	DECLARE_READ16_MEMBER( ctrl_word_r );
	DECLARE_WRITE16_MEMBER( ctrl_word_w );

	/* Functions for use with 68020 (Super-Z system) */
	DECLARE_READ32_MEMBER( long_r );
	DECLARE_WRITE32_MEMBER( long_w );
	DECLARE_READ32_MEMBER( ctrl_long_r );
	DECLARE_WRITE32_MEMBER( ctrl_long_w );

	void tilemap_update();
	void tilemap_draw(bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, UINT32 priority);

	/* Returns the priority order of the bg tilemaps set in the internal
	register. The order in which the four layers should be drawn is
	returned in the lowest four nibbles  (msn = bottom layer; lsn = top) */
	int get_bg_priority();

	/* Undrfire needs to read this for a sprite/tile priority hack */
	DECLARE_READ8_MEMBER( pri_reg_r );
	
	void postload();
	
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
	
private:
	// internal state
	UINT16           m_ctrl[0x18];

	UINT16 *         m_ram;
	UINT16 *         m_bg_ram[4];
	UINT16 *         m_tx_ram;
	UINT16 *         m_char_ram;
	UINT16 *         m_bgscroll_ram[4];
	UINT16 *         m_rowzoom_ram[4];
	UINT16 *         m_bgcolumn_ram[4];
	int              m_bgscrollx[4];
	int              m_bgscrolly[4];
	int              m_pri_reg;

	/* We keep two tilemaps for each of the 5 actual tilemaps: one at standard width, one double */
	tilemap_t         *m_tilemap[5][2];
	INT32           m_dblwidth;
	int             m_x_offs;

	void common_get_tc0480bg_tile_info( tile_data &tileinfo, int tile_index, UINT16 *ram, int gfxnum );
	void common_get_tc0480tx_tile_info( tile_data &tileinfo, int tile_index, UINT16 *ram, int gfxnum );
	
	TILE_GET_INFO_MEMBER(get_bg0_tile_info);
	TILE_GET_INFO_MEMBER(get_bg1_tile_info);
	TILE_GET_INFO_MEMBER(get_bg2_tile_info);
	TILE_GET_INFO_MEMBER(get_bg3_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	
	void dirty_tilemaps();
	void set_layer_ptrs();
	void bg01_draw( bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, UINT32 priority );
	void bg23_draw( bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, UINT32 priority );
};

extern const device_type TC0480SCP;

class tc0150rod_device : public device_t,
										 public tc0150rod_interface
{
public:
	tc0150rod_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~tc0150rod_device() {}

	DECLARE_READ16_MEMBER( word_r );
	DECLARE_WRITE16_MEMBER( word_w );
	void draw(bitmap_ind16 &bitmap, const rectangle &cliprect, int y_offs, int palette_offs, int type, int road_trans, UINT32 low_priority, UINT32 high_priority);
	
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	
private:
	// internal state
	UINT16 *        m_ram;
};

extern const device_type TC0150ROD;

class tc0110pcr_device : public device_t,
										  public tc0110pcr_interface
{
public:
	tc0110pcr_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~tc0110pcr_device() {}

	DECLARE_READ16_MEMBER( word_r );
	DECLARE_WRITE16_MEMBER( word_w ); /* color index goes up in step of 2 */
	DECLARE_WRITE16_MEMBER( step1_word_w );   /* color index goes up in step of 1 */
	DECLARE_WRITE16_MEMBER( step1_rbswap_word_w );    /* swaps red and blue components */
	DECLARE_WRITE16_MEMBER( step1_4bpg_word_w );  /* only 4 bits per color gun */

	void restore_colors();
	
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
	
private:
	UINT16 *     m_ram;
	int          m_type;
	int          m_addr;
};

extern const device_type TC0110PCR;

class tc0180vcu_device : public device_t,
											public tc0180vcu_interface
{
public:
	tc0180vcu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~tc0180vcu_device() {}
	
	DECLARE_READ8_MEMBER( get_fb_page );
	DECLARE_WRITE8_MEMBER( set_fb_page );
	DECLARE_READ8_MEMBER( get_videoctrl );
	DECLARE_READ16_MEMBER( ctrl_r );
	DECLARE_WRITE16_MEMBER( ctrl_w );
	DECLARE_READ16_MEMBER( scroll_r );
	DECLARE_WRITE16_MEMBER( scroll_w );
	DECLARE_READ16_MEMBER( word_r );
	DECLARE_WRITE16_MEMBER( word_w );
	void tilemap_draw(bitmap_ind16 &bitmap, const rectangle &cliprect, int tmap_num, int plane);

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

	private:
	// internal state
	UINT16         m_ctrl[0x10];

	UINT16 *       m_ram;
	UINT16 *       m_scrollram;

	tilemap_t      *m_tilemap[3];

	UINT16         m_bg_rambank[2], m_fg_rambank[2], m_tx_rambank;
	UINT8          m_framebuffer_page;
	UINT8          m_video_control;

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	
	void video_control( UINT8 data );
};

extern const device_type TC0180VCU;


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_PC080SN_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, PC080SN, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_PC090OJ_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, PC090OJ, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_TC0080VCO_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, TC0080VCO, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_TC0100SCN_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, TC0100SCN, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_TC0280GRD_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, TC0280GRD, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_TC0430GRW_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, TC0430GRW, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_TC0360PRI_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, TC0360PRI, 0)

#define MCFG_TC0150ROD_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, TC0150ROD, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_TC0480SCP_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, TC0480SCP, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_TC0110PCR_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, TC0110PCR, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_TC0180VCU_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, TC0180VCU, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#endif
