// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Oxford Computer Systems Interpod IEC to IEEE interface emulation

**********************************************************************/

#ifndef MAME_BUS_CBMIEC_INTERPOD_H
#define MAME_BUS_CBMIEC_INTERPOD_H

#pragma once

#include "cbmiec.h"
#include "bus/ieee488/ieee488.h"
#include "bus/rs232/rs232.h"
#include "cpu/m6502/m6502.h"
#include "machine/6522via.h"
#include "machine/6850acia.h"
#include "machine/mos6530.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cbm_interpod_device

class cbm_interpod_device : public device_t,
							public device_cbm_iec_interface
{
public:
	// construction/destruction
	cbm_interpod_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t via_pa_r();
	void via_pa_w(uint8_t data);
	uint8_t via_pb_r();
	void via_pb_w(uint8_t data);
	void iec_clk_w(int state);
	void iec_data_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_cbm_iec_interface overrides
	virtual void cbm_iec_atn(int state) override;
	virtual void cbm_iec_reset(int state) override;

	required_device<cpu_device> m_maincpu;
	required_device<via6522_device> m_via;
	required_device<mos6532_device> m_riot;
	required_device<acia6850_device> m_acia;
	required_device<ieee488_device> m_ieee;
	required_device<rs232_port_device> m_rs232;
	output_finder<> m_led;

private:
	void interpod_mem(address_map &map) ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(CBM_INTERPOD, cbm_interpod_device)


#endif // MAME_BUS_CBMIEC_INTERPOD_H
