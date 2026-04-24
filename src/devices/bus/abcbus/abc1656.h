// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor ABC 1656 bus expander emulation

*********************************************************************/

#ifndef MAME_BUS_ABCBUS_ABC1656_H
#define MAME_BUS_ABCBUS_ABC1656_H

#pragma once


#include "abcbus.h"
#include "machine/input_merger.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> abc1656_device

class abc1656_device :  public device_t, public device_abcbus_card_interface
{
public:
	// construction/destruction
	abc1656_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_abcbus_interface overrides
	virtual void abcbus_cs(uint8_t data) override;
	virtual int abcbus_csb() override { return m_bus[0]->csb_r(); }
	virtual int abcbus_xcsb2() override { return m_bus[1]->csb_r(); }
	virtual int abcbus_xcsb3() override { return m_bus[2]->csb_r(); }
	virtual int abcbus_xcsb4() override { return m_bus[3]->csb_r(); }
	virtual int abcbus_xcsb5() override { return m_bus[4]->csb_r(); }
	virtual uint8_t abcbus_inp() override;
	virtual void abcbus_out(uint8_t data) override;
	virtual uint8_t abcbus_stat() override;
	virtual void abcbus_c1(uint8_t data) override;
	virtual void abcbus_c2(uint8_t data) override;
	virtual void abcbus_c3(uint8_t data) override;
	virtual void abcbus_c4(uint8_t data) override;
	virtual uint8_t abcbus_tren() override;
	virtual void abcbus_tren(uint8_t data) override;
	virtual void abcbus_prac(int state) override;

private:
	void irq_w(int state) { m_slot->irq_w(state); }
	void xint2_w(int state) { m_slot->xint2_w(state); }
	void xint3_w(int state) { m_slot->xint3_w(state); }
	void xint4_w(int state) { m_slot->xint4_w(state); }
	void xint5_w(int state) { m_slot->xint5_w(state); }
	void pren_w(int state) { m_slot->pren_w(state); }
	void trrq_w(int state) { m_slot->trrq_w(state); }

	required_device_array<abcbus_slot_device, 5> m_bus;
};


// device type definition
DECLARE_DEVICE_TYPE(ABC1656, abc1656_device);


#endif // MAME_BUS_ABCBUS_ABC1656_H
