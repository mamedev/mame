class sbugger_state : public driver_device
{
public:
	sbugger_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	UINT8 *m_videoram_attr;

	tilemap_t *m_tilemap;
	DECLARE_WRITE8_MEMBER(sbugger_videoram_w);
	DECLARE_WRITE8_MEMBER(sbugger_videoram_attr_w);
};


/*----------- defined in video/sbugger.c -----------*/

PALETTE_INIT(sbugger);
SCREEN_UPDATE_IND16(sbugger);
VIDEO_START(sbugger);
