/***************************************************************************

    okim9810.h

    OKI MSM9810 ADCPM(2) sound chip.

***************************************************************************/

#include "emu.h"
#include "okim9810.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// devices
const device_type OKIM9810 = okim9810_device_config::static_alloc_device_config;


// volume lookup table. The manual lists a full 16 steps, 2dB per step. 
// Given the dB values, that seems to map to a 7-bit volume control.
const UINT8 okim9810_device::s_volume_table[16] =
{
	0x80,	//  0 dB
	0x65,	// -2 dB
	0x50,	// -4 dB
	0x40,	// -6 dB
	0x32,	// -8.0 dB
	0x28,	// -10.5 dB
	0x20,	// -12.0 dB
	0x19,	// -14.5 dB
	0x14,	// -16.0 dB
	0x10,	// -18.0 dB
	0x0c,	// -20.0 dB
	0x0a,	// -22.0 dB
	0x08,	// -24.0 dB
	0x06,	// -26.0 dB
	0x05,	// -28.0 dB
	0x04,	// -30.0 dB
};

// default address map
static ADDRESS_MAP_START( okim9810, 0, 8 )
    AM_RANGE(0x000000, 0xffffff) AM_ROM
ADDRESS_MAP_END


//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  okim9810_device_config - constructor
//-------------------------------------------------

okim9810_device_config::okim9810_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
	: device_config(mconfig, static_alloc_device_config, "OKI9810", tag, owner, clock),
	  device_config_sound_interface(mconfig, *this),
	  device_config_memory_interface(mconfig, *this),
	  m_space_config("samples", ENDIANNESS_BIG, 8, 24, 0, NULL, *ADDRESS_MAP_NAME(okim9810))
{
}


//-------------------------------------------------
//  static_alloc_device_config - allocate a new
//  configuration object
//-------------------------------------------------

device_config *okim9810_device_config::static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
{
	return global_alloc(okim9810_device_config(mconfig, tag, owner, clock));
}


//-------------------------------------------------
//  alloc_device - allocate a new device object
//-------------------------------------------------

