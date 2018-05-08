// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

                            -= Jaleco Mega System 1 =-

                    driver by   Luca Elia (l.elia@tin.it)

***************************************************************************/

#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "sound/okim6295.h"
#include "video/ms1_tmap.h"
#include "screen.h"


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

	DECLARE_WRITE_LINE_MEMBER(sound_irq);
	DECLARE_READ16_MEMBER(ip_select_r);
	DECLARE_WRITE16_MEMBER(ip_select_w);
	DECLARE_READ16_MEMBER(protection_peekaboo_r);
	DECLARE_WRITE16_MEMBER(protection_peekaboo_w);
	DECLARE_READ16_MEMBER(megasys1A_mcu_hs_r);
	DECLARE_WRITE16_MEMBER(megasys1A_mcu_hs_w);
	DECLARE_READ16_MEMBER(iganinju_mcu_hs_r);
	DECLARE_WRITE16_MEMBER(iganinju_mcu_hs_w);
	DECLARE_READ16_MEMBER(soldamj_spriteram16_r);
	DECLARE_WRITE16_MEMBER(soldamj_spriteram16_w);
	DECLARE_READ16_MEMBER(stdragon_mcu_hs_r);
	DECLARE_WRITE16_MEMBER(stdragon_mcu_hs_w);
	DECLARE_WRITE16_MEMBER(active_layers_w);
	DECLARE_WRITE16_MEMBER(sprite_bank_w);
	DECLARE_READ16_MEMBER(sprite_flag_r);
	DECLARE_WRITE16_MEMBER(sprite_flag_w);
	DECLARE_WRITE16_MEMBER(screen_flag_w);
	DECLARE_WRITE16_MEMBER(soundlatch_w);
	DECLARE_WRITE16_MEMBER(soundlatch_z_w);
	DECLARE_WRITE16_MEMBER(soundlatch_c_w);
	DECLARE_WRITE16_MEMBER(monkelf_scroll0_w);
	DECLARE_WRITE16_MEMBER(monkelf_scroll1_w);
	void megasys1_set_vreg_flag(int which, int data);
	DECLARE_READ8_MEMBER(oki_status_1_r);
	DECLARE_READ8_MEMBER(oki_status_2_r);
	DECLARE_WRITE16_MEMBER(okim6295_both_1_w);
	DECLARE_WRITE16_MEMBER(okim6295_both_2_w);
	DECLARE_WRITE16_MEMBER(ram_w);

	DECLARE_DRIVER_INIT(64street);
	DECLARE_DRIVER_INIT(chimerab);
	DECLARE_DRIVER_INIT(peekaboo);
	DECLARE_DRIVER_INIT(soldam);
	DECLARE_DRIVER_INIT(astyanax);
	DECLARE_DRIVER_INIT(stdragon);
	DECLARE_DRIVER_INIT(hayaosi1);
	DECLARE_DRIVER_INIT(soldamj);
	DECLARE_DRIVER_INIT(phantasm);
	DECLARE_DRIVER_INIT(jitsupro);
	DECLARE_DRIVER_INIT(iganinju);
	DECLARE_DRIVER_INIT(cybattlr);
	DECLARE_DRIVER_INIT(rodlandj);
	DECLARE_DRIVER_INIT(rittam);
	DECLARE_DRIVER_INIT(rodlandjb);
	DECLARE_DRIVER_INIT(avspirit);
	DECLARE_DRIVER_INIT(monkelf);
	DECLARE_DRIVER_INIT(edf);
	DECLARE_DRIVER_INIT(edfp);
	DECLARE_DRIVER_INIT(bigstrik);
	DECLARE_DRIVER_INIT(rodland);
	DECLARE_DRIVER_INIT(edfbl);
	DECLARE_DRIVER_INIT(stdragona);
	DECLARE_DRIVER_INIT(stdragonb);
	DECLARE_DRIVER_INIT(systemz);
	DECLARE_MACHINE_RESET(megasys1);
	DECLARE_VIDEO_START(megasys1);
	DECLARE_PALETTE_INIT(megasys1);
	DECLARE_MACHINE_RESET(megasys1_hachoo);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank);
	INTERRUPT_GEN_MEMBER(megasys1D_irq);
	TIMER_DEVICE_CALLBACK_MEMBER(megasys1A_scanline);
	TIMER_DEVICE_CALLBACK_MEMBER(megasys1A_iganinju_scanline);
	TIMER_DEVICE_CALLBACK_MEMBER(megasys1B_scanline);

	void priority_create();
	void mix_sprite_bitmap(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void partial_clear_sprite_bitmap(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t param);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect);
	inline void draw_16x16_priority_sprite(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect, int32_t code, int32_t color, int32_t sx, int32_t sy, int32_t flipx, int32_t flipy, uint8_t mosaic, uint8_t mosaicsol, int32_t priority);
	void rodland_gfx_unmangle(const char *region);
	void jitsupro_gfx_unmangle(const char *region);
	void stdragona_gfx_unmangle(const char *region);
	void system_A_soldam(machine_config &config);
	void system_B_monkelf(machine_config &config);
	void system_A_iganinju(machine_config &config);
	void system_A_hachoo(machine_config &config);
	void kickoffb(machine_config &config);
	void system_D(machine_config &config);
	void system_C(machine_config &config);
	void system_Bbl(machine_config &config);
	void system_A(machine_config &config);
	void system_B(machine_config &config);
	void system_B_hayaosi1(machine_config &config);
	void system_Z(machine_config &config);
	void kickoffb_sound_map(address_map &map);
	void megasys1A_map(address_map &map);
	void megasys1A_sound_map(address_map &map);
	void megasys1B_edfbl_map(address_map &map);
	void megasys1B_map(address_map &map);
	void megasys1B_monkelf_map(address_map &map);
	void megasys1B_sound_map(address_map &map);
	void megasys1C_map(address_map &map);
	void megasys1D_map(address_map &map);
	void megasys1D_oki_map(address_map &map);
	void megasys1Z_map(address_map &map);
	void z80_sound_io_map(address_map &map);
	void z80_sound_map(address_map &map);
};
