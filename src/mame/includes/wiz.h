// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/***************************************************************************

  Seibu Stinger/Wiz hardware

***************************************************************************/

#include "sound/discrete.h"

class wiz_state : public driver_device
{
public:
	wiz_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_discrete(*this, "discrete"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_videoram2(*this, "videoram2"),
		m_colorram(*this, "colorram"),
		m_colorram2(*this, "colorram2"),
		m_attrram(*this, "attrram"),
		m_attrram2(*this, "attrram2"),
		m_spriteram(*this, "spriteram"),
		m_spriteram2(*this, "spriteram2"),
		m_decrypted_opcodes(*this, "decrypted_opcodes")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<discrete_device> m_discrete;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_videoram2;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_colorram2;
	required_shared_ptr<uint8_t> m_attrram;
	required_shared_ptr<uint8_t> m_attrram2;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_spriteram2;
	optional_shared_ptr<uint8_t> m_decrypted_opcodes;

	int32_t m_flipx;
	int32_t m_flipy;
	int32_t m_bgcolor;
	uint8_t m_charbank[2];
	uint8_t m_palbank[2];
	uint8_t m_main_nmi_mask;
	uint8_t m_sound_nmi_mask;
	uint8_t m_sprite_bank;

	int m_dsc0;
	int m_dsc1;

	uint8_t wiz_protection_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void wiz_coin_counter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void wiz_main_nmi_mask_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void wiz_sound_nmi_mask_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void wiz_palette_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void wiz_sprite_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void wiz_bgcolor_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void wiz_char_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void wiz_flipx_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void wiz_flipy_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void stinger_explosion_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void stinger_shot_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void init_stinger();

	virtual void machine_reset() override;
	virtual void machine_start() override;
	void palette_init_wiz(palette_device &palette);
	uint32_t screen_update_wiz(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_stinger(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_kungfut(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void wiz_vblank_interrupt(device_t &device);
	void wiz_sound_interrupt(device_t &device);
	void draw_tiles(bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int charbank, int colortype);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int set, int charbank);
};
