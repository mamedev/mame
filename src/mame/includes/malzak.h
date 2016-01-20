// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, Barry Rodewald
/*************************************************************************

    Malzak

*************************************************************************/

#include "machine/s2636.h"
#include "video/saa5050.h"

class malzak_state : public driver_device
{
public:
	malzak_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_s2636_0(*this, "s2636_0"),
			m_s2636_1(*this, "s2636_1"),
			m_trom(*this, "saa5050"),
			m_videoram(*this, "videoram"),
			m_gfxdecode(*this, "gfxdecode"),
			m_screen(*this, "screen"),
			m_palette(*this, "palette") { }

	required_device<cpu_device> m_maincpu;
	required_device<s2636_device> m_s2636_0;
	required_device<s2636_device> m_s2636_1;
	required_device<saa5050_device> m_trom;
	required_shared_ptr<UINT8> m_videoram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	/* misc */
//  int playfield_x[256];
//  int playfield_y[256];
	int m_playfield_code[256];
	int m_malzak_x;
	int m_malzak_y;
	int m_collision_counter;

	/* devices */
	device_t *m_saa5050;
	DECLARE_READ8_MEMBER(fake_VRLE_r);
	DECLARE_READ8_MEMBER(s2636_portA_r);
	DECLARE_READ8_MEMBER(s2650_data_r);
	DECLARE_WRITE8_MEMBER(port40_w);
	DECLARE_WRITE8_MEMBER(port60_w);
	DECLARE_WRITE8_MEMBER(portc0_w);
	DECLARE_READ8_MEMBER(collision_r);
	DECLARE_WRITE8_MEMBER(malzak_playfield_w);
	DECLARE_READ8_MEMBER(videoram_r);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	DECLARE_PALETTE_INIT(malzak);
	UINT32 screen_update_malzak(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};
