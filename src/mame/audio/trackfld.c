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
	int      SN76496_latch;
	int      last_addr;
	int      last_irq;

	cpu_device *audiocpu;
	running_device *vlm;
};


INLINE trackfld_audio_state *get_safe_token( running_device *device )
{
	assert(device != NULL);
	assert(device->type() == TRACKFLD_AUDIO);

	return (trackfld_audio_state *)downcast<legacy_device_base *>(device)->token();
}



/*************************************
 *
 *  Initialization
 *
 *************************************/

static DEVICE_START( trackfld_audio )
{
	trackfld_audio_state *state = get_safe_token(device);

	state->audiocpu = device->machine->device<cpu_device>("audiocpu");
	state->vlm = device->machine->device("vlm");

	/* sound */
	state_save_register_device_item(device, 0, state->SN76496_latch);
	state_save_register_device_item(device, 0, state->last_addr);
	state_save_register_device_item(device, 0, state->last_irq);
}

static DEVICE_RESET( trackfld_audio )
{
	trackfld_audio_state *state = get_safe_token(device);

	state->SN76496_latch = 0;
	state->last_addr = 0;
	state->last_irq = 0;
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
	UINT32 clock = space->machine->device<cpu_device>("audiocpu")->total_cycles() / TIMER_RATE;

	return clock & 0xF;
}

READ8_DEVICE_HANDLER( trackfld_speech_r )
{
	return vlm5030_bsy(device) ? 0x10 : 0;
}

WRITE8_DEVICE_HANDLER( trackfld_sound_w )
{
	running_device *audio = device->machine->device("trackfld_audio");
	trackfld_audio_state *state = get_safe_token(audio);
	int changes = offset ^ state->last_addr;

	/* A7 = data enable for VLM5030 (don't care )          */
	/* A8 = STA pin (1->0 data data  , 0->1 start speech   */
	/* A9 = RST pin 1=reset                                */

	/* A8 VLM5030 ST pin */
	if (changes & 0x100)
		vlm5030_st(device, offset & 0x100);

	/* A9 VLM5030 RST pin */
	if (changes & 0x200)
		vlm5030_rst(device, offset & 0x200);

	state->last_addr = offset;
}

READ8_HANDLER( hyperspt_sh_timer_r )
{
	running_device *audio = space->machine->device("trackfld_audio");
	trackfld_audio_state *state = get_safe_token(audio);
	UINT32 clock = state->audiocpu->total_cycles() / TIMER_RATE;

	if (state->vlm != NULL)
		return (clock & 0x3) | (vlm5030_bsy(state->vlm) ? 0x04 : 0);
	else
		return (clock & 0x3);
}

WRITE8_DEVICE_HANDLER( hyperspt_sound_w )
{
	running_device *audio = device->machine->device("trackfld_audio");
	trackfld_audio_state *state = get_safe_token(audio);
	int changes = offset ^ state->last_addr;

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

	state->last_addr = offset;
}



WRITE8_HANDLER( konami_sh_irqtrigger_w )
{
	running_device *audio = space->machine->device("trackfld_audio");
	trackfld_audio_state *state = get_safe_token(audio);
	if (state->last_irq == 0 && data)
	{
		/* setting bit 0 low then high triggers IRQ on the sound CPU */
		cpu_set_input_line_and_vector(state->audiocpu, 0, HOLD_LINE, 0xff);
	}

	state->last_irq = data;
}


WRITE8_HANDLER( konami_SN76496_latch_w )
{
	running_device *audio = space->machine->device("trackfld_audio");
	trackfld_audio_state *state = get_safe_token(audio);
	state->SN76496_latch = data;
}


WRITE8_DEVICE_HANDLER( konami_SN76496_w )
{
	running_device *audio = device->machine->device("trackfld_audio");
	trackfld_audio_state *state = get_safe_token(audio);
	sn76496_w(device, offset, state->SN76496_latch);
}

/*****************************************************************************
    DEVICE DEFINITION
*****************************************************************************/


static const char DEVTEMPLATE_SOURCE[] = __FILE__;

#define DEVTEMPLATE_ID(p,s)				p##trackfld_audio##s
#define DEVTEMPLATE_FEATURES			DT_HAS_START | DT_HAS_RESET
#define DEVTEMPLATE_NAME				"Track And Field Audio"
#define DEVTEMPLATE_FAMILY				"Track And Field Audio IC"
#include "devtempl.h"

DEFINE_LEGACY_SOUND_DEVICE(TRACKFLD_AUDIO, trackfld_audio);
