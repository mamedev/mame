// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay
#ifndef MAME_BUS_INTERPRO_SR_GT_H
#define MAME_BUS_INTERPRO_SR_GT_H

#pragma once

#include "sr.h"
#include "screen.h"
#include "video/bt459.h"
#include "video/dp8510.h"
#include "machine/ram.h"
#include "machine/z80scc.h"

class gt_device_base : public device_t
{
protected:
	gt_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, const bool double_buffered, const bool masked_reads);

	virtual void map(address_map &map) ATTR_COLD;

public:
	static constexpr u32 GT_MASK_BITS   = 0x80808080; // mask RAM presents on high bit in each pixel/byte

	// FIXME: enable delays to pass diagnostic tests
	static constexpr bool GT_DIAG = false;

	enum control_mask
	{
		GFX_VERT_BLNK               = 0x00000001,
		GFX_BUF1_SEL                = 0x00000002, // write to buffer 1
		GFX_HILITE_SEL              = 0x00000004, // select highlight ram
		GFX_SOFT_BLNK               = 0x00000008,
		GFX_DRAW_FIRST              = 0x00000010, // draw first point
		GFX_BLIT_DIR                = 0x00000020, // bitblt addresses decrement
		GFX_RMW_MD                  = 0x00000040, // enable rmw mode
		GFX_DATA_SEL                = 0x00000080, // enable plane data as source
		GFX_ANTI_MD                 = 0x00000100, // enable antialias mode
		GFX_ADD_MD                  = 0x00000200, // enable antialias additive mode
		GFX_MASK_ENA                = 0x00000400, // enable pixel write mask
		GFX_MASK_SEL                = 0x00000800, // select mask ram
		GFX_SCR1_SEL                = 0x00001000, // select screen 1
		GFX_BSGA_RST                = 0x00002000,
		GFX_BLIT_BUSY               = 0x00004000,
		GFX_GRPHCS_BUSY             = 0x00008000,
		GFX_VFIFO_EMPTY             = 0x00010000,
		GFX_SCREEN0_DISP_BUF1       = 0x00020000, // select buffer 1 (screen 0)
		GFX_SCREEN1_DISP_BUF1       = 0x00040000, // select buffer 1 (screen 1)
		GFX_STEREO_EN               = 0x00080000,
		GFX_INTERLACE               = 0x00100000,
		GFX_STEREO_POLRITY_NORMAL   = 0x00200000,
		GFX_STEREO_GLASSES_EN       = 0x00400000,
		GFX_FIELD_1                 = 0x00800000,
		GFX_MASK_READ_ENA           = 0x01000000, // enable pixel read mask?
		GFX_MONSENSE_MASK           = 0x0e000000,
		GFX_MONSENSE_60HZ           = 0x0e000000,
	};

	u32 control_r() { return m_control | (m_screen[0]->vblank() ? GFX_VERT_BLNK : 0); }
	void control_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	void contrast_dac_w(u8 data);

	void blit_src_address_w(offs_t offset, u32 data, u32 mem_mask = ~0) { COMBINE_DATA(&m_blit_src_address); }
	void blit_dst_address_w(offs_t offset, u32 data, u32 mem_mask = ~0) { COMBINE_DATA(&m_blit_dst_address); }
	void blit_width_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	u8 plane_enable_r() { return m_plane_enable; }
	void plane_enable_w(u8 data);
	u8 plane_data_r() { return m_plane_data; }
	void plane_data_w(u8 data);

	u16 bsga_width_r() { return m_bsga_width; }
	void bsga_width_w(offs_t offset, u16 data, u16 mem_mask = ~0) { COMBINE_DATA(&m_bsga_width); }
	u16 bsga_tmp_r() { return m_bsga_tmp; }
	void bsga_tmp_w(offs_t offset, u16 data, u16 mem_mask = ~0) { COMBINE_DATA(&m_bsga_tmp); }

	u16 bsga_xmin_r() { return m_bsga_xmin; }
	void bsga_xmin_w(offs_t offset, u16 data, u16 mem_mask = ~0) { COMBINE_DATA(&m_bsga_xmin); }
	u16 bsga_ymin_r() { return m_bsga_ymin; }
	void bsga_ymin_w(offs_t offset, u16 data, u16 mem_mask = ~0) { COMBINE_DATA(&m_bsga_ymin); }

	u16 bsga_acc0_r() { return (m_bsga_width - m_bsga_xin1); }
	u16 bsga_acc1_r() { return -(m_bsga_width - m_bsga_xin1); }

	u16 bsga_xmax_r() { return m_bsga_xmax; }
	void bsga_xmax_w(offs_t offset, u16 data, u16 mem_mask = ~0) { COMBINE_DATA(&m_bsga_xmax); }
	u16 bsga_ymax_r() { return m_bsga_ymax; }
	void bsga_ymax_w(offs_t offset, u16 data, u16 mem_mask = ~0) { COMBINE_DATA(&m_bsga_ymax); bsga_clip_status(m_bsga_xin1, m_bsga_yin1); }

	u16 bsga_src0_r() { return m_bsga_xin1; }
	u16 bsga_src1_r() { return m_bsga_xin1; }

	void bsga_xin1_w(offs_t offset, u16 data, u16 mem_mask = ~0) { COMBINE_DATA(&m_bsga_xin1); m_bsga_xin = m_bsga_xin1; m_bsga_tmp = m_bsga_xin1; }
	void bsga_yin1_w(offs_t offset, u16 data, u16 mem_mask = ~0) { COMBINE_DATA(&m_bsga_yin1); m_bsga_yin = m_bsga_yin1; }
	void bsga_xin1yin1_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	enum bsga_status_mask : u16
	{
		STATUS_DATA_VALID  = 0x0001,
		STATUS_COMPLETE    = 0x0002,
		STATUS_ACCEPT      = 0x0004,
		STATUS_REJECT      = 0x0008,
		STATUS_FLOAT_OFLOW = 0x0010,
		STATUS_LEFT        = 0x0020,
		STATUS_RIGHT       = 0x0040,
		STATUS_ABOVE       = 0x0080,
		STATUS_BELOW       = 0x0100,
		STATUS_PREV_LEFT   = 0x0200,
		STATUS_PREV_RIGHT  = 0x0400,
		STATUS_PREV_ABOVE  = 0x0800,
		STATUS_PREV_BELOW  = 0x1000,

		STATUS_CLIP0_MASK  = 0x1e00,
		STATUS_CLIP1_MASK  = 0x01e0,
		STATUS_CLIP_MASK   = 0x1fe0
	};
	u16 bsga_status_r();

	void bsga_xin2yin2_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	void bsga_xin2_w(offs_t offset, u16 data, u16 mem_mask = ~0) { COMBINE_DATA(&m_bsga_xin2); m_bsga_xin = m_bsga_xin2; }
	void bsga_yin2_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	// FIXME: perhaps tmp is a counter of xin/yin and can be used to return correct value?
	u16 bsga_xin_r() { return m_bsga_xin; }
	u16 bsga_yin_r() { return m_bsga_yin; }

	void ri_initial_distance_w(offs_t offset, u32 data, u32 mem_mask = ~0) { COMBINE_DATA(&m_ri_initial_distance); }
	void ri_distance_both_w(offs_t offset, u32 data, u32 mem_mask = ~0) { COMBINE_DATA(&m_ri_distance_both); }
	void ri_distance_major_w(offs_t offset, u32 data, u32 mem_mask = ~0) { COMBINE_DATA(&m_ri_distance_major); }
	void ri_initial_address_w(offs_t offset, u32 data, u32 mem_mask = ~0) { COMBINE_DATA(&m_ri_initial_address); }
	void ri_address_both_w(offs_t offset, u32 data, u32 mem_mask = ~0) { COMBINE_DATA(&m_ri_address_both); }
	void ri_address_major_w(offs_t offset, u32 data, u32 mem_mask = ~0) { COMBINE_DATA(&m_ri_address_major); }
	void ri_initial_error_w(offs_t offset, u32 data, u32 mem_mask = ~0) { COMBINE_DATA(&m_ri_initial_error); }
	void ri_error_both_w(offs_t offset, u32 data, u32 mem_mask = ~0) { COMBINE_DATA(&m_ri_error_both); }
	void ri_error_major_w(offs_t offset, u32 data, u32 mem_mask = ~0) { COMBINE_DATA(&m_ri_error_major); }

	void ri_stop_count_w(offs_t offset, u32 data, u32 mem_mask = ~0) { COMBINE_DATA(&m_ri_stop_count); }
	void ri_control_w(offs_t offset, u32 data, u32 mem_mask = ~0) { COMBINE_DATA(&m_ri_control); }
	void ri_xfer_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	void bsga_float_w(offs_t offset, u32 data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_validity_check(validity_checker &valid) const override;

	u32 buffer_r(const offs_t offset);
	void buffer_w(const offs_t offset, u32 data, u32 mem_mask);
	virtual u32 vram_r(offs_t offset, const bool linear = false) const;
	virtual void vram_w(offs_t offset, const u32 data, u32 mem_mask, const bool linear = false) const;
	u32 mram_r(const offs_t offset) const;
	void mram_w(const offs_t offset, const u32 data, const u32 mem_mask) const;

	template <int N> u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
	{
		m_ramdac[N]->screen_update(screen, bitmap, cliprect, m_double_buffered && (m_control & (N == 0 ? GFX_SCREEN0_DISP_BUF1 : GFX_SCREEN1_DISP_BUF1)) ?
			m_vram[N]->pointer() + (m_vram[N]->size() >> 1) :
			m_vram[N]->pointer());

		return 0;
	}

	struct bpu_pair_t
	{
		u16 hi;
		u16 lo;
	};

	TIMER_CALLBACK_MEMBER(blit);
	TIMER_CALLBACK_MEMBER(line);
	TIMER_CALLBACK_MEMBER(done);

	void bsga_clip_status(s16 xin, s16 yin);
	bool kuzmin_clip(s16 sx1, s16 sy1, s16 sx2, s16 sy2, s16 wx1, s16 wy1, s16 wx2, s16 wy2);
	void bresenham_line(s16 major, s16 minor, s16 major_step, s16 minor_step, int steps, s16 error, s16 error_major, s16 error_minor, bool shallow);

	void bpu_control_w(const u32 data);
	void bpu_source_w(const u32 data, const bool fifo_write = true);
	void bpu_destination_w(const u32 data, const bool fifo_write = false);
	u32 bpu_output_r();
	void bpu_reset();
	void bpu_barrel_input_select(const int state);
	void bpu_left_mask_enable(const int state);
	void bpu_right_mask_enable(const int state);
	bpu_pair_t bpu_from_u32(const u32 data) const;
	u32 bpu_to_u32(bpu_pair_t data) const;

	// sub-devices
	optional_device_array<screen_device, 2> m_screen;
	optional_device_array<bt459_device, 2> m_ramdac;
	optional_device_array<ram_device, 2> m_vram;
	optional_device_array<ram_device, 2> m_mram;
	required_device_array<dp8510_device, 2> m_bpu;

	// device state
	u32 m_control;

	u32 m_blit_src_address;
	u32 m_blit_dst_address;
	u16 m_blit_width;

	u32 m_plane_enable;
	u32 m_plane_data;

	u16 m_bsga_width;
	u16 m_bsga_tmp;

	u16 m_bsga_xmin; // clipping boundary minimum x (left)
	u16 m_bsga_ymin; // clipping boundary minimum y (top)
	u16 m_bsga_xmax; // clipping boundary maximum x (right)
	u16 m_bsga_ymax; // clipping boundary maximum y (bottom)

	u16 m_bsga_status;

	u16 m_bsga_xin1;
	u16 m_bsga_yin1;
	u16 m_bsga_xin2;
	u16 m_bsga_yin2;

	u16 m_bsga_xin; // most recently written xin1/xin2
	u16 m_bsga_yin; // most recently written yin1/yin2

	u32 m_ri_initial_distance;
	u32 m_ri_distance_both;
	u32 m_ri_distance_major;
	u32 m_ri_initial_address;
	u32 m_ri_address_both;
	u32 m_ri_address_major;
	u32 m_ri_initial_error;
	u32 m_ri_error_both;
	u32 m_ri_error_major;
	u32 m_ri_stop_count;
	u32 m_ri_control;
	u32 m_ri_xfer;

private:
	emu_timer *m_blit_timer;
	emu_timer *m_line_timer;
	emu_timer *m_done_timer;

	const bool m_double_buffered;
	const bool m_masked_reads;
};

