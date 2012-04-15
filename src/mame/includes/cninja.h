/*************************************************************************

    Caveman Ninja (and other DECO 16bit titles)

*************************************************************************/

#include "sound/okim6295.h"
#include "video/deco16ic.h"
#include "video/decocomn.h"
#include "video/bufsprite.h"

class cninja_state : public driver_device
{
public:
	cninja_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu"),
		  m_audiocpu(*this, "audiocpu"),
		  m_decocomn(*this, "deco_common"),
		  m_deco_tilegen1(*this, "tilegen1"),
		  m_deco_tilegen2(*this, "tilegen2"),
		  m_raster_irq_timer(*this, "raster_timer"),
		  m_oki2(*this, "oki2"),
		  m_spriteram(*this, "spriteram"),
		  m_spriteram2(*this, "spriteram2") ,
		m_pf1_rowscroll(*this, "pf1_rowscroll"),
		m_pf2_rowscroll(*this, "pf2_rowscroll"),
		m_pf3_rowscroll(*this, "pf3_rowscroll"),
		m_pf4_rowscroll(*this, "pf4_rowscroll"),
		m_ram(*this, "ram"){ }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<decocomn_device> m_decocomn;
	required_device<deco16ic_device> m_deco_tilegen1;
	required_device<deco16ic_device> m_deco_tilegen2;
	optional_device<timer_device> m_raster_irq_timer;
	optional_device<okim6295_device> m_oki2;
	required_device<buffered_spriteram16_device> m_spriteram;
	optional_device<buffered_spriteram16_device> m_spriteram2;
	/* memory pointers */
	required_shared_ptr<UINT16> m_pf1_rowscroll;
	required_shared_ptr<UINT16> m_pf2_rowscroll;
	required_shared_ptr<UINT16> m_pf3_rowscroll;
	required_shared_ptr<UINT16> m_pf4_rowscroll;
	required_shared_ptr<UINT16> m_ram;

	/* misc */
	int        m_scanline;
	int        m_irq_mask;

	DECLARE_WRITE16_MEMBER(cninja_sound_w);
	DECLARE_WRITE16_MEMBER(stoneage_sound_w);
	DECLARE_READ16_MEMBER(cninja_irq_r);
	DECLARE_WRITE16_MEMBER(cninja_irq_w);
	DECLARE_READ16_MEMBER(robocop2_prot_r);
	DECLARE_WRITE16_MEMBER(cninja_pf12_control_w);
	DECLARE_WRITE16_MEMBER(cninja_pf34_control_w);
};

/*----------- defined in video/cninja.c -----------*/

VIDEO_START( stoneage );
VIDEO_START( mutantf );

SCREEN_UPDATE_IND16( cninja );
SCREEN_UPDATE_IND16( cninjabl );
SCREEN_UPDATE_IND16( edrandy );
SCREEN_UPDATE_IND16( robocop2 );
SCREEN_UPDATE_RGB32( mutantf );