device_t *okim9810_device_config::alloc_device(running_machine &machine) const
{
	return auto_alloc(&machine, okim9810_device(machine, *this));
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *okim9810_device_config::memory_space_config(int spacenum) const
{
	return (spacenum == 0) ? &m_space_config : NULL;
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  okim9810_device - constructor
//-------------------------------------------------

okim9810_device::okim9810_device(running_machine &_machine, const okim9810_device_config &config)
	: device_t(_machine, config),
	  device_sound_interface(_machine, config, *this),
	  device_memory_interface(_machine, config, *this),
	  m_config(config),
	  m_stream(NULL),
	  m_TMP_register(0x00)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void okim9810_device::device_start()
{
	// find our direct access
	m_direct = &space()->direct();

	// create the stream
	//int divisor = m_config.m_pin7 ? 132 : 165;
	m_stream = m_machine.sound().stream_alloc(*this, 0, 1, clock());

    // save state stuff
    // m_TMP_register
    // m_voice
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void okim9810_device::device_reset()
{
	m_stream->update();
	for (int voicenum = 0; voicenum < OKIM9810_VOICES; voicenum++)
		m_voice[voicenum].m_playing = false;
}


//-------------------------------------------------
//  device_post_load - device-specific post-load
//-------------------------------------------------

void okim9810_device::device_post_load()
{
}


//-------------------------------------------------
//  device_clock_changed - called if the clock
//  changes
//-------------------------------------------------

void okim9810_device::device_clock_changed()
{
}


//-------------------------------------------------
//  stream_generate - handle update requests for
//  our sound stream
//-------------------------------------------------

void okim9810_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// reset the output stream
	memset(outputs[0], 0, samples * sizeof(*outputs[0]));
    
	// iterate over voices and accumulate sample data
	for (int voicenum = 0; voicenum < OKIM9810_VOICES; voicenum++)
		m_voice[voicenum].generate_audio(*m_direct, outputs[0], samples);
}


//-------------------------------------------------
//  read_status - read the status register
//-------------------------------------------------

UINT8 okim9810_device::read_status()
{
    UINT8 result = 0x00;
	return result;
}


//-------------------------------------------------
//  read - memory interface for read
//-------------------------------------------------

READ8_MEMBER( okim9810_device::read )
{
	return read_status();
}


//-------------------------------------------------
//  write - memory interface for write
//-------------------------------------------------

// The command is written when the CMD pin is low
void okim9810_device::write_command(UINT8 data)
{
    const UINT8 cmd = (data & 0xf8) >> 3;
    const UINT8 channel = (data & 0x07);
    
    switch(cmd)
    {
        case 0x00:  // START
        {
            mame_printf_verbose("START channel mask %02x\n", m_TMP_register); 
            UINT8 channelMask = 0x01;
            for (int i = 0; i < OKIM9810_VOICES; i++, channelMask <<= 1)
            {
                if (channelMask & m_TMP_register)
                {
                    m_voice[i].m_playing = true;
                    mame_printf_verbose("\t\tPlaying channel %d: type %02x @ %08x for %d samples (looping=%d).\n", i,
                                        m_voice[i].m_startFlags,
                                        m_voice[i].m_base_offset,
                                        m_voice[i].m_count,
                                        m_voice[i].m_looping);
                }
            }
            break;
        }
        case 0x01:  // STOP
        {
            mame_printf_verbose("STOP  channel mask %02x\n", m_TMP_register);
            UINT8 channelMask = 0x01;
            for (int i = 0; i < OKIM9810_VOICES; i++, channelMask <<= 1)
            {
                if (channelMask & m_TMP_register)
                {
                    m_voice[i].m_playing = false;
                    mame_printf_verbose("\tChannel %d stopping.\n", i);
                }
            }
            break;
        }
        case 0x02:  // LOOP
        { 
            mame_printf_verbose("LOOP  channel mask %02x\n", m_TMP_register);
            UINT8 channelMask = 0x01;
            for (int i = 0; i < OKIM9810_VOICES; i++, channelMask <<= 1)
            {
                if (channelMask & m_TMP_register)
                {
                    m_voice[i].m_looping = true;
                    mame_printf_verbose("\tChannel %d looping.\n", i);
                }
                else
                {
                    m_voice[i].m_looping = false;
                    mame_printf_verbose("\tChannel %d done looping.\n", i);
                }
            }
            break;
        }
        case 0x03:  // OPT (options)
        {
            mame_printf_warning("OPT   complex data %02x\n", m_TMP_register);
            mame_printf_warning("MSM9810: UNIMPLEMENTED COMMAND!\n");
            break;
        }
        case 0x04:  // MUON (silence)
        {
            mame_printf_warning("MUON  channel %d length %02x\n", channel, m_TMP_register); 
            mame_printf_warning("MSM9810: UNIMPLEMENTED COMMAND!\n");
            break;
        }
        
        case 0x05:  // FADR (phrase address)
        {
            const offs_t base = m_TMP_register * 8;

            offs_t startAddr;
            const UINT8 startFlags = m_direct->read_raw_byte(base + 0);
            startAddr  = m_direct->read_raw_byte(base + 1) << 16;
            startAddr |= m_direct->read_raw_byte(base + 2) << 8;
            startAddr |= m_direct->read_raw_byte(base + 3) << 0;

            offs_t endAddr;
            const UINT8 endFlags = m_direct->read_raw_byte(base + 4);
            endAddr  = m_direct->read_raw_byte(base + 5) << 16;
            endAddr |= m_direct->read_raw_byte(base + 6) << 8;
            endAddr |= m_direct->read_raw_byte(base + 7) << 0;

            mame_printf_verbose("FADR  channel %d phrase offset %02x => ", channel, m_TMP_register);
            mame_printf_verbose("\tstartFlags(%02x) startAddr(%06x) endFlags(%02x) endAddr(%06x) bytes(%d)\n", startFlags, startAddr, endFlags, endAddr, endAddr-startAddr);
            m_voice[channel].m_sample = 0;
            m_voice[channel].m_startFlags = startFlags;
            m_voice[channel].m_base_offset = startAddr;
            m_voice[channel].m_endFlags = endFlags;
            m_voice[channel].m_playbackAlgo = (startFlags & 0x30) >> 4;     // Guess
            // debug printf("%02x %d\n", startFlags, m_voice[channel].m_playbackAlgo);
            // TODO: Sampling frequency is very likely : (startFlags & 0x0f)
            m_voice[channel].m_count = (endAddr-startAddr) + 1;
            if (m_voice[channel].m_playbackAlgo == OKIM9810_ADPCM_PLAYBACK || 
                m_voice[channel].m_playbackAlgo == OKIM9810_ADPCM2_PLAYBACK)
                m_voice[channel].m_count *= 2;
            else
                mame_printf_warning("UNIMPLEMENTED PLAYBACK METHOD %d\n", m_voice[channel].m_playbackAlgo);
            break;
        }

        case 0x06:  // DADR (direct address playback)
        {
            mame_printf_warning("DADR  channel %d complex data %02x\n", channel, m_TMP_register); 
            mame_printf_warning("MSM9810: UNIMPLEMENTED COMMAND!\n");
            break;
        }
        case 0x07:  // CVOL (channel volume)
        {
            mame_printf_verbose("CVOL  channel %d volume level %02x\n", channel, m_TMP_register); 
            mame_printf_verbose("\tChannel %d -> volume %d.\n", channel, s_volume_table[m_TMP_register & 0x0f]);

            m_voice[channel].m_volume = s_volume_table[m_TMP_register & 0x0f];
            break;
        }
        case 0x08:  // PAN
        {
            mame_printf_warning("PAN   channel %d volume level %02x\n", channel, m_TMP_register); 
            mame_printf_warning("MSM9810: UNIMPLEMENTED COMMAND!\n");
            break;
        }
        default: 
        {
            mame_printf_warning("MSM9810: UNKNOWN COMMAND!\n");
            break;
        }
    }
}

WRITE8_MEMBER( okim9810_device::write )
{
    write_command(data);
}


//-----------------------------------------------------------
//  writeTMP - memory interface for writing the TMP register
//-----------------------------------------------------------

// TMP is written when the CMD pin is high
void okim9810_device::write_TMP_register(UINT8 data)
{
    m_TMP_register = data;
}

WRITE8_MEMBER( okim9810_device::write_TMP_register )
{
	write_TMP_register(data);
}


//**************************************************************************
//  OKIM VOICE
//**************************************************************************

//-------------------------------------------------
//  okim_voice - constructor
//-------------------------------------------------

okim9810_device::okim_voice::okim_voice()
	: m_playbackAlgo(OKIM9810_ADPCM2_PLAYBACK),
      m_playing(false),
	  m_looping(false),
	  m_startFlags(0),
	  m_endFlags(0),
	  m_base_offset(0),
	  m_sample(0),
	  m_count(0),
	  m_volume(0)
{
}

//-------------------------------------------------
//  generate_audio - generate audio samples and
//  add them to an output stream
//-------------------------------------------------

void okim9810_device::okim_voice::generate_audio(direct_read_data &direct, stream_sample_t *buffer, int samples)
{
	// skip if not active
	if (!m_playing)
		return;

	// loop while we still have samples to generate
	while (samples-- != 0)
	{
		// fetch the next sample byte
		int nibble = direct.read_raw_byte(m_base_offset + m_sample / 2) >> (((m_sample & 1) << 2) ^ 4);

		// output to the buffer, scaling by the volume
		// signal in range -2048..2047, volume in range 2..128 => signal * volume / 8 in range -32768..32767
        switch (m_playbackAlgo)
        { 
            case OKIM9810_ADPCM_PLAYBACK:
        		*buffer++ += (INT32)m_adpcm.clock(nibble) * (INT32)m_volume / 8; 
                break;
            case OKIM9810_ADPCM2_PLAYBACK:
        		*buffer++ += (INT32)m_adpcm2.clock(nibble) * (INT32)m_volume / 8; 
                break;
            default:
                break;
        }
                    
		// next!
		if (++m_sample >= m_count)
		{
            if (!m_looping)
    			m_playing = false;
            else
                m_sample = 0;
			break;
		}
	}
}
