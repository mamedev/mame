/*****************************************************************************
 *
 * includes/galaxy.h
 *
 ****************************************************************************/

#ifndef GALAXY_H_
#define GALAXY_H_

#include "imagedev/snapquik.h"


class galaxy_state : public driver_device
{
public:
	galaxy_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_interrupts_enabled;
	UINT8 m_latch_value;
	UINT32 m_gal_cnt;
	UINT8 m_code;
	UINT8 m_first;
	UINT32 m_start_addr;
	emu_timer *m_gal_video_timer;
	bitmap_ind16 m_bitmap;
	DECLARE_READ8_MEMBER(galaxy_keyboard_r);
	DECLARE_WRITE8_MEMBER(galaxy_latch_w);
	DECLARE_DRIVER_INIT(galaxy);
	DECLARE_DRIVER_INIT(galaxyp);
	virtual void video_start();
	DECLARE_MACHINE_RESET(galaxy);
	DECLARE_MACHINE_RESET(galaxyp);
	UINT32 screen_update_galaxy(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(galaxy_interrupt);
	TIMER_CALLBACK_MEMBER(gal_video);
};


/*----------- defined in machine/galaxy.c -----------*/
SNAPSHOT_LOAD( galaxy );

/*----------- defined in video/galaxy.c -----------*/
void galaxy_set_timer(running_machine &machine);

#endif /* GALAXY_H_ */
