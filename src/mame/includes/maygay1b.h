// license:BSD-3-Clause
// copyright-holders:David Haywood



#define VERBOSE 0
#define LOG(x)  do { if (VERBOSE) logerror x; } while (0)

#define M1_MASTER_CLOCK (XTAL_8MHz)
#define M1_DUART_CLOCK  (XTAL_3_6864MHz)

#include "cpu/m6809/m6809.h"
#include "machine/i8279.h"

#include "video/awpvid.h"       //Fruit Machines Only
#include "machine/6821pia.h"
#include "machine/mc68681.h"
#include "machine/meters.h"
#include "machine/roc10937.h"   // vfd
#include "machine/steppers.h"   // stepper motor
#include "sound/ay8910.h"
#include "sound/2413intf.h"
#include "sound/okim6376.h"
#include "machine/nvram.h"
#include "sound/upd7759.h"

#include "sound/okim6295.h"

class maygay1b_state : public driver_device
{
public:
	maygay1b_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_vfd(*this, "vfd"),
		m_ay(*this, "aysnd"),
		m_msm6376(*this, "msm6376"),
		m_upd7759(*this, "upd"),
		m_okim6295(*this, "oki"),
		m_duart68681(*this, "duart68681"),
		m_sw1_port(*this, "SW1"),
		m_sw2_port(*this, "SW2"),
		m_s2_port(*this, "STROBE2"),
		m_s3_port(*this, "STROBE3"),
		m_s4_port(*this, "STROBE4"),
		m_s5_port(*this, "STROBE5"),
		m_s6_port(*this, "STROBE6"),
		m_s7_port(*this, "STROBE7"),
		m_bank1(*this, "bank1"),
		m_reel0(*this, "reel0"),
		m_reel1(*this, "reel1"),
		m_reel2(*this, "reel2"),
		m_reel3(*this, "reel3"),
		m_reel4(*this, "reel4"),
		m_reel5(*this, "reel5"),
		m_meters(*this, "meters")
	{}

	required_device<cpu_device> m_maincpu;
	optional_device<s16lf01_t> m_vfd;
	required_device<ay8910_device> m_ay;
	optional_device<okim6376_device> m_msm6376;
	optional_device<upd7759_device> m_upd7759;
	optional_device<okim6295_device> m_okim6295;
	required_device<mc68681_device> m_duart68681;
	required_ioport m_sw1_port;
	required_ioport m_sw2_port;
	required_ioport m_s2_port;
	required_ioport m_s3_port;
	required_ioport m_s4_port;
	required_ioport m_s5_port;
	required_ioport m_s6_port;
	required_ioport m_s7_port;
	required_memory_bank m_bank1;
	required_device<stepper_device> m_reel0;
	required_device<stepper_device> m_reel1;
	required_device<stepper_device> m_reel2;
	required_device<stepper_device> m_reel3;
	required_device<stepper_device> m_reel4;
	required_device<stepper_device> m_reel5;
	required_device<meters_device> m_meters;

	UINT8 m_lamppos;
	int m_lamp_strobe;
	int m_old_lamp_strobe;
	int m_lamp_strobe2;
	int m_old_lamp_strobe2;
	int m_RAMEN;
	int m_ALARMEN;
	int m_PSUrelay;
	bool m_Vmm;
	int m_WDOG;
	int m_NMIENABLE;
	int m_meter;
	TIMER_DEVICE_CALLBACK_MEMBER( maygay1b_nmitimer_callback );
	UINT8 m_Lamps[256];
	int m_optic_pattern;
	DECLARE_WRITE_LINE_MEMBER(reel0_optic_cb) { if (state) m_optic_pattern |= 0x01; else m_optic_pattern &= ~0x01; }
	DECLARE_WRITE_LINE_MEMBER(reel1_optic_cb) { if (state) m_optic_pattern |= 0x02; else m_optic_pattern &= ~0x02; }
	DECLARE_WRITE_LINE_MEMBER(reel2_optic_cb) { if (state) m_optic_pattern |= 0x04; else m_optic_pattern &= ~0x04; }
	DECLARE_WRITE_LINE_MEMBER(reel3_optic_cb) { if (state) m_optic_pattern |= 0x08; else m_optic_pattern &= ~0x08; }
	DECLARE_WRITE_LINE_MEMBER(reel4_optic_cb) { if (state) m_optic_pattern |= 0x10; else m_optic_pattern &= ~0x10; }
	DECLARE_WRITE_LINE_MEMBER(reel5_optic_cb) { if (state) m_optic_pattern |= 0x20; else m_optic_pattern &= ~0x20; }
	DECLARE_WRITE8_MEMBER(scanlines_w);
	DECLARE_WRITE8_MEMBER(lamp_data_w);
	DECLARE_WRITE8_MEMBER(lamp_data_2_w);
	DECLARE_READ8_MEMBER(kbd_r);
	DECLARE_WRITE8_MEMBER(reel12_w);
	DECLARE_WRITE8_MEMBER(reel34_w);
	DECLARE_WRITE8_MEMBER(reel56_w);
	DECLARE_WRITE8_MEMBER(m1_latch_w);
	DECLARE_WRITE8_MEMBER(latch_ch2_w);
	DECLARE_READ8_MEMBER(latch_st_hi);
	DECLARE_READ8_MEMBER(latch_st_lo);
	DECLARE_WRITE8_MEMBER(m1ab_no_oki_w);
	DECLARE_WRITE8_MEMBER(m1_pia_porta_w);
	DECLARE_WRITE8_MEMBER(m1_pia_portb_w);
	DECLARE_WRITE8_MEMBER(m1_lockout_w);
	DECLARE_WRITE8_MEMBER(m1_meter_w);
	DECLARE_READ8_MEMBER(m1_meter_r);
	DECLARE_READ8_MEMBER(m1_firq_clr_r);
	DECLARE_READ8_MEMBER(m1_firq_trg_r);
	DECLARE_READ8_MEMBER(m1_firq_nec_r);
	DECLARE_READ8_MEMBER(nec_reset_r);
	DECLARE_WRITE8_MEMBER(nec_bank0_w);
	DECLARE_WRITE8_MEMBER(nec_bank1_w);
	DECLARE_WRITE_LINE_MEMBER(duart_irq_handler);
	DECLARE_READ8_MEMBER(m1_duart_r);
	DECLARE_DRIVER_INIT(m1);
	DECLARE_DRIVER_INIT(m1common);
	DECLARE_DRIVER_INIT(m1nec);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	void cpu0_firq(int data);
	void cpu0_nmi();
};

MACHINE_CONFIG_EXTERN( maygay_m1 );
MACHINE_CONFIG_EXTERN( maygay_m1_nec );
MACHINE_CONFIG_EXTERN( maygay_m1_no_oki );
MACHINE_CONFIG_EXTERN( maygay_m1_empire );
