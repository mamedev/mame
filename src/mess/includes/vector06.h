/*****************************************************************************
 *
 * includes/vector06.h
 *
 ****************************************************************************/

#ifndef VECTOR06_H_
#define VECTOR06_H_

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/i8085/i8085.h"
#include "sound/wave.h"
#include "machine/i8255.h"
#include "machine/ram.h"
#include "machine/wd17xx.h"
#include "imagedev/cassette.h"
#include "imagedev/cartslot.h"
#include "imagedev/flopdrv.h"
#include "formats/basicdsk.h"


class vector06_state : public driver_device
{
public:
	vector06_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_cass(*this, CASSETTE_TAG),
	m_fdc(*this, "wd1793"),
	m_ppi(*this, "ppi8255"),
	m_ppi2(*this, "ppi8255_2"),
	m_ram(*this, RAM_TAG)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cass;
	required_device<device_t> m_fdc;
	required_device<i8255_device> m_ppi;
	required_device<i8255_device> m_ppi2;
	required_device<ram_device> m_ram;
	DECLARE_READ8_MEMBER(vector06_8255_portb_r);
	DECLARE_READ8_MEMBER(vector06_8255_portc_r);
	DECLARE_WRITE8_MEMBER(vector06_8255_porta_w);
	DECLARE_WRITE8_MEMBER(vector06_8255_portb_w);
	DECLARE_WRITE8_MEMBER(vector06_color_set);
	DECLARE_READ8_MEMBER(vector06_romdisk_portb_r);
	DECLARE_WRITE8_MEMBER(vector06_romdisk_porta_w);
	DECLARE_WRITE8_MEMBER(vector06_romdisk_portc_w);
	DECLARE_READ8_MEMBER(vector06_8255_1_r);
	DECLARE_WRITE8_MEMBER(vector06_8255_1_w);
	DECLARE_READ8_MEMBER(vector06_8255_2_r);
	DECLARE_WRITE8_MEMBER(vector06_8255_2_w);
	DECLARE_WRITE8_MEMBER(vector06_disc_w);
	UINT8 m_keyboard_mask;
	UINT8 m_color_index;
	UINT8 m_video_mode;
	UINT16 m_romdisk_msb;
	UINT8 m_romdisk_lsb;
	UINT8 m_vblank_state;
	void vector06_set_video_mode(int width);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	virtual void palette_init();
};


/*----------- defined in machine/vector06.c -----------*/

extern const i8255_interface vector06_ppi8255_interface;
extern const i8255_interface vector06_ppi8255_2_interface;

extern MACHINE_START( vector06 );
extern MACHINE_RESET( vector06 );

extern INTERRUPT_GEN( vector06_interrupt );


/*----------- defined in video/vector06.c -----------*/

extern PALETTE_INIT( vector06 );
extern VIDEO_START( vector06 );
extern SCREEN_UPDATE_IND16( vector06 );

#endif /* VECTOR06_H_ */
