// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
#include "machine/eepromser.h"
#include "machine/deco146.h"
#include "sound/ymz280b.h"


class deco_mlc_state : public driver_device
{
public:
	deco_mlc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_deco146(*this, "ioprot"),
		m_mlc_ram(*this, "mlc_ram"),
		m_irq_ram(*this, "irq_ram"),
		m_mlc_clip_ram(*this, "mlc_clip_ram"),
		m_mlc_vram(*this, "mlc_vram"),
		m_maincpu(*this, "maincpu"),
		m_eeprom(*this, "eeprom"),
		m_ymz(*this, "ymz"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_generic_paletteram_32(*this, "paletteram"),
		m_gfx2(*this,"gfx2")
		{ }

	optional_device<deco146_device> m_deco146;
	required_shared_ptr<uint32_t> m_mlc_ram;
	required_shared_ptr<uint32_t> m_irq_ram;
	required_shared_ptr<uint32_t> m_mlc_clip_ram;
	required_shared_ptr<uint32_t> m_mlc_vram;
	timer_device *m_raster_irq_timer;
	int m_mainCpuIsArm;
	uint32_t m_mlc_raster_table_1[4*256];
	uint32_t m_mlc_raster_table_2[4*256];
	uint32_t m_mlc_raster_table_3[4*256];
	uint32_t m_vbl_i;
	int m_lastScanline[9];
	uint32_t m_colour_mask;

	std::unique_ptr<uint16_t[]> m_mlc_spriteram;
	std::unique_ptr<uint16_t[]> m_mlc_spriteram_spare;
	std::unique_ptr<uint16_t[]> m_mlc_buffered_spriteram;
	uint32_t test2_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t mlc_440008_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t mlc_44001c_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void mlc_44001c_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

	void avengrs_palette_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t mlc_200000_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t mlc_200004_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t mlc_200070_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t mlc_20007c_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t mlc_scanline_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void mlc_irq_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t mlc_vram_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t avengrgs_speedup_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void avengrs_eprom_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t mlc_spriteram_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void mlc_spriteram_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);



	void init_mlc();
	void init_avengrgs();
	void machine_reset_mlc();
	void video_start_mlc();
	uint32_t screen_update_mlc(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void screen_eof_mlc(screen_device &screen, bool state);
	void interrupt_gen(timer_device &timer, void *ptr, int32_t param);
	void draw_sprites( const rectangle &cliprect, int scanline, uint32_t* dest);
	void descramble_sound(  );
	required_device<cpu_device> m_maincpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<ymz280b_device> m_ymz;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint32_t> m_generic_paletteram_32;
	required_region_ptr<uint8_t> m_gfx2;

	uint16_t sh96_protection_region_0_146_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void sh96_protection_region_0_146_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
};
