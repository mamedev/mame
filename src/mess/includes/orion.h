/*****************************************************************************
 *
 * includes/orion.h
 *
 ****************************************************************************/

#ifndef ORION_H_
#define ORION_H_

#include "includes/radio86.h"
#include "machine/i8255.h"

class orion_state : public radio86_state
{
public:
	orion_state(const machine_config &mconfig, device_type type, const char *tag)
		: radio86_state(mconfig, type, tag) { }

	UINT8 m_orion128_video_mode;
	UINT8 m_orion128_video_page;
	UINT8 m_orion128_video_width;
	UINT8 m_video_mode_mask;
	UINT8 m_orionpro_pseudo_color;
	UINT8 m_romdisk_lsb;
	UINT8 m_romdisk_msb;
	UINT8 m_orion128_memory_page;
	UINT8 m_orionz80_memory_page;
	UINT8 m_orionz80_dispatcher;
	UINT8 m_speaker;
	UINT8 m_orionpro_ram0_segment;
	UINT8 m_orionpro_ram1_segment;
	UINT8 m_orionpro_ram2_segment;
	UINT8 m_orionpro_page;
	UINT8 m_orionpro_128_page;
	UINT8 m_orionpro_rom2_segment;
	UINT8 m_orionpro_dispatcher;
	DECLARE_READ8_MEMBER(orion128_system_r);
	DECLARE_WRITE8_MEMBER(orion128_system_w);
	DECLARE_READ8_MEMBER(orion128_romdisk_r);
	DECLARE_WRITE8_MEMBER(orion128_romdisk_w);
	DECLARE_WRITE8_MEMBER(orion128_video_mode_w);
	DECLARE_WRITE8_MEMBER(orion128_video_page_w);
	DECLARE_WRITE8_MEMBER(orion128_memory_page_w);
	DECLARE_WRITE8_MEMBER(orion_disk_control_w);
	DECLARE_READ8_MEMBER(orion128_floppy_r);
	DECLARE_WRITE8_MEMBER(orion128_floppy_w);
	DECLARE_READ8_MEMBER(orionz80_floppy_rtc_r);
	DECLARE_WRITE8_MEMBER(orionz80_floppy_rtc_w);
	DECLARE_WRITE8_MEMBER(orionz80_sound_w);
	DECLARE_WRITE8_MEMBER(orionz80_sound_fe_w);
	DECLARE_WRITE8_MEMBER(orionz80_memory_page_w);
	DECLARE_WRITE8_MEMBER(orionz80_dispatcher_w);
	DECLARE_READ8_MEMBER(orionz80_io_r);
	DECLARE_WRITE8_MEMBER(orionz80_io_w);
	DECLARE_WRITE8_MEMBER(orionpro_memory_page_w);
	DECLARE_READ8_MEMBER(orionpro_io_r);
	DECLARE_WRITE8_MEMBER(orionpro_io_w);
};


/*----------- defined in machine/orion.c -----------*/

extern const i8255_interface orion128_ppi8255_interface_1;

extern MACHINE_START( orion128 );
extern MACHINE_RESET( orion128 );

extern MACHINE_START( orionz80 );
extern MACHINE_RESET( orionz80 );
extern INTERRUPT_GEN( orionz80_interrupt );


extern MACHINE_RESET( orionpro );

/*----------- defined in video/orion.c -----------*/

extern VIDEO_START( orion128 );
extern SCREEN_UPDATE_IND16( orion128 );
extern PALETTE_INIT( orion128 );

#endif /* ORION_H_ */

