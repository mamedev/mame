// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
#ifndef _INCLUDES_CDI_H_
#define _INCLUDES_CDI_H_

#include "machine/cdi070.h"
#include "machine/cdislave.h"
#include "machine/cdicdic.h"
#include "sound/dmadac.h"
#include "video/mcd212.h"

/*----------- driver state -----------*/

#define CLOCK_A XTAL_30MHz
#define CLOCK_B XTAL_19_6608MHz

class cdi_state : public driver_device
{
public:
	cdi_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_planea(*this, "planea")
		, m_planeb(*this, "planeb")
		, m_input1(*this, "INPUT1")
		, m_input2(*this, "INPUT2")
		, m_mousex(*this, "MOUSEX")
		, m_mousey(*this, "MOUSEY")
		, m_mousebtn(*this, "MOUSEBTN")
		, m_slave_hle(*this, "slave_hle")
		, m_servo(*this, "servo")
		, m_slave(*this, "slave")
		, m_scc(*this, "scc68070")
		, m_cdic(*this, "cdic")
		, m_cdda(*this, "cdda")
		, m_mcd212(*this, "mcd212") { }

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

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<UINT16> m_planea;
	required_shared_ptr<UINT16> m_planeb;
	optional_ioport m_input1;
	optional_ioport m_input2;
	required_ioport m_mousex;
	required_ioport m_mousey;
	required_ioport m_mousebtn;
	optional_device<cdislave_device> m_slave_hle;
	optional_device<cpu_device> m_servo;
	optional_device<cpu_device> m_slave;
	required_device<cdi68070_device> m_scc;
	optional_device<cdicdic_device> m_cdic;
	required_device<cdda_device> m_cdda;
	required_device<mcd212_device> m_mcd212;

	dmadac_sound_device *m_dmadac[2];

	INTERRUPT_GEN_MEMBER( mcu_frame );

	UINT8 m_servo_io_regs[0x20];
	UINT8 m_slave_io_regs[0x20];

	UINT8 m_timer_set;
	emu_timer *m_test_timer;

	bitmap_rgb32 m_lcdbitmap;

	DECLARE_INPUT_CHANGED_MEMBER(mcu_input);

	virtual void machine_start() override { }
	virtual void video_start() override;

	DECLARE_MACHINE_RESET(cdimono1);
	DECLARE_MACHINE_RESET(cdimono2);
	DECLARE_MACHINE_RESET(quizard12);
	DECLARE_MACHINE_RESET(quizard17);
	DECLARE_MACHINE_RESET(quizard18);
	DECLARE_MACHINE_RESET(quizard22);
	DECLARE_MACHINE_RESET(quizard23);
	DECLARE_MACHINE_RESET(quizard32);
	DECLARE_MACHINE_RESET(quizard34);
	DECLARE_MACHINE_RESET(quizard4r40);
	DECLARE_MACHINE_RESET(quizard4r41);
	DECLARE_MACHINE_RESET(quizard4r42);
	DECLARE_READ8_MEMBER(servo_io_r);
	DECLARE_WRITE8_MEMBER(servo_io_w);
	DECLARE_READ8_MEMBER(slave_io_r);
	DECLARE_WRITE8_MEMBER(slave_io_w);

	UINT32 screen_update_cdimono1(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_cdimono1_lcd(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

/*----------- debug defines -----------*/

#define VERBOSE_LEVEL   (1)

#define ENABLE_VERBOSE_LOG (0)

#define ENABLE_UART_PRINTING (0)

#endif // _INCLUDES_CDI_H_
