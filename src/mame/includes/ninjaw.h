// license:BSD-3-Clause
// copyright-holders:David Graves
/*************************************************************************

    Taito Triple Screen Games

*************************************************************************/

#include "audio/taitosnd.h"
#include "machine/taitoio.h"
#include "sound/flt_vol.h"
#include "video/tc0100scn.h"
#include "video/tc0110pcr.h"


class ninjaw_state : public driver_device
{
public:
	ninjaw_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_tc0140syt(*this, "tc0140syt"),
		m_tc0100scn_1(*this, "tc0100scn_1"),
		m_tc0100scn_2(*this, "tc0100scn_2"),
		m_tc0100scn_3(*this, "tc0100scn_3"),
		m_tc0110pcr_1(*this, "tc0110pcr_1"),
		m_tc0110pcr_2(*this, "tc0110pcr_2"),
		m_tc0110pcr_3(*this, "tc0110pcr_3"),
		m_2610_1l(*this, "2610.1.l"),
		m_2610_1r(*this, "2610.1.r"),
		m_2610_2l(*this, "2610.2.l"),
		m_2610_2r(*this, "2610.2.r"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram") { }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<tc0140syt_device> m_tc0140syt;
	required_device<tc0100scn_device> m_tc0100scn_1;
	required_device<tc0100scn_device> m_tc0100scn_2;
	required_device<tc0100scn_device> m_tc0100scn_3;
	required_device<tc0110pcr_device> m_tc0110pcr_1;
	required_device<tc0110pcr_device> m_tc0110pcr_2;
	required_device<tc0110pcr_device> m_tc0110pcr_3;
	required_device<filter_volume_device> m_2610_1l;
	required_device<filter_volume_device> m_2610_1r;
	required_device<filter_volume_device> m_2610_2l;
	required_device<filter_volume_device> m_2610_2r;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	/* memory pointers */
	required_shared_ptr<uint16_t> m_spriteram;

	/* misc */
	uint16_t     m_cpua_ctrl;
	int        m_pandata[4];

	DECLARE_WRITE8_MEMBER(coin_control_w);
	DECLARE_WRITE16_MEMBER(cpua_ctrl_w);
	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);
	DECLARE_WRITE16_MEMBER(sound_w);
	DECLARE_READ16_MEMBER(sound_r);
	DECLARE_WRITE8_MEMBER(pancontrol_w);
	DECLARE_WRITE16_MEMBER(tc0100scn_triple_screen_w);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	uint32_t screen_update_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_middle(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void postload();
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int primask, int x_offs, int y_offs );
	void parse_control(  );
	uint32_t update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int xoffs, tc0100scn_device *tc0100scn);
	void darius2(machine_config &config);
	void ninjaw(machine_config &config);
	void darius2_master_map(address_map &map);
	void darius2_slave_map(address_map &map);
	void ninjaw_master_map(address_map &map);
	void ninjaw_slave_map(address_map &map);
	void sound_map(address_map &map);
};
