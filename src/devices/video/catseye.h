// license:BSD-3-Clause
// copyright-holders:Sven Schnelle
#ifndef MAME_VIDEO_CATSEYE_H
#define MAME_VIDEO_CATSEYE_H

#pragma once

class catseye_device: public device_t
{
public:
	static constexpr feature_type imperfect_features() { return feature::GRAPHICS; }

	catseye_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint16_t vram_r(offs_t offset, uint16_t mem_mask = ~0);
	void vram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	uint16_t ctrl_r(address_space &space, offs_t offset, uint16_t mem_mask = ~0);
	void ctrl_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void vblank_w(int state);

	void set_fb_width(int width) { m_fb_width = width; }
	void set_fb_height(int height) { m_fb_height = height; }
	void set_plane(const u8 plane) {
		m_plane_mask_l = (1 << plane);
		m_plane_mask_h = (0x100 << plane);
		m_plane = plane;
	}

	bool has_changed() {
		bool ret = m_changed;
		m_changed = false;
		return ret;
	};

	u8 plane_enabled() const {
		if (!m_display_enable || (m_blink_enable && !m_blink_state))
			return 0;
		return m_plane_mask_l;
	}

	auto irq_out_cb() { return m_int_write_func.bind(); }

protected:

	required_shared_ptr_array<u8, 2> m_vram;
	catseye_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	devcb_write8 m_int_write_func;
	TIMER_CALLBACK_MEMBER(blink_callback);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void execute_rule(const bool src, const int rule, bool &dst) const;
	void update_int();

	template<int Idx> void window_move();
	template<int Idx> void draw_line();
	template<int Idx> void trigger_wm();
	template<int Idx> void draw_pixel(int x, int y, int color);
	template<int Idx> void vram_w(offs_t offset, u16 data, u16 mem_mask);
	template<int Idx> void vram_w_bit(offs_t offset, u16 data, u16 mem_mask);
	template<int Idx> void vram_w_word(offs_t offset, u16 data, u16 mem_mask);
	template<int Idx> u16 vram_r(offs_t offset, u16 mem_mask);
	template<int Idx> u16 vram_r_bit(offs_t offset);
	template<int Idx> u16 vram_r_word(offs_t offset, u16 mem_mask);

	int get_plane_from_idx(const int idx) const {
		if (idx == VRAM_VIDEO_PLANE) {
			if (m_acntrl == 0x0200)
				return VRAM_OVERLAY_PLANE;
			else
				return VRAM_VIDEO_PLANE;
		} else {
			if (m_acntrl == 0x0200)
				return VRAM_VIDEO_PLANE;
			else
				return VRAM_OVERLAY_PLANE;
		}
	}

	template<int Idx>
	void modify_vram(int x, int y, bool state) {
		m_changed = true;
		m_status |= CATSEYE_STATUS_UNCLIPPED;
		const int offset = y * m_fb_width + x;
		if (state)
			m_vram[Idx][offset] |= m_plane_mask_l;
		else
			m_vram[Idx][offset] &= ~m_plane_mask_l;
	}

	template<int Idx>
	void modify_vram_offset(int offset, bool state) {
		m_changed = true;
		m_status |= CATSEYE_STATUS_UNCLIPPED;
		if (state)
			m_vram[Idx][offset] |= m_plane_mask_l;
		else
			m_vram[Idx][offset] &= ~m_plane_mask_l;
	}

	template<int Idx>
	u8 get_vram_offset_plane(int offset) const {
		const int plane = (m_planemode[Idx] >> 8) & 0x0f;
		if (!m_planemode[Idx])
			return m_vram[Idx][offset] & m_plane_mask_l;
		else if (plane < 8)
			return m_vram[Idx][offset] & (1 << plane);
		else
			return m_vram[Idx][offset] & (1 << (plane-8));
	}


	template<int Idx>
	u8 get_vram_offset(int offset) const {
			return m_vram[Idx][offset] & m_plane_mask_l;
	}

