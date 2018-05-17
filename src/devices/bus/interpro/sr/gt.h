// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay
#ifndef MAME_BUS_INTERPRO_SR_GT_H
#define MAME_BUS_INTERPRO_SR_GT_H

#pragma once

#include "sr.h"
#include "screen.h"
#include "video/bt459.h"
#include "video/dp8510.h"

class gt_device_base : public device_t
{
protected:
	gt_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map &map);

public:
	static const int GT_BUFFER_SIZE = 0x100000;   // 1 megabyte
	static const u32 GT_BUFFER_MASK = (GT_BUFFER_SIZE - 1);

	// FIXME: enable delays to pass diagnostic tests
	static const bool GT_DIAG = false;

	enum control_mask
	{
		GFX_VERT_BLNK               = 0x00000001,
		GFX_HILITE_SEL              = 0x00000004,
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
		//GFX_MASK_READ               = 0x01000000, // enable pixel read mask?
		GFX_MONSENSE_MASK           = 0x0e000000,
		GFX_MONSENSE_60HZ           = 0x0e000000
	};

	DECLARE_READ32_MEMBER(control_r) { return m_control | (get_gt(0).screen->vblank() ? GFX_VERT_BLNK : 0); }
	DECLARE_WRITE32_MEMBER(control_w);

	DECLARE_READ8_MEMBER(contrast_dac_r) { return m_contrast_dac; }
	DECLARE_WRITE8_MEMBER(contrast_dac_w) { m_contrast_dac = data; }

	DECLARE_READ32_MEMBER(blit_src_address_r) { return m_blit_src_address; }
	DECLARE_WRITE32_MEMBER(blit_src_address_w) { COMBINE_DATA(&m_blit_src_address); }
	DECLARE_READ32_MEMBER(blit_dst_address_r) { return m_blit_dst_address; }
	DECLARE_WRITE32_MEMBER(blit_dst_address_w) { COMBINE_DATA(&m_blit_dst_address); }
	DECLARE_READ16_MEMBER(blit_width_r) { return m_blit_width; }
	DECLARE_WRITE16_MEMBER(blit_width_w);

	enum blit_control_mask
	{
		BLIT0_CONTROL_FS = 0x000000f0,
		BLIT1_CONTROL_FS = 0x0000000f,
		BLIT0_CONTROL_SN = 0x0000f000,
		BLIT1_CONTROL_SN = 0x00000f00,
		BLIT0_CONTROL_LM = 0x00f00000,
		BLIT1_CONTROL_LM = 0x000f0000,
		BLIT0_CONTROL_RM = 0xf0000000,
		BLIT1_CONTROL_RM = 0x0f000000
	};
	virtual DECLARE_WRITE32_MEMBER(blit_control_w) = 0;

	DECLARE_READ8_MEMBER(plane_enable_r) { return m_plane_enable; }
	DECLARE_WRITE8_MEMBER(plane_enable_w);
	DECLARE_READ8_MEMBER(plane_data_r) { return m_plane_data; }
	DECLARE_WRITE8_MEMBER(plane_data_w);

	DECLARE_READ16_MEMBER(bsga_width_r) { return m_bsga_width; }
	DECLARE_WRITE16_MEMBER(bsga_width_w) { COMBINE_DATA(&m_bsga_width); }
	DECLARE_READ16_MEMBER(bsga_tmp_r) { return m_bsga_tmp; }
	DECLARE_WRITE16_MEMBER(bsga_tmp_w) { COMBINE_DATA(&m_bsga_tmp); }

	DECLARE_READ16_MEMBER(bsga_xmin_r) { return m_bsga_xmin; }
	DECLARE_WRITE16_MEMBER(bsga_xmin_w) { COMBINE_DATA(&m_bsga_xmin); logerror("xmin = %04x\n", m_bsga_xmin); }
	DECLARE_READ16_MEMBER(bsga_ymin_r) { return m_bsga_ymin; }
	DECLARE_WRITE16_MEMBER(bsga_ymin_w) { COMBINE_DATA(&m_bsga_ymin); logerror("ymin = %04x\n", m_bsga_ymin); }

	DECLARE_READ16_MEMBER(bsga_acc0_r) { return (m_bsga_width - m_bsga_xin1); }
	DECLARE_READ16_MEMBER(bsga_acc1_r) { return -(m_bsga_width - m_bsga_xin1); }

	DECLARE_READ16_MEMBER(bsga_xmax_r) { return m_bsga_xmax; }
	DECLARE_WRITE16_MEMBER(bsga_xmax_w) { COMBINE_DATA(&m_bsga_xmax); logerror("xmax = %04x\n", m_bsga_xmax); }
	DECLARE_READ16_MEMBER(bsga_ymax_r) { return m_bsga_ymax; }
	DECLARE_WRITE16_MEMBER(bsga_ymax_w) { COMBINE_DATA(&m_bsga_ymax); logerror("ymax = %04x\n", m_bsga_ymax); bsga_clip_status(m_bsga_xin1, m_bsga_yin1); }

	DECLARE_READ16_MEMBER(bsga_src0_r) { return m_bsga_xin1; }
	DECLARE_READ16_MEMBER(bsga_src1_r) { return m_bsga_xin1; }

	DECLARE_WRITE16_MEMBER(bsga_xin1_w) { COMBINE_DATA(&m_bsga_xin1); m_bsga_xin = m_bsga_xin1; m_bsga_tmp = m_bsga_xin1; logerror("xin = %04x\n", m_bsga_xin1); }
	DECLARE_WRITE16_MEMBER(bsga_yin1_w) { COMBINE_DATA(&m_bsga_yin1); m_bsga_yin = m_bsga_yin1; logerror("yin = %04x\n", m_bsga_yin1); }
	DECLARE_WRITE32_MEMBER(bsga_xin1yin1_w);

	enum bsga_status_mask
	{
		STATUS_CLIP0_YMAX = 0x1000, // y1 > max y
		STATUS_CLIP0_YMIN = 0x0800, // y1 < min y
		STATUS_CLIP0_XMAX = 0x0400, // x1 > max x
		STATUS_CLIP0_XMIN = 0x0200, // x1 < min x
		STATUS_CLIP1_YMAX = 0x0100, // y2 > max y
		STATUS_CLIP1_YMIN = 0x0080, // y2 < min y
		STATUS_CLIP1_XMAX = 0x0040, // x2 > max x
		STATUS_CLIP1_XMIN = 0x0020, // x2 < min x

		STATUS_FLOAT_OFLOW = 0x0010,

		STATUS_CLIP_BOTH = 0x0008, // set if both inputs fall outside clipping region
		STATUS_CLIP_ANY = 0x0004, // set if any input falls outside clipping region

		STATUS_CLIP0_MASK = 0x1e00, // x1,y1 clip result
		STATUS_CLIP1_MASK = 0x01e0, // x2,y2 clip result
		STATUS_CLIP_MASK = 0x1fe0  // both clip results
	};
	DECLARE_READ16_MEMBER(bsga_status_r);

	DECLARE_WRITE32_MEMBER(bsga_xin2yin2_w);

	DECLARE_WRITE16_MEMBER(bsga_xin2_w) { COMBINE_DATA(&m_bsga_xin2); m_bsga_xin = m_bsga_xin2; }
	DECLARE_WRITE16_MEMBER(bsga_yin2_w);

	// FIXME: perhaps tmp is a counter of xin/yin and can be used to return correct value?
	DECLARE_READ16_MEMBER(bsga_xin_r) { return m_bsga_xin; }
	DECLARE_READ16_MEMBER(bsga_yin_r) { return m_bsga_yin; }

	DECLARE_READ32_MEMBER(ri_initial_distance_r) { return m_ri_initial_distance; }
	DECLARE_WRITE32_MEMBER(ri_initial_distance_w) { COMBINE_DATA(&m_ri_initial_distance); }
	DECLARE_READ32_MEMBER(ri_distance_both_r) { return m_ri_distance_both; }
	DECLARE_WRITE32_MEMBER(ri_distance_both_w) { COMBINE_DATA(&m_ri_distance_both); }
	DECLARE_READ32_MEMBER(ri_distance_major_r) { return m_ri_distance_major; }
	DECLARE_WRITE32_MEMBER(ri_distance_major_w) { COMBINE_DATA(&m_ri_distance_major); }
	DECLARE_READ32_MEMBER(ri_initial_address_r) { return m_ri_initial_address; }
	DECLARE_WRITE32_MEMBER(ri_initial_address_w) { COMBINE_DATA(&m_ri_initial_address); }
	DECLARE_READ32_MEMBER(ri_address_both_r) { return m_ri_address_both; }
	DECLARE_WRITE32_MEMBER(ri_address_both_w) { COMBINE_DATA(&m_ri_address_both); }
	DECLARE_READ32_MEMBER(ri_address_major_r) { return m_ri_address_major; }
	DECLARE_WRITE32_MEMBER(ri_address_major_w) { COMBINE_DATA(&m_ri_address_major); }
	DECLARE_READ32_MEMBER(ri_initial_error_r) { return m_ri_initial_error; }
	DECLARE_WRITE32_MEMBER(ri_initial_error_w) { COMBINE_DATA(&m_ri_initial_error); }
	DECLARE_READ32_MEMBER(ri_error_both_r) { return m_ri_error_both; }
	DECLARE_WRITE32_MEMBER(ri_error_both_w) { COMBINE_DATA(&m_ri_error_both); }
	DECLARE_READ32_MEMBER(ri_error_major_r) { return m_ri_error_major; }
	DECLARE_WRITE32_MEMBER(ri_error_major_w) { COMBINE_DATA(&m_ri_error_major); }

	DECLARE_READ32_MEMBER(ri_stop_count_r) { return m_ri_stop_count; }
	DECLARE_WRITE32_MEMBER(ri_stop_count_w) { COMBINE_DATA(&m_ri_stop_count); }
	DECLARE_READ32_MEMBER(ri_control_r) { return m_ri_control; }
	DECLARE_WRITE32_MEMBER(ri_control_w) { COMBINE_DATA(&m_ri_control); }
	DECLARE_READ32_MEMBER(ri_xfer_r) { return m_ri_xfer; }
	DECLARE_WRITE32_MEMBER(ri_xfer_w);

	DECLARE_WRITE32_MEMBER(bsga_float_w);

