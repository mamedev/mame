
#include "sound/okim6295.h"
#include "sound/msm5205.h"

class gcpinbal_state : public driver_device
{
public:
	gcpinbal_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu"),
		  m_oki(*this, "oki"),
		  m_msm(*this, "msm") { }

	/* memory pointers */
	UINT16 *    m_tilemapram;
	UINT16 *    m_ioc_ram;
	UINT16 *    m_spriteram;
//  UINT16 *    m_paletteram; // currently this uses generic palette handling
	size_t      m_spriteram_size;

	/* video-related */
	tilemap_t     *m_tilemap[3];
	UINT16      m_scrollx[3];
	UINT16      m_scrolly[3];
	UINT16      m_bg0_gfxset;
	UINT16      m_bg1_gfxset;
#ifdef MAME_DEBUG
	UINT8       m_dislayer[4];
#endif

	/* sound-related */
	UINT32      m_msm_start;
	UINT32      m_msm_end;
	UINT32      m_msm_bank;
	UINT32      m_adpcm_start;
	UINT32      m_adpcm_end;
	UINT32      m_adpcm_idle;
	UINT8       m_adpcm_trigger;
	UINT8       m_adpcm_data;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<okim6295_device> m_oki;
	required_device<msm5205_device> m_msm;
	DECLARE_READ16_MEMBER(ioc_r);
	DECLARE_WRITE16_MEMBER(ioc_w);
	DECLARE_READ16_MEMBER(gcpinbal_tilemaps_word_r);
	DECLARE_WRITE16_MEMBER(gcpinbal_tilemaps_word_w);
	DECLARE_READ16_MEMBER(gcpinbal_ctrl_word_r);
	DECLARE_WRITE16_MEMBER(gcpinbal_ctrl_word_w);
};


/*----------- defined in video/gcpinbal.c -----------*/

VIDEO_START( gcpinbal );
SCREEN_UPDATE_IND16( gcpinbal );

