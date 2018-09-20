// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Einstein "Tatung Pipe"

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

#ifndef MAME_BUS_EINSTEIN_PIPE_PIPE_H
#define MAME_BUS_EINSTEIN_PIPE_PIPE_H

#pragma once


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_TATUNG_PIPE_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, TATUNG_PIPE, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(tatung_pipe_cards, nullptr, false)

#define MCFG_TATUNG_PIPE_INT_HANDLER(_devcb) \
	downcast<tatung_pipe_device &>(*device).set_int_handler(DEVCB_##_devcb);

#define MCFG_TATUNG_PIPE_NMI_HANDLER(_devcb) \
	downcast<tatung_pipe_device &>(*device).set_nmi_handler(DEVCB_##_devcb);

#define MCFG_TATUNG_PIPE_RESET_HANDLER(_devcb) \
	downcast<tatung_pipe_device &>(*device).set_reset_handler(DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class device_tatung_pipe_interface;

class tatung_pipe_device : public device_t, public device_slot_interface
{
	friend class device_tatung_pipe_interface;
public:
	// construction/destruction
	tatung_pipe_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~tatung_pipe_device();

	void set_program_space(address_space *program);
	void set_io_space(address_space *io);

	// callbacks
	template <class Object> devcb_base &set_int_handler(Object &&cb) { return m_int_handler.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_nmi_handler(Object &&cb) { return m_nmi_handler.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_reset_handler(Object &&cb) { return m_reset_handler.set_callback(std::forward<Object>(cb)); }

	// called from host
	DECLARE_WRITE_LINE_MEMBER( host_int_w );

	// called from card device
	DECLARE_WRITE_LINE_MEMBER( int_w ) { m_int_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( nmi_w ) { m_nmi_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( reset_w ) { m_reset_handler(state); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	address_space *m_program;
	address_space *m_io;

	device_tatung_pipe_interface *m_card;

private:
	devcb_write_line m_int_handler;
	devcb_write_line m_nmi_handler;
	devcb_write_line m_reset_handler;
};

// class representing interface-specific live pipe device
class device_tatung_pipe_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_tatung_pipe_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_tatung_pipe_interface();

	// host to card
	virtual void int_w(int state) { }

protected:
	address_space &program_space() { return *m_slot->m_program; }
	address_space &io_space() { return *m_slot->m_io; }

	tatung_pipe_device *m_slot;
};

// device type definition
DECLARE_DEVICE_TYPE(TATUNG_PIPE, tatung_pipe_device)

// supported devices
void tatung_pipe_cards(device_slot_interface &device);

#endif // MAME_BUS_EINSTEIN_PIPE_PIPE_H
