// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Mertec Compact Companion

        http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Mertec_CompactComp.html

**********************************************************************/


#ifndef MAME_BUS_BBC_EXP_MERTEC_H
#define MAME_BUS_BBC_EXP_MERTEC_H

#include "exp.h"
#include "machine/6821pia.h"
#include "machine/upd7002.h"
#include "bus/bbc/1mhzbus/1mhzbus.h"
#include "bus/bbc/analogue/analogue.h"
#include "bus/bbc/userport/userport.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bbc_mertec_device :
	public device_t,
	public device_bbc_exp_interface
{
public:
	// construction/destruction
	bbc_mertec_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);


protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual uint8_t fred_r(offs_t offset) override;
	virtual void fred_w(offs_t offset, uint8_t data) override;
	virtual uint8_t jim_r(offs_t offset) override;
	virtual void jim_w(offs_t offset, uint8_t data) override;
	virtual uint8_t sheila_r(offs_t offset) override;
	virtual void sheila_w(offs_t offset, uint8_t data) override;

	virtual uint8_t pb_r() override;
	virtual void pb_w(uint8_t data) override;

private:
	int get_analogue_input(int channel_number);
	void upd7002_eoc(int data);

	required_device<pia6821_device> m_pia;
	required_device<upd7002_device> m_upd7002;
	required_device<bbc_analogue_slot_device> m_analog;
	required_device<bbc_userport_slot_device> m_userport;
	required_device<bbc_1mhzbus_slot_device> m_2mhzbus;
	required_memory_region m_ext_rom;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_MERTEC, bbc_mertec_device);


#endif /* MAME_BUS_BBC_EXP_MERTEC_H */
