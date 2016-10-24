// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari Food Fight hardware

*************************************************************************/

#include "machine/atarigen.h"
#include "machine/x2212.h"

class foodf_state : public atarigen_state
{
public:
	foodf_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag),
			m_nvram(*this, "nvram"),
			m_playfield_tilemap(*this, "playfield"),
			m_spriteram(*this, "spriteram") { }

	required_device<x2212_device> m_nvram;
	required_device<tilemap_device> m_playfield_tilemap;

	double          m_rweights[3];
	double          m_gweights[3];
	double          m_bweights[2];
	uint8_t           m_playfield_flip;

	uint8_t           m_whichport;
	required_shared_ptr<uint16_t> m_spriteram;
	virtual void update_interrupts() override;
	void nvram_recall_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void digital_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t analog_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void analog_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void foodf_paletteram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void foodf_set_flip(int flip);
	uint8_t pot_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void get_playfield_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void machine_start_foodf();
	void machine_reset_foodf();
	void video_start_foodf();
	uint32_t screen_update_foodf(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void scanline_update_timer(timer_device &timer, void *ptr, int32_t param);
};
