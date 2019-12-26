// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

#ifndef MAME_INCLUDES_CDI_H
#define MAME_INCLUDES_CDI_H

#include "machine/scc68070.h"
#include "machine/cdislave.h"
#include "machine/cdicdic.h"
#include "sound/dmadac.h"
#include "video/mcd212.h"
#include "cpu/mcs51/mcs51.h"
#include "cpu/m6805/m68hc05.h"
#include "screen.h"

/*----------- driver state -----------*/

class cdi_state : public driver_device
{
public:
	cdi_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_planea(*this, "mcd212:planea")
		, m_slave_hle(*this, "slave_hle")
		, m_cdic(*this, "cdic")
		, m_cdda(*this, "cdda")
		, m_mcd212(*this, "mcd212")
		, m_lcd(*this, "lcd")
		, m_dmadac(*this, "dac%u", 1U)
	{ }

	void cdimono1_base(machine_config &config);
	void cdimono1(machine_config &config);
	void cdimono1_dvc(machine_config &config);

protected:
	virtual void machine_start() override { }
	virtual void machine_reset() override;
	virtual void video_start() override;

	void cdimono1_mem(address_map &map);
	void cdimono1_dvc_mem(address_map &map);

	required_device<scc68070_device> m_maincpu;

	DECLARE_READ16_MEMBER(uart_loopback_enable);

	enum servo_portc_bit_t
	{
		INV_JUC_OUT = (1 << 2),
		INV_DIV4_IN = (1 << 5),
		INV_CADDYSWITCH_IN = (1 << 7)
	};

	void draw_lcd(int y);
	uint32_t screen_update_cdimono1_lcd(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_READ16_MEMBER(dvc_r);
	DECLARE_WRITE16_MEMBER(dvc_w);

	required_shared_ptr<uint16_t> m_planea;
	optional_device<cdislave_device> m_slave_hle;
	optional_device<cdicdic_device> m_cdic;
	required_device<cdda_device> m_cdda;
	required_device<mcd212_device> m_mcd212;
	optional_device<screen_device> m_lcd;

	required_device_array<dmadac_sound_device, 2> m_dmadac;

	bitmap_rgb32 m_lcdbitmap;
};

class cdimono2_state : public cdi_state
{
public:
	cdimono2_state(const machine_config &mconfig, device_type type, const char *tag)
		: cdi_state(mconfig, type, tag)
		, m_servo(*this, "servo")
		, m_slave(*this, "slave")
	{ }

	void cdimono2(machine_config &config);
	void cdi910(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	void cdi910_mem(address_map &map);
	void cdimono2_mem(address_map &map);

	DECLARE_READ8_MEMBER(slave_porta_r);
	DECLARE_READ8_MEMBER(slave_portb_r);
	DECLARE_READ8_MEMBER(slave_portc_r);
	DECLARE_READ8_MEMBER(slave_portd_r);
	DECLARE_WRITE8_MEMBER(slave_porta_w);
	DECLARE_WRITE8_MEMBER(slave_portb_w);
	DECLARE_WRITE8_MEMBER(slave_portc_w);
	DECLARE_WRITE8_MEMBER(slave_portd_w);
	DECLARE_WRITE8_MEMBER(servo_portb_w);

	DECLARE_READ8_MEMBER(slave_glue_r);
	DECLARE_WRITE8_MEMBER(slave_glue_w);

	DECLARE_READ8_MEMBER(dsp_r);
	DECLARE_WRITE8_MEMBER(dsp_w);

	DECLARE_WRITE8_MEMBER(controller_tx);

	DECLARE_READ16_MEMBER(uart_loopback_enable2);

	required_device<m68hc05c8_device> m_servo;
	required_device<m68hc05c8_device> m_slave;

	uint8_t m_porta_data;
	uint8_t m_portb_data;
	uint8_t m_portc_data;
	uint8_t m_portd_data;

	uint8_t m_disdat;
	uint8_t m_disclk;
	uint8_t m_disen;
	uint8_t m_disdata;
	uint8_t m_disbit;

	uint8_t m_mouse_buffer[6];
	uint8_t m_mouse_idx;
	emu_timer *m_mouse_timer;
};

class quizard_state : public cdi_state
{
public:
	quizard_state(const machine_config &mconfig, device_type type, const char *tag)
		: cdi_state(mconfig, type, tag)
		, m_input1(*this, "INPUT1")
		, m_input2(*this, "INPUT2")
	{ }

	void quizard(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(mcu_input);

protected:
	void set_mcu_value(uint16_t value);
	void set_mcu_ack(uint8_t ack);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	DECLARE_READ8_MEMBER(mcu_p1_r);

	void mcu_tx(uint8_t data);
	void mcu_calculate_state();
	void mcu_set_seeds(uint8_t *rx);
	void mcu_rx(uint8_t data);

	required_ioport m_input1;
	required_ioport m_input2;

	uint16_t m_seeds[10];
	uint8_t m_state[8];

	uint16_t m_mcu_value;
	uint8_t m_mcu_ack;
};

class quizard1_state : public quizard_state
{
public:
	quizard1_state(const machine_config &mconfig, device_type type, const char *tag)
		: quizard_state(mconfig, type, tag)
	{
		set_mcu_value(0x021f);
		set_mcu_ack(0x5a);
	}
};

class quizard2_state : public quizard_state
{
public:
	quizard2_state(const machine_config &mconfig, device_type type, const char *tag)
		: quizard_state(mconfig, type, tag)
	{
		// 0x2b1: Italian
		// 0x001: French
		// 0x188: German
		set_mcu_value(0x0188);
		set_mcu_ack(0x59);
	}
};

class quizard3_state : public quizard_state
{
public:
	quizard3_state(const machine_config &mconfig, device_type type, const char *tag)
		: quizard_state(mconfig, type, tag)
	{
		set_mcu_value(0x00ae);
		set_mcu_ack(0x58);
	}
};

class quizard4_state : public quizard_state
{
public:
	quizard4_state(const machine_config &mconfig, device_type type, const char *tag)
		: quizard_state(mconfig, type, tag)
	{
		set_mcu_value(0x011f);
		set_mcu_ack(0x57);
	}
};

#endif // MAME_INCLUDES_CDI_H
