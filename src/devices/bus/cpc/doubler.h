// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * doubler.c  --  Draysoft Doubler - external cassette interface for the 464 (works on 664/6128 with external cassette?), 
 *                intended for use in duplicating cassette software
 *
 * Uses only port F0E0 (may conflict with other peripherals, PPI port A is not usable while Doubler software is running)
 *
 */
 
#ifndef DOUBLER_H_
#define DOUBLER_H_

#include "emu.h"
#include "cpcexp.h"
#include "imagedev/cassette.h"
#include "formats/tzx_cas.h"

class cpc_doubler_device  : public device_t,
						public device_cpc_expansion_card_interface
{
public:
	// construction/destruction
	cpc_doubler_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;

	DECLARE_READ8_MEMBER(ext_tape_r);
	
protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

private:
	cpc_expansion_slot_device *m_slot;
	
	required_device<cassette_image_device> m_tape;
};

// device type definition
extern const device_type CPC_DOUBLER;
 
#endif /* DOUBLER_H_ */
