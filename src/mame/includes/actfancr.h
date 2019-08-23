// license:BSD-3-Clause
// copyright-holders:Bryan McPhail

#include "machine/gen_latch.h"
#include "video/decbac06.h"
#include "video/decmxc06.h"
#include "cpu/h6280/h6280.h"

/*************************************************************************

    Act Fancer

*************************************************************************/

class actfancr_state : public driver_device
{
public:
	actfancr_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_tilegen(*this, "tilegen%u", 1U),
		m_spritegen(*this, "spritegen"),
		m_soundlatch(*this, "soundlatch") { }

	/* memory pointers */
	std::unique_ptr<uint16_t[]> m_spriteram16; // a 16-bit copy of spriteram for use with the MXC06 code

	/* misc */
	int            m_trio_control_select;

	/* devices */
	required_device<h6280_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device_array<deco_bac06_device, 2> m_tilegen;
	required_device<deco_mxc06_device> m_spritegen;
	required_device<generic_latch_8_device> m_soundlatch;

	DECLARE_WRITE8_MEMBER(triothep_control_select_w);
	DECLARE_READ8_MEMBER(triothep_control_r);
	DECLARE_WRITE8_MEMBER(buffer_spriteram_w);
	DECLARE_MACHINE_START(triothep);
	DECLARE_MACHINE_RESET(triothep);
	virtual void video_start() override;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void triothep(machine_config &config);
	void actfancr(machine_config &config);
	void actfan_map(address_map &map);
	void dec0_s_map(address_map &map);
	void triothep_map(address_map &map);
};
