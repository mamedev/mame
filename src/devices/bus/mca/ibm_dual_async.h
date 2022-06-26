// license:BSD-3-Clause
// copyright-holders:Katherine Rohl
/***************************************************************************

    IBM Dual Asynchronous Adapter 
    FRU 90X9229

    16-bit MCA card with two 16550 UARTs.

***************************************************************************/

#ifndef MAME_BUS_MCA_DUALASYNC_H
#define MAME_BUS_MCA_DUALASYNC_H

#pragma once

#include "mca.h"
#include "machine/ins8250.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> isa8_com_device

class mca16_ibm_dual_async_device :
		public device_t,
		public device_mca16_card_interface
{
public:
	// construction/destruction
	mca16_ibm_dual_async_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_WRITE_LINE_MEMBER(pc_com_interrupt_1) { if(m_cur_irq_uart1 == 4) m_mca->ireq_w<4>(state); else m_mca->ireq_w<3>(state); }
	DECLARE_WRITE_LINE_MEMBER(pc_com_interrupt_2) { if(m_cur_irq_uart2 == 4) m_mca->ireq_w<4>(state); else m_mca->ireq_w<3>(state); }

	virtual void unmap() override;
	virtual void remap() override;

	uint8_t uart1_r(offs_t offset);
	void uart1_w(offs_t offset, uint8_t data);

	uint8_t uart2_r(offs_t offset);
	void uart2_w(offs_t offset, uint8_t data);

protected:
	mca16_ibm_dual_async_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;

	virtual uint8_t pos_r(offs_t offset) override;
	virtual void pos_w(offs_t offset, uint8_t data) override;

	required_device<ns16550_device> m_uart1;
	required_device<ns16550_device> m_uart2;

private:
	void update_serial_assignment(uint8_t pos);

	uint8_t m_serial_assignment;

	uint8_t m_cur_irq_uart1;
	uint8_t m_cur_irq_uart2;
	
	uint8_t m_is_mapped;
};

// device type definition
DECLARE_DEVICE_TYPE(MCA16_IBM_DUAL_ASYNC, mca16_ibm_dual_async_device)

#endif