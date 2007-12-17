/***************************************************************************

    M72 audio interface

****************************************************************************/

MACHINE_RESET( m72_sound );
void m72_ym2151_irq_handler(int irq);
WRITE8_HANDLER( m72_sound_command_byte_w );
WRITE16_HANDLER( m72_sound_command_w );
WRITE8_HANDLER( m72_sound_irq_ack_w );
READ8_HANDLER( m72_sample_r );
WRITE8_HANDLER( m72_sample_w );

/* the port goes to different address bits depending on the game */
void m72_set_sample_start(int start);
WRITE8_HANDLER( vigilant_sample_addr_w );
WRITE8_HANDLER( shisen_sample_addr_w );
WRITE8_HANDLER( rtype2_sample_addr_w );
WRITE8_HANDLER( poundfor_sample_addr_w );