class gt_device : public gt_device_base, public device_cbus_card_interface
{
protected:
	gt_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, const bool double_buffered);

	virtual void map(address_map &map) override ATTR_COLD;
};

class gtdb_device : public gt_device_base, public device_srx_card_interface
{
protected:
	gtdb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map &map) override ATTR_COLD;
	virtual void map_dynamic(address_map &map) ATTR_COLD;

	void serial_irq(int state);
	void mouse_status_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void srx_mapping_w(u32 data);

	enum int_status_mask
	{
		MOUSE_BTN = 0x08,
		// MOUSE_MOVED? = 0x10,
		MOUSE_X   = 0x20,
		MOUSE_Y   = 0x40,
		SERIAL    = 0x80,
	};
	u32 mouse_int_r() { return m_mouse_int; }
	void mouse_int_w(offs_t offset, u32 data, u32 mem_mask = ~0) { mem_mask &= ~0x7; COMBINE_DATA(&m_mouse_int); }
	u32 mouse_x_r();
	u32 mouse_y_r();

	enum vfifo_control_mask
	{
		FIFO_LW_ENB  = 0x08,
		FIFO_LW_INTR = 0x40,
		FIFO_HW_INTR = 0x80,
	};
	u32 fifo_control_r() { return m_fifo_control; }

	//virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual u32 vram_r(offs_t offset, const bool linear = false) const override;
	virtual void vram_w(offs_t offset, const u32 data, u32 mem_mask, const bool linear = false) const override;

	optional_device_array<ram_device, 2> m_hram;
	required_device<z80scc_device> m_scc;

