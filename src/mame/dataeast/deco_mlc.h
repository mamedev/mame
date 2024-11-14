// license:BSD-3-Clause
// copyright-holders:Bryan McPhail

#include "machine/eepromser.h"
#include "deco146.h"
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
	void init_acchi();
	void init_avengrgs();

	void acchi(machine_config &config);
	void avengrgs(machine_config &config);
	void mlc(machine_config &config);
	void mlc_5bpp(machine_config &config);
	void mlc_6bpp(machine_config &config);
	void stadhr96(machine_config &config);

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

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

	int m_irqLevel = 0;
	u32 m_vbl_i = 0U;
	u32 m_colour_mask = 0U;
	u32 m_shadow_mask = 0U;
	u32 m_shadow_shift = 0U;

	std::unique_ptr<u16[]> m_spriteram{};
	std::unique_ptr<u16[]> m_spriteram_spare{};
	std::unique_ptr<u16[]> m_buffered_spriteram{};

	u32 mlc_440008_r();
	u32 mlc_44001c_r(offs_t offset);
	void mlc_44001c_w(u32 data);

	u32 mlc_200000_r();
	u32 mlc_200004_r();
	u32 mlc_200070_r();
	u32 mlc_20007c_r();
	u32 mlc_scanline_r();
	void irq_ram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	u32 avengrgs_speedup_r();
	void eeprom_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	u32 spriteram_r(offs_t offset, uint32_t mem_mask = ~0);
	void spriteram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	u16 sh96_protection_region_0_146_r(offs_t offset);
	void sh96_protection_region_0_146_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void screen_vblank_mlc(int state);
	TIMER_DEVICE_CALLBACK_MEMBER(interrupt_gen);
	void draw_sprites( const rectangle &cliprect, int scanline, u32* dest, u8* pri);
	void drawgfxzoomline(u32* dest, u8* pri,const rectangle &clip,gfx_element *gfx,
		u32 code1,u32 code2, u32 color,int flipx,int sx,
		int transparent_color,int use8bpp,
		int scalex, int srcline, int shadowMode);
	void descramble_sound();

	void avengrgs_map(address_map &map) ATTR_COLD;
	void decomlc_146_map(address_map &map) ATTR_COLD;
	void decomlc_no146_map(address_map &map) ATTR_COLD;
};
