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
#include "cassette.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type A8SIO_SLOT = &device_creator<a8sio_slot_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  a8sio_slot_device - constructor
//-------------------------------------------------
a8sio_slot_device::a8sio_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, A8SIO_SLOT, "Atari 8 bit SIO Slot", tag, owner, clock, "a8sio_slot", __FILE__)
	, device_slot_interface(mconfig, *this), m_a8sio_tag(nullptr), m_a8sio_slottag(nullptr)
{
}

a8sio_slot_device::a8sio_slot_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
		device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_slot_interface(mconfig, *this), m_a8sio_tag(nullptr), m_a8sio_slottag(nullptr)
{
}

void a8sio_slot_device::static_set_a8sio_slot(device_t &device, const char *tag, const char *slottag)
{
	a8sio_slot_device &a8sio_ext = dynamic_cast<a8sio_slot_device &>(device);
	a8sio_ext.m_a8sio_tag = tag;
	a8sio_ext.m_a8sio_slottag = slottag;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a8sio_slot_device::device_start()
{
	device_a8sio_card_interface *dev = dynamic_cast<device_a8sio_card_interface *>(get_card_device());

	if (dev)
	{
		device_a8sio_card_interface::static_set_a8sio_tag(*dev, m_a8sio_tag, m_a8sio_slottag);
	}
}



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type A8SIO = &device_creator<a8sio_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  a8sio_device - constructor
//-------------------------------------------------

a8sio_device::a8sio_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, A8SIO, "Atari 8 biot SIO", tag, owner, clock, "a8sio", __FILE__)
	, m_out_clock_in_cb(*this)
	, m_out_data_in_cb(*this)
	, m_out_audio_in_cb(*this), m_device(nullptr)
{
}

a8sio_device::a8sio_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source)
	, m_out_clock_in_cb(*this)
	, m_out_data_in_cb(*this)
	, m_out_audio_in_cb(*this), m_device(nullptr)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a8sio_device::device_start()
{
	// resolve callbacks
	m_out_clock_in_cb.resolve_safe();
	m_out_data_in_cb.resolve_safe();
	m_out_audio_in_cb.resolve_safe();

	// clear slot
	m_device = NULL;
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

void a8sio_device::add_a8sio_card(device_a8sio_card_interface *card)
{
	m_device = card;
}

WRITE_LINE_MEMBER( a8sio_device::clock_in_w )
{
	m_out_clock_in_cb(state);
}

WRITE_LINE_MEMBER( a8sio_device::data_in_w )
{
	m_out_data_in_cb(state);
}

WRITE_LINE_MEMBER( a8sio_device::motor_w )
{
	if (m_device)
	{
		m_device->motor_w(state);
	}
}

WRITE8_MEMBER( a8sio_device::audio_in_w )
{
	m_out_audio_in_cb(data);
}


//**************************************************************************
//  DEVICE A8SIO CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_a8sio_card_interface - constructor
//-------------------------------------------------

device_a8sio_card_interface::device_a8sio_card_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
	, m_a8sio(NULL)
	, m_a8sio_tag(NULL), m_a8sio_slottag(nullptr)
{
}


//-------------------------------------------------
//  ~device_a8sio_card_interface - destructor
//-------------------------------------------------

device_a8sio_card_interface::~device_a8sio_card_interface()
{
}

void device_a8sio_card_interface::static_set_a8sio_tag(device_t &device, const char *tag, const char *slottag)
{
	device_a8sio_card_interface &a8sio_card = dynamic_cast<device_a8sio_card_interface &>(device);
	a8sio_card.m_a8sio_tag = tag;
	a8sio_card.m_a8sio_slottag = slottag;
}

void device_a8sio_card_interface::set_a8sio_device()
{
	m_a8sio = dynamic_cast<a8sio_device *>(device().machine().device(m_a8sio_tag));
	m_a8sio->add_a8sio_card(this);
}

WRITE_LINE_MEMBER( device_a8sio_card_interface::motor_w )
{
	//printf("device_a8sio_card_interface::motor_w %d\n", state);
}


SLOT_INTERFACE_START(a8sio_cards)
	SLOT_INTERFACE("cassette", A8SIO_CASSETTE)
SLOT_INTERFACE_END
