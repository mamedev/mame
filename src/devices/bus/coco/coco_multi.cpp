// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    coco_multi.cpp

    Code for emulating CoCo's Multi-Pak Interface

     The Multi-Pak interface multiplexes all I/O lines from the Color
     Computer's expansion port to four identical ports. All I/O lines
     are continuously multiplexed except:

     Pin 36 - *SCS
     Pin 32 - *CTS
     Pin  8 - *CART

     These I/O lines are switched in one of two ways. First, is the front
     panel, four position switch. When adjusted the switch will direct the
     MPI to target these three I/O lines to the selected slot.

     Second, the MPI will listen to writes to $FF7F and respond accordingly:

     bit 0 --\___ Target *SCS to this slot number
     bit 1 --/
     bit 2 ------ Ignore
     bit 3 ------ Ignore
     bit 4 --\___ Target *CTS and *CART to this slot number
     bit 5 --/
     bit 6 ------ Ignore
     bit 7 ------ Ignore

     After writing to $FF7F, the position of the physical switch has no
     effect until reset.

     Reading is supported on $FF7F. It will reflect the position of the
     physical switch. Until data is written to $FF7F, then it will only
     reflect what has been written until a reset.

     A common modification users of the OS-9 operating system made was to
     tie all of the *CART pins together on the MPI motherboard. The *CART
     line is connected to the 6809's IRQ line. This allowed any hardware
     device in any slot to signal an IRQ to the CPU, no matter what the
     switch position was. OS-9 was designed from the very start to poll
     each device attached on every IRQ signal.

     Because of sloppy address decoding the original MPI also responds to
     $FF9F. No software is known to take advantage of this. After the
     introduction of the CoCo 3, which uses $FF9F internally, Tandy provided
     free upgrades to any MPI to fix this problem.  This behavior is not
     emulated (yet).

     Slots seem to be one-counted (i.e. - 1-4, not 0-3) by convention

***************************************************************************/

#include "emu.h"
#include "coco_multi.h"

#include "cococart.h"

#define SLOT1_TAG           "slot1"
#define SLOT2_TAG           "slot2"
#define SLOT3_TAG           "slot3"
#define SLOT4_TAG           "slot4"

#define SWITCH_CONFIG_TAG   "switch"

#define LOG_CART   (1U << 1) // shows cart line changes
#define LOG_SWITCH (1U << 2) // shows switch changes
//#define VERBOSE (LOG_CART|LOG_SWITCH)

#include "logmacro.h"

#define LOGCART(...)     LOGMASKED(LOG_CART,  __VA_ARGS__)
#define LOGSWITCH(...)   LOGMASKED(LOG_SWITCH,  __VA_ARGS__)

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

static constexpr u8 MULTI_SLOT_LOOKUP[] = {0xcc, 0xdd, 0xee, 0xff};

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> coco_multipak_device

namespace
{
	class coco_multipak_device
		: public device_t
		, public device_cococart_interface
		, public device_cococart_host_interface
	{
	public:
		// construction/destruction
		coco_multipak_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

		INPUT_CHANGED_MEMBER( switch_changed );

	protected:
		coco_multipak_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

		// device-level overrides
		virtual void device_start() override ATTR_COLD;
		virtual void device_reset() override ATTR_COLD;
		virtual u8 cts_read(offs_t offset) override;
		virtual void cts_write(offs_t offset, u8 data) override;
		virtual u8 scs_read(offs_t offset) override;
		virtual void scs_write(offs_t offset, u8 data) override;
		virtual void set_sound_enable(bool sound_enable) override;

		// optional information overrides
		virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

		virtual u8 *get_cart_base() override;
		virtual u32 get_cart_size() override;

		virtual address_space &cartridge_space() override;
		virtual ioport_constructor device_input_ports() const override ATTR_COLD;

		// device references
		required_device_array<cococart_slot_device, 4> m_slots;

		void update_line(int slot_number, line ln);

	private:
		// internal state
		u8 m_select;
		u8 m_block;

		// internal accessors
		int active_scs_slot_number() const;
		int active_cts_slot_number() const;
		cococart_slot_device &slot(int slot_number);
		cococart_slot_device &active_scs_slot();
		cococart_slot_device &active_cts_slot();

		// methods
		void set_select(u8 new_select);
		u8 ff7f_read();
		void ff7f_write(u8 data);
	};
};


