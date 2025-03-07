// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************
 
    Speech Synthesizer sidecar device
    Michael Zapf

*****************************************************************************/

#ifndef MAME_BUS_TI99_SIDECAR_SPEECHSYN_H
#define MAME_BUS_TI99_SIDECAR_SPEECHSYN_H

#pragma once

#include "bus/ti99/internal/ioport.h"
#include "sound/tms5220.h"
#include "machine/tms6100.h"

namespace bus::ti99::sidecar {

class ti_speech_synthesizer_device : public bus::ti99::internal::ioport_attached_device
{
public:
	ti_speech_synthesizer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void readz(offs_t offset, uint8_t *value) override;
	void write(offs_t offset, uint8_t data) override;
	void setaddress_dbin(offs_t offset, int state) override;
	void sbe(int state) override;

	void crureadz(offs_t offset, uint8_t *value) override;
	void cruwrite(offs_t offset, uint8_t data) override;

	void memen_in(int state) override;
	void msast_in(int state) override;

	void clock_in(int state) override;
	void reset_in(int state) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// Callbacks from the external port
	void extint(int state);
	void extready(int state);

private:
	required_device<cd2501e_device> m_vsp;
	required_device<bus::ti99::internal::ioport_device>     m_port;
	
	void speech_ready(int state);

	bool m_reading;
	bool m_sbe;
	line_state m_ext_ready;
	line_state m_ssyn_ready;
};

} // end namespace bus::ti99::internal

DECLARE_DEVICE_TYPE_NS(TI99_SPEECHSYN, bus::ti99::sidecar, ti_speech_synthesizer_device)

#endif // MAME_BUS_TI99_SIDECAR_SPEECHSYN_H
