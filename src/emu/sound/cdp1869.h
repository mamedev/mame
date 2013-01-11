/**********************************************************************

    RCA CDP1869/1870/1876 Video Interface System (VIS) emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************
                            _____   _____
                   TPA   1 |*    \_/     | 40  Vdd
                   TPB   2 |             | 39  PMSEL
                  _MRD   3 |             | 38  _PMWR
                  _MWR   4 |             | 37  CMSEL
                 MA0/8   5 |             | 36  _CMWR
                 MA1/9   6 |             | 35  PMA0
                MA2/10   7 |             | 34  PMA1
                MA3/11   8 |             | 33  PMA2
                MA4/12   9 |             | 32  PMA3
                MA5/13  10 |             | 31  PMA4
                MA6/14  11 |   CDP1869   | 30  PMA5
                MA7/15  12 |             | 29  PMA6
                    N0  13 |             | 28  PMA7
                    N1  14 |             | 27  PMA8
                    N2  15 |             | 26  PMA9
               _H SYNC  16 |             | 25  CMA3/PMA10
              _DISPLAY  17 |             | 24  CMA2
              _ADDRSTB  18 |             | 23  CMA1
                 SOUND  19 |             | 22  CMA0
                   Vss  20 |_____________| 21  _N=3

                            _____   _____
           _PREDISPLAY   1 |*    \_/     | 40  Vdd
              _DISPLAY   2 |             | 39  PAL/_NTSC
                   PCB   3 |             | 38  CPUCLK
                  CCB1   4 |             | 37  XTAL (DOT)
                  BUS7   5 |             | 36  _XTAL (DOT)
                  CCB0   6 |             | 35  _ADDRSTB
                  BUS6   7 |             | 34  _MRD
                  CDB5   8 |             | 33  TPB
                  BUS5   9 |             | 32  CMSEL
                  CDB4  10 |             | 31  BURST
                  BUS4  11 |   CDP1870   | 30  _H SYNC
                  CDB3  12 |             | 29  _COMPSYNC
                  BUS3  13 |             | 28  LUM
                  CDB2  14 |             | 27  PAL CHROM
                  BUS2  15 |             | 26  NTSC CHROM
                  CDB1  16 |             | 25  _XTAL (CHROM)
                  BUS1  17 |             | 24  XTAL (CHROM)
                  CDB0  18 |             | 23  _EMS
                  BUS0  19 |             | 22  _EVS
                   Vss  20 |_____________| 21  _N=3

                            _____   _____
           _PREDISPLAY   1 |*    \_/     | 40  Vdd
              _DISPLAY   2 |             | 39  PAL/_NTSC
                   PCB   3 |             | 38  CPUCLK
                  CCB1   4 |             | 37  XTAL (DOT)
                  BUS7   5 |             | 36  _XTAL (DOT)
                  CCB0   6 |             | 35  _ADDRSTB
                  BUS6   7 |             | 34  _MRD
                  CDB5   8 |             | 33  TPB
                  BUS5   9 |             | 32  CMSEL
                  CDB4  10 |             | 31  BURST
                  BUS4  11 |   CDP1876   | 30  _H SYNC
                  CDB3  12 |             | 29  _COMPSYNC
                  BUS3  13 |             | 28  RED
                  CDB2  14 |             | 27  BLUE
                  BUS2  15 |             | 26  GREEN
                  CDB1  16 |             | 25  _XTAL (CHROM)
                  BUS1  17 |             | 24  XTAL (CHROM)
                  CDB0  18 |             | 23  _EMS
                  BUS0  19 |             | 22  _EVS
                   Vss  20 |_____________| 21  _N=3

**********************************************************************/

#pragma once

#ifndef __CDP1869__
#define __CDP1869__

#include "emu.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define CDP1869_DOT_CLK_PAL         (float)XTAL_5_626MHz
#define CDP1869_DOT_CLK_NTSC        (float)XTAL_5_67MHz
#define CDP1869_COLOR_CLK_PAL       (float)XTAL_8_867236MHz
#define CDP1869_COLOR_CLK_NTSC      (float)XTAL_7_15909MHz

#define CDP1869_CPU_CLK_PAL         (CDP1869_DOT_CLK_PAL / 2)
#define CDP1869_CPU_CLK_NTSC        (CDP1869_DOT_CLK_NTSC / 2)

#define CDP1869_CHAR_WIDTH          6

