


#define VERBOSE 1
#define LOG(x)	do { if (VERBOSE) logerror x; } while (0)

#define M1_MASTER_CLOCK (XTAL_8MHz)
#define M1_DUART_CLOCK  (XTAL_3_6864MHz)

#include "cpu/m6809/m6809.h"
#include "video/awpvid.h"		//Fruit Machines Only
#include "machine/6821pia.h"
#include "machine/68681.h"
#include "machine/meters.h"
#include "machine/roc10937.h"	// vfd
#include "machine/steppers.h"	// stepper motor
#include "sound/ay8910.h"
#include "sound/2413intf.h"
#include "sound/okim6376.h"
#include "machine/nvram.h"

struct i8279_state
{
	UINT8		command;
	UINT8		mode;
	UINT8		prescale;
	UINT8		inhibit;
	UINT8		clear;
	UINT8		ram[16];
	UINT8		read_sensor;
	UINT8		write_display;
	UINT8		sense_address;
	UINT8		sense_auto_inc;
	UINT8		disp_address;
	UINT8		disp_auto_inc;
};


class maygay1b_state : public driver_device
{
public:
	maygay1b_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_vfd(*this, "vfd")
	{

		m_NMIENABLE = 0;
	}

	UINT8 m_lamppos;
	int m_alpha_clock;
	int m_RAMEN;
	int m_ALARMEN;
	int m_PSUrelay;
	int m_WDOG;
	int m_SRSEL;
	int m_NMIENABLE;
	TIMER_DEVICE_CALLBACK_MEMBER( maygay1b_nmitimer_callback );
	UINT8 m_Lamps[256];
	int m_optic_pattern;
	optional_device<roc10937_t> m_vfd;
	device_t *m_duart68681;
	i8279_state m_i8279[2];
	DECLARE_READ8_MEMBER(m1_8279_r);
	DECLARE_WRITE8_MEMBER(m1_8279_w);
	DECLARE_READ8_MEMBER(m1_8279_2_r);
	DECLARE_WRITE8_MEMBER(m1_8279_2_w);
	DECLARE_WRITE8_MEMBER(reel12_w);
	DECLARE_WRITE8_MEMBER(reel34_w);
	DECLARE_WRITE8_MEMBER(reel56_w);
	DECLARE_WRITE8_MEMBER(m1_latch_w);
	DECLARE_WRITE8_MEMBER(latch_ch2_w);
	DECLARE_READ8_MEMBER(latch_st_hi);
	DECLARE_READ8_MEMBER(latch_st_lo);
	DECLARE_WRITE8_MEMBER(m1ab_no_oki_w);
	void m1_draw_lamps(int data,int strobe, int col);
	DECLARE_WRITE8_MEMBER(m1_pia_porta_w);
	DECLARE_WRITE8_MEMBER(m1_pia_portb_w);
	DECLARE_WRITE8_MEMBER(m1_meter_w);
	DECLARE_READ8_MEMBER(m1_meter_r);
	DECLARE_READ8_MEMBER(m1_firq_trg_r);
	DECLARE_DRIVER_INIT(m1);
	virtual void machine_start();
	virtual void machine_reset();
};
