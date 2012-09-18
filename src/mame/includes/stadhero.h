class stadhero_state : public driver_device
{
public:
	stadhero_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_pf1_data(*this, "pf1_data"),
		m_spriteram(*this, "spriteram"){ }

	required_shared_ptr<UINT16> m_pf1_data;
	tilemap_t *m_pf1_tilemap;
	int m_flipscreen;
	required_shared_ptr<UINT16> m_spriteram;
	DECLARE_READ16_MEMBER(stadhero_control_r);
	DECLARE_WRITE16_MEMBER(stadhero_control_w);
	DECLARE_WRITE16_MEMBER(stadhero_pf1_data_w);
	TILE_GET_INFO_MEMBER(get_pf1_tile_info);
	virtual void video_start();
	UINT32 screen_update_stadhero(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
