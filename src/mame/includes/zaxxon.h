/***************************************************************************

    Sega Zaxxon hardware

***************************************************************************/

class zaxxon_state : public driver_device
{
public:
	zaxxon_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_colorram(*this, "colorram"){ }

	required_shared_ptr<UINT8> m_videoram;
	optional_shared_ptr<UINT8> m_spriteram;
	optional_shared_ptr<UINT8> m_colorram;

	UINT8 m_int_enabled;
	UINT8 m_coin_status[3];
	UINT8 m_coin_enable[3];

	UINT8 m_razmataz_dial_pos[2];
	UINT16 m_razmataz_counter;

	UINT8 m_sound_state[3];
	UINT8 m_bg_enable;
	UINT8 m_bg_color;
	UINT16 m_bg_position;
	UINT8 m_fg_color;

	UINT8 m_congo_fg_bank;
	UINT8 m_congo_color_bank;
	UINT8 m_congo_custom[4];

	const UINT8 *m_color_codes;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	DECLARE_WRITE8_MEMBER(int_enable_w);
	DECLARE_READ8_MEMBER(razmataz_counter_r);
	DECLARE_WRITE8_MEMBER(zaxxon_coin_counter_w);
	DECLARE_WRITE8_MEMBER(zaxxon_coin_enable_w);
	DECLARE_WRITE8_MEMBER(zaxxon_flipscreen_w);
	DECLARE_WRITE8_MEMBER(zaxxon_fg_color_w);
	DECLARE_WRITE8_MEMBER(zaxxon_bg_position_w);
	DECLARE_WRITE8_MEMBER(zaxxon_bg_color_w);
	DECLARE_WRITE8_MEMBER(zaxxon_bg_enable_w);
	DECLARE_WRITE8_MEMBER(congo_fg_bank_w);
	DECLARE_WRITE8_MEMBER(congo_color_bank_w);
	DECLARE_WRITE8_MEMBER(zaxxon_videoram_w);
	DECLARE_WRITE8_MEMBER(congo_colorram_w);
	DECLARE_WRITE8_MEMBER(congo_sprite_custom_w);
	DECLARE_CUSTOM_INPUT_MEMBER(razmataz_dial_r);
	DECLARE_CUSTOM_INPUT_MEMBER(zaxxon_coin_r);
	DECLARE_INPUT_CHANGED_MEMBER(service_switch);
	DECLARE_INPUT_CHANGED_MEMBER(zaxxon_coin_inserted);
	DECLARE_DRIVER_INIT(razmataz);
	DECLARE_DRIVER_INIT(futspy);
	DECLARE_DRIVER_INIT(zaxxonj);
	DECLARE_DRIVER_INIT(szaxxon);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(zaxxon_get_fg_tile_info);
	TILE_GET_INFO_MEMBER(razmataz_get_fg_tile_info);
	TILE_GET_INFO_MEMBER(congo_get_fg_tile_info);
	virtual void machine_start();
	virtual void video_start();
	virtual void palette_init();
	DECLARE_VIDEO_START(razmataz);
	DECLARE_VIDEO_START(congo);
};


/*----------- defined in audio/zaxxon.c -----------*/

DECLARE_WRITE8_DEVICE_HANDLER( zaxxon_sound_a_w );
DECLARE_WRITE8_DEVICE_HANDLER( zaxxon_sound_b_w );
DECLARE_WRITE8_DEVICE_HANDLER( zaxxon_sound_c_w );

DECLARE_WRITE8_DEVICE_HANDLER( congo_sound_b_w );
DECLARE_WRITE8_DEVICE_HANDLER( congo_sound_c_w );

MACHINE_CONFIG_EXTERN( zaxxon_samples );
MACHINE_CONFIG_EXTERN( congo_samples );


/*----------- defined in video/zaxxon.c -----------*/










SCREEN_UPDATE_IND16( zaxxon );
SCREEN_UPDATE_IND16( razmataz );
SCREEN_UPDATE_IND16( congo );
SCREEN_UPDATE_IND16( futspy );
