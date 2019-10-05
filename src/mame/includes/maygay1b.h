// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_INCLUDES_MAYGAY1B_H
#define MAME_INCLUDES_MAYGAY1B_H

#pragma once

#include "cpu/m6809/m6809.h"
#include "machine/i8279.h"

#include "video/awpvid.h"       //Fruit Machines Only
#include "machine/6821pia.h"
#include "machine/mc68681.h"
#include "machine/meters.h"
#include "machine/roc10937.h"   // vfd
#include "machine/steppers.h"   // stepper motor
#include "sound/ay8910.h"
#include "sound/ym2413.h"
#include "sound/okim6376.h"
#include "machine/nvram.h"
#include "machine/timer.h"
#include "sound/upd7759.h"
#include "cpu/mcs51/mcs51.h"
#include "sound/okim6295.h"


class maygay1b_state : public driver_device
{
public:
	maygay1b_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mcu(*this, "mcu"),
		m_vfd(*this, "vfd"),
		m_ay(*this, "aysnd"),
		m_msm6376(*this, "msm6376"),
		m_upd7759(*this, "upd"),
		m_okim6295(*this, "oki"),
		m_duart68681(*this, "duart68681"),
		m_sw1_port(*this, "SW1"),
		m_sw2_port(*this, "SW2"),
		m_kbd_ports(*this, { "SW1", "SW2", "STROBE2", "STROBE3", "STROBE4", "STROBE5", "STROBE6", "STROBE7", }),
		m_bank1(*this, "bank1"),
		m_reels(*this, "reel%u", 0U),
		m_meters(*this, "meters"),
		m_oki_region(*this, "msm6376"),
		m_lamps(*this, "lamp%u", 0U),
		m_triacs(*this, "triac%u", 0U)
	{
	}

	void maygay_m1_no_oki(machine_config &config);
	void maygay_m1(machine_config &config);
	void maygay_m1_nec(machine_config &config);
	void maygay_m1_empire(machine_config &config);

	void init_m1();
	void init_m1common();
	void init_m1nec();

private:
	required_device<cpu_device> m_maincpu;
	required_device<i80c51_device> m_mcu;
	optional_device<s16lf01_device> m_vfd;
	required_device<ay8910_device> m_ay;
	optional_device<okim6376_device> m_msm6376;
	optional_device<upd7759_device> m_upd7759;
	optional_device<okim6295_device> m_okim6295;
	required_device<mc68681_device> m_duart68681;
	required_ioport m_sw1_port;
	required_ioport m_sw2_port;
	required_ioport_array<8> m_kbd_ports;
	required_memory_bank m_bank1;
	required_device_array<stepper_device, 6> m_reels;
	optional_device<meters_device> m_meters;
	optional_region_ptr<uint8_t> m_oki_region;
	output_finder<256> m_lamps;
	output_finder<8> m_triacs;

	uint8_t m_lamppos;
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
	uint8_t m_Lamps[256];
	int m_optic_pattern;
	template <unsigned N> DECLARE_WRITE_LINE_MEMBER(reel_optic_cb) { if (state) m_optic_pattern |= (1 << N); else m_optic_pattern &= ~(1 << N); }
	DECLARE_WRITE8_MEMBER(scanlines_w);
	DECLARE_WRITE8_MEMBER(scanlines_2_w);
	DECLARE_WRITE8_MEMBER(lamp_data_w);
	DECLARE_WRITE8_MEMBER(lamp_data_2_w);
	DECLARE_READ8_MEMBER(kbd_r);
	DECLARE_WRITE8_MEMBER(reel12_w);
	DECLARE_WRITE8_MEMBER(reel34_w);
	DECLARE_WRITE8_MEMBER(reel56_w);
	DECLARE_WRITE_LINE_MEMBER(ramen_w);
	DECLARE_WRITE_LINE_MEMBER(alarmen_w);
	DECLARE_WRITE_LINE_MEMBER(nmien_w);
	DECLARE_WRITE_LINE_MEMBER(rts_w);
	DECLARE_WRITE_LINE_MEMBER(psurelay_w);
	DECLARE_WRITE_LINE_MEMBER(wdog_w);
	DECLARE_WRITE_LINE_MEMBER(srsel_w);
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
	DECLARE_WRITE8_MEMBER(mcu_port0_w);
	DECLARE_WRITE8_MEMBER(mcu_port1_w);
	DECLARE_WRITE8_MEMBER(mcu_port2_w);
	DECLARE_WRITE8_MEMBER(mcu_port3_w);
	DECLARE_READ8_MEMBER(mcu_port0_r);
	DECLARE_READ8_MEMBER(mcu_port2_r);

	DECLARE_WRITE8_MEMBER(main_to_mcu_0_w);
	DECLARE_WRITE8_MEMBER(main_to_mcu_1_w);

	uint8_t m_main_to_mcu;

	virtual void machine_start() override;
	virtual void machine_reset() override;
	void cpu0_firq(int data);
	void cpu0_nmi();
	void m1_memmap(address_map &map);
	void m1_nec_memmap(address_map &map);
};

INPUT_PORTS_EXTERN( maygay_m1 );

#endif // MAME_INCLUDES_MAYGAY1B_H
