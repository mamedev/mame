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
	toaplan2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{
		m_vdp0 = NULL;
		m_vdp1 = NULL;
	}

	gp9001vdp_device* m_vdp0;
	gp9001vdp_device* m_vdp1;

	UINT8 *m_shared_ram;		/* 8 bit RAM shared between 68K and sound CPU */
	UINT16 *m_shared_ram16;	/* Really 8 bit RAM connected to Z180 */

	device_t *m_main_cpu;
	device_t *m_sub_cpu;

	UINT16 m_mcu_data;
	UINT16 m_video_status;
	INT8 m_old_p1_paddle_h;	/* For Ghox */
	INT8 m_old_p2_paddle_h;
	UINT8 m_v25_reset_line;	/* 0x20 for dogyuun/batsugun, 0x10 for vfive, 0x08 for fixeight */
	UINT8 m_sndirq_line;		/* IRQ4 for batrider, IRQ2 for bbakraid */
	UINT8 m_z80_busreq;

	UINT16 *m_txvideoram16;
	UINT16 *m_txvideoram16_offs;
	UINT16 *m_txscrollram16;
	UINT16 *m_tx_gfxram16;
	UINT16 *m_mainram16;

	size_t m_tx_vram_size;
	size_t m_tx_offs_vram_size;
	size_t m_tx_scroll_vram_size;
	size_t m_paletteram_size;
	size_t m_mainram_overlap_size;

	bitmap_ind8 m_custom_priority_bitmap;
	bitmap_ind16 m_secondary_render_bitmap;

	tilemap_t *m_tx_tilemap;	/* Tilemap for extra-text-layer */
	UINT8 m_tx_flip;
};

/*----------- defined in video/toaplan2.c -----------*/

VIDEO_START( toaplan2 );
VIDEO_START( truxton2 );
VIDEO_START( fixeightbl );
VIDEO_START( bgaregga );
VIDEO_START( batrider );
VIDEO_START( bgareggabl );

SCREEN_UPDATE_IND16( toaplan2 );
SCREEN_UPDATE_IND16( truxton2 );
SCREEN_UPDATE_IND16( batrider );
SCREEN_UPDATE_IND16( dogyuun );
SCREEN_UPDATE_IND16( batsugun );

SCREEN_VBLANK( toaplan2 );

/* non-vdp text layer */
WRITE16_HANDLER( toaplan2_txvideoram16_w );
WRITE16_HANDLER( toaplan2_txvideoram16_offs_w );
WRITE16_HANDLER( toaplan2_txscrollram16_w );
WRITE16_HANDLER( toaplan2_tx_gfxram16_w );

WRITE16_HANDLER( batrider_textdata_dma_w );
WRITE16_HANDLER( batrider_unknown_dma_w );
WRITE16_HANDLER( batrider_objectbank_w );
