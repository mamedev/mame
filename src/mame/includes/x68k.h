// license:BSD-3-Clause
// copyright-holders:Barry Rodewald,Carl
/*****************************************************************************
 *
 * includes/x68k.h
 *
 * Sharp X68000
 *
 ****************************************************************************/

#ifndef MAME_INCLUDES_X68K_H
#define MAME_INCLUDES_X68K_H

#pragma once

#include "cpu/m68000/m68000.h"
#include "imagedev/floppy.h"
#include "machine/8530scc.h"
#include "machine/hd63450.h"
#include "machine/i8255.h"
#include "machine/mb89352.h"
#include "machine/mc68901.h"
#include "machine/ram.h"
#include "machine/rp5c15.h"
#include "machine/upd765.h"
#include "sound/flt_vol.h"
#include "sound/okim6258.h"
#include "sound/ym2151.h"
#include "video/x68k_crtc.h"
#include "bus/x68k/x68kexp.h"

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
		, m_expansion(*this, "exp%u", 1U)
		, m_adpcm_out(*this, {"adpcm_outl", "adpcm_outr"})
		, m_options(*this, "options")
		, m_mouse1(*this, "mouse1")
		, m_mouse2(*this, "mouse2")
		, m_mouse3(*this, "mouse3")
		, m_xpd1lr(*this, "xpd1lr")
		, m_ctrltype(*this, "ctrltype")
		, m_joy1(*this, "joy1")
		, m_joy2(*this, "joy2")
		, m_md3b(*this, "md3b")
		, m_md6b(*this, "md6b")
		, m_md6b_extra(*this, "md6b_extra")
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

	virtual void driver_init() override;

