// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  a2bus.c - Apple II slot bus and card emulation

  by R. Belmont

  Pinout (/ indicates an inverted signal, ie, one that would have a bar over it
          on a schematic diagram)

      (rear of computer)

     GND  26  25  +5V
  DMA IN  27  24  DMA OUT
  INT IN  28  23  INT OUT
    /NMI  29  22  /DMA
    /IRQ  30  21  RDY
    /RES  31  20  /IOSTB
    /INH  32  19  N.C.
    -12V  33  18  R/W
     -5V  34  17  A15
    N.C.  35  16  A14
      7M  36  15  A13
      Q3  37  14  A12
     PH1  38  13  A11
  USER 1  39  12  A10
     PH0  40  11  A9
 /DEVSEL  41  10  A8
      D7  42   9  A7
      D6  43   8  A6
      D5  44   7  A5
      D4  45   6  A4
      D3  46   5  A3
      D2  47   4  A2
      D1  48   3  A1
      D0  49   2  A0
    -12V  50   1  /IOSEL

     (front of computer)

    Signal descriptions:
    GND - power supply ground
    DMA IN - daisy chain of DMA signal from higher priority devices.  usually connected to DMA OUT.
    INT IN - similar to DMA IN but for INT instead of DMA.
    /NMI - non-maskable interrupt input to the 6502
    /IRQ - maskable interrupt input to the 6502
    /RES - system reset signal
    /INH - On the II and II+, inhibits the motherboard ROMs, allowing a card to replace them.
           On the IIe, inhibits all motherboard RAM/ROM for both CPU and DMA accesses.
           On the IIgs, works like the IIe except for the address range 0x6000 to 0x9FFF where
           it will cause bus contention.
    -12V - negative 12 volts DC power
     -5V - negative 5 volts DC power
      7M - 7 MHz clock (1/4th of the master clock on the IIgs, 1/2 on 8-bit IIs)
      Q3 - 2 MHz asymmetrical clock
     PH1 - 6502 phase 1 clock
 /DEVSEL - asserted on an access to C0nX, where n = the slot number plus 8.
   D0-D7 - 8-bit data bus
     +5V - 5 volts DC power
 DMA OUT - see DMA IN
 INT OUT - see INT IN
    /DMA - pulling this low disconnects the 6502 from the bus and halts it
     RDY - 6502 RDY input.  Pulling this low when PH1 is active will halt the
           6502 and latch the current address bus value.
  /IOSTB - asserted on an access between C800 and CFFF.
  A0-A15 - 16-bit address bus
  /IOSEL - asserted on accesses to CnXX where n is the slot number.
           Not present on slot 0.

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "a2bus.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type A2BUS_SLOT = &device_creator<a2bus_slot_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  a2bus_slot_device - constructor
//-------------------------------------------------
a2bus_slot_device::a2bus_slot_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, A2BUS_SLOT, "Apple II Slot", tag, owner, clock, "a2bus_slot", __FILE__),
		device_slot_interface(mconfig, *this)
{
}

a2bus_slot_device::a2bus_slot_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source) :
		device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_slot_interface(mconfig, *this)
{
}

