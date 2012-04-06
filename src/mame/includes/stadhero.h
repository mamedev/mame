class stadhero_state : public driver_device
{
public:
	stadhero_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 *m_pf1_data;
	tilemap_t *m_pf1_tilemap;
	int m_flipscreen;
	UINT16 *m_spriteram;
	DECLARE_READ16_MEMBER(stadhero_control_r);
	DECLARE_WRITE16_MEMBER(stadhero_control_w);
	DECLARE_WRITE16_MEMBER(stadhero_pf1_data_w);
};


/*----------- defined in video/stadhero.c -----------*/

VIDEO_START( stadhero );
SCREEN_UPDATE_IND16( stadhero );

