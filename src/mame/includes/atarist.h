// license:BSD-3-Clause
// copyright-holders:Curt Coder, Olivier Galibert
#pragma once

#ifndef __ATARI_ST__
#define __ATARI_ST__

#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "cpu/m6800/m6800.h"
#include "machine/6850acia.h"
#include "machine/8530scc.h"
#include "bus/centronics/ctronics.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "machine/mc68901.h"
#include "machine/ram.h"
#include "machine/rescap.h"
#include "machine/rp5c15.h"
#include "machine/wd_fdc.h"
#include "sound/ay8910.h"
#include "sound/lmc1992.h"

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

#define Y1      XTAL_2_4576MHz

// STBook

#define U517    XTAL_16MHz
#define Y200    XTAL_2_4576MHz
#define Y700    XTAL_10MHz

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
			m_fdc(*this, WD1772_TAG),
			m_mfp(*this, MC68901_TAG),
			m_acia0(*this, MC6850_0_TAG),
			m_acia1(*this, MC6850_1_TAG),
			m_centronics(*this, CENTRONICS_TAG),
			m_cart(*this, "cartslot"),
			m_ram(*this, RAM_TAG),
			m_rs232(*this, RS232_TAG),
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
			m_acia_ikbd_irq(1),
			m_acia_midi_irq(1),
			m_ikbd_mouse_x(0),
			m_ikbd_mouse_y(0),
			m_ikbd_mouse_px(IKBD_MOUSE_PHASE_STATIC),
			m_ikbd_mouse_py(IKBD_MOUSE_PHASE_STATIC),
			m_ikbd_mouse_pc(0),
			m_ikbd_joy(1),
			m_monochrome(1),
			m_palette(*this, "palette")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<wd1772_t> m_fdc;
	required_device<mc68901_device> m_mfp;
	required_device<acia6850_device> m_acia0;
	required_device<acia6850_device> m_acia1;
	required_device<centronics_device> m_centronics;
	required_device<generic_slot_device> m_cart;
	required_device<ram_device> m_ram;
	required_device<rs232_port_device> m_rs232;
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

	void machine_start() override;

	void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	// video
	uint8_t shifter_base_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t shifter_counter_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t shifter_sync_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint16_t shifter_palette_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint8_t shifter_mode_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	void shifter_base_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void shifter_sync_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void shifter_palette_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void shifter_mode_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	uint16_t blitter_halftone_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t blitter_src_inc_x_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t blitter_src_inc_y_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t blitter_src_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t blitter_end_mask_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t blitter_dst_inc_x_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t blitter_dst_inc_y_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t blitter_dst_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t blitter_count_x_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t blitter_count_y_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t blitter_op_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t blitter_ctrl_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);

	void blitter_halftone_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void blitter_src_inc_x_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void blitter_src_inc_y_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void blitter_src_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void blitter_end_mask_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void blitter_dst_inc_x_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void blitter_dst_inc_y_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void blitter_dst_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void blitter_count_x_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void blitter_count_y_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void blitter_op_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void blitter_ctrl_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	void mouse_tick();
	inline pen_t shift_mode_0();
	inline pen_t shift_mode_1();
	inline pen_t shift_mode_2();
	void shifter_tick();
	inline void shifter_load();
	void glue_tick();
	void set_screen_parameters();
	void blitter_source();
	uint16_t blitter_hop();
	void blitter_op(uint16_t s, uint32_t dstaddr, uint16_t mask);
	void blitter_tick();

	// driver
	uint16_t fdc_data_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void fdc_data_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t dma_status_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void dma_mode_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t dma_counter_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void dma_base_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mmu_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mmu_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t berr_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void berr_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t ikbd_port1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t ikbd_port2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ikbd_port2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ikbd_port3_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ikbd_port4_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ikbd_port4_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void fdc_drq_w(int state);

	void psg_pa_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void ikbd_tx_w(int state);
	void acia_ikbd_irq_w(int state);
	void acia_midi_irq_w(int state);

	uint8_t mfp_gpio_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mfp_tdo_w(int state);

	void write_acia_clock(int state);

	void toggle_dma_fifo();
	void flush_dma_fifo();
	void fill_dma_fifo();
	void fdc_dma_transfer();

	void configure_memory();
	void state_save();

	/* memory state */
	uint8_t m_mmu;

	/* keyboard state */
	int m_acia_ikbd_irq;
	int m_acia_midi_irq;
	uint16_t m_ikbd_keylatch;
	uint8_t m_ikbd_mouse;
	uint8_t m_ikbd_mouse_x;
	uint8_t m_ikbd_mouse_y;
	uint8_t m_ikbd_mouse_px;
	uint8_t m_ikbd_mouse_py;
	uint8_t m_ikbd_mouse_pc;
	int m_ikbd_tx;
	int m_ikbd_joy;
	int m_midi_tx;

	/* floppy state */
	uint32_t m_dma_base;
	uint16_t m_dma_error;
	uint16_t m_fdc_mode;
	uint8_t m_fdc_sectors;
	uint16_t m_fdc_fifo[2][8];
	int m_fdc_fifo_sel;
	int m_fdc_fifo_index;
	int m_fdc_fifo_msb;
	int m_fdc_fifo_empty[2];
	int m_fdc_dmabytes;

	/* shifter state */
	uint32_t m_shifter_base;
	uint32_t m_shifter_ofs;
	uint8_t m_shifter_sync;
	uint8_t m_shifter_mode;
	uint16_t m_shifter_palette[16];
	uint16_t m_shifter_rr[4];
	uint16_t m_shifter_ir[4];
	int m_shifter_bitplane;
	int m_shifter_shift;
	int m_shifter_h;
	int m_shifter_v;
	int m_shifter_de;
	int m_shifter_x_start;
	int m_shifter_x_end;
	int m_shifter_y_start;
	int m_shifter_y_end;
	int m_shifter_hblank_start;
	int m_shifter_vblank_start;

	/* blitter state */
	uint16_t m_blitter_halftone[16];
	int16_t m_blitter_src_inc_x;
	int16_t m_blitter_src_inc_y;
	int16_t m_blitter_dst_inc_x;
	int16_t m_blitter_dst_inc_y;
	uint32_t m_blitter_src;
	uint32_t m_blitter_dst;
	uint16_t m_blitter_endmask1;
	uint16_t m_blitter_endmask2;
	uint16_t m_blitter_endmask3;
	uint16_t m_blitter_xcount;
	uint16_t m_blitter_ycount;
	uint16_t m_blitter_xcountl;
	uint8_t m_blitter_hop;
	uint8_t m_blitter_op;
	uint8_t m_blitter_ctrl;
	uint8_t m_blitter_skew;
	uint32_t m_blitter_srcbuf;

	/* timers */
	emu_timer *m_mouse_timer;
	emu_timer *m_glue_timer;
	emu_timer *m_shifter_timer;

	bitmap_rgb32 m_bitmap;

	floppy_image_device *floppy_devices[2];

	DECLARE_FLOPPY_FORMATS(floppy_formats);
	int atarist_int_ack(device_t &device, int irqline);

	int m_monochrome;
	required_device<palette_device> m_palette;
	void write_monochrome(int state);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};

