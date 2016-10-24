// license:BSD-3-Clause
// copyright-holders:Jarek Parchanski, Nicola Salmoria, Mirko Buffoni

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/i8255.h"
#include "machine/segacrp2_device.h"

class system1_state : public driver_device
{
public:
	system1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_ppi8255(*this, "ppi8255"),
		m_ram(*this, "ram"),
		m_spriteram(*this, "spriteram"),
		m_nob_mcu_latch(*this, "nob_mcu_latch"),
		m_nob_mcu_status(*this, "nob_mcu_status"),
		m_paletteram(*this, "palette"),
		m_videomode_custom(nullptr),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_mcu(*this, "mcu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_decrypted_opcodes(*this, "decrypted_opcodes"),
		m_maincpu_region(*this, "maincpu"),
		m_color_prom(*this, "palette"),
		m_bank1(*this, "bank1"),
		m_bank0d(*this, "bank0d"),
		m_bank1d(*this, "bank1d"),
		m_banked_decrypted_opcodes(nullptr)
		{ }

	optional_device<i8255_device>  m_ppi8255;
	required_shared_ptr<uint8_t> m_ram;
	required_shared_ptr<uint8_t> m_spriteram;
	optional_shared_ptr<uint8_t> m_nob_mcu_latch;
	optional_shared_ptr<uint8_t> m_nob_mcu_status;
	required_shared_ptr<uint8_t> m_paletteram;

	std::unique_ptr<uint8_t[]> m_videoram;
	void (system1_state::*m_videomode_custom)(uint8_t data, uint8_t prevdata);
	uint8_t m_mute_xor;
	uint8_t m_dakkochn_mux_data;
	uint8_t m_videomode_prev;
	uint8_t m_mcu_control;
	uint8_t m_nob_maincpu_latch;
	int m_nobb_inport23_step;
	std::unique_ptr<uint8_t[]> m_mix_collide;
	uint8_t m_mix_collide_summary;
	std::unique_ptr<uint8_t[]> m_sprite_collide;
	uint8_t m_sprite_collide_summary;
	bitmap_ind16 m_sprite_bitmap;
	uint8_t m_video_mode;
	uint8_t m_videoram_bank;
	tilemap_t *m_tilemap_page[8];
	uint8_t m_tilemap_pages;

	void videomode_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t sound_data_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void soundport_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mcu_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mcu_io_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mcu_io_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void nob_mcu_control_p2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t nob_maincpu_latch_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void nob_maincpu_latch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t nob_mcu_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t nobb_inport1c_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t nobb_inport22_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t nobb_inport23_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void nobb_outport24_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t nob_start_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t shtngmst_gunx_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void system1_videomode_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t system1_mixer_collision_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void system1_mixer_collision_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void system1_mixer_collision_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t system1_sprite_collision_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void system1_sprite_collision_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void system1_sprite_collision_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t system1_videoram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void system1_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void system1_paletteram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	ioport_value dakkochn_mux_data_r(ioport_field &field, void *param);
	ioport_value dakkochn_mux_status_r(ioport_field &field, void *param);
	void sound_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void init_bank00();
	void init_bank0c();
	void init_bank44();

	void init_nobb();
	void init_dakkochn();
	void init_bootleg();
	void init_shtngmst();
	void init_blockgal();
	void init_nob();
	void init_myherok();
	void init_ufosensi();
	void init_wbml();
	void init_bootsys2();
	void init_bootsys2d();
	void init_choplift();

	void tile_get_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void machine_start_system2();
	void video_start_system2();
	void machine_start_myherok();
	uint32_t screen_update_system1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_system2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_system2_rowscroll(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void mcu_irq_assert(device_t &device);
	void soundirq_gen(timer_device &timer, void *ptr, int32_t param);
	void mcu_t0_callback(timer_device &timer, void *ptr, int32_t param);
	void system1_videoram_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void video_start_common(int pagecount);
	inline void videoram_wait_states(cpu_device *cpu);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int xoffset);
	void video_update_common(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind16 &fgpixmap, bitmap_ind16 **bgpixmaps, const int *bgrowscroll, int bgyscroll, int spritexoffs);
	void bank44_custom_w(uint8_t data, uint8_t prevdata);
	void bank0c_custom_w(uint8_t data, uint8_t prevdata);
	void dakkochn_custom_w(uint8_t data, uint8_t prevdata);
	required_device<z80_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	optional_device<cpu_device> m_mcu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	optional_shared_ptr<uint8_t> m_decrypted_opcodes;
	required_memory_region m_maincpu_region;
	optional_region_ptr<uint8_t> m_color_prom;
	required_memory_bank m_bank1;
	optional_memory_bank m_bank0d;
	optional_memory_bank m_bank1d;

	std::unique_ptr<uint8_t[]> m_banked_decrypted_opcodes;
};
