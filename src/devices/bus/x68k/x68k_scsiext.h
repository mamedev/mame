// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * x68k_scsiext.h
 *
 *  Created on: 5/06/2012
 */

#ifndef X68K_SCSIEXT_H_
#define X68K_SCSIEXT_H_

#include "emu.h"
#include "machine/mb89352.h"
#include "x68kexp.h"

class x68k_scsiext_device : public device_t,
							public device_x68k_expansion_card_interface
{
public:
	// construction/destruction
	x68k_scsiext_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const rom_entry *device_rom_region() const override;

	void irq_w(int state);
	void drq_w(int state);
	DECLARE_READ8_MEMBER(register_r);
	DECLARE_WRITE8_MEMBER(register_w);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
private:
	x68k_expansion_slot_device *m_slot;

	required_device<mb89352_device> m_spc;
};

// device type definition
extern const device_type X68K_SCSIEXT;


#endif /* X68K_SCSIEXT_H_ */
