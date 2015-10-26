// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * ddi1.h  --  Amstrad DDI-1 Floppy Disk Drive interface
 *
 * Provides uPD765A FDC, AMSDOS ROM, and 3" floppy disk drive.
 * CPC464 only, 664/6128/464+/6128+ already has this hardware built-in (AMSDOS is on the included Burnin' Rubber / BASIC cartridge for the 464+ and 6128+)
 *
 */

#ifndef CPC_DDI1_H_
#define CPC_DDI1_H_

#include "emu.h"
#include "cpcexp.h"
#include "machine/upd765.h"

class cpc_ddi1_device : public device_t,
			public device_cpc_expansion_card_interface
{
public:
	// construction/destruction
	cpc_ddi1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual void set_mapping(UINT8 type);
	virtual WRITE_LINE_MEMBER( romen_w ) { m_romen = state; }

	DECLARE_WRITE8_MEMBER(motor_w);
	DECLARE_WRITE8_MEMBER(fdc_w);
	DECLARE_READ8_MEMBER(fdc_r);
	DECLARE_WRITE8_MEMBER(rombank_w);
protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

private:
	cpc_expansion_slot_device *m_slot;

	required_device<upd765_family_device> m_fdc;
	required_device<floppy_connector> m_connector;
	
	bool m_rom_active;
	bool m_romen;
};

// device type definition
extern const device_type CPC_DDI1;

#endif /* CPC_DDI1_H_ */
