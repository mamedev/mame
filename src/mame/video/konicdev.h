/*************************************************************************

    konicdev.h

    Implementation of various Konami custom video ICs

**************************************************************************/

#pragma once
#ifndef __KONICDEV_H__
#define __KONICDEV_H__

#include "devlegcy.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef void (*k007342_callback)(running_machine &machine, int tmap, int bank, int *code, int *color, int *flags);
typedef void (*k007420_callback)(running_machine &machine, int *code, int *color);
typedef void (*k052109_callback)(running_machine &machine, int layer, int bank, int *code, int *color, int *flags, int *priority);
typedef void (*k051960_callback)(running_machine &machine, int *code, int *color, int *priority, int *shadow);
typedef void (*k05324x_callback)(running_machine &machine, int *code, int *color, int *priority);
typedef void (*k051316_callback)(running_machine &machine, int *code, int *color, int *flags);
typedef void (*k056832_callback)(running_machine &machine, int layer, int *code, int *color, int *flags);


struct k007342_interface
{
	int                m_gfxnum;
	k007342_callback   m_callback;
};

struct k007420_interface
{
	int                m_banklimit;
	k007420_callback   m_callback;
};

struct k052109_interface
{
	const char         *m_gfx_memory_region;
	int                m_gfx_num;
	int                m_plane_order;
	int                m_deinterleave;
	k052109_callback   m_callback;
};

struct k051960_interface
{
	const char         *m_gfx_memory_region;
	int                m_gfx_num;
	int                m_plane_order;
	int                m_deinterleave;
	k051960_callback   m_callback;
};

struct k05324x_interface
{
	const char         *m_gfx_memory_region;
	int                m_gfx_num;
	int                m_plane_order;
	int                m_dx, m_dy;
	int                m_deinterleave;
	k05324x_callback   m_callback;
};

struct k053247_interface
{
	const char         *screen;
	const char         *gfx_memory_region;
	int                gfx_num;
	int                plane_order;
	int                dx, dy;
	int                deinterleave;
	k05324x_callback   callback;
};

struct k051316_interface
{
	const char         *m_gfx_memory_region_tag;
	int                m_gfx_num;
	int                m_bpp, m_pen_is_mask, m_transparent_pen;
	int                m_wrap, m_xoffs, m_yoffs;
	k051316_callback   m_callback;
};

struct k053936_interface
{
	int                m_wrap, m_xoff, m_yoff;
};

struct k056832_interface
{
	const char         *m_gfx_memory_region;
	int                m_gfx_num;
	int                m_bpp;
	int                m_big;
	int                m_djmain_hack;
	int                m_deinterleave;
	k056832_callback   m_callback;

	const char         *m_k055555_tag;    // tbyahhoo uses the k056832 together with a k055555
};

struct k054338_interface
{
	const char         *screen;
	int                alpha_inv;
	const char         *k055555;
};

struct k001006_interface
{
	const char     *gfx_region;
};

struct k001005_interface
{
	const char     *screen;
	const char     *cpu;
	const char     *dsp;
	const char     *k001006_1;
	const char     *k001006_2;

	const char     *gfx_memory_region;
	int            gfx_index;
};

struct k001604_interface
{
	int            gfx_index_1;
	int            gfx_index_2;
	int            layer_size;
	int            roz_size;
	int            txt_mem_offset;
	int            roz_mem_offset;
};

struct k037122_interface
{
	const char     *m_screen_tag;
	int            m_gfx_index;
};

class k007121_device : public device_t
{
public:
	k007121_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k007121_device() {}
	
	DECLARE_READ8_MEMBER( ctrlram_r );
	DECLARE_WRITE8_MEMBER( ctrl_w );

	/* shall we move source in the interface? */
	/* also notice that now we directly pass *gfx[chip] instead of **gfx !! */
	void sprites_draw( bitmap_ind16 &bitmap, const rectangle &cliprect, gfx_element *gfx, colortable_t *ctable,	const UINT8 *source, int base_color, int global_x_offset, int bank_base, UINT32 pri_mask );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
	
private:
	// internal state
	UINT8    m_ctrlram[8];
	int      m_flipscreen;
};

extern const device_type K007121;

class k007342_device : public device_t,
										public k007342_interface
{
public:
	k007342_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k007342_device() {}

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( scroll_r );
	DECLARE_WRITE8_MEMBER( scroll_w );
	DECLARE_WRITE8_MEMBER( vreg_w );

	void tilemap_update();
	void tilemap_draw(bitmap_ind16 &bitmap, const rectangle &cliprect, int num, int flags, UINT32 priority);
	int is_int_enabled();

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
private:
	// internal state
	UINT8    *m_ram;
	UINT8    *m_scroll_ram;
	UINT8    *m_videoram_0;
	UINT8    *m_videoram_1;
	UINT8    *m_colorram_0;
	UINT8    *m_colorram_1;

	tilemap_t  *m_tilemap[2];
	int      m_flipscreen, m_int_enabled;
	UINT8    m_regs[8];
	UINT16   m_scrollx[2];
	UINT8    m_scrolly[2];

	TILEMAP_MAPPER_MEMBER(scan);
	TILE_GET_INFO_MEMBER(get_tile_info0);
	TILE_GET_INFO_MEMBER(get_tile_info1);
	void get_tile_info( tile_data &tileinfo, int tile_index, int layer, UINT8 *cram, UINT8 *vram );
};

