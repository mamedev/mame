/*
    snescart.h

*/

#ifndef _SNESCART_H
#define _SNESCART_H

#include "imagedev/cartslot.h"

void snes_machine_stop(running_machine &machine);
void sufami_machine_stop(running_machine &machine);

MACHINE_CONFIG_EXTERN( snes_cartslot );
MACHINE_CONFIG_EXTERN( snesp_cartslot );
MACHINE_CONFIG_EXTERN( sufami_cartslot );

#endif /* _SNESCART_H */
