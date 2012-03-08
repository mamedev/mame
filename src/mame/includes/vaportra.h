/*************************************************************************

    Vapour Trail

*************************************************************************/

#include "video/bufsprite.h"

class vaportra_state : public driver_device
{
public:
	vaportra_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_spriteram(*this, "spriteram") { }

	/* memory pointers */
	UINT16 *  m_pf1_rowscroll;
	UINT16 *  m_pf2_rowscroll;
	UINT16 *  m_pf3_rowscroll;
	UINT16 *  m_pf4_rowscroll;

	/* misc */
	UINT16    m_priority[2];

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_deco_tilegen1;
	device_t *m_deco_tilegen2;
	required_device<buffered_spriteram16_device> m_spriteram;
};



/*----------- defined in video/vaportra.c -----------*/

WRITE16_HANDLER( vaportra_priority_w );
WRITE16_HANDLER( vaportra_palette_24bit_rg_w );
WRITE16_HANDLER( vaportra_palette_24bit_b_w );

SCREEN_UPDATE_IND16( vaportra );
