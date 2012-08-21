/*****************************************************************************
 *
 * includes/tandy1t.h
 *
 ****************************************************************************/

#ifndef TANDY1T_H_
#define TANDY1T_H_


/*----------- defined in machine/tandy1t.c -----------*/

extern WRITE8_HANDLER ( pc_t1t_p37x_w );
extern  READ8_HANDLER ( pc_t1t_p37x_r );

extern WRITE8_HANDLER ( tandy1000_pio_w );
extern  READ8_HANDLER(tandy1000_pio_r);

extern NVRAM_HANDLER( tandy1000 );

MACHINE_RESET( tandy1000rl );
READ8_HANDLER( tandy1000_bank_r );
WRITE8_HANDLER( tandy1000_bank_w );

INPUT_PORTS_EXTERN( t1000_keyboard );


#endif /* TANDY1T_H_ */
