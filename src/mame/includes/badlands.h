// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari Bad Lands hardware

*************************************************************************/

#include "machine/atarigen.h"
#include "video/atarimo.h"

class badlands_state : public atarigen_state
{
public:
	badlands_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag),
			m_audiocpu(*this, "audiocpu"),
			m_soundcomm(*this, "soundcomm"),
			m_playfield_tilemap(*this, "playfield"),
			m_mob(*this, "mob") { }

	optional_device<cpu_device> m_audiocpu;
	optional_device<atari_sound_comm_device> m_soundcomm;

	required_device<tilemap_device> m_playfield_tilemap;
	required_device<atari_motion_objects_device> m_mob;

	uint8_t           m_pedal_value[2];
	uint8_t           m_playfield_tile_bank;

	virtual void update_interrupts() override;
	virtual void scanline_update(screen_device &screen, int scanline) override;
	uint16_t sound_busy_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t pedal_0_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t pedal_1_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint8_t audio_io_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void audio_io_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t badlandsb_unk_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void init_badlands();
	void get_playfield_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void machine_start_badlands();
	void machine_reset_badlands();
	void video_start_badlands();
	void machine_reset_badlandsb();
	uint32_t screen_update_badlands(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_int(device_t &device);
	void badlands_pf_bank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	static const atari_motion_objects_config s_mob_config;
};
