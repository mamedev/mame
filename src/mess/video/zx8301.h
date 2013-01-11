/**********************************************************************

    Sinclair ZX8301 emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************
                            _____   _____
                DTACKL   1 |*    \_/     | 40  WEL
                   A17   2 |             | 39  PCENL
                   A16   3 |             | 38  VDA
                  RDWL   4 |             | 37  ROWL
                 DSMCL   5 |             | 36  TX0EL
                   VCC   6 |             | 35  XTAL2
                CLKCPU   7 |             | 34  XTAL1
                  RASL   8 |             | 33  ROM0EH
                 CAS0L   9 |             | 32  BLUE
                 CAS1L  10 |    ZX8301   | 31  GREEN
                VSYNCH  11 |     ULA     | 30  RED
                CSYNCL  12 |             | 29  DB7
                   DA0  13 |             | 28  DA7
                   DB0  14 |             | 27  DA6
                   VDD  15 |             | 26  DB6
                   DB1  16 |             | 25  DB5
                   DA1  17 |             | 24  DA5
                   DA2  18 |             | 23  DB4
                   DB2  19 |             | 22  DA4
                   DA3  20 |_____________| 21  DB3

**********************************************************************/

#pragma once

#ifndef __ZX8301__
#define __ZX8301__

#include "emu.h"



///*************************************************************************
//  MACROS / CONSTANTS
///*************************************************************************




///*************************************************************************
//  INTERFACE CONFIGURATION MACROS
///*************************************************************************

#define MCFG_ZX8301_ADD(_tag, _clock, _config) \
	MCFG_DEVICE_ADD(_tag, ZX8301, _clock) \
	MCFG_DEVICE_CONFIG(_config)

#define ZX8301_INTERFACE(name) \
	const zx8301_interface(name) =



///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************

// ======================> zx8301_interface

struct zx8301_interface
{
	const char *cpu_tag;
	const char *screen_tag;

	devcb_write_line    out_vsync_cb;
};


// ======================> zx8301_device

class zx8301_device :   public device_t,
						public device_memory_interface,
						public zx8301_interface
{
public:
	// construction/destruction
	zx8301_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_WRITE8_MEMBER( control_w );
	DECLARE_READ8_MEMBER( data_r );
	DECLARE_WRITE8_MEMBER( data_w );

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	virtual void device_config_complete();

	// device_config_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const;

	// address space configurations
	const address_space_config      m_space_config;

	inline UINT8 readbyte(offs_t address);
	inline void writebyte(offs_t address, UINT8 data);

	void draw_line_mode4(bitmap_rgb32 &bitmap, int y, UINT16 da);
	void draw_line_mode8(bitmap_rgb32 &bitmap, int y, UINT16 da);

private:
	static const device_timer_id TIMER_VSYNC = 0;
	static const device_timer_id TIMER_FLASH = 1;

	devcb_resolved_write_line   m_out_vsync_func;

	cpu_device *m_cpu;
	screen_device *m_screen;
	address_space *m_data;

	int m_dispoff;                  // display off
	int m_mode8;                    // mode8 active
	int m_base;                     // video ram base address
	int m_flash;                    // flash
	int m_vsync;                    // vertical sync
	int m_vda;                      // valid data address

	emu_timer *m_vsync_timer;       // vertical sync timer
	emu_timer *m_flash_timer;       // flash timer
};


// device type definition
extern const device_type ZX8301;



#endif
