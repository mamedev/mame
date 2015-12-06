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
	a2232_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// cpu
	WRITE8_MEMBER( int2_w );
	WRITE8_MEMBER( irq_ack_w );

	// zorro slot
	DECLARE_READ16_MEMBER( shared_ram_r );
	DECLARE_WRITE16_MEMBER( shared_ram_w );
	DECLARE_READ16_MEMBER( irq_ack_r );
	DECLARE_WRITE16_MEMBER( irq_ack_w );
	DECLARE_READ16_MEMBER( reset_low_r );
	DECLARE_WRITE16_MEMBER( reset_low_w );
	DECLARE_READ16_MEMBER( irq_r );
	DECLARE_WRITE16_MEMBER( irq_w );
	DECLARE_READ16_MEMBER( reset_high_r );
	DECLARE_WRITE16_MEMBER( reset_high_w );

	// acia
	DECLARE_READ8_MEMBER( acia_0_r );
	DECLARE_WRITE8_MEMBER( acia_0_w );
	DECLARE_WRITE_LINE_MEMBER( acia_0_irq_w );
	DECLARE_READ8_MEMBER( acia_1_r );
	DECLARE_WRITE8_MEMBER( acia_1_w );
	DECLARE_WRITE_LINE_MEMBER( acia_1_irq_w );
	DECLARE_READ8_MEMBER( acia_2_r );
	DECLARE_WRITE8_MEMBER( acia_2_w );
	DECLARE_WRITE_LINE_MEMBER( acia_2_irq_w );
	DECLARE_READ8_MEMBER( acia_3_r );
	DECLARE_WRITE8_MEMBER( acia_3_w );
	DECLARE_WRITE_LINE_MEMBER( acia_3_irq_w );
	DECLARE_READ8_MEMBER( acia_4_r );
	DECLARE_WRITE8_MEMBER( acia_4_w );
	DECLARE_WRITE_LINE_MEMBER( acia_4_irq_w );
	DECLARE_READ8_MEMBER( acia_5_r );
	DECLARE_WRITE8_MEMBER( acia_5_w );
	DECLARE_WRITE_LINE_MEMBER( acia_5_irq_w );
	DECLARE_READ8_MEMBER( acia_6_r );
	DECLARE_WRITE8_MEMBER( acia_6_w );
	DECLARE_WRITE_LINE_MEMBER( acia_6_irq_w );

	// cia
	DECLARE_READ8_MEMBER( cia_r );
	DECLARE_WRITE8_MEMBER( cia_w );
	DECLARE_WRITE_LINE_MEMBER( cia_irq_w );
	DECLARE_READ8_MEMBER( cia_port_a_r );
	DECLARE_READ8_MEMBER( cia_port_b_r );
	DECLARE_WRITE8_MEMBER( cia_port_b_w );

	// rs232
	DECLARE_WRITE_LINE_MEMBER( rs232_1_rxd_w );
	DECLARE_WRITE_LINE_MEMBER( rs232_1_dcd_w );
	DECLARE_WRITE_LINE_MEMBER( rs232_1_cts_w );
	DECLARE_WRITE_LINE_MEMBER( rs232_2_dcd_w );
	DECLARE_WRITE_LINE_MEMBER( rs232_2_cts_w );
	DECLARE_WRITE_LINE_MEMBER( rs232_3_dcd_w );
	DECLARE_WRITE_LINE_MEMBER( rs232_3_cts_w );
	DECLARE_WRITE_LINE_MEMBER( rs232_4_dcd_w );
	DECLARE_WRITE_LINE_MEMBER( rs232_4_cts_w );
	DECLARE_WRITE_LINE_MEMBER( rs232_5_dcd_w );
	DECLARE_WRITE_LINE_MEMBER( rs232_5_cts_w );
	DECLARE_WRITE_LINE_MEMBER( rs232_6_dcd_w );
	DECLARE_WRITE_LINE_MEMBER( rs232_6_cts_w );
	DECLARE_WRITE_LINE_MEMBER( rs232_7_dcd_w );
	DECLARE_WRITE_LINE_MEMBER( rs232_7_cts_w );

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;
	virtual void device_reset_after_children() override;

	// device_zorro2_card_interface overrides
	virtual DECLARE_WRITE_LINE_MEMBER( cfgin_w ) override;

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
	required_shared_ptr<UINT8> m_shared_ram;

	int m_irqs[IRQ_SOURCE_COUNT];

	UINT8 m_cia_port_a;
	UINT8 m_cia_port_b;
};

// device type definition
extern const device_type A2232;

#endif // __A2232_H__
