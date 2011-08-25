/*************************************************************************

    Atari Tetris hardware

*************************************************************************/

class atetris_state : public driver_device
{
public:
	atetris_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_nvram(*this, "nvram") { }

	required_shared_ptr<UINT8>	m_nvram;
	UINT8 *m_videoram;
	UINT8 *m_slapstic_source;
	UINT8 *m_slapstic_base;
	UINT8 m_current_bank;
	UINT8 m_nvram_write_enable;
	emu_timer *m_interrupt_timer;
	tilemap_t *m_bg_tilemap;
};

/*----------- defined in video/atetris.c -----------*/

VIDEO_START( atetris );
SCREEN_UPDATE( atetris );

WRITE8_HANDLER( atetris_videoram_w );
