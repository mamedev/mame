/*************************************************************************

    Double Dragon & Double Dragon II (but also China Gate)

*************************************************************************/


typedef struct _ddragon_state ddragon_state;
struct _ddragon_state
{
	/* memory pointers */
	UINT8 *        rambase;
	UINT8 *        bgvideoram;
	UINT8 *        fgvideoram;
	UINT8 *        spriteram;
	UINT8 *        scrollx_lo;
	UINT8 *        scrolly_lo;
	UINT8 *        darktowr_mcu_ports;
//  UINT8 *        paletteram;  // currently this uses generic palette handling
//  UINT8 *        paletteram_2;    // currently this uses generic palette handling
	size_t         spriteram_size;	// FIXME: this appears in chinagat.c, but is it really used?

	/* video-related */
	tilemap        *fg_tilemap, *bg_tilemap;
	UINT8          technos_video_hw;
	UINT8          scrollx_hi;
	UINT8          scrolly_hi;

	/* misc */
	UINT8          dd_sub_cpu_busy;
	UINT8          sprite_irq, sound_irq, ym_irq, adpcm_sound_irq;
	UINT32         adpcm_pos[2], adpcm_end[2];
	UINT8          adpcm_idle[2];
	int            adpcm_data[2];

	/* for Sai Yu Gou Ma Roku */
	int            adpcm_addr;
	int            i8748_P1;
	int            i8748_P2;
	int            pcm_shift;
	int            pcm_nibble;
	int            mcu_command;
#if 0
	int            m5205_clk;
#endif

	/* devices */
	const device_config *maincpu;
	const device_config *snd_cpu;
	const device_config *sub_cpu;
	const device_config *adpcm_1;
	const device_config *adpcm_2;
};


/*----------- defined in video/ddragon.c -----------*/

WRITE8_HANDLER( ddragon_bgvideoram_w );
WRITE8_HANDLER( ddragon_fgvideoram_w );

VIDEO_START( chinagat );
VIDEO_START( ddragon );
VIDEO_UPDATE( ddragon );

