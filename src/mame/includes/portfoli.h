// license:BSD-3-Clause
// copyright-holders:Curt Coder
#pragma once

#ifndef __PORTFOLIO__
#define __PORTFOLIO__

#include "emu.h"
#include "cpu/i86/i86.h"
#include "bus/centronics/ctronics.h"
#include "machine/i8255.h"
#include "machine/ins8250.h"
#include "machine/nvram.h"
#include "machine/ram.h"
#include "sound/speaker.h"
#include "video/hd61830.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#define M80C88A_TAG     "u1"
#define M82C55A_TAG     "hpc101_u1"
#define M82C50A_TAG     "hpc102_u1"
#define HD61830_TAG     "hd61830"
#define CENTRONICS_TAG  "centronics"
#define TIMER_TICK_TAG  "tick"
#define SCREEN_TAG      "screen"
#define RS232_TAG      "rs232"

class portfolio_state : public driver_device
{
public:
	portfolio_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, M80C88A_TAG),
			m_lcdc(*this, HD61830_TAG),
			m_ppi(*this, M82C55A_TAG),
			m_uart(*this, M82C50A_TAG),
			m_speaker(*this, "speaker"),
			m_timer_tick(*this, TIMER_TICK_TAG),
			m_rom(*this, M80C88A_TAG),
			m_char_rom(*this, HD61830_TAG),
			m_y0(*this, "Y0"),
			m_y1(*this, "Y1"),
			m_y2(*this, "Y2"),
			m_y3(*this, "Y3"),
			m_y4(*this, "Y4"),
			m_y5(*this, "Y5"),
			m_y6(*this, "Y6"),
			m_y7(*this, "Y7"),
			m_battery(*this, "BATTERY"),
			m_contrast(*this, "contrast"),
			m_ram(*this, RAM_TAG)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<hd61830_device> m_lcdc;
	required_device<i8255_device> m_ppi;
	required_device<ins8250_device> m_uart;
	required_device<speaker_sound_device> m_speaker;
	required_device<timer_device> m_timer_tick;
	required_region_ptr<UINT8> m_rom;
	required_region_ptr<UINT8> m_char_rom;
	required_ioport m_y0;
	required_ioport m_y1;
	required_ioport m_y2;
	required_ioport m_y3;
	required_ioport m_y4;
	required_ioport m_y5;
	required_ioport m_y6;
	required_ioport m_y7;
	required_ioport m_battery;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	void check_interrupt();
	void trigger_interrupt(int level);
	void scan_keyboard();

	DECLARE_READ8_MEMBER( irq_status_r );
	DECLARE_READ8_MEMBER( keyboard_r );
	DECLARE_READ8_MEMBER( battery_r );
	DECLARE_READ8_MEMBER( counter_r );
	DECLARE_READ8_MEMBER( pid_r );

	DECLARE_WRITE8_MEMBER( irq_mask_w );
	DECLARE_WRITE8_MEMBER( sivr_w );
	DECLARE_WRITE8_MEMBER( speaker_w );
	DECLARE_WRITE8_MEMBER( power_w );
	DECLARE_WRITE8_MEMBER( unknown_w );
	DECLARE_WRITE8_MEMBER( counter_w );
	DECLARE_WRITE8_MEMBER( ncc1_w );

	DECLARE_WRITE_LINE_MEMBER( i8250_intrpt_w );

	/* interrupt state */
	UINT8 m_ip;                         /* interrupt pending */
	UINT8 m_ie;                         /* interrupt enable */
	UINT8 m_sivr;                       /* serial interrupt vector register */

	/* counter state */
	UINT16 m_counter;

	/* keyboard state */
	UINT8 m_keylatch;

	/* video state */
	required_shared_ptr<UINT8> m_contrast;

	/* peripheral state */
	UINT8 m_pid;                        /* peripheral identification */
	DECLARE_PALETTE_INIT(portfolio);
	TIMER_DEVICE_CALLBACK_MEMBER(keyboard_tick);
	TIMER_DEVICE_CALLBACK_MEMBER(system_tick);
	TIMER_DEVICE_CALLBACK_MEMBER(counter_tick);
	DECLARE_READ8_MEMBER(hd61830_rd_r);
	IRQ_CALLBACK_MEMBER(portfolio_int_ack);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( portfolio_cart );
	required_device<ram_device> m_ram;
};

#endif
