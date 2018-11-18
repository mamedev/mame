// license:BSD-3-Clause
// copyright-holders:F. Ulivi
/*********************************************************************

    1mb5.h

    HP-8x I/O Translator chip (1MB5-0101)

*********************************************************************/

#ifndef MAME_MACHINE_1MB5_H
#define MAME_MACHINE_1MB5_H

#pragma once

class hp_1mb5_device : public device_t
{
public:
	// construction/destruction
	hp_1mb5_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// static configuration helpers
	auto irl_handler() { return m_irl_handler.bind(); }
	auto halt_handler() { return m_halt_handler.bind(); }
	auto reset_handler() { return m_reset_handler.bind(); }
	auto int_handler() { return m_int_handler.bind(); }

	// CPU access
	DECLARE_READ8_MEMBER(cpu_r);
	DECLARE_WRITE8_MEMBER(cpu_w);

	// uC access
	DECLARE_READ8_MEMBER(uc_r);
	DECLARE_WRITE8_MEMBER(uc_w);

	// Signals to CPU
	DECLARE_READ_LINE_MEMBER(irl_r);
	DECLARE_READ_LINE_MEMBER(halt_r);

	// Signals to uC
	DECLARE_READ_LINE_MEMBER(reset_r);
	DECLARE_READ_LINE_MEMBER(int_r);

	// Interrupt enable
	void inten();

	// Interrupt clearing
	void clear_service();

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	devcb_write_line m_irl_handler;
	devcb_write_line m_halt_handler;
	devcb_write_line m_reset_handler;
	devcb_write_line m_int_handler;

	// Registers
	uint8_t m_sr;
	uint8_t m_cr;
	uint8_t m_ib;
	uint8_t m_ob;
	bool m_ibf;
	bool m_obf;
	bool m_hlten;
	bool m_service;
	bool m_cint;
	bool m_reset;
	bool m_halt;

	bool set_service(bool new_service);
	bool update_halt();
	bool set_reset(bool new_reset);
	bool set_int(bool new_int);
};

// device type definition
DECLARE_DEVICE_TYPE(HP_1MB5, hp_1mb5_device)

#endif /* MAME_MACHINE_1MB5_H */
