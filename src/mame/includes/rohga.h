/*************************************************************************

    Data East 'Rohga' era hardware

*************************************************************************/

#include "sound/okim6295.h"
#include "video/deco16ic.h"
#include "video/decocomn.h"
#include "video/bufsprite.h"

class rohga_state : public driver_device
{
public:
	rohga_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu"),
		  m_audiocpu(*this, "audiocpu"),
		  m_decocomn(*this, "deco_common"),
		  m_deco_tilegen1(*this, "tilegen1"),
		  m_deco_tilegen2(*this, "tilegen2"),
		  m_oki1(*this, "oki1"),
		  m_oki2(*this, "oki2"),
		  m_spriteram(*this, "spriteram"),
		  m_spriteram2(*this, "spriteram2") { }

	/* memory pointers */
	UINT16 *  m_pf1_rowscroll;
	UINT16 *  m_pf2_rowscroll;
	UINT16 *  m_pf3_rowscroll;
	UINT16 *  m_pf4_rowscroll;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<decocomn_device> m_decocomn;
	required_device<deco16ic_device> m_deco_tilegen1;
	required_device<deco16ic_device> m_deco_tilegen2;
	required_device<okim6295_device> m_oki1;
	required_device<okim6295_device> m_oki2;
	required_device<buffered_spriteram16_device> m_spriteram;
	optional_device<buffered_spriteram16_device> m_spriteram2;
	DECLARE_READ16_MEMBER(rohga_irq_ack_r);
	DECLARE_WRITE16_MEMBER(wizdfire_irq_ack_w);
	DECLARE_WRITE16_MEMBER(rohga_buffer_spriteram16_w);
};



/*----------- defined in video/rohga.c -----------*/


VIDEO_START( rohga );
VIDEO_START( schmeisr );
VIDEO_START( wizdfire );


SCREEN_UPDATE_IND16( rohga );
SCREEN_UPDATE_RGB32( wizdfire );
SCREEN_UPDATE_RGB32( nitrobal );

UINT16 rohga_pri_callback(UINT16 x);
UINT16 schmeisr_col_callback(UINT16 x);
UINT16 rohga_col_callback(UINT16 x);
