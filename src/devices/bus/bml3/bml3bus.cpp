// license:GPL-2.0+
// copyright-holders:Jonathan Edwards
/***************************************************************************

  bml3bus.c - Hitachi MB-6890 slot bus and card emulation

  Adapted from a2bus by Jonathan Edwards

  Pinout (/ indicates an inverted signal, ie, one that would have a bar over it
          on a schematic diagram)

       out <-> CPU CPU <-> out
       ----------  -----------
       +5V <--  1   2  <-> GND
        D0 <->  3   4  <-> D1
        D2 <->  5   6  <-> D3
        D4 <->  7   8  <-> D5
        D6 <->  9  10  <-> D7
        A0 <-> 11  12  <-> A1
        A2 <-> 13  14  <-> A3
        A4 <-> 15  16  <-> A5
        A6 <-> 17  18  <-> A7
        A8 <-> 19  20  <-> A9
       A10 <-> 21  22  <-> A11
       A12 <-> 23  24  <-> A13
       A14 <-> 25  26  <-> A15
        BA <-- 27  28  --> BS
  /ROM-KIL --> 29  30  --> EXROM-KIL
    R/W IN --> 31  32  --> /EX-I/O
   R/W OUT <-- 33  34  --> VMA OUT
         E <-- 35  36  --> Q
      /RES <-- 37  38  <-- /NMI
      /IRQ --> 39  40  <-- /FIRQ
     /HALT --> 41  42  <-- /VMA CTRL
      /DMA --> 43  44  --> /BANK-SW
  HALT ACK <-- 45  46  <-- SOUND IN
     16MCK <-- 47  48  <-> GND
      2MCK <-- 49  50  <-> GND
       -5V <-- 51  52  --> /EX-I/O2
      -12V <-- 53  54  --> +12V
       GND <-> 55  56  --> +5V

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "bml3bus.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type BML3BUS_SLOT = &device_creator<bml3bus_slot_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bml3bus_slot_device - constructor
//-------------------------------------------------
bml3bus_slot_device::bml3bus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, BML3BUS_SLOT, "Hitachi MB-6890 Slot", tag, owner, clock, "bml3bus_slot", __FILE__),
		device_slot_interface(mconfig, *this), m_bml3bus_tag(nullptr), m_bml3bus_slottag(nullptr)
{
}

bml3bus_slot_device::bml3bus_slot_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
		device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_slot_interface(mconfig, *this), m_bml3bus_tag(nullptr), m_bml3bus_slottag(nullptr)
{
}

void bml3bus_slot_device::static_set_bml3bus_slot(device_t &device, const char *tag, const char *slottag)
{
	bml3bus_slot_device &bml3bus_card = dynamic_cast<bml3bus_slot_device &>(device);
	bml3bus_card.m_bml3bus_tag = tag;
	bml3bus_card.m_bml3bus_slottag = slottag;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bml3bus_slot_device::device_start()
{
	device_bml3bus_card_interface *dev = dynamic_cast<device_bml3bus_card_interface *>(get_card_device());

	if (dev) device_bml3bus_card_interface::static_set_bml3bus_tag(*dev, m_bml3bus_tag, m_bml3bus_slottag);
}

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type BML3BUS = &device_creator<bml3bus_device>;

void bml3bus_device::static_set_cputag(device_t &device, const char *tag)
{
	bml3bus_device &bml3bus = downcast<bml3bus_device &>(device);
	bml3bus.m_cputag = tag;
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bml3bus_device - constructor
//-------------------------------------------------

bml3bus_device::bml3bus_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, BML3BUS, "Hitachi MB-6890 Bus", tag, owner, clock, "bml3bus", __FILE__), m_maincpu(nullptr),
		m_out_nmi_cb(*this),
		m_out_irq_cb(*this),
		m_out_firq_cb(*this), m_cputag(nullptr)
{
}

bml3bus_device::bml3bus_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
		device_t(mconfig, type, name, tag, owner, clock, shortname, source), m_maincpu(nullptr),
		m_out_nmi_cb(*this),
		m_out_irq_cb(*this),
		m_out_firq_cb(*this), m_cputag(nullptr)
{
}
//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bml3bus_device::device_start()
{
	m_maincpu = machine().device<cpu_device>(m_cputag);

	// resolve callbacks
	m_out_nmi_cb.resolve_safe();
	m_out_irq_cb.resolve_safe();
	m_out_firq_cb.resolve_safe();

	// clear slots
	for (auto & elem : m_device_list)
	{
		elem = nullptr;
	}
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bml3bus_device::device_reset()
{
}

device_bml3bus_card_interface *bml3bus_device::get_bml3bus_card(int slot)
{
	if (slot < 0)
	{
		return nullptr;
	}

	return m_device_list[slot];
}

void bml3bus_device::add_bml3bus_card(int slot, device_bml3bus_card_interface *card)
{
	m_device_list[slot] = card;
}

void bml3bus_device::set_nmi_line(int state)
{
	m_out_nmi_cb(state);
}

void bml3bus_device::set_irq_line(int state)
{
	m_out_irq_cb(state);
}

void bml3bus_device::set_firq_line(int state)
{
	m_out_firq_cb(state);
}

// interrupt request from bml3bus card
WRITE_LINE_MEMBER( bml3bus_device::nmi_w ) { m_out_nmi_cb(state); }
WRITE_LINE_MEMBER( bml3bus_device::irq_w ) { m_out_irq_cb(state); }
WRITE_LINE_MEMBER( bml3bus_device::firq_w ) { m_out_firq_cb(state); }

//**************************************************************************
//  DEVICE CONFIG BML3BUS CARD INTERFACE
//**************************************************************************


//**************************************************************************
//  DEVICE BML3BUS CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_bml3bus_card_interface - constructor
//-------------------------------------------------

device_bml3bus_card_interface::device_bml3bus_card_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device),
		m_bml3bus(nullptr),
		m_bml3bus_tag(nullptr), m_bml3bus_slottag(nullptr), m_slot(0), m_next(nullptr)
{
}


//-------------------------------------------------
//  ~device_bml3bus_card_interface - destructor
//-------------------------------------------------

device_bml3bus_card_interface::~device_bml3bus_card_interface()
{
}

void device_bml3bus_card_interface::static_set_bml3bus_tag(device_t &device, const char *tag, const char *slottag)
{
	device_bml3bus_card_interface &bml3bus_card = dynamic_cast<device_bml3bus_card_interface &>(device);
	bml3bus_card.m_bml3bus_tag = tag;
	bml3bus_card.m_bml3bus_slottag = slottag;
}

void device_bml3bus_card_interface::set_bml3bus_device()
{
	// extract the slot number from the last digit of the slot tag
	int tlen = strlen(m_bml3bus_slottag);

	m_slot = (m_bml3bus_slottag[tlen-1] - '1');

	if (m_slot < 0 || m_slot >= BML3BUS_MAX_SLOTS)
	{
		fatalerror("Slot %x out of range for Hitachi MB-6890 Bus\n", m_slot);
	}

	m_bml3bus = dynamic_cast<bml3bus_device *>(device().machine().device(m_bml3bus_tag));
	m_bml3bus->add_bml3bus_card(m_slot, this);
}
