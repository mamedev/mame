/* XMODEM protocol implementation.

   Transfer between an emulated machine and an image using the XMODEM protocol.

   Used in the HP48 G/GX emulation.

   Author: Antoine Mine'
   Date: 29/03/2008
 */

#include "timer.h"
#include "image.h"


DECLARE_LEGACY_IMAGE_DEVICE(XMODEM, xmodem);


typedef struct {

	/* called by XMODEM when it wants to send a byte to the emulated machine */
	void (*send)( running_machine &machine, UINT8 data );

} xmodem_config;


#define MCFG_XMODEM_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, XMODEM, 0)	      \
	MCFG_DEVICE_CONFIG(_intrf)



/* call when the emulated machine has read the last byte sent by
   XMODEM through the send call-back */
extern void xmodem_byte_transmitted( device_t *device );

/* call when the emulated machine sends a byte to XMODEM */
extern void xmodem_receive_byte( device_t *device, UINT8 data );
