// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino

#include "video/bufsprite.h"
#include "sound/msm5205.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/2203intf.h"
#include "sound/msm5205.h"
#include "cpu/m6805/m6805.h"
#include "video/tigeroad_spr.h"

class tigeroad_state : public driver_device
{
public:
	tigeroad_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_spriteram(*this, "spriteram") ,
		m_videoram(*this, "videoram"),
		m_ram16(*this, "ram16"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_msm(*this, "msm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_mcu(*this, "mcu"),
		m_spritegen(*this, "spritegen"),
		m_soundlatch(*this, "soundlatch"),
		m_has_coinlock(1)
	{ }

	required_device<buffered_spriteram16_device> m_spriteram;
	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_ram16;
	int m_bgcharbank;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	void f1dream_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void tigeroad_soundcmd_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void tigeroad_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void tigeroad_videoctrl_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void tigeroad_scroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void msm5205_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_f1dream();
	void init_pushman();
	void init_bballs();
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	tilemap_memory_index tigeroad_tilemap_scan(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	virtual void video_start() override;
	uint32_t screen_update_tigeroad(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void f1dream_protection_w(address_space &space);
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<msm5205_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<cpu_device> m_mcu;
	required_device<tigeroad_spr_device> m_spritegen;
	required_device<generic_latch_8_device> m_soundlatch;

	uint16_t     m_control[2];

	/* misc */
	uint8_t      m_shared_ram[8];
	uint16_t     m_latch;
	uint16_t     m_new_latch;
	int m_has_coinlock;

	/* protection handling */
	uint16_t pushman_68705_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void pushman_68705_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t bballs_68705_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void bballs_68705_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t pushman_68000_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pushman_68000_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void machine_reset_pushman();
	void machine_reset_bballs();

	virtual void machine_start() override;

};
