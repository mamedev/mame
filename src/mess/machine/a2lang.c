/*********************************************************************

    a2lang.c

    Implementation of the Apple II Language Card

    TODO: refactor machine/apple2.c so it's possible to have an Apple II
          and II Plus without a language card (and to emulate other
          slot 0 stuff like hack/freezer cards).

*********************************************************************/

#include "emu.h"
#include "includes/apple2.h"
#include "machine/a2lang.h"


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
		device_t(mconfig, A2BUS_LANG, "Apple II Language Card", tag, owner, clock),
		device_a2bus_card_interface(mconfig, *this)
{
	m_shortname = "a2lang";
}

a2bus_lang_device::a2bus_lang_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, type, name, tag, owner, clock),
		device_a2bus_card_interface(mconfig, *this)
{
	m_shortname = "a2lang";
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_lang_device::device_start()
{
	// set_a2bus_device makes m_slot valid
	set_a2bus_device();
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
	apple2_setvar(machine(), val, mask);
}



/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

UINT8 a2bus_lang_device::read_c0nx(address_space &space, UINT8 offset)
{
	langcard_touch(offset);
	return 0;
}



/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_lang_device::write_c0nx(address_space &space, UINT8 offset, UINT8 data)
{
	langcard_touch(offset);
}
