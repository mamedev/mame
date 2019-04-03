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
#include "emupal.h"


class warriorb_state : public driver_device
{
public:
	warriorb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_tc0140syt(*this, "tc0140syt"),
		m_tc0100scn(*this, "tc0100scn_%u", 1),
		m_tc0110pcr(*this, "tc0110pcr_%u", 1),
		m_tc0220ioc(*this, "tc0220ioc"),
		m_tc0510nio(*this, "tc0510nio"),
		m_2610_l(*this, "2610.%u.l", 1),
		m_2610_r(*this, "2610.%u.r", 1),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram"),
		m_z80bank(*this, "z80bank") { }

	void warriorb(machine_config &config);
	void darius2d(machine_config &config);

private:
	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<tc0140syt_device> m_tc0140syt;
	required_device_array<tc0100scn_device, 2> m_tc0100scn;
	required_device_array<tc0110pcr_device, 2> m_tc0110pcr;
	optional_device<tc0220ioc_device> m_tc0220ioc;
	optional_device<tc0510nio_device> m_tc0510nio;
	required_device_array<filter_volume_device, 2> m_2610_l;
	required_device_array<filter_volume_device, 2> m_2610_r;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	/* memory pointers */
	required_shared_ptr<uint16_t> m_spriteram;

	/* memory regions */
	required_memory_bank m_z80bank;

	/* misc */
	int        m_pandata[4];

	void coin_control_w(u8 data);
	void sound_bankswitch_w(u8 data);
	void pancontrol_w(offs_t offset, u8 data);
	DECLARE_WRITE16_MEMBER(tc0100scn_dual_screen_w);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	uint32_t screen_update_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int x_offs, int y_offs );
	uint32_t update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int xoffs, int chip);

	void darius2d_map(address_map &map);
	void warriorb_map(address_map &map);
	void z80_sound_map(address_map &map);
};
