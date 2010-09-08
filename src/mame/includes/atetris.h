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
};

/*----------- defined in video/atetris.c -----------*/

VIDEO_START( atetris );
VIDEO_UPDATE( atetris );

WRITE8_HANDLER( atetris_videoram_w );



