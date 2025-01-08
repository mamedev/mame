// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/******************************************************************************

    Psion ASIC5

******************************************************************************/

#ifndef MAME_MACHINE_PSION_ASIC5_H
#define MAME_MACHINE_PSION_ASIC5_H

#pragma once

#include "diserial.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> psion_asic5_device

class psion_asic5_device : public device_t, public device_serial_interface
{
public:
	enum pc6_state
	{
		PACK_MODE = 0,
		PERIPHERAL_MODE = 1
	};

	psion_asic5_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void set_mode(pc6_state mode) { m_mode = mode; }

	auto readpa_handler() { return m_in_a_handler.bind(); }
	auto readpb_handler() { return m_in_b_handler.bind(); }
	auto readpc_handler() { return m_in_c_handler.bind(); }

	auto writepa_handler() { return m_out_a_handler.bind(); }
	auto writepb_handler() { return m_out_b_handler.bind(); }
	auto writepc_handler() { return m_out_c_handler.bind(); }
	auto writepd_handler() { return m_out_d_handler.bind(); }
	auto writecs_handler() { return m_out_cs_handler.bind(); }

	// UART handlers
	auto int_handler() { return m_int_handler.bind(); }
	auto txd_handler() { return m_txd_handler.bind(); }
	auto rts_handler() { return m_rts_handler.bind(); }
	auto dtr_handler() { return m_dtr_handler.bind(); }

	void write_rxd(int state);
	void write_cts(int state);
	void write_dsr(int state);
	void write_dcd(int state);

	void set_info_byte(uint8_t info) { m_info_byte = info; }

	void data_w(uint16_t data);
	uint8_t data_r();

protected:
	// device_t overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_serial overrides
	virtual void rcv_callback() override;
	virtual void rcv_complete() override;
	virtual void tra_callback() override;
	virtual void tra_complete() override;

private:
	static constexpr uint16_t NULL_FRAME    = 0x000;
	static constexpr uint16_t CONTROL_FRAME = 0x100;
	static constexpr uint16_t DATA_FRAME    = 0x200;

	void update_interrupts();

	uint8_t m_mode;

	devcb_read8 m_in_a_handler;
	devcb_read8 m_in_b_handler;
	devcb_read8 m_in_c_handler;
	devcb_write8 m_out_a_handler;
	devcb_write8 m_out_b_handler;
	devcb_write8 m_out_c_handler;
	devcb_write8 m_out_d_handler;
	devcb_write8 m_out_cs_handler;

	devcb_write_line m_int_handler;
	devcb_write_line m_txd_handler;
	devcb_write_line m_rts_handler;
	devcb_write_line m_dtr_handler;

	int m_rxd;
	int m_cts;
	int m_dsr;
	int m_dcd;

	uint8_t m_port_b_counter;
	uint8_t m_port_b_latch;
	uint8_t m_port_b_mode;
	bool m_port_dc_select;
	uint8_t m_int_mask;
	uint8_t m_int_status;
	uint8_t m_control;
	int16_t m_bdr;

	uint8_t m_info_byte;
	uint8_t m_sibo_control;
};


// device type definition
DECLARE_DEVICE_TYPE(PSION_ASIC5, psion_asic5_device)

#endif // MAME_MACHINE_PSION_ASIC5_H
