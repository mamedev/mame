/*
** File: tms9928a.h -- software implementation of the TMS9928A VDP.
**
** By Sean Young 1999 (sean@msxnet.org).
*/


#define TMS9928A_PALETTE_SIZE           16

/*
** The different models
*/
typedef enum
{
	TMS_INVALID_MODEL,
	TMS99x8,
	TMS9929,
	TMS99x8A,
	TMS9929A
} tms9928a_model;

/*
** MachineDriver video declarations for the TMS9928A chip
*/
typedef struct TMS9928a_interface
{
	tms9928a_model model;		/* model: tms9929(a) runs at 50Hz instead of 60Hz */
	int vram;					/* VRAM size in bytes (4k, 8k or 16k) */
	int borderx, bordery;		/* number of border pixels to show in each direction */
	void (*int_callback)(int);	/* callback which is called whenever the state
                                ** of the INT output of the TMS9918A changes (may be NULL)*/
} TMS9928a_interface;

/*
** configuration function
*/
extern void TMS9928A_configure (const TMS9928a_interface *intf);

/*
** visible area query
*/
extern const rectangle *TMS9928A_get_visarea (void);

/*
** reset function
*/
extern void TMS9928A_reset (void);

/*
** The I/O functions
*/
extern READ8_HANDLER (TMS9928A_vram_r);
extern WRITE8_HANDLER (TMS9928A_vram_w);
extern READ8_HANDLER (TMS9928A_register_r);
extern WRITE8_HANDLER (TMS9928A_register_w);

/*
** Call this function to render the screen.
*/
extern VIDEO_START( tms9928a );
extern VIDEO_UPDATE( tms9928a );

/*
** This next function must be called 50 (tms9929a) or 60 (tms99x8a) times per second,
** to generate the necessary interrupts
*/
int TMS9928A_interrupt (void);

/*
** The parameter is a function pointer. This function is called whenever
** the state of the INT output of the TMS9918A changes.
*/
/*void TMS9928A_int_callback (void (*callback)(int));*/

/*
** Set display of illegal sprites on or off
*/
void TMS9928A_set_spriteslimit (int);

/*
** After loading a state, call this function
*/
void TMS9928A_post_load (void);

/*
** MachineDriver video declarations for the TMS9928A chip
*/
MACHINE_DRIVER_EXTERN( tms9928a );
