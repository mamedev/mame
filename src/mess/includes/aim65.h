/*****************************************************************************
 *
 * includes/aim65.h
 *
 * Rockwell AIM-65
 *
 ****************************************************************************/

#ifndef AIM65_H_
#define AIM65_H_

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "video/dl1416.h"
#include "machine/6522via.h"
#include "machine/6532riot.h"
#include "machine/6821pia.h"
#include "machine/ram.h"
#include "imagedev/cartslot.h"
#include "imagedev/cassette.h"
#include "sound/wave.h"


/** R6502 Clock.
 *
 * The R6502 on AIM65 operates at 1 MHz. The frequency reference is a 4 MHz
 * crystal controlled oscillator. Dual D-type flip-flop Z10 divides the 4 MHz
 * signal by four to drive the R6502 phase 0 (O0) input with a 1 MHz clock.
 */
#define AIM65_CLOCK  XTAL_4MHz/4


class aim65_state : public driver_device
{
public:
	aim65_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_cass1(*this, CASSETTE_TAG),
	m_cass2(*this, CASSETTE2_TAG)
	{ }

	DECLARE_WRITE8_MEMBER(aim65_pia_a_w);
	DECLARE_WRITE8_MEMBER(aim65_pia_b_w);
	DECLARE_READ8_MEMBER(aim65_riot_b_r);
	DECLARE_WRITE8_MEMBER(aim65_riot_a_w);
	DECLARE_WRITE8_MEMBER(aim65_pa_w);
	DECLARE_WRITE8_MEMBER(aim65_pb_w);
	DECLARE_WRITE8_MEMBER(aim65_printer_on);
	DECLARE_READ8_MEMBER(aim65_pb_r);
	UINT8 m_pia_a;
	UINT8 m_pia_b;
	UINT8 m_riot_port_a;
	UINT8 m_pb_save;

	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cass1;
	required_device<cassette_image_device> m_cass2;
	virtual void machine_start();
	TIMER_CALLBACK_MEMBER(aim65_printer_timer);
};


/*----------- defined in machine/aim65.c -----------*/

void aim65_update_ds1(device_t *device, int digit, int data);
void aim65_update_ds2(device_t *device, int digit, int data);
void aim65_update_ds3(device_t *device, int digit, int data);
void aim65_update_ds4(device_t *device, int digit, int data);
void aim65_update_ds5(device_t *device, int digit, int data);


#endif /* AIM65_H_ */
