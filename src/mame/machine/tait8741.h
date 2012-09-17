#ifndef __TAITO8741__
#define __TAITO8741__

#include "devcb.h"

/****************************************************************************
  gladiatr and Great Swordsman set.
****************************************************************************/

#define MAX_TAITO8741 4

/* NEC 8741 program mode */
#define TAITO8741_MASTER 0
#define TAITO8741_SLAVE  1
#define TAITO8741_PORT   2

struct TAITO8741interface
{
	int num;
	int mode[MAX_TAITO8741];            /* program select */
	int serial_connect[MAX_TAITO8741];	/* serial port connection */
	read8_space_func portHandler_r[MAX_TAITO8741]; /* parallel port handler */
	const char *portName_r[MAX_TAITO8741];
};

int  TAITO8741_start(const struct TAITO8741interface *taito8741intf);

void TAITO8741_reset(int num);

/* write handler */
DECLARE_WRITE8_HANDLER( TAITO8741_0_w );
DECLARE_WRITE8_HANDLER( TAITO8741_1_w );
DECLARE_WRITE8_HANDLER( TAITO8741_2_w );
DECLARE_WRITE8_HANDLER( TAITO8741_3_w );
/* read handler */
DECLARE_READ8_HANDLER( TAITO8741_0_r );
DECLARE_READ8_HANDLER( TAITO8741_1_r );
DECLARE_READ8_HANDLER( TAITO8741_2_r );
DECLARE_READ8_HANDLER( TAITO8741_3_r );

/****************************************************************************
  joshi Volleyball set.
****************************************************************************/

void josvolly_8741_reset(void);
DECLARE_WRITE8_HANDLER( josvolly_8741_0_w );
DECLARE_WRITE8_HANDLER( josvolly_8741_1_w );
DECLARE_READ8_HANDLER( josvolly_8741_0_r );
DECLARE_READ8_HANDLER( josvolly_8741_1_r );
DECLARE_WRITE8_HANDLER( josvolly_nmi_enable_w );

#endif


