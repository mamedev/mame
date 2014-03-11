/*************************************************************************

    Hot Pinball
    Gals Pinball

*************************************************************************/

#include "video/tecmo_spr.h"

class galspnbl_state : public driver_device
{
public:
	galspnbl_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_bgvideoram(*this, "bgvideoram"),
		m_scroll(*this, "scroll"),
		m_audiocpu(*this, "audiocpu"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_colorram;
	required_shared_ptr<UINT16> m_videoram;
	required_shared_ptr<UINT16> m_bgvideoram;
	required_shared_ptr<UINT16> m_scroll;

	/* devices */
	required_device<cpu_device> m_audiocpu;
	DECLARE_WRITE16_MEMBER(soundcommand_w);
	virtual void machine_start();
	DECLARE_PALETTE_INIT(galspnbl);
	UINT32 screen_update_galspnbl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_background( bitmap_ind16 &bitmap, const rectangle &cliprect );
	DECLARE_WRITE_LINE_MEMBER(irqhandler);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
