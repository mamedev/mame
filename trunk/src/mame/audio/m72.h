/***************************************************************************

    M72 audio interface

****************************************************************************/

void m72_ym2151_irq_handler(device_t *device, int irq);
WRITE8_DEVICE_HANDLER( m72_sound_command_byte_w );
WRITE16_DEVICE_HANDLER( m72_sound_command_w );
WRITE8_DEVICE_HANDLER( m72_sound_irq_ack_w );
READ8_DEVICE_HANDLER( m72_sample_r );
WRITE8_DEVICE_HANDLER( m72_sample_w );

/* the port goes to different address bits depending on the game */
void m72_set_sample_start(device_t *device, int start);
WRITE8_DEVICE_HANDLER( vigilant_sample_addr_w );
WRITE8_DEVICE_HANDLER( shisen_sample_addr_w );
WRITE8_DEVICE_HANDLER( rtype2_sample_addr_w );
WRITE8_DEVICE_HANDLER( poundfor_sample_addr_w );

DECLARE_LEGACY_SOUND_DEVICE(M72, m72_audio);
