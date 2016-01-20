// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    MOS 6566/6567/6569 Video Interface Chip II (VIC-II) emulation

****************************************************************************
                            _____   _____
                   DB6   1 |*    \_/     | 40  Vcc
                   DB5   2 |             | 39  DB7
                   DB4   3 |             | 38  DB8
                   DB3   4 |             | 37  DB9
                   DB2   5 |             | 36  DB10
                   DB1   6 |             | 35  DB11
                   DB0   7 |             | 34  A13
                  _IRQ   8 |             | 33  A12
                    LP   9 |             | 32  A11
                   _CS  10 |   MOS6566   | 31  A10
                   R/W  11 |             | 30  A9
                    BA  12 |             | 29  A8
                   Vdd  13 |             | 28  A7
                 COLOR  14 |             | 27  A6
                 S/LUM  15 |             | 26  A5
                   AEC  16 |             | 25  A4
                   PH0  17 |             | 24  A3
                  PHIN  18 |             | 23  A2
                 PHCOL  19 |             | 22  A1
                   Vss  20 |_____________| 21  A0

                            _____   _____
                   DB6   1 |*    \_/     | 40  Vcc
                   DB5   2 |             | 39  DB7
                   DB4   3 |             | 38  DB8
                   DB3   4 |             | 37  DB9
                   DB2   5 |             | 36  DB10
                   DB1   6 |             | 35  DB11
                   DB0   7 |             | 34  A10
                  _IRQ   8 |             | 33  A9
                    LP   9 |   MOS6567   | 32  A8
                   _CS  10 |   MOS6569   | 31  A7
                   R/W  11 |   MOS8562   | 30  A6
                    BA  12 |   MOS8565   | 29  A5/A13
                   Vdd  13 |             | 28  A4/A12
                 COLOR  14 |             | 27  A3/A11
                 S/LUM  15 |             | 26  A2/A10
                   AEC  16 |             | 25  A1/A9
                   PH0  17 |             | 24  A0/A8
                  _RAS  18 |             | 23  A11
                   CAS  19 |             | 22  PHIN
                   Vss  20 |_____________| 21  PHCL

                            _____   _____
                    D6   1 |*    \_/     | 48  Vcc
                    D5   2 |             | 47  D7
                    D4   3 |             | 46  D8
                    D3   4 |             | 45  D9
                    D2   5 |             | 44  D10
                    D1   6 |             | 43  D11
                    D0   7 |             | 42  MA10
                  _IRQ   8 |             | 41  MA9
                   _LP   9 |             | 40  MA8
                    BA  10 |             | 39  A7
              _DMARQST  11 |             | 38  A6
                   AEC  12 |   MOS8564   | 37  MA5
                   _CS  13 |   MOS8566   | 36  MA4
                   R/W  14 |             | 35  MA3
               _DMAACK  15 |             | 34  MA2
                CHROMA  16 |             | 33  MA1
              SYNC/LUM  17 |             | 32  MA0
                 1 MHZ  18 |             | 31  MA11
                  _RAS  19 |             | 30  PHI IN
                  _CAS  20 |             | 29  PHI COLOR
                   MUX  21 |             | 28  K2
                _IOACC  22 |             | 27  K1
                 2 MHZ  23 |             | 26  K0
                   Vss  24 |_____________| 25  Z80 PHI

***************************************************************************/

#pragma once

#ifndef __MOS6566__
#define __MOS6566__

#include "emu.h"



//***************************************************************************
// DEVICE CONFIGURATION MACROS
//***************************************************************************

#define MCFG_MOS6566_CPU(_tag) \
	mos6566_device::static_set_cpu_tag(*device, "^" _tag);

