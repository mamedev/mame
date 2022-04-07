// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*************************************************************************

    Desert Assault

*************************************************************************/

#include "machine/gen_latch.h"
#include "cpu/h6280/h6280.h"
#include "sound/okim6295.h"
#include "video/deco16ic.h"
#include "video/decocomn.h"
#include "video/bufsprite.h"
#include "video/decospr.h"
#include "emupal.h"

class dassault_state : public driver_device
{
public:
	dassault_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_subcpu(*this, "sub")
		, m_deco_tilegen(*this, "tilegen%u", 1U)
		, m_oki2(*this, "oki2")
		, m_spriteram(*this, "spriteram%u", 1U)
		, m_sprgen(*this, "spritegen%u", 1U)
		, m_palette(*this, "palette")
		, m_soundlatch(*this, "soundlatch")
		, m_pf2_rowscroll(*this, "pf2_rowscroll")
		, m_pf4_rowscroll(*this, "pf4_rowscroll")
	{ }

	void dassault(machine_config &config);

	void init_dassault();

private:
	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<h6280_device> m_audiocpu;
	required_device<cpu_device> m_subcpu;
	required_device_array<deco16ic_device, 2> m_deco_tilegen;
	required_device<okim6295_device> m_oki2;
	required_device_array<buffered_spriteram16_device, 2> m_spriteram;
	required_device_array<decospr_device, 2> m_sprgen;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	/* memory pointers */
	required_shared_ptr<uint16_t> m_pf2_rowscroll;
	required_shared_ptr<uint16_t> m_pf4_rowscroll;

	uint16_t m_priority = 0U;

	void priority_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void main_irq_ack_w(uint16_t data);
	void sub_irq_ack_w(uint16_t data);
	uint16_t dassault_control_r(offs_t offset);
	void dassault_control_w(uint16_t data);
	uint16_t dassault_sub_control_r();
	void sound_bankswitch_w(uint8_t data);
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_dassault(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void mixdassaultlayer(bitmap_rgb32 &bitmap, bitmap_ind16* sprite_bitmap, const rectangle &cliprect, uint16_t pri, uint16_t primask, uint16_t penbase, uint8_t alpha);
	DECO16IC_BANK_CB_MEMBER(bank_callback);
	void dassault_map(address_map &map);
	void dassault_sub_map(address_map &map);
	void sound_map(address_map &map);
};
