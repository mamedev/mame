// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari "Round" hardware

*************************************************************************/

#include "machine/atarigen.h"
#include "audio/atarijsa.h"
#include "video/atarimo.h"

class offtwall_state : public atarigen_state
{
public:
	offtwall_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag),
			m_jsa(*this, "jsa"),
			m_vad(*this, "vad"),
			m_mainram(*this, "mainram"),
			m_bankrom_base(*this, "bankrom_base") { }

	required_device<atari_jsa_iii_device> m_jsa;
	required_device<atari_vad_device> m_vad;
	required_shared_ptr<uint16_t> m_mainram;

	uint16_t *m_bankswitch_base;
	required_shared_ptr<uint16_t> m_bankrom_base;
	uint32_t m_bank_offset;

	uint16_t *m_spritecache_count;
	uint16_t *m_unknown_verify_base;
	virtual void update_interrupts() override;
	void io_latch_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t bankswitch_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t bankrom_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t spritecache_count_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t unknown_verify_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void init_offtwall();
	void init_offtwalc();
	void get_playfield_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void machine_start_offtwall();
	void machine_reset_offtwall();
	void video_start_offtwall();
	uint32_t screen_update_offtwall(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	static const atari_motion_objects_config s_mob_config;
};
