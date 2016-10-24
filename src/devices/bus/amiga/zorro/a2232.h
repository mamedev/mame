// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Commodore A2232

    Zorro-II Serial Card

    Provides the Amiga with 7 additional RS232 ports.

***************************************************************************/

#pragma once

#ifndef __A2232_H__
#define __A2232_H__

#include "emu.h"
#include "zorro.h"
#include "machine/autoconfig.h"
#include "cpu/m6502/m65ce02.h"
#include "machine/mos6551.h"
#include "machine/mos6526.h"
#include "bus/rs232/rs232.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> a2232_device

class a2232_device : public device_t, public device_zorro2_card_interface, public amiga_autoconfig
{
public:
	// construction/destruction
	a2232_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// cpu
	void int2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask);
	void irq_ack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask);

	// zorro slot
	uint16_t shared_ram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void shared_ram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t irq_ack_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void irq_ack_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t reset_low_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void reset_low_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t irq_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void irq_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t reset_high_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void reset_high_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	// acia
	uint8_t acia_0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void acia_0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void acia_0_irq_w(int state);
	uint8_t acia_1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void acia_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void acia_1_irq_w(int state);
	uint8_t acia_2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void acia_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void acia_2_irq_w(int state);
	uint8_t acia_3_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void acia_3_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void acia_3_irq_w(int state);
	uint8_t acia_4_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void acia_4_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void acia_4_irq_w(int state);
	uint8_t acia_5_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void acia_5_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void acia_5_irq_w(int state);
	uint8_t acia_6_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void acia_6_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void acia_6_irq_w(int state);

	// cia
	uint8_t cia_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void cia_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void cia_irq_w(int state);
	uint8_t cia_port_a_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t cia_port_b_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void cia_port_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

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

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;
	virtual void device_reset_after_children() override;

	// device_zorro2_card_interface overrides
	virtual void cfgin_w(int state) override;

	// amiga_autoconfig overrides
	virtual void autoconfig_base_address(offs_t address) override;

private:
	enum
	{
		IRQ_ACIA_0,
		IRQ_ACIA_1,
		IRQ_ACIA_2,
		IRQ_ACIA_3,
		IRQ_ACIA_4,
		IRQ_ACIA_5,
		IRQ_ACIA_6,
		IRQ_CIA,
		IRQ_AMIGA,
		IRQ_SOURCE_COUNT
	};

	void update_irqs();

	required_device<m65ce02_device> m_iocpu;
	required_device<mos6551_device> m_acia_0;
	required_device<mos6551_device> m_acia_1;
	required_device<mos6551_device> m_acia_2;
	required_device<mos6551_device> m_acia_3;
	required_device<mos6551_device> m_acia_4;
	required_device<mos6551_device> m_acia_5;
	required_device<mos6551_device> m_acia_6;
	required_device<mos8520_device> m_cia;
	required_shared_ptr<uint8_t> m_shared_ram;

	int m_irqs[IRQ_SOURCE_COUNT];

	uint8_t m_cia_port_a;
	uint8_t m_cia_port_b;
};

// device type definition
extern const device_type A2232;

#endif // __A2232_H__
