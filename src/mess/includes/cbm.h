/*****************************************************************************
 *
 * includes/cbm.h
 *
 ****************************************************************************/

#ifndef CBM_H_
#define CBM_H_

#include "imagedev/cassette.h"


/* global header file for c16, c64, c65, c128, vc20 */

/*----------- defined in machine/cbm.c -----------*/

/* keyboard lines */
extern UINT8 c64_keyline[10];
void cbm_common_init(void);
void cbm_common_interrupt( device_t *device );

UINT8 cbm_common_cia0_port_a_r( device_t *device, UINT8 output_b );
UINT8 cbm_common_cia0_port_b_r( device_t *device, UINT8 output_a );

/***********************************************

    CBM Datasette Tapes

***********************************************/

extern const cassette_interface cbm_cassette_interface;


#endif /* CBM_H_ */
