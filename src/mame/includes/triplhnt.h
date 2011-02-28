/*************************************************************************

    Atari Triple Hunt hardware

*************************************************************************/

#include "sound/discrete.h"
#include "sound/samples.h"


/* Discrete Sound Input Nodes */
#define TRIPLHNT_BEAR_ROAR_DATA	NODE_01
#define TRIPLHNT_BEAR_EN		NODE_02
#define TRIPLHNT_SHOT_DATA		NODE_03
#define TRIPLHNT_SCREECH_EN		NODE_04
#define TRIPLHNT_LAMP_EN		NODE_05


class triplhnt_state : public driver_device
{
public:
	triplhnt_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 cmos[16];
	UINT8 da_latch;
	UINT8 misc_flags;
	UINT8 cmos_latch;
	UINT8 hit_code;
	UINT8* playfield_ram;
	UINT8* vpos_ram;
	UINT8* hpos_ram;
	UINT8* code_ram;
	UINT8* orga_ram;
	int sprite_zoom;
	int sprite_bank;
	bitmap_t* helper;
	tilemap_t* bg_tilemap;
};


/*----------- defined in drivers/triplhnt.c -----------*/

void triplhnt_set_collision(running_machine *machine, int data);


/*----------- defined in audio/triplhnt.c -----------*/

DISCRETE_SOUND_EXTERN( triplhnt );
extern const samples_interface triplhnt_samples_interface;


/*----------- defined in video/triplhnt.c -----------*/

VIDEO_START( triplhnt );
SCREEN_UPDATE( triplhnt );


