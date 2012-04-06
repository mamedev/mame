
class n8080_state : public driver_device
{
public:
	n8080_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 * m_videoram;
	UINT8 * m_colorram;	// for helifire

	/* video-related */
	emu_timer* m_cannon_timer;
	int m_spacefev_red_screen;
	int m_spacefev_red_cannon;
	int m_sheriff_color_mode;
	int m_sheriff_color_data;
	int m_helifire_flash;
	UINT8 m_helifire_LSFR[63];
	unsigned m_helifire_mv;
	unsigned m_helifire_sc; /* IC56 */

	/* sound-related */
	int m_n8080_hardware;
	emu_timer* m_sound_timer[3];
	int m_helifire_dac_phase;
	double m_helifire_dac_volume;
	double m_helifire_dac_timing;
	UINT16 m_prev_sound_pins;
	UINT16 m_curr_sound_pins;
	int m_mono_flop[3];
	UINT8 m_prev_snd_data;

	/* other */
	unsigned m_shift_data;
	unsigned m_shift_bits;
	int m_inte;

	/* devices */
	device_t *m_maincpu;
	DECLARE_WRITE8_MEMBER(n8080_shift_bits_w);
	DECLARE_WRITE8_MEMBER(n8080_shift_data_w);
	DECLARE_READ8_MEMBER(n8080_shift_r);
	DECLARE_WRITE8_MEMBER(n8080_video_control_w);
};



/*----------- defined in video/n8080.c -----------*/


PALETTE_INIT( n8080 );
PALETTE_INIT( helifire );

VIDEO_START( spacefev );
VIDEO_START( sheriff );
VIDEO_START( helifire );
SCREEN_UPDATE_IND16( spacefev );
SCREEN_UPDATE_IND16( sheriff );
SCREEN_UPDATE_IND16( helifire );
SCREEN_VBLANK( helifire );

void spacefev_start_red_cannon(running_machine &machine);

/*----------- defined in audio/n8080.c -----------*/

MACHINE_CONFIG_EXTERN( spacefev_sound );
MACHINE_CONFIG_EXTERN( sheriff_sound );
MACHINE_CONFIG_EXTERN( helifire_sound );

MACHINE_START( spacefev_sound );
MACHINE_START( sheriff_sound );
MACHINE_START( helifire_sound );
MACHINE_RESET( spacefev_sound );
MACHINE_RESET( sheriff_sound );
MACHINE_RESET( helifire_sound );

WRITE8_HANDLER( n8080_sound_1_w );
WRITE8_HANDLER( n8080_sound_2_w );
