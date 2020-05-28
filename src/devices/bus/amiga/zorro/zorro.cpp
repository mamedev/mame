// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Amiga Zorro Slots

    86-pin Expansion Slot (Zorro-I), Zorro-II, Zorro-III

***************************************************************************/

#include "emu.h"
#include "zorro.h"


//**************************************************************************
//  ZORRO SLOT DEVICE
//**************************************************************************

DEFINE_DEVICE_TYPE(ZORRO_SLOT, zorro_slot_device, "zorro_slot", "Zorro slot")

//-------------------------------------------------
//  zorro_slot_device - constructor
//-------------------------------------------------

zorro_slot_device::zorro_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	zorro_slot_device(mconfig, ZORRO_SLOT, tag, owner, clock)
{
}

zorro_slot_device::zorro_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_zorro_bus(*this, finder_base::DUMMY_TAG)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void zorro_slot_device::device_start()
{
	device_t *const card(get_card_device());
	device_zorro_card_interface *const dev(dynamic_cast<device_zorro_card_interface *>(card));

	if (dev)
	{
		m_zorro_bus->add_card(*dev);
	}
	else if (card)
	{
		throw emu_fatalerror(
				"zorro slot '%s' card device %s (%s) does not implement device_zorro_card_interface\n",
				tag(), card->tag(), card->name());
	}
}


//**************************************************************************
//  BASE ZORRO BUS DEVICE
//**************************************************************************

//-------------------------------------------------
//  exp_slot_device - constructor
//-------------------------------------------------

zorro_bus_device_base::zorro_bus_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_space(*this, finder_base::DUMMY_TAG, -1),
	m_ovr_handler(*this),
	m_int2_handler(*this),
	m_int6_handler(*this)
{
}

//-------------------------------------------------
//  device_resolve_objects - resolve objects that
//  may be needed for other devices to set
//  initial conditions at start time
//-------------------------------------------------

