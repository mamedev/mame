// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Sega 16-bit common hardware

***************************************************************************/
#ifndef MAME_VIDEO_SEGAIC16_H
#define MAME_VIDEO_SEGAIC16_H

#pragma once

typedef device_delegate<void (int, uint16_t*, uint16_t*, uint16_t*, uint16_t*)> segaic16_video_pagelatch_delegate;


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/



class segaic16_video_device :   public device_t,
								public device_video_interface
{
public:
	/* tilemap systems */
	static constexpr unsigned MAX_TILEMAPS       = 1;

	static constexpr unsigned TILEMAP_HANGON     = 0;
	static constexpr unsigned TILEMAP_16A        = 1;
	static constexpr unsigned TILEMAP_16B        = 2;
	static constexpr unsigned TILEMAP_16B_ALT    = 3;

	static constexpr unsigned TILEMAP_FOREGROUND = 0;
	static constexpr unsigned TILEMAP_BACKGROUND = 1;
	static constexpr unsigned TILEMAP_TEXT       = 2;



	/* rotation systems */
	static constexpr unsigned MAX_ROTATE         = 1;

	static constexpr unsigned ROTATE_YBOARD      = 0;



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

	template <typename T> segaic16_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&decode_tag)
		: segaic16_video_device(mconfig, tag, owner, clock)
	{
		m_gfxdecode.set_tag(std::forward<T>(decode_tag));
	}

	segaic16_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	template <typename... T> void set_pagelatch_cb(T &&... args) { m_pagelatch_cb = segaic16_video_pagelatch_delegate(std::forward<T>(args)...); }

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
	TIMER_CALLBACK_MEMBER( tilemap_16b_latch_values );

	struct rotate_info m_rotate[MAX_ROTATE];
	struct tilemap_info m_bg_tilemap[MAX_TILEMAPS];

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

DECLARE_DEVICE_TYPE(SEGAIC16VID, segaic16_video_device)

#endif // MAME_VIDEO_SEGAIC16_H
