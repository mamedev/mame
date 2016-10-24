// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

                            -= Jaleco Mega System 1 =-

                    driver by   Luca Elia (l.elia@tin.it)

***************************************************************************/

#include "machine/gen_latch.h"
#include "sound/okim6295.h"
#include "video/ms1_tmap.h"


class megasys1_state : public driver_device
{
public:
	megasys1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_objectram(*this, "objectram"),
		m_tmap(*this, "scroll%u", 0),
		m_ram(*this, "ram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_oki1(*this, "oki1"),
		m_oki2(*this, "oki2"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen"),
		m_soundlatch(*this, "soundlatch"),
		m_soundlatch2(*this, "soundlatch2"),
		m_soundlatch_z(*this, "soundlatch_z"),
		m_rom_maincpu(*this, "maincpu"),
		m_io_system(*this, "SYSTEM"),
		m_io_p1(*this, "P1"),
		m_io_p2(*this, "P2"),
		m_io_dsw(*this, "DSW"),
		m_io_dsw1(*this, "DSW1"),
		m_io_dsw2(*this, "DSW2")
		{ }

	required_shared_ptr<uint16_t> m_objectram;
	optional_device_array<megasys1_tilemap_device, 3> m_tmap;
	required_shared_ptr<uint16_t> m_ram;
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<okim6295_device> m_oki1;
	optional_device<okim6295_device> m_oki2;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	optional_device<generic_latch_16_device> m_soundlatch;
	optional_device<generic_latch_16_device> m_soundlatch2;
	optional_device<generic_latch_8_device> m_soundlatch_z;
	required_region_ptr<uint16_t> m_rom_maincpu;
	required_ioport m_io_system;
	required_ioport m_io_p1;
	required_ioport m_io_p2;
	optional_ioport m_io_dsw;
	optional_ioport m_io_dsw1;
	optional_ioport m_io_dsw2;

	// configuration
	uint16_t m_ip_select_values[7]; // System B and C
	int m_hardware_type_z; // System Z
	int m_layers_order[16];
	uint8_t m_ignore_oki_status;

	// all
	bitmap_ind16 m_sprite_buffer_bitmap;
	uint16_t m_screen_flag;
	std::unique_ptr<uint16_t[]> m_buffer_objectram;
	std::unique_ptr<uint16_t[]> m_buffer2_objectram;
	std::unique_ptr<uint16_t[]> m_buffer_spriteram16;
	std::unique_ptr<uint16_t[]> m_buffer2_spriteram16;

	// all but System Z
	uint16_t m_active_layers;
	uint16_t m_sprite_flag;

	// System B and C
	uint16_t m_ip_latched;

	 // System C
	uint16_t m_sprite_bank;

	// System A only
	int m_mcu_hs;
	uint16_t m_mcu_hs_ram[0x10];

	// peekaboo
	uint16_t m_protection_val;

	// soldam
	uint16_t *m_spriteram;

	void sound_irq(int state);
	uint16_t ip_select_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void ip_select_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t protection_peekaboo_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void protection_peekaboo_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t megasys1A_mcu_hs_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void megasys1A_mcu_hs_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t iganinju_mcu_hs_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void iganinju_mcu_hs_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t soldamj_spriteram16_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void soldamj_spriteram16_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t stdragon_mcu_hs_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void stdragon_mcu_hs_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void active_layers_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sprite_bank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t sprite_flag_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void sprite_flag_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void screen_flag_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void soundlatch_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void soundlatch_z_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void soundlatch_c_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void monkelf_scroll0_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void monkelf_scroll1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void megasys1_set_vreg_flag(int which, int data);
	uint8_t oki_status_1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t oki_status_2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void okim6295_both_1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void okim6295_both_2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void ram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	void init_64street();
	void init_chimerab();
	void init_peekaboo();
	void init_soldam();
	void init_astyanax();
	void init_stdragon();
	void init_hayaosi1();
	void init_soldamj();
	void init_phantasm();
	void init_jitsupro();
	void init_iganinju();
	void init_cybattlr();
	void init_rodlandj();
	void init_rittam();
	void init_rodlandjb();
	void init_avspirit();
	void init_monkelf();
	void init_edf();
	void init_edfp();
	void init_bigstrik();
	void init_rodland();
	void init_edfbl();
	void init_stdragona();
	void init_stdragonb();
	void init_systemz();
	void machine_reset_megasys1();
	void video_start_megasys1();
	void palette_init_megasys1(palette_device &palette);
	void machine_reset_megasys1_hachoo();

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof(screen_device &screen, bool state);
	void megasys1D_irq(device_t &device);
	void megasys1A_scanline(timer_device &timer, void *ptr, int32_t param);
	void megasys1A_iganinju_scanline(timer_device &timer, void *ptr, int32_t param);
	void megasys1B_scanline(timer_device &timer, void *ptr, int32_t param);

	void priority_create();
	void mix_sprite_bitmap(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void partial_clear_sprite_bitmap(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t param);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect);
	inline void draw_16x16_priority_sprite(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect, int32_t code, int32_t color, int32_t sx, int32_t sy, int32_t flipx, int32_t flipy, uint8_t mosaic, uint8_t mosaicsol, int32_t priority);
	void rodland_gfx_unmangle(const char *region);
	void jitsupro_gfx_unmangle(const char *region);
	void stdragona_gfx_unmangle(const char *region);
};
