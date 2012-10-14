/***************************************************************************

    M72 audio interface

****************************************************************************/

WRITE_LINE_DEVICE_HANDLER(m72_ym2151_irq_handler);
DECLARE_WRITE8_DEVICE_HANDLER( m72_sound_command_byte_w );
DECLARE_WRITE16_DEVICE_HANDLER( m72_sound_command_w );
DECLARE_WRITE8_DEVICE_HANDLER( m72_sound_irq_ack_w );
DECLARE_READ8_DEVICE_HANDLER( m72_sample_r );
DECLARE_WRITE8_DEVICE_HANDLER( m72_sample_w );

/* the port goes to different address bits depending on the game */
void m72_set_sample_start(device_t *device, int start);
DECLARE_WRITE8_DEVICE_HANDLER( vigilant_sample_addr_w );
DECLARE_WRITE8_DEVICE_HANDLER( shisen_sample_addr_w );
DECLARE_WRITE8_DEVICE_HANDLER( rtype2_sample_addr_w );
DECLARE_WRITE8_DEVICE_HANDLER( poundfor_sample_addr_w );

class m72_audio_device : public device_t,
                                  public device_sound_interface
{
public:
	m72_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~m72_audio_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
private:
	// internal state
	void *m_token;
};

extern const device_type M72;