#define CDP1869_HSYNC_START         (56 * CDP1869_CHAR_WIDTH)
#define CDP1869_HSYNC_END           (60 * CDP1869_CHAR_WIDTH)
#define CDP1869_HBLANK_START        (54 * CDP1869_CHAR_WIDTH)
#define CDP1869_HBLANK_END          ( 5 * CDP1869_CHAR_WIDTH)
#define CDP1869_SCREEN_START_PAL    ( 9 * CDP1869_CHAR_WIDTH)
#define CDP1869_SCREEN_START_NTSC   (10 * CDP1869_CHAR_WIDTH)
#define CDP1869_SCREEN_START        (10 * CDP1869_CHAR_WIDTH)
#define CDP1869_SCREEN_END          (50 * CDP1869_CHAR_WIDTH)
#define CDP1869_SCREEN_WIDTH        (60 * CDP1869_CHAR_WIDTH)

#define CDP1869_TOTAL_SCANLINES_PAL             312
#define CDP1869_SCANLINE_VBLANK_START_PAL       304
#define CDP1869_SCANLINE_VBLANK_END_PAL         10
#define CDP1869_SCANLINE_VSYNC_START_PAL        308
#define CDP1869_SCANLINE_VSYNC_END_PAL          312
#define CDP1869_SCANLINE_DISPLAY_START_PAL      44
#define CDP1869_SCANLINE_DISPLAY_END_PAL        260
#define CDP1869_SCANLINE_PREDISPLAY_START_PAL   43
#define CDP1869_SCANLINE_PREDISPLAY_END_PAL     260
#define CDP1869_VISIBLE_SCANLINES_PAL           (CDP1869_SCANLINE_DISPLAY_END_PAL - CDP1869_SCANLINE_DISPLAY_START_PAL)

#define CDP1869_TOTAL_SCANLINES_NTSC            262
#define CDP1869_SCANLINE_VBLANK_START_NTSC      252
#define CDP1869_SCANLINE_VBLANK_END_NTSC        10
#define CDP1869_SCANLINE_VSYNC_START_NTSC       258
#define CDP1869_SCANLINE_VSYNC_END_NTSC         262
#define CDP1869_SCANLINE_DISPLAY_START_NTSC     36
#define CDP1869_SCANLINE_DISPLAY_END_NTSC       228
#define CDP1869_SCANLINE_PREDISPLAY_START_NTSC  35
#define CDP1869_SCANLINE_PREDISPLAY_END_NTSC    228
#define CDP1869_VISIBLE_SCANLINES_NTSC          (CDP1869_SCANLINE_DISPLAY_END_NTSC - CDP1869_SCANLINE_DISPLAY_START_NTSC)

#define CDP1869_PALETTE_LENGTH  8+64



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_CDP1869_ADD(_tag, _pixclock, _config, _map) \
	MCFG_DEVICE_ADD(_tag, CDP1869, _pixclock) \
	MCFG_DEVICE_CONFIG(_config) \
	MCFG_DEVICE_ADDRESS_MAP(AS_0, _map)

#define MCFG_CDP1869_SCREEN_PAL_ADD(_cdptag, _tag, _clock) \
	MCFG_SCREEN_ADD(_tag, RASTER) \
	MCFG_SCREEN_UPDATE_DEVICE(_cdptag, cdp1869_device, screen_update) \
	MCFG_SCREEN_RAW_PARAMS(_clock, CDP1869_SCREEN_WIDTH, CDP1869_HBLANK_END, CDP1869_HBLANK_START, CDP1869_TOTAL_SCANLINES_PAL, CDP1869_SCANLINE_VBLANK_END_PAL, CDP1869_SCANLINE_VBLANK_START_PAL)

#define MCFG_CDP1869_SCREEN_NTSC_ADD(_cdptag, _tag, _clock) \
	MCFG_SCREEN_ADD(_tag, RASTER) \
	MCFG_SCREEN_UPDATE_DEVICE(_cdptag, cdp1869_device, screen_update) \
	MCFG_SCREEN_RAW_PARAMS(_clock, CDP1869_SCREEN_WIDTH, CDP1869_HBLANK_END, CDP1869_HBLANK_START, CDP1869_TOTAL_SCANLINES_NTSC, CDP1869_SCANLINE_VBLANK_END_NTSC, CDP1869_SCANLINE_VBLANK_START_NTSC)

#define CDP1869_INTERFACE(_name) \
	const cdp1869_interface (_name) =

#define CDP1869_CHAR_RAM_READ(name) UINT8 name(device_t *device, UINT16 pma, UINT8 cma, UINT8 pmd)
#define CDP1869_CHAR_RAM_WRITE(name) void name(device_t *device, UINT16 pma, UINT8 cma, UINT8 pmd, UINT8 data)
#define CDP1869_PCB_READ(name) int name(device_t *device, UINT16 pma, UINT8 cma, UINT8 pmd)

#define CDP1869_PAL \
	DEVCB_LINE_VCC

#define CDP1869_NTSC \
	DEVCB_LINE_GND



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

