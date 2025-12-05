// license:GPL-2.0+
// copyright-holders:Jarek Burczynski, Hiromitsu Shioya, Hannes Janetzek
// (Based on MSM5232 device)
//
//    Hohner MEG sound generator
//
// TODO
// - Check hc55516 for integrator and clock handling
// - Use the actual 'next VOICE pointer' linked-list
// - Some unknown flags probably used for skipping unused SLOTs

#include "emu.h"

#include "honmeg.h"

#define LOG_READ(...) // logerror(__VA_ARGS__)
#define LOG_WRITE(...) // logerror(__VA_ARGS__)

/* save output as raw 16-bit sample */
//#define SAVE_SAMPLE
//#define SAVE_SEPARATE_CHANNELS

#if defined SAVE_SAMPLE || defined SAVE_SEPARATE_CHANNELS
static FILE *sample[9];
#endif

#define CLOCK_RATE_DIVIDER 16

// default address map
void honmeg_device::wave_memory(address_map& map)
{
    if (!has_configured_map(0))
    {
        map(0x000, 0x9ff).ram();
    }
}

DEFINE_DEVICE_TYPE(HONMEG, honmeg_device, "honmeg", "HONMEG")

honmeg_device::honmeg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, HONMEG, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, device_sound_interface(mconfig, *this)
	, m_space_config("meg_data", ENDIANNESS_LITTLE, 8, 12, 0,
                     address_map_constructor(FUNC(honmeg_device::wave_memory), this))
	, m_stream(nullptr)
    , m_chip_clock(0)
{
}

//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector honmeg_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
int _meg_read_patch = 0;

