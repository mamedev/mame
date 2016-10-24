// license:BSD-3-Clause
// copyright-holders:Luca Elia,Olivier Galibert,Paul Priest
/*************************************************************************

    Psikyo Games

*************************************************************************/
#include "sound/okim6295.h"

class psikyo_state : public driver_device
{
public:
	psikyo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_vram_0(*this, "vram_0"),
		m_vram_1(*this, "vram_1"),
		m_vregs(*this, "vregs"),
		m_bootleg_spritebuffer(*this, "boot_spritebuf"),
		m_audiocpu(*this, "audiocpu"),
		m_maincpu(*this, "maincpu"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<uint32_t> m_spriteram;
	required_shared_ptr<uint32_t> m_vram_0;
	required_shared_ptr<uint32_t> m_vram_1;
	required_shared_ptr<uint32_t> m_vregs;
	optional_shared_ptr<uint32_t> m_bootleg_spritebuffer;
	std::unique_ptr<uint32_t[]>       m_spritebuf1;
	std::unique_ptr<uint32_t[]>       m_spritebuf2;

	/* video-related */
	tilemap_t        *m_tilemap_0_size0;
	tilemap_t        *m_tilemap_0_size1;
	tilemap_t        *m_tilemap_0_size2;
	tilemap_t        *m_tilemap_0_size3;
	tilemap_t        *m_tilemap_1_size0;
	tilemap_t        *m_tilemap_1_size1;
	tilemap_t        *m_tilemap_1_size2;
	tilemap_t        *m_tilemap_1_size3;
	int            m_tilemap_0_bank;
	int            m_tilemap_1_bank;
	int            m_ka302c_banking;

	/* misc */
	uint8_t          m_soundlatch;
	int            m_z80_nmi;
	int            m_mcu_status;

	/* devices */
	optional_device<cpu_device> m_audiocpu;

	/* game-specific */
	// 1945 MCU
	uint8_t          m_s1945_mcu_direction;
	uint8_t          m_s1945_mcu_latch1;
	uint8_t          m_s1945_mcu_latch2;
	uint8_t          m_s1945_mcu_inlatch;
	uint8_t          m_s1945_mcu_index;
	uint8_t          m_s1945_mcu_latching;
	uint8_t          m_s1945_mcu_mode;
	uint8_t          m_s1945_mcu_control;
	uint8_t          m_s1945_mcu_bctrl;
	const uint8_t    *m_s1945_mcu_table;
	uint32_t sngkace_input_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t gunbird_input_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void psikyo_soundlatch_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void s1945_soundlatch_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void s1945_mcu_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t s1945_mcu_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t s1945_input_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t s1945bl_oki_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void s1945bl_oki_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint8_t psikyo_soundlatch_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void psikyo_clear_nmi_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sngkace_sound_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gunbird_sound_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void psikyo_vram_0_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void psikyo_vram_1_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	ioport_value z80_nmi_r(ioport_field &field, void *param);
	ioport_value mcu_status_r(ioport_field &field, void *param);
	void init_s1945a();
	void init_s1945j();
	void init_sngkace();
	void init_s1945();
	void init_s1945bl();
	void init_tengai();
	void init_s1945jn();
	void init_gunbird();
	void get_tile_info_0(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info_1(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	void video_start_sngkace();
	void video_start_psikyo();
	uint32_t screen_update_psikyo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_psikyo_bootleg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_psikyo(screen_device &screen, bool state);
	void psikyo_soundlatch_callback(void *ptr, int32_t param);
	void psikyo_switch_banks( int tmap, int bank );
	void draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int trans_pen );
	void draw_sprites_bootleg( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int trans_pen );
	int tilemap_width( int size );
	void s1945_mcu_init(  );
	required_device<cpu_device> m_maincpu;
	optional_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
};
