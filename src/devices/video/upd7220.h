// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Miodrag Milanovic, Carl, Brian Johnson
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
	using display_pixels_delegate = device_delegate<void (bitmap_rgb32 &bitmap, int y, int x, uint32_t address)>;
	using draw_text_delegate = device_delegate<void (bitmap_rgb32 &bitmap, uint32_t addr, int y, int wd, int pitch, int lr, int cursor_on, int cursor_addr)>;

	// construction/destruction
	upd7220_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename... T> void set_display_pixels(T &&... args) { m_display_cb.set(std::forward<T>(args)...); }
	template <typename... T> void set_draw_text(T &&... args) { m_draw_text_cb.set(std::forward<T>(args)...); }

	auto drq_wr_callback() { return m_write_drq.bind(); }
	auto hsync_wr_callback() { return m_write_hsync.bind(); }
	auto vsync_wr_callback() { return m_write_vsync.bind(); }
	auto blank_wr_callback() { return m_write_blank.bind(); }

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	uint8_t dack_r();
	void dack_w(uint8_t data);

	void ext_sync_w(int state);
	void lpen_w(int state);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	upd7220_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual space_config_vector memory_space_config() const override;

	virtual int translate_command(uint8_t data);

	TIMER_CALLBACK_MEMBER(hsync_update);
	TIMER_CALLBACK_MEMBER(vsync_update);
	TIMER_CALLBACK_MEMBER(blank_update);

	void start_dma();
	void stop_dma();

private:
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
	inline void rdat(uint8_t type, uint8_t mod);
	inline uint16_t read_vram();
	inline void wdat(uint8_t type, uint8_t mod);
	inline void write_vram(uint8_t type, uint8_t mod, uint16_t data, uint16_t mask = 0xffff);
	inline void get_text_partition(int index, uint32_t *sad, uint16_t *len, int *im, int *wd);
	inline void get_graphics_partition(int index, uint32_t *sad, uint16_t *len, int *im, int *wd);

	uint16_t get_pitch();
	uint16_t get_pattern(uint8_t cycle);
	void next_pixel(int direction);
	void draw_pixel();
	void draw_line();
	void draw_rectangle();
	void draw_arc();
	void draw_char();
	void process_fifo();
	void continue_command();
	void update_text(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_graphics_line(bitmap_rgb32 &bitmap, uint32_t addr, int y, int wd, int mixed);
	void update_graphics(bitmap_rgb32 &bitmap, const rectangle &cliprect, int force_bitmap);

	void upd7220_vram(address_map &map) ATTR_COLD;

	display_pixels_delegate     m_display_cb;
	draw_text_delegate          m_draw_text_cb;

	devcb_write_line   m_write_drq;
	devcb_write_line   m_write_hsync;
	devcb_write_line   m_write_vsync;
	devcb_write_line   m_write_blank;

	uint16_t m_pattern;

	uint8_t m_dma_type;               // DMA transfer type
	uint8_t m_dma_mod;                // DMA transfer mode
	uint16_t m_dma_data;              // current word transferred via DMA
	uint32_t m_dma_transfer_length;   // DMA transfer length in bytes

	uint16_t m_mask;                  // mask register
	uint16_t m_pitch;                 // number of word addresses in display memory in the horizontal direction
	uint32_t m_ead;                   // execute word address
	uint32_t m_lad;                   // light pen address

	uint8_t m_ra[16];                 // parameter RAM
	int m_ra_addr;                    // parameter RAM address

	uint8_t m_sr;                     // status register
	uint8_t m_cr;                     // command register
	uint8_t m_pr[17];                 // parameter byte register
	int m_param_ptr;                  // parameter pointer

	uint8_t m_fifo[16];               // FIFO data queue
	int m_fifo_flag[16];              // FIFO flag queue
	int m_fifo_ptr;                   // FIFO pointer
	int m_fifo_dir;                   // FIFO direction

	uint8_t m_mode;                   // mode of operation

	int m_de;                         // display enabled
	int m_m;                          // 0 = accept external vertical sync (slave mode) / 1 = generate & output vertical sync (master mode)
	int m_aw;                         // active display words per line - 2 (must be even number with bit 0 = 0)
	int m_al;                         // active display lines per video field
	int m_vs;                         // vertical sync width - 1
	int m_vfp;                        // vertical front porch width - 1
	int m_vbp;                        // vertical back porch width - 1
	int m_hs;                         // horizontal sync width - 1
	int m_hfp;                        // horizontal front porch width - 1
	int m_hbp;                        // horizontal back porch width - 1

	int m_dc;                         // display cursor
	int m_sc;                         // 0 = blinking cursor / 1 = steady cursor
	int m_br;                         // blink rate
	int m_ctop;                       // cursor top line number in the row
	int m_cbot;                       // cursor bottom line number in the row (CBOT < LR)
	int m_lr;                         // lines per character row - 1

	int m_disp;                       // display zoom factor
	int m_gchr;                       // zoom factor for graphics character writing and area filling

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


// ======================> upd7220a_device

class upd7220a_device : public upd7220_device
{
public:
	// construction/destruction
	upd7220a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual int translate_command(uint8_t data) override;
};


// device type definition
DECLARE_DEVICE_TYPE(UPD7220, upd7220_device)
DECLARE_DEVICE_TYPE(UPD7220A, upd7220a_device)

#endif // MAME_VIDEO_UPD7220_H
