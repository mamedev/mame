#include "emu.h"
#include "sound/vlm5030.h"
#include "sound/sn76496.h"
#include "sound/msm5205.h"
#include "cpu/m6800/m6800.h"
#include "includes/trackfld.h"
#include "audio/trackfld.h"


#define TIMER_RATE (4096/4)

typedef struct _trackfld_audio_state trackfld_audio_state;
struct _trackfld_audio_state
{
	/* sound-related */
	int      m_last_addr;
	int      m_last_irq;

	cpu_device *m_audiocpu;
	device_t *m_vlm;
};


INLINE trackfld_audio_state *get_safe_token( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == TRACKFLD_AUDIO);

	return (trackfld_audio_state *)downcast<trackfld_audio_device *>(device)->token();
}



/*************************************
 *
 *  Initialization
 *
 *************************************/

static DEVICE_START( trackfld_audio )
{
	trackfld_audio_state *state = get_safe_token(device);

	state->m_audiocpu = device->machine().device<cpu_device>("audiocpu");
	state->m_vlm = device->machine().device("vlm");

	/* sound */
	device->save_item(NAME(state->m_last_addr));
	device->save_item(NAME(state->m_last_irq));
}

static DEVICE_RESET( trackfld_audio )
{
	trackfld_audio_state *state = get_safe_token(device);

	state->m_last_addr = 0;
	state->m_last_irq = 0;
}



/* The timer port on TnF and HyperSports sound hardware is derived from
   a 14.318 MHz clock crystal which is passed  through a couple of 74ls393
    ripple counters.
    Various outputs of the ripper counters clock the various chips.
    The Z80 uses 14.318 MHz / 4 (3.4MHz)
    The SN chip uses 14.318 MHz / 8 (1.7MHz)
    And the timer is connected to 14.318 MHz / 4096
    As we are using the Z80 clockrate as a base value we need to multiply
    the no of cycles by 4 to undo the 14.318/4 operation
*/

READ8_HANDLER( trackfld_sh_timer_r )
{
	UINT32 clock = space->machine().device<cpu_device>("audiocpu")->total_cycles() / TIMER_RATE;

	return clock & 0xF;
}

READ8_DEVICE_HANDLER( trackfld_speech_r )
{
	return vlm5030_bsy(device) ? 0x10 : 0;
}

WRITE8_DEVICE_HANDLER( trackfld_sound_w )
{
	device_t *audio = device->machine().device("trackfld_audio");
	trackfld_audio_state *state = get_safe_token(audio);
	int changes = offset ^ state->m_last_addr;

	/* A7 = data enable for VLM5030 (don't care )          */
	/* A8 = STA pin (1->0 data data  , 0->1 start speech   */
	/* A9 = RST pin 1=reset                                */

	/* A8 VLM5030 ST pin */
	if (changes & 0x100)
		vlm5030_st(device, offset & 0x100);

	/* A9 VLM5030 RST pin */
	if (changes & 0x200)
		vlm5030_rst(device, offset & 0x200);

	state->m_last_addr = offset;
}

READ8_HANDLER( hyperspt_sh_timer_r )
{
	device_t *audio = space->machine().device("trackfld_audio");
	trackfld_audio_state *state = get_safe_token(audio);
	UINT32 clock = state->m_audiocpu->total_cycles() / TIMER_RATE;

	if (state->m_vlm != NULL)
		return (clock & 0x3) | (vlm5030_bsy(state->m_vlm) ? 0x04 : 0);
	else
		return (clock & 0x3);
}

WRITE8_DEVICE_HANDLER( hyperspt_sound_w )
{
	device_t *audio = device->machine().device("trackfld_audio");
	trackfld_audio_state *state = get_safe_token(audio);
	int changes = offset ^ state->m_last_addr;

	/* A3 = data enable for VLM5030 (don't care )          */
	/* A4 = STA pin (1->0 data data  , 0->1 start speech   */
	/* A5 = RST pin 1=reset                                */
	/* A6 = VLM5030    output disable (don't care ) */
	/* A7 = kONAMI DAC output disable (don't care ) */
	/* A8 = SN76489    output disable (don't care ) */

	/* A4 VLM5030 ST pin */
	if (changes & 0x10)
		vlm5030_st(device, offset & 0x10);

	/* A5 VLM5030 RST pin */
	if( changes & 0x20 )
		vlm5030_rst(device, offset & 0x20);

	state->m_last_addr = offset;
}



WRITE8_HANDLER( konami_sh_irqtrigger_w )
{
	device_t *audio = space->machine().device("trackfld_audio");
	trackfld_audio_state *state = get_safe_token(audio);
	if (state->m_last_irq == 0 && data)
	{
		/* setting bit 0 low then high triggers IRQ on the sound CPU */
		device_set_input_line_and_vector(state->m_audiocpu, 0, HOLD_LINE, 0xff);
	}

	state->m_last_irq = data;
}




/*****************************************************************************
    DEVICE DEFINITION
*****************************************************************************/

DEVICE_GET_INFO(trackfld_audio)
{
 switch (state)
 {
  case DEVINFO_INT_TOKEN_BYTES: info->i = sizeof(trackfld_audio_state); break;

  case DEVINFO_FCT_START: info->start = DEVICE_START_NAME(trackfld_audio); break;

  case DEVINFO_FCT_RESET: info->reset = DEVICE_RESET_NAME(trackfld_audio); break;

  case DEVINFO_STR_NAME: strcpy(info->s, "Track And Field Audio"); break;
 }
}

const device_type TRACKFLD_AUDIO = &device_creator<trackfld_audio_device>;

trackfld_audio_device::trackfld_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TRACKFLD_AUDIO, "Track And Field Audio", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(trackfld_audio_state));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void trackfld_audio_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void trackfld_audio_device::device_start()
{
	DEVICE_START_NAME( trackfld_audio )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void trackfld_audio_device::device_reset()
{
	DEVICE_RESET_NAME( trackfld_audio )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void trackfld_audio_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