extern const device_type K007342;

class k007420_device : public device_t,
										public k007420_interface
{
public:
	k007420_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k007420_device() {}

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	void sprites_draw(bitmap_ind16 &bitmap, const rectangle &cliprect, gfx_element *gfx);

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
private:
	// internal state
	UINT8        *m_ram;

	int          m_flipscreen;    // current code uses the 7342 flipscreen!!
	UINT8        m_regs[8];   // current code uses the 7342 regs!! (only [2])
};

extern const device_type K007420;

class k052109_device : public device_t,
										public k052109_interface
{
public:
	k052109_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k052109_device() {}

	/*
	You don't have to decode the graphics: the vh_start() routines will do that
	for you, using the plane order passed.
	Of course the ROM data must be in the correct order. This is a way to ensure
	that the ROM test will pass.
	The konami_rom_deinterleave() function above will do the reorganization for
	you in most cases (but see tmnt.c for additional bit rotations or byte
	permutations which may be required).
	*/
	#define NORMAL_PLANE_ORDER 0x0123
	#define REVERSE_PLANE_ORDER 0x3210
	#define GRADIUS3_PLANE_ORDER 0x1111
	#define TASMAN_PLANE_ORDER 0x1616
	
	/*
	The callback is passed:
	- layer number (0 = FIX, 1 = A, 2 = B)
	- bank (range 0-3, output of the pins CAB1 and CAB2)
	- code (range 00-FF, output of the pins VC3-VC10)
	NOTE: code is in the range 0000-FFFF for X-Men, which uses extra RAM
	- color (range 00-FF, output of the pins COL0-COL7)
	The callback must put:
	- in code the resulting tile number
	- in color the resulting color index
	- if necessary, put flags and/or priority for the TileMap code in the tile_info
	structure (e.g. TILE_FLIPX). Note that TILE_FLIPY is handled internally by the
	chip so it must not be set by the callback.
	*/

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );	
	DECLARE_READ16_MEMBER( word_r );
	DECLARE_WRITE16_MEMBER( word_w );
	DECLARE_READ16_MEMBER( lsb_r );
	DECLARE_WRITE16_MEMBER( lsb_w );
	
	void set_rmrd_line(int state);
	int get_rmrd_line();
	void tilemap_update();
	int is_irq_enabled();
	void set_layer_offsets(int layer, int dx, int dy);
	void tilemap_mark_dirty(int tmap_num);
	void tilemap_draw(bitmap_ind16 &bitmap, const rectangle &cliprect, int tmap_num, UINT32 flags, UINT8 priority);

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
private:
	// internal state
	UINT8    *m_ram;
	UINT8    *m_videoram_F;
	UINT8    *m_videoram_A;
	UINT8    *m_videoram_B;
	UINT8    *m_videoram2_F;
	UINT8    *m_videoram2_A;
	UINT8    *m_videoram2_B;
	UINT8    *m_colorram_F;
	UINT8    *m_colorram_A;
	UINT8    *m_colorram_B;

	tilemap_t  *m_tilemap[3];
	int      m_tileflip_enable;
	UINT8    m_charrombank[4];
	UINT8    m_charrombank_2[4];
	UINT8    m_has_extra_video_ram;
	INT32    m_rmrd_line;
	UINT8    m_irq_enabled;
	INT32    m_dx[3], m_dy[3];
	UINT8    m_romsubbank, m_scrollctrl;

	TILE_GET_INFO_MEMBER(get_tile_info0);
	TILE_GET_INFO_MEMBER(get_tile_info1);
	TILE_GET_INFO_MEMBER(get_tile_info2);
	
	void get_tile_info( tile_data &tileinfo, int tile_index, int layer, UINT8 *cram, UINT8 *vram1, UINT8 *vram2 );
	void tileflip_reset();
};

extern const device_type K052109;

