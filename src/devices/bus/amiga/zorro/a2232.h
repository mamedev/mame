// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Commodore A2232

    Zorro-II Serial Card

    Provides the Amiga with 7 additional RS232 ports.

***************************************************************************/

#ifndef MAME_BUS_AMIGA_ZORRO_A2232_H
#define MAME_BUS_AMIGA_ZORRO_A2232_H

#pragma once

#include "zorro.h"
#include "machine/autoconfig.h"
#include "cpu/m6502/m65ce02.h"
#include "machine/input_merger.h"
#include "machine/mos6551.h"
#include "machine/mos6526.h"
#include "bus/rs232/rs232.h"


namespace bus::amiga::zorro {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2232_device : public device_t, public device_zorro2_card_interface, public amiga_autoconfig
{
public:
	// construction/destruction
	a2232_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// zorro slot
	uint16_t shared_ram_r(offs_t offset, uint16_t mem_mask = ~0);
	void shared_ram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t irq_ack_r();
	void irq_ack_w(uint16_t data);
	uint16_t reset_low_r();
	void reset_low_w(uint16_t data);
	uint16_t irq_r();
	void irq_w(uint16_t data);
	uint16_t reset_high_r(offs_t offset, uint16_t mem_mask = ~0);
	void reset_high_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void iocpu_map(address_map &map) ATTR_COLD;

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset_after_children() override;

	// device_zorro2_card_interface overrides
	virtual void cfgin_w(int state) override;

	// amiga_autoconfig overrides
	virtual void autoconfig_base_address(offs_t address) override;

private:
	// cpu
	void int2_w(uint8_t data);
	void irq_ack8_w(uint8_t data);

	// acia
	template<int N> uint8_t acia_r(offs_t offset);
	template<int N> void acia_w(offs_t offset, uint8_t data);

	// cia
	uint8_t cia_port_a_r();
	uint8_t cia_port_b_r();
	void cia_port_b_w(uint8_t data);
	uint8_t cia_r(offs_t offset);
	void cia_w(offs_t offset, uint8_t data);

	// rs232
	void rs232_1_rxd_w(int state);
	void rs232_1_dcd_w(int state);
	void rs232_1_cts_w(int state);
	void rs232_2_dcd_w(int state);
	void rs232_2_cts_w(int state);
	void rs232_3_dcd_w(int state);
	void rs232_3_cts_w(int state);
	void rs232_4_dcd_w(int state);
	void rs232_4_cts_w(int state);
	void rs232_5_dcd_w(int state);
	void rs232_5_cts_w(int state);
	void rs232_6_dcd_w(int state);
	void rs232_6_cts_w(int state);
	void rs232_7_dcd_w(int state);
	void rs232_7_cts_w(int state);

	required_device<m65ce02_device> m_iocpu;
	required_device<input_merger_device> m_ioirq;
	required_device_array<mos6551_device, 7> m_acia;
	required_device<mos8520_device> m_cia;
	required_shared_ptr<uint8_t> m_shared_ram;

	uint8_t m_cia_port_a;
	uint8_t m_cia_port_b;
};

} // namespace bus::amiga::zorro

// device type declaration
DECLARE_DEVICE_TYPE_NS(AMIGA_A2232, bus::amiga::zorro, a2232_device)

#endif // MAME_BUS_AMIGA_ZORRO_A2232_H