protected:
	enum
	{
		TIMER_X68K_LED,
		TIMER_X68K_SCC_ACK,
		TIMER_MD_6BUTTON_PORT1_TIMEOUT,
		TIMER_MD_6BUTTON_PORT2_TIMEOUT,
		TIMER_X68K_BUS_ERROR,
		TIMER_X68K_FDC_TC,
		TIMER_X68K_ADPCM
	};

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
	required_device<scc8530_t> m_scc;
	required_device<ym2151_device> m_ym2151;
	required_device<i8255_device> m_ppi;
	required_device<screen_device> m_screen;
	required_device<upd72065_device> m_upd72065;
	required_device_array<x68k_expansion_slot_device, 2> m_expansion;

	required_device_array<filter_volume_device, 2> m_adpcm_out;

	required_ioport m_options;
	required_ioport m_mouse1;
	required_ioport m_mouse2;
	required_ioport m_mouse3;
	required_ioport m_xpd1lr;
	required_ioport m_ctrltype;
	required_ioport m_joy1;
	required_ioport m_joy2;
	required_ioport m_md3b;
	required_ioport m_md6b;
	required_ioport m_md6b_extra;

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
	image_init_result floppy_load(floppy_image_device *dev);
	void floppy_unload(floppy_image_device *dev);
	DECLARE_FLOPPY_FORMATS( floppy_formats );

	struct
	{
		int sram_writeprotect;
		int monitor;
		int contrast;
		int keyctrl;
		uint16_t cputype;
	} m_sysport;
	struct
	{
		floppy_image_device *floppy[4];
		int led_ctrl[4];
		int led_eject[4];
		int eject[4];
		int motor;
		int control_drives;
		int select_drive;
	} m_fdc;
	struct
	{
		int ioc7;  // "Function B operation of joystick # one option"
		int ioc6;  // "Function A operation of joystick # one option"
		int joy1_enable;  // IOC4
		int joy2_enable;  // IOC5
	} m_joy;
	struct
	{
		int rate;  // ADPCM sample rate
		int pan;  // ADPCM output switch
		int clock;  // ADPCM clock speed
	} m_adpcm;
	struct
	{   // video controller at 0xe82000
		unsigned short reg[3];
		int text_pri;
		int sprite_pri;
		int gfx_pri;
		int gfxlayer_pri[4];  // block displayed for each priority level
		int tile8_dirty[1024];
		int tile16_dirty[256];
		int bg_visible_height;
		int bg_visible_width;
		int bg_hshift;
		int bg_vshift;
		int bg_hvres;  // bits 0,1 = H-Res, bits 2,3 = V-Res, bit 4 = L/H Freq (0=15.98kHz, 1=31.5kHz)
		int bg_double;  // 1 if PCG is to be doubled.
	} m_video;
	struct
	{
		uint8_t irqstatus;
		uint8_t fdcvector;
		uint8_t fddvector;
		uint8_t hdcvector;
		uint8_t prnvector;
	} m_ioc;
	struct
	{
		int inputtype;  // determines which input is to be received
		bool irqactive;  // true if IRQ is being serviced
		uint8_t irqvector;
		char last_mouse_x;  // previous mouse x-axis value
		char last_mouse_y;  // previous mouse y-axis value
		int bufferempty;  // non-zero if buffer is empty
	} m_mouse;
	struct
	{
		// port A
		int mux1;  // multiplexer value
		int seq1;  // part of 6-button input sequence.
		emu_timer* io_timeout1;
		// port B
		int mux2;  // multiplexer value
		int seq2;  // part of 6-button input sequence.
		emu_timer* io_timeout2;
	} m_mdctrl;
	uint8_t m_ppi_port[3];
	bool m_dmac_int;
	bool m_mfp_int;
	bool m_exp_irq2[2];
	bool m_exp_irq4[2];
	bool m_exp_nmi[2];
	uint8_t m_current_ipl;
	int m_led_state;
	emu_timer* m_mouse_timer;
	emu_timer* m_led_timer;
	unsigned char m_scc_prev;
	uint16_t m_ppi_prev;
	emu_timer* m_fdc_tc;
	emu_timer* m_adpcm_timer;
	emu_timer* m_bus_error_timer;
	uint16_t* m_spriteram;
	tilemap_t* m_bg0_8;
	tilemap_t* m_bg1_8;
	tilemap_t* m_bg0_16;
	tilemap_t* m_bg1_16;
	int m_sprite_shift;
	bool m_is_32bit;

	TILE_GET_INFO_MEMBER(get_bg0_tile);
	TILE_GET_INFO_MEMBER(get_bg1_tile);
	TILE_GET_INFO_MEMBER(get_bg0_tile_16);
	TILE_GET_INFO_MEMBER(get_bg1_tile_16);
	virtual void video_start() override;
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(led_callback);
	TIMER_CALLBACK_MEMBER(scc_ack);
	TIMER_CALLBACK_MEMBER(md_6button_port1_timeout);
	TIMER_CALLBACK_MEMBER(md_6button_port2_timeout);
	TIMER_CALLBACK_MEMBER(bus_error);
	DECLARE_READ8_MEMBER(ppi_port_a_r);
	DECLARE_READ8_MEMBER(ppi_port_b_r);
	DECLARE_READ8_MEMBER(ppi_port_c_r);
	DECLARE_WRITE8_MEMBER(ppi_port_c_w);
	DECLARE_WRITE_LINE_MEMBER(fdc_irq);
	DECLARE_WRITE8_MEMBER(ct_w);
	DECLARE_WRITE8_MEMBER(adpcm_w);
	DECLARE_WRITE_LINE_MEMBER(mfp_irq_callback);

	//dmac
	DECLARE_WRITE_LINE_MEMBER(dma_irq);
	DECLARE_WRITE8_MEMBER(dma_end);

	int read_mouse();
	void set_adpcm();
	uint8_t md_3button_r(int port);
	void md_6button_init();
	uint8_t md_6button_r(int port);
	uint8_t xpd1lr_r(int port);

	DECLARE_WRITE_LINE_MEMBER(fm_irq);
	template <int N> DECLARE_WRITE_LINE_MEMBER(irq2_line);
	template <int N> DECLARE_WRITE_LINE_MEMBER(irq4_line);
	template <int N> DECLARE_WRITE_LINE_MEMBER(nmi_line);

	DECLARE_WRITE16_MEMBER(scc_w);
	DECLARE_READ16_MEMBER(scc_r);
	DECLARE_WRITE16_MEMBER(fdc_w);
	DECLARE_READ16_MEMBER(fdc_r);
	DECLARE_WRITE16_MEMBER(ioc_w);
	DECLARE_READ16_MEMBER(ioc_r);
	DECLARE_WRITE16_MEMBER(sysport_w);
	DECLARE_READ16_MEMBER(sysport_r);
	DECLARE_WRITE16_MEMBER(ppi_w);
	DECLARE_READ16_MEMBER(ppi_r);
	DECLARE_WRITE16_MEMBER(sram_w);
	DECLARE_READ16_MEMBER(sram_r);
	DECLARE_WRITE16_MEMBER(vid_w);
	DECLARE_READ16_MEMBER(vid_r);
	DECLARE_READ16_MEMBER(areaset_r);
	DECLARE_WRITE16_MEMBER(areaset_w);
	DECLARE_WRITE16_MEMBER(enh_areaset_w);
	DECLARE_READ16_MEMBER(rom0_r);
	DECLARE_WRITE16_MEMBER(rom0_w);
	DECLARE_READ16_MEMBER(emptyram_r);
	DECLARE_WRITE16_MEMBER(emptyram_w);
	DECLARE_READ16_MEMBER(exp_r);
	DECLARE_WRITE16_MEMBER(exp_w);

	DECLARE_READ16_MEMBER(spritereg_r);
	DECLARE_WRITE16_MEMBER(spritereg_w);
	DECLARE_READ16_MEMBER(spriteram_r);
	DECLARE_WRITE16_MEMBER(spriteram_w);
	DECLARE_READ16_MEMBER(tvram_read);
	DECLARE_WRITE16_MEMBER(tvram_write);
	DECLARE_READ16_MEMBER(gvram_read);
	DECLARE_WRITE16_MEMBER(gvram_write);

	void update_ipl();
	uint8_t iack1();
	uint8_t iack2();
	uint8_t iack4();
	uint8_t iack5();

	void x68k_base_map(address_map &map);
	void x68k_map(address_map &map);
	void cpu_space_map(address_map &map);

	inline void plot_pixel(bitmap_rgb32 &bitmap, int x, int y, uint32_t color);
	void draw_text(bitmap_rgb32 &bitmap, int xscr, int yscr, rectangle rect);
	bool draw_gfx_scanline(bitmap_ind16 &bitmap, rectangle cliprect, uint8_t priority);
	void draw_gfx(bitmap_rgb32 &bitmap,rectangle cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, int priority, rectangle cliprect);

