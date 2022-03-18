// license:BSD-3-Clause
// copyright-holders:Curt Coder, Olivier Galibert
#ifndef MAME_INCLUDES_ATARI_ST_H
#define MAME_INCLUDES_ATARI_ST_H

#pragma once

#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "cpu/m6800/m6801.h"
#include "machine/6850acia.h"
#include "machine/8530scc.h"
#include "bus/centronics/ctronics.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "imagedev/floppy.h"
#include "machine/mc68901.h"
#include "machine/ram.h"
#include "machine/rescap.h"
#include "machine/rp5c15.h"
#include "machine/wd_fdc.h"
#include "sound/ay8910.h"
#include "sound/lmc1992.h"
#include "emupal.h"
#include "screen.h"

#define M68000_TAG      "m68000"
#define HD6301V1_TAG    "hd6301"
#define YM2149_TAG      "ym2149"
#define MC6850_0_TAG    "mc6850_0"
#define MC6850_1_TAG    "mc6850_1"
#define Z8530_TAG       "z8530"
#define COP888_TAG      "u703"
#define RP5C15_TAG      "rp5c15"
#define YM3439_TAG      "ym3439"
#define MC68901_TAG     "mc68901"
#define LMC1992_TAG     "lmc1992"
#define WD1772_TAG      "wd1772"
#define SCREEN_TAG      "screen"
#define CENTRONICS_TAG  "centronics"
#define RS232_TAG       "rs232"

// Atari ST

#define Y1      XTAL(2'457'600)

// STBook

#define U517    XTAL(16'000'000)
#define Y200    XTAL(2'457'600)
#define Y700    XTAL(10'000'000)

#define DMA_STATUS_DRQ              0x04
#define DMA_STATUS_SECTOR_COUNT     0x02
#define DMA_STATUS_ERROR            0x01

#define DMA_MODE_READ_WRITE         0x100
#define DMA_MODE_FDC_HDC_ACK        0x080
#define DMA_MODE_ENABLED            0x040
#define DMA_MODE_SECTOR_COUNT       0x010
#define DMA_MODE_FDC_HDC_CS         0x008
#define DMA_MODE_A1                 0x004
#define DMA_MODE_A0                 0x002
#define DMA_MODE_ADDRESS_MASK       0x006

#define DMA_SECTOR_SIZE             512

enum
{
	IKBD_MOUSE_PHASE_STATIC = 0,
	IKBD_MOUSE_PHASE_POSITIVE,
	IKBD_MOUSE_PHASE_NEGATIVE
};

class st_state : public driver_device
{
public:
	enum
	{
		TIMER_MOUSE_TICK,
		TIMER_SHIFTER_TICK,
		TIMER_GLUE_TICK,
		TIMER_BLITTER_TICK
	};

	st_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, M68000_TAG),
			m_ikbd(*this, HD6301V1_TAG),
			m_fdc(*this, WD1772_TAG),
			m_floppy(*this, WD1772_TAG ":%u", 0U),
			m_mfp(*this, MC68901_TAG),
			m_acia(*this, {MC6850_0_TAG, MC6850_1_TAG}),
			m_centronics(*this, CENTRONICS_TAG),
			m_cart(*this, "cartslot"),
			m_ram(*this, RAM_TAG),
			m_rs232(*this, RS232_TAG),
			m_ymsnd(*this, YM2149_TAG),
			m_p31(*this, "P31"),
			m_p32(*this, "P32"),
			m_p33(*this, "P33"),
			m_p34(*this, "P34"),
			m_p35(*this, "P35"),
			m_p36(*this, "P36"),
			m_p37(*this, "P37"),
			m_p40(*this, "P40"),
			m_p41(*this, "P41"),
			m_p42(*this, "P42"),
			m_p43(*this, "P43"),
			m_p44(*this, "P44"),
			m_p45(*this, "P45"),
			m_p46(*this, "P46"),
			m_p47(*this, "P47"),
			m_joy0(*this, "IKBD_JOY0"),
			m_joy1(*this, "IKBD_JOY1"),
			m_mousex(*this, "IKBD_MOUSEX"),
			m_mousey(*this, "IKBD_MOUSEY"),
			m_config(*this, "config"),
			m_ikbd_mouse_x(0),
			m_ikbd_mouse_y(0),
			m_ikbd_mouse_px(IKBD_MOUSE_PHASE_STATIC),
			m_ikbd_mouse_py(IKBD_MOUSE_PHASE_STATIC),
			m_ikbd_mouse_pc(0),
			m_ikbd_joy(1),
			m_monochrome(1),
			m_palette(*this, "palette"),
			m_screen(*this, "screen"),
			m_led(*this, "led1")
	{ }

	DECLARE_WRITE_LINE_MEMBER( write_monochrome );

	void st(machine_config &config);

