
#define MASTER_CLOCK 57272700	// main oscillator frequency

/* Psikyo PS6406B */
#define FLIPSCREEN (((state->vidregs[3] & 0x0000c000) == 0x0000c000) ? 1:0)
#define DISPLAY_DISABLE (((state->vidregs[2] & 0x0000000f) == 0x00000006) ? 1:0)
#define BG_LARGE(n) (((state->vidregs[7] << (4*n)) & 0x00001000 ) ? 1:0)
#define BG_DEPTH_8BPP(n) (((state->vidregs[7] << (4*n)) & 0x00004000 ) ? 1:0)
#define BG_LAYER_ENABLE(n) (((state->vidregs[7] << (4*n)) & 0x00008000 ) ? 1:0)

#define BG_TYPE(n) (((state->vidregs[6] << (8*n)) & 0x7f000000 ) >> 24)
#define BG_NORMAL      0x0a
#define BG_NORMAL_ALT  0x0b /* Same as above but with different location for scroll/priority reg */

/* All below have 0x80 bit set, probably row/linescroll enable/toggle */
#define BG_SCROLL_0C   0x0c /* 224 v/h scroll values in bank 0x0c; Used in daraku, for text */
#define BG_SCROLL_0D   0x0d /* 224 v/h scroll values in bank 0x0d; Used in daraku, for alternate characters of text */
#define BG_SCROLL_ZOOM 0x0e /* 224 v/h scroll values in bank 0x0e-0x1f; Used in s1945ii, s1945iii */

enum { eeprom_0=0, eeprom_DEFAULT=1, eeprom_DARAKU, eeprom_S1945III, eeprom_DRAGNBLZ, eeprom_GNBARICH, eeprom_USER1, eeprom_MJGTASTE };


typedef struct _psikyosh_state psikyosh_state;
struct _psikyosh_state
{
	/* memory pointers */
	UINT32 *       bgram;
	UINT32 *       zoomram;
	UINT32 *       vidregs;
	UINT32 *       ram;
	UINT32 *       paletteram;
//  UINT32 *       spriteram;   // currently this uses generic buffered spriteram
//  size_t         spriteram_size;

	/* video-related */
	bitmap_t       *zoom_bitmap, *z_bitmap;
//  UINT8          alphatable[256];

	/* misc */
	UINT32         sample_offs;	// only used if ROMTEST = 1

	/* devices */
	const device_config *maincpu;
};

/*----------- defined in video/psikyosh.c -----------*/

VIDEO_START( psikyosh );
VIDEO_UPDATE( psikyosh );
VIDEO_EOF( psikyosh );