protected:
	virtual void device_start() override;

	typedef struct
	{
		required_device<screen_device> screen;
		required_device<bt459_device> ramdac;
		required_device<dp8510_device> bpu;

		std::unique_ptr<u8[]> buffer;
		std::unique_ptr<u8[]> mask;
	}
	gt_t;

	virtual const int get_screen_count() const = 0;
	virtual gt_t &get_gt(const int number) = 0;
	virtual const gt_t &active_gt() const = 0;

	u32 buffer_read(const gt_t &gt, const offs_t offset) const;
	void buffer_write(const gt_t &gt, const offs_t offset, const u32 data, const u32 mask);

	TIMER_CALLBACK_MEMBER(blit);
	TIMER_CALLBACK_MEMBER(line);
	TIMER_CALLBACK_MEMBER(done);

	void bsga_clip_status(s16 xin, s16 yin);

	bool kuzmin_clip(s16 sx1, s16 sy1, s16 sx2, s16 sy2, s16 wx1, s16 wy1, s16 wx2, s16 wy2);
	void bresenham_line(s16 major, s16 minor, s16 major_step, s16 minor_step, int steps, s16 error, s16 error_major, s16 error_minor, bool shallow);
	void write_vram(const gt_t &gt, const offs_t offset, const u8 data);

	u8 m_contrast_dac;

	u32 m_control;

	u32 m_blit_src_address;
	u32 m_blit_dst_address;
	u16 m_blit_width;

	u8 m_plane_enable;
	u8 m_plane_data;

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

	emu_timer *m_blit_timer;
	emu_timer *m_line_timer;
	emu_timer *m_done_timer;
};

