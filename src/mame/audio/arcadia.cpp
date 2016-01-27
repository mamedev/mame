// license:GPL-2.0+
// copyright-holders:David Viens, Peter Trauner
/***************************************************************************

  PeT mess@utanet.at
  main part in video/

  Refined with recording/analysis on MPT-03 (PAL UVI chip) by plgDavid

  NTSC UVI sound clock: 15734Hz (arcadia)
  PAL  UVI sound clock: 15625Hz (Soundic MPT-03 - owned by plgDavid)
***************************************************************************/


#include "includes/arcadia.h"

//known UVI audio clocks
#define UVI_NTSC 15734
#define UVI_PAL  15625

/* we need to create pulse transitions that sound 'decent'
   with the current mess/mame interp scheme

  this is not needed anymore with the new trick in streams.c
*/

#define OSAMP  1

//lfsr is 9 bits long (and same as Atari TIA pure noise)
#define LFSR_MASK (1<<8)

//makes alien invaders samples noise sync.
#define LFSR_INIT 0x00f0

//lfsr states at resynch borders
//0x01c1
//0x01e0
//0x00f0  //good synch
//0x0178
//0x01bc


// device type definition
const device_type ARCADIA_SOUND = &device_creator<arcadia_sound_device>;

//-------------------------------------------------
//  arcadia_sound_device - constructor
//-------------------------------------------------

arcadia_sound_device::arcadia_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ARCADIA_SOUND, "Arcadia Audio Custom", tag, owner, clock, "arcadia_sound", __FILE__),
		device_sound_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void arcadia_sound_device::device_start()
{
	m_channel = machine().sound().stream_alloc(*this, 0, 1, UVI_PAL*OSAMP);
	m_lfsr    = LFSR_INIT;
	m_tval    = 1;
	logerror("arcadia_sound start\n");
}

//-------------------------------------------------
//  device_start - device-specific reset
//-------------------------------------------------
void arcadia_sound_device::device_reset()
{
	memset(m_reg, 0, sizeof(m_reg));
	m_omode = 0;
	m_pos = 0;
}

//-------------------------------------------------
//  sound_stream_update - handle update requests for
//  our sound stream
//-------------------------------------------------

void arcadia_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	int i;
	stream_sample_t *buffer = outputs[0];

	for (i = 0; i < samples; i++, buffer++)
	{
		*buffer = 0;

		//if minimal pitch ?
		if (m_reg[1]){
			switch (m_mode){
				//dont play anything
				case 0:break;

				//tone only
				case 1:
					*buffer = m_volume * m_tval;
				break;

				//noise only
				case 2:
					*buffer = m_volume * m_nval;
				break;

				//tone AND noise (bitwise and)
				case 3:
					*buffer = m_volume * (m_tval & m_nval);
				break;
			}

			//counter
			m_pos++;

			if (m_pos >= m_size){
				//calculate new noise bit ( taps: 0000T000T)
				unsigned char newBit = m_lfsr & 1;         //first tap
				newBit = (newBit ^ ((m_lfsr & 0x10)?1:0) );//xor with second tap

				m_nval = m_lfsr & 1; //taking new output from LSB
				m_lfsr = m_lfsr >> 1;//shifting

				//insert new bit at end position (size-1) (only if non null)
				if (newBit)
					m_lfsr |= LFSR_MASK;

				//invert tone
				m_tval = !m_tval;

				m_pos = 0;
			}
		}
	}
}



//-------------------------------------------------
//  soundport_w
//-------------------------------------------------

WRITE8_MEMBER(arcadia_sound_device::write)
{
	m_channel->update();
	m_reg[offset] = data;

	//logerror("arcadia_sound write:%x=%x\n",offset,data);

	switch (offset)
	{
		case 1:
			//as per Gobbler samples:
			//the freq counter is only applied on the next change in the flip flop
			m_size = (data & 0x7f)*OSAMP;
			//logerror("arcadia_sound write: frq:%d\n",data);

			//reset LFSR
			if(!m_size)
				m_lfsr = LFSR_INIT;
		break;

		case 2:
			m_volume = (data & 0x07) * 0x800;
			m_mode   = (data & 0x18) >> 3;

			//logerror("arcadia_sound write: vol:%d mode:%d\n",m_volume,m_mode );

			if (m_mode != m_omode){
				//not 100% sure about this, maybe we should not reset anything
				//m_pos  = 0;
				m_tval = 0;
			}
			m_omode = m_mode;
		break;
	}
}
