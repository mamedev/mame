// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*************************************************************************

    Crude Buster

*************************************************************************/

#include "machine/gen_latch.h"
#include "cpu/h6280/h6280.h"
#include "video/bufsprite.h"
#include "video/decospr.h"
#include "video/deco16ic.h"
#include "emupal.h"

class cbuster_state : public driver_device
{
public:
	cbuster_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_deco_tilegen(*this, "tilegen%u", 1U)
		, m_palette(*this, "palette")
		, m_spriteram(*this, "spriteram")
		, m_soundlatch(*this, "soundlatch")
		, m_sprgen(*this, "spritegen")
		, m_pf_rowscroll(*this, "pf%u_rowscroll", 1U)
		, m_paletteram(*this, "palette")
		, m_paletteram_ext(*this, "palette_ext")
	{ }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<h6280_device> m_audiocpu;
	required_device_array<deco16ic_device, 2> m_deco_tilegen;
	required_device<palette_device> m_palette;
	required_device<buffered_spriteram16_device> m_spriteram;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<decospr_device> m_sprgen;

	/* memory pointers */
	required_shared_ptr_array<uint16_t, 4> m_pf_rowscroll;
	required_shared_ptr<uint16_t> m_paletteram;
	required_shared_ptr<uint16_t> m_paletteram_ext;

	/* misc */
	uint16_t    m_prot;
	int       m_pri;

	DECLARE_WRITE16_MEMBER(twocrude_control_w);
	DECLARE_READ16_MEMBER(twocrude_control_r);
	void init_twocrude();
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_twocrude(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECO16IC_BANK_CB_MEMBER(bank_callback);
	DECLARE_WRITE16_MEMBER(palette_w);
	DECLARE_WRITE16_MEMBER(palette_ext_w);
	void update_palette(int offset);
	void twocrude(machine_config &config);
	void sound_map(address_map &map);
	void twocrude_map(address_map &map);
};
