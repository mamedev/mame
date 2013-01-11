/**********************************************************************

    SMC CRT9007 CRT Video Processor and Controller (VPAC) emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************
                            _____   _____
                   VA2   1 |*    \_/     | 40  GND
                  VA10   2 |             | 39  VA9
                   VA3   3 |             | 38  VA1
                  VA11   4 |             | 37  VA8
                  VA12   5 |             | 36  VA0
                   VA4   6 |             | 35  CBLANK
                  VA13   7 |             | 34  CURS
                   VA5   8 |             | 33  ACK/_TSC
                   VA6   9 |             | 32  _CSYNC/LPSTB
                   VA7  10 |   CRT9007   | 31  SLD/SL0
                   VLT  11 |             | 30  _SLG/SL1
                   _VS  12 |             | 29  WBEN/SL2/_CSYNC
                   _HS  13 |             | 28  DMAR/SL3/VBLANK
                 _CCLK  14 |             | 27  INT
                  _DRB  15 |             | 26  _RST
                   VD7  16 |             | 25  _CS
                   VD6  17 |             | 24  VD0
                   VD5  18 |             | 23  VD1
                   VD4  19 |             | 22  VD2
                   VD3  20 |_____________| 21  +5V

**********************************************************************/

#pragma once

#ifndef __CRT9007__
#define __CRT9007__

#include "emu.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************




//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_CRT9007_ADD(_tag, _clock, _config, _map) \
	MCFG_DEVICE_ADD(_tag, CRT9007, _clock) \
	MCFG_DEVICE_CONFIG(_config) \
	MCFG_DEVICE_ADDRESS_MAP(AS_0, _map)


#define CRT9007_INTERFACE(name) \
	const crt9007_interface (name) =



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> crt9007_interface

struct crt9007_interface
{
	const char *m_screen_tag;       /* screen we are acting on */
	int hpixels_per_column;     /* number of pixels per video memory address */

	devcb_write_line        m_out_int_cb;
	devcb_write_line        m_out_dmar_cb;

	devcb_write_line        m_out_vs_cb;
	devcb_write_line        m_out_hs_cb;

	devcb_write_line        m_out_vlt_cb;
	devcb_write_line        m_out_curs_cb;
	devcb_write_line        m_out_drb_cb;
	devcb_write_line        m_out_cblank_cb;

	devcb_write_line        m_out_slg_cb;
	devcb_write_line        m_out_sld_cb;
};



// ======================> crt9007_device

class crt9007_device :  public device_t,
						public device_memory_interface,
						public crt9007_interface
{
public:
	// construction/destruction
	crt9007_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_WRITE_LINE_MEMBER( ack_w );
	DECLARE_WRITE_LINE_MEMBER( lpstb_w );
	DECLARE_READ_LINE_MEMBER( vlt_r );
	DECLARE_READ_LINE_MEMBER( wben_r );

	void set_hpixels_per_column(int hpixels_per_column);

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
	virtual void device_clock_changed();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const;

private:
	static const device_timer_id TIMER_HSYNC = 0;
	static const device_timer_id TIMER_VSYNC = 1;
	static const device_timer_id TIMER_VLT = 2;
	static const device_timer_id TIMER_CURS = 3;
	static const device_timer_id TIMER_DRB = 4;
	static const device_timer_id TIMER_DMA = 5;

	inline UINT8 readbyte(offs_t address);

	inline void trigger_interrupt(int line);
	inline void update_cblank_line();
	inline void update_hsync_timer(int state);
	inline void update_vsync_timer(int state);
	inline void update_vlt_timer(int state);
	inline void update_curs_timer(int state);
	inline void update_drb_timer(int state);
	inline void update_dma_timer();

	inline void recompute_parameters();

	devcb_resolved_write_line   m_out_int_func;
	devcb_resolved_write_line   m_out_dmar_func;
	devcb_resolved_write_line   m_out_hs_func;
	devcb_resolved_write_line   m_out_vs_func;
	devcb_resolved_write_line   m_out_vlt_func;
	devcb_resolved_write_line   m_out_curs_func;
	devcb_resolved_write_line   m_out_drb_func;
	devcb_resolved_write_line   m_out_cblank_func;
	devcb_resolved_write_line   m_out_slg_func;
	devcb_resolved_write_line   m_out_sld_func;

	screen_device *m_screen;

	// registers
	UINT8 m_reg[0x3d];
	UINT8 m_status;

	int m_disp;
	int m_hpixels_per_column;

	// runtime variables, do not state save
	int m_vsync_start;
	int m_vsync_end;
	int m_hsync_start;
	int m_hsync_end;
	int m_vlt_start;
	int m_vlt_end;
	int m_vlt_bottom;
	int m_drb_bottom;
	int m_hs;
	int m_vs;
	int m_cblank;
	int m_vlt;
	int m_drb;
	int m_wben;
	int m_slg;
	int m_sld;
	int m_lpstb;

	// DMA
	int m_dmar;
	int m_ack;
	int m_dma_count;
	int m_dma_burst;
	int m_dma_delay;

	// timers
	emu_timer *m_vsync_timer;
	emu_timer *m_hsync_timer;
	emu_timer *m_vlt_timer;
	emu_timer *m_curs_timer;
	emu_timer *m_drb_timer;
	emu_timer *m_dma_timer;

	// address space configurations
	const address_space_config      m_space_config;
};


// device type definition
extern const device_type CRT9007;



#endif
