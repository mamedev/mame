/*************************************************************************

    Simple 156 based board

*************************************************************************/

#include "machine/eeprom.h"
#include "sound/okim6295.h"
#include "video/deco16ic.h"

class simpl156_state : public driver_device
{
public:
	simpl156_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config),
		  maincpu(*this, "maincpu"),
		  deco16ic(*this, "deco_custom"),
		  eeprom(*this, "eeprom"),
		  okimusic(*this, "okimusic") { }

	/* memory pointers */
	UINT16 *  pf1_rowscroll;
	UINT16 *  pf2_rowscroll;
	UINT32 *  mainram;
	UINT32 *  systemram;

	/* devices */
	required_device<cpu_device> maincpu;
	required_device<deco16ic_device> deco16ic;
	required_device<eeprom_device> eeprom;
	required_device<okim6295_device> okimusic;
};



/*----------- defined in video/simpl156.c -----------*/

VIDEO_START( simpl156 );
VIDEO_UPDATE( simpl156 );
