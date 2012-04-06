class sidepckt_state : public driver_device
{
public:
	sidepckt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	tilemap_t *m_bg_tilemap;
	UINT8 *m_colorram;
	UINT8 *m_videoram;
	size_t m_videoram_size;
	UINT8 *m_spriteram;
	size_t m_spriteram_size;
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
};


/*----------- defined in video/sidepckt.c -----------*/

PALETTE_INIT( sidepckt );
VIDEO_START( sidepckt );
SCREEN_UPDATE_IND16( sidepckt );

