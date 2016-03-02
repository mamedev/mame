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
	newbrain_eim_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	DECLARE_READ8_MEMBER( anout_r );
	DECLARE_WRITE8_MEMBER( anout_w );
	DECLARE_READ8_MEMBER( anin_r );
	DECLARE_WRITE8_MEMBER( anio_w );
	DECLARE_READ8_MEMBER( st0_r );
	DECLARE_READ8_MEMBER( st1_r );
	DECLARE_READ8_MEMBER( st2_r );
	DECLARE_READ8_MEMBER( usbs_r );
	DECLARE_WRITE8_MEMBER( usbs_w );
	DECLARE_WRITE8_MEMBER( paging_w );
	DECLARE_WRITE_LINE_MEMBER( acia_tx );
	DECLARE_WRITE_LINE_MEMBER( acia_interrupt );
	DECLARE_WRITE_LINE_MEMBER( ctc_z2_w );
	DECLARE_WRITE_LINE_MEMBER( adc_eoc_w );

	ADC0808_ANALOG_READ_CB(adc_vref_pos_r);
	ADC0808_ANALOG_READ_CB(adc_vref_neg_r);
	ADC0808_ANALOG_READ_CB(adc_input_r);

	TIMER_DEVICE_CALLBACK_MEMBER(ctc_c2_tick);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_newbrain_expansion_slot_interface overrides
	virtual UINT8 mreq_r(address_space &space, offs_t offset, UINT8 data, bool &romov, int &exrm, bool &raminh) override;
	virtual void mreq_w(address_space &space, offs_t offset, UINT8 data, bool &romov, int &exrm, bool &raminh) override;
	virtual UINT8 iorq_r(address_space &space, offs_t offset, UINT8 data, bool &prtov) override;
	virtual void iorq_w(address_space &space, offs_t offset, UINT8 data, bool &prtov) override;

private:
	required_device<z80ctc_device> m_ctc;
	required_device<acia6850_device> m_acia;
	required_device<newbrain_expansion_slot_t> m_exp;
	required_memory_region m_rom;
	optional_shared_ptr<UINT8> m_ram;

	int m_aciaint;
	int m_anint;
};


// device type definition
extern const device_type NEWBRAIN_EIM;



#endif
