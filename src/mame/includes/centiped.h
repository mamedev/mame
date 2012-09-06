/*************************************************************************

    Atari Centipede hardware

*************************************************************************/

class centiped_state : public driver_device
{
public:
	centiped_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_rambase(*this, "rambase"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_bullsdrt_tiles_bankram(*this, "bullsdrt_bank")
	{ }

	optional_shared_ptr<UINT8> m_rambase;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_spriteram;
	optional_shared_ptr<UINT8> m_bullsdrt_tiles_bankram;

	UINT8 m_oldpos[4];
	UINT8 m_sign[4];
	UINT8 m_dsw_select;
	UINT8 m_control_select;
	UINT8 m_flipscreen;
	UINT8 m_prg_bank;
	UINT8 m_gfx_bank;
	UINT8 m_bullsdrt_sprites_bank;
	UINT8 m_penmask[64];
	tilemap_t *m_bg_tilemap;

	// drivers/centiped.c
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
	DECLARE_READ8_MEMBER(caterplr_rand_r);
	DECLARE_WRITE8_MEMBER(caterplr_AY8910_w);
	DECLARE_READ8_MEMBER(caterplr_AY8910_r);
	DECLARE_READ8_MEMBER(multiped_eeprom_r);
	DECLARE_WRITE8_MEMBER(multiped_eeprom_w);
	DECLARE_WRITE8_MEMBER(multiped_prgbank_w);

	// video/centiped.c
	DECLARE_WRITE8_MEMBER(centiped_videoram_w);
	DECLARE_WRITE8_MEMBER(centiped_flip_screen_w);
	DECLARE_WRITE8_MEMBER(multiped_gfxbank_w);
	DECLARE_WRITE8_MEMBER(bullsdrt_tilesbank_w);
	DECLARE_WRITE8_MEMBER(bullsdrt_sprites_bank_w);
	DECLARE_WRITE8_MEMBER(centiped_paletteram_w);
	DECLARE_WRITE8_MEMBER(milliped_paletteram_w);
	DECLARE_WRITE8_MEMBER(mazeinv_paletteram_w);
	DECLARE_DRIVER_INIT(multiped);
	DECLARE_DRIVER_INIT(bullsdrt);
	TILE_GET_INFO_MEMBER(centiped_get_tile_info);
	TILE_GET_INFO_MEMBER(warlords_get_tile_info);
	TILE_GET_INFO_MEMBER(milliped_get_tile_info);
	TILE_GET_INFO_MEMBER(bullsdrt_get_tile_info);
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

