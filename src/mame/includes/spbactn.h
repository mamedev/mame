class spbactn_state : public driver_device
{
public:
	spbactn_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 *m_bgvideoram;
	UINT16 *m_fgvideoram;
	UINT16 *m_spvideoram;

	bitmap_ind16 m_tile_bitmap_bg;
	bitmap_ind16 m_tile_bitmap_fg;
	DECLARE_WRITE16_MEMBER(soundcommand_w);
};


/*----------- defined in video/spbactn.c -----------*/

VIDEO_START( spbactn );
SCREEN_UPDATE_RGB32( spbactn );