// ======================> dragon_multipak_device

class dragon_multipak_device : public coco_multipak_device
{
public:
	// construction/destruction
	dragon_multipak_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

static void coco_cart_slot1_3(device_slot_interface &device)
{
	coco_cart_add_basic_devices(device);
}

static void coco_cart_slot4(device_slot_interface &device)
{
	coco_cart_add_basic_devices(device);
	coco_cart_add_fdcs(device);
}


void coco_multipak_device::device_add_mconfig(machine_config &config)
{
	COCOCART_SLOT(config, m_slots[0], DERIVED_CLOCK(1, 1), coco_cart_slot1_3, nullptr);
	m_slots[0]->cart_callback().set([this](int state) { update_line(1, line::CART); });
	m_slots[0]->nmi_callback().set([this](int state) { update_line(1, line::NMI); });
	m_slots[0]->halt_callback().set([this](int state) { update_line(1, line::HALT); });
	COCOCART_SLOT(config, m_slots[1], DERIVED_CLOCK(1, 1), coco_cart_slot1_3, nullptr);
	m_slots[1]->cart_callback().set([this](int state) { update_line(2, line::CART); });
	m_slots[1]->nmi_callback().set([this](int state) { update_line(2, line::NMI); });
	m_slots[1]->halt_callback().set([this](int state) { update_line(2, line::HALT); });
	COCOCART_SLOT(config, m_slots[2], DERIVED_CLOCK(1, 1), coco_cart_slot1_3, nullptr);
	m_slots[2]->cart_callback().set([this](int state) { update_line(3, line::CART); });
	m_slots[2]->nmi_callback().set([this](int state) { update_line(3, line::NMI); });
	m_slots[2]->halt_callback().set([this](int state) { update_line(3, line::HALT); });
	COCOCART_SLOT(config, m_slots[3], DERIVED_CLOCK(1, 1), coco_cart_slot4, "fdc");
	m_slots[3]->cart_callback().set([this](int state) { update_line(4, line::CART); });
	m_slots[3]->nmi_callback().set([this](int state) { update_line(4, line::NMI); });
	m_slots[3]->halt_callback().set([this](int state) { update_line(4, line::HALT); });
}


static void dragon_cart_slot1_3(device_slot_interface &device)
{
	dragon_cart_add_basic_devices(device);
}

static void dragon_cart_slot4(device_slot_interface &device)
{
	dragon_cart_add_basic_devices(device);
	dragon_cart_add_fdcs(device);
}


void dragon_multipak_device::device_add_mconfig(machine_config &config)
{
	COCOCART_SLOT(config, m_slots[0], DERIVED_CLOCK(1, 1), dragon_cart_slot1_3, nullptr);
	m_slots[0]->cart_callback().set([this](int state) { update_line(1, line::CART); });
	m_slots[0]->nmi_callback().set([this](int state) { update_line(1, line::NMI); });
	m_slots[0]->halt_callback().set([this](int state) { update_line(1, line::HALT); });
	COCOCART_SLOT(config, m_slots[1], DERIVED_CLOCK(1, 1), dragon_cart_slot1_3, nullptr);
	m_slots[1]->cart_callback().set([this](int state) { update_line(2, line::CART); });
	m_slots[1]->nmi_callback().set([this](int state) { update_line(2, line::NMI); });
	m_slots[1]->halt_callback().set([this](int state) { update_line(2, line::HALT); });
	COCOCART_SLOT(config, m_slots[2], DERIVED_CLOCK(1, 1), dragon_cart_slot1_3, nullptr);
	m_slots[2]->cart_callback().set([this](int state) { update_line(3, line::CART); });
	m_slots[2]->nmi_callback().set([this](int state) { update_line(3, line::NMI); });
	m_slots[2]->halt_callback().set([this](int state) { update_line(3, line::HALT); });
	COCOCART_SLOT(config, m_slots[3], DERIVED_CLOCK(1, 1), dragon_cart_slot4, "dragon_fdc");
	m_slots[3]->cart_callback().set([this](int state) { update_line(4, line::CART); });
	m_slots[3]->nmi_callback().set([this](int state) { update_line(4, line::NMI); });
	m_slots[3]->halt_callback().set([this](int state) { update_line(4, line::HALT); });
}

INPUT_PORTS_START( coco_multipack )
	PORT_START( SWITCH_CONFIG_TAG )
	PORT_CONFNAME( 0x03, 0x03, "Multi-Pak Slot Switch" ) PORT_CHANGED_MEMBER(DEVICE_SELF, coco_multipak_device, switch_changed, 0)
		PORT_CONFSETTING( 0x00, "Slot 1" )
		PORT_CONFSETTING( 0x01, "Slot 2" )
		PORT_CONFSETTING( 0x02, "Slot 3" )
		PORT_CONFSETTING( 0x03, "Slot 4" )
INPUT_PORTS_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(COCO_MULTIPAK, device_cococart_interface, coco_multipak_device, "coco_multipack", "CoCo Multi-Pak Interface")
DEFINE_DEVICE_TYPE_PRIVATE(DRAGON_MULTIPAK, device_cococart_interface, dragon_multipak_device, "dragon_multipack", "Dragon Multi-Pak Interface")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor coco_multipak_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( coco_multipack );
}

