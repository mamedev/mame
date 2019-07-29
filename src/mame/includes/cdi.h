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
#include "screen.h"

/*----------- driver state -----------*/

class cdi_state : public driver_device
{
public:
	cdi_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_planea(*this, "mcd212:planea")
		, m_input1(*this, "INPUT1")
		, m_input2(*this, "INPUT2")
		, m_slave_hle(*this, "slave_hle")
		, m_servo(*this, "servo")
		, m_slave(*this, "slave")
		, m_cdic(*this, "cdic")
		, m_cdda(*this, "cdda")
		, m_mcd212(*this, "mcd212")
		, m_lcd(*this, "lcd")
		, m_dmadac(*this, "dac%u", 1U)
	{ }

	enum m68hc05eg_io_reg_t
	{
		PORT_A_DATA = 0x00,
		PORT_B_DATA = 0x01,
		PORT_C_DATA = 0x02,
		PORT_D_INPUT = 0x03,
		PORT_A_DDR = 0x04,
		PORT_B_DDR = 0x05,
		PORT_C_DDR = 0x06,
		SPI_CTRL = 0x0a,
		SPI_STATUS = 0x0b,
		SPI_DATA = 0x0c,
		SCC_BAUD = 0x0d,
		SCC_CTRL1 = 0x0e,
		SCC_CTRL2 = 0x0f,
		SCC_STATUS = 0x10,
		SCC_DATA = 0x11,
		TIMER_CTRL = 0x12,
		TIMER_STATUS = 0x13,
		ICAP_HI = 0x14,
		ICAP_LO = 0x15,
		OCMP_HI = 0x16,
		OCMP_LO = 0x17,
		COUNT_HI = 0x18,
		COUNT_LO = 0x19,
		ACOUNT_HI = 0x1a,
		ACOUNT_LO = 0x1b
	};

	enum servo_portc_bit_t
	{
		INV_JUC_OUT = (1 << 2),
		INV_DIV4_IN = (1 << 5),
		INV_CADDYSWITCH_IN = (1 << 7)
	};

	required_device<scc68070_device> m_maincpu;
	required_shared_ptr<uint16_t> m_planea;
	optional_ioport m_input1;
	optional_ioport m_input2;
	optional_device<cdislave_device> m_slave_hle;
	optional_device<cpu_device> m_servo;
	optional_device<cpu_device> m_slave;
	optional_device<cdicdic_device> m_cdic;
	required_device<cdda_device> m_cdda;
	required_device<mcd212_device> m_mcd212;
	optional_device<screen_device> m_lcd;

	required_device_array<dmadac_sound_device, 2> m_dmadac;

	INTERRUPT_GEN_MEMBER( mcu_frame );

	uint8_t m_servo_io_regs[0x20];
	uint8_t m_slave_io_regs[0x20];

	uint8_t m_timer_set;
	emu_timer *m_test_timer;

	bitmap_rgb32 m_lcdbitmap;

	DECLARE_INPUT_CHANGED_MEMBER(mcu_input);

	virtual void machine_start() override { }
	virtual void video_start() override;

	DECLARE_MACHINE_RESET(cdimono1);
	DECLARE_MACHINE_RESET(cdimono2);
	DECLARE_MACHINE_RESET(quizard1);
	DECLARE_MACHINE_RESET(quizard2);
	DECLARE_MACHINE_RESET(quizard3);
	DECLARE_MACHINE_RESET(quizard4);
	DECLARE_READ8_MEMBER(servo_io_r);
	DECLARE_WRITE8_MEMBER(servo_io_w);
	DECLARE_READ8_MEMBER(slave_io_r);
	DECLARE_WRITE8_MEMBER(slave_io_w);

	DECLARE_READ8_MEMBER(quizard_mcu_p1_r);

	void draw_lcd(int y);
	uint32_t screen_update_cdimono1_lcd(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void cdimono1(machine_config &config);
	void cdimono2(machine_config &config);
	void quizard4(machine_config &config);
	void cdimono1_base(machine_config &config);
	void cdi910(machine_config &config);
	void quizard2(machine_config &config);
	void quizard3(machine_config &config);
	void quizard1(machine_config &config);
	void quizard(machine_config &config);

	void cdi910_mem(address_map &map);
	void cdimono1_mem(address_map &map);
	void cdimono2_mem(address_map &map);
	void cdimono2_servo_mem(address_map &map);
	void cdimono2_slave_mem(address_map &map);
	void cdi070_cpuspace(address_map &map);

	// Quizard Protection HLE
	void set_quizard_mcu_value(uint16_t value);
	void set_quizard_mcu_ack(uint8_t ack);
	void quizard_rx(uint8_t data);
	void quizard_calculate_state();
	void quizard_set_seeds(uint8_t *rx);
	void quizard_handle_byte_tx(uint8_t data);

	uint16_t m_seeds[10];
	uint8_t m_state[8];

	uint16_t m_mcu_value;
	uint8_t m_mcu_ack;
};

#endif // MAME_INCLUDES_CDI_H
