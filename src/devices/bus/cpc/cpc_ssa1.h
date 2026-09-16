// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * cpc_ssa1.h  --  Amstrad SSA-1 Speech Synthesiser, dk'Tronics Speech Synthesiser
 *
 *  Created on: 16/07/2011
 *
 *  Amstrad SSA-1 - SP0256-AL2 based Speech Synthesiser and Sound Amplifier
 *
 *  Uses on-board resonator, clocked at 3.12MHz
 *
 *  Decodes only I/O lines A10, A4 and A0
 *  Official I/O ports:
 *    &FBEE (read)
 *     - bit 7: SP0256 Status 1 (SBY)
 *     - bit 6: SP0256 Status 2 (/LRQ)
 *
 *    &FBEE (write)
 *     - bits 7-0: SP0256 Allophone number (must be 0x00 to 0x3f, however, all data lines are hooked up)
 *
 *    &FAEE (write)
 *     - same as above, used because of a bug in the driver software, but still works due to the way the I/O ports are
 *       decoded on the CPC.
 *
 *  More info and PCB pics at http://www.cpcwiki.eu/index.php/Amstrad_SSA-1_Speech_Synthesizer
 *
 *
 *  dk'Tronics Speech Synthesiser - SP0256-AL2 based speech synthesiser
 *
 *  Uses the CPC's clock of 4MHz from pin 50 of the expansion port, gives faster and higher pitched voices than the SSA-1
 *
 *  Official I/O ports:
 *    &FBFE (read)
 *     - bit 7: SP0256 Status 2 (/LRQ)
 *
 *    &FBFE (write)
 *     - bits 5-0: SP0256 Allophone number
 *
 *  More info and PCB pics at http://www.cpcwiki.eu/index.php/Dk%27tronics_Speech_Synthesizer
 *
 */

#ifndef MAME_BUS_CPC_CPC_SSA1_H
#define MAME_BUS_CPC_CPC_SSA1_H

#pragma once


#include "cpcexp.h"
#include "sound/sp0256.h"

class cpc_ssa1_device : public device_t,
						public device_cpc_expansion_card_interface
{
public:
	// construction/destruction
	cpc_ssa1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t ssa1_r();
	void ssa1_w(uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	cpc_expansion_slot_device *m_slot;
	uint8_t *m_rom;
	required_device<sp0256_device> m_sp0256_device;
};

class cpc_dkspeech_device : public device_t,
							public device_cpc_expansion_card_interface
{
public:
	// construction/destruction
	cpc_dkspeech_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t dkspeech_r();
	void dkspeech_w(uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	cpc_expansion_slot_device *m_slot;
	uint8_t *m_rom;
	required_device<sp0256_device> m_sp0256_device;
};

// device type definition
DECLARE_DEVICE_TYPE(CPC_SSA1,     cpc_ssa1_device)
DECLARE_DEVICE_TYPE(CPC_DKSPEECH, cpc_dkspeech_device)


#endif // MAME_BUS_CPC_CPC_SSA1_H
