/**************** Machine stuff ******************/
//#define USE_HD64x180          /* Define if CPU support is available */
//#define TRUXTON2_STEREO       /* Uncomment to hear truxton2 music in stereo */

// We encode priority with colour in the tilemaps, so need a larger palette
#define T2PALETTE_LENGTH 0x10000

// VDP related
#include "video/gp9001.h"

// Cache the CPUs and VDPs for faster access
class toaplan2_state : public driver_device
{
public:
	toaplan2_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config)
	{
		vdp0 = NULL;
		vdp1 = NULL;
	}

	gp9001vdp_device* vdp0;
	gp9001vdp_device* vdp1;

	UINT8 *shared_ram;		/* 8 bit RAM shared between 68K and sound CPU */
	UINT16 *shared_ram16;	/* Really 8 bit RAM connected to Z180 */

	device_t *main_cpu;
	device_t *sub_cpu;

	UINT16 mcu_data;
	UINT16 video_status;
	INT8 old_p1_paddle_h;	/* For Ghox */
	INT8 old_p2_paddle_h;
	UINT8 v25_reset_line;	/* 0x20 for dogyuun/batsugun, 0x10 for vfive, 0x08 for fixeight */
	UINT8 sndirq_line;		/* IRQ4 for batrider, IRQ2 for bbakraid */
	UINT8 z80_busreq;

	UINT16 *txvideoram16;
	UINT16 *txvideoram16_offs;
	UINT16 *txscrollram16;
	UINT16 *tx_gfxram16;
	UINT16 *mainram16;

	size_t tx_vram_size;
	size_t tx_offs_vram_size;
	size_t tx_scroll_vram_size;
	size_t paletteram_size;
	size_t mainram_overlap_size;

	bitmap_t* custom_priority_bitmap;
	bitmap_t* secondary_render_bitmap;

	tilemap_t *tx_tilemap;	/* Tilemap for extra-text-layer */
	UINT8 tx_flip;
};

/*----------- defined in video/toaplan2.c -----------*/

VIDEO_START( toaplan2 );
VIDEO_START( truxton2 );
VIDEO_START( fixeightbl );
VIDEO_START( bgaregga );
VIDEO_START( batrider );

SCREEN_UPDATE( toaplan2 );
SCREEN_UPDATE( truxton2 );
SCREEN_UPDATE( batrider );
SCREEN_UPDATE( dogyuun );
SCREEN_UPDATE( batsugun );

SCREEN_EOF( toaplan2 );

/* non-vdp text layer */
WRITE16_HANDLER( toaplan2_txvideoram16_w );
WRITE16_HANDLER( toaplan2_txvideoram16_offs_w );
WRITE16_HANDLER( toaplan2_txscrollram16_w );
WRITE16_HANDLER( toaplan2_tx_gfxram16_w );

WRITE16_HANDLER( batrider_textdata_dma_w );
WRITE16_HANDLER( batrider_unknown_dma_w );
WRITE16_HANDLER( batrider_objectbank_w );
