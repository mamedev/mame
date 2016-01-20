// license:BSD-3-Clause
// copyright-holders:James Wallace
// thanks-to:Martin Buchholz,Juergen Oppermann,Volker Hann, Jan-Ole Christian
/***************************************************************************

  Poly-Play
  (c) 1985 by VEB Polytechnik Karl-Marx-Stadt

  TODO:
  There appears to be additional sound banking system in PORTB of the PIO,
  not emulated here due to lack of documentation.
***************************************************************************/

#include "emu.h"
#include "sound/samples.h"
#include "includes/polyplay.h"


SAMPLES_START_CB_MEMBER(polyplay_state::sh_start)
{
	int i;
	//Generate a simple square wave
	for (i = 0; i < SAMPLE_LENGTH / 2; i++) {
		m_backgroundwave[i] = + 0x4000;
	}
	for (i = SAMPLE_LENGTH / 2; i < SAMPLE_LENGTH; i++) {
		m_backgroundwave[i] = - 0x4000;
	}
	for (i = 0; i < 2; i++)
	{
		m_freq[i] = 110;
	}
}


void polyplay_state::process_channel(int channel, int data)
{
	if (m_channel_const[channel]) 
	{
		if (data > 1)
		{	
			play_channel(channel, (data*m_prescale[channel]));
		}
		m_channel_const[channel] = 0;
	}
	else
	{
		m_prescale[channel] = (data & 0x20) ? 16 : 1;

		if (data & 0x04) 
		{
			m_channel_const[channel] = 1;
		}
		if ((data == 0x41) || (data == 0x65) || (data == 0x45)) {
			play_channel(channel,0);
		}		
	}
}

void polyplay_state::play_channel(int channel, int data)
{
	if (data) {
		m_freq[channel] = (POLYPLAY_MAIN_CLOCK/4) / 16 / data / 8;
		m_samples->start_raw(channel, m_backgroundwave, SAMPLE_LENGTH, (SAMPLE_LENGTH*2)*m_freq[channel],true);
	}
	else {
		m_samples->stop(channel);
	}
}
