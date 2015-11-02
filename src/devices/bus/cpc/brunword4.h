// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*

    Brunword MK4 - Word processor ROM / expansion

*/

#include "emu.h"
#include "cpcexp.h"

class cpc_brunword4_device  : public device_t,
				public device_cpc_expansion_card_interface
{
public:
	// construction/destruction
	cpc_brunword4_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;

	DECLARE_WRITE8_MEMBER(rombank_w);
	virtual void set_mapping(UINT8 type);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

private:
	cpc_expansion_slot_device *m_slot;

	bool m_rombank_active;
	UINT8 m_bank_sel;
};

// device type definition
extern const device_type CPC_BRUNWORD_MK4;
