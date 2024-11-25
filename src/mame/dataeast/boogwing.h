// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, David Haywood
/*************************************************************************

    Boogie Wings

*************************************************************************/

#include "cpu/h6280/h6280.h"
#include "sound/okim6295.h"
#include "deco16ic.h"
#include "deco_ace.h"
#include "video/bufsprite.h"
#include "decospr.h"
#include "deco104.h"
#include "screen.h"

class boogwing_state : public driver_device
{
public:
	boogwing_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_deco104(*this, "ioprot")
		, m_deco_ace(*this, "deco_ace")
		, m_screen(*this, "screen")
		, m_deco_tilegen(*this, "tilegen%u", 1)
		, m_oki(*this, "oki%u", 1)
		, m_sprgen(*this, "spritegen%u", 1)
		, m_spriteram(*this, "spriteram%u", 1)
		, m_pf_rowscroll(*this, "pf%u_rowscroll", 1)
		, m_decrypted_opcodes(*this, "decrypted_opcodes")
	{ }

	void init_boogwing();
	void boogwing(machine_config &config);

private:
	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<h6280_device> m_audiocpu;
	required_device<deco104_device> m_deco104;
	required_device<deco_ace_device> m_deco_ace;
	required_device<screen_device> m_screen;
	required_device_array<deco16ic_device, 2> m_deco_tilegen;
	required_device_array<okim6295_device, 2> m_oki;
	required_device_array<decospr_device, 2> m_sprgen;
	required_device_array<buffered_spriteram16_device, 2> m_spriteram;
	/* memory pointers */
	required_shared_ptr_array<uint16_t, 4> m_pf_rowscroll;
	required_shared_ptr<uint16_t> m_decrypted_opcodes;

	uint16_t m_priority = 0U;
	bitmap_ind16 m_temp_bitmap = 0;
	bitmap_ind16 m_alpha_tmap_bitmap = 0;

	void sound_bankswitch_w(uint8_t data);
	void priority_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	uint32_t screen_update_boogwing(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void mix_boogwing(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint16_t ioprot_r(offs_t offset);
	void ioprot_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	DECO16IC_BANK_CB_MEMBER(bank_callback);
	DECO16IC_BANK_CB_MEMBER(bank_callback2);
	void audio_map(address_map &map) ATTR_COLD;
	void boogwing_map(address_map &map) ATTR_COLD;
	void decrypted_opcodes_map(address_map &map) ATTR_COLD;
};
