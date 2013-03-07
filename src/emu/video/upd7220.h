/**********************************************************************

    NEC uPD7220 Graphics Display Controller emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

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

#pragma once

#ifndef __UPD7220__
#define __UPD7220__

#include "emu.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************




//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_UPD7220_ADD(_tag, _clock, _config, _map) \
	MCFG_DEVICE_ADD(_tag, UPD7220, _clock) \
	MCFG_DEVICE_CONFIG(_config) \
	MCFG_DEVICE_ADDRESS_MAP(AS_0, _map)

#define UPD7220_INTERFACE(name) \
	const upd7220_interface (name) =



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

typedef void (*upd7220_display_pixels_func)(device_t *device, bitmap_rgb32 &bitmap, int y, int x, UINT32 address);
#define UPD7220_DISPLAY_PIXELS(name) void name(device_t *device, bitmap_rgb32 &bitmap, int y, int x, UINT32 address)

typedef void (*upd7220_draw_text_line)(device_t *device, bitmap_rgb32 &bitmap, UINT32 addr, int y, int wd, int pitch, int lr, int cursor_on, int cursor_addr);
#define UPD7220_DRAW_TEXT_LINE(name) void name(device_t *device, bitmap_rgb32 &bitmap, UINT32 addr, int y, int wd, int pitch, int lr, int cursor_on, int cursor_addr)


// ======================> upd7220_interface

struct upd7220_interface
{
	const char *m_screen_tag;

	upd7220_display_pixels_func m_display_cb;
	upd7220_draw_text_line m_draw_text_cb;

	devcb_write_line        m_out_drq_cb;
	devcb_write_line        m_out_hsync_cb;
	devcb_write_line        m_out_vsync_cb;
	devcb_write_line        m_out_blank_cb;
};

// ======================> upd7220_device

class upd7220_device :  public device_t,
						public device_memory_interface,
						public upd7220_interface
{
public:
	// construction/destruction
	upd7220_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_READ8_MEMBER( dack_r );
	DECLARE_WRITE8_MEMBER( dack_w );

	DECLARE_WRITE_LINE_MEMBER( ext_sync_w );
	DECLARE_WRITE_LINE_MEMBER( lpen_w );

	DECLARE_WRITE8_MEMBER( bank_w );
	DECLARE_READ8_MEMBER( vram_r );
	DECLARE_WRITE8_MEMBER( vram_w );

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	virtual const rom_entry *device_rom_region() const;
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const;

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	virtual void device_config_complete();

private:
	static const device_timer_id TIMER_VSYNC = 0;
	static const device_timer_id TIMER_HSYNC = 1;
	static const device_timer_id TIMER_BLANK = 2;

	inline UINT8 readbyte(offs_t address);
	inline void writebyte(offs_t address, UINT8 data);
	inline void fifo_clear();
	inline int fifo_param_count();
	inline void fifo_set_direction(int dir);
	inline void queue(UINT8 data, int flag);
	inline void dequeue(UINT8 *data, int *flag);
	inline void update_vsync_timer(int state);
	inline void update_hsync_timer(int state);
	inline void update_blank_timer(int state);
	inline void recompute_parameters();
	inline void reset_figs_param();
	inline void advance_ead();
	inline void read_vram(UINT8 type, UINT8 mod);
	inline void write_vram(UINT8 type, UINT8 mod);
	inline UINT16 check_pattern(UINT16 pattern);
	inline void get_text_partition(int index, UINT32 *sad, UINT16 *len, int *im, int *wd);
	inline void get_graphics_partition(int index, UINT32 *sad, UINT16 *len, int *im, int *wd);

	void draw_pixel(int x, int y, UINT8 tile_data);
	void draw_line(int x, int y);
	void draw_rectangle(int x, int y);
	void draw_char(int x, int y);
	int translate_command(UINT8 data);
	void process_fifo();
	void update_text(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_graphics_line(bitmap_rgb32 &bitmap, UINT32 addr, int y, int wd);
	void update_graphics(bitmap_rgb32 &bitmap, const rectangle &cliprect, int force_bitmap);

	devcb_resolved_write_line   m_out_drq_func;
	devcb_resolved_write_line   m_out_hsync_func;
	devcb_resolved_write_line   m_out_vsync_func;
	devcb_resolved_write_line   m_out_blank_func;

	screen_device *m_screen;

	UINT16 m_mask;                  // mask register
	UINT8 m_pitch;                  // number of word addresses in display memory in the horizontal direction
	UINT32 m_ead;                   // execute word address
	UINT16 m_dad;                   // dot address within the word
	UINT32 m_lad;                   // light pen address

	UINT8 m_ra[16];                 // parameter RAM
	int m_ra_addr;                  // parameter RAM address

	UINT8 m_sr;                     // status register
	UINT8 m_cr;                     // command register
	UINT8 m_pr[17];                 // parameter byte register
	int m_param_ptr;                // parameter pointer

	UINT8 m_fifo[16];               // FIFO data queue
	int m_fifo_flag[16];            // FIFO flag queue
	int m_fifo_ptr;                 // FIFO pointer
	int m_fifo_dir;                 // FIFO direction

	UINT8 m_mode;                   // mode of operation
	UINT8 m_draw_mode;              // mode of drawing

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

	UINT8 m_bitmap_mod;

	struct {
		UINT8 m_dir;                // figs param 0: drawing direction
		UINT8 m_figure_type;        // figs param 1: figure type
		UINT16 m_dc;                // figs param 2:
		UINT16 m_d;                 // figs param 3:
		UINT16 m_d1;                // figs param 4:
		UINT16 m_d2;                // figs param 5:
		UINT16 m_dm;                // figs param 6:
	} m_figs;

	// timers
	emu_timer *m_vsync_timer;       // vertical sync timer
	emu_timer *m_hsync_timer;       // horizontal sync timer
	emu_timer *m_blank_timer;       // CRT blanking timer

	const address_space_config      m_space_config;
};


// device type definition
extern const device_type UPD7220;



#endif
