// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Micro-Professor Expansion Slot

    60-pin slot

    +5V   1   2  D7
    +5V   3   4  D6
    GND   5   6  D5
    GND   7   8  D4
    GND   9  10  D3
    GND  11  12  D2
    GND  13  14  D1
    GND  15  16  D0
    GND  17  18  A15
    A14  19  20  A13
    A12  21  22  A11
    A10  23  24  A9
     A8  25  26  A7
     A6  27  28  A5
     A4  29  30  A3
     A2  31  32  A1
     A0  33  34  /RESET
    GND  35  36  /RFSH
    GND  37  38  /M1
    GND  39  40  /BUSACK
    GND  41  42  /WR
    GND  43  44  /RD
    GND  45  46  /IORQ
    GND  47  48  /MREQ
    GND  49  50  /HALT
    GND  51  52  /NMI
    GND  53  54  /INT
    GND  55  56  /WAIT
    GND  57  58  /BUSREQ
    GND  59  60  SYSCLK

***************************************************************************/

#ifndef MAME_BUS_MPF1_SLOT_H
#define MAME_BUS_MPF1_SLOT_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class device_mpf1_exp_interface;

class mpf1_exp_device : public device_t, public device_single_card_slot_interface<device_mpf1_exp_interface>
{
	friend class device_mpf1_exp_interface;
public:
	// construction/destruction
	template <typename T>
	mpf1_exp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&opts, const char *dflt)
		: mpf1_exp_device(mconfig, tag, owner, clock)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	mpf1_exp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_program_space(T &&tag, int spacenum) { m_program.set_tag(std::forward<T>(tag), spacenum); }
	template <typename T> void set_io_space(T &&tag, int spacenum) { m_io.set_tag(std::forward<T>(tag), spacenum); }

	// callbacks
	auto int_handler() { return m_int_handler.bind(); }
	auto nmi_handler() { return m_nmi_handler.bind(); }
	auto wait_handler() { return m_wait_handler.bind(); }

	// called from card device
	void int_w(int state) { m_int_handler(state); }
	void nmi_w(int state) { m_nmi_handler(state); }
	void wait_w(int state) { m_wait_handler(state); }

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;

	required_address_space m_program;
	required_address_space m_io;

	device_mpf1_exp_interface *m_card;

private:
	devcb_write_line m_int_handler;
	devcb_write_line m_nmi_handler;
	devcb_write_line m_wait_handler;
};


class device_mpf1_exp_interface : public device_interface
{
public:
	// construction/destruction
	device_mpf1_exp_interface(const machine_config &mconfig, device_t &device);

protected:
	address_space &program_space() { return *m_slot->m_program; }
	address_space &io_space() { return *m_slot->m_io; }

	mpf1_exp_device *m_slot;
};

// device type definition
DECLARE_DEVICE_TYPE(MPF1_EXP, mpf1_exp_device)

// supported devices
void mpf1_exp_devices(device_slot_interface &device);
void mpf1p_exp_devices(device_slot_interface &device);
void mpf1_88_exp_devices(device_slot_interface &device);

#endif // MAME_BUS_MPF1_SLOT_H
