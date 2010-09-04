class gladiatr_state : public driver_device
{
public:
	gladiatr_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config),
		  m_nvram(*this, "nvram") { }

	required_shared_ptr<UINT8>	m_nvram;
};

/*----------- defined in video/gladiatr.c -----------*/

extern UINT8 *gladiatr_videoram, *gladiatr_colorram,*gladiatr_textram;

WRITE8_HANDLER( gladiatr_videoram_w );
WRITE8_HANDLER( gladiatr_colorram_w );
WRITE8_HANDLER( gladiatr_textram_w );
WRITE8_HANDLER( gladiatr_paletteram_w );
WRITE8_HANDLER( ppking_video_registers_w );
WRITE8_HANDLER( gladiatr_video_registers_w );
WRITE8_HANDLER( gladiatr_spritebuffer_w );
WRITE8_HANDLER( gladiatr_spritebank_w );
VIDEO_START( ppking );
VIDEO_UPDATE( ppking );
VIDEO_START( gladiatr );
VIDEO_UPDATE( gladiatr );