class k051960_device : public device_t,
										public k051960_interface
{
public:
	k051960_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k051960_device() {}

	/*
	The callback is passed:
	- code (range 00-1FFF, output of the pins CA5-CA17)
	- color (range 00-FF, output of the pins OC0-OC7). Note that most of the
	  time COL7 seems to be "shadow", but not always (e.g. Aliens).
	The callback must put:
	- in code the resulting sprite number
	- in color the resulting color index
	- if necessary, in priority the priority of the sprite wrt tilemaps
	- if necessary, alter shadow to indicate whether the sprite has shadows enabled.
	  shadow is preloaded with color & 0x80 so it doesn't need to be changed unless
	  the game has special treatment (Aliens)
	*/

	DECLARE_READ8_MEMBER( k051960_r );
	DECLARE_WRITE8_MEMBER( k051960_w );
	DECLARE_READ16_MEMBER( k051960_word_r );
	DECLARE_WRITE16_MEMBER( k051960_word_w );

	DECLARE_READ8_MEMBER( k051937_r );
	DECLARE_WRITE8_MEMBER( k051937_w );
	DECLARE_READ16_MEMBER( k051937_word_r );
	DECLARE_WRITE16_MEMBER( k051937_word_w );

	void k051960_sprites_draw(bitmap_ind16 &bitmap, const rectangle &cliprect, int min_priority, int max_priority);
	int k051960_is_irq_enabled();
	int k051960_is_nmi_enabled();
	void k051960_set_sprite_offsets(int dx, int dy);

	#if 0 // to be moved in the specific drivers!
	/* special handling for the chips sharing address space */
	DECLARE_READ8_HANDLER( k052109_051960_r );
	DECLARE_WRITE8_HANDLER( k052109_051960_w );
	#endif

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
private:
	// internal state
	UINT8    *m_ram;

	gfx_element *m_gfx;

	UINT8    m_spriterombank[3];
	int      m_dx, m_dy;
	int      m_romoffset;
	int      m_spriteflip, m_readroms;
	int      m_irq_enabled, m_nmi_enabled;

	int      m_k051937_counter;
	
	int k051960_fetchromdata( int byte );
};

extern const device_type K051960;

