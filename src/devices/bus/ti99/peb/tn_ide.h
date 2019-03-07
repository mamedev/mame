// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    Nouspikel IDE controller card
    See tn_ide.c for documentation

    Michael Zapf

    February 2012: Rewritten as class

*****************************************************************************/

#ifndef MAME_BUS_TI99_PEB_TN_IDE_H
#define MAME_BUS_TI99_PEB_TN_IDE_H

#pragma once

#include "peribox.h"
#include "machine/ataintf.h"
#include "machine/ram.h"
#include "machine/rtc65271.h"

namespace bus { namespace ti99 { namespace peb {

class nouspikel_ide_interface_device : public device_t, public device_ti99_peribox_card_interface
{
public:
	nouspikel_ide_interface_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	DECLARE_READ8Z_MEMBER(readz) override;
	void write(offs_t offset, uint8_t data) override;

	DECLARE_READ8Z_MEMBER(crureadz) override;
	void cruwrite(offs_t offset, uint8_t data) override;

	void    do_inta(int state);
	bool    m_ata_irq;
	int     m_cru_register;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

private:
	DECLARE_WRITE_LINE_MEMBER(clock_interrupt_callback);
	DECLARE_WRITE_LINE_MEMBER(ide_interrupt_callback);

	required_device<rtc65271_device> m_rtc;
	required_device<ata_interface_device> m_ata;

	bool    m_clk_irq;
	bool    m_sram_enable;
	bool    m_sram_enable_dip;
	int     m_cur_page;

	bool    m_tms9995_mode;

	uint16_t  m_input_latch;
	uint16_t  m_output_latch;

	required_device<ram_device> m_ram;
};

} } } // end namespace bus::ti99::peb

DECLARE_DEVICE_TYPE_NS(TI99_IDE, bus::ti99::peb, nouspikel_ide_interface_device)

#endif // MAME_BUS_TI99_PEB_TN_IDE_H
