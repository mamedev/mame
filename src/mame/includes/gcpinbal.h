
#include "sound/okim6295.h"
#include "sound/msm5205.h"

class gcpinbal_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, gcpinbal_state(machine)); }

	gcpinbal_state(running_machine &machine)
		: maincpu(machine.device<cpu_device>("maincpu")),
		  oki(machine.device<okim6295_device>("oki")),
		  msm(machine.device<msm5205_sound_device>("msm")) { }

	/* memory pointers */
	UINT16 *    tilemapram;
	UINT16 *    ioc_ram;
	UINT16 *    spriteram;
//  UINT16 *    paletteram; // currently this uses generic palette handling
	size_t      spriteram_size;

	/* video-related */
	tilemap_t     *tilemap[3];
	UINT16      scrollx[3], scrolly[3];
	UINT16      bg0_gfxset, bg1_gfxset;
#ifdef MAME_DEBUG
	UINT8       dislayer[4];
#endif

	/* sound-related */
	UINT32      msm_start, msm_end, msm_bank;
	UINT32      adpcm_start, adpcm_end, adpcm_idle;
	UINT8       adpcm_trigger, adpcm_data;

	/* devices */
	cpu_device *maincpu;
	okim6295_device *oki;
	msm5205_sound_device *msm;
};


/*----------- defined in video/gcpinbal.c -----------*/

VIDEO_START( gcpinbal );
VIDEO_UPDATE( gcpinbal );

READ16_HANDLER ( gcpinbal_tilemaps_word_r );
WRITE16_HANDLER( gcpinbal_tilemaps_word_w );
