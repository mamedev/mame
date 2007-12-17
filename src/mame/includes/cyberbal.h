/*************************************************************************

    Atari Cyberball hardware

*************************************************************************/


/*----------- defined in audio/cyberbal.c -----------*/

void cyberbal_sound_reset(void);

INTERRUPT_GEN( cyberbal_sound_68k_irq_gen );

READ8_HANDLER( cyberbal_special_port3_r );
READ8_HANDLER( cyberbal_sound_6502_stat_r );
READ8_HANDLER( cyberbal_sound_68k_6502_r );
WRITE8_HANDLER( cyberbal_sound_bank_select_w );
WRITE8_HANDLER( cyberbal_sound_68k_6502_w );

READ16_HANDLER( cyberbal_sound_68k_r );
WRITE16_HANDLER( cyberbal_io_68k_irq_ack_w );
WRITE16_HANDLER( cyberbal_sound_68k_w );
WRITE16_HANDLER( cyberbal_sound_68k_dac_w );


/*----------- defined in video/cyberbal.c -----------*/

void cyberbal_set_screen(int which);

READ16_HANDLER( cyberbal_paletteram_0_r );
READ16_HANDLER( cyberbal_paletteram_1_r );
WRITE16_HANDLER( cyberbal_paletteram_0_w );
WRITE16_HANDLER( cyberbal_paletteram_1_w );

VIDEO_START( cyberbal );
VIDEO_START( cyberb2p );
VIDEO_UPDATE( cyberbal );

void cyberbal_scanline_update(running_machine *machine, int scrnum, int scanline);

extern UINT16 *cyberbal_paletteram_0;
extern UINT16 *cyberbal_paletteram_1;
