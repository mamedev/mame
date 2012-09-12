/***************************************************************************

IREM "M72" sound hardware

All games have a YM2151 for music, and most of them also samples. Samples
are not handled consistently by all the games, some use a high frequency NMI
handler to push them through a DAC, others use external hardware.
In the following table, the NMI column indicates with a No the games whose
NMI handler only consists of RETN. R-Type is an exception, it doesn't have
a valid NMI handler at all.

Game                                    Year  ID string     NMI
--------------------------------------  ----  ------------  ---
R-Type                                  1987  - (earlier version, no samples)
Battle Chopper / Mr. Heli               1987  Rev 2.20      Yes
Vigilante                               1988  Rev 2.20      Yes
Ninja Spirit                            1988  Rev 2.20      Yes
Image Fight                             1988  Rev 2.20      Yes
Legend of Hero Tonma                    1989  Rev 2.20      Yes
X Multiply                              1989  Rev 2.20      Yes
Dragon Breed                            1989  Rev 2.20      Yes
Kickle Cubicle                          1988  Rev 2.21      Yes
Shisensho                               1989  Rev 2.21      Yes
R-Type II                               1989  Rev 2.21      Yes
Major Title                             1990  Rev 2.21      Yes
Air Duel                                1990  Rev 3.14 M72   No
Daiku no Gensan                         1990  Rev 3.14 M81  Yes
Daiku no Gensan (M72)                   1990  Rev 3.15 M72   No
Hammerin' Harry                         1990  Rev 3.15 M81  Yes
Ken-Go                                  1991  Rev 3.15 M81  Yes
Pound for Pound                         1990  Rev 3.15 M83   No
Cosmic Cop                              1991  Rev 3.15 M81  Yes
Gallop - Armed Police Unit              1991  Rev 3.15 M72   No
Hasamu                                  1991  Rev 3.15 M81  Yes
Bomber Man                              1991  Rev 3.15 M81  Yes
Bomber Man World (Japan)                1992  Rev 3.31 M81  Yes
Bomber Man World (World) / Atomic Punk  1992  Rev 3.31 M99   No
Quiz F-1 1,2finish                      1992  Rev 3.33 M81  Yes
Risky Challenge                         1993  Rev 3.34 M81  Yes
Shisensho II                            1993  Rev 3.34 M81  Yes

***************************************************************************/

#include "emu.h"
#include "sound/dac.h"
#include "m72.h"


/*

  The sound CPU runs in interrup mode 0. IRQ is shared by two sources: the
  YM2151 (bit 4 of the vector), and the main CPU (bit 5).
  Since the vector can be changed from different contexts (the YM2151 timer
  callback, the main CPU context, and the sound CPU context), it's important
  to accurately arbitrate the changes to avoid out-of-order execution. We do
  that by handling all vector changes in a single timer callback.

*/


enum
{
	VECTOR_INIT,
	YM2151_ASSERT,
	YM2151_CLEAR,
	Z80_ASSERT,
	Z80_CLEAR
};

typedef struct _m72_audio_state m72_audio_state;
struct _m72_audio_state
{
	UINT8 irqvector;
	UINT32 sample_addr;
	UINT8 *samples;
	UINT32 samples_size;
	address_space *space;
	dac_device *dac;
};

INLINE m72_audio_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == M72);

	return (m72_audio_state *)downcast<m72_audio_device *>(device)->token();
}


static TIMER_CALLBACK( setvector_callback )
{
	m72_audio_state *state = (m72_audio_state *)ptr;

	switch(param)
	{
		case VECTOR_INIT:
			state->irqvector = 0xff;
			break;

		case YM2151_ASSERT:
			state->irqvector &= 0xef;
			break;

		case YM2151_CLEAR:
			state->irqvector |= 0x10;
			break;

		case Z80_ASSERT:
			state->irqvector &= 0xdf;
			break;

		case Z80_CLEAR:
			state->irqvector |= 0x20;
			break;
	}

	if (state->irqvector == 0)
		logerror("You didn't call m72_init_sound()\n");

	machine.device("soundcpu")->execute().set_input_line_and_vector(0, (state->irqvector == 0xff) ? CLEAR_LINE : ASSERT_LINE, state->irqvector);
}

static DEVICE_START( m72_audio )
{
	m72_audio_state *state = get_safe_token(device);

	state->samples = device->machine().root_device().memregion("samples")->base();
	state->samples_size = device->machine().root_device().memregion("samples")->bytes();
	state->space = device->machine().device("soundcpu")->memory().space(AS_IO);
	state->dac = device->machine().device<dac_device>("dac");

	device->save_item(NAME(state->irqvector));
	device->save_item(NAME(state->sample_addr));
}

