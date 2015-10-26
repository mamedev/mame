// license:BSD-3-Clause
// copyright-holders:David Viens
/***************************************************************************

    gic.h

   GI AY-3-8800-1 (Datasheet exists as AY-3-8500-1 Graphics Interface Chip)
   For the GIMINI "Challenger" programmable game system.
   
   Really only ever used in the Unisonic Champion 2711

***************************************************************************/

#pragma once

#ifndef __GIC_H__
#define __GIC_H__

#include "emu.h"


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_GIC_ADD(_tag, _clock, _screen_tag) \
	MCFG_DEVICE_ADD(_tag, GIC, _clock) \
	MCFG_VIDEO_SET_SCREEN(_screen_tag) 

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> gic_device

//Palette entries
#define GIC_BLACK 0
#define GIC_RED   1
#define GIC_GREEN 2
#define GIC_WHITE 3

#define GIC_CHAR_W 6
#define GIC_CHAR_H 8

#define GIC_LEFT_H 12
#define GIC_LEFT_W 6

#define GIC_RIGHT_H 6
#define GIC_RIGHT_W 13

class gic_device :  public device_t
				  , public device_sound_interface
				  , public device_video_interface
{
public:
	// construction/destruction
	gic_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	gic_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, int lines, const char *shortname, const char *source);

	// static configuration helpers
	static void set_screen_tag(device_t &device, const char *screen_tag) { downcast<gic_device &>(device).m_screen_tag = screen_tag; }

	DECLARE_PALETTE_INIT(gic);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	inline bitmap_ind16 *get_bitmap() { return &m_bitmap; }

	//plgDavid please change this to a MESS friendly handshake
	void set_shared_memory(const UINT8*m){ m_ram = m;};
	
	// Global constants (non mesured figures)
	static const int START_ACTIVE_SCAN = 10;
	static const int BORDER_SIZE       = GIC_CHAR_W*3;
	static const int END_ACTIVE_SCAN   = 10 + GIC_CHAR_W*2 + 150 + GIC_CHAR_W*2;
	static const int START_Y           = 1;
	static const int SCREEN_HEIGHT     = GIC_CHAR_H*(GIC_LEFT_H+2);
	static const int LINE_CLOCKS       = 455;
	static const int LINES             = 262;

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	
	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

	/* timers */
	static const device_timer_id TIMER_VBLANK = 0;

	void draw_char_left (int x, int y, UINT8 code, bitmap_ind16 &bitmap);
	void draw_char_right(int x, int y, UINT8 code, bitmap_ind16 &bitmap,int bg_col);
	
	bitmap_ind16 m_bitmap;
	UINT8 *      m_cgrom;          // internal chargen ROM
	
	emu_timer    *m_vblank_timer;
	sound_stream *m_stream;
	
	int m_audiocnt;
	int m_audioval;	
	int m_audioreset;	
	const UINT8* m_ram;
};

// device type definition
extern const device_type GIC;

#endif  /* __GIC_H__ */
