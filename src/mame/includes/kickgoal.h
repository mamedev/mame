/*************************************************************************

    Kick Goal - Action Hollywood

*************************************************************************/

#include "sound/okim6295.h"
#include "machine/eeprom.h"

class kickgoal_state : public driver_device
{
public:
	kickgoal_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config),
		  adpcm(*this, "oki"),
		  eeprom(*this, "eeprom") { }

	/* memory pointers */
	UINT16 *    fgram;
	UINT16 *    bgram;
	UINT16 *    bg2ram;
	UINT16 *    scrram;
	UINT16 *    spriteram;
//      UINT16 *    paletteram;    // currently this uses generic palette handling
	size_t      spriteram_size;

	/* video-related */
	tilemap_t     *fgtm, *bgtm, *bg2tm;

	/* misc */
	int         melody_loop;
	int         snd_new, snd_sam[4];
	int         m6295_comm, m6295_bank;
	UINT16      m6295_key_delay;

	/* devices */
	required_device<okim6295_device> adpcm;
	required_device<eeprom_device> eeprom;
};


/*----------- defined in video/kickgoal.c -----------*/

WRITE16_HANDLER( kickgoal_fgram_w  );
WRITE16_HANDLER( kickgoal_bgram_w  );
WRITE16_HANDLER( kickgoal_bg2ram_w );

VIDEO_START( kickgoal );
VIDEO_UPDATE( kickgoal );

VIDEO_START( actionhw );
VIDEO_UPDATE( actionhw );
