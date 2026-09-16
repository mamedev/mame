// license:BSD-3-Clause
// copyright-holders:Luca Elia
#ifndef MAME_KANEKO_GALPANI2_H
#define MAME_KANEKO_GALPANI2_H

#pragma once

#include "kaneko_spr.h"
#include "sound/okim6295.h"
#include "machine/eepromser.h"
#include "machine/timer.h"
#include "emupal.h"

class galpani2_state : public driver_device
{
public:
	galpani2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_subcpu(*this,"sub"),
		m_kaneko_spr(*this, "kan_spr"),
		m_oki2(*this, "oki2"),
		m_eeprom(*this, "eeprom"),
		m_palette(*this, "palette"),
		m_bg15palette(*this, "bgpalette"),
		m_bg8palette(*this, "bg8palette"),
		m_bg8(*this, "bg8.%u", 0),
		m_palette_val(*this, "palette.%u", 0),
		m_bg8_scrollx(*this, "bg8_scrollx.%u", 0),
		m_bg8_scrolly(*this, "bg8_scrolly.%u", 0),
		m_bg15(*this, "bg15"),
		m_ram(*this, "ram"),
		m_ram2(*this, "ram2"),
		m_spriteram(*this, "spriteram")
	{ }

	void galpani2(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	optional_device<kaneko16_sprite_device> m_kaneko_spr;
	required_device<okim6295_device> m_oki2;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<palette_device> m_palette;
	required_device<palette_device> m_bg15palette;
	required_device<palette_device> m_bg8palette;

	required_shared_ptr_array<uint16_t, 2> m_bg8;
	optional_shared_ptr_array<uint16_t, 2> m_palette_val;
	required_shared_ptr_array<uint16_t, 2> m_bg8_scrollx;
	required_shared_ptr_array<uint16_t, 2> m_bg8_scrolly;
	required_shared_ptr<uint16_t> m_bg15;
	required_shared_ptr<uint16_t> m_ram;
	required_shared_ptr<uint16_t> m_ram2;
	optional_shared_ptr<uint16_t> m_spriteram;

	uint16_t m_eeprom_word = 0U;
	uint16_t m_old_mcu_nmi1 = 0U;
	uint16_t m_old_mcu_nmi2 = 0U;

	void galpani2_mcu_init_w(uint8_t data);
	void galpani2_mcu_nmi1_w(uint8_t data);
	void galpani2_mcu_nmi2_w(uint8_t data);
	void galpani2_coin_lockout_w(uint8_t data);
	uint16_t galpani2_eeprom_r();
	void galpani2_eeprom_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void galpani2_oki1_bank_w(uint8_t data);
	void galpani2_oki2_bank_w(uint8_t data);
	void subdatabank_select_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	uint32_t screen_update_galpani2(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void copybg8(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int layer);
	void copybg15(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	TIMER_DEVICE_CALLBACK_MEMBER(galpani2_interrupt1);
	TIMER_DEVICE_CALLBACK_MEMBER(galpani2_interrupt2);
	void galpani2_mcu_nmi1();
	void galpani2_mcu_nmi2();
	void galpani2_mem1(address_map &map) ATTR_COLD;
	void galpani2_mem2(address_map &map) ATTR_COLD;
};

#endif // MAME_KANEKO_GALPANI2_H
