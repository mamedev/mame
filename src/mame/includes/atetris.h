// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari, Aaron Giles
/*************************************************************************

    Atari Tetris hardware

*************************************************************************/

#include "includes/slapstic.h"

class atetris_state : public driver_device
{
public:
	atetris_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_slapstic_device(*this, "slapstic"),
		m_nvram(*this, "nvram") ,
		m_videoram(*this, "videoram")
		{ }

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	optional_device<atari_slapstic_device> m_slapstic_device;

	required_shared_ptr<uint8_t>  m_nvram;
	required_shared_ptr<uint8_t> m_videoram;

	uint8_t *m_slapstic_source;
	uint8_t *m_slapstic_base;
	uint8_t m_current_bank;
	uint8_t m_nvram_write_enable;
	emu_timer *m_interrupt_timer;
	tilemap_t *m_bg_tilemap;

	void irq_ack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t slapstic_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void coincount_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void nvram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void nvram_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_atetris();
	void get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void interrupt_gen(void *ptr, int32_t param);
	void reset_bank();
};
