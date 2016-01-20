// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2lang.c

    Implementation of the Apple II Language Card

    TODO: refactor machine/apple2.c so it's possible to have an Apple II
          and II Plus without a language card (and to emulate other
          slot 0 stuff like hack/freezer cards).

*********************************************************************/

#include "emu.h"
#include "includes/apple2.h"
#include "a2lang.h"


/***************************************************************************
    PARAMETERS
***************************************************************************/

#define LOG_LANGCARD    0

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type A2BUS_LANG = &device_creator<a2bus_lang_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_lang_device::a2bus_lang_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, A2BUS_LANG, "Apple II Language Card", tag, owner, clock, "a2lang", __FILE__),
		device_a2bus_card_interface(mconfig, *this)
{
	last_offset = -1;
}

a2bus_lang_device::a2bus_lang_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
		device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_a2bus_card_interface(mconfig, *this)
{
	last_offset = -1;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_lang_device::device_start()
{
	// set_a2bus_device makes m_slot valid
	set_a2bus_device();

	save_item(NAME(last_offset));
}

void a2bus_lang_device::device_reset()
{
}

/*-------------------------------------------------
    apple2_langcard_touch - device read callback
-------------------------------------------------*/

void a2bus_lang_device::langcard_touch(offs_t offset)
{
	UINT32 val, mask;

	if (LOG_LANGCARD)
		logerror("language card bankswitch read, offset: $c08%0x\n", offset);

	/* determine which flags to change */
	mask = VAR_LCWRITE | VAR_LCRAM | VAR_LCRAM2;
	val = 0;

	if (offset & 0x01)
		val |= VAR_LCWRITE;

	switch(offset & 0x03)
	{
		case 0x03:
		case 0x00:
			val |= VAR_LCRAM;
			break;
	}

	if ((offset & 0x08) == 0)
		val |= VAR_LCRAM2;

	/* change the flags */
	apple2_state *state = machine().driver_data<apple2_state>();
	state->apple2_setvar(val, mask);
}



/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

UINT8 a2bus_lang_device::read_c0nx(address_space &space, UINT8 offset)
{
	apple2_state *state = machine().driver_data<apple2_state>();

	// enforce "read twice" for c081/3/9/B
	// but only on the II/II+ with a discrete language card.
	// later machines' ASICs dropped the double-read requirement,
	// likely to be interrupt-safe.
	if (state->m_machinetype == APPLE_II)
	{
		switch (offset & 0x03)
		{
			case 1:
			case 3:
				if (offset != last_offset)
				{
					last_offset = offset;
					return 0;
				}
				break;
		}
	}

	langcard_touch(offset);
	last_offset = offset;
	return 0;
}



/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_lang_device::write_c0nx(address_space &space, UINT8 offset, UINT8 data)
{
	langcard_touch(offset);
	last_offset = -1;
}