void zorro_bus_device_base::device_resolve_objects()
{
	// resolve callbacks
	m_ovr_handler.resolve_safe();
	m_int2_handler.resolve_safe();
	m_int6_handler.resolve_safe();
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void zorro_bus_device_base::device_start()
{
}

// from slot device
WRITE_LINE_MEMBER( zorro_bus_device_base::ovr_w ) { m_ovr_handler(state); }
WRITE_LINE_MEMBER( zorro_bus_device_base::int2_w ) { m_int2_handler(state); }
WRITE_LINE_MEMBER( zorro_bus_device_base::int6_w ) { m_int6_handler(state); }


//**************************************************************************
//  EXPANSION SLOT DEVICE
//**************************************************************************

DEFINE_DEVICE_TYPE(EXP_SLOT, exp_slot_device, "exp_slot", "86-pin expansion slot")

//-------------------------------------------------
//  exp_slot_device - constructor
//-------------------------------------------------

exp_slot_device::exp_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	exp_slot_device(mconfig, EXP_SLOT, tag, owner, clock)
{
}

exp_slot_device::exp_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	zorro_bus_device_base(mconfig, type, tag, owner, clock),
	m_ipl_handler(*this),
	m_dev(nullptr)
{
}

//-------------------------------------------------
//  device_resolve_objects - resolve objects that
//  may be needed for other devices to set
//  initial conditions at start time
//-------------------------------------------------

void exp_slot_device::device_resolve_objects()
{
	// call base device
	zorro_bus_device_base::device_resolve_objects();

	// resolve callbacks
	m_ipl_handler.resolve_safe();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void exp_slot_device::device_reset()
{
	// call base device
	zorro_bus_device_base::device_reset();

	// if we have a device, start the autoconfig chain
	if (m_dev)
		m_dev->cfgin_w(0);
}

//-------------------------------------------------
//  add_card - add new card to our bus
//-------------------------------------------------

void exp_slot_device::add_card(device_zorro_card_interface &card)
{
	m_dev = dynamic_cast<device_exp_card_interface *>(&card);
	if (!m_dev)
	{
		throw emu_fatalerror(
				"exp slot '%s' card device %s (%s) does not implement device_exp_card_interface\n",
				tag(), card.device().tag(), card.device().name());
	}
	card.set_zorro_bus(*this);
}

// from slot device
void exp_slot_device::ipl_w(int interrupt) { m_ipl_handler(0, interrupt, 0xff); }

// from host
void exp_slot_device::fc_w(int code) { if (m_dev) m_dev->fc_w(code); }


//**************************************************************************
//  ZORRO2 DEVICE
//**************************************************************************

DEFINE_DEVICE_TYPE(ZORRO2, zorro2_bus_device, "zorro2", "Zorro-II bus")

//-------------------------------------------------
//  zorro2_bus_device - constructor
//-------------------------------------------------

zorro2_bus_device::zorro2_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	zorro2_bus_device(mconfig, ZORRO2, tag, owner, clock)
{
}

zorro2_bus_device::zorro2_bus_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	zorro_bus_device_base(mconfig, type, tag, owner, clock),
	m_eint1_handler(*this),
	m_eint4_handler(*this),
	m_eint5_handler(*this),
	m_eint7_handler(*this),
	m_autoconfig_device(nullptr)
{
}

//-------------------------------------------------
//  zorro2_bus_device - destructor
//-------------------------------------------------

zorro2_bus_device::~zorro2_bus_device()
{
	m_dev.detach_all();
}

//-------------------------------------------------
//  device_resolve_objects - resolve objects that
//  may be needed for other devices to set
//  initial conditions at start time
//-------------------------------------------------

void zorro2_bus_device::device_resolve_objects()
{
	// call base device
	zorro_bus_device_base::device_resolve_objects();

	// resolve callbacks
	m_eint1_handler.resolve_safe();
	m_eint4_handler.resolve_safe();
	m_eint5_handler.resolve_safe();
	m_eint7_handler.resolve_safe();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void zorro2_bus_device::device_reset()
{
	// call base device
	zorro_bus_device_base::device_reset();

	// initiate autoconfig
	m_autoconfig_device = m_dev.first();

	// if we have a device, start the autoconfig chain
	if (m_autoconfig_device)
		m_autoconfig_device->cfgin_w(0);
}

//-------------------------------------------------
//  add_card - add new card to our bus
//-------------------------------------------------

void zorro2_bus_device::add_card(device_zorro_card_interface &card)
{
	device_zorro2_card_interface *const zorro2_card(dynamic_cast<device_zorro2_card_interface *>(&card));
	if (!zorro2_card)
	{
		throw emu_fatalerror(
				"zorro2 slot '%s' card device %s (%s) does not implement device_zorro2_card_interface\n",
				tag(), card.device().tag(), card.device().name());
	}
	card.set_zorro_bus(*this);
	m_dev.append(*zorro2_card);
}

// from slot device
WRITE_LINE_MEMBER( zorro2_bus_device::eint1_w ) { m_eint1_handler(state); }
WRITE_LINE_MEMBER( zorro2_bus_device::eint4_w ) { m_eint4_handler(state); }
WRITE_LINE_MEMBER( zorro2_bus_device::eint5_w ) { m_eint5_handler(state); }
WRITE_LINE_MEMBER( zorro2_bus_device::eint7_w ) { m_eint7_handler(state); }

WRITE_LINE_MEMBER( zorro2_bus_device::cfgout_w )
{
	m_autoconfig_device = m_autoconfig_device->next();

	// if there is still a device in the chain, tell it to configure itself
	if (m_autoconfig_device)
		m_autoconfig_device->cfgin_w(0);
}

// from host
void zorro2_bus_device::fc_w(int code)
{
	device_zorro2_card_interface *entry = m_dev.first();

	while (entry)
	{
		entry->fc_w(code);
		entry = entry->next();
	}
}


//**************************************************************************
//  ZORRO INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_zorro_card_interface - constructor
//-------------------------------------------------

device_zorro_card_interface::device_zorro_card_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "zorro"),
	m_zorro(nullptr)
{
}

//-------------------------------------------------
//  ~device_zorro_card_interface - destructor
//-------------------------------------------------

device_zorro_card_interface::~device_zorro_card_interface()
{
}

void device_zorro_card_interface::set_zorro_bus(zorro_bus_device_base &device)
{
	assert(!m_zorro);
	m_zorro = &device;
}

void device_zorro_card_interface::fc_w(int code)
{
}

WRITE_LINE_MEMBER( device_zorro_card_interface::cfgin_w )
{
}


//**************************************************************************
//  EXPANSION CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_exp_card_interface - constructor
//-------------------------------------------------

device_exp_card_interface::device_exp_card_interface(const machine_config &mconfig, device_t &device) :
	device_zorro_card_interface(mconfig, device),
	m_slot(nullptr)
{
}

//-------------------------------------------------
//  ~device_exp_card_interface - destructor
//-------------------------------------------------

device_exp_card_interface::~device_exp_card_interface()
{
}

void device_exp_card_interface::interface_pre_start()
{
	device_zorro_card_interface::interface_pre_start();

	if (!m_zorro)
		throw device_missing_dependencies();

	m_slot = dynamic_cast<exp_slot_device *>(m_zorro);
	if (!m_slot)
	{
		throw emu_fatalerror(
				"exp card '%s' zorro device %s (%s) does not derive from exp_slot_device\n",
				device().tag(), m_zorro->tag(), m_zorro->name());
	}
}


//**************************************************************************
//  ZORRO-II CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_zorro2_interface - constructor
//-------------------------------------------------

device_zorro2_card_interface::device_zorro2_card_interface(const machine_config &mconfig, device_t &device) :
	device_zorro_card_interface(mconfig, device),
	m_next(nullptr),
	m_slot(nullptr)
{
}

//-------------------------------------------------
//  ~device_zorro2_interface - destructor
//-------------------------------------------------

device_zorro2_card_interface::~device_zorro2_card_interface()
{
}

void device_zorro2_card_interface::interface_pre_start()
{
	device_zorro_card_interface::interface_pre_start();

	if (!m_zorro)
		throw device_missing_dependencies();

	m_slot = dynamic_cast<zorro2_bus_device *>(m_zorro);
	if (!m_slot)
	{
		throw emu_fatalerror(
				"zorro2 card '%s' zorro device %s (%s) does not derive from zorro2_bus_device\n",
				device().tag(), m_zorro->tag(), m_zorro->name());
	}
}
