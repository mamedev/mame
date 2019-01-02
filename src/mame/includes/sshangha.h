// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Charles MacDonald, David Haywood
#include "video/deco16ic.h"
#include "video/decospr.h"
#include "machine/deco146.h"
#include "emupal.h"

class sshangha_state : public driver_device
{
public:
	sshangha_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_deco146(*this, "ioprot"),
		m_tilegen(*this, "tilegen"),
		m_spriteram(*this, "spriteram"),
		m_spriteram2(*this, "spriteram2"),
		m_sound_shared_ram(*this, "sound_shared"),
		m_pf1_rowscroll(*this, "pf1_rowscroll"),
		m_pf2_rowscroll(*this, "pf2_rowscroll"),
		m_prot_data(*this, "prot_data"),
		m_sprgen1(*this, "spritegen1"),
		m_sprgen2(*this, "spritegen2"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_palette(*this, "palette")
	{ }

	void sshanghab(machine_config &config);
	void sshangha(machine_config &config);

	void init_sshangha();

protected:
	virtual void video_start() override;

private:
	optional_device<deco146_device> m_deco146;
	required_device<deco16ic_device> m_tilegen;
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_spriteram2;

	required_shared_ptr<uint16_t> m_sound_shared_ram;
	required_shared_ptr<uint16_t> m_pf1_rowscroll;
	required_shared_ptr<uint16_t> m_pf2_rowscroll;

	optional_shared_ptr<uint16_t> m_prot_data;

	required_device<decospr_device> m_sprgen1;
	required_device<decospr_device> m_sprgen2;
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<palette_device> m_palette;

	int m_video_control;
	DECO16IC_BANK_CB_MEMBER(bank_callback);

	DECLARE_READ16_MEMBER(sshangha_protection_region_8_146_r);
	DECLARE_WRITE16_MEMBER(sshangha_protection_region_8_146_w);
	DECLARE_READ16_MEMBER(sshangha_protection_region_d_146_r);
	DECLARE_WRITE16_MEMBER(sshangha_protection_region_d_146_w);
	DECLARE_READ16_MEMBER(deco_71_r);
	DECLARE_READ16_MEMBER(sshanghab_protection16_r);

	DECLARE_READ16_MEMBER(palette_r);
	DECLARE_WRITE16_MEMBER(palette_w);

	DECLARE_READ8_MEMBER(sound_shared_r);
	DECLARE_WRITE8_MEMBER(sound_shared_w);

	DECLARE_WRITE16_MEMBER(video_w);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void sshangha_map(address_map &map);
	void sound_map(address_map &map);
	void sshanghab_map(address_map &map);
};
