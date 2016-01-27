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

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	// video
	DECLARE_READ8_MEMBER( shifter_base_r );
	DECLARE_READ8_MEMBER( shifter_counter_r );
	DECLARE_READ8_MEMBER( shifter_sync_r );
	DECLARE_READ16_MEMBER( shifter_palette_r );
	DECLARE_READ8_MEMBER( shifter_mode_r );

	DECLARE_WRITE8_MEMBER( shifter_base_w );
	DECLARE_WRITE8_MEMBER( shifter_sync_w );
	DECLARE_WRITE16_MEMBER( shifter_palette_w );
	DECLARE_WRITE8_MEMBER( shifter_mode_w );

	DECLARE_READ16_MEMBER( blitter_halftone_r );
	DECLARE_READ16_MEMBER( blitter_src_inc_x_r );
	DECLARE_READ16_MEMBER( blitter_src_inc_y_r );
	DECLARE_READ16_MEMBER( blitter_src_r );
	DECLARE_READ16_MEMBER( blitter_end_mask_r );
	DECLARE_READ16_MEMBER( blitter_dst_inc_x_r );
	DECLARE_READ16_MEMBER( blitter_dst_inc_y_r );
	DECLARE_READ16_MEMBER( blitter_dst_r );
	DECLARE_READ16_MEMBER( blitter_count_x_r );
	DECLARE_READ16_MEMBER( blitter_count_y_r );
	DECLARE_READ16_MEMBER( blitter_op_r );
	DECLARE_READ16_MEMBER( blitter_ctrl_r );

	DECLARE_WRITE16_MEMBER( blitter_halftone_w );
	DECLARE_WRITE16_MEMBER( blitter_src_inc_x_w );
	DECLARE_WRITE16_MEMBER( blitter_src_inc_y_w );
	DECLARE_WRITE16_MEMBER( blitter_src_w );
	DECLARE_WRITE16_MEMBER( blitter_end_mask_w );
	DECLARE_WRITE16_MEMBER( blitter_dst_inc_x_w );
	DECLARE_WRITE16_MEMBER( blitter_dst_inc_y_w );
	DECLARE_WRITE16_MEMBER( blitter_dst_w );
	DECLARE_WRITE16_MEMBER( blitter_count_x_w );
	DECLARE_WRITE16_MEMBER( blitter_count_y_w );
	DECLARE_WRITE16_MEMBER( blitter_op_w );
	DECLARE_WRITE16_MEMBER( blitter_ctrl_w );

	void mouse_tick();
	inline pen_t shift_mode_0();
	inline pen_t shift_mode_1();
	inline pen_t shift_mode_2();
	void shifter_tick();
	inline void shifter_load();
	void glue_tick();
	void set_screen_parameters();
	void blitter_source();
	UINT16 blitter_hop();
	void blitter_op(UINT16 s, UINT32 dstaddr, UINT16 mask);
	void blitter_tick();

	// driver
	DECLARE_READ16_MEMBER( fdc_data_r );
	DECLARE_WRITE16_MEMBER( fdc_data_w );
	DECLARE_READ16_MEMBER( dma_status_r );
	DECLARE_WRITE16_MEMBER( dma_mode_w );
	DECLARE_READ8_MEMBER( dma_counter_r );
	DECLARE_WRITE8_MEMBER( dma_base_w );
	DECLARE_READ8_MEMBER( mmu_r );
	DECLARE_WRITE8_MEMBER( mmu_w );
	DECLARE_READ16_MEMBER( berr_r );
	DECLARE_WRITE16_MEMBER( berr_w );
	DECLARE_READ8_MEMBER( ikbd_port1_r );
	DECLARE_READ8_MEMBER( ikbd_port2_r );
	DECLARE_WRITE8_MEMBER( ikbd_port2_w );
	DECLARE_WRITE8_MEMBER( ikbd_port3_w );
	DECLARE_READ8_MEMBER( ikbd_port4_r );
	DECLARE_WRITE8_MEMBER( ikbd_port4_w );

	DECLARE_WRITE_LINE_MEMBER( fdc_drq_w );

	DECLARE_WRITE8_MEMBER( psg_pa_w );

	DECLARE_WRITE_LINE_MEMBER( ikbd_tx_w );
	DECLARE_WRITE_LINE_MEMBER( acia_ikbd_irq_w );
	DECLARE_WRITE_LINE_MEMBER( acia_midi_irq_w );

	DECLARE_READ8_MEMBER( mfp_gpio_r );
	DECLARE_WRITE_LINE_MEMBER( mfp_tdo_w );

	DECLARE_WRITE_LINE_MEMBER( write_acia_clock );

	void toggle_dma_fifo();
	void flush_dma_fifo();
	void fill_dma_fifo();
	void fdc_dma_transfer();

	void configure_memory();
	void state_save();

	/* memory state */
	UINT8 m_mmu;

	/* keyboard state */
	int m_acia_ikbd_irq;
	int m_acia_midi_irq;
	UINT16 m_ikbd_keylatch;
	UINT8 m_ikbd_mouse;
	UINT8 m_ikbd_mouse_x;
	UINT8 m_ikbd_mouse_y;
	UINT8 m_ikbd_mouse_px;
	UINT8 m_ikbd_mouse_py;
	UINT8 m_ikbd_mouse_pc;
	int m_ikbd_tx;
	int m_ikbd_joy;
	int m_midi_tx;

	/* floppy state */
	UINT32 m_dma_base;
	UINT16 m_dma_error;
	UINT16 m_fdc_mode;
	UINT8 m_fdc_sectors;
	UINT16 m_fdc_fifo[2][8];
	int m_fdc_fifo_sel;
	int m_fdc_fifo_index;
	int m_fdc_fifo_msb;
	int m_fdc_fifo_empty[2];
	int m_fdc_dmabytes;

	/* shifter state */
	UINT32 m_shifter_base;
	UINT32 m_shifter_ofs;
	UINT8 m_shifter_sync;
	UINT8 m_shifter_mode;
	UINT16 m_shifter_palette[16];
	UINT16 m_shifter_rr[4];
	UINT16 m_shifter_ir[4];
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
	UINT16 m_blitter_halftone[16];
	INT16 m_blitter_src_inc_x;
	INT16 m_blitter_src_inc_y;
	INT16 m_blitter_dst_inc_x;
	INT16 m_blitter_dst_inc_y;
	UINT32 m_blitter_src;
	UINT32 m_blitter_dst;
	UINT16 m_blitter_endmask1;
	UINT16 m_blitter_endmask2;
	UINT16 m_blitter_endmask3;
	UINT16 m_blitter_xcount;
	UINT16 m_blitter_ycount;
	UINT16 m_blitter_xcountl;
	UINT8 m_blitter_hop;
	UINT8 m_blitter_op;
	UINT8 m_blitter_ctrl;
	UINT8 m_blitter_skew;
	UINT32 m_blitter_srcbuf;

	/* timers */
	emu_timer *m_mouse_timer;
	emu_timer *m_glue_timer;
	emu_timer *m_shifter_timer;

	bitmap_rgb32 m_bitmap;

	floppy_image_device *floppy_devices[2];

	DECLARE_FLOPPY_FORMATS(floppy_formats);
	IRQ_CALLBACK_MEMBER(atarist_int_ack);

	int m_monochrome;
	required_device<palette_device> m_palette;
	DECLARE_WRITE_LINE_MEMBER( write_monochrome );

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};

