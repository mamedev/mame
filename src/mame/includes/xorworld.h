class xorworld_state : public driver_device
{
public:
	xorworld_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 *m_videoram;
	tilemap_t *m_bg_tilemap;
	UINT16 *m_spriteram;
	DECLARE_WRITE16_MEMBER(xorworld_irq2_ack_w);
	DECLARE_WRITE16_MEMBER(xorworld_irq6_ack_w);
	DECLARE_WRITE16_MEMBER(xorworld_videoram16_w);
};


/*----------- defined in video/xorworld.c -----------*/


PALETTE_INIT( xorworld );
VIDEO_START( xorworld );
SCREEN_UPDATE_IND16( xorworld );
