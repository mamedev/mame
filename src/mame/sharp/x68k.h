// license:BSD-3-Clause
// copyright-holders:Barry Rodewald,Carl
/*****************************************************************************
 *
 * includes/x68k.h
 *
 * Sharp X68000
 *
 ****************************************************************************/

#ifndef MAME_SHARP_X68K_H
#define MAME_SHARP_X68K_H

#pragma once

#include "x68k_crtc.h"

#include "bus/msx/ctrl/ctrl.h"
#include "bus/x68k/x68kexp.h"
#include "cpu/m68000/m68000.h"
#include "cpu/m68000/m68030.h"
#include "imagedev/floppy.h"
#include "machine/8530scc.h"
#include "machine/hd63450.h"
#include "machine/i8255.h"
#include "machine/mb87030.h"
#include "machine/mc68901.h"
#include "machine/ram.h"
#include "machine/rp5c15.h"
#include "machine/upd765.h"
#include "sound/flt_vol.h"
#include "sound/okim6258.h"
#include "sound/ymopm.h"

#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

#define GFX16     0
#define GFX256    1
#define GFX65536  2

class x68k_state : public driver_device
{
public:
	x68k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_okim6258(*this, "okim6258")
		, m_hd63450(*this, "hd63450")
		, m_ram(*this, RAM_TAG)
		, m_crtc(*this, "crtc")
		, m_gfxdecode(*this, "gfxdecode")
		, m_gfxpalette(*this, "gfxpalette")
		, m_pcgpalette(*this, "pcgpalette")
		, m_mfpdev(*this, "mc68901")
		, m_rtc(*this, "rp5c15")
		, m_scc(*this, "scc")
		, m_ym2151(*this, "ym2151")
		, m_ppi(*this, "ppi8255")
		, m_screen(*this, "screen")
		, m_upd72065(*this, "upd72065")
		, m_joy(*this, "joy%u", 1U)
		, m_expansion(*this, "exp%u", 1U)
		, m_adpcm_out(*this, {"adpcm_outl", "adpcm_outr"})
		, m_options(*this, "options")
		, m_mouse1(*this, "mouse1")
		, m_mouse2(*this, "mouse2")
		, m_mouse3(*this, "mouse3")
		, m_eject_drv_out(*this, "eject_drv%u", 0U)
		, m_ctrl_drv_out(*this, "ctrl_drv%u", 0U)
		, m_access_drv_out(*this, "access_drv%u", 0U)
		, m_nvram(0x4000/sizeof(uint16_t))
		, m_tvram(0x80000/sizeof(uint16_t))
		, m_gvram(0x80000/sizeof(uint16_t))
		, m_spritereg(0x8000/sizeof(uint16_t), 0)
	{ }

	void x68000_base(machine_config &config);
	void x68000(machine_config &config);

