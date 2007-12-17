/*----------- defined in drivers/psikyosh.c -----------*/

#define MASTER_CLOCK 57272700	// main oscillator frequency

extern UINT32 *psikyosh_bgram, *psikyosh_zoomram, *psikyosh_vidregs;

/*----------- defined in video/psikyosh.c -----------*/

VIDEO_START( psikyosh );
VIDEO_UPDATE( psikyosh );
VIDEO_EOF( psikyosh );

enum { EEPROM_0=0, EEPROM_DEFAULT=1, EEPROM_DARAKU, EEPROM_S1945III, EEPROM_DRAGNBLZ, EEPROM_GNBARICH, EEPROM_USER1, EEPROM_MJGTASTE };

/* Psikyo PS6406B */
#define FLIPSCREEN (((psikyosh_vidregs[3] & 0x0000c000) == 0x0000c000) ? 1:0)
#define DISPLAY_DISABLE (((psikyosh_vidregs[2] & 0x0000000f) == 0x00000006) ? 1:0)
#define BG_LARGE(n) (((psikyosh_vidregs[7] << (4*n)) & 0x00001000 ) ? 1:0)
#define BG_DEPTH_8BPP(n) (((psikyosh_vidregs[7] << (4*n)) & 0x00004000 ) ? 1:0)
#define BG_LAYER_ENABLE(n) (((psikyosh_vidregs[7] << (4*n)) & 0x00008000 ) ? 1:0)

#define BG_TYPE(n) (((psikyosh_vidregs[6] << (8*n)) & 0x7f000000 ) >> 24)
#define BG_NORMAL      0x0a
#define BG_NORMAL_ALT  0x0b /* Same as above but with different location for scroll/priority reg */

/* All below have 0x80 bit set, probably row/linescroll enable/toggle */
#define BG_SCROLL_0C   0x0c /* 224 v/h scroll values in bank 0x0c; Used in daraku, for text */
#define BG_SCROLL_0D   0x0d /* 224 v/h scroll values in bank 0x0d; Used in daraku, for alternate characters of text */
#define BG_SCROLL_ZOOM 0x0e /* 224 v/h scroll values in bank 0x0e-0x1f; Used in s1945ii, s1945iii */
