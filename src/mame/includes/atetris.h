/*************************************************************************

    Atari Tetris hardware

*************************************************************************/

class atetris_state : public driver_device
{
public:
	atetris_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config),
		  m_nvram(*this, "nvram") { }

	required_shared_ptr<UINT8>	m_nvram;
	UINT8 *videoram;
	UINT8 *slapstic_source;
	UINT8 *slapstic_base;
	UINT8 current_bank;
	UINT8 nvram_write_enable;
	emu_timer *interrupt_timer;
	tilemap_t *bg_tilemap;
};

/*----------- defined in video/atetris.c -----------*/

VIDEO_START( atetris );
SCREEN_UPDATE( atetris );

WRITE8_HANDLER( atetris_videoram_w );
