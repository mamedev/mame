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

#include "emu.h"
#include "a8sio.h"
#include "atari810.h"
#include "atari1050.h"
#include "cassette.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(A8SIO, a8sio_device, "a8sio", "Atari 8 bit SIO Slot")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  a8sio_device - constructor
//-------------------------------------------------

a8sio_device::a8sio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, A8SIO, tag, owner, clock)
	, device_single_card_slot_interface<device_a8sio_card_interface>(mconfig, *this)
	, m_out_clock_in_cb(*this)
	, m_out_data_in_cb(*this)
	, m_out_proceed_cb(*this)
	, m_out_audio_in_cb(*this)
	, m_out_interrupt_cb(*this)
	, m_device(nullptr)
{
}

//-------------------------------------------------
//  device_resolve_objects - resolve objects that
//  may be needed for other devices to set
//  initial conditions at start time
//-------------------------------------------------

void a8sio_device::device_resolve_objects()
{
	m_device = get_card_device();
	if (m_device)
		m_device->set_a8sio_device(this);

	// resolve callbacks
	m_out_clock_in_cb.resolve_safe();
	m_out_data_in_cb.resolve_safe();
	m_out_proceed_cb.resolve_safe();
	m_out_audio_in_cb.resolve_safe();
	m_out_interrupt_cb.resolve_safe();
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a8sio_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void a8sio_device::device_reset()
{
}

device_a8sio_card_interface *a8sio_device::get_a8sio_card()
{
	return m_device;
}

WRITE_LINE_MEMBER( a8sio_device::clock_in_w )
{
	m_out_clock_in_cb(state);
}

WRITE_LINE_MEMBER( a8sio_device::clock_out_w )
{
	if (m_device)
		m_device->clock_out_w(state);
}

WRITE_LINE_MEMBER( a8sio_device::data_in_w )
{
	m_out_data_in_cb(state);
}

WRITE_LINE_MEMBER( a8sio_device::data_out_w )
{
	if (m_device)
		m_device->data_out_w(state);
}

WRITE_LINE_MEMBER( a8sio_device::command_w )
{
	if (m_device)
		m_device->command_w(state);
}

WRITE_LINE_MEMBER( a8sio_device::motor_w )
{
	if (m_device)
		m_device->motor_w(state);
}

WRITE_LINE_MEMBER( a8sio_device::proceed_w )
{
	m_out_proceed_cb(state);
}

WRITE8_MEMBER( a8sio_device::audio_in_w )
{
	m_out_audio_in_cb(data);
}

WRITE_LINE_MEMBER( a8sio_device::interrupt_w )
{
	m_out_interrupt_cb(state);
}


//**************************************************************************
//  DEVICE A8SIO CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_a8sio_card_interface - constructor
//-------------------------------------------------

device_a8sio_card_interface::device_a8sio_card_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "a8sio")
	, m_a8sio(nullptr)
{
}


//-------------------------------------------------
//  ~device_a8sio_card_interface - destructor
//-------------------------------------------------

device_a8sio_card_interface::~device_a8sio_card_interface()
{
}

void device_a8sio_card_interface::set_a8sio_device(a8sio_device *sio)
{
	m_a8sio = sio;
}

WRITE_LINE_MEMBER( device_a8sio_card_interface::clock_out_w )
{
}

WRITE_LINE_MEMBER( device_a8sio_card_interface::data_out_w )
{
}

WRITE_LINE_MEMBER( device_a8sio_card_interface::command_w )
{
}

WRITE_LINE_MEMBER( device_a8sio_card_interface::motor_w )
{
	//printf("device_a8sio_card_interface::motor_w %d\n", state);
}

WRITE_LINE_MEMBER( device_a8sio_card_interface::ready_w )
{
}


void a8sio_cards(device_slot_interface &device)
{
	device.option_add("a810", ATARI810);
	device.option_add("a1050", ATARI1050);
	device.option_add("cassette", A8SIO_CASSETTE);
}