class single_gt_device_base : public gt_device_base
{
protected:
	single_gt_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map &map) override;

	virtual DECLARE_WRITE32_MEMBER(blit_control_w) override;

	DECLARE_READ32_MEMBER(buffer_r) { return buffer_read(m_gt[0], offset); }
	DECLARE_WRITE32_MEMBER(buffer_w) { buffer_write(m_gt[0], offset, data, mem_mask); }

	virtual const int get_screen_count() const override { return GT_SCREEN_COUNT; }
	virtual gt_t &get_gt(const int number) override { return m_gt[number]; }
	virtual const gt_t &active_gt() const override { return m_gt[0]; }

	u32 screen_update0(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

private:
	static const int GT_SCREEN_COUNT = 1;

	gt_t m_gt[GT_SCREEN_COUNT];
};

class dual_gt_device_base : public gt_device_base
{
protected:
	dual_gt_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map &map) override;

	virtual DECLARE_WRITE32_MEMBER(blit_control_w) override;

	DECLARE_READ32_MEMBER(buffer_r) { return buffer_read(m_gt[(offset & 0x80000) ? 1 : 0], offset); }
	DECLARE_WRITE32_MEMBER(buffer_w) { buffer_write(m_gt[(offset & 0x80000) ? 1 : 0], offset, data, mem_mask); }

	virtual const int get_screen_count() const override { return GT_SCREEN_COUNT; }
	virtual gt_t &get_gt(const int number) override { return m_gt[number]; }
	virtual const gt_t &active_gt() const override { return m_gt[(m_control & GFX_SCR1_SEL) ? 1 : 0]; }

	u32 screen_update0(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	u32 screen_update1(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

private:
	static const int GT_SCREEN_COUNT = 2;

	gt_t m_gt[GT_SCREEN_COUNT];
};

class mpcb963_device : public single_gt_device_base, public cbus_card_device_base
{
public:
	mpcb963_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override { gt_device_base::device_start(); set_bus_device(); }

	virtual void map(address_map &map) override { cbus_card_device_base::map(map); single_gt_device_base::map(map); }
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
};

class mpcba79_device : public dual_gt_device_base, public cbus_card_device_base
{
public:
	mpcba79_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override { gt_device_base::device_start(); set_bus_device(); }

	virtual void map(address_map &map) override { cbus_card_device_base::map(map); dual_gt_device_base::map(map); }
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
};


class msmt070_device : public single_gt_device_base, public cbus_card_device_base
{
public:
	msmt070_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override { gt_device_base::device_start(); set_bus_device(); }

	virtual void map(address_map &map) override { cbus_card_device_base::map(map); single_gt_device_base::map(map); }
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
};

class msmt071_device : public dual_gt_device_base, public cbus_card_device_base
{
public:
	msmt071_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override { gt_device_base::device_start(); set_bus_device(); }

	virtual void map(address_map &map) override { cbus_card_device_base::map(map); dual_gt_device_base::map(map); }
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
};

class msmt081_device : public single_gt_device_base, public cbus_card_device_base
{
public:
	msmt081_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override { gt_device_base::device_start(); set_bus_device(); }

	virtual void map(address_map &map) override { cbus_card_device_base::map(map); single_gt_device_base::map(map); }
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
};

class mpcbb92_device : public single_gt_device_base, public srx_card_device_base
{
public:
	mpcbb92_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override { gt_device_base::device_start(); set_bus_device(); }

	virtual void map(address_map &map) override { srx_card_device_base::map(map); single_gt_device_base::map(map); }
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
};

// device type definition
DECLARE_DEVICE_TYPE(MPCB963, mpcb963_device)
DECLARE_DEVICE_TYPE(MPCBA79, mpcba79_device)
DECLARE_DEVICE_TYPE(MSMT070, msmt070_device)
DECLARE_DEVICE_TYPE(MSMT071, msmt071_device)
DECLARE_DEVICE_TYPE(MSMT081, msmt081_device)
DECLARE_DEVICE_TYPE(MPCBB92, mpcbb92_device)

#endif // MAME_BUS_INTERPRO_SR_GT_H
