class sbugger_state : public driver_device
{
public:
	sbugger_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	UINT8 *m_videoram_attr;

	tilemap_t *m_tilemap;
};


/*----------- defined in video/sbugger.c -----------*/

PALETTE_INIT(sbugger);
SCREEN_UPDATE(sbugger);
VIDEO_START(sbugger);
WRITE8_HANDLER( sbugger_videoram_attr_w );
WRITE8_HANDLER( sbugger_videoram_w );
