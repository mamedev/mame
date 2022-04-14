// license:BSD-3-Clause
// copyright-holders:Robbbert, Vas Crabb
#ifndef MAME_INCLUDES_ZORBA_H
#define MAME_INCLUDES_ZORBA_H

#pragma once

#include "sound/beep.h"

#include "bus/ieee488/ieee488.h"

#include "imagedev/floppy.h"

#include "machine/6821pia.h"
#include "machine/i8251.h"
#include "machine/wd_fdc.h"
#include "machine/z80dma.h"

#include "video/i8275.h"

#include "emupal.h"


class zorba_state : public driver_device
{
public:
	zorba_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_config_port(*this, "CNF")
		, m_rom(*this, "maincpu")
		, m_ram(*this, "mainram")
		, m_bank1(*this, "bank1")
		, m_p_chargen(*this, "chargen")
		, m_maincpu(*this, "maincpu")
		, m_dma(*this, "dma")
		, m_uart0(*this, "uart0")
		, m_uart1(*this, "uart1")
		, m_uart2(*this, "uart2")
		, m_pia0(*this, "pia0")
		, m_pia1(*this, "pia1")
		, m_palette(*this, "palette")
		, m_crtc(*this, "crtc")
		, m_beep(*this, "beeper")
		, m_fdc (*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		, m_floppy1(*this, "fdc:1")
		, m_ieee(*this, IEEE488_TAG)
	{
	}

	DECLARE_INPUT_CHANGED_MEMBER(printer_type);
	void zorba(machine_config &config);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	void zorba_io(address_map &map);
	void zorba_mem(address_map &map);

	// Memory banking control
	uint8_t ram_r();
	void ram_w(uint8_t data);
	uint8_t rom_r();
	void rom_w(uint8_t data);

	// Interrupt vectoring glue
	void intmask_w(uint8_t data);
	template <unsigned N> DECLARE_WRITE_LINE_MEMBER(tx_rx_rdy_w);
	template <unsigned N> DECLARE_WRITE_LINE_MEMBER(irq_w);

	// DMA controller handlers
	DECLARE_WRITE_LINE_MEMBER(busreq_w);
	uint8_t memory_read_byte(offs_t offset);
	void memory_write_byte(offs_t offset, uint8_t data);
	uint8_t io_read_byte(offs_t offset);
	void io_write_byte(offs_t offset, uint8_t data);

	// PIT handlers
	DECLARE_WRITE_LINE_MEMBER(br1_w);

	// PIA handlers
	void pia0_porta_w(uint8_t data);
	uint8_t pia1_portb_r();
	void pia1_portb_w(uint8_t data);

	// Video
	I8275_DRAW_CHARACTER_MEMBER(zorba_update_chr);

	// Printer port glue
	DECLARE_WRITE_LINE_MEMBER(printer_fault_w);
	DECLARE_WRITE_LINE_MEMBER(printer_select_w);

	required_ioport                     m_config_port;

	required_region_ptr<u8>             m_rom;
	required_shared_ptr<u8>             m_ram;
	required_memory_bank                m_bank1;
	required_region_ptr<uint8_t>        m_p_chargen;

	required_device<cpu_device>         m_maincpu;
	required_device<z80dma_device>      m_dma;
	required_device<i8251_device>       m_uart0;
	required_device<i8251_device>       m_uart1;
	required_device<i8251_device>       m_uart2;
	required_device<pia6821_device>     m_pia0;
	required_device<pia6821_device>     m_pia1;

	required_device<palette_device>     m_palette;
	required_device<i8275_device>       m_crtc;

	required_device<beep_device>        m_beep;

	required_device<fd1793_device>      m_fdc;
	required_device<floppy_connector>   m_floppy0;
	required_device<floppy_connector>   m_floppy1;

	required_device<ieee488_device>     m_ieee;

	uint8_t m_intmask = 0U;
	uint8_t m_tx_rx_rdy = 0U;
	uint8_t m_irq = 0U;

	bool    m_printer_prowriter = false;
	int     m_printer_fault = 0;
	int     m_printer_select = 0;

	uint8_t m_term_data = 0U;
};

#endif // MAME_INCLUDES_ZORBA_H
