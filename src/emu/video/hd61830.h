/**********************************************************************

    HD61830 LCD Timing Controller emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __HD61830__
#define __HD61830__

#include "emu.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************




//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_HD61830_ADD(_tag, _clock, _config) \
	MCFG_DEVICE_ADD(_tag, HD61830, _clock) \
	MCFG_DEVICE_CONFIG(_config)


#define HD61830_INTERFACE(name) \
	const hd61830_interface (name) =



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> hd61830_interface

struct hd61830_interface
{
	const char *screen_tag;

	devcb_read8 m_in_rd_cb;
};



// ======================> hd61830_device

class hd61830_device :  public device_t,
						public device_memory_interface,
						public hd61830_interface
{
public:
	// construction/destruction
	hd61830_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER( status_r );
	DECLARE_WRITE8_MEMBER( control_w );

	DECLARE_READ8_MEMBER( data_r );
	DECLARE_WRITE8_MEMBER( data_w );

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual const rom_entry *device_rom_region() const;
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const;

	inline UINT8 readbyte(offs_t address);
	inline void writebyte(offs_t address, UINT8 data);

private:
	void set_busy_flag();

	void draw_scanline(bitmap_ind16 &bitmap, const rectangle &cliprect, int y, UINT16 ra);
	void update_graphics(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_char(bitmap_ind16 &bitmap, const rectangle &cliprect, UINT16 ma, int x, int y, UINT8 md);
	void update_text(bitmap_ind16 &bitmap, const rectangle &cliprect);

	devcb_resolved_read8 m_in_rd_func;

	screen_device *m_screen;
	emu_timer *m_busy_timer;
	address_space *m_data;

	bool m_bf;                      // busy flag

	UINT8 m_ir;                     // instruction register
	UINT8 m_mcr;                    // mode control register
	UINT8 m_dor;                    // data output register

	UINT16 m_dsa;                   // display start address
	UINT16 m_cac;                   // cursor address counter

	int m_vp;                       // vertical character pitch
	int m_hp;                       // horizontal character pitch
	int m_hn;                       // horizontal number of characters
	int m_nx;                       // number of time divisions
	int m_cp;                       // cursor position

	int m_blink;                    // blink counter
	int m_cursor;                   // cursor visible

	// address space configurations
	const address_space_config      m_space_config;
};


// device type definition
extern const device_type HD61830;



#endif