class k05324x_device : public device_t,
										public k05324x_interface
{
public:
	k05324x_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k05324x_device() {}

	DECLARE_READ16_MEMBER( k053245_word_r );
	DECLARE_WRITE16_MEMBER( k053245_word_w );
	DECLARE_READ8_MEMBER( k053245_r );
	DECLARE_WRITE8_MEMBER( k053245_w );
	DECLARE_READ8_MEMBER( k053244_r );
	DECLARE_WRITE8_MEMBER( k053244_w );
	DECLARE_READ16_MEMBER( k053244_lsb_r );
	DECLARE_WRITE16_MEMBER( k053244_lsb_w );
	DECLARE_READ16_MEMBER( k053244_word_r );
	DECLARE_WRITE16_MEMBER( k053244_word_w );
	void k053244_bankselect(int bank);    /* used by TMNT2, Asterix and Premier Soccer for ROM testing */
	void k053245_sprites_draw(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void k053245_sprites_draw_lethal(bitmap_ind16 &bitmap, const rectangle &cliprect); /* for lethal enforcers */
	void k053245_clear_buffer();
	void k053245_update_buffer();
	void k053245_set_sprite_offs(int offsx, int offsy);
	void k05324x_set_z_rejection(int zcode); // common to k053244/5

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
private:
	// internal state
	UINT16    *m_ram;
	UINT16    *m_buffer;

	gfx_element *m_gfx;

	UINT8    m_regs[0x10];    // 053244
	int      m_rombank;       // 053244
	int      m_ramsize;
	int      m_z_rejection;
	
	DECLARE_READ16_MEMBER( k053244_reg_word_r );    // OBJSET0 debug handler
};

extern const device_type K053244;

#define K053245 K053244
class k053247_device : public device_t
{
public:
	k053247_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k053247_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
private:
	// internal state
	void *m_token;
};

extern const device_type K053246;

#define K053247 K053246
class k055673_device : public device_t
{
public:
	k055673_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k055673_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
private:
	// internal state
	void *m_token;
};

extern const device_type K055673;

class k051316_device : public device_t,
										public k051316_interface
{
public:
	k051316_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k051316_device() {}

	/*
	The callback is passed:
	- code (range 00-FF, contents of the first tilemap RAM byte)
	- color (range 00-FF, contents of the first tilemap RAM byte). Note that bit 6
	  seems to be hardcoded as flip X.
	The callback must put:
	- in code the resulting tile number
	- in color the resulting color index
	- if necessary, put flags for the TileMap code in the tile_info
	  structure (e.g. TILE_FLIPX)
	*/

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( rom_r );
	DECLARE_WRITE8_MEMBER( ctrl_w );
	void zoom_draw(bitmap_ind16 &bitmap,const rectangle &cliprect,int flags,UINT32 priority);
	void wraparound_enable(int status);

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
private:
	// internal state
	UINT8    *m_ram;
	tilemap_t  *m_tmap;
	int      m_bpp;
	UINT8    m_ctrlram[16];
		
	TILE_GET_INFO_MEMBER(get_tile_info0);
	void get_tile_info( tile_data &tileinfo, int tile_index );
};

extern const device_type K051316;

class k053936_device : public device_t,
										public k053936_interface
{
public:
	k053936_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k053936_device() {}

	DECLARE_WRITE16_MEMBER( ctrl_w );
	DECLARE_READ16_MEMBER( ctrl_r );    
	DECLARE_WRITE16_MEMBER( linectrl_w );
	DECLARE_READ16_MEMBER( linectrl_r );
	void zoom_draw(bitmap_ind16 &bitmap, const rectangle &cliprect, tilemap_t *tmap, int flags, UINT32 priority, int glfgreat_hack);
	// void wraparound_enable(int status);   unused? // shall we merge this into the configuration intf?
	// void set_offset(int xoffs, int yoffs); unused?   // shall we merge this into the configuration intf?

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

private:
	// internal state
	UINT16    *m_ctrl;
	UINT16    *m_linectrl;
};

extern const device_type K053936;

class k053251_device : public device_t
{
public:
	k053251_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k053251_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
private:
	// internal state
	void *m_token;
};

extern const device_type K053251;

class k054000_device : public device_t
{
public:
	k054000_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k054000_device() {}

	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE16_MEMBER( lsb_w );
	DECLARE_READ16_MEMBER( lsb_r );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

private:
	// internal state
	UINT8    m_regs[0x20];
};

extern const device_type K054000;

class k051733_device : public device_t
{
public:
	k051733_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k051733_device() {}

	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( read );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
private:
	// internal state
	UINT8    m_ram[0x20];
	UINT8    m_rng;
};

extern const device_type K051733;

#define K056832_PAGE_COUNT 16

/* bit depths for the 56832 */
#define K056832_BPP_4   0
#define K056832_BPP_5   1
#define K056832_BPP_6   2
#define K056832_BPP_8   3
#define K056832_BPP_4dj 4
#define K056832_BPP_8LE 5
#define K056832_BPP_8TASMAN 6

class k056832_device : public device_t,
										public k056832_interface
{
public:
	k056832_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k056832_device() {}

	void SetExtLinescroll();    /* Lethal Enforcers */

	DECLARE_READ16_MEMBER( ram_word_r );
	DECLARE_WRITE16_MEMBER( ram_word_w );
	DECLARE_READ16_MEMBER( ram_half_word_r );
	DECLARE_WRITE16_MEMBER( ram_half_word_w );
	DECLARE_READ16_MEMBER( k_5bpp_rom_word_r );
	DECLARE_READ32_MEMBER( k_5bpp_rom_long_r );
	DECLARE_READ32_MEMBER( k_6bpp_rom_long_r );
	DECLARE_READ16_MEMBER( rom_word_r );
	DECLARE_READ16_MEMBER( mw_rom_word_r );
	DECLARE_READ16_MEMBER( bishi_rom_word_r );
	DECLARE_READ16_MEMBER( old_rom_word_r );
	DECLARE_READ16_MEMBER( rom_word_8000_r );
	DECLARE_WRITE16_MEMBER( word_w ); // "VRAM" registers
	DECLARE_WRITE16_MEMBER( b_word_w );
	DECLARE_READ8_MEMBER( ram_code_lo_r );
	DECLARE_READ8_MEMBER( ram_code_hi_r );
	DECLARE_READ8_MEMBER( ram_attr_lo_r );
	DECLARE_READ8_MEMBER( ram_attr_hi_r );
	DECLARE_WRITE8_MEMBER( ram_code_lo_w );
	DECLARE_WRITE8_MEMBER( ram_code_hi_w );
	DECLARE_WRITE8_MEMBER( ram_attr_lo_w );
	DECLARE_WRITE8_MEMBER( ram_attr_hi_w );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_WRITE8_MEMBER( b_w );
	void mark_plane_dirty(int num);
	void mark_all_tmaps_dirty();
	void tilemap_draw(bitmap_ind16 &bitmap, const rectangle &cliprect, int num, UINT32 flags, UINT32 priority);
	void tilemap_draw(bitmap_rgb32 &bitmap, const rectangle &cliprect, int num, UINT32 flags, UINT32 priority);
	void tilemap_draw_dj(bitmap_rgb32 &bitmap, const rectangle &cliprect, int layer, UINT32 flags, UINT32 priority);
	void set_layer_association(int status);
	int  get_layer_association();
	void set_layer_offs(int layer, int offsx, int offsy);
	void set_lsram_page(int logical_page, int physical_page, int physical_offset);
	void linemap_enable(int enable);
	int  is_irq_enabled(int irqline);
	void read_avac(int *mode, int *data);
	int  read_register(int regnum);
	int get_current_rambank();
	int get_lookup(int bits); /* Asterix */
	void set_tile_bank(int bank); /* Asterix */

	DECLARE_READ32_MEMBER( ram_long_r );
	DECLARE_READ32_MEMBER( rom_long_r );
	DECLARE_WRITE32_MEMBER( ram_long_w );
	DECLARE_READ32_MEMBER( unpaged_ram_long_r );
	DECLARE_WRITE32_MEMBER( unpaged_ram_long_w );
	DECLARE_WRITE32_MEMBER( long_w );
	DECLARE_WRITE32_MEMBER( b_long_w );
	
	DECLARE_READ16_MEMBER( word_r );        // VACSET
	DECLARE_READ16_MEMBER( b_word_r );      // VSCCS  (board dependent)
	DECLARE_READ32_MEMBER( long_r );        // VACSET


protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	
private:
	// internal state
	tilemap_t   *m_tilemap[K056832_PAGE_COUNT];
	bitmap_ind16  *m_pixmap[K056832_PAGE_COUNT];

	UINT16    m_regs[0x20];   // 157/832 regs group 1
	UINT16    m_regsb[4]; // 157/832 regs group 2, board dependent

	UINT8 *   m_rombase;  // pointer to tile gfx data
	UINT16 *  m_videoram;
	int       m_num_gfx_banks;    // depends on size of graphics ROMs
	int       m_cur_gfx_banks;        // cached info for K056832_regs[0x1a]

	// ROM readback involves reading 2 halves of a word
	// from the same location in a row.  Reading the
	// RAM window resets this state so you get the first half.
	int       m_rom_half;

	// locally cached values
	int       m_layer_assoc_with_page[K056832_PAGE_COUNT];
	int       m_layer_offs[8][2];
	int       m_lsram_page[8][2];
	INT32     m_x[8]; // 0..3 left
	INT32     m_y[8]; // 0..3 top
	INT32     m_w[8]; // 0..3 width  -> 1..4 pages
	INT32     m_h[8]; // 0..3 height -> 1..4 pages
	INT32     m_dx[8];    // scroll
	INT32     m_dy[8];    // scroll
	UINT32    m_line_dirty[K056832_PAGE_COUNT][8];
	UINT8     m_all_lines_dirty[K056832_PAGE_COUNT];
	UINT8     m_page_tile_mode[K056832_PAGE_COUNT];
	int       m_last_colorbase[K056832_PAGE_COUNT];
	UINT8     m_layer_tile_mode[8];
	int       m_default_layer_association;
	int       m_layer_association;
	int       m_active_layer;
	int       m_selected_page;
	int       m_selected_page_x4096;
	int       m_linemap_enabled;
	int       m_use_ext_linescroll;
	int       m_uses_tile_banks, m_cur_tile_bank;

	device_t *m_k055555;  /* used to choose colorbase */

	void get_tile_info(  tile_data &tileinfo, int tile_index, int pageIndex );
	
	TILE_GET_INFO_MEMBER(get_tile_info0);
	TILE_GET_INFO_MEMBER(get_tile_info1);
	TILE_GET_INFO_MEMBER(get_tile_info2);
	TILE_GET_INFO_MEMBER(get_tile_info3);
	TILE_GET_INFO_MEMBER(get_tile_info4);
	TILE_GET_INFO_MEMBER(get_tile_info5);
	TILE_GET_INFO_MEMBER(get_tile_info6);
	TILE_GET_INFO_MEMBER(get_tile_info7);
	TILE_GET_INFO_MEMBER(get_tile_info8);
	TILE_GET_INFO_MEMBER(get_tile_info9);
	TILE_GET_INFO_MEMBER(get_tile_infoa);
	TILE_GET_INFO_MEMBER(get_tile_infob);
	TILE_GET_INFO_MEMBER(get_tile_infoc);
	TILE_GET_INFO_MEMBER(get_tile_infod);
	TILE_GET_INFO_MEMBER(get_tile_infoe);
	TILE_GET_INFO_MEMBER(get_tile_infof);
	
	void mark_page_dirty(int page);
	void mark_all_tilemaps_dirty();
	void update_page_layout();
	void change_rambank();
	void change_rombank();
	void postload();
	int rom_read_b(int offset, int blksize, int blksize2, int zerosec);
	
	template<class _BitmapClass>
	int update_linemap(_BitmapClass &bitmap, int page, int flags);
	
	template<class _BitmapClass>
	void tilemap_draw_common(_BitmapClass &bitmap, const rectangle &cliprect, int layer, UINT32 flags, UINT32 priority);
};

extern const device_type K056832;

class k055555_device : public device_t
{
public:
	k055555_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k055555_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
private:
	// internal state
	void *m_token;
};

extern const device_type K055555;

class k054338_device : public device_t
{
public:
	k054338_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k054338_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
private:
	// internal state
	void *m_token;
};

extern const device_type K054338;

class k001006_device : public device_t
{
public:
	k001006_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k001006_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
private:
	// internal state
	void *m_token;
};

extern const device_type K001006;

class k001005_device : public device_t
{
public:
	k001005_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k001005_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_stop();
	virtual void device_reset();
private:
	// internal state
	void *m_token;
};

extern const device_type K001005;

class k001604_device : public device_t
{
public:
	k001604_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k001604_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
private:
	// internal state
	void *m_token;

	TILEMAP_MAPPER_MEMBER(k001604_scan_layer_8x8_0_size0);
	TILEMAP_MAPPER_MEMBER(k001604_scan_layer_8x8_0_size1);
	TILEMAP_MAPPER_MEMBER(k001604_scan_layer_8x8_1_size0);
	TILEMAP_MAPPER_MEMBER(k001604_scan_layer_8x8_1_size1);
	TILEMAP_MAPPER_MEMBER(k001604_scan_layer_roz_256);
	TILEMAP_MAPPER_MEMBER(k001604_scan_layer_roz_128);
	TILE_GET_INFO_MEMBER(k001604_tile_info_layer_8x8);
	TILE_GET_INFO_MEMBER(k001604_tile_info_layer_roz);
};

extern const device_type K001604;

class k037122_device : public device_t,
										public k037122_interface
{
public:
	k037122_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k037122_device() {}

	void tile_draw( bitmap_rgb32 &bitmap, const rectangle &cliprect );
	DECLARE_READ32_MEMBER( sram_r );
	DECLARE_WRITE32_MEMBER( sram_w );
	DECLARE_READ32_MEMBER( char_r );
	DECLARE_WRITE32_MEMBER( char_w );
	DECLARE_READ32_MEMBER( reg_r );
	DECLARE_WRITE32_MEMBER( reg_w );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

private:
	// internal state
	screen_device *m_screen;
	tilemap_t     *m_layer[2];
	
	UINT32 *       m_tile_ram;
	UINT32 *       m_char_ram;
	UINT32 *       m_reg;

	TILE_GET_INFO_MEMBER(tile_info_layer0);
	TILE_GET_INFO_MEMBER(tile_info_layer1);
	void update_palette_color( UINT32 palette_base, int color );
};

extern const device_type K037122;



/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_K007121_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, K007121, 0)

