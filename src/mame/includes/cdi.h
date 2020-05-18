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
		, m_servo(*this, "servo")
		, m_slave(*this, "slave")
		, m_cdic(*this, "cdic")
		, m_cdda(*this, "cdda")
		, m_mcd212(*this, "mcd212")
		, m_lcd(*this, "lcd")
		, m_dmadac(*this, "dac%u", 1U)
	{ }

	void cdimono1_base(machine_config &config);
	void cdimono1(machine_config &config);
	void cdimono2(machine_config &config);
	void cdi910(machine_config &config);

protected:
	virtual void machine_reset() override;

	void cdimono1_mem(address_map &map);

	required_device<scc68070_device> m_maincpu;

private:
	virtual void video_start() override;

	enum servo_portc_bit_t
	{
		INV_JUC_OUT = (1 << 2),
		INV_DIV4_IN = (1 << 5),
		INV_CADDYSWITCH_IN = (1 << 7)
	};

	void draw_lcd(int y);
	uint32_t screen_update_cdimono1_lcd(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void cdi910_mem(address_map &map);
	void cdimono2_mem(address_map &map);
	void cdi070_cpuspace(address_map &map);

	DECLARE_READ16_MEMBER(dvc_r);
	DECLARE_WRITE16_MEMBER(dvc_w);

	required_shared_ptr<uint16_t> m_planea;
	optional_device<cdislave_device> m_slave_hle;
	optional_device<m68hc05c8_device> m_servo;
	optional_device<m68hc05c8_device> m_slave;
	optional_device<cdicdic_device> m_cdic;
	required_device<cdda_device> m_cdda;
	required_device<mcd212_device> m_mcd212;
	optional_device<screen_device> m_lcd;

	required_device_array<dmadac_sound_device, 2> m_dmadac;

	bitmap_rgb32 m_lcdbitmap;
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

	uint8_t mcu_p1_r();

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
