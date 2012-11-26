#pragma once

#ifndef __XOR100__
#define __XOR100__

#include "emu.h"
#include "cpu/z80/z80.h"
#include "formats/basicdsk.h"
#include "imagedev/flopdrv.h"
#include "machine/ram.h"
#include "machine/com8116.h"
#include "machine/ctronics.h"
#include "machine/i8255.h"
#include "machine/i8251.h"
#include "machine/terminal.h"
#include "machine/wd17xx.h"
#include "machine/z80ctc.h"

#define SCREEN_TAG		"screen"
#define Z80_TAG			"5b"
#define I8251_A_TAG		"12b"
#define I8251_B_TAG		"14b"
#define I8255A_TAG		"8a"
#define COM5016_TAG		"15c"
#define Z80CTC_TAG		"11b"
#define WD1795_TAG		"wd1795"
#define CENTRONICS_TAG	"centronics"

class xor100_state : public driver_device
{
public:
	xor100_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, Z80_TAG),
		  m_dbrg(*this, COM5016_TAG),
		  m_uart_b(*this, I8251_B_TAG),
		  m_fdc(*this, WD1795_TAG),
		  m_ctc(*this, Z80CTC_TAG),
		  m_ram(*this, RAM_TAG),
		  m_terminal(*this, TERMINAL_TAG),
		  m_floppy0(*this, FLOPPY_0),
		  m_floppy1(*this, FLOPPY_1)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<com8116_device> m_dbrg;
	required_device<i8251_device> m_uart_b;
	required_device<fd1795_device> m_fdc;
	required_device<z80ctc_device> m_ctc;
	required_device<ram_device> m_ram;
	required_device<generic_terminal_device> m_terminal;
	required_device<legacy_floppy_image_device> m_floppy0;
	required_device<legacy_floppy_image_device> m_floppy1;

	virtual void machine_start();
	virtual void machine_reset();

	DECLARE_WRITE8_MEMBER( mmu_w );
	DECLARE_WRITE8_MEMBER( prom_toggle_w );
	DECLARE_READ8_MEMBER( prom_disable_r );
	DECLARE_WRITE8_MEMBER( baud_w );
	DECLARE_WRITE8_MEMBER( i8251_b_data_w );
	DECLARE_READ8_MEMBER( fdc_wait_r );
	DECLARE_WRITE8_MEMBER( fdc_dcont_w );
	DECLARE_WRITE8_MEMBER( fdc_dsel_w );
	DECLARE_WRITE_LINE_MEMBER( fdc_irq_w );
	DECLARE_WRITE_LINE_MEMBER( fdc_drq_w );

	void bankswitch();

	// memory state
	int m_mode;
	int m_bank;

	// floppy state
	int m_fdc_irq;
	int m_fdc_drq;
	int m_fdc_dden;
	DECLARE_WRITE_LINE_MEMBER(com5016_fr_w);
	DECLARE_WRITE_LINE_MEMBER(com5016_ft_w);
	DECLARE_READ8_MEMBER(i8255_pc_r);
	DECLARE_WRITE_LINE_MEMBER(ctc_z0_w);
	DECLARE_WRITE_LINE_MEMBER(ctc_z1_w);
	DECLARE_WRITE_LINE_MEMBER(ctc_z2_w);
	DECLARE_WRITE8_MEMBER(xor100_kbd_put);
};

#endif
