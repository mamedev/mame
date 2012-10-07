/*************************************************************************

    Atari Battle Zone hardware

*************************************************************************/

#include "devlegcy.h"
#include "sound/discrete.h"

#define BZONE_MASTER_CLOCK (XTAL_12_096MHz)
#define BZONE_CLOCK_3KHZ  (MASTER_CLOCK / 4096)

class bzone_state : public driver_device
{
public:
	bzone_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_discrete(*this, "discrete") { }

	optional_device<discrete_device> m_discrete;

	UINT8 m_analog_data;
	UINT8 m_rb_input_select;
	DECLARE_WRITE8_MEMBER(bzone_coin_counter_w);
	DECLARE_READ8_MEMBER(analog_data_r);
	DECLARE_WRITE8_MEMBER(analog_select_w);
	DECLARE_CUSTOM_INPUT_MEMBER(clock_r);
	DECLARE_READ8_MEMBER(redbaron_joy_r);
	DECLARE_WRITE8_MEMBER(redbaron_joysound_w);
	DECLARE_DRIVER_INIT(bradley);
	virtual void machine_start();
	DECLARE_MACHINE_START(redbaron);
	INTERRUPT_GEN_MEMBER(bzone_interrupt);
	DECLARE_WRITE8_MEMBER(bzone_sounds_w);
};


/*----------- defined in audio/bzone.c -----------*/
MACHINE_CONFIG_EXTERN( bzone_audio );

/*----------- defined in audio/redbaron.c -----------*/

DECLARE_WRITE8_DEVICE_HANDLER( redbaron_sounds_w );

class redbaron_sound_device : public device_t,
                                  public device_sound_interface
{
public:
	redbaron_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~redbaron_sound_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
private:
	// internal state
	void *m_token;
};

extern const device_type REDBARON;

