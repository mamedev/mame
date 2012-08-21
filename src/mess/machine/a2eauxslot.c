/***************************************************************************

  a2eauxslot.c - Apple IIe auxiliary slot and card emulation

  by R. Belmont

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "machine/a2eauxslot.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type A2EAUXSLOT_SLOT = &device_creator<a2eauxslot_slot_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  a2eauxslot_slot_device - constructor
//-------------------------------------------------
a2eauxslot_slot_device::a2eauxslot_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
        device_t(mconfig, A2EAUXSLOT_SLOT, "Apple II Slot", tag, owner, clock),
		device_slot_interface(mconfig, *this)
{
}

a2eauxslot_slot_device::a2eauxslot_slot_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock) :
        device_t(mconfig, type, name, tag, owner, clock),
		device_slot_interface(mconfig, *this)
{
}

void a2eauxslot_slot_device::static_set_a2eauxslot_slot(device_t &device, const char *tag, const char *slottag)
{
	a2eauxslot_slot_device &a2eauxslot_card = dynamic_cast<a2eauxslot_slot_device &>(device);
	a2eauxslot_card.m_a2eauxslot_tag = tag;
	a2eauxslot_card.m_a2eauxslot_slottag = slottag;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2eauxslot_slot_device::device_start()
{
	device_a2eauxslot_card_interface *dev = dynamic_cast<device_a2eauxslot_card_interface *>(get_card_device());

	if (dev) device_a2eauxslot_card_interface::static_set_a2eauxslot_tag(*dev, m_a2eauxslot_tag, m_a2eauxslot_slottag);
}

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type A2EAUXSLOT = &device_creator<a2eauxslot_device>;

void a2eauxslot_device::static_set_cputag(device_t &device, const char *tag)
{
	a2eauxslot_device &a2eauxslot = downcast<a2eauxslot_device &>(device);
	a2eauxslot.m_cputag = tag;
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void a2eauxslot_device::device_config_complete()
{
	// inherit a copy of the static data
	const a2eauxslot_interface *intf = reinterpret_cast<const a2eauxslot_interface *>(static_config());
	if (intf != NULL)
	{
		*static_cast<a2eauxslot_interface *>(this) = *intf;
	}

	// or initialize to defaults if none provided
	else
	{
    	memset(&m_out_irq_cb, 0, sizeof(m_out_irq_cb));
    	memset(&m_out_nmi_cb, 0, sizeof(m_out_nmi_cb));
	}
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  a2eauxslot_device - constructor
//-------------------------------------------------

a2eauxslot_device::a2eauxslot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
        device_t(mconfig, A2EAUXSLOT, "Apple II Bus", tag, owner, clock)
{
}

a2eauxslot_device::a2eauxslot_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock) :
        device_t(mconfig, type, name, tag, owner, clock)
{
}
//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2eauxslot_device::device_start()
{
	m_maincpu = machine().device(m_cputag);

    // resolve callbacks
	m_out_irq_func.resolve(m_out_irq_cb, *this);
	m_out_nmi_func.resolve(m_out_nmi_cb, *this);

    // clear slot
    m_device = NULL;
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void a2eauxslot_device::device_reset()
{
}

device_a2eauxslot_card_interface *a2eauxslot_device::get_a2eauxslot_card(int slot)
{
    return m_device;
}

void a2eauxslot_device::add_a2eauxslot_card(device_a2eauxslot_card_interface *card)
{
	m_device = card;
}

void a2eauxslot_device::set_irq_line(int state)
{
    m_out_irq_func(state);
}

void a2eauxslot_device::set_nmi_line(int state)
{
    m_out_nmi_func(state);
}

// interrupt request from a2eauxslot card
WRITE_LINE_MEMBER( a2eauxslot_device::irq_w ) { m_out_irq_func(state); }
WRITE_LINE_MEMBER( a2eauxslot_device::nmi_w ) { m_out_nmi_func(state); }

//**************************************************************************
//  DEVICE CONFIG A2EAUXSLOT CARD INTERFACE
//**************************************************************************


//**************************************************************************
//  DEVICE A2EAUXSLOT CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_a2eauxslot_card_interface - constructor
//-------------------------------------------------

device_a2eauxslot_card_interface::device_a2eauxslot_card_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device),
	  m_a2eauxslot(NULL),
	  m_a2eauxslot_tag(NULL)
{
}


//-------------------------------------------------
//  ~device_a2eauxslot_card_interface - destructor
//-------------------------------------------------

device_a2eauxslot_card_interface::~device_a2eauxslot_card_interface()
{
}

void device_a2eauxslot_card_interface::static_set_a2eauxslot_tag(device_t &device, const char *tag, const char *slottag)
{
	device_a2eauxslot_card_interface &a2eauxslot_card = dynamic_cast<device_a2eauxslot_card_interface &>(device);
	a2eauxslot_card.m_a2eauxslot_tag = tag;
	a2eauxslot_card.m_a2eauxslot_slottag = slottag;
}

void device_a2eauxslot_card_interface::set_a2eauxslot_device()
{
	m_a2eauxslot = dynamic_cast<a2eauxslot_device *>(device().machine().device(m_a2eauxslot_tag));
	m_a2eauxslot->add_a2eauxslot_card(this);
}

