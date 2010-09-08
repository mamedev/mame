/***************************************************************************

    Gottlieb hardware

***************************************************************************/

#include "machine/6532riot.h"


#define GOTTLIEB_VIDEO_HCOUNT	318
#define GOTTLIEB_VIDEO_HBLANK	256
#define GOTTLIEB_VIDEO_VCOUNT	256
#define GOTTLIEB_VIDEO_VBLANK	240


class gottlieb_state : public driver_device
{
public:
	gottlieb_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
};


/*----------- defined in audio/gottlieb.c -----------*/

WRITE8_HANDLER( gottlieb_sh_w );

MACHINE_CONFIG_EXTERN( gottlieb_soundrev1 );
MACHINE_CONFIG_EXTERN( gottlieb_soundrev2 );

INPUT_PORTS_EXTERN( gottlieb1_sound );
INPUT_PORTS_EXTERN( gottlieb2_sound );


/*----------- defined in video/gottlieb.c -----------*/

extern UINT8 gottlieb_gfxcharlo;
extern UINT8 gottlieb_gfxcharhi;
extern UINT8 *gottlieb_charram;

extern WRITE8_HANDLER( gottlieb_videoram_w );
extern WRITE8_HANDLER( gottlieb_charram_w );
extern WRITE8_HANDLER( gottlieb_video_control_w );
extern WRITE8_HANDLER( gottlieb_laserdisc_video_control_w );
extern WRITE8_HANDLER( gottlieb_paletteram_w );

VIDEO_START( gottlieb );
VIDEO_START( screwloo );
VIDEO_UPDATE( gottlieb );
