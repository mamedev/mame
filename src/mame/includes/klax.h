// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari Klax hardware

*************************************************************************/

#include "machine/atarigen.h"
#include "video/atarimo.h"

class klax_state : public atarigen_state
{
public:
	klax_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag)
		, m_playfield_tilemap(*this, "playfield")
		, m_mob(*this, "mob")
		, m_p1(*this, "P1")
	{ }

	void machine_start_klax();
	void machine_reset_klax();

	virtual void scanline_update(screen_device &screen, int scanline) override;

	virtual void update_interrupts() override;
	void interrupt_ack_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	void klax_latch_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	void video_start_klax();
	void get_playfield_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	uint32_t screen_update_klax(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	static const atari_motion_objects_config s_mob_config;

private:
	required_device<tilemap_device> m_playfield_tilemap;
	required_device<atari_motion_objects_device> m_mob;

	required_ioport m_p1;
};
