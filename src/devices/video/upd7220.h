// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Miodrag Milanovic, Carl
/**********************************************************************

    NEC uPD7220 Graphics Display Controller emulation

**********************************************************************
                            _____   _____
                2xWCLK   1 |*    \_/     | 40  Vcc
                 _DBIN   2 |             | 39  A17
                 HSYNC   3 |             | 38  A16
            V/EXT SYNC   4 |             | 37  AD15
                 BLANK   5 |             | 36  AD14
                   ALE   6 |             | 35  AD13
                   DRQ   7 |             | 34  AD12
                 _DACK   8 |             | 33  AD11
                   _RD   9 |             | 32  AD10
                   _WR  10 |   uPD7220   | 31  AD9
                    A0  11 |    82720    | 30  AD8
                   DB0  12 |             | 29  AD7
                   DB1  13 |             | 28  AD6
                   DB2  14 |             | 27  AD5
                   DB3  15 |             | 26  AD4
                   DB4  16 |             | 25  AD3
                   DB5  17 |             | 24  AD2
                   DB6  18 |             | 23  AD1
                   DB7  19 |             | 22  AD0
                   GND  20 |_____________| 21  LPEN

**********************************************************************/

#ifndef MAME_VIDEO_UPD7220_H
#define MAME_VIDEO_UPD7220_H

#pragma once



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define UPD7220_DISPLAY_PIXELS_MEMBER(_name) void _name(bitmap_rgb32 &bitmap, int y, int x, uint32_t address)
#define UPD7220_DRAW_TEXT_LINE_MEMBER(_name) void _name(bitmap_rgb32 &bitmap, uint32_t addr, int y, int wd, int pitch, int lr, int cursor_on, int cursor_addr)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> upd7220_device

