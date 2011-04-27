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
	triplhnt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_cmos[16];
	UINT8 m_da_latch;
	UINT8 m_misc_flags;
	UINT8 m_cmos_latch;
	UINT8 m_hit_code;
	UINT8* m_playfield_ram;
	UINT8* m_vpos_ram;
	UINT8* m_hpos_ram;
	UINT8* m_code_ram;
	UINT8* m_orga_ram;
	int m_sprite_zoom;
	int m_sprite_bank;
	bitmap_t* m_helper;
	tilemap_t* m_bg_tilemap;
};


/*----------- defined in drivers/triplhnt.c -----------*/

void triplhnt_set_collision(running_machine &machine, int data);


/*----------- defined in audio/triplhnt.c -----------*/

DISCRETE_SOUND_EXTERN( triplhnt );
extern const samples_interface triplhnt_samples_interface;


/*----------- defined in video/triplhnt.c -----------*/

VIDEO_START( triplhnt );
SCREEN_UPDATE( triplhnt );


