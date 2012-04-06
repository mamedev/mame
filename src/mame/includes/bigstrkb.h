class bigstrkb_state : public driver_device
{
public:
	bigstrkb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	tilemap_t *m_tilemap;
	tilemap_t *m_tilemap2;
	tilemap_t *m_tilemap3;

	UINT16 *m_videoram;
	UINT16 *m_videoram2;
	UINT16 *m_videoram3;

	UINT16 *m_vidreg1;
	UINT16 *m_vidreg2;
	UINT16 *m_spriteram;
	DECLARE_WRITE16_MEMBER(bsb_videoram_w);
	DECLARE_WRITE16_MEMBER(bsb_videoram2_w);
	DECLARE_WRITE16_MEMBER(bsb_videoram3_w);
};


/*----------- defined in video/bigstrkb.c -----------*/

VIDEO_START(bigstrkb);
SCREEN_UPDATE_IND16(bigstrkb);
