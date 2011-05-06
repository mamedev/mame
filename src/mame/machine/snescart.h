/*
    snescart.h

*/

#ifndef _SNESCART_H
#define _SNESCART_H

#include "imagedev/cartslot.h"

MACHINE_START( snes_mess );
MACHINE_START( snesst );

DRIVER_INIT( snes_mess );
DRIVER_INIT( snesst );

MACHINE_CONFIG_EXTERN( snes_cartslot );
MACHINE_CONFIG_EXTERN( sufami_cartslot );
MACHINE_CONFIG_EXTERN( bsx_cartslot );

#endif /* _SNESCART_H */

