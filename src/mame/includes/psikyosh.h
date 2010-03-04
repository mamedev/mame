
#define MASTER_CLOCK 57272700	// main oscillator frequency

/* Psikyo PS6406B */
#define FLIPSCREEN (((state->vidregs[3] & 0x0000c000) == 0x0000c000) ? 1:0)
#define DISPLAY_DISABLE (((state->vidregs[2] & 0x0000000f) == 0x00000006) ? 1:0)
#define BG_LARGE(n) (((state->vidregs[7] << (4*n)) & 0x00001000 ) ? 1:0)
#define BG_DEPTH_8BPP(n) (((state->vidregs[7] << (4*n)) & 0x00004000 ) ? 1:0)
#define BG_LAYER_ENABLE(n) (((state->vidregs[7] << (4*n)) & 0x00008000 ) ? 1:0)

#define BG_TYPE(n) (((state->vidregs[6] << (8*n)) & 0x7f000000 ) >> 24)
#define BG_LINE(n) (((state->vidregs[6] << (8*n)) & 0x80000000 ) ? 1:0)

#define BG_TRANSPEN MAKE_ARGB(0x00,0xff,0x00,0xff) // used for representing transparency in temporary bitmaps

#define SPRITE_PRI(n) (((state->vidregs[2] << (4*n)) & 0xf0000000 ) >> 28)


class psikyosh_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, psikyosh_state(machine)); }

	psikyosh_state(running_machine &machine) { }

	/* memory pointers */
	UINT32 *       bgram;
	UINT32 *       zoomram;
	UINT32 *       vidregs;
	UINT32 *       ram;
	UINT32 *       paletteram;
//  UINT32 *       spriteram;   // currently this uses generic buffered spriteram
//  size_t         spriteram_size;

	/* video-related */
	bitmap_t       *zoom_bitmap, *z_bitmap, *bg_bitmap;
	UINT16         *bg_zoom;
//  UINT8          *alphatable;

	/* misc */
	UINT32         sample_offs;	// only used if ROMTEST = 1

	/* devices */
	running_device *maincpu;
};

/*----------- defined in video/psikyosh.c -----------*/

VIDEO_START( psikyosh );
VIDEO_UPDATE( psikyosh );
VIDEO_EOF( psikyosh );