#define MCFG_K007342_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, K007342, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_K007420_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, K007420, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_K052109_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, K052109, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_K051960_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, K051960, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_K053244_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, K053244, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_K053245_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, K053245, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_K053246_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, K053246, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_K053247_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, K053247, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_K055673_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, K055673, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_K051316_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, K051316, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_K053936_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, K053936, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_K053251_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, K053251, 0)

#define MCFG_K054000_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, K054000, 0)

#define MCFG_K051733_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, K051733, 0)

#define MCFG_K056832_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, K056832, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_K055555_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, K055555, 0)

#define MCFG_K054338_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, K054338, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_K001006_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, K001006, 0) \
	MCFG_DEVICE_CONFIG(_interface)


#define MCFG_K001005_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, K001005, 0) \
	MCFG_DEVICE_CONFIG(_interface)


#define MCFG_K001604_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, K001604, 0) \
	MCFG_DEVICE_CONFIG(_interface)


#define MCFG_K037122_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, K037122, 0) \
	MCFG_DEVICE_CONFIG(_interface)


/***************************************************************************
    HELPERS FOR DRIVERS
***************************************************************************/

enum
{
	KONAMI_ROM_DEINTERLEAVE_NONE = 0,
	KONAMI_ROM_DEINTERLEAVE_2,
	KONAMI_ROM_DEINTERLEAVE_2_HALF,
	KONAMI_ROM_DEINTERLEAVE_4,
	KONAMI_ROM_SHUFFLE8
};

