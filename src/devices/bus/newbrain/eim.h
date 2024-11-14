// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Grundy NewBrain Expansion Interface Module emulation

**********************************************************************/

#ifndef MAME_BUS_NEWBRAIN_EIM_H
#define MAME_BUS_NEWBRAIN_EIM_H

#pragma once

#include "exp.h"
#include "bus/rs232/rs232.h"
#include "machine/6850acia.h"
#include "machine/adc0808.h"
#include "machine/z80ctc.h"
#include "machine/ram.h"
#include "machine/timer.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> newbrain_eim_device

class newbrain_eim_device :  public device_t, public device_newbrain_expansion_slot_interface
{
public:
	// construction/destruction
	newbrain_eim_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t anout_r();
	void anout_w(uint8_t data);
	uint8_t anin_r();
	void anio_w(uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_newbrain_expansion_slot_interface overrides
	virtual uint8_t mreq_r(offs_t offset, uint8_t data, bool &romov, int &exrm, bool &raminh) override;
	virtual void mreq_w(offs_t offset, uint8_t data, bool &romov, int &exrm, bool &raminh) override;
	virtual uint8_t iorq_r(offs_t offset, uint8_t data, bool &prtov) override;
	virtual void iorq_w(offs_t offset, uint8_t data, bool &prtov) override;

private:
	void acia_interrupt(int state);
	void ctc_z2_w(int state);
	void adc_eoc_w(int state);

	TIMER_DEVICE_CALLBACK_MEMBER(ctc_c2_tick);

	required_device<z80ctc_device> m_ctc;
	required_device<acia6850_device> m_acia;
	required_device<newbrain_expansion_slot_device> m_exp;
	required_memory_region m_rom;
	optional_shared_ptr<uint8_t> m_ram;

	int m_aciaint;
	int m_anint;
};


// device type definition
DECLARE_DEVICE_TYPE(NEWBRAIN_EIM, newbrain_eim_device)

#endif // MAME_BUS_NEWBRAIN_EIM_H
