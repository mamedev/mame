// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*************************************************************************

    Caveman Ninja (and other DECO 16bit titles)

*************************************************************************/

#include "sound/okim6295.h"
#include "video/deco16ic.h"
#include "video/decocomn.h"
#include "video/bufsprite.h"
#include "video/decospr.h"
#include "machine/deco146.h"
#include "machine/deco104.h"
#include "machine/gen_latch.h"

class cninja_state : public driver_device
{
public:
	cninja_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_deco146(*this, "ioprot"),
		m_deco104(*this, "ioprot104"),
		m_decocomn(*this, "deco_common"),
		m_deco_tilegen1(*this, "tilegen1"),
		m_deco_tilegen2(*this, "tilegen2"),
		m_raster_irq_timer(*this, "raster_timer"),
		m_oki2(*this, "oki2"),
		m_sprgen(*this, "spritegen"),
		m_sprgen1(*this, "spritegen1"),
		m_sprgen2(*this, "spritegen2"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_spriteram(*this, "spriteram"),
		m_spriteram2(*this, "spriteram2") ,
		m_pf1_rowscroll(*this, "pf1_rowscroll"),
		m_pf2_rowscroll(*this, "pf2_rowscroll"),
		m_pf3_rowscroll(*this, "pf3_rowscroll"),
		m_pf4_rowscroll(*this, "pf4_rowscroll"),
		m_ram(*this, "ram")
	{ }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<deco146_device> m_deco146;
	optional_device<deco104_device> m_deco104;
	required_device<decocomn_device> m_decocomn;
	required_device<deco16ic_device> m_deco_tilegen1;
	required_device<deco16ic_device> m_deco_tilegen2;
	optional_device<timer_device> m_raster_irq_timer;
	optional_device<okim6295_device> m_oki2;
	optional_device<decospr_device> m_sprgen;
	optional_device<decospr_device> m_sprgen1;
	optional_device<decospr_device> m_sprgen2;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<buffered_spriteram16_device> m_spriteram;
	optional_device<buffered_spriteram16_device> m_spriteram2;

	/* memory pointers */
	required_shared_ptr<uint16_t> m_pf1_rowscroll;
	required_shared_ptr<uint16_t> m_pf2_rowscroll;
	required_shared_ptr<uint16_t> m_pf3_rowscroll;
	required_shared_ptr<uint16_t> m_pf4_rowscroll;
	optional_shared_ptr<uint16_t> m_ram;

	/* misc */
	int        m_scanline;
	int        m_irq_mask;

	void cninja_sound_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void stoneage_sound_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t cninja_irq_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void cninja_irq_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void cninja_pf12_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void cninja_pf34_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sound_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_stoneage();
	void init_mutantf();
	void init_cninja();
	void init_cninjabl2();
	virtual void machine_start() override;
	virtual void machine_reset() override;
	void video_start_stoneage();
	void video_start_mutantf();
	uint32_t screen_update_cninja(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_cninjabl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_cninjabl2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_edrandy(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_robocop2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_mutantf(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void interrupt_gen(timer_device &timer, void *ptr, int32_t param);
	void cninjabl_draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect );

	DECO16IC_BANK_CB_MEMBER(cninja_bank_callback);
	DECO16IC_BANK_CB_MEMBER(robocop2_bank_callback);
	DECO16IC_BANK_CB_MEMBER(mutantf_1_bank_callback);
	DECO16IC_BANK_CB_MEMBER(mutantf_2_bank_callback);

	DECOSPR_PRIORITY_CB_MEMBER(pri_callback);

	uint16_t sshangha_protection_region_6_146_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void sshangha_protection_region_6_146_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t sshangha_protection_region_8_146_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void sshangha_protection_region_8_146_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	uint16_t mutantf_protection_region_0_146_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void mutantf_protection_region_0_146_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t cninja_protection_region_0_104_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void cninja_protection_region_0_104_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	uint16_t cninjabl2_sprite_dma_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
};
