class mainsnk_state : public driver_device
{
public:
	mainsnk_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_bgram(*this, "bgram"),
		m_spriteram(*this, "spriteram"),
		m_fgram(*this, "fgram"){ }

	tilemap_t *m_tx_tilemap;
	tilemap_t *m_bg_tilemap;
	required_shared_ptr<UINT8> m_bgram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_fgram;

	int m_sound_cpu_busy;
	UINT32 m_bg_tile_offset;
	DECLARE_WRITE8_MEMBER(sound_command_w);
	DECLARE_READ8_MEMBER(sound_command_r);
	DECLARE_READ8_MEMBER(sound_ack_r);
	DECLARE_WRITE8_MEMBER(mainsnk_c600_w);
	DECLARE_WRITE8_MEMBER(mainsnk_fgram_w);
	DECLARE_WRITE8_MEMBER(mainsnk_bgram_w);
	DECLARE_CUSTOM_INPUT_MEMBER(mainsnk_sound_r);
	TILEMAP_MAPPER_MEMBER(marvins_tx_scan_cols);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
};


/*----------- defined in video/mainsnk.c -----------*/

PALETTE_INIT(mainsnk);
VIDEO_START(mainsnk);
SCREEN_UPDATE_IND16(mainsnk);
