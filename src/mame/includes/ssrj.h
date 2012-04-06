class ssrj_state : public driver_device
{
public:
	ssrj_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_oldport;
	tilemap_t *m_tilemap1;
	tilemap_t *m_tilemap2;
	tilemap_t *m_tilemap4;
	UINT8 *m_vram1;
	UINT8 *m_vram2;
	UINT8 *m_vram3;
	UINT8 *m_vram4;
	UINT8 *m_scrollram;
	UINT8 *m_buffer_spriteram;
	DECLARE_READ8_MEMBER(ssrj_wheel_r);
	DECLARE_WRITE8_MEMBER(ssrj_vram1_w);
	DECLARE_WRITE8_MEMBER(ssrj_vram2_w);
	DECLARE_WRITE8_MEMBER(ssrj_vram4_w);
};


/*----------- defined in video/ssrj.c -----------*/


VIDEO_START( ssrj );
SCREEN_UPDATE_IND16( ssrj );
SCREEN_VBLANK( ssrj );
PALETTE_INIT( ssrj );
