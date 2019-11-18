// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

  a8sio.h - Atari 8 bit SIO bus interface


              1 1
      2 4 6 8 0 2
     +-----------+
    / o o o o o o \
   / o o o o o o o \
  +-----------------+
     1 3 5 7 9 1 1
               1 3

  1 - clock in (to computer)
  2 - clock out
  3 - data in
  4 - GND
  5 - data out
  6 - GND
  7 - command (active low)
  8 - motor
  9 - proceed (active low)
 10 - +5V/ready
 11 - audio in
 12 - +12V (A400/A800)
 13 - interrupt (active low)

***************************************************************************/

#ifndef MAME_BUS_A800_A8SIO_H
#define MAME_BUS_A800_A8SIO_H

#pragma once

void a8sio_cards(device_slot_interface &device);

class device_a8sio_card_interface;

class a8sio_device : public device_t, public device_single_card_slot_interface<device_a8sio_card_interface>
{
public:
	// construction/destruction
	a8sio_device(machine_config const &mconfig, char const *tag, device_t *owner, char const *dflt)
		: a8sio_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		a8sio_cards(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	a8sio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// inline configuration
	auto clock_in() { return m_out_clock_in_cb.bind(); }
	auto data_in() { return m_out_data_in_cb.bind(); }
	auto audio_in() { return m_out_audio_in_cb.bind(); }
	auto proceed() { return m_out_proceed_cb.bind(); }
	auto interrupt() { return m_out_interrupt_cb.bind(); }

	device_a8sio_card_interface *get_a8sio_card();

	DECLARE_WRITE_LINE_MEMBER( clock_in_w );  // pin 1
	DECLARE_WRITE_LINE_MEMBER( clock_out_w ); // pin 2
	DECLARE_WRITE_LINE_MEMBER( data_in_w );   // pin 3
	DECLARE_WRITE_LINE_MEMBER( data_out_w );  // pin 5
	DECLARE_WRITE_LINE_MEMBER( command_w );   // pin 7
	DECLARE_WRITE_LINE_MEMBER( motor_w );     // pin 8
	DECLARE_WRITE_LINE_MEMBER( proceed_w );   // pin 9
	DECLARE_WRITE8_MEMBER( audio_in_w );      // pin 11
	DECLARE_WRITE_LINE_MEMBER( interrupt_w ); // pin 13

protected:
	a8sio_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_resolve_objects() override;
	virtual void device_start() override;
	virtual void device_reset() override;

	devcb_write_line    m_out_clock_in_cb; // pin 1
	devcb_write_line    m_out_data_in_cb; // pin 3
	devcb_write_line    m_out_proceed_cb; // pin 9
	devcb_write8        m_out_audio_in_cb; // pin 11
	devcb_write_line    m_out_interrupt_cb; // pin 13

	device_a8sio_card_interface *m_device;
};

// device type definition
DECLARE_DEVICE_TYPE(A8SIO, a8sio_device)


class device_a8sio_card_interface : public device_interface
{
	friend class a8sio_device;
public:
	// construction/destruction
	virtual ~device_a8sio_card_interface();

	void set_a8sio_device(a8sio_device *sio);

	virtual DECLARE_WRITE_LINE_MEMBER( clock_out_w );
	virtual DECLARE_WRITE_LINE_MEMBER( data_out_w );
	virtual DECLARE_WRITE_LINE_MEMBER( command_w );
	virtual DECLARE_WRITE_LINE_MEMBER( motor_w );
	virtual DECLARE_WRITE_LINE_MEMBER( ready_w );

public:
	device_a8sio_card_interface(const machine_config &mconfig, device_t &device);

	a8sio_device  *m_a8sio;
};

#endif // MAME_BUS_A800_A8SIO_H
