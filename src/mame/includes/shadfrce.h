class shadfrce_state : public driver_device
{
public:
	shadfrce_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	tilemap_t *m_fgtilemap;
	tilemap_t *m_bg0tilemap;
	tilemap_t *m_bg1tilemap;

	UINT16 *m_fgvideoram;
	UINT16 *m_bg0videoram;
	UINT16 *m_bg1videoram;
	UINT16 *m_spvideoram;
	UINT16 *m_spvideoram_old;
	size_t m_spvideoram_size;

	int m_video_enable;
	int m_irqs_enable;
	int m_raster_scanline;
	int m_raster_irq_enable;
	int m_vblank;
	int m_prev_value;
	DECLARE_WRITE16_MEMBER(shadfrce_flip_screen);
	DECLARE_READ16_MEMBER(shadfrce_input_ports_r);
	DECLARE_WRITE16_MEMBER(shadfrce_sound_brt_w);
	DECLARE_WRITE16_MEMBER(shadfrce_irq_ack_w);
	DECLARE_WRITE16_MEMBER(shadfrce_irq_w);
	DECLARE_WRITE16_MEMBER(shadfrce_scanline_w);
	DECLARE_WRITE16_MEMBER(shadfrce_fgvideoram_w);
	DECLARE_WRITE16_MEMBER(shadfrce_bg0videoram_w);
	DECLARE_WRITE16_MEMBER(shadfrce_bg1videoram_w);
	DECLARE_WRITE16_MEMBER(shadfrce_bg0scrollx_w);
	DECLARE_WRITE16_MEMBER(shadfrce_bg0scrolly_w);
	DECLARE_WRITE16_MEMBER(shadfrce_bg1scrollx_w);
	DECLARE_WRITE16_MEMBER(shadfrce_bg1scrolly_w);
};


/*----------- defined in video/shadfrce.c -----------*/

VIDEO_START( shadfrce );
SCREEN_VBLANK(shadfrce);
SCREEN_UPDATE_IND16( shadfrce );