protected:
	required_device<m68000_base_device> m_maincpu;
	required_device<cpu_device> m_ikbd;
	required_device<wd1772_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	required_device<mc68901_device> m_mfp;
	required_device_array<acia6850_device, 2> m_acia;
	required_device<centronics_device> m_centronics;
	required_device<generic_slot_device> m_cart;
	required_device<ram_device> m_ram;
	required_device<rs232_port_device> m_rs232;
	required_device<ym2149_device> m_ymsnd;
	required_ioport m_p31;
	required_ioport m_p32;
	required_ioport m_p33;
	required_ioport m_p34;
	required_ioport m_p35;
	required_ioport m_p36;
	required_ioport m_p37;
	required_ioport m_p40;
	required_ioport m_p41;
	required_ioport m_p42;
	required_ioport m_p43;
	required_ioport m_p44;
	required_ioport m_p45;
	required_ioport m_p46;
	required_ioport m_p47;
	optional_ioport m_joy0;
	optional_ioport m_joy1;
	optional_ioport m_mousex;
	optional_ioport m_mousey;
	optional_ioport m_config;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	// video
	uint8_t shifter_base_r(offs_t offset);
	uint8_t shifter_counter_r(offs_t offset);
	uint8_t shifter_sync_r();
	uint16_t shifter_palette_r(offs_t offset);
	uint8_t shifter_mode_r();

	void shifter_base_w(offs_t offset, uint8_t data);
	void shifter_sync_w(uint8_t data);
	void shifter_palette_w(offs_t offset, uint16_t data);
	void shifter_mode_w(uint8_t data);

	uint16_t blitter_halftone_r(offs_t offset);
	uint16_t blitter_src_inc_x_r();
	uint16_t blitter_src_inc_y_r();
	uint16_t blitter_src_r(offs_t offset);
	uint16_t blitter_end_mask_r(offs_t offset);
	uint16_t blitter_dst_inc_x_r();
	uint16_t blitter_dst_inc_y_r();
	uint16_t blitter_dst_r(offs_t offset);
	uint16_t blitter_count_x_r();
	uint16_t blitter_count_y_r();
	uint16_t blitter_op_r(offs_t offset, uint16_t mem_mask = ~0);
	uint16_t blitter_ctrl_r(offs_t offset, uint16_t mem_mask = ~0);

	void blitter_halftone_w(offs_t offset, uint16_t data);
	void blitter_src_inc_x_w(uint16_t data);
	void blitter_src_inc_y_w(uint16_t data);
	void blitter_src_w(offs_t offset, uint16_t data);
	void blitter_end_mask_w(offs_t offset, uint16_t data);
	void blitter_dst_inc_x_w(uint16_t data);
	void blitter_dst_inc_y_w(uint16_t data);
	void blitter_dst_w(offs_t offset, uint16_t data);
	void blitter_count_x_w(uint16_t data);
	void blitter_count_y_w(uint16_t data);
	void blitter_op_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void blitter_ctrl_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void mouse_tick();
	inline pen_t shift_mode_0();
	inline pen_t shift_mode_1();
	inline pen_t shift_mode_2();
	void shifter_tick();
	inline void shifter_load();
	inline void draw_pixel(int x, int y, u32 pen);
	void glue_tick();
	void set_screen_parameters();
	void blitter_source();
	uint16_t blitter_hop();
	void blitter_op(uint16_t s, uint32_t dstaddr, uint16_t mask);
	void blitter_tick();

	// driver
	uint16_t fdc_data_r(offs_t offset);
	void fdc_data_w(offs_t offset, uint16_t data);
	uint16_t dma_status_r();
	void dma_mode_w(uint16_t data);
	uint8_t dma_counter_r(offs_t offset);
	void dma_base_w(offs_t offset, uint8_t data);
	uint8_t mmu_r();
	void mmu_w(uint8_t data);
	uint16_t berr_r();
	void berr_w(uint16_t data);
	uint8_t ikbd_port1_r();
	uint8_t ikbd_port2_r();
	void ikbd_port2_w(uint8_t data);
	void ikbd_port3_w(uint8_t data);
	uint8_t ikbd_port4_r();
	void ikbd_port4_w(uint8_t data);

	DECLARE_WRITE_LINE_MEMBER( fdc_drq_w );

	void psg_pa_w(uint8_t data);

	DECLARE_WRITE_LINE_MEMBER( ikbd_tx_w );

	DECLARE_WRITE_LINE_MEMBER( reset_w );

	void toggle_dma_fifo();
	void flush_dma_fifo();
	void fill_dma_fifo();
	void fdc_dma_transfer();

	void configure_memory();
	void state_save();

	/* memory state */
	uint8_t m_mmu = 0U;

	/* keyboard state */
	uint16_t m_ikbd_keylatch = 0U;
	uint8_t m_ikbd_mouse = 0U;
	uint8_t m_ikbd_mouse_x = 0U;
	uint8_t m_ikbd_mouse_y = 0U;
	uint8_t m_ikbd_mouse_px = 0U;
	uint8_t m_ikbd_mouse_py = 0U;
	uint8_t m_ikbd_mouse_pc = 0U;
	int m_ikbd_tx = 0;
	int m_ikbd_joy = 0;
	int m_midi_tx = 0;

	/* floppy state */
	uint32_t m_dma_base = 0U;
	uint16_t m_dma_error = 0U;
	uint16_t m_fdc_mode = 0U;
	uint8_t m_fdc_sectors = 0U;
	uint16_t m_fdc_fifo[2][8]{};
	int m_fdc_fifo_sel = 0;
	int m_fdc_fifo_index = 0;
	int m_fdc_fifo_msb = 0;
	int m_fdc_fifo_empty[2]{};
	int m_fdc_dmabytes = 0;

	/* shifter state */
	uint32_t m_shifter_base = 0U;
	uint32_t m_shifter_ofs = 0U;
	uint8_t m_shifter_sync = 0U;
	uint8_t m_shifter_mode = 0U;
	uint16_t m_shifter_palette[16]{};
	uint16_t m_shifter_rr[4]{};
	uint16_t m_shifter_ir[4]{};
	int m_shifter_bitplane = 0;
	int m_shifter_shift = 0;
	int m_shifter_h = 0;
	int m_shifter_v = 0;
	int m_shifter_de = 0;
	int m_shifter_x_start = 0;
	int m_shifter_x_end = 0;
	int m_shifter_y_start = 0;
	int m_shifter_y_end = 0;
	int m_shifter_hblank_start = 0;
	int m_shifter_vblank_start = 0;

	/* blitter state */
	uint16_t m_blitter_halftone[16]{};
	int16_t m_blitter_src_inc_x = 0;
	int16_t m_blitter_src_inc_y = 0;
	int16_t m_blitter_dst_inc_x = 0;
	int16_t m_blitter_dst_inc_y = 0;
	uint32_t m_blitter_src = 0U;
	uint32_t m_blitter_dst = 0U;
	uint16_t m_blitter_endmask1 = 0U;
	uint16_t m_blitter_endmask2 = 0U;
	uint16_t m_blitter_endmask3 = 0U;
	uint16_t m_blitter_xcount = 0U;
	uint16_t m_blitter_ycount = 0U;
	uint16_t m_blitter_xcountl = 0U;
	uint8_t m_blitter_hop = 0U;
	uint8_t m_blitter_op = 0U;
	uint8_t m_blitter_ctrl = 0U;
	uint8_t m_blitter_skew = 0U;
	uint32_t m_blitter_srcbuf = 0U;

	/* timers */
	emu_timer *m_mouse_timer = 0;
	emu_timer *m_glue_timer = 0;
	emu_timer *m_shifter_timer = 0;

	bitmap_rgb32 m_bitmap = 0;

	static void floppy_formats(format_registration &fr);

	int m_monochrome = 0;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;

	void common(machine_config &config);
	void ikbd_map(address_map &map);
	void cpu_space_map(address_map &map);
	void st_map(address_map &map);
	void keyboard(machine_config &config);

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;
	virtual void machine_start() override;
	virtual void video_start() override;

	output_finder<> m_led;
};

