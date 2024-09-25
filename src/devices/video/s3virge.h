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

	void s3d_register_map(address_map &map) ATTR_COLD;

	void streams_control_map(address_map &map) ATTR_COLD;

	void image_xfer(offs_t offset, uint32_t data, uint32_t mem_mask)
	{
		if (mem_mask != 0xffff'ffff)
			logerror("Warning: image_xfer access with non-32 parallelism %08x & %08x\n", data, mem_mask);

		m_xfer_fifo.enqueue(data);
		//machine().scheduler().synchronize();
	}

	uint32_t get_linear_address() { return m_linear_address; }
	void set_linear_address(uint32_t addr) { m_linear_address = addr; }
	uint8_t get_linear_address_size() { return m_linear_address_size; }
	uint32_t get_linear_address_size_full() { return m_linear_address_size_full; }
	bool is_linear_address_active() { return m_linear_address_enable; }
	bool is_new_mmio_active() { return s3.cr53 & 0x08; }

	// has no 8514/A device
	// FIXME: should map this dependency in machine_config
	ibm8514a_device* get_8514() { fatalerror("s3virge requested non-existent 8514/A device\n"); return nullptr; }

protected:
	s3virge_vga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual uint16_t offset() override;

	virtual void crtc_map(address_map &map) override ATTR_COLD;

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

	enum s3d_state_t
	{
		S3D_STATE_IDLE = 0,
		S3D_STATE_COMMAND_RX,
		S3D_STATE_BITBLT,
		S3D_STATE_2DLINE,
		S3D_STATE_2DPOLY,
		S3D_STATE_3DLINE,
		S3D_STATE_3DPOLY
	};

	struct {
		u8 psidf = 0;
		u8 pshfc = 0;
		u16 primary_stride = 0;
	} m_streams;

	u32 m_interrupt_enable = 0;

	bool m_linear_address_enable = false;
	u32 m_linear_address = 0;
	u8 m_linear_address_size = 0;
	u32 m_linear_address_size_full = 0;

	u8 m_cr66 = 0;

//  util::fifo<u32, 16 * 15> m_bitblt_fifo;
	// TODO: sketchy, command pipeline size is unclear
	util::fifo<u32, 0x8000> m_bitblt_fifo;
	util::fifo<u32, 0x8000> m_xfer_fifo;
	// TODO: sketchy type, verify implications of using a struct class with util::fifo
	// (may be required if we want to glue in a "execute command" insert flag)
	u32 m_bitblt_latch[15]{};
	s3d_state_t m_s3d_state = S3D_STATE_IDLE;

	struct
	{
		bool xfer_mode = false;

		u8 pattern[0xc0]{};

		u16 x_src = 0;
		u16 y_src = 0;
		u16 x_dst = 0;
		u16 y_dst = 0;
		s16 x_current = 0;
		s16 y_current = 0;
		s16 x_src_current = 0;
		s16 y_src_current = 0;
		s8 pat_x = 0;
		s8 pat_y = 0;
		u16 height = 0;
		u16 width = 0;
		u32 step_count = 0;
		u64 mono_pattern = 0;
		u32 current_pixel = 0;
		// current position in a pixel (for packed 24bpp colour image transfers)
		u32 pixel_pos = 0;
		// source data via image transfer ports
		u32 image_xfer = 0;
		u16 clip_l = 0;
		u16 clip_r = 0;
		u16 clip_t = 0;
		u16 clip_b = 0;
		u32 command = 0;
		u32 src_base = 0;
		u32 dest_base = 0;
		u32 pat_bg_clr = 0;
		u32 pat_fg_clr = 0;
		u32 src_bg_clr = 0;
		u32 src_fg_clr = 0;
		u16 dest_stride = 0;
		u16 src_stride = 0;
	} m_bitblt;

	TIMER_CALLBACK_MEMBER(op_timer_cb);

	inline void write_pixel24(uint32_t base, uint16_t x, uint16_t y, uint32_t val);
	inline void write_pixel16(uint32_t base, uint16_t x, uint16_t y, uint16_t val);
	inline void write_pixel8(uint32_t base, uint16_t x, uint16_t y, uint8_t val);
	inline uint32_t read_pixel24(uint32_t base, uint16_t x, uint16_t y, u16 stride_select);
	inline uint16_t read_pixel16(uint32_t base, uint16_t x, uint16_t y, u16 stride_select);
	inline uint8_t read_pixel8(uint32_t base, uint16_t x, uint16_t y, u16 stride_select);

	uint32_t GetROP(uint8_t rop, uint32_t src, uint32_t dst, uint32_t pat);
	bool advance_pixel();

	devcb_write_line m_linear_config_changed_cb;

	virtual void s3_define_video_mode(void) override;

private:
	emu_timer *m_op_timer;
	void bitblt_step();
	void bitblt_colour_step();
	void bitblt_monosrc_step();
	void line2d_step();
	void poly2d_step();
	void line3d_step();
	void poly3d_step();
	void add_command(u8 cmd_type);
	void command_enqueue(u8 op_type);
	void command_dequeue(u8 op_type);
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
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
};

// ======================> s3virgedx_vga_device

class s3virgedx_rev1_vga_device :  public s3virgedx_vga_device
{
public:
	// construction/destruction
	s3virgedx_rev1_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
};

// device type definition
DECLARE_DEVICE_TYPE(S3VIRGE,    s3virge_vga_device)
DECLARE_DEVICE_TYPE(S3VIRGEDX,  s3virgedx_vga_device)
DECLARE_DEVICE_TYPE(S3VIRGEDX1, s3virgedx_rev1_vga_device)

#endif // MAME_BUS_ISA_S3VIRGE_H
