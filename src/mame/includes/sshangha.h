#include "video/deco16ic.h"

class sshangha_state : public driver_device
{
public:
	sshangha_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config),
		 deco_tilegen1(*this, "tilegen1")
		{ }

	UINT16 *prot_data;
	UINT16 *sound_shared_ram;
	int video_control;

	UINT16* pf1_rowscroll;
	UINT16* pf2_rowscroll;
	
	UINT16* sprite_paletteram;
	UINT16* sprite_paletteram2;
	UINT16* tile_paletteram1;
	UINT16* tile_paletteram2;
	
	required_device<deco16ic_device> deco_tilegen1;
};


/*----------- defined in video/sshangha.c -----------*/

VIDEO_START( sshangha );
SCREEN_UPDATE( sshangha );

WRITE16_HANDLER( sshangha_video_w );