class megast_state : public st_state
{
public:
	megast_state(const machine_config &mconfig, device_type type, const char *tag)
		: st_state(mconfig, type, tag)
	{ }

	uint16_t fpu_r();
	void fpu_w(uint16_t data);
	void megast(machine_config &config);
	void megast_map(address_map &map);
};

class ste_state : public st_state
{
public:
	enum
	{
		TIMER_DMASOUND_TICK,
		TIMER_MICROWIRE_TICK
	};

	ste_state(const machine_config &mconfig, device_type type, const char *tag)
		: st_state(mconfig, type, tag),
			m_lmc1992(*this, LMC1992_TAG)
	{ }

	optional_device<lmc1992_device> m_lmc1992;

	uint8_t shifter_base_low_r();
	void shifter_base_low_w(uint8_t data);
	uint8_t shifter_counter_r(offs_t offset);
	void shifter_counter_w(offs_t offset, uint8_t data);
	void shifter_palette_w(offs_t offset, uint16_t data);
	uint8_t shifter_lineofs_r();
	void shifter_lineofs_w(uint8_t data);
	uint8_t shifter_pixelofs_r();
	void shifter_pixelofs_w(uint8_t data);

	uint8_t sound_dma_control_r();
	uint8_t sound_dma_base_r(offs_t offset);
	uint8_t sound_dma_counter_r(offs_t offset);
	uint8_t sound_dma_end_r(offs_t offset);
	uint8_t sound_mode_r();
	void sound_dma_control_w(uint8_t data);
	void sound_dma_base_w(offs_t offset, uint8_t data);
	void sound_dma_end_w(offs_t offset, uint8_t data);
	void sound_mode_w(uint8_t data);
	uint16_t microwire_data_r();
	void microwire_data_w(uint16_t data);
	uint16_t microwire_mask_r();
	void microwire_mask_w(uint16_t data);

