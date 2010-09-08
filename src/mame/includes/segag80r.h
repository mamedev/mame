/*************************************************************************

    Sega G-80 raster hardware

*************************************************************************/

class segag80r_state : public driver_device
{
public:
	segag80r_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
};


/*----------- defined in machine/segag80.c -----------*/

extern UINT8 (*sega_decrypt)(offs_t, UINT8);

void sega_security(int chip);


/*----------- defined in audio/segag80r.c -----------*/

MACHINE_CONFIG_EXTERN( astrob_sound_board );
MACHINE_CONFIG_EXTERN( 005_sound_board );
MACHINE_CONFIG_EXTERN( spaceod_sound_board );
MACHINE_CONFIG_EXTERN( monsterb_sound_board );

WRITE8_HANDLER( astrob_sound_w );

WRITE8_HANDLER( spaceod_sound_w );


/*----------- defined in video/segag80r.c -----------*/

#define G80_BACKGROUND_NONE			0
#define G80_BACKGROUND_SPACEOD		1
#define G80_BACKGROUND_MONSTERB		2
#define G80_BACKGROUND_PIGNEWT		3
#define G80_BACKGROUND_SINDBADM		4

extern UINT8 segag80r_background_pcb;

INTERRUPT_GEN( segag80r_vblank_start );

WRITE8_HANDLER( segag80r_videoram_w );

READ8_HANDLER( segag80r_video_port_r );
WRITE8_HANDLER( segag80r_video_port_w );

VIDEO_START( segag80r );
VIDEO_UPDATE( segag80r );


READ8_HANDLER( spaceod_back_port_r );
WRITE8_HANDLER( spaceod_back_port_w );


WRITE8_HANDLER( monsterb_videoram_w );
WRITE8_HANDLER( monsterb_back_port_w );


WRITE8_HANDLER( pignewt_videoram_w );
WRITE8_HANDLER( pignewt_back_port_w );
WRITE8_HANDLER( pignewt_back_color_w );


INTERRUPT_GEN( sindbadm_vblank_start );

WRITE8_HANDLER( sindbadm_videoram_w );
WRITE8_HANDLER( sindbadm_back_port_w );
