// license:BSD-3-Clause
// copyright-holders:Barry Rodewald,Carl

#ifndef MAME_SHARP_X68K_CRTC_H
#define MAME_SHARP_X68K_CRTC_H

class x68k_crtc_device : public device_t, public device_video_interface
{
public:
	// device callback configuration
	auto vdisp_cb() { return m_vdisp_callback.bind(); }
	auto rint_cb() { return m_rint_callback.bind(); }
	auto hsync_cb() { return m_hsync_callback.bind(); }
	auto tvram_read_cb() { return m_tvram_read_callback.bind(); }
	auto tvram_write_cb() { return m_tvram_write_callback.bind(); }
	auto gvram_read_cb() { return m_gvram_read_callback.bind(); }
	auto gvram_write_cb() { return m_gvram_write_callback.bind(); }

	// clock configuration
	void set_clock_69m(uint32_t clock) { m_clock_69m = clock; }
	void set_clock_69m(const XTAL &xtal) { set_clock_69m(xtal.value()); }
	void set_clock_50m(uint32_t clock) { m_clock_50m = clock; }
	void set_clock_50m(const XTAL &xtal) { set_clock_50m(xtal.value()); }
	u32 clock_39m() const { return clock(); }
	u32 clock_69m() const { return m_clock_69m; }
	u32 clock_50m() const { return m_clock_50m; }

	void crtc_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 crtc_r(offs_t offset);
	void gvram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 gvram_r(offs_t offset);
	void tvram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 tvram_r(offs_t offset);

	// getters
	u16 xscr_text() const { return m_reg[10]; }
	u16 yscr_text() const { return m_reg[11]; }
	u16 xscr_gfx(int page) const { return m_reg[12 + page * 2]; }
	u16 yscr_gfx(int page) const { return m_reg[13 + page * 2]; }
	u8 vfactor() const { return (m_reg[20] & 0x0c) >> 2; }
	bool is_1024x1024() const { return BIT(m_reg[20], 10); }
	bool gfx_double_scan() const { return (m_reg[20] & 0x1e) == 0x10; }
	bool gfx_layer_buffer() const { return BIT(m_reg[20], 11); }
	u8 gfx_color_mode() const { return (m_reg[20] >> 8) & 3; }
	bool text_layer_buffer() const { return BIT(m_reg[20], 12); }
	u16 hbegin() const { return m_hbegin; }
	u16 vbegin() const { return m_vbegin; }
	u16 hend() const { return m_hend; }
	u16 vend() const { return m_vend; }
	u16 visible_height() const { return m_visible_height; }
	u16 visible_width() const { return m_visible_width; }

protected:
	// base constructor
	x68k_crtc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// internal helpers
	void text_copy(unsigned src, unsigned dest, u8 planes);
	TIMER_CALLBACK_MEMBER(operation_end);
	void refresh_mode();
	TIMER_CALLBACK_MEMBER(hsync);
	TIMER_CALLBACK_MEMBER(raster_end);
	TIMER_CALLBACK_MEMBER(raster_irq);
	TIMER_CALLBACK_MEMBER(vblank_irq);

	// device callbacks
	devcb_write_line m_vdisp_callback;
	devcb_write_line m_rint_callback;
	devcb_write_line m_hsync_callback;
	devcb_read16 m_tvram_read_callback;
	devcb_read16 m_gvram_read_callback;
	devcb_write16 m_tvram_write_callback;
	devcb_write16 m_gvram_write_callback;

	// software-selectable oscillators
	u32 m_clock_69m;
	u32 m_clock_50m;

	// internal state
	u16 m_reg[24];  // registers
	u8 m_operation;  // operation port (0xe80481)
	bool m_vblank;  // true if in VBlank
	bool m_hblank;  // true if in HBlank
	u16 m_htotal;  // Horizontal Total (in characters)
	u16 m_vtotal;  // Vertical Total
	u16 m_hbegin;  // Horizontal Begin
	u16 m_vbegin;  // Vertical Begin
	u16 m_hend;    // Horizontal End
	u16 m_vend;    // Vertical End
	u16 m_hsync_end;  // Horizontal Sync End
	u16 m_vsync_end;  // Vertical Sync End
	u16 m_hsyncadjust;  // Horizontal Sync Adjustment
	//float m_hmultiple;  // Horizontal pixel multiplier
	float m_vmultiple;  // Vertical scanline multiplier (x2 for doublescan modes)
	u16 m_height;
	u16 m_width;
	u16 m_visible_height;
	u16 m_visible_width;
	//u16 m_video_width;  // horizontal total (in pixels)
	//u16 m_video_height; // vertical total
	bool m_interlace;  // 1024 vertical resolution is interlaced
	//u16 m_scanline;
	bool m_oddscanline;

	emu_timer *m_scanline_timer;
	emu_timer *m_raster_irq_timer;
	emu_timer *m_vblank_irq_timer;
	emu_timer *m_raster_end_timer;
	emu_timer *m_operation_end_timer;
};

class vinas_device : public x68k_crtc_device
{
public:
	vinas_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class vicon_device : public x68k_crtc_device
{
public:
	vicon_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

// device type definitions
DECLARE_DEVICE_TYPE(VINAS, vinas_device)
DECLARE_DEVICE_TYPE(VICON, vicon_device)

#endif // MAME_SHARP_X68K_CRTC_H
