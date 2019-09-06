// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_INCLUDES_SEICROSS_H
#define MAME_INCLUDES_SEICROSS_H

#pragma once

#include "machine/nvram.h"
#include "sound/dac.h"
#include "emupal.h"
#include "tilemap.h"

class seicross_state : public driver_device
{
public:
	seicross_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mcu(*this, "mcu"),
		m_dac(*this, "dac"),
		m_nvram(*this, "nvram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_debug_port(*this, "DEBUG"),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_row_scroll(*this, "row_scroll"),
		m_spriteram2(*this, "spriteram2"),
		m_colorram(*this, "colorram"),
		m_decrypted_opcodes(*this, "decrypted_opcodes")
	{ }

	void no_nvram(machine_config &config);
	void friskytb(machine_config &config);
	void nvram(machine_config &config);

	void init_friskytb();

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_mcu;
	required_device<dac_byte_interface> m_dac;
	optional_device<nvram_device> m_nvram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	optional_ioport m_debug_port;

	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_row_scroll;
	required_shared_ptr<uint8_t> m_spriteram2;
	required_shared_ptr<uint8_t> m_colorram;
	optional_shared_ptr<uint8_t> m_decrypted_opcodes;

	uint8_t m_portb;
	tilemap_t *m_bg_tilemap;
	uint8_t m_irq_mask;

	DECLARE_WRITE8_MEMBER(videoram_w);
	DECLARE_WRITE8_MEMBER(colorram_w);
	DECLARE_READ8_MEMBER(portB_r);
	DECLARE_WRITE8_MEMBER(portB_w);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	INTERRUPT_GEN_MEMBER(vblank_irq);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void seicross_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );

	void nvram_init(nvram_device &nvram, void *data, size_t size);

	DECLARE_WRITE8_MEMBER(dac_w);

	void decrypted_opcodes_map(address_map &map);
	void main_map(address_map &map);
	void main_portmap(address_map &map);
	void mcu_no_nvram_map(address_map &map);
	void mcu_nvram_map(address_map &map);
};

#endif // MAME_INCLUDES_SEICROSS_H
