/*

    SunA 8 Bit Games samples

    Format: PCM unsigned 8 bit mono 4Khz

*/

#include "emu.h"
#include "sound/samples.h"
#include "includes/suna8.h"


WRITE8_DEVICE_HANDLER( suna8_play_samples_w )
{
	suna8_state *state = device->machine().driver_data<suna8_state>();
	if( data )
	{
		samples_device *samples = downcast<samples_device *>(device);
		if( ~data & 0x10 )
		{
			samples->start_raw(0, &state->m_samplebuf[0x800*state->m_sample], 0x0800, 4000);
		}
		else if( ~data & 0x08 )
		{
			state->m_sample &= 3;
			samples->start_raw(0, &state->m_samplebuf[0x800*(state->m_sample+7)], 0x0800, 4000);
		}
	}
}

WRITE8_DEVICE_HANDLER( rranger_play_samples_w )
{
	suna8_state *state = device->machine().driver_data<suna8_state>();
	if( data )
	{
		if(( state->m_sample != 0 ) && ( ~data & 0x30 ))	// don't play state->m_sample zero when the bit is active
		{
			samples_device *samples = downcast<samples_device *>(device);
			samples->start_raw(0, &state->m_samplebuf[0x800*state->m_sample], 0x0800, 4000);
		}
	}
}

WRITE8_DEVICE_HANDLER( suna8_samples_number_w )
{
	suna8_state *state = device->machine().driver_data<suna8_state>();
	state->m_sample = data & 0xf;
}

SAMPLES_START( suna8_sh_start )
{
	suna8_state *state = device.machine().driver_data<suna8_state>();
	running_machine &machine = device.machine();
	int i, len = machine.root_device().memregion("samples")->bytes();
	UINT8 *ROM = state->memregion("samples")->base();

	state->m_samplebuf = auto_alloc_array(machine, INT16, len);

	for(i=0;i<len;i++)
		state->m_samplebuf[i] = (INT8)(ROM[i] ^ 0x80) * 256;
}
