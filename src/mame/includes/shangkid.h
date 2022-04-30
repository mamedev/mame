// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
#ifndef MAME_INCLUDES_SHANGKID_H
#define MAME_INCLUDES_SHANGKID_H

#pragma once

#include "sound/ay8910.h"
#include "emupal.h"
#include "tilemap.h"

class dynamski_state : public driver_device
{
public:
	dynamski_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram")
	{ }

	void dynamski(machine_config &config);

protected:
	virtual void machine_start() override;

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;

	bool m_int_enable[2];

	DECLARE_WRITE_LINE_MEMBER(int_enable_1_w);
	DECLARE_WRITE_LINE_MEMBER(irq_1_w);

private:
	void palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect, int pri);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprec );

	void prgmap(address_map &map);
	void portmap(address_map &map);
};

class chinhero_state : public dynamski_state
{
public:
	chinhero_state(const machine_config &mconfig, device_type type, const char *tag) :
		dynamski_state(mconfig, type, tag),
		m_audiocpu(*this, "audiocpu"),
		m_bbx(*this, "bbx"),
		m_aysnd(*this, "aysnd"),
		m_videoreg(*this, "videoreg")
	{ }

	void chinhero(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void video_start() override;

	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_bbx;
	required_device<ay8910_device> m_aysnd;
	required_shared_ptr<uint8_t> m_videoreg;

	int m_gfx_type = 0;

	void nmiq_1_w(uint8_t data);
	void nmiq_2_w(uint8_t data);
	void videoram_w(offs_t offset, uint8_t data);

private:
	bool m_nmi_enable[2]{};
	tilemap_t *m_background = nullptr;

	void ay8910_porta_w(uint8_t data);

	DECLARE_WRITE_LINE_MEMBER(sound_enable_w);
	DECLARE_WRITE_LINE_MEMBER(int_enable_2_w);
	DECLARE_WRITE_LINE_MEMBER(nmi_enable_1_w);
	DECLARE_WRITE_LINE_MEMBER(nmi_enable_2_w);
	DECLARE_WRITE_LINE_MEMBER(irq_2_w);
	DECLARE_WRITE_LINE_MEMBER(coin_counter_1_w);
	DECLARE_WRITE_LINE_MEMBER(coin_counter_2_w);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprite(const uint8_t *source, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void bbx_map(address_map &map);
	void bbx_portmap(address_map &map);
	void main_map(address_map &map);
	void sound_map(address_map &map);
	void sound_portmap(address_map &map);
};

class shangkid_state : public chinhero_state
{
public:
	shangkid_state(const machine_config &mconfig, device_type type, const char *tag) :
		chinhero_state(mconfig, type, tag),
		m_soundbank(*this, "soundbank")
	{ }

	void shangkid(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_memory_bank m_soundbank;

	void ay8910_porta_w(uint8_t data);

	void bbx_map(address_map &map);
	void bbx_portmap(address_map &map);
	void main_map(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_INCLUDES_SHANGKID_H
