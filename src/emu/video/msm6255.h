/**********************************************************************

    OKI MSM6255 Dot Matrix LCD Controller implementation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __MSM6255__
#define __MSM6255__

#include "emu.h"



///*************************************************************************
//  INTERFACE CONFIGURATION MACROS
///*************************************************************************

#define MCFG_MSM6255_ADD(_tag, _clock, _config) \
	MCFG_DEVICE_ADD(_tag, MSM6255, _clock) \
	MCFG_DEVICE_CONFIG(_config)


#define MSM6255_INTERFACE(name) \
	const msm6255_interface (name) =


#define MSM6255_CHAR_RAM_READ(_name) \
	UINT8 _name(device_t *device, UINT16 ma, UINT8 ra)



///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************

// ======================> msm6255_char_ram_read_func

typedef UINT8 (*msm6255_char_ram_read_func)(device_t *device, UINT16 ma, UINT8 ra);


// ======================> msm6255_interface

struct msm6255_interface
{
	const char *m_screen_tag;

	int m_character_clock;

	// ROM/RAM data read function
	msm6255_char_ram_read_func		m_char_ram_r;
};


// ======================> msm6255_device

class msm6255_device :	public device_t,
						public msm6255_interface
{
public:
    // construction/destruction
    msm6255_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

    DECLARE_READ8_MEMBER( read );
    DECLARE_WRITE8_MEMBER( write );

	void update_screen(bitmap_t *bitmap, const rectangle *cliprect);

protected:
    // device-level overrides
	virtual void device_config_complete();
    virtual void device_start();
    virtual void device_reset();

private:
	inline UINT8 read_video_data(UINT16 ma, UINT8 ra);
	void update_cursor();
	void draw_scanline(bitmap_t *bitmap, const rectangle *cliprect, int y, UINT16 ma, UINT8 ra);
	void update_graphics(bitmap_t *bitmap, const rectangle *cliprect);
	void update_text(bitmap_t *bitmap, const rectangle *cliprect);

	screen_device *m_screen;

	UINT8 m_ir;						// instruction register
	UINT8 m_mor;					// mode control register
	UINT8 m_pr;						// character pitch register
	UINT8 m_hnr;					// horizontal character number register
	UINT8 m_dvr;					// duty number register
	UINT8 m_cpr;					// cursor form register
	UINT8 m_slr;					// start address (lower) register
	UINT8 m_sur;					// start address (upper) register
	UINT8 m_clr;					// cursor address (lower) register
	UINT8 m_cur;					// cursor address (upper) register

	int m_cursor;					// is cursor displayed
	int m_frame;					// frame counter
};


// device type definition
extern const device_type MSM6255;



#endif
