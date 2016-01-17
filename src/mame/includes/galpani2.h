// license:BSD-3-Clause
// copyright-holders:Luca Elia
#include "video/kaneko_spr.h"
#include "sound/okim6295.h"
#include "machine/eepromser.h"

class galpani2_state : public driver_device
{
public:
	galpani2_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_subcpu(*this,"sub"),
		m_kaneko_spr(*this, "kan_spr"),
		m_oki2(*this, "oki2"),
		m_eeprom(*this, "eeprom"),
		m_palette(*this, "palette"),
		m_bg15palette(*this, "bgpalette"),
		m_bg8palette(*this, "bg8palette"),
		m_bg8(*this, "bg8"),
		m_palette_val(*this, "palette"),
		m_bg8_scrollx(*this, "bg8_scrollx"),
		m_bg8_scrolly(*this, "bg8_scrolly"),
		m_bg15(*this, "bg15"),
		m_ram(*this, "ram"),
		m_ram2(*this, "ram2"),
		m_spriteram(*this, "spriteram")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	optional_device<kaneko16_sprite_device> m_kaneko_spr;
	required_device<okim6295_device> m_oki2;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<palette_device> m_palette;
	required_device<palette_device> m_bg15palette;
	required_device<palette_device> m_bg8palette;

	required_shared_ptr_array<UINT16, 2> m_bg8;
	optional_shared_ptr_array<UINT16, 2> m_palette_val;
	required_shared_ptr_array<UINT16, 2> m_bg8_scrollx;
	required_shared_ptr_array<UINT16, 2> m_bg8_scrolly;
	required_shared_ptr<UINT16> m_bg15;
	required_shared_ptr<UINT16> m_ram;
	required_shared_ptr<UINT16> m_ram2;
	optional_shared_ptr<UINT16> m_spriteram;

	UINT16 m_eeprom_word;
	UINT16 m_old_mcu_nmi1;
	UINT16 m_old_mcu_nmi2;

	DECLARE_WRITE8_MEMBER(galpani2_mcu_init_w);
	DECLARE_WRITE8_MEMBER(galpani2_mcu_nmi1_w);
	DECLARE_WRITE8_MEMBER(galpani2_mcu_nmi2_w);
	DECLARE_WRITE8_MEMBER(galpani2_coin_lockout_w);
	DECLARE_READ16_MEMBER(galpani2_eeprom_r);
	DECLARE_WRITE16_MEMBER(galpani2_eeprom_w);
	DECLARE_WRITE8_MEMBER(galpani2_oki1_bank_w);
	DECLARE_WRITE8_MEMBER(galpani2_oki2_bank_w);
	DECLARE_WRITE16_MEMBER(subdatabank_select_w);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(galpani2);
	UINT32 screen_update_galpani2(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void copybg8(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int layer);
	void copybg15(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	TIMER_DEVICE_CALLBACK_MEMBER(galpani2_interrupt1);
	TIMER_DEVICE_CALLBACK_MEMBER(galpani2_interrupt2);
	void galpani2_mcu_nmi1();
	void galpani2_mcu_nmi2();
};