class megast_state : public st_state
{
public:
	megast_state(const machine_config &mconfig, device_type type, const char *tag)
		: st_state(mconfig, type, tag)
	{ }

	uint16_t fpu_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void fpu_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
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

	void machine_start() override;

	void video_start() override;

	uint8_t shifter_base_low_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void shifter_base_low_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t shifter_counter_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void shifter_counter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void shifter_palette_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t shifter_lineofs_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void shifter_lineofs_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t shifter_pixelofs_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void shifter_pixelofs_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	uint8_t sound_dma_control_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t sound_dma_base_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t sound_dma_counter_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t sound_dma_end_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t sound_mode_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void sound_dma_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sound_dma_base_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sound_dma_end_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sound_mode_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t microwire_data_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void microwire_data_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t microwire_mask_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void microwire_mask_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	void write_monochrome(int state);

	void dmasound_set_state(int level);
	void dmasound_tick();
	void microwire_shift();
	void microwire_tick();
	void state_save();

	// shifter state
	uint8_t m_shifter_lineofs;
	uint8_t m_shifter_pixelofs;

	/* microwire state */
	uint16_t m_mw_data;
	uint16_t m_mw_mask;
	int m_mw_shift;

	/* DMA sound state */
	uint32_t m_dmasnd_base;
	uint32_t m_dmasnd_end;
	uint32_t m_dmasnd_cntr;
	uint32_t m_dmasnd_baselatch;
	uint32_t m_dmasnd_endlatch;
	uint8_t m_dmasnd_ctrl;
	uint8_t m_dmasnd_mode;
	uint8_t m_dmasnd_fifo[8];
	uint8_t m_dmasnd_samples;
	int m_dmasnd_active;

	// timers
	emu_timer *m_microwire_timer;
	emu_timer *m_dmasound_timer;

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};

class megaste_state : public ste_state
{
public:
	megaste_state(const machine_config &mconfig, device_type type, const char *tag)
		: ste_state(mconfig, type, tag)
	{ }

	void machine_start() override;

	uint16_t cache_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void cache_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	uint16_t m_cache;
};

class stbook_state : public ste_state
{
public:
	stbook_state(const machine_config &mconfig, device_type type, const char *tag)
		: ste_state(mconfig, type, tag),
			m_sw400(*this, "SW400")
	{ }

	required_ioport m_sw400;

	void machine_start() override;
	void video_start() override;

	uint16_t config_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void lcd_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	void psg_pa_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mfp_gpio_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
};

#endif
