/*************************************************************************

    Double Dragon & Double Dragon II (but also China Gate)

*************************************************************************/


class ddragon_state : public driver_device
{
public:
	ddragon_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_rambase(*this, "rambase"),
		m_bgvideoram(*this, "bgvideoram"),
		m_fgvideoram(*this, "fgvideoram"),
		m_spriteram(*this, "spriteram"),
		m_scrollx_lo(*this, "scrollx_lo"),
		m_scrolly_lo(*this, "scrolly_lo"),
		m_darktowr_mcu_ports(*this, "darktowr_mcu"){ }

	/* memory pointers */
	optional_shared_ptr<UINT8> m_rambase;
	required_shared_ptr<UINT8> m_bgvideoram;
	required_shared_ptr<UINT8> m_fgvideoram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_scrollx_lo;
	required_shared_ptr<UINT8> m_scrolly_lo;
	optional_shared_ptr<UINT8> m_darktowr_mcu_ports;
//  UINT8 *        m_paletteram;  // currently this uses generic palette handling
//  UINT8 *        m_paletteram_2;    // currently this uses generic palette handling

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
	DECLARE_CUSTOM_INPUT_MEMBER(sub_cpu_busy);
	DECLARE_WRITE8_MEMBER(ddragon_bankswitch_w);
	DECLARE_WRITE8_MEMBER(toffy_bankswitch_w);
	DECLARE_READ8_MEMBER(darktowr_mcu_bank_r);
	DECLARE_WRITE8_MEMBER(darktowr_mcu_bank_w);
	DECLARE_WRITE8_MEMBER(darktowr_bankswitch_w);
	DECLARE_WRITE8_MEMBER(ddragon_interrupt_w);
	DECLARE_WRITE8_MEMBER(ddragon2_sub_irq_ack_w);
	DECLARE_WRITE8_MEMBER(ddragon2_sub_irq_w);
	DECLARE_WRITE8_MEMBER(darktowr_mcu_w);
	DECLARE_READ8_MEMBER(ddragon_hd63701_internal_registers_r);
	DECLARE_WRITE8_MEMBER(ddragon_hd63701_internal_registers_w);
	DECLARE_READ8_MEMBER(ddragon_spriteram_r);
	DECLARE_WRITE8_MEMBER(ddragon_spriteram_w);
	DECLARE_WRITE8_MEMBER(dd_adpcm_w);
	DECLARE_READ8_MEMBER(dd_adpcm_status_r);
	DECLARE_WRITE8_MEMBER(ddragonba_port_w);
	DECLARE_DRIVER_INIT(toffy);
	DECLARE_DRIVER_INIT(darktowr);
	DECLARE_DRIVER_INIT(ddragon2);
	DECLARE_DRIVER_INIT(ddragon);
	DECLARE_DRIVER_INIT(ddragon6809);
	DECLARE_DRIVER_INIT(chinagat);
	TILEMAP_MAPPER_MEMBER(background_scan);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_16color_tile_info);
};


/*----------- defined in video/ddragon.c -----------*/


VIDEO_START( chinagat );
VIDEO_START( ddragon );
SCREEN_UPDATE_IND16( ddragon );

