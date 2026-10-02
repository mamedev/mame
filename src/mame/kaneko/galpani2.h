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
		m_bg8_palette_ram(*this, "bg8palette"),
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

	void init_asia();
	void init_japan();
	void init_korea();
	void init_special();

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	optional_device<kaneko16_sprite_device> m_kaneko_spr;
	required_device<okim6295_device> m_oki2;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<palette_device> m_palette;
	required_device<palette_device> m_bg15palette;
	required_device<palette_device> m_bg8palette;

	required_shared_ptr<uint16_t> m_bg8_palette_ram;
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

	// Regional background image offset table (set per game in init)
	const uint32_t *m_bg_image_offsets = nullptr;
	uint32_t m_bg_image_count = 0;
	const uint32_t *m_bg_image_prefix_offsets = nullptr;
	uint32_t m_bg_image_prefix_count = 0;

	// Per-region field/mask layout (set in init functions).
	// English/Asia/Japan/Korea use field x=48, mask pen 00A0/80A0.
	// SE uses field x=32, mask pen 0050/8050.
	int      m_field_origin_x = 48;
	uint16_t m_mask_girl_pen = 0x80a0;
	uint16_t m_mask_nongirl_pen = 0x00a0;
	uint8_t  m_mask_captured_lo = 0xa0;  // SE writes 0x60 to plane 0 on capture
	uint32_t m_girl_total_addr = 0x109e28;  // where the game stores girl_total
	uint32_t m_other_total_addr = 0x109e2a;

	// Deferred girl-bitmap HLE: the bitmap is generated after the
	// photo decode completes, not at MCU command time when BG15 may
	// still contain stale data from the previous round.
	static constexpr uint32_t GIRL_BITMAP_OFFSET = 0xe000;
	static constexpr uint32_t GIRL_BITMAP_ADDR   = 0x10e000;
	uint32_t m_girl_bitmap_pending_addr = 0;
	int      m_girl_bitmap_delay = 0;

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
	void copybg8(bitmap_rgb32 &bitmap, bitmap_ind8 &priority_bitmap, const rectangle &cliprect, int layer, bool foreground);
	void copybg15(bitmap_rgb32 &bitmap, bitmap_ind8 &priority_bitmap, const rectangle &cliprect, int priority);

	TIMER_DEVICE_CALLBACK_MEMBER(galpani2_interrupt1);
	TIMER_DEVICE_CALLBACK_MEMBER(galpani2_interrupt2);
	void galpani2_mcu_nmi1();
	void galpani2_mcu_nmi2();
	uint16_t generate_girl_bitmap(address_space &mspace);
	void galpani2_mem1(address_map &map) ATTR_COLD;
	void galpani2_mem2(address_map &map) ATTR_COLD;
};

#endif // MAME_KANEKO_GALPANI2_H
