// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino, Manuel Abadia
/*************************************************************************

    Combat School

*************************************************************************/

#include "machine/gen_latch.h"
#include "sound/upd7759.h"
#include "sound/msm5205.h"
#include "video/k007121.h"

class combatsc_state : public driver_device
{
public:
	combatsc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k007121_1(*this, "k007121_1"),
		m_k007121_2(*this, "k007121_2"),
		m_upd7759(*this, "upd"),
		m_msm5205(*this, "msm5205"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_track_ports(*this, {"TRACK0_Y", "TRACK0_X", "TRACK1_Y", "TRACK1_X"})
	{
	}

	/* memory pointers */
	uint8_t *    m_videoram;
	uint8_t *    m_scrollram;
	uint8_t *    m_io_ram;
	std::unique_ptr<uint8_t[]>    m_spriteram[2];

	/* video-related */
	tilemap_t *m_bg_tilemap[2];
	tilemap_t *m_textlayer;
	uint8_t m_scrollram0[0x40];
	uint8_t m_scrollram1[0x40];
	int m_priority;

	int  m_vreg;
	int  m_bank_select; /* 0x00..0x1f */
	int  m_video_circuit; /* 0 or 1 */
	uint8_t *m_page[2];

	/* misc */
	uint8_t m_pos[4];
	uint8_t m_sign[4];
	int m_prot[2];
	int m_boost;
	emu_timer *m_interleave_timer;


	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<k007121_device> m_k007121_1;
	optional_device<k007121_device> m_k007121_2;
	optional_device<upd7759_device> m_upd7759;
	optional_device<msm5205_device> m_msm5205;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	optional_ioport_array<4> m_track_ports;

	void combatsc_vreg_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void combatscb_sh_irqtrigger_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t combatscb_io_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void combatscb_priority_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void combatsc_bankselect_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void combatscb_io_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void combatscb_bankselect_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void combatsc_coin_counter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t trackball_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void protection_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t protection_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t unk_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void protection_clock_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void combatsc_sh_irqtrigger_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t combatsc_video_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void combatsc_video_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void combatsc_pf_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t combatsc_scrollram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void combatsc_scrollram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t combatsc_busy_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void combatsc_play_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void combatsc_voice_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void combatsc_portA_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void combatscb_dac_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_combatsc();
	void get_tile_info0(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info1(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_text_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info0_bootleg(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info1_bootleg(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_text_info_bootleg(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_reset() override;
	void machine_start_combatsc();
	void video_start_combatsc();
	void palette_init_combatsc(palette_device &palette);
	void machine_start_combatscb();
	void video_start_combatscb();
	void palette_init_combatscb(palette_device &palette);
	uint32_t screen_update_combatsc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_combatscb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, const uint8_t *source, int circuit, bitmap_ind8 &priority_bitmap, uint32_t pri_mask );
	void bootleg_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, const uint8_t *source, int circuit );
};