typedef UINT8 (*cdp1869_char_ram_read_func)(device_t *device, UINT16 pma, UINT8 cma, UINT8 pmd);
typedef void (*cdp1869_char_ram_write_func)(device_t *device, UINT16 pma, UINT8 cma, UINT8 pmd, UINT8 data);
typedef int (*cdp1869_pcb_read_func)(device_t *device, UINT16 pma, UINT8 cma, UINT8 pmd);


// ======================> cdp1869_interface

struct cdp1869_interface
{
	const char *screen_tag;     // screen we are acting on

	// pixel clock of the chip is the device clock
	int color_clock;            // the chroma clock of the chip

	// screen format
	devcb_read_line                 in_pal_ntsc_cb;

	// page memory color bit read function
	cdp1869_pcb_read_func           in_pcb_cb;

	// character memory read function
	cdp1869_char_ram_read_func      in_char_ram_cb;

	// character memory write function
	cdp1869_char_ram_write_func     out_char_ram_cb;

	// if specified, this gets called for every change of the predisplay pin (CDP1870/76 pin 1)
	devcb_write_line                out_prd_cb;
};



// ======================> cdp1869_device

class cdp1869_device :  public device_t,
						public device_sound_interface,
						public device_memory_interface,
						public cdp1869_interface
{
public:
	// construction/destruction
	cdp1869_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_WRITE8_MEMBER( out3_w );
	DECLARE_WRITE8_MEMBER( out4_w );
	DECLARE_WRITE8_MEMBER( out5_w );
	DECLARE_WRITE8_MEMBER( out6_w );
	DECLARE_WRITE8_MEMBER( out7_w );

	DECLARE_READ8_MEMBER( char_ram_r );
	DECLARE_WRITE8_MEMBER( char_ram_w );

	DECLARE_READ8_MEMBER( page_ram_r );
	DECLARE_WRITE8_MEMBER( page_ram_w );

	DECLARE_READ_LINE_MEMBER( predisplay_r );
	DECLARE_READ_LINE_MEMBER( pal_ntsc_r );

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_post_load();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const;

	// device_sound_interface callbacks
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

	inline bool is_ntsc();
	inline UINT8 read_page_ram_byte(offs_t address);
	inline void write_page_ram_byte(offs_t address, UINT8 data);
	inline UINT8 read_char_ram_byte(offs_t pma, offs_t cma, UINT8 pmd);
	inline void write_char_ram_byte(offs_t pma, offs_t cma, UINT8 pmd, UINT8 data);
	inline int read_pcb(offs_t pma, offs_t cma, UINT8 pmd);
	inline void update_prd_changed_timer();
	inline rgb_t get_rgb(int i, int c, int l);
	inline int get_lines();
	inline UINT16 get_pmemsize(int cols, int rows);
	inline UINT16 get_pma();
	inline int get_pen(int ccb0, int ccb1, int pcb);

	void initialize_palette();
	void draw_line(bitmap_rgb32 &bitmap, const rectangle &rect, int x, int y, UINT8 data, int color);
	void draw_char(bitmap_rgb32 &bitmap, const rectangle &rect, int x, int y, UINT16 pma);

private:
	devcb_resolved_read_line        m_in_pal_ntsc_func;
	devcb_resolved_write_line       m_out_prd_func;
	cdp1869_pcb_read_func           m_in_pcb_func;
	cdp1869_char_ram_read_func      m_in_char_ram_func;
	cdp1869_char_ram_write_func     m_out_char_ram_func;

	screen_device *m_screen;
	address_space *m_page_ram;
	emu_timer *m_prd_timer;
	sound_stream *m_stream;

	// video state
	rgb_t m_palette[8+64];
	int m_prd;                      // predisplay
	int m_dispoff;                  // display off
	int m_fresvert;                 // full resolution vertical
	int m_freshorz;                 // full resolution horizontal
	int m_cmem;                     // character memory access mode
	int m_dblpage;                  // double page mode
	int m_line16;                   // 16-line hi-res mode
	int m_line9;                    // 9 line mode
	int m_cfc;                      // color format control
	UINT8 m_col;                    // character color control
	UINT8 m_bkg;                    // background color
	UINT16 m_pma;                   // page memory address
	UINT16 m_hma;                   // home memory address

	// sound state
	INT16 m_signal;                 // current signal
	int m_incr;                     // initial wave state
	int m_toneoff;                  // tone off
	int m_wnoff;                    // white noise off
	UINT8 m_tonediv;                // tone divisor
	UINT8 m_tonefreq;               // tone range select
	UINT8 m_toneamp;                // tone output amplitude
	UINT8 m_wnfreq;                 // white noise range select
	UINT8 m_wnamp;                  // white noise output amplitude

	const address_space_config      m_space_config;
};


// device type definition
extern const device_type CDP1869;



#endif
