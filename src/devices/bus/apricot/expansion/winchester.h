// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    ACT Apricot Winchester Controller

***************************************************************************/

#ifndef MAME_BUS_APRICOT_WINCHESTER_H
#define MAME_BUS_APRICOT_WINCHESTER_H

#pragma once

#include "expansion.h"
#include "machine/wd1010.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> apricot_winchester_device

class apricot_winchester_device : public device_t, public device_apricot_expansion_card_interface
{
public:
	// construction/destruction
	apricot_winchester_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void hdc_intrq_w(int state);
	uint8_t hdc_data_r();
	void hdc_data_w(uint8_t data);
	uint8_t int_r();
	template<int N> void head_w(int state);
	template<int N> void drive_w(int state);
	void xferd_w(int state);
	void hbcr_w(int state);
	void clksel_w(int state);
	uint8_t data_r();
	void data_w(uint8_t data);

	void regs(address_map &map) ATTR_COLD;

	required_device<wd1010_device> m_hdc;

	std::unique_ptr<uint8_t[]> m_ram;
	unsigned m_ram_ptr;
	int m_int;
	int m_drive;
	int m_head;
	int m_hbcr;
	int m_clksel;
};

// device type definition
DECLARE_DEVICE_TYPE(APRICOT_WINCHESTER, apricot_winchester_device)

#endif // MAME_BUS_APRICOT_WINCHESTER_H