class upd7220_device :  public device_t,
						public device_memory_interface,
						public device_video_interface
{
public:
	typedef device_delegate<void (bitmap_rgb32 &bitmap, int y, int x, uint32_t address)> display_pixels_delegate;
	typedef device_delegate<void (bitmap_rgb32 &bitmap, uint32_t addr, int y, int wd, int pitch, int lr, int cursor_on, int cursor_addr)> draw_text_delegate;

	// construction/destruction
	upd7220_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// FIXME: these should be aware of current device for resolving the tag
	template <class FunctionClass>
	void set_display_pixels(void (FunctionClass::*init)(bitmap_rgb32 &, int, int, uint32_t), const char *name)
	{
		m_display_cb = display_pixels_delegate(init, name, nullptr, static_cast<FunctionClass *>(nullptr));
	}
	template <class FunctionClass>
	void set_display_pixels(void (FunctionClass::*init)(bitmap_rgb32 &, int, int, uint32_t) const, const char *name)
	{
		m_display_cb = display_pixels_delegate(init, name, nullptr, static_cast<FunctionClass *>(nullptr));
	}
	template <class FunctionClass>
	void set_display_pixels(const char *devname, void (FunctionClass::*init)(bitmap_rgb32 &, int, int, uint32_t), const char *name)
	{
		m_display_cb = display_pixels_delegate(init, name, devname, static_cast<FunctionClass *>(nullptr));
	}
	template <class FunctionClass>
	void set_display_pixels(const char *devname, void (FunctionClass::*init)(bitmap_rgb32 &, int, int, uint32_t) const, const char *name)
	{
		m_display_cb = display_pixels_delegate(init, name, devname, static_cast<FunctionClass *>(nullptr));
	}
	template <class FunctionClass>
	void set_draw_text(void (FunctionClass::*init)(bitmap_rgb32 &, uint32_t, int, int, int, int, int, int), const char *name)
	{
		m_draw_text_cb = draw_text_delegate(init, name, nullptr, static_cast<FunctionClass *>(nullptr));
	}
	template <class FunctionClass>
	void set_draw_text(void (FunctionClass::*init)(bitmap_rgb32 &, uint32_t, int, int, int, int, int, int) const, const char *name)
	{
		m_draw_text_cb = draw_text_delegate(init, name, nullptr, static_cast<FunctionClass *>(nullptr));
	}
	template <class FunctionClass>
	void set_draw_text(const char *devname, void (FunctionClass::*init)(bitmap_rgb32 &, uint32_t, int, int, int, int, int, int), const char *name)
	{
		m_draw_text_cb = draw_text_delegate(init, name, devname, static_cast<FunctionClass *>(nullptr));
	}
	template <class FunctionClass>
	void set_draw_text(const char *devname, void (FunctionClass::*init)(bitmap_rgb32 &, uint32_t, int, int, int, int, int, int) const, const char *name)
	{
		m_draw_text_cb = draw_text_delegate(init, name, devname, static_cast<FunctionClass *>(nullptr));
	}

	auto drq_wr_callback() { return m_write_drq.bind(); }
	auto hsync_wr_callback() { return m_write_hsync.bind(); }
	auto vsync_wr_callback() { return m_write_vsync.bind(); }
	auto blank_wr_callback() { return m_write_blank.bind(); }

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	uint8_t dack_r();
	void dack_w(uint8_t data);

	DECLARE_WRITE_LINE_MEMBER( ext_sync_w );
	DECLARE_WRITE_LINE_MEMBER( lpen_w );

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual space_config_vector memory_space_config() const override;

private:
	enum
	{
		TIMER_VSYNC,
		TIMER_HSYNC,
		TIMER_BLANK
	};

	inline uint8_t readbyte(offs_t address);
	inline void writebyte(offs_t address, uint8_t data);
	inline uint16_t readword(offs_t address);
	inline void writeword(offs_t address, uint16_t data);
	inline void fifo_clear();
	inline int fifo_param_count();
	inline void fifo_set_direction(int dir);
	inline void queue(uint8_t data, int flag);
	inline void dequeue(uint8_t *data, int *flag);
	inline void update_vsync_timer(int state);
	inline void update_hsync_timer(int state);
	inline void update_blank_timer(int state);
	inline void recompute_parameters();
	inline void reset_figs_param();
	inline void read_vram(uint8_t type, uint8_t mod);
	inline void write_vram(uint8_t type, uint8_t mod);
	inline void get_text_partition(int index, uint32_t *sad, uint16_t *len, int *im, int *wd);
	inline void get_graphics_partition(int index, uint32_t *sad, uint16_t *len, int *im, int *wd);

	void draw_pixel(int x, int y, int xi, uint16_t tile_data);
	void draw_line(int x, int y);
	void draw_rectangle(int x, int y);
	void draw_arc(int x, int y);
	void draw_char(int x, int y);
	int translate_command(uint8_t data);
	void process_fifo();
	void continue_command();
	void update_text(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_graphics_line(bitmap_rgb32 &bitmap, uint32_t addr, int y, int wd, int pitch);
	void update_graphics(bitmap_rgb32 &bitmap, const rectangle &cliprect, int force_bitmap);

	void upd7220_vram(address_map &map);

	display_pixels_delegate     m_display_cb;
	draw_text_delegate          m_draw_text_cb;

	devcb_write_line   m_write_drq;
	devcb_write_line   m_write_hsync;
	devcb_write_line   m_write_vsync;
	devcb_write_line   m_write_blank;

	uint16_t m_mask;                  // mask register
	uint8_t m_pitch;                  // number of word addresses in display memory in the horizontal direction
	uint32_t m_ead;                   // execute word address
	uint16_t m_dad;                   // dot address within the word
	uint32_t m_lad;                   // light pen address

	uint8_t m_ra[16];                 // parameter RAM
	int m_ra_addr;                  // parameter RAM address

	uint8_t m_sr;                     // status register
	uint8_t m_cr;                     // command register
	uint8_t m_pr[17];                 // parameter byte register
	int m_param_ptr;                // parameter pointer

	uint8_t m_fifo[16];               // FIFO data queue
	int m_fifo_flag[16];            // FIFO flag queue
	int m_fifo_ptr;                 // FIFO pointer
	int m_fifo_dir;                 // FIFO direction

	uint8_t m_mode;                   // mode of operation

	int m_de;                       // display enabled
	int m_m;                        // 0 = accept external vertical sync (slave mode) / 1 = generate & output vertical sync (master mode)
	int m_aw;                       // active display words per line - 2 (must be even number with bit 0 = 0)
	int m_al;                       // active display lines per video field
	int m_vs;                       // vertical sync width - 1
	int m_vfp;                      // vertical front porch width - 1
	int m_vbp;                      // vertical back porch width - 1
	int m_hs;                       // horizontal sync width - 1
	int m_hfp;                      // horizontal front porch width - 1
	int m_hbp;                      // horizontal back porch width - 1

	int m_dc;                       // display cursor
	int m_sc;                       // 0 = blinking cursor / 1 = steady cursor
	int m_br;                       // blink rate
	int m_ctop;                     // cursor top line number in the row
	int m_cbot;                     // cursor bottom line number in the row (CBOT < LR)
	int m_lr;                       // lines per character row - 1

	int m_disp;                     // display zoom factor
	int m_gchr;                     // zoom factor for graphics character writing and area filling

	uint8_t m_bitmap_mod;

	struct {
		uint8_t m_dir;                // figs param 0: drawing direction
		uint8_t m_figure_type;        // figs param 1: figure type
		uint16_t m_dc;                // figs param 2:
		uint8_t  m_gd;                // mixed mode only
		uint16_t m_d;                 // figs param 3:
		uint16_t m_d1;                // figs param 4:
		uint16_t m_d2;                // figs param 5:
		uint16_t m_dm;                // figs param 6:
	} m_figs;

	// timers
	emu_timer *m_vsync_timer;       // vertical sync timer
	emu_timer *m_hsync_timer;       // horizontal sync timer
	emu_timer *m_blank_timer;       // CRT blanking timer

	const address_space_config      m_space_config;
};


// device type definition
DECLARE_DEVICE_TYPE(UPD7220, upd7220_device)

#endif // MAME_VIDEO_UPD7220_H