	DECLARE_WRITE_LINE_MEMBER( write_monochrome );

	void dmasound_set_state(int level);
	void dmasound_tick();
	void microwire_shift();
	void microwire_tick();
	void state_save();

	// shifter state
	uint8_t m_shifter_lineofs = 0U;
	uint8_t m_shifter_pixelofs = 0U;

	/* microwire state */
	uint16_t m_mw_data = 0U;
	uint16_t m_mw_mask = 0U;
	int m_mw_shift = 0;

	/* DMA sound state */
	uint32_t m_dmasnd_base = 0U;
	uint32_t m_dmasnd_end = 0U;
	uint32_t m_dmasnd_cntr = 0U;
	uint32_t m_dmasnd_baselatch = 0U;
	uint32_t m_dmasnd_endlatch = 0U;
	uint8_t m_dmasnd_ctrl = 0U;
	uint8_t m_dmasnd_mode = 0U;
	uint8_t m_dmasnd_fifo[8]{};
	uint8_t m_dmasnd_samples = 0U;
	int m_dmasnd_active = 0;

	// timers
	emu_timer *m_microwire_timer = 0;
	emu_timer *m_dmasound_timer = 0;

	void falcon40(machine_config &config);
	void tt030(machine_config &config);
	void falcon(machine_config &config);
	void ste(machine_config &config);
	void ste_map(address_map &map);
protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;
	virtual void machine_start() override;
	virtual void video_start() override;
};

class megaste_state : public ste_state
{
public:
	megaste_state(const machine_config &mconfig, device_type type, const char *tag)
		: ste_state(mconfig, type, tag)
	{ }

	uint16_t cache_r();
	void cache_w(uint16_t data);

	uint16_t m_cache;
	void megaste(machine_config &config);
	void megaste_map(address_map &map);

protected:
	virtual void machine_start() override;
};

class stbook_state : public ste_state
{
public:
	stbook_state(const machine_config &mconfig, device_type type, const char *tag)
		: ste_state(mconfig, type, tag),
			m_sw400(*this, "SW400")
	{ }

	required_ioport m_sw400;

	uint16_t config_r();
	void lcd_control_w(uint16_t data);

	void psg_pa_w(uint8_t data);
	uint8_t mfp_gpio_r();
	void stbook_map(address_map &map);
protected:
	virtual void machine_start() override;
	virtual void video_start() override;
};

#endif // MAME_INCLUDES_ATARI_ST_H
