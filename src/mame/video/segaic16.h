// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Sega 16-bit common hardware

***************************************************************************/

#pragma once
#ifndef __SEGAIC16VID_H__
#define __SEGAIC16VID_H__



/* tilemap systems */
#define SEGAIC16_MAX_TILEMAPS       1

#define SEGAIC16_TILEMAP_HANGON     0
#define SEGAIC16_TILEMAP_16A        1
#define SEGAIC16_TILEMAP_16B        2
#define SEGAIC16_TILEMAP_16B_ALT    3

#define SEGAIC16_TILEMAP_FOREGROUND 0
#define SEGAIC16_TILEMAP_BACKGROUND 1
#define SEGAIC16_TILEMAP_TEXT       2



/* rotation systems */
#define SEGAIC16_MAX_ROTATE         1

#define SEGAIC16_ROTATE_YBOARD      0



/*************************************
 *
 *  Type definitions
 *
 *************************************/



struct tilemap_callback_info
{
	UINT16 *        rambase;                        /* base of RAM for this tilemap page */
	const UINT8 *   bank;                           /* pointer to bank array */
	UINT16          banksize;                       /* size of banks */
};


struct tilemap_info
{
	UINT8           index;                          /* index of this structure */
	UINT8           type;                           /* type of tilemap (see segaic16.h for details) */
	UINT8           numpages;                       /* number of allocated pages */
	UINT8           flip;                           /* screen flip? */
	UINT8           rowscroll, colscroll;           /* are rowscroll/colscroll enabled (if external enables are used) */
	UINT8           bank[8];                        /* indexes of the tile banks */
	UINT16          banksize;                       /* number of tiles per bank */
	UINT16          latched_xscroll[4];             /* latched X scroll values */
	UINT16          latched_yscroll[4];             /* latched Y scroll values */
	UINT16          latched_pageselect[4];          /* latched page select values */
	INT32           xoffs;                          /* X scroll offset */
	tilemap_t *     tilemaps[16];                   /* up to 16 tilemap pages */
	tilemap_t *     textmap;                        /* a single text tilemap */
	struct tilemap_callback_info tmap_info[16];     /* callback info for 16 tilemap pages */
	struct tilemap_callback_info textmap_info;      /* callback info for a single textmap page */
	void            (*reset)(screen_device &screen, struct tilemap_info *info);/* reset callback */
	void            (*draw_layer)(screen_device &screen, struct tilemap_info *info, bitmap_ind16 &bitmap, const rectangle &cliprect, int which, int flags, int priority);
	UINT16 *        textram;                        /* pointer to textram pointer */
	UINT16 *        tileram;                        /* pointer to tileram pointer */
	emu_timer *     latch_timer;                    /* timer for latching 16b tilemap scroll values */
};


struct rotate_info
{
	UINT8           index;                          /* index of this structure */
	UINT8           type;                           /* type of rotate system (see segaic16.h for details) */
	UINT16          colorbase;                      /* base color index */
	INT32           ramsize;                        /* size of rotate RAM */
	UINT16 *        rotateram;                      /* pointer to rotateram pointer */
	UINT16 *        buffer;                         /* buffered data */
};


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/



class segaic16_video_device :   public device_t,
								public device_video_interface
{
public:
	segaic16_video_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~segaic16_video_device() {}

	// static configuration
	static void static_set_gfxdecode_tag(device_t &device, const char *tag);

	UINT8 m_display_enable;
	optional_shared_ptr<UINT16> m_tileram;
	optional_shared_ptr<UINT16> m_textram;
	optional_shared_ptr<UINT16> m_rotateram;

	void tilemap_set_colscroll(int which, int enable);
	void tilemap_set_rowscroll(int which, int enable);
	void tilemap_set_flip(int which, int flip);
	void tilemap_set_bank(int which, int banknum, int offset);
	void tilemap_reset(screen_device &screen);
	void tilemap_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int which, int map, int priority, int priority_mark);
//  void tilemap_16b_draw_layer(screen_device &screen, struct tilemap_info *info, bitmap_ind16 &bitmap, const rectangle &cliprect, int which, int flags, int priority);
//  void tilemap_16a_draw_layer(screen_device &screen, struct tilemap_info *info, bitmap_ind16 &bitmap, const rectangle &cliprect, int which, int flags, int priority);
//  void draw_virtual_tilemap(screen_device &screen, struct tilemap_info *info, bitmap_ind16 &bitmap, const rectangle &cliprect, UINT16 pages, UINT16 xscroll, UINT16 yscroll, UINT32 flags, UINT32 priority);
//  void tilemap_16b_reset(screen_device &screen, struct tilemap_info *info);

	TIMER_CALLBACK_MEMBER( tilemap_16b_latch_values );

	struct rotate_info m_rotate[SEGAIC16_MAX_ROTATE];
	struct tilemap_info m_bg_tilemap[SEGAIC16_MAX_TILEMAPS];

	void set_display_enable(int enable);
	void tilemap_init(int which, int type, int colorbase, int xoffs, int numbanks);
	void rotate_init(int which, int type, int colorbase);

	DECLARE_READ16_MEMBER( tileram_r );
	DECLARE_READ16_MEMBER( textram_r );
	DECLARE_WRITE16_MEMBER( tileram_w );
	DECLARE_WRITE16_MEMBER( textram_w );

	void rotate_draw(int which, bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, bitmap_ind16 &srcbitmap);

	DECLARE_READ16_MEMBER( rotate_control_r );

	TILE_GET_INFO_MEMBER( tilemap_16b_tile_info );
	TILE_GET_INFO_MEMBER( tilemap_16b_text_info );
	TILE_GET_INFO_MEMBER( tilemap_16b_alt_tile_info );
	TILE_GET_INFO_MEMBER( tilemap_16b_alt_text_info );

	TILE_GET_INFO_MEMBER( tilemap_16a_tile_info );
	TILE_GET_INFO_MEMBER( tilemap_16a_text_info );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state
	required_device<gfxdecode_device> m_gfxdecode;
};

extern const device_type SEGAIC16VID;

#define MCFG_SEGAIC16VID_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, SEGAIC16VID, 0)

#define MCFG_SEGAIC16VID_GFXDECODE(_gfxtag) \
	segaic16_video_device::static_set_gfxdecode_tag(*device, "^" _gfxtag);

#endif
