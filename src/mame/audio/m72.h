/***************************************************************************

    M72 audio interface

****************************************************************************/

SOUND_START( m72 );
SOUND_RESET( m72 );
void m72_ym2151_irq_handler(device_t *device, int irq);
WRITE8_HANDLER( m72_sound_command_byte_w );
WRITE16_HANDLER( m72_sound_command_w );
WRITE8_HANDLER( m72_sound_irq_ack_w );
READ8_HANDLER( m72_sample_r );
WRITE8_DEVICE_HANDLER( m72_sample_w );

/* the port goes to different address bits depending on the game */
void m72_set_sample_start(int start);
WRITE8_HANDLER( vigilant_sample_addr_w );
WRITE8_HANDLER( shisen_sample_addr_w );
WRITE8_HANDLER( rtype2_sample_addr_w );
WRITE8_HANDLER( poundfor_sample_addr_w );