#define MCFG_MOS6566_IRQ_CALLBACK(_write) \
	devcb = &mos6566_device::set_irq_wr_callback(*device, DEVCB_##_write);

#define MCFG_MOS6566_BA_CALLBACK(_write) \
	devcb = &mos6566_device::set_ba_wr_callback(*device, DEVCB_##_write);

#define MCFG_MOS6566_AEC_CALLBACK(_write) \
	devcb = &mos6566_device::set_aec_wr_callback(*device, DEVCB_##_write);

#define MCFG_MOS8564_K_CALLBACK(_write) \
	devcb = &mos6566_device::set_k_wr_callback(*device, DEVCB_##_write);



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define VIC6566_CLOCK           (XTAL_8MHz / 8) // 1000000
#define VIC6567R56A_CLOCK       (XTAL_8MHz / 8) // 1000000
#define VIC6567_CLOCK           (XTAL_14_31818MHz / 14) // 1022727
#define VIC6569_CLOCK           (XTAL_17_734472MHz / 18) // 985248

#define VIC6566_DOTCLOCK        (VIC6566_CLOCK * 8) // 8000000
#define VIC6567R56A_DOTCLOCK    (VIC6567R56A_CLOCK * 8) // 8000000
#define VIC6567_DOTCLOCK        (VIC6567_CLOCK * 8) // 8181818
#define VIC6569_DOTCLOCK        (VIC6569_CLOCK * 8) // 7881988

#define VIC6567_CYCLESPERLINE   65
#define VIC6569_CYCLESPERLINE   63

#define VIC6567_LINES       263
#define VIC6569_LINES       312

#define VIC6566_VRETRACERATE        ((float)VIC6566_CLOCK / 262 / 64)
#define VIC6567R56A_VRETRACERATE    ((float)VIC6567R56A_CLOCK / 262 / 64)
#define VIC6567_VRETRACERATE        ((float)VIC6567_CLOCK / 263 / 65)
#define VIC6569_VRETRACERATE        ((float)VIC6569_CLOCK / 312 / 63)

#define VIC6566_HRETRACERATE    (VIC6566_CLOCK / VIC6566_CYCLESPERLINE)
#define VIC6567_HRETRACERATE    (VIC6567_CLOCK / VIC6567_CYCLESPERLINE)
#define VIC6569_HRETRACERATE    (VIC6569_CLOCK / VIC6569_CYCLESPERLINE)

#define VIC2_HSIZE      320
#define VIC2_VSIZE      200

#define VIC6567_VISIBLELINES    235
#define VIC6569_VISIBLELINES    284

#define VIC6567_FIRST_DMA_LINE  0x30
#define VIC6569_FIRST_DMA_LINE  0x30

#define VIC6567_LAST_DMA_LINE   0xf7
#define VIC6569_LAST_DMA_LINE   0xf7

#define VIC6567_FIRST_DISP_LINE 0x29
#define VIC6569_FIRST_DISP_LINE 0x10

#define VIC6567_LAST_DISP_LINE  (VIC6567_FIRST_DISP_LINE + VIC6567_VISIBLELINES - 1)
#define VIC6569_LAST_DISP_LINE  (VIC6569_FIRST_DISP_LINE + VIC6569_VISIBLELINES - 1)

#define VIC6567_RASTER_2_EMU(a) ((a >= VIC6567_FIRST_DISP_LINE) ? (a - VIC6567_FIRST_DISP_LINE) : (a + 222))
#define VIC6569_RASTER_2_EMU(a) (a - VIC6569_FIRST_DISP_LINE)

#define VIC6567_FIRSTCOLUMN 50
#define VIC6569_FIRSTCOLUMN 50

#define VIC6567_VISIBLECOLUMNS  418
#define VIC6569_VISIBLECOLUMNS  403

#define VIC6567_X_2_EMU(a)  (a)
#define VIC6569_X_2_EMU(a)  (a)

#define VIC6567_STARTVISIBLELINES ((VIC6567_LINES - VIC6567_VISIBLELINES)/2)
#define VIC6569_STARTVISIBLELINES 16 /* ((VIC6569_LINES - VIC6569_VISIBLELINES)/2) */

#define VIC6567_FIRSTRASTERLINE 34
#define VIC6569_FIRSTRASTERLINE 0

#define VIC6567_COLUMNS 512
#define VIC6569_COLUMNS 504

#define VIC6567_STARTVISIBLECOLUMNS ((VIC6567_COLUMNS - VIC6567_VISIBLECOLUMNS)/2)
#define VIC6569_STARTVISIBLECOLUMNS ((VIC6569_COLUMNS - VIC6569_VISIBLECOLUMNS)/2)

#define VIC6567_FIRSTRASTERCOLUMNS 412
#define VIC6569_FIRSTRASTERCOLUMNS 404

#define VIC6569_FIRST_X 0x194
#define VIC6567_FIRST_X 0x19c

#define VIC6569_FIRST_VISIBLE_X 0x1e0
#define VIC6567_FIRST_VISIBLE_X 0x1e8

#define VIC6569_MAX_X 0x1f7
#define VIC6567_MAX_X 0x1ff

#define VIC6569_LAST_VISIBLE_X 0x17c
#define VIC6567_LAST_VISIBLE_X 0x184

#define VIC6569_LAST_X 0x193
#define VIC6567_LAST_X 0x19b



//***************************************************************************
//  TYPE DEFINITIONS
//***************************************************************************

// ======================> mos6566_device

class mos6566_device :  public device_t,
						public device_memory_interface,
						public device_video_interface,
						public device_execute_interface
{
public:
	// construction/destruction
	mos6566_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, UINT32 variant, std::string shortname, std::string source);
	mos6566_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	static void static_set_cpu_tag(device_t &device, std::string tag) { downcast<mos6566_device &>(device).m_cpu.set_tag(tag); }
	template<class _Object> static devcb_base &set_irq_wr_callback(device_t &device, _Object object) { return downcast<mos6566_device &>(device).m_write_irq.set_callback(object); }
	template<class _Object> static devcb_base &set_ba_wr_callback(device_t &device, _Object object) { return downcast<mos6566_device &>(device).m_write_ba.set_callback(object); }
	template<class _Object> static devcb_base &set_aec_wr_callback(device_t &device, _Object object) { return downcast<mos6566_device &>(device).m_write_aec.set_callback(object); }
	template<class _Object> static devcb_base &set_k_wr_callback(device_t &device, _Object object) { return downcast<mos6566_device &>(device).m_write_k.set_callback(object); }

	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_WRITE_LINE_MEMBER( lp_w );

	DECLARE_READ_LINE_MEMBER( phi0_r ) { return m_phi0; } // phi 0
	DECLARE_READ_LINE_MEMBER( ba_r )   { return m_ba; }   // bus available
	DECLARE_READ_LINE_MEMBER( aec_r )  { return m_aec; }  // address enable control

	UINT8 bus_r() { return m_last_data; }

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	enum
	{
		TYPE_6566,  // NTSC-M (SRAM)
		TYPE_6567,  // NTSC-M (NMOS)
		TYPE_8562,  // NTSC-M (HMOS)
		TYPE_8564,  // NTSC-M VIC-IIe (C128)

		TYPE_6569,  // PAL-B
		TYPE_6572,  // PAL-N
		TYPE_6573,  // PAL-M
		TYPE_8565,  // PAL-B (HMOS)
		TYPE_8566,  // PAL-B VIC-IIe (C128)
		TYPE_8569   // PAL-N VIC-IIe (C128)
	};

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void execute_run() override;

	int m_icount;
	int m_variant;

	const address_space_config      m_videoram_space_config;
	const address_space_config      m_colorram_space_config;

	inline void set_interrupt( int mask );
	inline void clear_interrupt( int mask );
	inline void set_ba(int state);
	inline void set_aec(int state);
	inline void bad_line_ba();
	inline UINT8 read_videoram(offs_t offset);
	inline UINT8 read_colorram(offs_t offset);
	inline void idle_access();
	inline void spr_ba(int num);
	inline void spr_ptr_access( int num );
	inline void spr_data_access( int num, int bytenum );
	inline void display_if_bad_line();
	inline void refresh_access();
	inline void fetch_if_bad_line();
	inline void rc_if_bad_line();
	inline void sample_border();
	inline void check_sprite_dma();
	inline void matrix_access();
	inline void graphics_access();
	inline void draw_background();
	inline void draw_mono( UINT16 p, UINT8 c0, UINT8 c1 );
	inline void draw_multi( UINT16 p, UINT8 c0, UINT8 c1, UINT8 c2, UINT8 c3 );
	void draw_graphics();
	void draw_sprites();

	devcb_write_line       m_write_irq;
	devcb_write_line       m_write_ba;
	devcb_write_line       m_write_aec;
	devcb_write8           m_write_k;

	required_device<cpu_device> m_cpu;

	int m_phi0;
	int m_ba;
	int m_aec;
	UINT8 m_aec_delay;
	int m_rdy_cycles;

	UINT8 m_reg[0x80];

	int m_on;                               /* rastering of the screen */

	UINT16 m_chargenaddr, m_videoaddr, m_bitmapaddr;

	bitmap_rgb32 m_bitmap;

	UINT16 m_colors[4], m_spritemulti[4];

	int m_rasterline;
	UINT8 m_cycle;
	UINT16 m_raster_x;
	UINT16 m_graphic_x;
	UINT8 m_last_data;
	int m_lp;

	/* convert multicolor byte to background/foreground for sprite collision */
	UINT16 m_expandx[256];
	UINT16 m_expandx_multi[256];

	/* Display */
	UINT16 m_dy_start;
	UINT16 m_dy_stop;

	/* GFX */
	UINT8 m_draw_this_line;
	UINT8 m_is_bad_line;
	UINT8 m_bad_lines_enabled;
	UINT8 m_display_state;
	UINT8 m_char_data;
	UINT8 m_gfx_data;
	UINT8 m_color_data;
	UINT8 m_last_char_data;
	UINT8 m_matrix_line[40];                        // Buffer for video line, read in Bad Lines
	UINT8 m_color_line[40];                     // Buffer for color line, read in Bad Lines
	UINT8 m_vblanking;
	UINT16 m_ml_index;
	UINT8 m_rc;
	UINT16 m_vc;
	UINT16 m_vc_base;
	UINT8 m_ref_cnt;

	/* Sprites */
	UINT8 m_spr_coll_buf[0x400];                    // Buffer for sprite-sprite collisions and priorities
	UINT8 m_fore_coll_buf[0x400];                   // Buffer for foreground-sprite collisions and priorities
	UINT8 m_spr_draw_data[8][4];                    // Sprite data for drawing
	UINT8 m_spr_exp_y;
	UINT8 m_spr_dma_on;
	UINT8 m_spr_draw;
	UINT8 m_spr_disp_on;
	UINT16 m_spr_ptr[8];
	UINT8 m_spr_data[8][4];
	UINT16 m_mc_base[8];                        // Sprite data counter bases
	UINT16 m_mc[8];                         // Sprite data counters

	/* Border */
	UINT8 m_border_on;
	UINT8 m_ud_border_on;
	UINT8 m_border_on_sample[5];
	UINT8 m_border_color_sample[0x400 / 8];         // Samples of border color at each "displayed" cycle

	/* Cycles */
	UINT64 m_first_ba_cycle;
	UINT8 m_device_suspended;
};


// ======================> mos6567_device

class mos6567_device :  public mos6566_device
{
public:
	// construction/destruction
	mos6567_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	mos6567_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, UINT32 variant, std::string shortname, std::string source);
};


// ======================> mos8562_device

class mos8562_device :  public mos6567_device
{
public:
	// construction/destruction
	mos8562_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};


// ======================> mos8564_device

class mos8564_device :  public mos6567_device
{
public:
	// construction/destruction
	mos8564_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device_execute_interface overrides
	virtual UINT64 execute_clocks_to_cycles(UINT64 clocks) const override { return (clocks / 8); }
	virtual UINT64 execute_cycles_to_clocks(UINT64 cycles) const override { return (cycles * 8); }
};


// ======================> mos6569_device

class mos6569_device :  public mos6566_device
{
public:
	// construction/destruction
	mos6569_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	mos6569_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, UINT32 variant, std::string shortname, std::string source);

	// device-level overrides
	virtual void execute_run() override;
};


// ======================> mos8565_device

class mos8565_device :  public mos6569_device
{
public:
	// construction/destruction
	mos8565_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};


// ======================> mos8566_device

class mos8566_device :  public mos6569_device
{
public:
	// construction/destruction
	mos8566_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device_execute_interface overrides
	virtual UINT64 execute_clocks_to_cycles(UINT64 clocks) const override { return (clocks / 8); }
	virtual UINT64 execute_cycles_to_clocks(UINT64 cycles) const override { return (cycles * 8); }
};


// device type definitions
extern const device_type MOS6566;
extern const device_type MOS6567;
extern const device_type MOS8562;
extern const device_type MOS8564;
extern const device_type MOS6569;
extern const device_type MOS8565;
extern const device_type MOS8566;



#endif
