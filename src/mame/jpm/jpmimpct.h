// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    JPM IMPACT with Video hardware

****************************************************************************/
#ifndef MAME_JPM_JPMIMPCT_H
#define MAME_JPM_JPMIMPCT_H

#pragma once

#include "cpu/tms34010/tms34010.h"
#include "machine/bacta_datalogger.h"
#include "machine/i8255.h"
#include "machine/mc68681.h"
#include "machine/meters.h"
#include "machine/roc10937.h"
#include "machine/steppers.h"
#include "machine/timer.h"
#include "sound/upd7759.h"
#include "video/bt47x.h"

#include "diserial.h"
#include "emupal.h"




class jpmtouch_device : public device_t,
	public device_serial_interface
{
public:
	jpmtouch_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto rxd_handler() { return m_rxd_handler.bind(); }

	void output_rxd(int state) { m_rxd_handler(state); }

	void touched(uint8_t x, uint8_t y);

protected:
	jpmtouch_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void tra_callback() override;
	virtual void tra_complete() override;

private:
	void tx_queue();

	devcb_write_line m_rxd_handler;
	int8_t m_sending = 0;
	uint8_t m_touch_data[3]{};
	uint8_t m_sendpos = 0;
};

DECLARE_DEVICE_TYPE(JPM_TOUCHSCREEN, jpmtouch_device)

class jpmimpct_state : public driver_device
{
public:
	jpmimpct_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_meters(*this, "meters")
		, m_datalogger(*this, "datalogger")
		, m_testdemo(*this, "TEST_DEMO")
		, m_digits(*this, "digit%u", 0U)
		, m_cointimer(*this, "cointimer%u", 0U)
		, m_ppi(*this, "ppi8255")
		, m_duart(*this, "main_duart")
		, m_vfd(*this, "vfd")
		, m_upd7759(*this, "upd")
		, m_reel(*this, "reel%u", 0U)
		, m_lamp_output(*this, "lamp%u", 0U)
		, m_pwrled(*this, "PWRLED")
		, m_statled(*this, "STATLED")
	{ }

	void impact_nonvideo(machine_config &config);
	void impact_nonvideo_altreels(machine_config &config);
	void impact_nonvideo_disc(machine_config &config);


	DECLARE_INPUT_CHANGED_MEMBER(coin_changed);
	template <unsigned N> int coinsense_r() { return (m_coinstate >> N) & 1; }

	int hopper_b_0_r();
	int hopper_b_3_r();
	int hopper_c_4_r();
	int hopper_c_6_r();
	int hopper_c_7_r();

protected:
	void impact_nonvideo_base(machine_config &config);

	void base(machine_config &config);

	required_device<cpu_device> m_maincpu;
	required_device<meters_device> m_meters;
	required_device<bacta_datalogger_device> m_datalogger;
	required_ioport m_testdemo;
	output_finder<300> m_digits;

	uint16_t jpmio_r();

	uint16_t unk_r();
	void unk_w(uint16_t data);

	void common_map(address_map &map) ATTR_COLD;

	int m_lamp_strobe = 0;

	void set_duart_1_hack_ip(bool state);

	void jpm_draw_lamps(uint16_t data, int lamp_strobe);

	TIMER_DEVICE_CALLBACK_MEMBER(duart_set_ip5);

	virtual void update_irqs();
protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	template <unsigned N> void reel_optic_cb(int state) { if (state) m_optic_pattern |= (1 << N); else m_optic_pattern &= ~(1 << N); }
	template <unsigned N> void reel_optic_inv_cb(int state) { if (!state) m_optic_pattern |= (1 << N); else m_optic_pattern &= ~(1 << N); }
	template <unsigned N> TIMER_DEVICE_CALLBACK_MEMBER(coinoff) { m_coinstate |= (1 << N); logerror("coin state lowered %d\n", N+1); }


	uint16_t optos_r();
	uint16_t prot_1_r();
	uint16_t prot_0_r();
	uint16_t ump_r();
	void volume_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void upd7759_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t upd7759_r(offs_t offset, uint16_t mem_mask = ~0);
	uint8_t hopper_c_r();
	void payen_a_w(uint8_t data);
	void display_c_w(uint8_t data);

	void pwrled_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void reels_0123_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void reels_45_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void slides_non_video_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void lamps_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void digits_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void lampstrobe_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);



	void duart_irq_handler(int state);
	void impact_non_video_map(address_map &map) ATTR_COLD;

	uint8_t m_Lamps[256]{};
	uint8_t m_optic_pattern = 0;
	bool m_payen = false;
	uint8_t m_hopinhibit = 0;
	uint8_t m_slidesout = 0;
	uint8_t m_hopper[3]{};
	uint8_t m_motor[3]{};
	uint8_t m_volume_latch = 0;
	uint8_t m_global_volume = 0;
	uint16_t m_coinstate = 0;
	required_device_array<timer_device, 6> m_cointimer;

	required_device<i8255_device> m_ppi;
	required_device<mc68681_device> m_duart;
	optional_device<s16lf01_device> m_vfd;
	required_device<upd7759_device> m_upd7759;
	optional_device_array<stepper_device, 6> m_reel;
	output_finder<256> m_lamp_output;
	output_finder<> m_pwrled;
	output_finder<> m_statled;
};

class jpmimpct_video_state : public jpmimpct_state
{
public:
	jpmimpct_video_state(const machine_config &mconfig, device_type type, const char *tag)
		: jpmimpct_state(mconfig, type, tag)
		, m_vidduart(*this, "vid_duart")
		, m_touch(*this, "touch")
		, m_dsp(*this, "dsp")
		, m_vram(*this, "vram")
		, m_ramdac(*this, "ramdac")
		, m_touchx(*this, "TOUCH_X")
		, m_touchy(*this, "TOUCH_Y")
	{
	}

	void impact_video(machine_config &config);
	void impact_video_touch(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(touch_port_changed);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void impact_video_map(address_map &map) ATTR_COLD;

	void slides_video_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void tms_program_map(address_map &map) ATTR_COLD;

	void tms_irq(int state);
	TMS340X0_TO_SHIFTREG_CB_MEMBER(to_shiftreg);
	TMS340X0_FROM_SHIFTREG_CB_MEMBER(from_shiftreg);
	TMS340X0_SCANLINE_RGB32_CB_MEMBER(scanline_update);
	uint8_t m_tms_irq = 0;

	virtual void update_irqs() override;

protected:
	required_device<mc68681_device> m_vidduart;
	optional_device<jpmtouch_device> m_touch;
private:
	optional_device<tms34010_device> m_dsp;
	optional_shared_ptr<uint16_t> m_vram;
	required_device<bt477_device> m_ramdac;
	optional_ioport m_touchx;
	optional_ioport m_touchy;
};

#endif // MAME_JPM_JPMIMPCT_H
