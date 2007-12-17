#ifndef __TAITO8741__
#define __TAITO8741__

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
	read8_handler portHandler_r[MAX_TAITO8741]; /* parallel port handler */
};

int  TAITO8741_start(const struct TAITO8741interface *taito8741intf);

void TAITO8741_reset(int num);

/* write handler */
WRITE8_HANDLER( TAITO8741_0_w );
WRITE8_HANDLER( TAITO8741_1_w );
WRITE8_HANDLER( TAITO8741_2_w );
WRITE8_HANDLER( TAITO8741_3_w );
/* read handler */
READ8_HANDLER( TAITO8741_0_r );
READ8_HANDLER( TAITO8741_1_r );
READ8_HANDLER( TAITO8741_2_r );
READ8_HANDLER( TAITO8741_3_r );

/****************************************************************************
  joshi Vollyball set.
****************************************************************************/

extern int josvolly_nmi_enable;

void josvolly_8741_reset(void);
WRITE8_HANDLER( josvolly_8741_0_w );
WRITE8_HANDLER( josvolly_8741_1_w );
READ8_HANDLER( josvolly_8741_0_r );
READ8_HANDLER( josvolly_8741_1_r );

#endif