	template<int Idx>
	bool get_vram_pixel(int x, int y) const {
		return m_vram[get_plane_from_idx(Idx)][y * m_fb_width + x] & m_plane_mask_l;
	}

	template<int Idx>
	bool get_vram_pixel_plane(int x, int y) const {
		const int plane = (m_planemode[Idx] >> 8) & 0x0f;
		const int offset = y * m_fb_width + x;
		if (!(m_planemode[Idx] & (1 << 12)))
			return m_vram[Idx][offset] & m_plane_mask_l;
		else if (plane < 8)
			return m_vram[0][offset] & (1 << plane);
		else
			return m_vram[1][offset] & (1 << (plane-8));
	}

	static constexpr int CATSEYE_REPLACE_RULE_CLEAR = 0;
	static constexpr int CATSEYE_REPLACE_RULE_SRC_AND_DST = 1;
	static constexpr int CATSEYE_REPLACE_RULE_SRC_AND_NOT_DST = 2;
	static constexpr int CATSEYE_REPLACE_RULE_SRC = 3;
	static constexpr int CATSEYE_REPLACE_RULE_NOT_SRC_AND_DST = 4;
	static constexpr int CATSEYE_REPLACE_RULE_NOP = 5;
	static constexpr int CATSEYE_REPLACE_RULE_SRC_XOR_DST = 6;
	static constexpr int CATSEYE_REPLACE_RULE_SRC_OR_DST = 7;
	static constexpr int CATSEYE_REPLACE_RULE_NOT_SRC_AND_NOT_DST = 8;
	static constexpr int CATSEYE_REPLACE_RULE_NOT_SRC_XOR_DST = 9;
	static constexpr int CATSEYE_REPLACE_RULE_NOT_DST = 10;
	static constexpr int CATSEYE_REPLACE_RULE_SRC_OR_NOT_DST = 11;
	static constexpr int CATSEYE_REPLACE_RULE_NOT_SRC = 12;
	static constexpr int CATSEYE_REPLACE_RULE_NOT_SRC_OR_DST = 13;
	static constexpr int CATSEYE_REPLACE_RULE_NOT_SRC_OR_NOT_DST = 14;
	static constexpr int CATSEYE_REPLACE_RULE_SET = 15;

	// registers compatible to topcat
	static constexpr int TOPCAT_REG_VBLANK=0x20;
	static constexpr int TOPCAT_REG_WMOVE_ACTIVE=0x22;
	static constexpr int TOPCAT_REG_VBLANK_INTRQ=0x24;
	static constexpr int TOPCAT_REG_WMOVE_INTRQ=0x26;
	static constexpr int TOPCAT_REG_DISPLAY_PLANE_ENABLE=0x40;
	static constexpr int TOPCAT_REG_SYNC_ENABLE=0x42;
	static constexpr int TOPCAT_REG_WRITE_ENABLE_PLANE=0x44;
	static constexpr int TOPCAT_REG_READ_ENABLE_PLANE=0x46;
	static constexpr int TOPCAT_REG_FB_WRITE_ENABLE=0x48;
	static constexpr int TOPCAT_REG_WMOVE_IE=0x4a;
	static constexpr int TOPCAT_REG_VBLANK_IE=0x4c;
	static constexpr int TOPCAT_REG_START_WMOVE=0x4e;
	static constexpr int TOPCAT_REG_ENABLE_BLINK_PLANES=0x50;
	static constexpr int TOPCAT_REG_ENABLE_ALT_FRAME=0x54;
	static constexpr int TOPCAT_REG_PRR=0x75;
	static constexpr int TOPCAT_REG_WRR=0x77;
	static constexpr int TOPCAT_REG_WMSOURCEX=0x79;
	static constexpr int TOPCAT_REG_WMSOURCEY=0x7b;
	static constexpr int TOPCAT_REG_WMDESTX=0x7d;
	static constexpr int TOPCAT_REG_WMDESTY=0x7f;
	static constexpr int TOPCAT_REG_WMWIDTH=0x81;
	static constexpr int TOPCAT_REG_WMHEIGHT=0x83;

