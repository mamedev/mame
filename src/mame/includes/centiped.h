/*************************************************************************

    Atari Centipede hardware

*************************************************************************/

class centiped_state : public driver_device
{
public:
	centiped_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	UINT8 m_oldpos[4];
	UINT8 m_sign[4];
	UINT8 m_dsw_select;
	UINT8 m_control_select;
	UINT8 *m_rambase;
	UINT8 m_flipscreen;
	UINT8 *m_bullsdrt_tiles_bankram;
	tilemap_t *m_bg_tilemap;
	UINT8 m_bullsdrt_sprites_bank;
	UINT8 m_penmask[64];
	UINT8 *m_spriteram;
	DECLARE_WRITE8_MEMBER(irq_ack_w);
	DECLARE_READ8_MEMBER(centiped_IN0_r);
	DECLARE_READ8_MEMBER(centiped_IN2_r);
	DECLARE_READ8_MEMBER(milliped_IN1_r);
	DECLARE_READ8_MEMBER(milliped_IN2_r);
	DECLARE_WRITE8_MEMBER(input_select_w);
	DECLARE_WRITE8_MEMBER(control_select_w);
	DECLARE_READ8_MEMBER(mazeinv_input_r);
	DECLARE_WRITE8_MEMBER(mazeinv_input_select_w);
	DECLARE_READ8_MEMBER(bullsdrt_data_port_r);
	DECLARE_WRITE8_MEMBER(led_w);
	DECLARE_WRITE8_MEMBER(coin_count_w);
	DECLARE_WRITE8_MEMBER(bullsdrt_coin_count_w);
	DECLARE_WRITE8_MEMBER(centiped_videoram_w);
	DECLARE_WRITE8_MEMBER(centiped_flip_screen_w);
	DECLARE_WRITE8_MEMBER(bullsdrt_tilesbank_w);
	DECLARE_WRITE8_MEMBER(bullsdrt_sprites_bank_w);
	DECLARE_WRITE8_MEMBER(centiped_paletteram_w);
	DECLARE_WRITE8_MEMBER(milliped_paletteram_w);
	DECLARE_WRITE8_MEMBER(mazeinv_paletteram_w);
};


/*----------- defined in video/centiped.c -----------*/

PALETTE_INIT( warlords );

VIDEO_START( centiped );
VIDEO_START( milliped );
VIDEO_START( warlords );
VIDEO_START( bullsdrt );

SCREEN_UPDATE_IND16( centiped );
SCREEN_UPDATE_IND16( milliped );
SCREEN_UPDATE_IND16( warlords );
SCREEN_UPDATE_IND16( bullsdrt );



