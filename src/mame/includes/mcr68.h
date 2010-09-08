#include "machine/6821pia.h"

class mcr68_state : public driver_device
{
public:
	mcr68_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *videoram;
};


/*----------- defined in drivers/mcr68.c -----------*/

READ8_DEVICE_HANDLER( zwackery_port_2_r );


/*----------- defined in machine/mcr68.c -----------*/

extern attotime mcr68_timing_factor;

extern const pia6821_interface zwackery_pia0_intf;
extern const pia6821_interface zwackery_pia1_intf;
extern const pia6821_interface zwackery_pia2_intf;


MACHINE_START( mcr68 );
MACHINE_RESET( mcr68 );
MACHINE_START( zwackery );
MACHINE_RESET( zwackery );

WRITE16_HANDLER( mcr68_6840_upper_w );
WRITE16_HANDLER( mcr68_6840_lower_w );
READ16_HANDLER( mcr68_6840_upper_r );
READ16_HANDLER( mcr68_6840_lower_r );

INTERRUPT_GEN( mcr68_interrupt );


/*----------- defined in video/mcr68.c -----------*/

extern UINT8 mcr68_sprite_clip;
extern INT8 mcr68_sprite_xoffset;

WRITE16_HANDLER( mcr68_paletteram_w );
WRITE16_HANDLER( mcr68_videoram_w );

VIDEO_START( mcr68 );
VIDEO_UPDATE( mcr68 );

WRITE16_HANDLER( zwackery_paletteram_w );
WRITE16_HANDLER( zwackery_videoram_w );
WRITE16_HANDLER( zwackery_spriteram_w );

VIDEO_START( zwackery );
VIDEO_UPDATE( zwackery );
