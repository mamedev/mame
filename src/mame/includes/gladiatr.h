class gladiatr_state : public driver_device
{
public:
	gladiatr_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config),
		  m_nvram(*this, "nvram") { }

	required_shared_ptr<UINT8>	m_nvram;
	int data1;
	int data2;
	int flag1;
	int flag2;
	UINT8 *videoram;
	UINT8 *colorram;
	UINT8 *textram;
	int video_attributes;
	int fg_scrollx;
	int fg_scrolly;
	int bg_scrollx;
	int bg_scrolly;
	int sprite_bank;
	int sprite_buffer;
	tilemap_t *fg_tilemap;
	tilemap_t *bg_tilemap;
	int fg_tile_bank;
	int bg_tile_bank;
};

/*----------- defined in video/gladiatr.c -----------*/

WRITE8_HANDLER( gladiatr_videoram_w );
WRITE8_HANDLER( gladiatr_colorram_w );
WRITE8_HANDLER( gladiatr_textram_w );
WRITE8_HANDLER( gladiatr_paletteram_w );
WRITE8_HANDLER( ppking_video_registers_w );
WRITE8_HANDLER( gladiatr_video_registers_w );
WRITE8_HANDLER( gladiatr_spritebuffer_w );
WRITE8_HANDLER( gladiatr_spritebank_w );
VIDEO_START( ppking );
SCREEN_UPDATE( ppking );
VIDEO_START( gladiatr );
SCREEN_UPDATE( gladiatr );