/* helper function to join two 16-bit ROMs and form a 32-bit data stream */
void konamid_rom_deinterleave_2(running_machine &machine, const char *mem_region);
void konamid_rom_deinterleave_2_half(running_machine &machine, const char *mem_region);
/* helper function to join four 16-bit ROMs and form a 64-bit data stream */
void konamid_rom_deinterleave_4(running_machine &machine, const char *mem_region);

/* helper function to sort three tile layers by priority order */
void konami_sortlayers3(int *layer, int *pri);
/* helper function to sort four tile layers by priority order */
void konami_sortlayers4(int *layer, int *pri);
/* helper function to sort five tile layers by priority order */
void konami_sortlayers5(int *layer, int *pri);

/***************************************************************************
    DEVICE I/O FUNCTIONS
***************************************************************************/

/**  Konami 053246 / 053247 / 055673  **/
#define K055673_LAYOUT_GX  0
#define K055673_LAYOUT_RNG 1
#define K055673_LAYOUT_LE2 2
#define K055673_LAYOUT_GX6 3

DECLARE_READ16_DEVICE_HANDLER( k055673_rom_word_r );
DECLARE_READ16_DEVICE_HANDLER( k055673_GX6bpp_rom_word_r );

/*
Callback procedures for non-standard shadows:

1) translate shadow code to the correct 2-bit form (0=off, 1-3=style)
2) shift shadow code left by K053247_SHDSHIFT and add the K053247_CUSTOMSHADOW flag
3) combine the result with sprite color
*/
#define K053247_CUSTOMSHADOW    0x20000000
#define K053247_SHDSHIFT        20

DECLARE_READ8_DEVICE_HANDLER( k053247_r );
DECLARE_WRITE8_DEVICE_HANDLER( k053247_w );
DECLARE_READ16_DEVICE_HANDLER( k053247_word_r );
DECLARE_WRITE16_DEVICE_HANDLER( k053247_word_w );
DECLARE_READ32_DEVICE_HANDLER( k053247_long_r );
DECLARE_WRITE32_DEVICE_HANDLER( k053247_long_w );
DECLARE_WRITE16_DEVICE_HANDLER( k053247_reg_word_w ); // "OBJSET2" registers
DECLARE_WRITE32_DEVICE_HANDLER( k053247_reg_long_w );

