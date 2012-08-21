/* Kermit protocol implementation.

   Transfer between an emulated machine and an image using the kermit protocol.

   Used in the HP48 S/SX/G/GX emulation.

   Author: Antoine Mine'
   Date: 29/03/2008
 */

#include "timer.h"
#include "image.h"


DECLARE_LEGACY_IMAGE_DEVICE(KERMIT, kermit);


typedef struct {

	/* called by Kermit when it wants to send a byte to the emulated machine */
	void (*send)( running_machine &machine, UINT8 data );

} kermit_config;


#define MCFG_KERMIT_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, KERMIT, 0)	      \
	MCFG_DEVICE_CONFIG(_intrf)


/* call this when the emulated machine has read the last byte sent by
   Kermit through the send call-back */
extern void kermit_byte_transmitted( device_t *device );

/* call this when the emulated machine sends a byte to Kermit */
extern void kermit_receive_byte( device_t *device, UINT8 data );