public:
	static rgb_t GGGGGRRRRRBBBBBI(uint32_t raw);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void machine_start() override;
	virtual void machine_reset() override;
	void set_bus_error(uint32_t address, bool write, uint16_t mem_mask);
	bool m_bus_error;
};

class x68ksupr_state : public x68k_state
{
public:
	x68ksupr_state(const machine_config &mconfig, device_type type, const char *tag)
		: x68k_state(mconfig, type, tag)
		, m_scsictrl(*this, "mb89352")
	{
	}

	void x68ksupr_base(machine_config &config);
	void x68kxvi(machine_config &config);
	void x68ksupr(machine_config &config);

	virtual void driver_init() override;

protected:
	DECLARE_WRITE_LINE_MEMBER(scsi_irq);
	DECLARE_WRITE_LINE_MEMBER(scsi_drq);

	required_device<mb89352_device> m_scsictrl;

	void x68kxvi_map(address_map &map);
};

class x68030_state : public x68ksupr_state
{
public:
	x68030_state(const machine_config &mconfig, device_type type, const char *tag)
		: x68ksupr_state(mconfig, type, tag)
	{
	}

	void x68030(machine_config &config);

	virtual void driver_init() override;

protected:
	void x68030_map(address_map &map);
};

#endif // MAME_INCLUDES_X68K_H