class megast_state : public st_state
{
public:
	megast_state(const machine_config &mconfig, device_type type, const char *tag)
		: st_state(mconfig, type, tag)
	{ }

	DECLARE_READ16_MEMBER( fpu_r );
	DECLARE_WRITE16_MEMBER( fpu_w );
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

	DECLARE_READ8_MEMBER( shifter_base_low_r );
	DECLARE_WRITE8_MEMBER( shifter_base_low_w );
	DECLARE_READ8_MEMBER( shifter_counter_r );
	DECLARE_WRITE8_MEMBER( shifter_counter_w );
	DECLARE_WRITE16_MEMBER( shifter_palette_w );
	DECLARE_READ8_MEMBER( shifter_lineofs_r );
	DECLARE_WRITE8_MEMBER( shifter_lineofs_w );
	DECLARE_READ8_MEMBER( shifter_pixelofs_r );
	DECLARE_WRITE8_MEMBER( shifter_pixelofs_w );

	DECLARE_READ8_MEMBER( sound_dma_control_r );
	DECLARE_READ8_MEMBER( sound_dma_base_r );
	DECLARE_READ8_MEMBER( sound_dma_counter_r );
	DECLARE_READ8_MEMBER( sound_dma_end_r );
	DECLARE_READ8_MEMBER( sound_mode_r );
	DECLARE_WRITE8_MEMBER( sound_dma_control_w );
	DECLARE_WRITE8_MEMBER( sound_dma_base_w );
	DECLARE_WRITE8_MEMBER( sound_dma_end_w );
	DECLARE_WRITE8_MEMBER( sound_mode_w );
	DECLARE_READ16_MEMBER( microwire_data_r );
	DECLARE_WRITE16_MEMBER( microwire_data_w );
	DECLARE_READ16_MEMBER( microwire_mask_r );
	DECLARE_WRITE16_MEMBER( microwire_mask_w );

	DECLARE_WRITE_LINE_MEMBER( write_monochrome );

	void dmasound_set_state(int level);
	void dmasound_tick();
	void microwire_shift();
	void microwire_tick();
	void state_save();

	// shifter state
	UINT8 m_shifter_lineofs;
	UINT8 m_shifter_pixelofs;

	/* microwire state */
	UINT16 m_mw_data;
	UINT16 m_mw_mask;
	int m_mw_shift;

	/* DMA sound state */
	UINT32 m_dmasnd_base;
	UINT32 m_dmasnd_end;
	UINT32 m_dmasnd_cntr;
	UINT32 m_dmasnd_baselatch;
	UINT32 m_dmasnd_endlatch;
	UINT8 m_dmasnd_ctrl;
	UINT8 m_dmasnd_mode;
	UINT8 m_dmasnd_fifo[8];
	UINT8 m_dmasnd_samples;
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

	DECLARE_READ16_MEMBER( cache_r );
	DECLARE_WRITE16_MEMBER( cache_w );

	UINT16 m_cache;
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

	DECLARE_READ16_MEMBER( config_r );
	DECLARE_WRITE16_MEMBER( lcd_control_w );

	DECLARE_WRITE8_MEMBER( psg_pa_w );
	DECLARE_READ8_MEMBER( mfp_gpio_r );
};

#endif
