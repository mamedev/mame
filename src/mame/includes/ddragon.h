/*************************************************************************

    Double Dragon & Double Dragon II (but also China Gate)

*************************************************************************/


class ddragon_state : public driver_device
{
public:
	ddragon_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *        m_rambase;
	UINT8 *        m_bgvideoram;
	UINT8 *        m_fgvideoram;
	UINT8 *        m_spriteram;
	UINT8 *        m_scrollx_lo;
	UINT8 *        m_scrolly_lo;
	UINT8 *        m_darktowr_mcu_ports;
//  UINT8 *        m_paletteram;  // currently this uses generic palette handling
//  UINT8 *        m_paletteram_2;    // currently this uses generic palette handling
	size_t         m_spriteram_size;	// FIXME: this appears in chinagat.c, but is it really used?

	/* video-related */
	tilemap_t        *m_fg_tilemap;
	tilemap_t        *m_bg_tilemap;
	UINT8          m_technos_video_hw;
	UINT8          m_scrollx_hi;
	UINT8          m_scrolly_hi;

	/* misc */
	UINT8          m_dd_sub_cpu_busy;
	UINT8          m_sprite_irq;
	UINT8          m_sound_irq;
	UINT8          m_ym_irq;
	UINT8          m_adpcm_sound_irq;
	UINT32         m_adpcm_pos[2];
	UINT32         m_adpcm_end[2];
	UINT8          m_adpcm_idle[2];
	int            m_adpcm_data[2];

	/* for Sai Yu Gou Ma Roku */
	int            m_adpcm_addr;
	int            m_i8748_P1;
	int            m_i8748_P2;
	int            m_pcm_shift;
	int            m_pcm_nibble;
	int            m_mcu_command;
#if 0
	int            m_m5205_clk;
#endif

	/* devices */
	device_t *m_maincpu;
	device_t *m_snd_cpu;
	device_t *m_sub_cpu;
	device_t *m_adpcm_1;
	device_t *m_adpcm_2;
	DECLARE_WRITE8_MEMBER(ddragon_bgvideoram_w);
	DECLARE_WRITE8_MEMBER(ddragon_fgvideoram_w);
};


/*----------- defined in video/ddragon.c -----------*/


VIDEO_START( chinagat );
VIDEO_START( ddragon );
SCREEN_UPDATE_IND16( ddragon );

