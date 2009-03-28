#define RASTER_LINES 262
#define FIRST_VISIBLE_LINE 0
#define LAST_VISIBLE_LINE 223

/*----------- defined in video/hyprduel.c -----------*/

extern UINT16 *hyprduel_videoregs;
extern UINT16 *hyprduel_screenctrl;
extern UINT16 *hyprduel_tiletable;
extern size_t hyprduel_tiletable_size;
extern UINT16 *hyprduel_vram_0, *hyprduel_vram_1, *hyprduel_vram_2;
extern UINT16 *hyprduel_window;
extern UINT16 *hyprduel_scroll;

WRITE16_HANDLER( hyprduel_paletteram_w );
WRITE16_HANDLER( hyprduel_window_w );
WRITE16_HANDLER( hyprduel_vram_0_w );
WRITE16_HANDLER( hyprduel_vram_1_w );
WRITE16_HANDLER( hyprduel_vram_2_w );
WRITE16_HANDLER( hyprduel_scrollreg_w );
WRITE16_HANDLER( hyprduel_scrollreg_init_w );
VIDEO_START( hyprduel_14220 );
VIDEO_UPDATE( hyprduel );

