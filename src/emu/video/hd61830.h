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

    devcb_read8 m_in_rd_func;
};



// ======================> hd61830_device_config

class hd61830_device_config :   public device_config,
								public device_config_memory_interface,
                                public hd61830_interface
{
    friend class hd61830_device;

    // construction/destruction
    hd61830_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);

public:
    // allocators
    static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
    virtual device_t *alloc_device(running_machine &machine) const;

	// optional information overrides
	virtual const rom_entry *rom_region() const;

protected:
	// device_config overrides
	virtual void device_config_complete();

	// device_config_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const;

    // address space configurations
	const address_space_config		m_space_config;
};



// ======================> hd61830_device

class hd61830_device :	public device_t,
						public device_memory_interface
{
    friend class hd61830_device_config;

    // construction/destruction
    hd61830_device(running_machine &_machine, const hd61830_device_config &_config);

public:
    DECLARE_READ8_MEMBER( status_r );
    DECLARE_WRITE8_MEMBER( control_w );

    DECLARE_READ8_MEMBER( data_r );
    DECLARE_WRITE8_MEMBER( data_w );

	void update_screen(bitmap_t *bitmap, const rectangle *cliprect);

protected:
    // device-level overrides
    virtual void device_start();
    virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	inline UINT8 readbyte(offs_t address);
	inline void writebyte(offs_t address, UINT8 data);

private:
	void set_busy_flag();

	void draw_scanline(bitmap_t *bitmap, const rectangle *cliprect, int y, UINT16 ra);
	void update_graphics(bitmap_t *bitmap, const rectangle *cliprect);
	void draw_char(bitmap_t *bitmap, const rectangle *cliprect, UINT16 ma, int x, int y, UINT8 md);
	void update_text(bitmap_t *bitmap, const rectangle *cliprect);

    devcb_resolved_read8 m_in_rd_func;

	screen_device *m_screen;
	emu_timer *m_busy_timer;
	address_space *m_data;

	bool m_bf;						// busy flag

	UINT8 m_ir;						// instruction register
	UINT8 m_mcr;					// mode control register
	UINT8 m_dor;					// data output register

	UINT16 m_dsa;					// display start address
	UINT16 m_cac;					// cursor address counter

	int m_vp;						// vertical character pitch
	int m_hp;						// horizontal character pitch
	int m_hn;						// horizontal number of characters
	int m_nx;						// number of time divisions
	int m_cp;						// cursor position

    int m_blink;					// blink counter
	int m_cursor;					// cursor visible

	const hd61830_device_config &m_config;
};


// device type definition
extern const device_type HD61830;



#endif
