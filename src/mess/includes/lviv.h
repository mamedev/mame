/*****************************************************************************
 *
 * includes/lviv.h
 *
 ****************************************************************************/

#ifndef LVIV_H_
#define LVIV_H_

#include "imagedev/snapquik.h"
#include "machine/i8255.h"

class lviv_state : public driver_device
{
public:
	lviv_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	unsigned char * m_video_ram;
	unsigned short m_colortable[1][4];
	UINT8 m_ppi_port_outputs[2][3];
	UINT8 m_startup_mem_map;
	DECLARE_READ8_MEMBER(lviv_io_r);
	DECLARE_WRITE8_MEMBER(lviv_io_w);
	DECLARE_DIRECT_UPDATE_MEMBER(lviv_directoverride);
	virtual void machine_reset();
	virtual void video_start();
	virtual void palette_init();
	UINT32 screen_update_lviv(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


/*----------- defined in machine/lviv.c -----------*/

extern const i8255_interface lviv_ppi8255_interface_0;
extern const i8255_interface lviv_ppi8255_interface_1;


SNAPSHOT_LOAD( lviv );


/*----------- defined in video/lviv.c -----------*/

extern VIDEO_START( lviv );
extern SCREEN_UPDATE_IND16( lviv );
extern const unsigned char lviv_palette[8*3];
extern PALETTE_INIT( lviv );
extern void lviv_update_palette(running_machine &, UINT8);


#endif /* LVIV_H_ */
