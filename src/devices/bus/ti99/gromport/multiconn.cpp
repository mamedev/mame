// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/**************************************************************************
    The multi-cartridge extender

    This is a somewhat mythical device which was never available for the normal
    customer, but there are reports of the existence of such a device
    in development labs or demonstrations.

    The interesting thing about this is that the OS of the console
    fully supports this multi-cartridge extender, providing a selection
    option on the screen to switch between different plugged-in
    cartridges.

    The switching is possible by decoding address lines that are reserved
    for GROM access. GROMs are accessed via four separate addresses
    9800, 9802, 9C00, 9C02. The addressing scheme looks like this:

    1001 1Wxx xxxx xxM0        W = write(1), read(0), M = address(1), data(0)

    This leaves 8 bits (256 options) which are not decoded inside the
    console. As the complete address is routed to the port, some circuit
    just needs to decode the xxx lines and turn on the respective slot.

    One catch must be considered: Some cartridges contain ROMs which are
    directly accessed and not via ports. This means that the ROMs must
    be activated according to the slot that is selected.

    Another issue: Each GROM contains an own address counter and an ID.
    According to the ID the GROM only delivers data if the address counter
    is within the ID area (0 = 0000-1fff, 1=2000-3fff ... 7=e000-ffff).
    Thus it is essential that all GROMs stay in sync with their address
    counters. We have to route all address settings to all slots and their
    GROMs, even when the slot has not been selected before. The selected
    just shows its effect when data is read. In this case, only the
    data from the selected slot will be delivered.

    This may be considered as a design flaw within the complete cartridge system
    which eventually led to TI not manufacturing that device for the broad
    market.
***************************************************************************/

#include "emu.h"
#include "multiconn.h"

#define LOG_WARN             (1U<<1)
#define LOG_CHANGE          (1U<<2)
#define VERBOSE ( LOG_WARN )

#include "logmacro.h"

DEFINE_DEVICE_TYPE_NS(TI99_GROMPORT_MULTI,  bus::ti99::gromport, ti99_multi_cart_conn_device,  "ti99_mcartconn", "TI-99 Multi-cartridge extender")

