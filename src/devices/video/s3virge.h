// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * s3virge.h
 *
 * S3 ViRGE 2D/3D video card
 *
 */

#ifndef MAME_BUS_ISA_S3VIRGE_H
#define MAME_BUS_ISA_S3VIRGE_H

#pragma once

#include "video/pc_vga_s3.h"

// ======================> s3virge_vga_device

class s3virge_vga_device :  public s3trio64_vga_device
{
public:
	// construction/destruction
	s3virge_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto linear_config_changed() { return m_linear_config_changed_cb.bind(); }

	virtual uint8_t mem_r(offs_t offset) override;
	virtual void mem_w(offs_t offset, uint8_t data) override;

	uint8_t fb_r(offs_t offset);
	void fb_w(offs_t offset, uint8_t data);
	uint32_t s3d_sub_status_r();
	void s3d_sub_control_w(uint32_t data);
	uint32_t s3d_func_ctrl_r();
//  void s3d_func_ctrl_w(offs_t offset, uint32_t data, u32 mem_mask = ~0);

	void s3d_register_map(address_map &map);

	void image_xfer(uint32_t data)
	{
//      if(s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_COMMAND] & 0x00000080)
		{
//      logerror("IMG Xfer:(%u):%08x  X:%u(%u) Y:%u(%u)\n",s3virge.s3d.bitblt_step_count,data,s3virge.s3d.bitblt_x_current,s3virge.s3d.bitblt_width,s3virge.s3d.bitblt_y_current,s3virge.s3d.bitblt_height);
		s3virge.s3d.image_xfer = data;
		bitblt_step();
		}
	}

	uint32_t get_linear_address() { return s3virge.linear_address; }
	void set_linear_address(uint32_t addr) { s3virge.linear_address = addr; }
	uint8_t get_linear_address_size() { return s3virge.linear_address_size; }
	uint32_t get_linear_address_size_full() { return s3virge.linear_address_size_full; }
	bool is_linear_address_active() { return s3virge.linear_address_enable; }
	bool is_new_mmio_active() { return s3.cr53 & 0x08; }
	uint16_t src_stride()
	{
		return (s3virge.s3d.dest_src_stride >> 0) & 0xfff8;
	}
	uint16_t dest_stride()
	{
//      if((s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_COMMAND] & 0x0000001c) == 0x08)
//      {
//          popmessage("Stride=%08x",(((s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_DEST_SRC_STR] >> 16) & 0xfff8) / 3)
//              + ((s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_DEST_SRC_STR] >> 16) & 0xfff8));
//          return (((s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_DEST_SRC_STR] >> 16) & 0xfff8) / 3)
//              + ((s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_DEST_SRC_STR] >> 16) & 0xfff8);
//      }
//      else
			return (s3virge.s3d.dest_src_stride >> 16) & 0xfff8;
	}

	ibm8514a_device* get_8514() { fatalerror("s3virge requested non-existent 8514/A device\n"); return nullptr; }

protected:
	s3virge_vga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual uint16_t offset() override;

	virtual void crtc_map(address_map &map) override;

	enum
	{
		LAW_64K = 0,
		LAW_1MB,
		LAW_2MB,
		LAW_4MB
	};

	// register groups
	enum
	{
		OP_BITBLT = 0,
		OP_2DLINE,
		OP_2DPOLY,
		OP_3DLINE,
		OP_3DTRI
	};

	enum
	{
		S3D_STATE_IDLE = 0,
		S3D_STATE_BITBLT,
		S3D_STATE_2DLINE,
		S3D_STATE_2DPOLY,
		S3D_STATE_3DLINE,
		S3D_STATE_3DPOLY
	};

	struct bitblt_struct {
		u32 src_base = 0;
		u32 dest_base = 0;
		u32 clip_l_r = 0;
		u32 clip_t_b = 0;
		u32 dest_src_str = 0;
		u64 mono_pat = 0;
		u32 pat_bg_clr = 0;
		u32 pat_fg_clr = 0;
		u32 src_bg_clr = 0;
		u32 src_fg_clr = 0;
		u32 cmd_set = 0;
		u32 rwidth_height = 0;
		u32 rsrc_xy = 0;
		u32 rdest_xy = 0;
	};

