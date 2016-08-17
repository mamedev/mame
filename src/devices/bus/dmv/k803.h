// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
#pragma once

#ifndef __DMV_K803_H__
#define __DMV_K803_H__

#include "emu.h"
#include "dmvbus.h"
#include "machine/mm58167.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> dmv_k803_device

class dmv_k803_device :
		public device_t,
		public device_dmvslot_interface
{
public:
	// construction/destruction
	dmv_k803_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	DECLARE_WRITE_LINE_MEMBER(rtc_irq_w);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void io_read(address_space &space, int ifsel, offs_t offset, UINT8 &data) override;
	virtual void io_write(address_space &space, int ifsel, offs_t offset, UINT8 data) override;

	void update_int();

private:
	required_device<mm58167_device> m_rtc;
	required_ioport m_dsw;
	dmvcart_slot_device * m_bus;
	UINT8   m_latch;
	int     m_rtc_int;
};


// device type definition
extern const device_type DMV_K803;

#endif  /* __DMV_K803_H__ */
