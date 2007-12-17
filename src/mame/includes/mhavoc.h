/*************************************************************************

    Atari Major Havoc hardware

*************************************************************************/

#define MHAVOC_CLOCK		10000000
#define MHAVOC_CLOCK_5M		(MHAVOC_CLOCK/2)
#define MHAVOC_CLOCK_2_5M	(MHAVOC_CLOCK/4)
#define MHAVOC_CLOCK_1_25M	(MHAVOC_CLOCK/8)
#define MHAVOC_CLOCK_625K	(MHAVOC_CLOCK/16)

#define MHAVOC_CLOCK_156K	(MHAVOC_CLOCK_625K/4)
#define MHAVOC_CLOCK_5K		(MHAVOC_CLOCK_625K/16/8)
#define MHAVOC_CLOCK_2_4K	(MHAVOC_CLOCK_625K/16/16)


/*----------- defined in machine/mhavoc.c -----------*/

WRITE8_HANDLER( mhavoc_alpha_irq_ack_w );
WRITE8_HANDLER( mhavoc_gamma_irq_ack_w );

MACHINE_RESET( mhavoc );

WRITE8_HANDLER( mhavoc_gamma_w );
READ8_HANDLER( mhavoc_alpha_r );

WRITE8_HANDLER( mhavoc_alpha_w );
READ8_HANDLER( mhavoc_gamma_r );

WRITE8_HANDLER( mhavoc_ram_banksel_w );
WRITE8_HANDLER( mhavoc_rom_banksel_w );

READ8_HANDLER( mhavoc_port_0_r );
READ8_HANDLER( alphaone_port_0_r );
READ8_HANDLER( mhavoc_port_1_r );
READ8_HANDLER( mhavoc_port_1_sp_r );

WRITE8_HANDLER( mhavoc_out_0_w );
WRITE8_HANDLER( alphaone_out_0_w );
WRITE8_HANDLER( mhavoc_out_1_w );

extern UINT8 *mhavoc_zram0, *mhavoc_zram1;