//-------------------------------------------------
//  coco_multipak_device - constructor
//-------------------------------------------------

coco_multipak_device::coco_multipak_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_cococart_interface(mconfig, *this)
	, m_slots(*this, "slot%u", 1), m_select(0), m_block(0)
{
}

coco_multipak_device::coco_multipak_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: coco_multipak_device(mconfig, COCO_MULTIPAK, tag, owner, clock)
{
}

dragon_multipak_device::dragon_multipak_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: coco_multipak_device(mconfig, DRAGON_MULTIPAK, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void coco_multipak_device::device_start()
{
	// install $FF7F handler
	install_readwrite_handler(0xFF7F, 0xFF7F, read8smo_delegate(*this, FUNC(coco_multipak_device::ff7f_read)), write8smo_delegate(*this, FUNC(coco_multipak_device::ff7f_write)));

	// initial state
	m_select = 0xFF;
	m_block = 0;

	// save state
	save_item(NAME(m_select));
	save_item(NAME(m_block));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void coco_multipak_device::device_reset()
{
	set_select(MULTI_SLOT_LOOKUP[ioport(SWITCH_CONFIG_TAG)->read()]);
	m_block = 0;
}


//-------------------------------------------------
//  switch_changed - panel switch changed
//-------------------------------------------------

INPUT_CHANGED_MEMBER( coco_multipak_device::switch_changed )
{
	if (m_block == 0)
		set_select(MULTI_SLOT_LOOKUP[newval]);
}


//**************************************************************************
//  INTERNAL ACCESSORS
//**************************************************************************

//-------------------------------------------------
//  active_scs_slot_number
//-------------------------------------------------

int coco_multipak_device::active_scs_slot_number() const
{
	return ((m_select >> 0) & 0x03) + 1;
}


//-------------------------------------------------
//  active_cts_slot_number
//-------------------------------------------------

int coco_multipak_device::active_cts_slot_number() const
{
	return ((m_select >> 4) & 0x03) + 1;
}


//-------------------------------------------------
//  slot
//-------------------------------------------------

cococart_slot_device &coco_multipak_device::slot(int slot_number)
{
	assert(slot_number >= 1 && slot_number <= 4);
	return *m_slots[slot_number - 1];
}


//-------------------------------------------------
//  active_scs_slot
//-------------------------------------------------

cococart_slot_device &coco_multipak_device::active_scs_slot()
{
	int slot_number = active_scs_slot_number();
	return slot(slot_number);
}


//-------------------------------------------------
//  active_cts_slot
//-------------------------------------------------

cococart_slot_device &coco_multipak_device::active_cts_slot()
{
	int slot_number = active_cts_slot_number();
	return slot(slot_number);
}


//**************************************************************************
//  METHODS
//**************************************************************************

//-------------------------------------------------
//  set_select
//-------------------------------------------------

void coco_multipak_device::set_select(u8 new_select)
{
	LOGSWITCH( "set_select: 0x%02X\n", new_select);

	// identify old value for CART, in case this needs to change
	cococart_slot_device::line_value old_cart = active_cts_slot().get_line_value(line::CART);

	// change value
	m_select = new_select;

	// did the CART line change?
	line_value new_cart = active_cts_slot().get_line_value(line::CART);
	if (new_cart != old_cart)
		update_line(active_cts_slot_number(), line::CART);

	cart_base_changed();
}


//-------------------------------------------------
//  ff7f_read
//-------------------------------------------------

u8 coco_multipak_device::ff7f_read()
{
	return m_select | 0xcc;
}

//-------------------------------------------------
//  ff7f_write
//-------------------------------------------------

void coco_multipak_device::ff7f_write(u8 data)
{
	m_block = 0xff;
	set_select(data);
}


//-------------------------------------------------
//  update_line
//-------------------------------------------------

void coco_multipak_device::update_line(int slot_number, line ln)
{
	bool propagate;

	// one of our child devices set a line; we may need to propagate it upwards
	switch (ln)
	{
	case line::CART:
		// only propagate if this is coming from the slot specified
		propagate = slot_number == active_cts_slot_number();
		LOGCART( "update_line: slot: %d, line: CART, value: %s, propogate: %s\n", slot_number, owning_slot().line_value_string(slot(slot_number).get_line_value(ln)), propagate ? "yes" : "no" );
		break;

	case line::NMI:
	case line::HALT:
		// always propagate these
		propagate = true;
		break;

	default:
		// do nothing
		propagate = false;
		break;
	}

	if (propagate)
		owning_slot().set_line_value(ln, slot(slot_number).get_line_value(ln));
}


//-------------------------------------------------
//  set_sound_enable
//-------------------------------------------------

void coco_multipak_device::set_sound_enable(bool sound_enable)
{
	// the SOUND_ENABLE (SNDEN) line is different; it is controlled by the CPU
	for (cococart_slot_device *slot : m_slots)
		slot->set_line_value(line::SOUND_ENABLE, sound_enable ? line_value::ASSERT : line_value::CLEAR);
}


//-------------------------------------------------
//  get_cart_base
//-------------------------------------------------

u8 *coco_multipak_device::get_cart_base()
{
	return active_cts_slot().get_cart_base();
}


//-------------------------------------------------
//  get_cart_size
//-------------------------------------------------

u32 coco_multipak_device::get_cart_size()
{
	return active_cts_slot().get_cart_size();
}


//-------------------------------------------------
//  cts_read
//-------------------------------------------------

u8 coco_multipak_device::cts_read(offs_t offset)
{
	return active_cts_slot().cts_read(offset);
}


//-------------------------------------------------
//  cts_write
//-------------------------------------------------

void coco_multipak_device::cts_write(offs_t offset, u8 data)
{
	active_cts_slot().cts_write(offset, data);
}


//-------------------------------------------------
//  scs_read
//-------------------------------------------------

u8 coco_multipak_device::scs_read(offs_t offset)
{
	return active_scs_slot().scs_read(offset);
}


//-------------------------------------------------
//  scs_write
//-------------------------------------------------

void coco_multipak_device::scs_write(offs_t offset, u8 data)
{
	active_scs_slot().scs_write(offset, data);
}


//-------------------------------------------------
//  cartridge_space
//-------------------------------------------------

address_space &coco_multipak_device::cartridge_space()
{
	return device_cococart_interface::cartridge_space();
}
