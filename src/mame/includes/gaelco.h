/***************************************************************************

    Gaelco game hardware from 1991-1996

***************************************************************************/

class gaelco_state : public driver_device
{
public:
	gaelco_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *     m_videoram;
	UINT16 *     m_spriteram;
	UINT16 *     m_vregs;
	UINT16 *     m_screen;
//  UINT16 *     paletteram;    // currently this uses generic palette handling

	/* video-related */
	tilemap_t      *m_tilemap[2];

	/* devices */
	device_t *m_audiocpu;
	DECLARE_WRITE16_MEMBER(bigkarnk_sound_command_w);
	DECLARE_WRITE16_MEMBER(bigkarnk_coin_w);
	DECLARE_WRITE16_MEMBER(OKIM6295_bankswitch_w);
	DECLARE_WRITE16_MEMBER(gaelco_vram_encrypted_w);
	DECLARE_WRITE16_MEMBER(gaelco_encrypted_w);
	DECLARE_WRITE16_MEMBER(thoop_vram_encrypted_w);
	DECLARE_WRITE16_MEMBER(thoop_encrypted_w);
	DECLARE_WRITE16_MEMBER(gaelco_vram_w);
};



/*----------- defined in video/gaelco.c -----------*/


VIDEO_START( bigkarnk );
VIDEO_START( maniacsq );

SCREEN_UPDATE_IND16( bigkarnk );
SCREEN_UPDATE_IND16( maniacsq );