void k053247_sprites_draw(device_t *device, bitmap_ind16 &bitmap,const rectangle &cliprect);
void k053247_sprites_draw(device_t *device, bitmap_rgb32 &bitmap,const rectangle &cliprect);
int k053247_read_register(device_t *device, int regnum);
void k053247_set_sprite_offs(device_t *device, int offsx, int offsy);
void k053247_wraparound_enable(device_t *device, int status);
void k053247_set_z_rejection(device_t *device, int zcode); // common to k053246/7
void k053247_get_ram(device_t *device, UINT16 **ram);
int k053247_get_dx(device_t *device);
int k053247_get_dy(device_t *device);

DECLARE_READ8_DEVICE_HANDLER( k053246_r );
DECLARE_WRITE8_DEVICE_HANDLER( k053246_w );
DECLARE_READ16_DEVICE_HANDLER( k053246_word_r );
DECLARE_WRITE16_DEVICE_HANDLER( k053246_word_w );
DECLARE_READ32_DEVICE_HANDLER( k053246_long_r );
DECLARE_WRITE32_DEVICE_HANDLER( k053246_long_w );

void k053246_set_objcha_line(device_t *device, int state);
int k053246_is_irq_enabled(device_t *device);
int k053246_read_register(device_t *device, int regnum);

/**  Konami 053251 **/
/*
  Note: k053251_w() automatically does a ALL_TILEMAPS->mark_all_dirty()
  when some palette index changes. If ALL_TILEMAPS is too expensive, use
  k053251_set_tilemaps() to indicate which tilemap is associated with each index.
 */
DECLARE_WRITE8_DEVICE_HANDLER( k053251_w );
DECLARE_WRITE16_DEVICE_HANDLER( k053251_lsb_w );
DECLARE_WRITE16_DEVICE_HANDLER( k053251_msb_w );
int k053251_get_priority(device_t *device, int ci);
int k053251_get_palette_index(device_t *device, int ci);
int k053251_get_tmap_dirty(device_t *device, int tmap_num);
void k053251_set_tmap_dirty(device_t *device, int tmap_num, int data);

enum
{
	K053251_CI0 = 0,
	K053251_CI1,
	K053251_CI2,
	K053251_CI3,
	K053251_CI4
};

/**  Konami 055555  **/
void k055555_write_reg(device_t *device, UINT8 regnum, UINT8 regdat);
DECLARE_WRITE16_DEVICE_HANDLER( k055555_word_w );
DECLARE_WRITE32_DEVICE_HANDLER( k055555_long_w );
int k055555_read_register(device_t *device, int regnum);
int k055555_get_palette_index(device_t *device, int idx);


/* K055555 registers */
/* priority inputs */
#define K55_PALBASE_BG        0 // background palette
#define K55_CONTROL           1 // control register
#define K55_COLSEL_0          2 // layer A, B color depth
#define K55_COLSEL_1          3 // layer C, D color depth
#define K55_COLSEL_2          4 // object, S1 color depth
#define K55_COLSEL_3          5 // S2, S3 color depth

#define K55_PRIINP_0          7 // layer A pri 0
#define K55_PRIINP_1          8 // layer A pri 1
#define K55_PRIINP_2          9 // layer A "COLPRI"
#define K55_PRIINP_3          10    // layer B pri 0
#define K55_PRIINP_4          11    // layer B pri 1
#define K55_PRIINP_5          12    // layer B "COLPRI"
#define K55_PRIINP_6          13    // layer C pri
#define K55_PRIINP_7          14    // layer D pri
#define K55_PRIINP_8          15    // OBJ pri
#define K55_PRIINP_9          16    // sub 1 (GP:PSAC) pri
#define K55_PRIINP_10         17    // sub 2 (GX:PSAC) pri
#define K55_PRIINP_11         18    // sub 3 pri

#define K55_OINPRI_ON         19    // object priority bits selector

#define K55_PALBASE_A         23    // layer A palette
#define K55_PALBASE_B         24    // layer B palette
#define K55_PALBASE_C         25    // layer C palette
#define K55_PALBASE_D         26    // layer D palette
#define K55_PALBASE_OBJ       27    // OBJ palette
#define K55_PALBASE_SUB1      28    // SUB1 palette
#define K55_PALBASE_SUB2      29    // SUB2 palette
#define K55_PALBASE_SUB3      30    // SUB3 palette

#define K55_BLEND_ENABLES     33    // blend enables for tilemaps
#define K55_VINMIX_ON         34    // additional blend enables for tilemaps
#define K55_OSBLEND_ENABLES   35    // obj/sub blend enables
#define K55_OSBLEND_ON        36    // not sure, related to obj/sub blend

#define K55_SHAD1_PRI         37    // shadow/highlight 1 priority
#define K55_SHAD2_PRI         38    // shadow/highlight 2 priority
#define K55_SHAD3_PRI         39    // shadow/highlight 3 priority
#define K55_SHD_ON            40    // shadow/highlight
#define K55_SHD_PRI_SEL       41    // shadow/highlight