	// catseye specific registers
	static constexpr int CATSEYE_REG_WMX=0x100;
	static constexpr int CATSEYE_REG_RUG_REV=0x102;
	static constexpr int CATSEYE_REG_RUG_SC=0x103;
	static constexpr int CATSEYE_REG_WMWIDTH=0x104;
	static constexpr int CATSEYE_REG_WMHEIGHT=0x105;
	static constexpr int CATSEYE_REG_LINEPATH=0x106;
	static constexpr int CATSEYE_REG_LINETYPE=0x107;
	static constexpr int CATSEYE_REG_WMSOURCEX=0x108;
	static constexpr int CATSEYE_REG_WMSOURCEY=0x109;
	static constexpr int CATSEYE_REG_WMDESTX=0x10a;
	static constexpr int CATSEYE_REG_WMDESTY=0x10b;
	static constexpr int CATSEYE_REG_WMCLIPLEFT=0x10c;
	static constexpr int CATSEYE_REG_WMCLIPRIGHT=0x10d;
	static constexpr int CATSEYE_REG_WMCLIPTOP=0x10e;
	static constexpr int CATSEYE_REG_WMCLIPBOTTOM=0x10f;
	static constexpr int CATSEYE_REG_TWMWIDTH=0x184;
	static constexpr int CATSEYE_REG_TWMHEIGHT=0x185;
	static constexpr int CATSEYE_REG_TWMSOURCEX=0x188;
	static constexpr int CATSEYE_REG_TWMSOURCEY=0x189;
	static constexpr int CATSEYE_REG_TWMDESTX=0x18a;
	static constexpr int CATSEYE_REG_TWMDESTY=0x18b;

	// Window mover 1 registers
	static constexpr int CATSEYE_REG_PATTERNS1=0x200;
	static constexpr int CATSEYE_REG_FBEN1=0x280;
	static constexpr int CATSEYE_REG_PRR1=0x281;
	static constexpr int CATSEYE_REG_TCREN1=0x282;
	static constexpr int CATSEYE_REG_WRR1=0x283;
	static constexpr int CATSEYE_REG_TCWEN1=0x284;
	static constexpr int CATSEYE_REG_BARC_REV1=0x285;
	static constexpr int CATSEYE_REG_TRR1=0x286;
	static constexpr int CATSEYE_REG_COLOR1=0x287;
	static constexpr int CATSEYE_REG_VB1=0x288;
	static constexpr int CATSEYE_REG_TRRCTL1=0x289;
	static constexpr int CATSEYE_REG_ACNTL1=0x28a;
	static constexpr int CATSEYE_REG_PLANEMODE1=0x28b;

	// Window mover 2 registers
	static constexpr int CATSEYE_REG_PATTERNS2=0x300;
	static constexpr int CATSEYE_REG_FBEN2=0x380;
	static constexpr int CATSEYE_REG_PRR2=0x381;
	static constexpr int CATSEYE_REG_TCREN2=0x382;
	static constexpr int CATSEYE_REG_WRR2=0x383;
	static constexpr int CATSEYE_REG_TCWEN2=0x384;
	static constexpr int CATSEYE_REG_BARC_REV2=0x385;
	static constexpr int CATSEYE_REG_TRR2=0x386;
	static constexpr int CATSEYE_REG_COLOR2=0x387;
	static constexpr int CATSEYE_REG_VB2=0x388;
	static constexpr int CATSEYE_REG_TRRCTL2=0x389;
	static constexpr int CATSEYE_REG_ACNTL2=0x38a;
	static constexpr int CATSEYE_REG_PLANEMODE2=0x38b;

	static constexpr int CATSEYE_REG_STATUS=0x400;

	static constexpr u16 CATSEYE_STATUS_READY = 0x02;
	static constexpr u16 CATSEYE_STATUS_UNCLIPPED = 0x04;
	static constexpr u16 CATSEYE_STATUS_NO_DAUGHTERBOARD = 0x08;
	static constexpr u16 CATSEYE_STATUS_VBLANK = 0x10;

