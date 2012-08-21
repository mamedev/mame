/*****************************************************************************
 *
 * includes/pokemini.h
 *
 ****************************************************************************/

#ifndef POKEMINI_H_
#define POKEMINI_H_

#include "sound/speaker.h"
#include "machine/i2cmem.h"
#include "cpu/minx/minx.h"
#include "imagedev/cartslot.h"
#include "rendlay.h"

#define MACHINE_START_MEMBER(name) void name::machine_start()

typedef struct
{
	UINT8		colors_inverted;
	UINT8		background_enabled;
	UINT8		sprites_enabled;
	UINT8		copy_enabled;
	UINT8		map_size;
	UINT8		map_size_x;
	UINT8		frame_count;
	UINT8		max_frame_count;
	UINT32		bg_tiles;
	UINT32		spr_tiles;
	UINT8		count;
	emu_timer	*count_timer;
} PRC;


typedef struct
{
	emu_timer	*seconds_timer;
	emu_timer	*hz256_timer;
	emu_timer	*timer1;				/* Timer 1 low or 16bit */
	emu_timer	*timer1_hi;				/* Timer 1 hi */
	emu_timer	*timer2;				/* Timer 2 low or 16bit */
	emu_timer	*timer2_hi;				/* Timer 2 high */
	emu_timer	*timer3;				/* Timer 3 low or 16bit */
	emu_timer	*timer3_hi;				/* Timer 3 high */
} TIMERS;


class pokemini_state : public driver_device
{
public:
	pokemini_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu")
	,
		m_p_ram(*this, "p_ram"){ }

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<UINT8> m_p_ram;
	UINT8 m_pm_reg[0x100];
	PRC m_prc;
	TIMERS m_timers;
	bitmap_ind16 m_bitmap;
	virtual void video_start();
	virtual void machine_start();

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


/*----------- defined in machine/pokemini.c -----------*/

WRITE8_DEVICE_HANDLER( pokemini_hwreg_w );
READ8_DEVICE_HANDLER( pokemini_hwreg_r );

DEVICE_IMAGE_LOAD( pokemini_cart );

#endif /* POKEMINI_H */
