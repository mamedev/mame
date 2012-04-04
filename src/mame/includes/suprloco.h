class suprloco_state : public driver_device
{
public:
	suprloco_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	UINT8 *m_scrollram;
	tilemap_t *m_bg_tilemap;
	int m_control;
	UINT8 *m_spriteram;
	size_t m_spriteram_size;
	DECLARE_WRITE8_MEMBER(suprloco_soundport_w);
};


/*----------- defined in video/suprloco.c -----------*/

PALETTE_INIT( suprloco );
VIDEO_START( suprloco );
SCREEN_UPDATE_IND16( suprloco );
WRITE8_HANDLER( suprloco_videoram_w );
WRITE8_HANDLER( suprloco_scrollram_w );
WRITE8_HANDLER( suprloco_control_w );
READ8_HANDLER( suprloco_control_r );