	// bit definitions for various registers
	static constexpr u16 CATSEYE_VB_VECTOR = 0x100;
	static constexpr u16 CATSEYE_MISC_ENABLE_CLIP = 0x80;
	static constexpr int VRAM_VIDEO_PLANE = 0;
	static constexpr int VRAM_OVERLAY_PLANE = 1;

	emu_timer *m_blink_timer;

	// set when screen contents have changed
	bool m_changed;

	// keeps track of visibility when plane is enabled for blinking
	bool m_blink_state;

	// configuration settings
	int m_fb_width;
	int m_fb_height;
	int m_plane;

	// we keep the plane mask twice - once for matching the lower byte,
	// and once for matching the higher byte.
	u16 m_plane_mask_l;
	u16 m_plane_mask_h;

	// registers are enabled to be written by host cpu
	u16 m_write_enable[2];

	// registers are enabled to be read by host cpu
	u16 m_read_enable[2];

	// enables writing to plane from window mover
	u16 m_fb_enable[2];

	// plane is visible on screen
	u16 m_display_enable;

	// plane is blinking
	u16 m_blink_enable;

	// sync output to monitor is enabled (ignored)
	u16 m_sync_enable;

	// video output is blanked due to vert retrace
	u16 m_in_vblank;

	// interrupt status & enable
	u16 m_wm_int_pending;
	u16 m_vblank_int_pending;
	u16 m_wm_int_enable;
	u16 m_vblank_int_enable;

	// RUGCMD:
	// bit 7-6: 0 - solid vector/circle
	//          1 - linetype vector/circle
	//          2 - blit mode
	//          3 - fill mode
	//   bit 5: 1 - draw circle, 0 - draw vector
	//   bit 4: 1 - RUG is allowed to write to framebuffer

	u16 m_rugsc;

	// unknown, bit 7 enables window mover clipping
	u16 m_misc;

	// WMX register allows to set SOURCEX and DESTX register
	// in the window mover at the same time
	u16 m_wmx;

	// co-ordinates for window mover
	u16 m_wmwidth;
	u16 m_wmheight;
	u16 m_wmsourcex;
	u16 m_wmsourcey;
	u16 m_wmdestx;
	u16 m_wmdesty;

	// limits for window mover
	// (pixels outside of this range will not be drawn)
	u16 m_wmclipleft;
	u16 m_wmclipright;
	u16 m_wmcliptop;
	u16 m_wmclipbottom;

	// patterns for tripple replacement rules
	std::array<std::array<u16, 128>, 2> m_patterns;

	 // 16 bit pattern for lines drawn
	u16 m_linepath;

	// bit 11-8: repeat length
	// bit  7-4: current pattern bit
	// bit  3-0: current repeat count
	u16 m_linetype;

	// Pixel replacement rule, applied when writing to VRAM from host CPU
	u16 m_prr[2];

	// Window replacement rule, applied by window mover
	u16 m_wrr[2];

	// Tripple replacement rule, used together with the patterns register
	u16 m_trr[2];

	// bit 8: 1 - enable tripple replacement rule
	u16 m_trrctl[2];

	// color used for drawing vectors
	u16 m_color[2];

	// unknown, bit 8: 1 - vector mode, 0 - bitblt mode
	u16 m_vb[2];

	// bit 1: 1 - VRAM is accessed in bit-mode by host CPU
	//            2 bytes are expanded to 16 bytes, with 1 bit per byte information
	// bit 2: 1 - host CPU reads overlay plane via host interface
	u16 m_acntrl;

	// source plane for window mover operations
	// bit 4: 1 - source plane is given in bit 3-0
	//        0 - current plane is source for move
	u16 m_planemode[2];

	// bit 0: 1 - window mover busy
	// bit 1: 1 - registers ready for host CPU
	// bit 2: 1 - pixels generated by RUG
	// bit 3: 1 - no daughter board present
	// bit 4: 1 - in vertical blank
	// bit 4: 1 - in horizontal sync
	u16 m_status;

};

DECLARE_DEVICE_TYPE(CATSEYE, catseye_device)
#endif // MAME_VIDEO_CATSEYE_H