void a2bus_slot_device::static_set_a2bus_slot(device_t &device, std::string tag, std::string slottag)
{
	a2bus_slot_device &a2bus_card = dynamic_cast<a2bus_slot_device &>(device);
	a2bus_card.m_a2bus_tag = tag;
	a2bus_card.m_a2bus_slottag = slottag;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_slot_device::device_start()
{
	device_a2bus_card_interface *dev = dynamic_cast<device_a2bus_card_interface *>(get_card_device());

	if (dev) device_a2bus_card_interface::static_set_a2bus_tag(*dev, m_a2bus_tag, m_a2bus_slottag);
}

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type A2BUS = &device_creator<a2bus_device>;

void a2bus_device::static_set_cputag(device_t &device, std::string tag)
{
	a2bus_device &a2bus = downcast<a2bus_device &>(device);
	a2bus.m_cputag = tag;
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  a2bus_device - constructor
//-------------------------------------------------

a2bus_device::a2bus_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, A2BUS, "Apple II Bus", tag, owner, clock, "a2bus", __FILE__), m_maincpu(nullptr), m_maincpu_space(nullptr),
		m_out_irq_cb(*this),
		m_out_nmi_cb(*this),
		m_out_inh_cb(*this), m_slot_irq_mask(0), m_slot_nmi_mask(0)
{
}

a2bus_device::a2bus_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source) :
		device_t(mconfig, type, name, tag, owner, clock, shortname, source), m_maincpu(nullptr), m_maincpu_space(nullptr),
		m_out_irq_cb(*this),
		m_out_nmi_cb(*this),
		m_out_inh_cb(*this), m_slot_irq_mask(0), m_slot_nmi_mask(0)
{
}
//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_device::device_start()
{
	m_maincpu = machine().device<cpu_device>(m_cputag);
	m_maincpu_space = &machine().device<cpu_device>(m_cputag)->space(AS_PROGRAM);

	// resolve callbacks
	m_out_irq_cb.resolve_safe();
	m_out_nmi_cb.resolve_safe();
	m_out_inh_cb.resolve_safe();

	// clear slots
	for (auto & elem : m_device_list)
	{
		elem = nullptr;
	}

	m_slot_irq_mask = m_slot_nmi_mask = 0;
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void a2bus_device::device_reset()
{
}

device_a2bus_card_interface *a2bus_device::get_a2bus_card(int slot)
{
	if (slot < 0)
	{
		return nullptr;
	}

	if (m_device_list[slot])
	{
		return m_device_list[slot];
	}

	return nullptr;
}

void a2bus_device::add_a2bus_card(int slot, device_a2bus_card_interface *card)
{
	m_device_list[slot] = card;
}

UINT8 a2bus_device::get_a2bus_irq_mask()
{
	return m_slot_irq_mask;
}

UINT8 a2bus_device::get_a2bus_nmi_mask()
{
	return m_slot_nmi_mask;
}

void a2bus_device::set_irq_line(int state, int slot)
{
	m_out_irq_cb(state);

	if (state == CLEAR_LINE)
	{
		m_slot_irq_mask &= ~(1<<slot);
	}
	else if (state == ASSERT_LINE)
	{
		m_slot_irq_mask |= (1<<slot);
	}
}

void a2bus_device::set_nmi_line(int state, int slot)
{
	m_out_nmi_cb(state);

	if (state == CLEAR_LINE)
	{
		m_slot_nmi_mask &= ~(1<<slot);
	}
	else if (state == ASSERT_LINE)
	{
		m_slot_nmi_mask |= (1<<slot);
	}
}

void a2bus_device::set_maincpu_halt(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state);
}

UINT8 a2bus_device::dma_r(address_space &space, UINT16 offset)
{
	m_maincpu_space->set_debugger_access(space.debugger_access());

	return m_maincpu_space->read_byte(offset);
}

void a2bus_device::dma_w(address_space &space, UINT16 offset, UINT8 data)
{
	m_maincpu_space->set_debugger_access(space.debugger_access());

	m_maincpu_space->write_byte(offset, data);
}

UINT8 a2bus_device::dma_nospace_r(UINT16 offset)
{
	return m_maincpu_space->read_byte(offset);
}

void a2bus_device::dma_nospace_w(UINT16 offset, UINT8 data)
{
	m_maincpu_space->write_byte(offset, data);
}

void a2bus_device::recalc_inh(int slot)
{
	m_out_inh_cb(ASSERT_LINE);
	m_out_inh_cb(CLEAR_LINE);
}

// interrupt request from a2bus card
WRITE_LINE_MEMBER( a2bus_device::irq_w ) { m_out_irq_cb(state); }
WRITE_LINE_MEMBER( a2bus_device::nmi_w ) { m_out_nmi_cb(state); }

//**************************************************************************
//  DEVICE CONFIG A2BUS CARD INTERFACE
//**************************************************************************


//**************************************************************************
//  DEVICE A2BUS CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_a2bus_card_interface - constructor
//-------------------------------------------------

device_a2bus_card_interface::device_a2bus_card_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device),
		m_a2bus(nullptr),
		m_slot(0), m_next(nullptr)
{
}


//-------------------------------------------------
//  ~device_a2bus_card_interface - destructor
//-------------------------------------------------

device_a2bus_card_interface::~device_a2bus_card_interface()
{
}

void device_a2bus_card_interface::static_set_a2bus_tag(device_t &device, std::string tag, std::string slottag)
{
	device_a2bus_card_interface &a2bus_card = dynamic_cast<device_a2bus_card_interface &>(device);
	a2bus_card.m_a2bus_tag = tag;
	a2bus_card.m_a2bus_slottag = slottag;
}

void device_a2bus_card_interface::set_a2bus_device()
{
	// extract the slot number from the last digit of the slot tag
	int tlen = m_a2bus_slottag.length();

	m_slot = (m_a2bus_slottag[tlen-1] - '0');

	if (m_slot < 0 || m_slot > 7)
	{
		fatalerror("Slot %x out of range for Apple II Bus\n", m_slot);
	}

	m_a2bus = dynamic_cast<a2bus_device *>(device().machine().device(m_a2bus_tag));
	m_a2bus->add_a2bus_card(m_slot, this);
}