void honmeg_device::device_start()
{
    _meg_read_patch = 0;
    
	int stream_rate = 44100; //clock()/CLOCK_RATE_DIVIDER;

	init(clock(), stream_rate);

	m_stream = stream_alloc(0, 8, stream_rate);
    
    m_counter = 0;

    // FIXME
    m_ticks_per_sample = clock() / stream_rate;
    logerror("rate: %d clock:%d ticks_per_sample:%d\n", stream_rate, clock(), m_ticks_per_sample);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void honmeg_device::device_reset()
{
}

//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void honmeg_device::device_stop()
{
#ifdef SAVE_SAMPLE
	fclose(sample[8]);
#endif
#ifdef SAVE_SEPARATE_CHANNELS
	fclose(sample[0]);
	fclose(sample[1]);
	fclose(sample[2]);
	fclose(sample[3]);
	fclose(sample[4]);
	fclose(sample[5]);
	fclose(sample[6]);
	fclose(sample[7]);
#endif
}


#define STEP_SH (16)    /* step calculations accuracy */

void honmeg_device::init_tables()
{
 	// sample rate = chip clock !!!  But :
    // TODO calcuate for MEG
	// highest possible frequency is chipclock/13/16 (pitch data=0x57)
	// at 2MHz : 2000000/13/16 = 9615 Hz

	// m_UpdateStep = int(double(1 << STEP_SH) * double(m_rate) / double(m_chip_clock));
	//logerror("clock=%i Hz rate=%i Hz, UpdateStep=%i\n", m_chip_clock, m_rate, m_UpdateStep);

#ifdef SAVE_SAMPLE
	sample[8]=fopen("sampsum.pcm","wb");
#endif
#ifdef SAVE_SEPARATE_CHANNELS
	sample[0]=fopen("samp0.pcm","wb");
	sample[1]=fopen("samp1.pcm","wb");
	sample[2]=fopen("samp2.pcm","wb");
	sample[3]=fopen("samp3.pcm","wb");
	sample[4]=fopen("samp4.pcm","wb");
	sample[5]=fopen("samp5.pcm","wb");
	sample[6]=fopen("samp6.pcm","wb");
	sample[7]=fopen("samp7.pcm","wb");
#endif
}

void honmeg_device::init(int clock, int rate)
{
	m_chip_clock = clock;
	m_rate  = rate ? rate : 44100;  /* avoid division by 0 */

	for (int j=0; j<32; j++)
	{
		memset(&m_voi[j],0,sizeof(VOICE));
	}

    init_tables();
}

uint8_t honmeg_device::read(offs_t offset)
{
    int voice_idx = (offset & 0xf8) >> 3;
    int slot_idx = (offset & 0x07);
    LOG_READ("(%02u,%02u) ", voice_idx, slot_idx);
    
    const VOICE *voice = &m_voi[voice_idx];

    uint8_t kind = (offset & 0xf00) >> 8;
    uint8_t data = 0xff;
    
    switch (kind) {
    case 0: {
      if (slot_idx == 0) {
        data = voice->next_voice;
        LOG_READ("<- sg [ptr] %03X (%02u) => %02X (%02u)\n", offset, (offset & 0xff), data, data);
      } else if (slot_idx == 1) {
        data = voice->pitch_1;
        LOG_READ("<- sg [time0] %03X (%u) => %02X (%02u)\n", offset, kind, data, data);
      } else if (slot_idx == 2) {
        data = voice->pitch_2;
        LOG_READ("<- sg [time1] %03X (%u) => %02X (%02u)\n", offset, kind, data, data);
      } else if (slot_idx == 3) {
        data = voice->phase;
        LOG_READ("<- sg [phase] %03X (%u) => %02X (%02u)\n", offset, kind, data, data);
      } else {
        data = voice->slot[slot_idx].pointer;
        LOG_READ("<- sg [ptr err] %03X (%02u) => %02X (%02u)\n", offset, kind, data, data);
      }
      break;
    }
    case 1: {
      data = voice->slot[slot_idx].octave | (voice->slot[slot_idx].unused << 4);
      if (slot_idx < 6) {
        LOG_READ("<- sg [oct|???] %03X (%u) => oct:%02X ???:%02X\n", offset, kind,
                 voice->slot[slot_idx].octave, voice->slot[slot_idx].unused);
      } else {
        LOG_READ("<- sg [oct|??? err] %03X (%u) => oct:%02X ???:%02X\n", offset, kind,
                 voice->slot[slot_idx].octave, voice->slot[slot_idx].unused);
      }
      break;
    }
    case 2: {
      data = voice->slot[slot_idx].sample | (voice->slot[slot_idx].output << 4);
      if (slot_idx < 6) {
        LOG_READ("<- sg [smp|out] %03X (%u) => smp:%02X out:%02X\n", offset, kind,
                 voice->slot[slot_idx].sample, voice->slot[slot_idx].output);
      } else {
        LOG_READ("<- sg [smp|out err] %03X (%u) => smp:%02X out:%02X\n", offset, kind,
                 voice->slot[slot_idx].sample, voice->slot[slot_idx].output);
      }
      break;
    }
    case 3: {
      // PATCH FOR VOX5 TEST
      if (_meg_read_patch == 0) {
          LOG_READ("<- sg [amp] %03X (%02u) <= patch 0 0x55\n", offset, kind);
          data = 0x55;
          _meg_read_patch++;
      } else if (_meg_read_patch == 1) {
          LOG_READ("<- sg [amp] %03X (%02u) <= patch 1 0x55\n", offset, kind);
          _meg_read_patch++;
          data = 0x55;
      } else if (_meg_read_patch == 2) {
          LOG_READ("<- sg [amp] %03X (%02u) <= patch 2 0xAA\n", offset, kind);
          _meg_read_patch++;
          data = 0xAA;
      }
      // REAL FUNCTION
      else  if (slot_idx < 6) {
          data = voice->slot[slot_idx].amplitude;
          LOG_READ("<- sg [amp] %03X (%u) => %02X (%02u)\n", offset, kind, data, data);
      } else {
          data = voice->slot[slot_idx].amplitude;
          LOG_READ("<- sg [amp err] %03X (%u) => %02X (%02u)\n", offset, kind, data, data);
      }
        break;
    }
    case 4: {
        LOG_READ("<- sg [bnk] %03X (%u) => %02X (%02u)\n", offset, offset, data, data);
        break;
    }
    case 5: {
        LOG_READ("<- sg [snd] %03X (%u) => %02X (%02u)\n", offset, offset, data, data);
        break;
    }
    default:
        LOG_READ("<- sg [error] %03X (%u) => %02X (%02u)\n", offset, offset, data, data);
        abort();
        break;
    } 
    return data;
}

void honmeg_device::write(offs_t offset, uint8_t data)
{
	m_stream->update ();
    
    int voice_idx = (offset & 0xf8) >> 3;
    int slot_idx = (offset & 0x07);
    
    VOICE *voice = &m_voi[voice_idx];

    uint8_t kind = (offset & 0xf00) >> 8;
    switch (kind) {
    case 0: {
      if (slot_idx == 0) {
        LOG_WRITE("-> %02u,%02u [ptr] %03X (%02u) => %02X (%02u)\n", voice_idx, slot_idx, offset, (offset & 0xff), data, data);
        voice->next_voice = data;
      } else if (slot_idx == 1) {
        LOG_WRITE("-> %02u,%02u [time0] %03X (%u) => %02X (%02u)\n", voice_idx, slot_idx, offset, kind, data, data);
        voice->pitch_1 = data;
        //voice->TG_count_period = (voice->pitch_1 | (voice->pitch_2 << 8));
      } else if (slot_idx == 2) {
        LOG_WRITE("-> %02u,%02u [time1] %03X (%u) => %02X (%02u)\n", voice_idx, slot_idx, offset, kind, data, data);
        voice->pitch_2 = data;
        //voice->TG_count_period = (voice->pitch_1 | (voice->pitch_2 << 8)) & 0xfff;
      } else if (slot_idx == 3) {
        LOG_WRITE("-> %02u,%02u [phase] %03X (%u) => %02X (%02u)\n", voice_idx, slot_idx, offset, kind, data, data);
        voice->phase = data;
      } else {
        LOG_WRITE("-> %02u,%02u [ptr err] %03X (%u) => %02X (%02u)\n", voice_idx, slot_idx, offset, kind, data, data);
        voice->slot[slot_idx].pointer = data;
      }
      break;
    }
    case 1: {
      if (slot_idx < 6) {
        LOG_WRITE("-> %02u,%02u [oct|???] %03X (%u) => oct:%02X ???:%02X\n", voice_idx, slot_idx, offset, kind, data & 0xf, data >> 4);
      } else {
        LOG_WRITE("-> %02u,%02u [oct|??? err] %03X (%u) => oct:%02X ???:%02X\n", voice_idx, slot_idx, offset, kind, data & 0xf, data >> 4);
      }
      voice->slot[slot_idx].octave = data & 0xf;
      voice->slot[slot_idx].unused = data >> 4;
      break;
    }
    case 2: {
      if (slot_idx < 6) {
          LOG_WRITE("-> %02u,%02u [out|smp] %03X (%u) => smp:%02X out:%02X\n", voice_idx, slot_idx, offset, kind, data & 0xf, data >> 4);
      } else {
          LOG_WRITE("-> %02u,%02u [out|smp err] %03X (%u) => smp:%02X out:%02X\n", voice_idx, slot_idx, offset, kind, data & 0xf, data >> 4);
      }
      voice->slot[slot_idx].sample = data & 0xf;
      voice->slot[slot_idx].output = data >> 4;
      break;
    }
    case 3: {
      if (slot_idx < 6) {
          //LOG_WRITE("-> %02u,%02u [amp] %03X (%u) => %02X (%02u)\n", voice_idx, slot_idx, offset, kind, data, data);
          voice->slot[slot_idx].amplitude = data;
      } else {
          LOG_WRITE("-> %02u,%02u [amp err] %03X (%u) => %02X (%02u)\n", voice_idx, slot_idx, offset, kind, data, data);
          voice->slot[slot_idx].amplitude = data;
      }
      break;
    }
    case 4: {
        LOG_WRITE("-> %02u,%02u [bnk] %03X (%u) => %02X (%02u)\n", voice_idx, slot_idx, offset, kind, data, data);
        m_sample_wr_latch = data & 0xf;
        break;
    }
    case 5: {
        LOG_WRITE("-> %02u,%02u [snd] %03X (%u) => %02X (%02u)\n", voice_idx, slot_idx, offset, kind, data, data);
        space().write_byte((m_sample_wr_latch << 8) + (offset & 0xff), data);
        break;
    }
    default:
        LOG_WRITE("-> %02u,%02u [error] %03X (%u) => %02X (%02u)\n", voice_idx, slot_idx, offset, kind, data, data);
        abort();
        break;
  } 
}

/* macro saves feet data to mono file */
#ifdef SAVE_SEPARATE_CHANNELS
	#define SAVE_SINGLE_CHANNEL(j,val) \
	{   signed int pom= val; \
	if (pom > 32767) pom = 32767; else if (pom < -32768) pom = -32768; \
	fputc((unsigned short)pom&0xff,sample[j]); \
	fputc(((unsigned short)pom>>8)&0xff,sample[j]);  }
#else
	#define SAVE_SINGLE_CHANNEL(j,val)
#endif

/* first macro saves all 8 feet outputs to mixed (mono) file */
/* second macro saves one group into left and the other in right channel */
#if 1   /*MONO*/
	#ifdef SAVE_SAMPLE
		#define SAVE_ALL_CHANNELS \
		{   signed int pom = buf1[i] + buf2[i]; \
		fputc((unsigned short)pom&0xff,sample[8]); \
		fputc(((unsigned short)pom>>8)&0xff,sample[8]); \
		}
	#else
		#define SAVE_ALL_CHANNELS
	#endif
#else   /*STEREO*/
	#ifdef SAVE_SAMPLE
		#define SAVE_ALL_CHANNELS \
		{   signed int pom = buf1[i]; \
		fputc((unsigned short)pom&0xff,sample[8]); \
		fputc(((unsigned short)pom>>8)&0xff,sample[8]); \
		pom = buf2[i]; \
		fputc((unsigned short)pom&0xff,sample[8]); \
		fputc(((unsigned short)pom>>8)&0xff,sample[8]); \
		}
	#else
		#define SAVE_ALL_CHANNELS
	#endif
#endif


/* MAME Interface */
void honmeg_device::device_post_load()
{
	init_tables();
}

void honmeg_device::set_clock(int clock)
{
	if (m_chip_clock != clock)
	{
		m_stream->update ();
		m_chip_clock = clock;
		m_rate = clock/CLOCK_RATE_DIVIDER;
		init_tables();
		m_stream->set_sample_rate(m_rate);
	}
}

uint16_t decode_sample_address(uint8_t sample, uint8_t octave, uint8_t phase) {
    // A8-A11
    uint16_t sample_offset = (sample & 0xf) << 8;
    // A1-A7
    uint8_t octave_offset = 1 << (8 - octave);
    // A0-A6
    uint8_t phase_mask = 0xff >> (octave - 1);

    uint16_t address = (phase & phase_mask) | octave_offset;

    return sample_offset | address; 
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void honmeg_device::sound_stream_update(sound_stream& stream,
                                        std::vector<read_stream_view> const& inputs,
                                        std::vector<write_stream_view>& outputs)
{
    for (int i = 0; i < outputs[0].samples(); i++)
    {
        for (int j = 0; j < m_ticks_per_sample; j++)
        {
            if (m_counter >= honmeg_device::counter_advance)
            {
                m_counter -= honmeg_device::counter_advance;
            }
            else
            {
                m_counter = 4095 - m_counter;
            }

            for (size_t n = 0; n < 32; n++)
            {
                VOICE& voice = m_voi[n];
                if (((m_counter + voice.next_event) & 0xfff) >= 2048)
                {
                    uint32_t next = (voice.next_event + voice.count_period()) & 0xfff;

                    voice.next_event = next;
                    voice.phase++;

                    for (int s = 0; s < 6; s++)
                    {
                        VOICE::SLOT& slot = voice.slot[s];
                        if (slot.amplitude > 0)
                        {
                            auto sample = (slot.sample);
                            auto octave = (slot.octave & 0x07);
                            auto address = decode_sample_address(sample, octave, voice.phase);
                            auto out = space().read_byte(address);

                            integrators[slot.output & 0x7] += (out - 128) * (slot.amplitude * 0.2f);

                            // logerror("%d phase: %u sample:%d octave:%d address:%d
                            // out:%d\n", i, voice->TG_phase, sample, octave,
                            // address, out);
                        }
                    }
                }
                else
                {
                    if (n == 0 && voice.slot[0].amplitude > 0)
                    {
                        /// logerror("skip T0:%04d t:%04d TE:%04d t+TE:%04d/%04d\n",
                        //          voice->TG_count_period,
                        //          m_counter,
                        //          voice->TG_nextevent,
                        //          (m_counter + voice->TG_nextevent),
                        //          (m_counter + voice->TG_nextevent) & 0xfff);
                    }
                }
            }
        }

        for (int c = 0; c < 8; c++)
        {
            outputs[c].put_int(i, std::min(32767, std::max(-32767, static_cast<int>(integrators[c]))), 32768);
            integrators[c] *= 0.999;
        }
    }
}
