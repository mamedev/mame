/*************************************************************************

    Kick Goal - Action Hollywood

*************************************************************************/

#include "sound/okim6295.h"
#include "machine/eeprom.h"

class kickgoal_state : public driver_device
{
public:
	kickgoal_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_fgram(*this, "fgram"),
		m_bgram(*this, "bgram"),
		m_bg2ram(*this, "bg2ram"),
		m_scrram(*this, "scrram"),
		m_spriteram(*this, "spriteram"),
		m_adpcm(*this, "oki"),
		m_eeprom(*this, "eeprom") { }

	/* memory pointers */
	required_shared_ptr<UINT16> m_fgram;
	required_shared_ptr<UINT16> m_bgram;
	required_shared_ptr<UINT16> m_bg2ram;
	required_shared_ptr<UINT16> m_scrram;
	required_shared_ptr<UINT16> m_spriteram;
//      UINT16 *    m_paletteram;    // currently this uses generic palette handling

	/* video-related */
	tilemap_t     *m_fgtm;
	tilemap_t     *m_bgtm;
	tilemap_t     *m_bg2tm;

	/* misc */
	int         m_melody_loop;
	int         m_snd_new;
	int         m_snd_sam[4];
	int         m_m6295_comm;
	int         m_m6295_bank;
	UINT16      m_m6295_key_delay;

	int	m_fg_base;

	int m_bg_base;
	int m_bg_mask;
	
	int m_bg2_base;
	int m_bg2_mask;
	int m_bg2_region;

	int m_sprbase;

	/* devices */
	required_device<okim6295_device> m_adpcm;
	required_device<eeprom_device> m_eeprom;
	DECLARE_READ16_MEMBER(kickgoal_eeprom_r);
	DECLARE_WRITE16_MEMBER(kickgoal_eeprom_w);
	DECLARE_WRITE16_MEMBER(kickgoal_fgram_w);
	DECLARE_WRITE16_MEMBER(kickgoal_bgram_w);
	DECLARE_WRITE16_MEMBER(kickgoal_bg2ram_w);
	DECLARE_WRITE16_MEMBER(kickgoal_snd_w);
	DECLARE_WRITE16_MEMBER(actionhw_snd_w);
};


/*----------- defined in video/kickgoal.c -----------*/

VIDEO_START( actionhw );
VIDEO_START( kickgoal );
SCREEN_UPDATE_IND16( kickgoal );

