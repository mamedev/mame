#include "video/bufsprite.h"

#define MASTER_CLOCK 57272700	// main oscillator frequency

/* Psikyo PS6406B */
#define FLIPSCREEN (((state->m_vidregs[3] & 0x0000c000) == 0x0000c000) ? 1:0)
#define DISPLAY_DISABLE (((state->m_vidregs[2] & 0x0000000f) == 0x00000006) ? 1:0)
#define BG_LARGE(n) (((state->m_vidregs[7] << (4*n)) & 0x00001000 ) ? 1:0)
#define BG_DEPTH_8BPP(n) (((state->m_vidregs[7] << (4*n)) & 0x00004000 ) ? 1:0)
#define BG_LAYER_ENABLE(n) (((state->m_vidregs[7] << (4*n)) & 0x00008000 ) ? 1:0)

#define BG_TYPE(n) (((state->m_vidregs[6] << (8*n)) & 0x7f000000 ) >> 24)
#define BG_LINE(n) (((state->m_vidregs[6] << (8*n)) & 0x80000000 ) ? 1:0)

#define BG_TRANSPEN MAKE_ARGB(0x00,0xff,0x00,0xff) // used for representing transparency in temporary bitmaps

#define SPRITE_PRI(n) (((state->m_vidregs[2] << (4*n)) & 0xf0000000 ) >> 28)


class psikyosh_state : public driver_device
{
public:
	psikyosh_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_spriteram(*this, "spriteram") { }

	/* memory pointers */
	UINT32 *       m_bgram;
	UINT32 *       m_zoomram;
	UINT32 *       m_vidregs;
	UINT32 *       m_ram;
	UINT32 *       m_paletteram;

	/* video-related */
	bitmap_ind8 m_zoom_bitmap;
	bitmap_ind16 m_z_bitmap;
	bitmap_rgb32   m_bg_bitmap;
	UINT16         *m_bg_zoom;
	UINT8          m_alphatable[256];

	/* devices */
	device_t *m_maincpu;
	required_device<buffered_spriteram32_device> m_spriteram;
	DECLARE_WRITE32_MEMBER(psikyosh_irqctrl_w);
	DECLARE_WRITE32_MEMBER(paletteram32_RRRRRRRRGGGGGGGGBBBBBBBBxxxxxxxx_dword_w);
	DECLARE_WRITE32_MEMBER(psikyosh_vidregs_w);
	DECLARE_READ32_MEMBER(mjgtaste_input_r);
};

/*----------- defined in video/psikyosh.c -----------*/

VIDEO_START( psikyosh );
SCREEN_UPDATE_RGB32( psikyosh );
SCREEN_VBLANK( psikyosh );
