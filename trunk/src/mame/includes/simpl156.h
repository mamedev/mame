/*************************************************************************

    Simple 156 based board

*************************************************************************/

#include "machine/eeprom.h"
#include "sound/okim6295.h"
#include "video/deco16ic.h"

class simpl156_state : public driver_device
{
public:
	simpl156_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu"),
		  m_deco_tilegen1(*this, "tilegen1"),
		  m_eeprom(*this, "eeprom"),
		  m_okimusic(*this, "okimusic") { }

	/* memory pointers */
	UINT16 *  m_pf1_rowscroll;
	UINT16 *  m_pf2_rowscroll;
	UINT32 *  m_mainram;
	UINT32 *  m_systemram;
	UINT16 *m_spriteram;
	size_t m_spriteram_size;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<deco16ic_device> m_deco_tilegen1;
	required_device<eeprom_device> m_eeprom;
	required_device<okim6295_device> m_okimusic;
};



/*----------- defined in video/simpl156.c -----------*/

VIDEO_START( simpl156 );
SCREEN_UPDATE( simpl156 );
