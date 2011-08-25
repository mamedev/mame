#define RASTER_LINES 262
#define FIRST_VISIBLE_LINE 0
#define LAST_VISIBLE_LINE 223

class hyprduel_state : public driver_device
{
public:
	hyprduel_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *  m_videoregs;
	UINT16 *  m_screenctrl;
	UINT16 *  m_tiletable_old;
	UINT16 *  m_tiletable;
	UINT16 *  m_vram_0;
	UINT16 *  m_vram_1;
	UINT16 *  m_vram_2;
	UINT16 *  m_window;
	UINT16 *  m_scroll;
	UINT16 *  m_rombank;
	UINT16 *  m_blitter_regs;
	UINT16 *  m_irq_enable;
	UINT16 *  m_sharedram1;
	UINT16 *  m_sharedram3;
	UINT16 *  m_spriteram;
	UINT16 *  m_paletteram;
	size_t    m_tiletable_size;
	size_t    m_spriteram_size;

	/* video-related */
	tilemap_t   *m_bg_tilemap[3];
	UINT8     *m_empty_tiles;
	UINT8     *m_dirtyindex;
	int       m_sprite_xoffs;
	int       m_sprite_yoffs;
	int       m_sprite_yoffs_sub;

	/* misc */
	emu_timer *m_magerror_irq_timer;
	int       m_blitter_bit;
	int       m_requested_int;
	int       m_subcpu_resetline;
	int       m_cpu_trigger;
	int       m_int_num;

	/* devices */
	device_t *m_maincpu;
	device_t *m_subcpu;
};



/*----------- defined in video/hyprduel.c -----------*/


WRITE16_HANDLER( hyprduel_paletteram_w );
WRITE16_HANDLER( hyprduel_window_w );
WRITE16_HANDLER( hyprduel_vram_0_w );
WRITE16_HANDLER( hyprduel_vram_1_w );
WRITE16_HANDLER( hyprduel_vram_2_w );
WRITE16_HANDLER( hyprduel_scrollreg_w );
WRITE16_HANDLER( hyprduel_scrollreg_init_w );

VIDEO_START( hyprduel_14220 );
VIDEO_START( magerror_14220 );
SCREEN_UPDATE( hyprduel );
