/*****************************************************************************
 *
 * includes/sorcerer.h
 *
 ****************************************************************************/

#ifndef SORCERER_H_
#define SORCERER_H_

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/dac.h"
#include "sound/wave.h"
#include "machine/ay31015.h"
#include "machine/ctronics.h"
#include "machine/ram.h"
#include "imagedev/cartslot.h"
#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"
#include "imagedev/flopdrv.h"
#include "formats/sorc_dsk.h"
#include "machine/micropolis.h"

#define SORCERER_USING_RS232 0

struct cass_data_t {
	struct {
		int length;     /* time cassette level is at input.level */
		int level;      /* cassette level */
		int bit;        /* bit being read */
	} input;
	struct {
		int length;     /* time cassette level is at output.level */
		int level;      /* cassette level */
		int bit;        /* bit to to output */
	} output;
};


class sorcerer_state : public driver_device
{
public:
	sorcerer_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_cass1(*this, CASSETTE_TAG),
	m_cass2(*this, CASSETTE2_TAG),
	m_wave1(*this, WAVE_TAG),
	m_wave2(*this, WAVE2_TAG),
	m_dac(*this, "dac"),
	m_uart(*this, "uart"),
	m_centronics(*this, "centronics"),
	m_ram(*this, RAM_TAG),
	m_iop_config(*this, "CONFIG"),
	m_iop_vs(*this, "VS")
	{ }

	DECLARE_READ8_MEMBER(sorcerer_fc_r);
	DECLARE_READ8_MEMBER(sorcerer_fd_r);
	DECLARE_READ8_MEMBER(sorcerer_fe_r);
	DECLARE_READ8_MEMBER(sorcerer_ff_r);
	DECLARE_WRITE8_MEMBER(sorcerer_fc_w);
	DECLARE_WRITE8_MEMBER(sorcerer_fd_w);
	DECLARE_WRITE8_MEMBER(sorcerer_fe_w);
	DECLARE_WRITE8_MEMBER(sorcerer_ff_w);
	UINT8 m_fe;
	UINT8 m_keyboard_line;
	const UINT8 *m_p_videoram;
	emu_timer *m_serial_timer;
	emu_timer *m_cassette_timer;
	cass_data_t m_cass_data;
	virtual void video_start();
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cass1;
	required_device<cassette_image_device> m_cass2;
	required_device<wave_device> m_wave1;
	required_device<wave_device> m_wave2;
	required_device<dac_device> m_dac;
	required_device<ay31015_device> m_uart;
	required_device<centronics_device> m_centronics;
	required_device<ram_device> m_ram;
	required_ioport m_iop_config;
	required_ioport m_iop_vs;
	DECLARE_DRIVER_INIT(sorcerer);
	virtual void machine_start();
	virtual void machine_reset();
	DECLARE_MACHINE_START(sorcererd);
	TIMER_CALLBACK_MEMBER(sorcerer_serial_tc);
	TIMER_CALLBACK_MEMBER(sorcerer_cassette_tc);
	TIMER_CALLBACK_MEMBER(sorcerer_reset);
};


/*----------- defined in machine/sorcerer.c -----------*/
SNAPSHOT_LOAD( sorcerer );

#endif /* SORCERER_H_ */
