// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Peter Trauner, Angelo Salese
/***************************************************************************

    pc_vga.h

    PC standard VGA adaptor

***************************************************************************/

#ifndef MAME_VIDEO_PC_VGA_H
#define MAME_VIDEO_PC_VGA_H

#pragma once

#include "ibm8514a.h"

#include "screen.h"


// ======================> vga_device

class vga_device : public device_t
				 , public device_video_interface
				 , public device_palette_interface
				 , public device_memory_interface
{
	friend class ibm8514a_device;

public:
	// construction/destruction
	vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void zero();
	virtual uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void io_map(address_map &map) ATTR_COLD;

	virtual uint8_t mem_r(offs_t offset);
	virtual void mem_w(offs_t offset, uint8_t data);
	virtual uint8_t mem_linear_r(offs_t offset);
	virtual void mem_linear_w(offs_t offset,uint8_t data);

	void set_offset(uint16_t val) { vga.crtc.offset = val; }
	void set_vram_size(size_t vram_size) { vga.svga_intf.vram_size = vram_size; }
	// FIXME: should be protected, but virge_pci.cpp violates this
	inline uint16_t get_crtc_port() { return BIT(vga.miscellaneous_output, 0) ? 0x3d0 : 0x3b0; }

protected:
	enum
	{
		SCREEN_OFF = 0,
		TEXT_MODE,
		VGA_MODE,
		EGA_MODE,
		CGA_MODE,
		MONO_MODE,
		RGB8_MODE,
		RGB15_MODE,
		RGB16_MODE,
		RGB24_MODE,
		RGB32_MODE
	};

	vga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	TIMER_CALLBACK_MEMBER(vblank_timer_cb);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_palette_interface overrides
	virtual uint32_t palette_entries() const noexcept override { return 0x100; }

	void vga_vh_text(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void vga_vh_ega(bitmap_rgb32 &bitmap,  const rectangle &cliprect);
	void vga_vh_vga(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void vga_vh_cga(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void vga_vh_mono(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	virtual uint8_t pc_vga_choosevideomode();
	void recompute_params_clock(int divisor, int xtal);
	virtual void recompute_params();
	uint8_t vga_vblank();

	virtual space_config_vector memory_space_config() const override;

	virtual void io_3bx_3dx_map(address_map &map) ATTR_COLD;

	u8 crtc_address_r(offs_t offset);
	void crtc_address_w(offs_t offset, u8 data);
	virtual u8 crtc_data_r(offs_t offset);
	virtual void crtc_data_w(offs_t offset, u8 data);
	u8 input_status_1_r(offs_t offset);
	void feature_control_w(offs_t offset, u8 data);

	virtual void io_3cx_map(address_map &map) ATTR_COLD;

	u8 atc_address_r(offs_t offset);
	void atc_address_data_w(offs_t offset, u8 data);
	u8 atc_data_r(offs_t offset);
	u8 input_status_0_r(offs_t offset);
	void miscellaneous_output_w(offs_t offset, u8 data);
	u8 sequencer_address_r(offs_t offset);
	void sequencer_address_w(offs_t offset, u8 data);
	virtual u8 sequencer_data_r(offs_t offset);
	virtual void sequencer_data_w(offs_t offset, u8 data);
	u8 ramdac_mask_r(offs_t offset);
	void ramdac_mask_w(offs_t offset, u8 data);
	u8 ramdac_state_r(offs_t offset);
	void ramdac_read_index_w(offs_t offset, u8 data);
	u8 ramdac_write_index_r(offs_t offset);
	void ramdac_write_index_w(offs_t offset, u8 data);
	u8 ramdac_data_r(offs_t offset);
	void ramdac_data_w(offs_t offset, u8 data);
	u8 feature_control_r(offs_t offset);
	u8 miscellaneous_output_r(offs_t offset);
	virtual u8 gc_address_r(offs_t offset);
	virtual void gc_address_w(offs_t offset, u8 data);
	virtual u8 gc_data_r(offs_t offset);
	virtual void gc_data_w(offs_t offset, u8 data);

	virtual void crtc_map(address_map &map) ATTR_COLD;
	virtual void sequencer_map(address_map &map) ATTR_COLD;
	virtual void gc_map(address_map &map) ATTR_COLD;
	virtual void attribute_map(address_map &map) ATTR_COLD;

	// NOTE: do not use the subclassed result when determining pitch in SVGA modes.
	// dw & word mode should apply to normal VGA modes only.
	virtual uint16_t offset();
	virtual uint32_t latch_start_addr() { return vga.crtc.start_addr_latch; }
	virtual uint8_t vga_latch_write(int offs, uint8_t data);
	inline uint8_t rotate_right(uint8_t val) { return (val >> vga.gc.rotate_count) | (val << (8 - vga.gc.rotate_count)); }
	inline uint8_t vga_logical_op(uint8_t data, uint8_t plane, uint8_t mask)
	{
		uint8_t res = 0;

		switch (vga.gc.logical_op & 3)
		{
		case 0: /* NONE */
			res = (data & mask) | (vga.gc.latch[plane] & ~mask);
			break;
		case 1: /* AND */
			res = (data | ~mask) & (vga.gc.latch[plane]);
			break;
		case 2: /* OR */
			res = (data & mask) | (vga.gc.latch[plane]);
			break;
		case 3: /* XOR */
			res = (data & mask) ^ (vga.gc.latch[plane]);
			break;
		}

		return res;
	}
	virtual bool get_interlace_mode() { return false; }
	virtual void palette_update();

	struct vga_t
	{
		vga_t(device_t &owner) { }
		struct
		{
			size_t vram_size;
		} svga_intf;

		std::unique_ptr<uint8_t []> memory;
		uint32_t pens[16]; /* the current 16 pens */

		uint8_t miscellaneous_output;
		uint8_t feature_control;

		struct
		{
			uint8_t index;
			uint8_t data[0x100];
			uint8_t map_mask;
			struct
			{
				uint8_t A, B;
			}char_sel;
		} sequencer;

		/* An empty comment at the start of the line indicates that register is currently unused */
		struct
		{
			uint8_t index;
			uint8_t data[0x100];
			uint16_t horz_total;
			uint16_t horz_disp_end;
	/**/    uint8_t horz_blank_start;
	/**/    uint8_t horz_blank_end;
	/**/    uint8_t horz_retrace_start;
	/**/    uint8_t horz_retrace_skew;
	/**/    uint8_t horz_retrace_end;
	/**/    uint8_t disp_enable_skew;
	/**/    uint8_t evra;
			uint16_t vert_total;
			uint16_t vert_disp_end;
	/**/    uint16_t vert_retrace_start;
	/**/    uint8_t vert_retrace_end;
			uint16_t vert_blank_start;
			uint16_t line_compare;
			uint32_t cursor_addr;
	/**/    uint8_t byte_panning;
			uint8_t preset_row_scan;
			uint8_t scan_doubling;
			uint8_t maximum_scan_line;
			uint8_t cursor_enable;
			uint8_t cursor_scan_start;
	/**/    uint8_t cursor_skew;
			uint8_t cursor_scan_end;
			uint32_t start_addr;
			uint32_t start_addr_latch;
			uint8_t protect_enable;
	/**/    uint8_t bandwidth;
			uint16_t offset;
			uint8_t word_mode;
			uint8_t dw;
	/**/    uint8_t div4;
	/**/    uint8_t underline_loc;
			uint16_t vert_blank_end;
			uint8_t sync_en;
	/**/    uint8_t aw;
			uint8_t div2;
	/**/    uint8_t sldiv;
	/**/    uint8_t map14;
	/**/    uint8_t map13;
	/**/    uint8_t irq_clear;
	/**/    uint8_t irq_disable;
			uint8_t no_wrap;
		} crtc;

		struct
		{
			uint8_t index;
			uint8_t latch[4];
			uint8_t set_reset;
			uint8_t enable_set_reset;
			uint8_t color_compare;
			uint8_t logical_op;
			uint8_t rotate_count;
			uint8_t shift256;
			uint8_t shift_reg;
			uint8_t read_map_sel;
			uint8_t read_mode;
			uint8_t write_mode;
			uint8_t color_dont_care;
			uint8_t bit_mask;
			uint8_t alpha_dis;
			uint8_t memory_map_sel;
			uint8_t host_oe;
			uint8_t chain_oe;
		} gc;

		struct
		{
			uint8_t index, data[0x15]; int state;
			uint8_t prot_bit;
			uint8_t pel_shift;
			uint8_t pel_shift_latch;
		} attribute;

		struct {
			uint8_t read_index, write_index, mask;
			int read;
			int state;
			uint8_t color[0x300]; /* flat RGB triplets */
			int dirty;
		} dac;

		struct {
			uint8_t visible;
		} cursor;

		/* oak vga */
		struct { uint8_t reg; } oak;
	} vga;

	required_ioport m_input_sense;

	emu_timer *m_vblank_timer;

	enum {
		MAIN_IF_REG = 0,
		CRTC_REG,
		GC_REG,
		SEQ_REG,
		ATC_REG,
		// pointer for subclasses to declare further spaces
		EXT_REG
	};

	address_space_config m_main_if_space_config;
	address_space_config m_crtc_space_config;
	address_space_config m_gc_space_config;
	address_space_config m_seq_space_config;
	address_space_config m_atc_space_config;

	bool m_ioas = false;
private:
	uint32_t start_addr();
};


// device type definition
DECLARE_DEVICE_TYPE(VGA, vga_device)

// ======================> svga_device

class svga_device :  public vga_device
{
public:
	virtual void zero() override;
	virtual uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) override;
	uint8_t get_video_depth();

protected:
	// construction/destruction
	svga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	void svga_vh_rgb8(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void svga_vh_rgb15(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void svga_vh_rgb16(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void svga_vh_rgb24(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void svga_vh_rgb32(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	virtual uint8_t pc_vga_choosevideomode() override;
	virtual void device_start() override ATTR_COLD;
	virtual u16 line_compare_mask();
	struct
	{
		uint8_t bank_r, bank_w;
		uint8_t rgb8_en;
		uint8_t rgb15_en;
		uint8_t rgb16_en;
		uint8_t rgb24_en;
		uint8_t rgb32_en;
		uint8_t id;
		bool ignore_chain4;
	} svga;
};


/*
  pega notes (paradise)
  build in amstrad pc1640

  ROM_LOAD("40100", 0xc0000, 0x8000, CRC(d2d1f1ae) SHA1(98302006ee38a17c09bd75504cc18c0649174e33) )

*/

/*
  oak vga (oti 037 chip)
  (below bios patch needed for running)

  ROM_LOAD("oakvga.bin", 0xc0000, 0x8000, CRC(318c5f43) SHA1(2aeb6cf737fd87dfd08c9f5b5bc421fcdbab4ce9) )
*/


#endif // MAME_VIDEO_PC_VGA_H