private:
	u8 m_mouse_int;
	u32 m_fifo_control;

	u8 m_mouse_x;
	u8 m_mouse_y;
};

class mpcb963_device : public gt_device
{
public:
	mpcb963_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class mpcba79_device : public gt_device
{
public:
	mpcba79_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


class msmt070_device : public gt_device
{
public:
	msmt070_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class msmt071_device : public gt_device
{
public:
	msmt071_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class msmt081_device : public gt_device
{
public:
	msmt081_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class mpcbb68_device : public gtdb_device
{
public:
	mpcbb68_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class mpcbb92_device : public gtdb_device
{
public:
	mpcbb92_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

// device type definition
DECLARE_DEVICE_TYPE(MPCB963, mpcb963_device)
DECLARE_DEVICE_TYPE(MPCBA79, mpcba79_device)
DECLARE_DEVICE_TYPE(MSMT070, msmt070_device)
DECLARE_DEVICE_TYPE(MSMT071, msmt071_device)
DECLARE_DEVICE_TYPE(MSMT081, msmt081_device)
DECLARE_DEVICE_TYPE(MPCBB68, mpcbb68_device)
DECLARE_DEVICE_TYPE(MPCBB92, mpcbb92_device)

#endif // MAME_BUS_INTERPRO_SR_GT_H
