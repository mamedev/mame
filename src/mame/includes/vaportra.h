/*************************************************************************

    Vapor Trail

*************************************************************************/

#include "video/bufsprite.h"
#include "video/deco16ic.h"
#include "video/decmxc06.h"

class vaportra_state : public driver_device
{
public:
	vaportra_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_deco_tilegen1(*this, "tilegen1"),
		m_deco_tilegen2(*this, "tilegen2"),
		m_spritegen(*this, "spritegen"),
		m_spriteram(*this, "spriteram"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	UINT16 *  m_pf1_rowscroll;
	UINT16 *  m_pf2_rowscroll;
	UINT16 *  m_pf3_rowscroll;
	UINT16 *  m_pf4_rowscroll;

	/* misc */
	UINT16    m_priority[2];

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<deco16ic_device> m_deco_tilegen1;
	required_device<deco16ic_device> m_deco_tilegen2;
	required_device<deco_mxc06_device> m_spritegen;
	required_device<buffered_spriteram16_device> m_spriteram;
	required_device<palette_device> m_palette;

	DECLARE_WRITE16_MEMBER(vaportra_sound_w);
	DECLARE_READ16_MEMBER(vaportra_control_r);
	DECLARE_READ8_MEMBER(vaportra_soundlatch_r);
	DECLARE_WRITE16_MEMBER(vaportra_priority_w);
	DECLARE_WRITE16_MEMBER(vaportra_palette_24bit_rg_w);
	DECLARE_WRITE16_MEMBER(vaportra_palette_24bit_b_w);
	DECLARE_DRIVER_INIT(vaportra);
	virtual void machine_start();
	virtual void machine_reset();
	UINT32 screen_update_vaportra(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void update_24bitcol( int offset );
	int bank_callback(int bank);
};
