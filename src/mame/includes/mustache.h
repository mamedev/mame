class mustache_state : public driver_device
{
public:
	mustache_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	emu_timer *m_clear_irq_timer;
	tilemap_t *m_bg_tilemap;
	int m_control_byte;
	UINT8 *m_spriteram;
	size_t m_spriteram_size;
	DECLARE_WRITE8_MEMBER(mustache_videoram_w);
	DECLARE_WRITE8_MEMBER(mustache_video_control_w);
	DECLARE_WRITE8_MEMBER(mustache_scroll_w);
};


/*----------- defined in video/mustache.c -----------*/

VIDEO_START( mustache );
SCREEN_UPDATE_IND16( mustache );
PALETTE_INIT( mustache );
