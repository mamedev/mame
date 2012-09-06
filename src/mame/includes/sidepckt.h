class sidepckt_state : public driver_device
{
public:
	sidepckt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram"){ }

	tilemap_t *m_bg_tilemap;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_spriteram;
	int m_i8751_return;
	int m_current_ptr;
	int m_current_table;
	int m_in_math;
	int m_math_param;
	DECLARE_WRITE8_MEMBER(sound_cpu_command_w);
	DECLARE_READ8_MEMBER(sidepckt_i8751_r);
	DECLARE_WRITE8_MEMBER(sidepckt_i8751_w);
	DECLARE_WRITE8_MEMBER(sidepctj_i8751_w);
	DECLARE_WRITE8_MEMBER(sidepckt_videoram_w);
	DECLARE_WRITE8_MEMBER(sidepckt_colorram_w);
	DECLARE_WRITE8_MEMBER(sidepckt_flipscreen_w);
	DECLARE_DRIVER_INIT(sidepckt);
	DECLARE_DRIVER_INIT(sidepctj);
	TILE_GET_INFO_MEMBER(get_tile_info);
};


/*----------- defined in video/sidepckt.c -----------*/

PALETTE_INIT( sidepckt );
VIDEO_START( sidepckt );
SCREEN_UPDATE_IND16( sidepckt );

