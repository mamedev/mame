class mainsnk_state : public driver_device
{
public:
	mainsnk_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	tilemap_t *m_tx_tilemap;
	tilemap_t *m_bg_tilemap;
	UINT8 *m_spriteram;
	UINT8 *m_fgram;
	UINT8 *m_bgram;

	int m_sound_cpu_busy;
	UINT32 m_bg_tile_offset;
	DECLARE_WRITE8_MEMBER(sound_command_w);
	DECLARE_READ8_MEMBER(sound_command_r);
	DECLARE_READ8_MEMBER(sound_ack_r);
	DECLARE_WRITE8_MEMBER(mainsnk_c600_w);
	DECLARE_WRITE8_MEMBER(mainsnk_fgram_w);
	DECLARE_WRITE8_MEMBER(mainsnk_bgram_w);
};


/*----------- defined in video/mainsnk.c -----------*/

PALETTE_INIT(mainsnk);
VIDEO_START(mainsnk);
SCREEN_UPDATE_IND16(mainsnk);
