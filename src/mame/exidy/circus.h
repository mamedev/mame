// license:BSD-3-Clause
// copyright-holders:Mike Coates
#ifndef MAME_EXIDY_CIRCUS_H
#define MAME_EXIDY_CIRCUS_H

#pragma once

#include "sound/discrete.h"
#include "sound/samples.h"

#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

// circus

class circus_state : public driver_device
{
public:
	circus_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_samples(*this, "samples"),
		m_discrete(*this, "discrete"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_paddle(*this, "PADDLE")
	{ }

	void base_mcfg(machine_config &config);
	void circus(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<samples_device> m_samples;
	required_device<discrete_sound_device> m_discrete;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint8_t> m_videoram;
	required_ioport m_paddle;

	tilemap_t *m_bg_tilemap = nullptr;
	int16_t m_clown_x = 0;
	int16_t m_clown_y = 0;
	uint8_t m_clown_z = 0;

	void videoram_w(offs_t offset, uint8_t data);
	void clown_x_w(uint8_t data) { m_clown_x = 240 - data; }
	void clown_y_w(uint8_t data) { m_clown_y = 240 - data; }
	void clown_z_w(uint8_t data);
	uint8_t paddle_r();
	virtual void sound_w(uint8_t data);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_line(bitmap_ind16 &bitmap, const rectangle &cliprect, int x1, int y1, int x2, int y2, int dotted);
	void draw_sprite_collision(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;

private:
	void draw_fg(bitmap_ind16 &bitmap, const rectangle &cliprect);
};


// robotbwl

class robotbwl_state : public circus_state
{
public:
	robotbwl_state(const machine_config &mconfig, device_type type, const char *tag) :
		circus_state(mconfig, type, tag)
	{ }

	void robotbwl(machine_config &config);

private:
	virtual uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect) override;
	virtual void sound_w(uint8_t data) override;

	void draw_box(bitmap_ind16 &bitmap, const rectangle &cliprect, int x, int y);
	void draw_scoreboard(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_bowling_alley(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_ball(bitmap_ind16 &bitmap, const rectangle &cliprect);
};


// crash

class crash_state : public circus_state
{
public:
	crash_state(const machine_config &mconfig, device_type type, const char *tag) :
		circus_state(mconfig, type, tag)
	{ }

	void crash(machine_config &config);

private:
	virtual uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect) override;
	virtual void sound_w(uint8_t data) override;

	void draw_car(bitmap_ind16 &bitmap, const rectangle &cliprect);
};


// ripcord

class ripcord_state : public circus_state
{
public:
	ripcord_state(const machine_config &mconfig, device_type type, const char *tag) :
		circus_state(mconfig, type, tag)
	{ }

	void ripcord(machine_config &config);

private:
	virtual uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect) override;
	virtual void sound_w(uint8_t data) override;
};


// defined in circus_a.cpp

DISCRETE_SOUND_EXTERN( circus_discrete );
DISCRETE_SOUND_EXTERN( robotbwl_discrete );
DISCRETE_SOUND_EXTERN( crash_discrete );

extern const char *const circus_sample_names[];
extern const char *const crash_sample_names[];
extern const char *const ripcord_sample_names[];
extern const char *const robotbwl_sample_names[];

#endif // MAME_EXIDY_CIRCUS_H
