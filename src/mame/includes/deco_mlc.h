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
		m_generic_paletteram_32(*this, "paletteram")
		{ }

	optional_device<deco146_device> m_deco146;
	required_shared_ptr<UINT32> m_mlc_ram;
	required_shared_ptr<UINT32> m_irq_ram;
	required_shared_ptr<UINT32> m_mlc_clip_ram;
	required_shared_ptr<UINT32> m_mlc_vram;
	timer_device *m_raster_irq_timer;
	int m_mainCpuIsArm;
	UINT32 m_mlc_raster_table_1[4*256];
	UINT32 m_mlc_raster_table_2[4*256];
	UINT32 m_mlc_raster_table_3[4*256];
	UINT32 m_vbl_i;
	int m_lastScanline[9];
	UINT32 m_colour_mask;

	std::unique_ptr<UINT16[]> m_mlc_spriteram;
	std::unique_ptr<UINT16[]> m_mlc_spriteram_spare;
	std::unique_ptr<UINT16[]> m_mlc_buffered_spriteram;
	DECLARE_READ32_MEMBER(test2_r);
	DECLARE_READ32_MEMBER(mlc_440008_r);
	DECLARE_READ32_MEMBER(mlc_44001c_r);
	DECLARE_WRITE32_MEMBER(mlc_44001c_w);

	DECLARE_WRITE32_MEMBER(avengrs_palette_w);
	DECLARE_READ32_MEMBER(mlc_200000_r);
	DECLARE_READ32_MEMBER(mlc_200004_r);
	DECLARE_READ32_MEMBER(mlc_200070_r);
	DECLARE_READ32_MEMBER(mlc_20007c_r);
	DECLARE_READ32_MEMBER(mlc_scanline_r);
	DECLARE_WRITE32_MEMBER(mlc_irq_w);
	DECLARE_READ32_MEMBER(mlc_vram_r);
	DECLARE_READ32_MEMBER(avengrgs_speedup_r);
	DECLARE_WRITE32_MEMBER(avengrs_eprom_w);
	DECLARE_READ32_MEMBER(mlc_spriteram_r);
	DECLARE_WRITE32_MEMBER(mlc_spriteram_w);



	DECLARE_DRIVER_INIT(mlc);
	DECLARE_DRIVER_INIT(avengrgs);
	DECLARE_MACHINE_RESET(mlc);
	DECLARE_VIDEO_START(mlc);
	UINT32 screen_update_mlc(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void screen_eof_mlc(screen_device &screen, bool state);
	TIMER_DEVICE_CALLBACK_MEMBER(interrupt_gen);
	void draw_sprites( const rectangle &cliprect, int scanline, UINT32* dest);
	void descramble_sound(  );
	required_device<cpu_device> m_maincpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<ymz280b_device> m_ymz;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_shared_ptr<UINT32> m_generic_paletteram_32;

	DECLARE_READ16_MEMBER( sh96_protection_region_0_146_r );
	DECLARE_WRITE16_MEMBER( sh96_protection_region_0_146_w );
};
