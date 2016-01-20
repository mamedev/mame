// license:BSD-3-Clause
// copyright-holders:David Graves
/*************************************************************************

    Taito Dual Screen Games

*************************************************************************/

#include "audio/taitosnd.h"
#include "machine/taitoio.h"
#include "sound/flt_vol.h"
#include "video/tc0100scn.h"
#include "video/tc0110pcr.h"


class warriorb_state : public driver_device
{
public:
	warriorb_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_tc0140syt(*this, "tc0140syt"),
		m_tc0100scn_1(*this, "tc0100scn_1"),
		m_tc0100scn_2(*this, "tc0100scn_2"),
		m_tc0110pcr_1(*this, "tc0110pcr_1"),
		m_tc0110pcr_2(*this, "tc0110pcr_2"),
		m_tc0220ioc(*this, "tc0220ioc"),
		m_tc0510nio(*this, "tc0510nio"),
		m_2610_1l(*this, "2610.1.l"),
		m_2610_1r(*this, "2610.1.r"),
		m_2610_2l(*this, "2610.2.l"),
		m_2610_2r(*this, "2610.2.r"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram") { }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<tc0140syt_device> m_tc0140syt;
	required_device<tc0100scn_device> m_tc0100scn_1;
	required_device<tc0100scn_device> m_tc0100scn_2;
	required_device<tc0110pcr_device> m_tc0110pcr_1;
	required_device<tc0110pcr_device> m_tc0110pcr_2;
	optional_device<tc0220ioc_device> m_tc0220ioc;
	optional_device<tc0510nio_device> m_tc0510nio;
	required_device<filter_volume_device> m_2610_1l;
	required_device<filter_volume_device> m_2610_1r;
	required_device<filter_volume_device> m_2610_2l;
	required_device<filter_volume_device> m_2610_2r;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	/* memory pointers */
	required_shared_ptr<UINT16> m_spriteram;

	/* misc */
	int        m_pandata[4];

	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);
	DECLARE_WRITE16_MEMBER(sound_w);
	DECLARE_READ16_MEMBER(sound_r);
	DECLARE_WRITE8_MEMBER(pancontrol);
	DECLARE_WRITE16_MEMBER(tc0100scn_dual_screen_w);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	UINT32 screen_update_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int x_offs, int y_offs );
	UINT32 update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int xoffs, tc0100scn_device *tc0100scn);
};