	virtual void driver_start() override;

protected:
	template <typename CpuType, typename AddrMap, typename Clock>
	void add_cpu(machine_config &config, CpuType &&type, AddrMap &&map, Clock &&clock)
	{
		type(config, m_maincpu, std::forward<Clock>(clock));
		m_maincpu->set_addrmap(AS_PROGRAM, std::forward<AddrMap>(map));
		m_maincpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &x68k_state::cpu_space_map);
	}

	required_device<m68000_base_device> m_maincpu;
	required_device<okim6258_device> m_okim6258;
	required_device<hd63450_device> m_hd63450;
	required_device<ram_device> m_ram;
	required_device<x68k_crtc_device> m_crtc;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_gfxpalette;
	required_device<palette_device> m_pcgpalette;
	required_device<mc68901_device> m_mfpdev;
	required_device<rp5c15_device> m_rtc;
	required_device<scc8530_legacy_device> m_scc;
	required_device<ym2151_device> m_ym2151;
	required_device<i8255_device> m_ppi;
	required_device<screen_device> m_screen;
	required_device<upd72065_device> m_upd72065;
	required_device_array<msx_general_purpose_port_device, 2> m_joy;
	required_device_array<x68k_expansion_slot_device, 2> m_expansion;

	required_device_array<filter_volume_device, 2> m_adpcm_out;

	required_ioport m_options;
	required_ioport m_mouse1;
	required_ioport m_mouse2;
	required_ioport m_mouse3;

	output_finder<4> m_eject_drv_out;
	output_finder<4> m_ctrl_drv_out;
	output_finder<4> m_access_drv_out;

	std::vector<uint16_t> m_nvram;
	std::vector<uint16_t> m_tvram;
	std::vector<uint16_t> m_gvram;
	std::vector<uint16_t> m_spritereg;

	bitmap_ind16 m_pcgbitmap;
	bitmap_ind16 m_gfxbitmap;
	bitmap_ind16 m_special;

	void floppy_load_unload(bool load, floppy_image_device *dev);
	void floppy_load(floppy_image_device *dev);
	void floppy_unload(floppy_image_device *dev);
	static void floppy_formats(format_registration &fr);

	struct
	{
		int sram_writeprotect = 0;
		int monitor = 0;
		int contrast = 0;
		int keyctrl = 0;
		uint16_t cputype = 0;
	} m_sysport;
	struct
	{
		floppy_image_device *floppy[4]{};
		int led_ctrl[4]{};
		int led_eject[4]{};
		int eject[4]{};
		int motor = 0;
		int control_drives = 0;
		int select_drive = 0;
	} m_fdc;
	struct
	{
		int rate = 0;  // ADPCM sample rate
		int pan = 0;  // ADPCM output switch
		int clock = 0;  // ADPCM clock speed
	} m_adpcm;
	struct
	{   // video controller at 0xe82000
		unsigned short reg[3]{};
		int text_pri = 0;
		int sprite_pri = 0;
		int gfx_pri = 0;
		int gfxlayer_pri[4]{};  // block displayed for each priority level
		int tile8_dirty[1024]{};
		int tile16_dirty[256]{};
		int bg_hstart = 0;
		int bg_vstart = 0;
		int bg_hvres = 0;  // bits 0,1 = H-Res, bits 2,3 = V-Res, bit 4 = L/H Freq (0=15.98kHz, 1=31.5kHz)
	} m_video;
	struct
	{
		uint8_t irqstatus = 0;
		uint8_t fdcvector = 0;
		uint8_t fddvector = 0;
		uint8_t hdcvector = 0;
		uint8_t prnvector = 0;
	} m_ioc;
	struct
	{
		int inputtype = 0;  // determines which input is to be received
		bool irqactive = false;  // true if IRQ is being serviced
		uint8_t irqvector = 0;
		char last_mouse_x = 0;  // previous mouse x-axis value
		char last_mouse_y = 0;  // previous mouse y-axis value
		int bufferempty = 0;  // non-zero if buffer is empty
	} m_mouse;
	uint8_t m_ppi_portc = 0;
	bool m_dmac_int = false;
	bool m_mfp_int = false;
	bool m_exp_irq2[2]{};
	bool m_exp_irq4[2]{};
	bool m_exp_nmi[2]{};
	uint8_t m_current_ipl = 0;
	int m_led_state = 0;
	emu_timer* m_mouse_timer = nullptr;
	emu_timer* m_led_timer = nullptr;
	unsigned char m_scc_prev = 0;
	emu_timer* m_fdc_tc = nullptr;
	emu_timer* m_adpcm_timer = nullptr;
	emu_timer* m_bus_error_timer = nullptr;
	uint16_t* m_spriteram = nullptr;
	tilemap_t* m_bg0_8 = nullptr;
	tilemap_t* m_bg1_8 = nullptr;
	tilemap_t* m_bg0_16 = nullptr;
	tilemap_t* m_bg1_16 = nullptr;
	int m_sprite_shift = 0;
	bool m_is_32bit = false;

	TILE_GET_INFO_MEMBER(get_bg0_tile);
	TILE_GET_INFO_MEMBER(get_bg1_tile);
	TILE_GET_INFO_MEMBER(get_bg0_tile_16);
	TILE_GET_INFO_MEMBER(get_bg1_tile_16);
	virtual void video_start() override ATTR_COLD;
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(floppy_tc_tick);
	TIMER_CALLBACK_MEMBER(adpcm_drq_tick);
	TIMER_CALLBACK_MEMBER(led_callback);
	TIMER_CALLBACK_MEMBER(scc_ack);
	TIMER_CALLBACK_MEMBER(bus_error);
	uint8_t ppi_port_a_r();
	uint8_t ppi_port_b_r();
	uint8_t ppi_port_c_r();
	void ppi_port_c_w(uint8_t data);
	void fdc_irq(int state);
	void ct_w(uint8_t data);
	void adpcm_w(offs_t offset, uint8_t data);
	void mfp_irq_callback(int state);

	//dmac
	void dma_irq(int state);
	void dma_end(offs_t offset, uint8_t data);

	int read_mouse();
	void set_adpcm();

	void fm_irq(int state);
	template <int N> void irq2_line(int state);
	template <int N> void irq4_line(int state);
	template <int N> void nmi_line(int state);

	void scc_w(offs_t offset, uint16_t data);
	uint16_t scc_r(offs_t offset);
	void fdc_w(offs_t offset, uint16_t data);
	uint16_t fdc_r(offs_t offset);
	void ioc_w(offs_t offset, uint16_t data);
	uint16_t ioc_r(offs_t offset);
	void sysport_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t sysport_r(offs_t offset);
	void ppi_w(offs_t offset, uint16_t data);
	uint16_t ppi_r(offs_t offset);
	void sram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t sram_r(offs_t offset);
	void vid_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t vid_r(offs_t offset);
	uint16_t areaset_r();
	void areaset_w(uint16_t data);
	void enh_areaset_w(offs_t offset, uint16_t data);
	uint16_t rom0_r(offs_t offset, uint16_t mem_mask = ~0);
	void rom0_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t emptyram_r(offs_t offset, uint16_t mem_mask = ~0);
	void emptyram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t exp_r(offs_t offset, uint16_t mem_mask = ~0);
	void exp_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t spritereg_r(offs_t offset);
	void spritereg_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t spriteram_r(offs_t offset);
	void spriteram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t tvram_read(offs_t offset);
	void tvram_write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t gvram_read(offs_t offset);
	void gvram_write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void update_ipl();
	uint8_t iack1();
	uint8_t iack2();
	uint8_t iack4();
	uint8_t iack5();

	void x68k_base_map(address_map &map) ATTR_COLD;
	void x68k_map(address_map &map) ATTR_COLD;
	void cpu_space_map(address_map &map) ATTR_COLD;

	inline void plot_pixel(bitmap_rgb32 &bitmap, int x, int y, uint32_t color);
	bool get_text_pixel(int line, int pixel, uint16_t *pix);
	bool draw_gfx_scanline(bitmap_ind16 &bitmap, rectangle cliprect, uint8_t priority);
	bool draw_gfx(bitmap_rgb32 &bitmap,rectangle cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, screen_device &screen, rectangle cliprect);
	void draw_bg(bitmap_ind16 &bitmap, screen_device &screen, int layer, bool opaque, rectangle rect);
	template <bool Blend> rgb_t get_gfx_pixel(int scanline, int pixel, bool gfxblend, rgb_t blendpix);

public:
	static rgb_t GGGGGRRRRRBBBBBI(uint32_t raw);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	void set_bus_error(uint32_t address, bool write, uint16_t mem_mask);
	bool m_bus_error = false;
};

class x68ksupr_state : public x68k_state
{
public:
	x68ksupr_state(const machine_config &mconfig, device_type type, const char *tag)
		: x68k_state(mconfig, type, tag)
		, m_scsictrl(*this, "scsi:7:spc")
	{
	}

	void x68ksupr_base(machine_config &config);
	void x68kxvi(machine_config &config);
	void x68ksupr(machine_config &config);

	virtual void driver_start() override;

protected:
	void scsi_irq(int state);
	void scsi_unknown_w(uint8_t data);

	required_device<mb89352_device> m_scsictrl;

	void x68kxvi_map(address_map &map) ATTR_COLD;
};

class x68030_state : public x68ksupr_state
{
public:
	x68030_state(const machine_config &mconfig, device_type type, const char *tag)
		: x68ksupr_state(mconfig, type, tag)
	{
	}

	void x68030(machine_config &config);

	virtual void driver_start() override;

protected:
	void x68030_map(address_map &map) ATTR_COLD;
};

#endif // MAME_SHARP_X68K_H
