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

#pragma once

#ifndef __A8SIO_H_
#define __A8SIO_H_


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_A8SIO_SLOT_ADD(_nbtag, _tag, _def_slot) \
	MCFG_DEVICE_ADD(_tag, A8SIO_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(a8sio_cards, _def_slot, false) \
	a8sio_slot_device::static_set_a8sio_slot(*device, _nbtag, _tag);

#define MCFG_A8SIO_DATA_IN_CB(_devcb) \
	devcb = &a8sio_device::set_data_in_callback(*device, DEVCB_##_devcb);


class a8sio_slot_device : public device_t,
							public device_slot_interface
{
public:
	// construction/destruction
	a8sio_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	a8sio_slot_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	// device-level overrides
	virtual void device_start() override;

	// inline configuration
	static void static_set_a8sio_slot(device_t &device, const char *tag, const char *slottag);

protected:
	// configuration
	const char *m_a8sio_tag;
	const char *m_a8sio_slottag;
};


// device type definition
extern const device_type A8SIO_SLOT;


class device_a8sio_card_interface;

class a8sio_device : public device_t
{
public:
	// construction/destruction
	a8sio_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	a8sio_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	// inline configuration
	template<class _Object> static devcb_base &set_clock_in_callback(device_t &device, _Object object) { return downcast<a8sio_device &>(device).m_out_clock_in_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_data_in_callback(device_t &device, _Object object) { return downcast<a8sio_device &>(device).m_out_data_in_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_audio_in_callback(device_t &device, _Object object) { return downcast<a8sio_device &>(device).m_out_audio_in_cb.set_callback(object); }

	void add_a8sio_card(device_a8sio_card_interface *card);
	device_a8sio_card_interface *get_a8sio_card();

	DECLARE_WRITE_LINE_MEMBER( clock_in_w );  // pin 1
	//virtual DECLARE_WRITE_LINE_MEMBER( clock_out_w ); // pin 2
	DECLARE_WRITE_LINE_MEMBER( data_in_w );   // pin 3
	//DECLARE_WRITE_LINE_MEMBER( data_out_wi ); // pin 5
	//DECLARE_WRITE_LINE_MEMBER( command_w );   // pin 7
	DECLARE_WRITE_LINE_MEMBER( motor_w );     // pin 8
	//DECLARE_WRITE_LINE_MEMBER( proceed_w );   // pin 9
	DECLARE_WRITE8_MEMBER( audio_in_w );      // pin 11

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	devcb_write_line    m_out_clock_in_cb; // pin 1
	devcb_write_line    m_out_data_in_cb; // pin 3
	devcb_write8        m_out_audio_in_cb; // pin 11

	device_a8sio_card_interface *m_device;
};


// device type definition
extern const device_type A8SIO;


class device_a8sio_card_interface : public device_slot_card_interface
{
	friend class a8sio_device;
public:
	// construction/destruction
	device_a8sio_card_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_a8sio_card_interface();

	void set_a8sio_device();

	// inline configuration
	static void static_set_a8sio_tag(device_t &device, const char *tag, const char *slottag);

	virtual DECLARE_WRITE_LINE_MEMBER( motor_w );

public:
	a8sio_device  *m_a8sio;
	const char *m_a8sio_tag;
	const char *m_a8sio_slottag;
};


SLOT_INTERFACE_EXTERN(a8sio_cards);

#endif
