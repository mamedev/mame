// license:BSD-3-Clause
// copyright-holders:F. Ulivi

// High level emulation of 8041 UPI in HP98X6 systems
//
// No known dump is available of this device
//
// A functional description is in this doc:
// HP-1980-Theory of operation-09826-66501 & 09826-66502 mother boards
// http://www.bitsavers.org/pdf/hp/9000_200/specs/A-09826-90315-1_9826_Mother_Board_Theory_of_Operation_Nov81.pdf

#ifndef MAME_HP_HP98X6_UPI_H
#define MAME_HP_HP98X6_UPI_H

#pragma once

#include "machine/timer.h"
#include "sound/beep.h"


class hp98x6_upi_device : public device_t
{
public:
	// construction/destruction
	hp98x6_upi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	// Set CB for IRQ1 signal
	auto irq1_write_cb() { return m_irq1_write_func.bind(); }
	// Set CB for IRQ7 signal
	auto irq7_write_cb() { return m_irq7_write_func.bind(); }

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	enum class fsm_st {
		ST_IDLE,
		ST_POR_TEST1,
		ST_POR_TEST2,
		ST_RESETTING
	};

	required_ioport_array<4> m_keys;
	required_ioport m_shift;
	required_ioport m_dial;
	required_device<beep_device> m_beep;
	required_device<timer_device> m_10ms_timer;
	required_device<timer_device> m_delay_timer;
	required_device<timer_device> m_input_delay_timer;

	// ID PROM
	optional_region_ptr<uint8_t> m_idprom;

	devcb_write_line m_irq1_write_func;
	devcb_write_line m_irq7_write_func;
	uint8_t m_ram[64];
	uint8_t m_data_in;
	uint8_t m_data_out;
	uint8_t m_status;
	bool m_ready;
	ioport_value m_last_dial;

	fsm_st m_fsm_state;

	TIMER_DEVICE_CALLBACK_MEMBER(ten_ms);
	TIMER_DEVICE_CALLBACK_MEMBER(delay);
	TIMER_DEVICE_CALLBACK_MEMBER(input_delay);

	void write_ob_st(uint8_t data, uint8_t st);
	bool read_ib(uint8_t &data);
	void update_fsm();
	void decode_cmd(uint8_t cmd);
	bool add_to_ctr(unsigned ram_idx, unsigned len, const uint8_t *op);
	void ten_ms_update_key();
	void ten_ms_update_dial();
	void ten_ms_update_timers();
	void ten_ms_update_beep();
	void set_new_key(uint8_t scancode);
	void acquire_keys(ioport_value input[4]);
	bool is_key_down(const ioport_value input[4], uint8_t idx);
	uint8_t encode_shift_ctrl(uint8_t st) const;
	void try_output();
	void try_fhs_output();
};

// device type definition
DECLARE_DEVICE_TYPE(HP98X6_UPI, hp98x6_upi_device)

#endif /* MAME_HP_HP98X6_UPI_H */
