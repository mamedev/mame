// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
    Dobbertin Smartwatch

    Dallas DS1216 Smartwatch + DS1315 Phantom Time chip

    Further info at: http://www.cpcwiki.eu/index.php/Dobbertin_Smart_Watch

*/

#ifndef SMARTWATCH_H_
#define SMARTWATCH_H_

#include "emu.h"
#include "cpcexp.h"
#include "machine/ds1315.h"

class cpc_smartwatch_device   : public device_t,
				public device_cpc_expansion_card_interface
{
public:
	// construction/destruction
	cpc_smartwatch_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const rom_entry *device_rom_region() const override;

	DECLARE_READ8_MEMBER(rtc_w);
	DECLARE_READ8_MEMBER(rtc_r);
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	cpc_expansion_slot_device *m_slot;

	required_device<ds1315_device> m_rtc;
	memory_bank* m_bank;
};

// device type definition
extern const device_type CPC_SMARTWATCH;


#endif /* SMARTWATCH_H_ */
