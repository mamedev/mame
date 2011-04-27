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
};


/*----------- defined in video/shadfrce.c -----------*/

WRITE16_HANDLER ( shadfrce_bg0scrollx_w );
WRITE16_HANDLER ( shadfrce_bg1scrollx_w );
WRITE16_HANDLER ( shadfrce_bg0scrolly_w );
WRITE16_HANDLER ( shadfrce_bg1scrolly_w );
VIDEO_START( shadfrce );
SCREEN_EOF(shadfrce);
SCREEN_UPDATE( shadfrce );
WRITE16_HANDLER( shadfrce_fgvideoram_w );
WRITE16_HANDLER( shadfrce_bg0videoram_w );
WRITE16_HANDLER( shadfrce_bg1videoram_w );
