/***************************************************************************

    Z80 PIO implementation

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

/***************************************************************************
    CONSTANTS
***************************************************************************/

#define MAX_PIO 2



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct _z80pio_interface
{
	void (*intr)(running_machine *machine, int which);    /* callback when change interrupt status */
	read8_machine_func portAread;    /* port A read callback */
	read8_machine_func portBread;    /* port B read callback */
	write8_machine_func portAwrite;  /* port A write callback */
	write8_machine_func portBwrite;  /* port B write callback */
	void (*rdyA)(int data);     /* portA ready active callback (do not support yet)*/
	void (*rdyB)(int data);     /* portB ready active callback (do not support yet)*/
};
typedef struct _z80pio_interface z80pio_interface;



/***************************************************************************
    INITIALIZATION/CONFIGURATION
***************************************************************************/

void z80pio_init(int which, const z80pio_interface *intf);
void z80pio_reset(int which);



/***************************************************************************
    CONTROL REGISTER READ/WRITE
***************************************************************************/

void z80pio_c_w(int which, int ch, UINT8 data);
UINT8 z80pio_c_r(int which, int ch);



/***************************************************************************
    DATA REGISTER READ/WRITE
***************************************************************************/

void z80pio_d_w(int which, int ch, UINT8 data);
UINT8 z80pio_d_r(int which, int ch);



/***************************************************************************
    PORT I/O
***************************************************************************/

void z80pio_p_w(int which, UINT8 ch, UINT8 data);
int z80pio_p_r(int which, UINT8 ch);

WRITE8_HANDLER( z80pioA_0_p_w );
WRITE8_HANDLER( z80pioB_0_p_w );
READ8_HANDLER( z80pioA_0_p_r );
READ8_HANDLER( z80pioB_0_p_r );
WRITE8_HANDLER( z80pioA_1_p_w );
WRITE8_HANDLER( z80pioB_1_p_w );
READ8_HANDLER( z80pioA_1_p_r );
READ8_HANDLER( z80pioB_1_p_r );



/***************************************************************************
    STROBE STATE MANAGEMENT
***************************************************************************/

void z80pio_astb_w(int which, int state);
void z80pio_bstb_w(int which, int state);



/***************************************************************************
    DAISY CHAIN INTERFACE
***************************************************************************/

int z80pio_irq_state(int which);
int z80pio_irq_ack(int which);
void z80pio_irq_reti(int which);

/***************************************************************************
    READ/WRITE HANDLERS
***************************************************************************/
READ8_HANDLER(z80pio_0_r);
WRITE8_HANDLER(z80pio_0_w);
READ8_HANDLER(z80pio_1_r);
WRITE8_HANDLER(z80pio_1_w);
