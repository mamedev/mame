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
		: driver_device(mconfig, type, tag),
		m_spriteram(*this,"spriteram") { }

	/* memory pointers */
	required_shared_ptr<UINT16> m_spriteram;

	/* video-related */
	struct othunder_tempsprite *m_spritelist;

	/* misc */
	int        m_vblank_irq;
	int        m_ad_irq;
	INT32      m_banknum;
	int        m_pan[4];

	/* devices */
	cpu_device *m_maincpu;
	cpu_device *m_audiocpu;
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
	DECLARE_WRITE16_MEMBER(irq_ack_w);
	DECLARE_WRITE16_MEMBER(othunder_tc0220ioc_w);
	DECLARE_READ16_MEMBER(othunder_tc0220ioc_r);
	DECLARE_READ16_MEMBER(othunder_lightgun_r);
	DECLARE_WRITE16_MEMBER(othunder_lightgun_w);
	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);
	DECLARE_WRITE16_MEMBER(othunder_sound_w);
	DECLARE_READ16_MEMBER(othunder_sound_r);
	DECLARE_WRITE8_MEMBER(othunder_TC0310FAM_w);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
};


/*----------- defined in video/othunder.c -----------*/


SCREEN_UPDATE_IND16( othunder );
