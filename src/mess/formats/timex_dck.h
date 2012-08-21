/*****************************************************************************
 *
 * formats/timex_dck.h
 *
 ****************************************************************************/

#ifndef __TIMEX_DCK_H__
#define __TIMEX_DCK_H__

#include "imagedev/snapquik.h"
#include "imagedev/cartslot.h"

enum
{
	TIMEX_CART_NONE,
	TIMEX_CART_DOCK,
	TIMEX_CART_EXROM,
	TIMEX_CART_HOME
};

typedef struct
{
	int type;
	UINT8 chunks;
	UINT8 *data;
} timex_cart_t;

const timex_cart_t *timex_cart_data(void);

DEVICE_IMAGE_LOAD( timex_cart );
DEVICE_IMAGE_UNLOAD( timex_cart );

#endif  /* __TIMEX_DCK_H__ */
