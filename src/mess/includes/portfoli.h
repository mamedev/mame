#pragma once

#ifndef __PORTFOLIO__
#define __PORTFOLIO__


#include "emu.h"
#include "cpu/i86/i86.h"
#include "imagedev/cartslot.h"
#include "machine/ram.h"
#include "imagedev/printer.h"
#include "machine/ctronics.h"
#include "machine/i8255.h"
#include "machine/ins8250.h"
#include "machine/nvram.h"
#include "sound/speaker.h"
#include "video/hd61830.h"

#define SCREEN_TAG		"screen"
#define M80C88A_TAG		"u1"
#define M82C55A_TAG		"hpc101_u1"
#define M82C50A_TAG		"hpc102_u1"
#define HD61830_TAG		"hd61830"
#define CENTRONICS_TAG	"centronics"
#define TIMER_TICK_TAG	"tick"

class portfolio_state : public driver_device
{
public:
	portfolio_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, M80C88A_TAG),
		  m_lcdc(*this, HD61830_TAG),
		  m_ppi(*this, M82C55A_TAG),
		  m_uart(*this, M82C50A_TAG),
		  m_speaker(*this, SPEAKER_TAG),
		  m_timer_tick(*this, TIMER_TICK_TAG)
	,
		m_contrast(*this, "contrast"){ }

	required_device<cpu_device> m_maincpu;
	required_device<hd61830_device> m_lcdc;
	required_device<i8255_device> m_ppi;
	required_device<ins8250_device> m_uart;
	required_device<device_t> m_speaker;
	required_device<timer_device> m_timer_tick;

	virtual void machine_start();
	virtual void machine_reset();

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
	UINT8 m_ip;							/* interrupt pending */
	UINT8 m_ie;							/* interrupt enable */
	UINT8 m_sivr;						/* serial interrupt vector register */

	/* counter state */
	UINT16 m_counter;

	/* keyboard state */
	UINT8 m_keylatch;

	/* video state */
	required_shared_ptr<UINT8> m_contrast;

	/* peripheral state */
	UINT8 m_pid;						/* peripheral identification */
	virtual void palette_init();
};

#endif
