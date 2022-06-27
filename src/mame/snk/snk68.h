// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Acho A. Tang, Nicola Salmoria

#include "machine/gen_latch.h"
#include "sound/upd7759.h"
#include "video/snk68_spr.h"
#include "video/alpha68k_palette.h"
#include "screen.h"
#include "tilemap.h"

class snk68_state : public driver_device
{
public:
	snk68_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_upd7759(*this, "upd"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_sprites(*this, "sprites"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_fg_videoram(*this, "fg_videoram"),
		m_spriteram(*this, "spriteram"),
		m_p1_io(*this, "P1"),
		m_p2_io(*this, "P2"),
		m_system_io(*this, "SYSTEM")
	{
	}

	void streetsm(machine_config &config);
	void pow(machine_config &config);
	void powb(machine_config &config);

	void init_powb();

protected:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	optional_device<upd7759_device> m_upd7759;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<snk68_spr_device> m_sprites;
	required_device<alpha68k_palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint16_t> m_fg_videoram;
	required_shared_ptr<uint16_t> m_spriteram;

	optional_ioport m_p1_io;
	optional_ioport m_p2_io;
	optional_ioport m_system_io;

	bool m_sprite_flip_axis;
	tilemap_t *m_fg_tilemap;

	// common
	void sound_w(uint8_t data);
	void D7759_write_port_0_w(uint8_t data);

	virtual void video_start() override;
	void common_video_start();

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void tile_callback_notpow(int &tile, int& fx, int& fy, int& region);

	void sound_io_map(address_map &map);
	void sound_map(address_map &map);

private:
	uint32_t m_fg_tile_offset;

	// pow and streetsm
	uint16_t fg_videoram_r(offs_t offset);
	void fg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void flipscreen_w(uint8_t data);

	TILE_GET_INFO_MEMBER(get_tile_info);

	void tile_callback_pow(int &tile, int& fx, int& fy, int& region);

	void pow_map(address_map &map);
	void powb_sound_io_map(address_map &map);
};

class searchar_state : public snk68_state
{
public:
	searchar_state(const machine_config &mconfig, device_type type, const char *tag) :
		snk68_state(mconfig, type, tag),
		m_rotary_io(*this, "ROT%u", 1U)
	{
	}

	void searchar(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void video_start() override;

private:
	optional_ioport_array<2> m_rotary_io;

	uint8_t m_invert_controls = 0;

	// searchar and ikari3
	void fg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void flipscreen_w(uint8_t data);
	uint16_t rotary_1_r();
	uint16_t rotary_2_r();
	uint16_t rotary_lsb_r();

	TILE_GET_INFO_MEMBER(get_tile_info);

	void searchar_map(address_map &map);
};
