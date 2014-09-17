/**********************************************************************
 
 Generic ROM / RAM socket slots
 
 **********************************************************************/


#include "carts.h"

SLOT_INTERFACE_START(generic_plain_slot)
	SLOT_INTERFACE_INTERNAL("rom", GENERIC_ROM_PLAIN)
SLOT_INTERFACE_END

SLOT_INTERFACE_START(generic_linear_slot)
	SLOT_INTERFACE_INTERNAL("rom", GENERIC_ROM_LINEAR)
SLOT_INTERFACE_END