#define K55_VBRI              42    // VRAM layer brightness enable
#define K55_OSBRI             43    // obj/sub brightness enable, part 1
#define K55_OSBRI_ON          44    // obj/sub brightness enable, part 2
#define K55_INPUT_ENABLES     45    // input enables

/* bit masks for the control register */
#define K55_CTL_GRADDIR       0x01  // 0=vertical, 1=horizontal
#define K55_CTL_GRADENABLE    0x02  // 0=BG is base color only, 1=gradient
#define K55_CTL_FLIPPRI       0x04  // 0=standard Konami priority, 1=reverse
#define K55_CTL_SDSEL         0x08  // 0=normal shadow timing, 1=(not used by GX)

/* bit masks for the input enables */
#define K55_INP_VRAM_A        0x01
#define K55_INP_VRAM_B        0x02
#define K55_INP_VRAM_C        0x04
#define K55_INP_VRAM_D        0x08
#define K55_INP_OBJ           0x10
#define K55_INP_SUB1          0x20
#define K55_INP_SUB2          0x40
#define K55_INP_SUB3          0x80


/**  Konami 054338  **/
/* mixer/alpha blender */

DECLARE_WRITE16_DEVICE_HANDLER( k054338_word_w ); // "CLCT" registers
DECLARE_WRITE32_DEVICE_HANDLER( k054338_long_w );
int k054338_register_r(device_t *device, int reg);
void k054338_update_all_shadows(device_t *device, int rushingheroes_hack);          // called at the beginning of SCREEN_UPDATE()
void k054338_fill_solid_bg(device_t *device, bitmap_ind16 &bitmap);             // solid backcolor fill
void k054338_fill_backcolor(device_t *device, bitmap_rgb32 &bitmap, int mode);  // unified fill, 0=solid, 1=gradient (by using a k055555)
int  k054338_set_alpha_level(device_t *device, int pblend);                         // blend style 0-2
void k054338_invert_alpha(device_t *device, int invert);                                // 0=0x00(invis)-0x1f(solid), 1=0x1f(invis)-0x00(solod)
//void K054338_export_config(device_t *device, int **shdRGB);

#define K338_REG_BGC_R      0
#define K338_REG_BGC_GB     1
#define K338_REG_SHAD1R     2
#define K338_REG_BRI3       11
#define K338_REG_PBLEND     13
#define K338_REG_CONTROL    15

#define K338_CTL_KILL       0x01    /* 0 = no video output, 1 = enable */
#define K338_CTL_MIXPRI     0x02
#define K338_CTL_SHDPRI     0x04
#define K338_CTL_BRTPRI     0x08
#define K338_CTL_WAILSL     0x10
#define K338_CTL_CLIPSL     0x20


/**  Konami 001006  **/
UINT32 k001006_get_palette(device_t *device, int index);

DECLARE_READ32_DEVICE_HANDLER( k001006_r );
DECLARE_WRITE32_DEVICE_HANDLER( k001006_w );


/**  Konami 001005  **/
void k001005_draw(device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect);
void k001005_swap_buffers(device_t *device);
void k001005_preprocess_texture_data(UINT8 *rom, int length, int gticlub);

DECLARE_READ32_DEVICE_HANDLER( k001005_r );
DECLARE_WRITE32_DEVICE_HANDLER( k001005_w );


/**  Konami 001604  **/
void k001604_draw_back_layer( device_t *device, bitmap_rgb32 &bitmap, const rectangle &cliprect );
void k001604_draw_front_layer( device_t *device, bitmap_rgb32 &bitmap, const rectangle &cliprect );
DECLARE_WRITE32_DEVICE_HANDLER( k001604_tile_w );
DECLARE_READ32_DEVICE_HANDLER( k001604_tile_r );
DECLARE_WRITE32_DEVICE_HANDLER( k001604_char_w );
DECLARE_READ32_DEVICE_HANDLER( k001604_char_r );
DECLARE_WRITE32_DEVICE_HANDLER( k001604_reg_w );
DECLARE_READ32_DEVICE_HANDLER( k001604_reg_r );

#define K056832_DRAW_FLAG_MIRROR      0x00800000

// debug handlers
DECLARE_READ16_DEVICE_HANDLER( k053246_reg_word_r );    // OBJSET1
DECLARE_READ16_DEVICE_HANDLER( k053247_reg_word_r );    // OBJSET2
DECLARE_READ16_DEVICE_HANDLER( k053251_lsb_r );         // PCU1
DECLARE_READ16_DEVICE_HANDLER( k053251_msb_r );         // PCU1
DECLARE_READ16_DEVICE_HANDLER( k055555_word_r );        // PCU2
DECLARE_READ16_DEVICE_HANDLER( k054338_word_r );        // CLTC


DECLARE_READ32_DEVICE_HANDLER( k053247_reg_long_r );    // OBJSET2
DECLARE_READ32_DEVICE_HANDLER( k055555_long_r );        // PCU2

#endif