	util::fifo<bitblt_struct, 16> m_bitblt_fifo;
	bitblt_struct m_bitblt_latch; 

	struct
	{
		uint32_t linear_address;
		uint8_t linear_address_size;
		uint32_t linear_address_size_full;
		bool linear_address_enable;
		uint32_t interrupt_enable;

		struct
		{
			int state;
			bool busy;

			uint8_t pattern[0xc0];

			// BitBLT command state
			uint16_t bitblt_x_src;
			uint16_t bitblt_y_src;
			uint16_t bitblt_x_dst;
			uint16_t bitblt_y_dst;
			int16_t bitblt_x_current;
			int16_t bitblt_y_current;
			int16_t bitblt_x_src_current;
			int16_t bitblt_y_src_current;
			int8_t bitblt_pat_x;
			int8_t bitblt_pat_y;
			uint16_t bitblt_height;
			uint16_t bitblt_width;
			uint32_t bitblt_step_count;
			uint64_t bitblt_mono_pattern;
			uint32_t bitblt_current_pixel;
			uint32_t bitblt_pixel_pos;  // current position in a pixel (for packed 24bpp colour image transfers)
			uint32_t image_xfer;  // source data via image transfer ports
			uint16_t clip_l;
			uint16_t clip_r;
			uint16_t clip_t;
			uint16_t clip_b;
			uint32_t command;
			uint32_t src_base;
			uint32_t dest_base;
			uint32_t pat_bg_clr;
			uint32_t pat_fg_clr;
			uint32_t src_bg_clr;
			uint32_t src_fg_clr;
			uint32_t dest_src_stride;
		} s3d;
		uint8_t cr66;
	} s3virge;

	TIMER_CALLBACK_MEMBER(draw_step_tick);
	TIMER_CALLBACK_MEMBER(command_timer_cb);

	inline void write_pixel32(uint32_t base, uint16_t x, uint16_t y, uint32_t val);
	inline void write_pixel24(uint32_t base, uint16_t x, uint16_t y, uint32_t val);
	inline void write_pixel16(uint32_t base, uint16_t x, uint16_t y, uint16_t val);
	inline void write_pixel8(uint32_t base, uint16_t x, uint16_t y, uint8_t val);
	inline uint32_t read_pixel32(uint32_t base, uint16_t x, uint16_t y, u16 stride_select);
	inline uint32_t read_pixel24(uint32_t base, uint16_t x, uint16_t y, u16 stride_select);
	inline uint16_t read_pixel16(uint32_t base, uint16_t x, uint16_t y, u16 stride_select);
	inline uint8_t read_pixel8(uint32_t base, uint16_t x, uint16_t y, u16 stride_select);

	uint32_t GetROP(uint8_t rop, uint32_t src, uint32_t dst, uint32_t pat);
	bool advance_pixel();

	devcb_write_line m_linear_config_changed_cb;

	virtual void s3_define_video_mode(void) override;

	// has no 8514/A device
private:
	emu_timer* m_draw_timer;
	emu_timer* m_cmd_timer;
	void bitblt_step();
	void bitblt_colour_step();
	void bitblt_monosrc_step();
	void line2d_step();
	void poly2d_step();
	void line3d_step();
	void poly3d_step();
	void add_command(u8 cmd_type);
	void command_finish();

	void s3d_reset();
};


// ======================> s3virgedx_vga_device

class s3virgedx_vga_device :  public s3virge_vga_device
{
public:
	// construction/destruction
	s3virgedx_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	s3virgedx_vga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
};

// ======================> s3virgedx_vga_device

class s3virgedx_rev1_vga_device :  public s3virgedx_vga_device
{
public:
	// construction/destruction
	s3virgedx_rev1_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
};

// device type definition
DECLARE_DEVICE_TYPE(S3VIRGE,    s3virge_vga_device)
DECLARE_DEVICE_TYPE(S3VIRGEDX,  s3virgedx_vga_device)
DECLARE_DEVICE_TYPE(S3VIRGEDX1, s3virgedx_rev1_vga_device)

#endif // MAME_BUS_ISA_S3VIRGE_H