static DEVICE_RESET( m72_audio )
{
	m72_audio_state *state = get_safe_token(device);

	setvector_callback(device->machine(), state, VECTOR_INIT);
}

void m72_ym2151_irq_handler(device_t *device, int irq)
{
	device_t *audio = device->machine().device("m72");
	m72_audio_state *state = get_safe_token(audio);

	device->machine().scheduler().synchronize(FUNC(setvector_callback), irq ? YM2151_ASSERT : YM2151_CLEAR, state);
}

WRITE16_DEVICE_HANDLER( m72_sound_command_w )
{
	if (ACCESSING_BITS_0_7)
	{
		m72_audio_state *state = get_safe_token(device);
		driver_device *drvstate = device->machine().driver_data<driver_device>();
		drvstate->soundlatch_byte_w(*state->space, offset, data);
		device->machine().scheduler().synchronize(FUNC(setvector_callback), Z80_ASSERT, state);
	}
}

WRITE8_DEVICE_HANDLER( m72_sound_command_byte_w )
{
	m72_audio_state *state = get_safe_token(device);
	driver_device *drvstate = device->machine().driver_data<driver_device>();
	drvstate->soundlatch_byte_w(*state->space, offset, data);
	device->machine().scheduler().synchronize(FUNC(setvector_callback), Z80_ASSERT, state);
}

WRITE8_DEVICE_HANDLER( m72_sound_irq_ack_w )
{
	m72_audio_state *state = get_safe_token(device);

	device->machine().scheduler().synchronize(FUNC(setvector_callback), Z80_CLEAR, state);
}



void m72_set_sample_start(device_t *device, int start)
{
	m72_audio_state *state = get_safe_token(device);

	state->sample_addr = start;
}

WRITE8_DEVICE_HANDLER( vigilant_sample_addr_w )
{
	m72_audio_state *state = get_safe_token(device);

	if (offset == 1)
		state->sample_addr = (state->sample_addr & 0x00ff) | ((data << 8) & 0xff00);
	else
		state->sample_addr = (state->sample_addr & 0xff00) | ((data << 0) & 0x00ff);
}

WRITE8_DEVICE_HANDLER( shisen_sample_addr_w )
{
	m72_audio_state *state = get_safe_token(device);

	state->sample_addr >>= 2;

	if (offset == 1)
		state->sample_addr = (state->sample_addr & 0x00ff) | ((data << 8) & 0xff00);
	else
		state->sample_addr = (state->sample_addr & 0xff00) | ((data << 0) & 0x00ff);

	state->sample_addr <<= 2;
}

WRITE8_DEVICE_HANDLER( rtype2_sample_addr_w )
{
	m72_audio_state *state = get_safe_token(device);

	state->sample_addr >>= 5;

	if (offset == 1)
		state->sample_addr = (state->sample_addr & 0x00ff) | ((data << 8) & 0xff00);
	else
		state->sample_addr = (state->sample_addr & 0xff00) | ((data << 0) & 0x00ff);

	state->sample_addr <<= 5;
}

WRITE8_DEVICE_HANDLER( poundfor_sample_addr_w )
{
	m72_audio_state *state = get_safe_token(device);

	/* poundfor writes both sample start and sample END - a first for Irem...
       we don't handle the end written here, 00 marks the sample end as usual. */
	if (offset > 1) return;

	state->sample_addr >>= 4;

	if (offset == 1)
		state->sample_addr = (state->sample_addr & 0x00ff) | ((data << 8) & 0xff00);
	else
		state->sample_addr = (state->sample_addr & 0xff00) | ((data << 0) & 0x00ff);

	state->sample_addr <<= 4;
}

READ8_DEVICE_HANDLER( m72_sample_r )
{
	m72_audio_state *state = get_safe_token(device);

	return state->samples[state->sample_addr];
}

WRITE8_DEVICE_HANDLER( m72_sample_w )
{
	m72_audio_state *state = get_safe_token(device);

	state->dac->write_signed8(data);
	state->sample_addr = (state->sample_addr + 1) & (state->samples_size - 1);
}

const device_type M72 = &device_creator<m72_audio_device>;

m72_audio_device::m72_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, M72, "M72 Custom", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(m72_audio_state));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void m72_audio_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void m72_audio_device::device_start()
{
	DEVICE_START_NAME( m72_audio )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void m72_audio_device::device_reset()
{
	DEVICE_RESET_NAME( m72_audio )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void m72_audio_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


