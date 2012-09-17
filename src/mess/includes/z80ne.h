/*****************************************************************************
 *
 * includes/z80ne.h
 *
 * Nuova Elettronica Z80NE
 *
 * http://www.z80ne.com/
 *
 ****************************************************************************/

#ifndef Z80NE_H_
#define Z80NE_H_

#include "video/mc6847.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define Z80NE_CPU_SPEED_HZ		1920000	/* 1.92 MHz */

#define LX383_KEYS			16
#define LX383_DOWNSAMPLING	16

#define LX385_TAPE_SAMPLE_FREQ 38400

/* wave duration threshold */
enum z80netape_speed
{
	TAPE_300BPS  = 300, /*  300 bps */
	TAPE_600BPS  = 600, /*  600 bps */
	TAPE_1200BPS = 1200 /* 1200 bps */
};

struct cass_data_t {
	struct {
		int length;		/* time cassette level is at input.level */
		int level;		/* cassette level */
		int bit;		/* bit being read */
	} input;
	struct {
		int length;		/* time cassette level is at output.level */
		int level;		/* cassette level */
		int bit;		/* bit to to output */
	} output;
	z80netape_speed speed;			/* 300 - 600 - 1200 */
	int wave_filter;
	int wave_length;
	int wave_short;
	int wave_long;
};

struct wd17xx_state_t {
	int drq;
	int intrq;
	UINT8 drive; /* current drive */
	UINT8 head;  /* current head */
};


class z80ne_state : public driver_device
{
public:
	z80ne_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_vdg(*this, "mc6847"),
		  m_videoram(*this,"videoram") {}

	optional_device<mc6847_base_device> m_vdg;
	optional_shared_ptr<UINT8> m_videoram;
	UINT8 m_lx383_scan_counter;
	UINT8 m_lx383_key[LX383_KEYS];
	int m_lx383_downsampler;
	int m_nmi_delay_counter;
	int m_reset_delay_counter;
	device_t *m_ay31015;
	UINT8 m_lx385_ctrl;
	device_t *m_lx388_kr2376;
	emu_timer *m_cassette_timer;
	cass_data_t m_cass_data;
	wd17xx_state_t m_wd17xx_state;
	DECLARE_READ8_MEMBER(lx383_r);
	DECLARE_WRITE8_MEMBER(lx383_w);
	DECLARE_READ8_MEMBER(lx385_data_r);
	DECLARE_READ8_MEMBER(lx385_ctrl_r);
	DECLARE_WRITE8_MEMBER(lx385_data_w);
	DECLARE_WRITE8_MEMBER(lx385_ctrl_w);
	DECLARE_READ8_MEMBER(lx388_data_r);
	DECLARE_READ8_MEMBER(lx388_read_field_sync);
	DECLARE_DIRECT_UPDATE_MEMBER(z80ne_default);
	DECLARE_DIRECT_UPDATE_MEMBER(z80ne_nmi_delay_count);
	DECLARE_DIRECT_UPDATE_MEMBER(z80ne_reset_delay_count);
	DECLARE_DRIVER_INIT(z80netf);
	DECLARE_DRIVER_INIT(z80net);
	DECLARE_DRIVER_INIT(z80netb);
	DECLARE_DRIVER_INIT(z80ne);
	DECLARE_MACHINE_START(z80ne);
	DECLARE_MACHINE_RESET(z80ne);
	DECLARE_MACHINE_START(z80netb);
	DECLARE_MACHINE_RESET(z80netb);
	DECLARE_MACHINE_START(z80netf);
	DECLARE_MACHINE_RESET(z80netf);
	DECLARE_MACHINE_START(z80net);
	DECLARE_MACHINE_RESET(z80net);
	DECLARE_MACHINE_RESET(z80ne_base);
};


/*----------- defined in machine/z80ne.c -----------*/

READ8_DEVICE_HANDLER(lx388_mc6847_videoram_r);
READ8_DEVICE_HANDLER(lx390_fdc_r);
WRITE8_DEVICE_HANDLER(lx390_fdc_w);
READ8_DEVICE_HANDLER(lx390_reset_bank);
WRITE8_DEVICE_HANDLER(lx390_motor_w);











INPUT_CHANGED(z80ne_reset);
INPUT_CHANGED(z80ne_nmi);

#endif /* Z80NE_H_ */
