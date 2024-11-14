// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_MAYGAY_MAYGAY1B_H
#define MAME_MAYGAY_MAYGAY1B_H

#pragma once

#include "awpvid.h"       //Fruit Machines Only

#include "cpu/m6809/m6809.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/6821pia.h"
#include "machine/i8279.h"
#include "machine/mc68681.h"
#include "machine/meters.h"
#include "machine/nvram.h"
#include "machine/roc10937.h"   // vfd
#include "machine/steppers.h"   // stepper motor
#include "machine/timer.h"
#include "sound/ay8910.h"
#include "sound/okim6295.h"
#include "sound/okim6376.h"
#include "sound/upd7759.h"
#include "sound/ymopl.h"


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

	uint8_t m_lamppos = 0;
	int m_lamp_strobe = 0;
	int m_old_lamp_strobe = 0;
	int m_lamp_strobe2 = 0;
	int m_old_lamp_strobe2 = 0;
	int m_RAMEN = 0;
	int m_ALARMEN = 0;
	int m_PSUrelay = 0;
	bool m_Vmm = false;
	int m_WDOG = 0;
	int m_NMIENABLE = 0;
	int m_meter = 0;
	TIMER_DEVICE_CALLBACK_MEMBER( maygay1b_nmitimer_callback );
	uint8_t m_Lamps[256]{};
	int m_optic_pattern = 0;
	template <unsigned N> void reel_optic_cb(int state) { if (state) m_optic_pattern |= (1 << N); else m_optic_pattern &= ~(1 << N); }
	void scanlines_w(uint8_t data);
	void scanlines_2_w(uint8_t data);
	void lamp_data_w(uint8_t data);
	void lamp_data_2_w(uint8_t data);
	uint8_t kbd_r();
	void reel12_w(uint8_t data);
	void reel34_w(uint8_t data);
	void reel56_w(uint8_t data);
	void ramen_w(int state);
	void alarmen_w(int state);
	void nmien_w(int state);
	void rts_w(int state);
	void psurelay_w(int state);
	void wdog_w(int state);
	void srsel_w(int state);
	void latch_ch2_w(uint8_t data);
	uint8_t latch_st_hi();
	uint8_t latch_st_lo();
	void m1ab_no_oki_w(uint8_t data);
	void m1_pia_porta_w(uint8_t data);
	void m1_pia_portb_w(uint8_t data);
	void m1_lockout_w(uint8_t data);
	void m1_meter_w(uint8_t data);
	uint8_t m1_meter_r();
	uint8_t m1_firq_clr_r();
	uint8_t m1_firq_trg_r();
	uint8_t m1_firq_nec_r();
	uint8_t nec_reset_r();
	void nec_bank0_w(uint8_t data);
	void nec_bank1_w(uint8_t data);
	void duart_irq_handler(int state);
	uint8_t m1_duart_r();
	void mcu_port0_w(uint8_t data);
	void mcu_port1_w(uint8_t data);
	void mcu_port2_w(uint8_t data);
	void mcu_port3_w(uint8_t data);
	uint8_t mcu_port0_r();
	uint8_t mcu_port2_r();

	void main_to_mcu_0_w(uint8_t data);
	void main_to_mcu_1_w(uint8_t data);

	uint8_t m_main_to_mcu;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	void cpu0_firq(int data);
	void cpu0_nmi();
	void m1_memmap(address_map &map) ATTR_COLD;
	void m1_nec_memmap(address_map &map) ATTR_COLD;
};

INPUT_PORTS_EXTERN( maygay_m1 );

#endif // MAME_MAYGAY_MAYGAY1B_H