namespace bus { namespace ti99 { namespace gromport {

#define AUTO -1

ti99_multi_cart_conn_device::ti99_multi_cart_conn_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cartridge_connector_device(mconfig, TI99_GROMPORT_MULTI, tag, owner, clock),
	m_active_slot(0),
	m_fixed_slot(0),
	m_next_free_slot(0)
{
}

/*
    Activates a slot in the multi-cartridge extender.
    Setting the slot is done by accessing the GROM ports using a
    specific address:

    slot 0:  0x9800 (0x9802, 0x9c00, 0x9c02)   : cartridge1
    slot 1:  0x9804 (0x9806, 0x9c04, 0x9c06)   : cartridge2
    ...
    slot 15: 0x983c (0x983e, 0x9c3c, 0x9c3e)   : cartridge16

    Scheme:

    1001 1Wxx xxxx xxM0 (M=mode; M=0: data, M=1: address; W=write)

    The following addresses are theoretically available, but the
    built-in OS does not use them; i.e. cartridges will not be
    included in the selection list, and their features will not be
    found by lookup, but they could be accessed directly by user
    programs.
    slot 16: 0x9840 (0x9842, 0x9c40, 0x9c42)
    ...
    slot 255:  0x9bfc (0x9bfe, 0x9ffc, 0x9ffe)

    Setting the GROM base should select one cartridge, but the ROMs in the
    CPU space must also be switched. As there is no known special mechanism
    we assume that by switching the GROM base, the ROM is automatically
    switched.

    Caution: This means that cartridges which do not have at least one
    GROM cannot be switched with this mechanism.

    We assume that the slot number is already calculated in the caller:
    slotnumber>=0 && slotnumber<=255

    NOTE: The OS will stop searching when it finds slots 1 and 2 empty.
    Interestingly, cartridge subroutines are found nevertheless, even when
    the cartridge is plugged into a higher slot.
*/
void ti99_multi_cart_conn_device::set_slot(int slotnumber)
{
	if (m_active_slot != slotnumber)
		LOGMASKED(LOG_CHANGE, "Setting cartslot to %d\n", slotnumber);

	if (m_fixed_slot==AUTO)
		m_active_slot = slotnumber;
	else
		m_active_slot = m_fixed_slot;
}

int ti99_multi_cart_conn_device::get_active_slot(bool changebase, offs_t offset)
{
	int slot;
	if (changebase)
	{
		// GROM selected?
		if (m_grom_selected) set_slot((offset>>2) & 0x00ff);
	}
	slot = m_active_slot;
	return slot;
}

void ti99_multi_cart_conn_device::insert(int index, ti99_cartridge_device* cart)
{
	LOGMASKED(LOG_CHANGE, "Insert slot %d\n", index);
	m_cartridge[index] = cart;
	m_gromport->cartridge_inserted();
}

void ti99_multi_cart_conn_device::remove(int index)
{
	LOGMASKED(LOG_CHANGE, "Remove slot %d\n", index);
	m_cartridge[index] = nullptr;
}

WRITE_LINE_MEMBER(ti99_multi_cart_conn_device::romgq_line)
{
	m_readrom = state;

	// Propagate to all slots
	for (int i=0; i < NUMBER_OF_CARTRIDGE_SLOTS; i++)
	{
		if (m_cartridge[i] != nullptr)
		{
			m_cartridge[i]->romgq_line(state);
		}
	}
}

/*
    Combined select lines
*/
void ti99_multi_cart_conn_device::set_gromlines(line_state mline, line_state moline, line_state gsq)
{
	// GROM selected?
	m_grom_selected = (gsq == ASSERT_LINE);

	// Propagate to all slots
	for (int i=0; i < NUMBER_OF_CARTRIDGE_SLOTS; i++)
	{
		if (m_cartridge[i] != nullptr)
		{
			m_cartridge[i]->set_gromlines(mline, moline, gsq);
		}
	}
}

WRITE_LINE_MEMBER(ti99_multi_cart_conn_device::gclock_in)
{
	// Propagate to all slots
	for (int i=0; i < NUMBER_OF_CARTRIDGE_SLOTS; i++)
	{
		if (m_cartridge[i] != nullptr)
		{
			m_cartridge[i]->gclock_in(state);
		}
	}
}

READ8Z_MEMBER(ti99_multi_cart_conn_device::readz)
{
	int slot = get_active_slot(true, offset);

	// If we have a GROM access, we need to send the read request to all
	// attached cartridges so the slot is irrelevant here. Each GROM
	// contains an internal address counter, and we must make sure they all stay in sync.
	if (m_grom_selected)
	{
		for (int i=0; i < NUMBER_OF_CARTRIDGE_SLOTS; i++)
		{
			if (m_cartridge[i] != nullptr)
			{
				uint8_t newval = *value;
				m_cartridge[i]->readz(space, offset, &newval, 0xff);
				if (i==slot)
				{
					*value = newval;
				}
			}
		}
	}
	else
	{
		if (slot < NUMBER_OF_CARTRIDGE_SLOTS && m_cartridge[slot] != nullptr)
		{
			m_cartridge[slot]->readz(space, offset, value, 0xff);
		}
	}
}

WRITE8_MEMBER(ti99_multi_cart_conn_device::write)
{
	// Same issue as above (read)
	// We don't have GRAM cartridges, anyway, so it's just used for setting the address.
	if (m_grom_selected)
	{
		for (auto & elem : m_cartridge)
		{
			if (elem != nullptr)
			{
				elem->write(space, offset, data, 0xff);
			}
		}
	}
	else
	{
		int slot = get_active_slot(true, offset);
		if (slot < NUMBER_OF_CARTRIDGE_SLOTS && m_cartridge[slot] != nullptr)
		{
			// logerror("writing %04x (slot %d) <- %02x\n", offset, slot, data);
			m_cartridge[slot]->write(space, offset, data, 0xff);
		}
	}
}

READ8Z_MEMBER(ti99_multi_cart_conn_device::crureadz)
{
	int slot = get_active_slot(false, offset);
	/* Sanity check. Higher slots are always empty. */
	if (slot >= NUMBER_OF_CARTRIDGE_SLOTS)
		return;

	if (m_cartridge[slot] != nullptr)
	{
		m_cartridge[slot]->crureadz(space, offset, value);
	}
}

WRITE8_MEMBER(ti99_multi_cart_conn_device::cruwrite)
{
	int slot = get_active_slot(true, offset);

	/* Sanity check. Higher slots are always empty. */
	if (slot >= NUMBER_OF_CARTRIDGE_SLOTS)
		return;

	if (m_cartridge[slot] != nullptr)
	{
		m_cartridge[slot]->cruwrite(space, offset, data);
	}
}

/*
    Check whether the GROMs are idle. Just ask the currently
    active cartridge.
*/
bool ti99_multi_cart_conn_device::is_grom_idle()
{
	/* Sanity check. Higher slots are always empty. */
	if (m_active_slot >= NUMBER_OF_CARTRIDGE_SLOTS)
		return false;

	if (m_cartridge[m_active_slot] != nullptr)
		return m_cartridge[m_active_slot]->is_grom_idle();

	return false;
}

void ti99_multi_cart_conn_device::device_start()
{
	m_next_free_slot = 0;
	m_active_slot = 0;
	for (auto & elem : m_cartridge)
	{
		elem = nullptr;
	}
	save_item(NAME(m_readrom));
	save_item(NAME(m_active_slot));
	save_item(NAME(m_fixed_slot));
	save_item(NAME(m_next_free_slot));
}

void ti99_multi_cart_conn_device::device_reset(void)
{
	m_active_slot = 0;
	m_fixed_slot = ioport("CARTSLOT")->read() - 1;
	m_grom_selected = false;
}

void ti99_multi_cart_conn_device::device_add_mconfig(machine_config &config)
{
	TI99_CART(config, "cartridge1", 0);
	TI99_CART(config, "cartridge2", 0);
	TI99_CART(config, "cartridge3", 0);
	TI99_CART(config, "cartridge4", 0);
}

INPUT_CHANGED_MEMBER( ti99_multi_cart_conn_device::switch_changed )
{
	LOGMASKED(LOG_CHANGE, "Slot changed %d - %d\n", (int)((uint64_t)param & 0x07), newval);
	m_active_slot = m_fixed_slot = newval - 1;
}

INPUT_PORTS_START(multi_slot)
	PORT_START( "CARTSLOT" )
	PORT_DIPNAME( 0x0f, 0x00, "Multi-cartridge slot" ) PORT_CHANGED_MEMBER(DEVICE_SELF, ti99_multi_cart_conn_device, switch_changed, 0)
		PORT_DIPSETTING(    0x00, "Auto" )
		PORT_DIPSETTING(    0x01, "Slot 1" )
		PORT_DIPSETTING(    0x02, "Slot 2" )
		PORT_DIPSETTING(    0x03, "Slot 3" )
		PORT_DIPSETTING(    0x04, "Slot 4" )
INPUT_PORTS_END

ioport_constructor ti99_multi_cart_conn_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(multi_slot);
}

} } } // end namespace bus::ti99::gromport

