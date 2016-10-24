// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Sega 16-bit common hardware

***************************************************************************/

#pragma once
#ifndef __SEGAIC16VID_H__
#define __SEGAIC16VID_H__

typedef device_delegate<void (int, uint16_t*, uint16_t*, uint16_t*, uint16_t*)> segaic16_video_pagelatch_delegate;

#define MCFG_SEGAIC16_VIDEO_SET_PAGELATCH_CB( _class, _method) \
	segaic16_video_device::set_pagelatch_cb(*device, segaic16_video_pagelatch_delegate(&_class::_method, #_class "::" #_method, nullptr, (_class *)nullptr));


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
	uint16_t *        rambase;                        /* base of RAM for this tilemap page */
	const uint8_t *   bank;                           /* pointer to bank array */
	uint16_t          banksize;                       /* size of banks */
};


struct tilemap_info
{
	uint8_t           index;                          /* index of this structure */
	uint8_t           type;                           /* type of tilemap (see segaic16.h for details) */
	uint8_t           numpages;                       /* number of allocated pages */
	uint8_t           flip;                           /* screen flip? */
	uint8_t           rowscroll, colscroll;           /* are rowscroll/colscroll enabled (if external enables are used) */
	uint8_t           bank[8];                        /* indexes of the tile banks */
	uint16_t          banksize;                       /* number of tiles per bank */
	uint16_t          latched_xscroll[4];             /* latched X scroll values */
	uint16_t          latched_yscroll[4];             /* latched Y scroll values */
	uint16_t          latched_pageselect[4];          /* latched page select values */
	int32_t           xoffs;                          /* X scroll offset */
	tilemap_t *     tilemaps[16];                   /* up to 16 tilemap pages */
	tilemap_t *     textmap;                        /* a single text tilemap */
	struct tilemap_callback_info tmap_info[16];     /* callback info for 16 tilemap pages */
	struct tilemap_callback_info textmap_info;      /* callback info for a single textmap page */
	void            (*reset)(screen_device &screen, struct tilemap_info *info);/* reset callback */
	void            (*draw_layer)(screen_device &screen, struct tilemap_info *info, bitmap_ind16 &bitmap, const rectangle &cliprect, int which, int flags, int priority);
	uint16_t *        textram;                        /* pointer to textram pointer */
	uint16_t *        tileram;                        /* pointer to tileram pointer */
	emu_timer *     latch_timer;                    /* timer for latching 16b tilemap scroll values */
};


struct rotate_info
{
	uint8_t           index;                          /* index of this structure */
	uint8_t           type;                           /* type of rotate system (see segaic16.h for details) */
	uint16_t          colorbase;                      /* base color index */
	int32_t           ramsize;                        /* size of rotate RAM */
	uint16_t *        rotateram;                      /* pointer to rotateram pointer */
	std::unique_ptr<uint16_t[]>        buffer;                         /* buffered data */
};


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/



class segaic16_video_device :   public device_t,
								public device_video_interface
{
public:
	segaic16_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~segaic16_video_device() {}

	// static configuration
	static void static_set_gfxdecode_tag(device_t &device, const char *tag);
	static void set_pagelatch_cb(device_t &device,segaic16_video_pagelatch_delegate newtilecb);

	uint8_t m_display_enable;
	optional_shared_ptr<uint16_t> m_tileram;
	optional_shared_ptr<uint16_t> m_textram;
	optional_shared_ptr<uint16_t> m_rotateram;

	void tilemap_set_colscroll(int which, int enable);
	void tilemap_set_rowscroll(int which, int enable);
	void tilemap_set_flip(int which, int flip);
	void tilemap_set_bank(int which, int banknum, int offset);
	void tilemap_reset(screen_device &screen);
	void tilemap_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int which, int map, int priority, int priority_mark);
//  void tilemap_16b_draw_layer(screen_device &screen, struct tilemap_info *info, bitmap_ind16 &bitmap, const rectangle &cliprect, int which, int flags, int priority);
//  void tilemap_16a_draw_layer(screen_device &screen, struct tilemap_info *info, bitmap_ind16 &bitmap, const rectangle &cliprect, int which, int flags, int priority);
//  void draw_virtual_tilemap(screen_device &screen, struct tilemap_info *info, bitmap_ind16 &bitmap, const rectangle &cliprect, uint16_t pages, uint16_t xscroll, uint16_t yscroll, uint32_t flags, uint32_t priority);
//  void tilemap_16b_reset(screen_device &screen, struct tilemap_info *info);

	segaic16_video_pagelatch_delegate m_pagelatch_cb;
	void tilemap_16b_fill_latch(int i, uint16_t* latched_pageselect, uint16_t* latched_yscroll, uint16_t* latched_xscroll, uint16_t* textram);
	void tilemap_16b_latch_values(void *ptr, int32_t param);

	struct rotate_info m_rotate[SEGAIC16_MAX_ROTATE];
	struct tilemap_info m_bg_tilemap[SEGAIC16_MAX_TILEMAPS];

	void set_display_enable(int enable);
	void tilemap_init(int which, int type, int colorbase, int xoffs, int numbanks);
	void rotate_init(int which, int type, int colorbase);

	uint16_t tileram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t textram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void tileram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void textram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	void rotate_draw(int which, bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, bitmap_ind16 &srcbitmap);

	uint16_t rotate_control_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);

	void tilemap_16b_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void tilemap_16b_text_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void tilemap_16b_alt_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void tilemap_16b_alt_text_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	void tilemap_16a_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void tilemap_16a_text_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

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
