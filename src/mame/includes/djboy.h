// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/*************************************************************************

    DJ Boy

*************************************************************************/

#include "machine/gen_latch.h"
#include "video/kan_pand.h"

#define PROT_OUTPUT_BUFFER_SIZE 8

class djboy_state : public driver_device
{
public:
	djboy_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_paletteram(*this, "paletteram"),
		m_maincpu(*this, "maincpu"),
		m_cpu1(*this, "cpu1"),
		m_cpu2(*this, "cpu2"),
		m_beast(*this, "beast"),
		m_pandora(*this, "pandora"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch")
		{ }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_paletteram;

	/* ROM banking */
	uint8_t       m_bankxor;

	/* video-related */
	tilemap_t   *m_background;
	uint8_t       m_videoreg;
	uint8_t       m_scrollx;
	uint8_t       m_scrolly;

	/* Kaneko BEAST state */
	uint8_t       m_data_to_beast;
	uint8_t       m_data_to_z80;
	uint8_t       m_beast_to_z80_full;
	uint8_t       m_z80_to_beast_full;
	uint8_t       m_beast_int0_l;
	uint8_t       m_beast_p0;
	uint8_t       m_beast_p1;
	uint8_t       m_beast_p2;
	uint8_t       m_beast_p3;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_cpu1;
	required_device<cpu_device> m_cpu2;
	required_device<cpu_device> m_beast;
	required_device<kaneko_pandora_device> m_pandora;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	void beast_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t beast_data_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t beast_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void trigger_nmi_on_cpu0(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void cpu0_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void cpu1_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void coin_count_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void trigger_nmi_on_sound_cpu2(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void cpu2_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t beast_p0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void beast_p0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t beast_p1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void beast_p1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t beast_p2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void beast_p2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t beast_p3_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void beast_p3_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void djboy_scrollx_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void djboy_scrolly_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void djboy_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void djboy_paletteram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_djboy();
	void init_djboyj();
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_djboy(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_djboy(screen_device &screen, bool state);
	void djboy_scanline(timer_device &timer, void *ptr, int32_t param);
};
