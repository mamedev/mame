// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/******************************************************************************

    Psion ASIC5

******************************************************************************/

#ifndef MAME_MACHINE_PSION_ASIC5_H
#define MAME_MACHINE_PSION_ASIC5_H

#pragma once

#include "machine/ins8250.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> psion_asic5_device

class psion_asic5_device : public device_t
{
public:
	auto readpa_handler() { return m_in_a_handler.bind(); }
	auto readpb_handler() { return m_in_b_handler.bind(); }

	auto writepa_handler() { return m_out_a_handler.bind(); }
	auto writepb_handler() { return m_out_b_handler.bind(); }
	auto writepc_handler() { return m_out_c_handler.bind(); }
	auto writepd_handler() { return m_out_d_handler.bind(); }

	void set_info_byte(uint8_t info) { m_info_byte = info; }

	void data_w(uint16_t data);
	uint8_t data_r();

protected:
	// construction/destruction
	psion_asic5_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	static constexpr uint16_t NULL_FRAME    = 0x000;
	static constexpr uint16_t CONTROL_FRAME = 0x100;
	static constexpr uint16_t DATA_FRAME    = 0x200;

	devcb_read8 m_in_a_handler;
	devcb_read8 m_in_b_handler;
	devcb_write8 m_out_a_handler;
	devcb_write8 m_out_b_handler;
	devcb_write8 m_out_c_handler;
	devcb_write8 m_out_d_handler;

	// Slave control mode
	static constexpr uint8_t READ_WRITE_SELECT     = 0xc0;
	static constexpr uint8_t BYTE_WORD_TRANSFER    = 0xa0;
	static constexpr uint8_t SINGLE_MULTI_TRANSFER = 0x90;

	// registers
	uint8_t m_portb_counter;
	uint8_t m_portb_ctrl;
	int m_port_dc_writes;
	uint8_t m_interrupt_mask;

	// internal
	uint8_t m_info_byte;
	uint8_t m_sibo_control;
};


// ======================> psion_asic5_pack_device

class psion_asic5_pack_device : public psion_asic5_device
{
public:
	// construction/destruction
	psion_asic5_pack_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};


// ======================> psion_asic5_peripheral_device

class psion_asic5_peripheral_device : public psion_asic5_device
{
public:
	// construction/destruction
	psion_asic5_peripheral_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	//auto irq4() { return m_irq4_callback.bind(); }
	//auto txd1() { return m_txd1_callback.bind(); }
	//auto dtr1() { return m_dtr1_callback.bind(); }
	//auto rts1() { return m_rts1_callback.bind(); }

protected:
	virtual void device_add_mconfig(machine_config &config) override;

private:
	required_device<ns16550_device> m_serial;

	//devcb_write_line m_irq4_callback;
	//devcb_write_line m_txd1_callback;
	//devcb_write_line m_dtr1_callback;
	//devcb_write_line m_rts1_callback;
};


// device type definition
DECLARE_DEVICE_TYPE(PSION_ASIC5_PACK, psion_asic5_pack_device)
DECLARE_DEVICE_TYPE(PSION_ASIC5_PERIPHERAL, psion_asic5_peripheral_device)

#endif // MAME_MACHINE_PSION_ASIC5_H
