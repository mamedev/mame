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
	DECLARE_READ16_MEMBER(video_count_r);
	DECLARE_WRITE8_MEMBER(toaplan2_coin_w);
	DECLARE_WRITE16_MEMBER(toaplan2_coin_word_w);
	DECLARE_WRITE16_MEMBER(toaplan2_v25_coin_word_w);
	DECLARE_WRITE16_MEMBER(shippumd_coin_word_w);
	DECLARE_READ16_MEMBER(shared_ram_r);
	DECLARE_WRITE16_MEMBER(shared_ram_w);
	DECLARE_WRITE16_MEMBER(toaplan2_hd647180_cpu_w);
	DECLARE_READ16_MEMBER(ghox_p1_h_analog_r);
	DECLARE_READ16_MEMBER(ghox_p2_h_analog_r);
	DECLARE_READ16_MEMBER(ghox_mcu_r);
	DECLARE_WRITE16_MEMBER(ghox_mcu_w);
	DECLARE_READ16_MEMBER(ghox_shared_ram_r);
	DECLARE_WRITE16_MEMBER(ghox_shared_ram_w);
	DECLARE_WRITE16_MEMBER(fixeight_subcpu_ctrl_w);
	DECLARE_WRITE16_MEMBER(fixeightbl_oki_bankswitch_w);
	DECLARE_READ8_MEMBER(v25_dswa_r);
	DECLARE_READ8_MEMBER(v25_dswb_r);
	DECLARE_READ8_MEMBER(v25_jmpr_r);
	DECLARE_READ8_MEMBER(fixeight_region_r);
	DECLARE_WRITE8_MEMBER(raizing_z80_bankswitch_w);
	DECLARE_WRITE8_MEMBER(raizing_oki_bankswitch_w);
	DECLARE_WRITE16_MEMBER(bgaregga_soundlatch_w);
	DECLARE_READ8_MEMBER(bgaregga_E01D_r);
	DECLARE_WRITE8_MEMBER(bgaregga_E00C_w);
	DECLARE_READ16_MEMBER(batrider_z80_busack_r);
	DECLARE_WRITE16_MEMBER(batrider_z80_busreq_w);
	DECLARE_READ16_MEMBER(batrider_z80rom_r);
	DECLARE_WRITE16_MEMBER(batrider_soundlatch_w);
	DECLARE_WRITE16_MEMBER(batrider_soundlatch2_w);
	DECLARE_WRITE16_MEMBER(batrider_unknown_sound_w);
	DECLARE_WRITE16_MEMBER(batrider_clear_sndirq_w);
	DECLARE_WRITE8_MEMBER(batrider_sndirq_w);
	DECLARE_WRITE8_MEMBER(batrider_clear_nmi_w);
	DECLARE_READ16_MEMBER(bbakraid_eeprom_r);
	DECLARE_WRITE16_MEMBER(bbakraid_eeprom_w);
	DECLARE_WRITE16_MEMBER(toaplan2_txvideoram16_w);
	DECLARE_WRITE16_MEMBER(toaplan2_txvideoram16_offs_w);
	DECLARE_WRITE16_MEMBER(toaplan2_txscrollram16_w);
	DECLARE_WRITE16_MEMBER(toaplan2_tx_gfxram16_w);
	DECLARE_WRITE16_MEMBER(batrider_textdata_dma_w);
	DECLARE_WRITE16_MEMBER(batrider_unknown_dma_w);
	DECLARE_WRITE16_MEMBER(batrider_objectbank_w);
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

