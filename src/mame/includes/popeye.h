// license:BSD-3-Clause
// copyright-holders:smf, Nicola Salmoria, Couriersud
// thanks-to: Marc Lafontaine
#ifndef MAME_INCLUDES_POPEYE_H
#define MAME_INCLUDES_POPEYE_H

#pragma once

#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "video/resnet.h"
#include "emupal.h"

class tnx1_state : public driver_device
{
public:
	tnx1_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_aysnd(*this, "aysnd"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_dmasource(*this, "dmasource"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_color_prom(*this, "proms"),
		m_color_prom_spr(*this, "sprpal")
	{ }

	DECLARE_CUSTOM_INPUT_MEMBER(dsw1_read);
	DECLARE_CUSTOM_INPUT_MEMBER(pop_field_r);

	virtual void config(machine_config &config);

protected:
	required_device<z80_device> m_maincpu;
	required_device<ay8910_device> m_aysnd;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint8_t> m_dmasource;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_region_ptr<uint8_t> m_color_prom;
	required_region_ptr<uint8_t> m_color_prom_spr;

	static const res_net_decode_info mb7051_decode_info;
	static const res_net_decode_info mb7052_decode_info;
	static const res_net_info txt_mb7051_net_info;
	static const res_net_info tnx1_bak_mb7051_net_info;
	static const res_net_info obj_mb7052_net_info;
	virtual const res_net_info bak_mb7051_net_info() { return tnx1_bak_mb7051_net_info; };

	std::unique_ptr<bitmap_ind16> m_sprite_bitmap;
	std::vector<uint8_t> m_sprite_ram;
	std::vector<uint8_t> m_background_ram;
	uint8_t m_background_scroll[3];
	tilemap_t *m_fg_tilemap;
	uint8_t m_palette_bank;
	uint8_t m_palette_bank_cache;
	int   m_field;
	uint8_t m_prot0;
	uint8_t m_prot1;
	uint8_t m_prot_shift;
	uint8_t m_dswbit;
	bool m_nmi_enabled;

	virtual DECLARE_WRITE8_MEMBER(refresh_w);
	DECLARE_READ8_MEMBER(protection_r);
	DECLARE_WRITE8_MEMBER(protection_w);
	DECLARE_WRITE8_MEMBER(popeye_videoram_w);
	DECLARE_WRITE8_MEMBER(popeye_colorram_w);
	virtual DECLARE_WRITE8_MEMBER(background_w);
	DECLARE_WRITE8_MEMBER(popeye_portB_w);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void driver_start() override;
	virtual void video_start() override;
	virtual void tnx1_palette(palette_device &palette);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	virtual DECLARE_WRITE_LINE_MEMBER(screen_vblank);
	void update_palette();
	virtual void decrypt_rom();
	virtual void draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_field(bitmap_ind16 &bitmap, const rectangle &cliprect);

	virtual void maincpu_program_map(address_map &map);
	void maincpu_io_map(address_map &map);

	virtual bool bootleg_sprites() const { return false; }
};

class tpp1_state : public tnx1_state
{
	using tnx1_state::tnx1_state;
protected:
	virtual void tnx1_palette(palette_device &palette) override;
	virtual void draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect) override;

	static const res_net_info tpp1_bak_mb7051_net_info;
	virtual const res_net_info bak_mb7051_net_info() override { return tpp1_bak_mb7051_net_info; };
};

class popeyebl_state : public tpp1_state
{
	using tpp1_state::tpp1_state;
protected:
	virtual void decrypt_rom() override;
	virtual void maincpu_program_map(address_map &map) override;

	virtual bool bootleg_sprites() const override { return true; }
};

class tpp2_state : public tpp1_state
{
	using tpp1_state::tpp1_state;
public:
	virtual void config(machine_config &config) override;
protected:
	bool m_watchdog_enabled;
	uint8_t m_watchdog_counter;

	virtual void driver_start() override;
	virtual DECLARE_WRITE8_MEMBER(refresh_w) override;
	virtual DECLARE_WRITE_LINE_MEMBER(screen_vblank) override;
	virtual void maincpu_program_map(address_map &map) override;
	virtual void decrypt_rom() override;
	virtual void draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect) override;
	virtual DECLARE_WRITE8_MEMBER(background_w) override;
};

class tpp2_noalu_state : public tpp2_state
{
	using tpp2_state::tpp2_state;

protected:
	virtual void maincpu_program_map(address_map &map) override;
};

#endif // MAME_INCLUDES_POPEYE_H
