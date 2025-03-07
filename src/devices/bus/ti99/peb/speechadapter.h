// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    TI-99 Speech Synthesizer connector adapter 
    for the Peripheral Expansion Box

    Michael Zapf, March 2025

*****************************************************************************/

#ifndef MAME_BUS_TI99_PEB_SPCHADPT_H
#define MAME_BUS_TI99_PEB_SPCHADPT_H

#pragma once

#include "peribox.h"
#include "bus/ti99/internal/ioport.h"

namespace bus::ti99::peb {

class ti_speechsyn_adapter_device : public device_t, public device_ti99_peribox_card_interface
{
public:
	ti_speechsyn_adapter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void readz(offs_t offset, uint8_t *value) override;
	void write(offs_t offset, uint8_t data) override;
	void setaddress_dbin(offs_t offset, int state) override;
	void crureadz(offs_t offset, uint8_t *value) override { }
	void cruwrite(offs_t offset, uint8_t data) override { }
	
protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_device<bus::ti99::internal::ioport_device> m_port;
	bool m_dec_high;

	void ready(int state);
};

} // end namespace bus::ti99::peb

DECLARE_DEVICE_TYPE_NS(TI99_SPEECHADAPTER, bus::ti99::peb, ti_speechsyn_adapter_device)

#endif // MAME_BUS_TI99_PEB_SPCHADPT_H
