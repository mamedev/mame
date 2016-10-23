// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*************************************************************************

    Desert Assault

*************************************************************************/

#include "machine/gen_latch.h"
#include "sound/okim6295.h"
#include "video/deco16ic.h"
#include "video/decocomn.h"
#include "video/bufsprite.h"
#include "video/decospr.h"

class dassault_state : public driver_device
{
public:
	dassault_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_subcpu(*this, "sub"),
		m_decocomn(*this, "deco_common"),
		m_deco_tilegen1(*this, "tilegen1"),
		m_deco_tilegen2(*this, "tilegen2"),
		m_oki2(*this, "oki2"),
		m_spriteram(*this, "spriteram"),
		m_spriteram2(*this, "spriteram2") ,
		m_sprgen1(*this, "spritegen1"),
		m_sprgen2(*this, "spritegen2"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_pf2_rowscroll(*this, "pf2_rowscroll"),
		m_pf4_rowscroll(*this, "pf4_rowscroll"),
		m_ram(*this, "ram"),
		m_shared_ram(*this, "shared_ram"),
		m_ram2(*this, "ram2")

	{ }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_subcpu;
	required_device<decocomn_device> m_decocomn;
	required_device<deco16ic_device> m_deco_tilegen1;
	required_device<deco16ic_device> m_deco_tilegen2;
	required_device<okim6295_device> m_oki2;
	required_device<buffered_spriteram16_device> m_spriteram;
	required_device<buffered_spriteram16_device> m_spriteram2;
	optional_device<decospr_device> m_sprgen1;
	optional_device<decospr_device> m_sprgen2;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	/* memory pointers */
	required_shared_ptr<uint16_t> m_pf2_rowscroll;
	required_shared_ptr<uint16_t> m_pf4_rowscroll;
	required_shared_ptr<uint16_t> m_ram;
	required_shared_ptr<uint16_t> m_shared_ram;
	required_shared_ptr<uint16_t> m_ram2;

	uint16_t dassault_control_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void dassault_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t dassault_sub_control_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void dassault_sound_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t dassault_irq_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void dassault_irq_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void shared_ram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t shared_ram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void sound_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_thndzone();
	void init_dassault();
	virtual void video_start() override;
	uint32_t screen_update_dassault(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void mixdassaultlayer(bitmap_rgb32 &bitmap, bitmap_ind16* sprite_bitmap, const rectangle &cliprect, uint16_t pri, uint16_t primask, uint16_t penbase, uint8_t alpha);
	DECO16IC_BANK_CB_MEMBER(bank_callback);
};
