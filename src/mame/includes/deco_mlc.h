// license:BSD-3-Clause
// copyright-holders:Bryan McPhail

#include "machine/eepromser.h"
#include "machine/deco146.h"
#include "machine/timer.h"
#include "sound/ymz280b.h"
#include "emupal.h"
#include "screen.h"


class deco_mlc_state : public driver_device
{
public:
	deco_mlc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_eeprom(*this, "eeprom"),
		m_ymz(*this, "ymz"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_deco146(*this, "ioprot"),
		m_raster_irq_timer(*this, "int_timer"),
		m_mainram(*this, "mainram"),
		m_irq_ram(*this, "irq_ram"),
		m_clip_ram(*this, "clip_ram"),
		m_vram(*this, "vram"),
		m_gfx2(*this,"gfx2")
		{ }

	void init_mlc();
	void init_avengrgs();

	void mlc(machine_config &config);
	void mlc_6bpp(machine_config &config);
	void avengrgs(machine_config &config);
	void mlc_5bpp(machine_config &config);

protected:
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<ymz280b_device> m_ymz;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_device<deco146_device> m_deco146;
	required_device<timer_device> m_raster_irq_timer;

	required_shared_ptr<u32> m_mainram;
	required_shared_ptr<u32> m_irq_ram;
	required_shared_ptr<u32> m_clip_ram;
	required_shared_ptr<u32> m_vram;

	required_region_ptr<u8> m_gfx2;

	int m_irqLevel;
	u32 m_mlc_raster_table_1[4*256];
	u32 m_mlc_raster_table_2[4*256];
	u32 m_mlc_raster_table_3[4*256];
	u32 m_vbl_i;
	int m_lastScanline[9];
	u32 m_colour_mask;
	u32 m_shadow_mask;
	u32 m_shadow_shift;

	std::unique_ptr<u16[]> m_spriteram;
	std::unique_ptr<u16[]> m_spriteram_spare;
	std::unique_ptr<u16[]> m_buffered_spriteram;

	DECLARE_READ32_MEMBER(mlc_440008_r);
	DECLARE_READ32_MEMBER(mlc_44001c_r);
	DECLARE_WRITE32_MEMBER(mlc_44001c_w);

	DECLARE_READ32_MEMBER(mlc_200000_r);
	DECLARE_READ32_MEMBER(mlc_200004_r);
	DECLARE_READ32_MEMBER(mlc_200070_r);
	DECLARE_READ32_MEMBER(mlc_20007c_r);
	DECLARE_READ32_MEMBER(mlc_scanline_r);
	DECLARE_WRITE32_MEMBER(irq_ram_w);
	DECLARE_READ32_MEMBER(avengrgs_speedup_r);
	DECLARE_WRITE32_MEMBER(eeprom_w);
	DECLARE_READ32_MEMBER(spriteram_r);
	DECLARE_WRITE32_MEMBER(spriteram_w);

	DECLARE_READ16_MEMBER( sh96_protection_region_0_146_r );
	DECLARE_WRITE16_MEMBER( sh96_protection_region_0_146_w );

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank_mlc);
	TIMER_DEVICE_CALLBACK_MEMBER(interrupt_gen);
	void draw_sprites( const rectangle &cliprect, int scanline, u32* dest, u8* pri);
	void drawgfxzoomline(u32* dest, u8* pri,const rectangle &clip,gfx_element *gfx,
		u32 code1,u32 code2, u32 color,int flipx,int sx,
		int transparent_color,int use8bpp,
		int scalex, int srcline, int shadowMode);
	void descramble_sound();

	void avengrgs_map(address_map &map);
	void decomlc_map(address_map &map);
};
