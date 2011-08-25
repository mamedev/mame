/*************************************************************************

    Operation Thunderbolt

*************************************************************************/

#include "machine/eeprom.h"

struct othunder_tempsprite
{
	int gfx;
	int code,color;
	int flipx,flipy;
	int x,y;
	int zoomx,zoomy;
	int primask;
};


class othunder_state : public driver_device
{
public:
	othunder_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *   m_spriteram;
	size_t     m_spriteram_size;

	/* video-related */
	struct othunder_tempsprite *m_spritelist;

	/* misc */
	int        m_vblank_irq;
	int        m_ad_irq;
	INT32      m_banknum;
	int        m_pan[4];

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	eeprom_device *m_eeprom;
	device_t *m_tc0220ioc;
	device_t *m_tc0100scn;
	device_t *m_tc0110pcr;
	device_t *m_tc0140syt;
	device_t *m_2610_0l;
	device_t *m_2610_0r;
	device_t *m_2610_1l;
	device_t *m_2610_1r;
	device_t *m_2610_2l;
	device_t *m_2610_2r;
};


/*----------- defined in video/othunder.c -----------*/

VIDEO_START( othunder );
SCREEN_UPDATE( othunder );
