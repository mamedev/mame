// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Grundy NewBrain Expansion Interface Module emulation

**********************************************************************/

#pragma once

#ifndef __NEWBRAIN_EIM__
#define __NEWBRAIN_EIM__

#include "emu.h"
#include "exp.h"
#include "bus/rs232/rs232.h"
#include "machine/6850acia.h"
#include "machine/adc0808.h"
#include "machine/z80ctc.h"
#include "machine/ram.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> newbrain_eim_t

class newbrain_eim_t :  public device_t,
						public device_newbrain_expansion_slot_interface
{
public:
	// construction/destruction
	newbrain_eim_t(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	uint8_t anout_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void anout_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t anin_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void anio_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void acia_interrupt(int state);
	void ctc_z2_w(int state);
	void adc_eoc_w(int state);

	ADC0808_ANALOG_READ_CB(adc_vref_pos_r);
	ADC0808_ANALOG_READ_CB(adc_vref_neg_r);
	ADC0808_ANALOG_READ_CB(adc_input_r);

	void ctc_c2_tick(timer_device &timer, void *ptr, int32_t param);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_newbrain_expansion_slot_interface overrides
	virtual uint8_t mreq_r(address_space &space, offs_t offset, uint8_t data, bool &romov, int &exrm, bool &raminh) override;
	virtual void mreq_w(address_space &space, offs_t offset, uint8_t data, bool &romov, int &exrm, bool &raminh) override;
	virtual uint8_t iorq_r(address_space &space, offs_t offset, uint8_t data, bool &prtov) override;
	virtual void iorq_w(address_space &space, offs_t offset, uint8_t data, bool &prtov) override;

private:
	required_device<z80ctc_device> m_ctc;
	required_device<acia6850_device> m_acia;
	required_device<newbrain_expansion_slot_t> m_exp;
	required_memory_region m_rom;
	optional_shared_ptr<uint8_t> m_ram;

	int m_aciaint;
	int m_anint;
};


// device type definition
extern const device_type NEWBRAIN_EIM;



#endif
