// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Compucolor Floppy Disk Drive emulation

*********************************************************************/

#ifndef MAME_BUS_COMPUCOLOR_FLOPPY_H
#define MAME_BUS_COMPUCOLOR_FLOPPY_H

#pragma once

#include "bus/rs232/rs232.h"
#include "imagedev/floppy.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> device_compucolor_floppy_port_interface

class device_compucolor_floppy_port_interface : public device_rs232_port_interface
{
public:
	virtual void rw_w(int state) = 0;
	virtual void stepper_w(uint8_t data) = 0;
	virtual void select_w(int state) = 0;

protected:
	device_compucolor_floppy_port_interface(const machine_config &mconfig, device_t &device);
};


// ======================> compucolor_floppy_port_device

class compucolor_floppy_port_device : public rs232_port_device
{
public:
	template <typename T>
	compucolor_floppy_port_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: rs232_port_device(mconfig, tag, owner, 0)
	{
		set_options(std::forward<T>(opts), dflt, false);
	}
	compucolor_floppy_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void rw_w(int state) { if (m_dev) m_dev->rw_w(state); }
	void stepper_w(uint8_t data) { if (m_dev) m_dev->stepper_w(data); }
	void select_w(int state) { if (m_dev) m_dev->select_w(state); }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_config_complete() override;

private:
	device_compucolor_floppy_port_interface *m_dev;
};


// ======================> compucolor_floppy_device

class compucolor_floppy_device : public device_t, public device_compucolor_floppy_port_interface
{
public:
	// construction/destruction
	compucolor_floppy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	compucolor_floppy_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, bool four_phase);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_rs232_port_interface overrides
	virtual void input_txd(int state) override;

	// device_compucolor_floppy_port_interface overrides
	virtual void rw_w(int state) override;
	virtual void stepper_w(uint8_t data) override;
	virtual void select_w(int state) override;

private:
	static void floppy_formats(format_registration &fr);

	required_device<floppy_connector> m_floppy;
	bool const m_four_phase;

	TIMER_CALLBACK_MEMBER(rxd_tick);
	TIMER_CALLBACK_MEMBER(motor_off);
	void read_sync();
	void update_gates();

	int m_rw;
	uint8_t m_stp;
	uint8_t m_step;
	int m_sel;
	bool m_txd;
	bool m_rxd;
	bool m_writing;
	attotime m_last_edge;
	attotime m_pos;

	attotime m_period;

	compucolor_floppy_port_device *m_owner;

	emu_timer *m_timer;
	emu_timer *m_motor_timer;
};


// ======================> compucolor_floppy_4ph_device

class compucolor_floppy_4ph_device : public compucolor_floppy_device
{
public:
	// construction/destruction
	compucolor_floppy_4ph_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// device type declaration
DECLARE_DEVICE_TYPE(COMPUCOLOR_FLOPPY_PORT, compucolor_floppy_port_device)
DECLARE_DEVICE_TYPE(COMPUCOLOR_FLOPPY,      compucolor_floppy_device)
DECLARE_DEVICE_TYPE(COMPUCOLOR_FLOPPY_4PH,  compucolor_floppy_4ph_device)


// slot devices
void compucolor_floppy_port_devices(device_slot_interface &device);

#endif // MAME_BUS_COMPUCOLOR_FLOPPY_H
