/***************************************************************************

    Z80 CTC (Z8430) implementation

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

/***************************************************************************
    CONSTANTS
***************************************************************************/

#define MAX_CTC 2

#define NOTIMER_0 (1<<0)
#define NOTIMER_1 (1<<1)
#define NOTIMER_2 (1<<2)
#define NOTIMER_3 (1<<3)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct
{
	int baseclock;                           /* timer clock */
	int notimer;                         /* timer disablers */
	void (*intr)(int which);             /* callback when change interrupt status */
	write8_machine_func zc0;   /* ZC/TO0 callback */
	write8_machine_func zc1;   /* ZC/TO1 callback */
	write8_machine_func zc2;   /* ZC/TO2 callback */
} z80ctc_interface;



/***************************************************************************
    INITIALIZATION/CONFIGURATION
***************************************************************************/

void z80ctc_init(int which, z80ctc_interface *intf);
void z80ctc_reset(int which);
attotime z80ctc_getperiod (int which, int ch);



/***************************************************************************
    WRITE HANDLERS
***************************************************************************/

void z80ctc_w(int which, int ch, UINT8 data);
WRITE8_HANDLER( z80ctc_0_w );
WRITE8_HANDLER( z80ctc_1_w );



/***************************************************************************
    READ HANDLERS
***************************************************************************/

UINT8 z80ctc_r(int which, int ch);
READ8_HANDLER( z80ctc_0_r );
READ8_HANDLER( z80ctc_1_r );



/***************************************************************************
    EXTERNAL TRIGGERS
***************************************************************************/

void z80ctc_trg_w(int which, int trg, UINT8 data);
WRITE8_HANDLER( z80ctc_0_trg0_w );
WRITE8_HANDLER( z80ctc_0_trg1_w );
WRITE8_HANDLER( z80ctc_0_trg2_w );
WRITE8_HANDLER( z80ctc_0_trg3_w );
WRITE8_HANDLER( z80ctc_1_trg0_w );
WRITE8_HANDLER( z80ctc_1_trg1_w );
WRITE8_HANDLER( z80ctc_1_trg2_w );
WRITE8_HANDLER( z80ctc_1_trg3_w );



/***************************************************************************
    DAISY CHAIN INTERFACE
***************************************************************************/

int z80ctc_irq_state(int which);
int z80ctc_irq_ack(int which);
void z80ctc_irq_reti(int which);
