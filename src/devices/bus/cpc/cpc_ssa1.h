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

#ifndef CPC_SSA1_H_
#define CPC_SSA1_H_


#include "emu.h"
#include "cpcexp.h"
#include "sound/sp0256.h"

class cpc_ssa1_device : public device_t,
						public device_cpc_expansion_card_interface
{
public:
	// construction/destruction
	cpc_ssa1_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	void set_lrq(UINT8 state) { m_lrq = state; }
	UINT8 get_lrq() { return m_lrq; }
	void set_sby(UINT8 state) { m_sby = state; }
	UINT8 get_sby() { return m_sby; }

	DECLARE_READ8_MEMBER(ssa1_r);
	DECLARE_WRITE8_MEMBER(ssa1_w);
	DECLARE_WRITE_LINE_MEMBER(lrq_cb);
	DECLARE_WRITE_LINE_MEMBER(sby_cb);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	cpc_expansion_slot_device *m_slot;

	UINT8 *m_rom;
	UINT8 m_lrq;
	UINT8 m_sby;

	required_device<sp0256_device> m_sp0256_device;
};

class cpc_dkspeech_device : public device_t,
							public device_cpc_expansion_card_interface
{
public:
	// construction/destruction
	cpc_dkspeech_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	void set_lrq(UINT8 state) { m_lrq = state; }
	UINT8 get_lrq() { return m_lrq; }
	void set_sby(UINT8 state) { m_sby = state; }
	UINT8 get_sby() { return m_sby; }

	DECLARE_READ8_MEMBER(dkspeech_r);
	DECLARE_WRITE8_MEMBER(dkspeech_w);
	DECLARE_WRITE_LINE_MEMBER(lrq_cb);
	DECLARE_WRITE_LINE_MEMBER(sby_cb);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	cpc_expansion_slot_device *m_slot;

	UINT8 *m_rom;
	UINT8 m_lrq;
	UINT8 m_sby;

	required_device<sp0256_device> m_sp0256_device;
};

// device type definition
extern const device_type CPC_SSA1;
extern const device_type CPC_DKSPEECH;


#endif /* CPC_SSA1_H_ */
