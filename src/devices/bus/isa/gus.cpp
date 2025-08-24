// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 *  Gravis Ultrasound ISA card
 *
 *  Started: 28/01/2012
 *
 *  to do: xref with lowsrc.doc from GUS SDK
 *  - 256K DMA and 16-bit sample playback boundaries
 */


#include "emu.h"
#include "gus.h"

#include "bus/midi/midi.h"
#include "machine/clock.h"
#include "speaker.h"


#define IRQ_2XF           0x00
#define IRQ_MIDI_TRANSMIT 0x01
#define IRQ_MIDI_RECEIVE  0x02
#define IRQ_TIMER1        0x04
#define IRQ_TIMER2        0x08
#define IRQ_SB            0x10
#define IRQ_WAVETABLE     0x20
#define IRQ_VOLUME_RAMP   0x40
#define IRQ_DRAM_TC_DMA   0x80

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// uncomment this to save wave RAM content to a file
//#define SAVE_WAVE_RAM 1
//#define LOG_SOUND 1

static const uint16_t volume_ramp_table[4] =
{
	1, 8, 64, 512
};

DEFINE_DEVICE_TYPE(GGF1,      gf1_device,       "gf1",     "Gravis GF1")
DEFINE_DEVICE_TYPE(ISA16_GUS, isa16_gus_device, "isa_gus", "Gravis Ultrasound")

#ifdef LOG_SOUND
FILE* f;
#endif

void gf1_device::update_irq()
{
	int txirq = calculate_txirq();

	if (m_txirq != txirq)
	{
		m_txirq = txirq;
		m_txirq_handler(!m_txirq);
	}

	int rxirq = calculate_rxirq();

	if (m_rxirq != rxirq)
	{
		m_rxirq = rxirq;
		m_rxirq_handler(!m_rxirq);
	}
}

/* only the Adlib timers are implemented in hardware */
uint8_t gf1_device::adlib_r(offs_t offset)
{
	uint8_t retVal = 0xff;
	switch(offset)
	{
		case 0:
//          if(m_timer_ctrl & 0x01)
				return m_adlib_status;
//          return m_fake_adlib_status;
		case 1:
			return m_adlib_data;
	}
	return retVal;
}

void gf1_device::adlib_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
		case 0:
			m_adlib_cmd = data;
			break;
		case 1:
			if(m_adlib_cmd == 0x04 && !(m_timer_ctrl & 0x01))
			{
				if(data & 0x80)
				{
					m_timer1_irq_handler(0);
					m_timer2_irq_handler(0);
					m_adlib_status &= ~0xe0;
					logerror("GUS: Timer flags reset\n");
				}
				else
				{
					if((data & 0x01) && !(data & 0x40))
					{
						m_adlib_timer1_enable = 1;
						m_timer1->adjust(attotime::zero,0,attotime::from_usec(80));
					}
					if((data & 0x02) && !(data & 0x20))
					{
						m_adlib_timer2_enable = 1;
						m_timer2->adjust(attotime::zero,0,attotime::from_usec(320));
					}
					if(!(data & 0x01) && !(data & 0x40))
					{
						m_adlib_timer1_enable = 0;
						m_timer1->reset();
					}
					if(!(data & 0x02) && !(data & 0x20))
					{
						m_adlib_timer2_enable = 0;
						m_timer2->reset();
					}
					logerror("GUS: Timer enable - %02x\n",data);
				}
				m_adlib_timer_cmd = data;
			}
			else
			{
				m_adlib_data = data;
				if(m_timer_ctrl & 0x02)
				{
					m_adlib_status |= 0x01;
					m_nmi_handler(1);
					logerror("GUS: 2X9 Timer triggered!\n");
				}
			}
			break;
	}
}

TIMER_CALLBACK_MEMBER(gf1_device::update_volume_ramps)
{
	int x;

	for(x=0;x<32;x++)
	{
		if(!(m_voice[x].vol_ramp_ctrl & 0x01))  // if ramping is enabled
		{
			m_voice[x].vol_count++;
			if(m_voice[x].vol_count % volume_ramp_table[(m_voice[x].vol_ramp_rate & 0xc0)>>6] == 0)
			{
				// increase/decrease volume
				if(m_voice[x].vol_ramp_ctrl & 0x40)
				{
					//m_voice[x].current_vol = (m_voice[x].current_vol & 0xf000) | ((m_voice[x].current_vol & 0x0ff0) + ((m_voice[x].vol_ramp_rate & 0x0f)<<8));
					m_voice[x].current_vol -= ((m_voice[x].vol_ramp_rate & 0x3f) << 4);
					if(m_voice[x].current_vol <= (m_voice[x].vol_ramp_start << 8))  // end of ramp?
					{
						if(m_voice[x].vol_ramp_ctrl & 0x08)
						{
							if(m_voice[x].vol_ramp_ctrl & 0x10)
							{
								m_voice[x].vol_ramp_ctrl &= ~0x40; // change direction and continue
								m_voice[x].current_vol = (m_voice[x].vol_ramp_start << 8);
							}
							else
								m_voice[x].current_vol = (m_voice[x].vol_ramp_end << 8);
						}
						else
						{
							m_voice[x].vol_ramp_ctrl |= 0x01;  // stop volume ramp
							m_voice[x].current_vol = (m_voice[x].vol_ramp_start << 8);
						}
						if(m_voice[x].vol_ramp_ctrl & 0x20)
							set_irq(IRQ_VOLUME_RAMP,x);
					}
				}
				else
				{
					//m_voice[x].current_vol = (m_voice[x].current_vol & 0xf000) | ((m_voice[x].current_vol & 0x0ff0) - ((m_voice[x].vol_ramp_rate & 0x0f)<<8));
					m_voice[x].current_vol += ((m_voice[x].vol_ramp_rate & 0x3f) << 4);
					if(m_voice[x].current_vol >= (m_voice[x].vol_ramp_end << 8))  // end of ramp?
					{
						if(m_voice[x].vol_ramp_ctrl & 0x08)
						{
							if(m_voice[x].vol_ramp_ctrl & 0x10)
							{
								m_voice[x].vol_ramp_ctrl |= 0x40; // change direction and continue
								m_voice[x].current_vol = (m_voice[x].vol_ramp_end << 8);
							}
							else
								m_voice[x].current_vol = (m_voice[x].vol_ramp_start << 8);
						}
						else
						{
							m_voice[x].vol_ramp_ctrl |= 0x01;  // stop volume ramp
							m_voice[x].current_vol = (m_voice[x].vol_ramp_end << 8);
						}
						if(m_voice[x].vol_ramp_ctrl & 0x20)
							set_irq(IRQ_VOLUME_RAMP,x);
					}
				}
			}
		}
	}
}

TIMER_CALLBACK_MEMBER(gf1_device::adlib_timer1_tick)
{
	if(m_adlib_timer1_enable != 0)
	{
		if(m_timer1_count == 0xff)
		{
			m_adlib_status |= 0xc0;
			m_timer1_count = m_timer1_value;
			if(m_timer_ctrl & 0x04)
				m_timer1_irq_handler(1);
		}
		m_timer1_count++;
	}
}

TIMER_CALLBACK_MEMBER(gf1_device::adlib_timer2_tick)
{
	if(m_adlib_timer2_enable != 0)
	{
		if(m_timer2_count == 0xff)
		{
			m_adlib_status |= 0xa0;
			m_timer2_count = m_timer2_value;
			if(m_timer_ctrl & 0x08)
				m_timer2_irq_handler(1);
		}
		m_timer2_count++;
	}
}

TIMER_CALLBACK_MEMBER(gf1_device::dma_tick)
{
	m_drq1_handler(1);
}

void gf1_device::sound_stream_update(sound_stream &stream)
{
	int x;

	for(x=0;x<32;x++)  // for each voice
	{
		uint16_t vol = (m_volume_table[(m_voice[x].current_vol & 0xfff0) >> 4]);
		for (int sampindex = 0; sampindex < stream.samples(); sampindex++)
		{
			uint32_t current = m_voice[x].current_addr >> 9;
			// TODO: implement proper panning
			stream.add_int(0, sampindex, m_voice[x].sample * vol, 32768 * 8192);
			stream.add_int(1, sampindex, m_voice[x].sample * vol, 32768 * 8192);
			if((!(m_voice[x].voice_ctrl & 0x40)) && (m_voice[x].current_addr >= m_voice[x].end_addr) && !m_voice[x].rollover && !(m_voice[x].voice_ctrl & 0x01))
			{
				if(m_voice[x].vol_ramp_ctrl & 0x04)
				{
					m_voice[x].rollover = true;  // set roll over condition - generate IRQ, but keep voice playing
				}

				if(m_voice[x].voice_ctrl & 0x20)
					set_irq(IRQ_WAVETABLE,x);

				// end voice, unless looping, or rollover is active, which disables looping
				if(!m_voice[x].rollover)
				{
					if(!(m_voice[x].voice_ctrl & 0x08))
					{
						m_voice[x].voice_ctrl |= 0x01;
//                        m_voice[x].current_addr = m_voice[x].end_addr;
					}
				}
				// looping is not supposed to happen when rollover is active, but the Windows drivers have other ideas...
				if(m_voice[x].voice_ctrl & 0x08)
				{
					if(m_voice[x].voice_ctrl & 0x10)
						m_voice[x].voice_ctrl |= 0x40; // change direction
					else
						m_voice[x].current_addr = m_voice[x].start_addr; // start sample again
				}
			}
			if((m_voice[x].voice_ctrl & 0x40) && (m_voice[x].current_addr <= m_voice[x].start_addr) && !m_voice[x].rollover && !(m_voice[x].voice_ctrl & 0x01))
			{
				if(m_voice[x].vol_ramp_ctrl & 0x04)
				{
					m_voice[x].rollover = true;  // set roll over condition - generate IRQ, but keep voice playing
				}

				if(m_voice[x].voice_ctrl & 0x20)
					set_irq(IRQ_WAVETABLE,x);

				// end voice, unless looping, or rollover is active, which disables looping
				if(!m_voice[x].rollover)
				{
					// end voice, unless looping
					if(!(m_voice[x].voice_ctrl & 0x08))
					{
						m_voice[x].voice_ctrl |= 0x01;
//                          m_voice[x].current_addr = m_voice[x].start_addr;
					}
				}
				// looping is not supposed to happen when rollover is active, but the Windows drivers have other ideas...
				if(m_voice[x].voice_ctrl & 0x08)
				{
					if(m_voice[x].voice_ctrl & 0x10)
						m_voice[x].voice_ctrl &= ~0x40; // change direction
					else
						m_voice[x].current_addr = m_voice[x].end_addr; // start sample again
				}
			}
			if(!(m_voice[x].voice_ctrl & 0x01))
			{
				if(m_voice[x].voice_ctrl & 0x04)
				{  // 16-bit PCM
					current = ((m_voice[x].current_addr >> 9) & 0xc0000) + (((m_voice[x].current_addr >> 9) & 0x1ffff) << 1);
					m_voice[x].sample = (int16_t)((m_wave_ram[current & 0xffffe]) | ((m_wave_ram[(current & 0xffffe)+1])<<8));
				}
				else
				{  // 8-bit PCM
					m_voice[x].sample = (int16_t)(m_wave_ram[current & 0xfffff] << 8);
				}
				if(m_voice[x].voice_ctrl & 0x40)  // voice direction
					m_voice[x].current_addr -= (m_voice[x].freq_ctrl >> 1);
				else
					m_voice[x].current_addr += (m_voice[x].freq_ctrl >> 1);
			}
#ifdef LOG_SOUND
			int16_t smp = (m_voice[x].sample) * (vol / 8192.0);
			fwrite(&smp,4,1,f);
#endif
		}
	}
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  gf1_device - constructor
//-------------------------------------------------

gf1_device::gf1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	acia6850_device(mconfig, GGF1, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	m_dma_dram_ctrl(0), m_dma_start_addr(0), m_dram_addr(0), m_timer_ctrl(0), m_timer1_count(0), m_timer2_count(0), m_timer1_value(0), m_timer2_value(0), m_sampling_freq(0), m_sampling_ctrl(0), m_joy_trim_dac(0), m_reset(0), m_active_voices(14), m_irq_source(0),
	m_stream(nullptr),
	m_timer1(nullptr), m_timer2(nullptr), m_dmatimer(nullptr), m_voltimer(nullptr),
	m_current_voice(0), m_current_reg(0), m_adlib_cmd(0), m_mix_ctrl(0), m_gf1_irq(0), m_midi_irq(0), m_dma_channel1(0), m_dma_channel2(0), m_irq_combine(0), m_dma_combine(0), m_adlib_timer_cmd(0), m_adlib_timer1_enable(0), m_adlib_timer2_enable(0), m_adlib_status(0), m_adlib_data(0), m_voice_irq_ptr(0), m_voice_irq_current(0), m_dma_16bit(0), m_statread(0), m_sb_data_2xc(0), m_sb_data_2xe(0), m_reg_ctrl(0), m_fake_adlib_status(0), m_dma_current(0), m_txirq(0), m_rxirq(0),
	m_txirq_handler(*this),
	m_rxirq_handler(*this),
	m_wave_irq_handler(*this),
	m_ramp_irq_handler(*this),
	m_timer1_irq_handler(*this),
	m_timer2_irq_handler(*this),
	m_sb_irq_handler(*this),
	m_dma_irq_handler(*this),
	m_drq1_handler(*this),
	m_drq2_handler(*this),
	m_nmi_handler(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void gf1_device::device_start()
{
	acia6850_device::device_start();

	// TODO: make DRAM size configurable.  Can be 256k, 512k, 768k, or 1024k
	m_wave_ram.resize(1024*1024);
	memset(&m_wave_ram[0], 0, 1024*1024);

	m_stream = stream_alloc(0,2,clock() / (14 * 16));

	// init timers
	m_timer1 = timer_alloc(FUNC(gf1_device::adlib_timer1_tick), this);
	m_timer2 = timer_alloc(FUNC(gf1_device::adlib_timer2_tick), this);
	m_dmatimer = timer_alloc(FUNC(gf1_device::dma_tick), this);
	m_voltimer = timer_alloc(FUNC(gf1_device::update_volume_ramps), this);

	save_item(NAME(m_wave_ram));

	m_voice_irq_current = 0;
	m_voice_irq_ptr = 0;
	m_dma_channel1 = 0;
	m_dma_channel2 = 0;
	m_gf1_irq = 0;
	m_midi_irq = 0;

	double out = double(1 << 13);
	for (int i = 4095; i >= 0; i--)
	{
		m_volume_table[i] = int16_t(out);
		out /= 1.002709201; /* 0.0235 dB Steps */
	}

#ifdef LOG_SOUND
	f = fopen("soundlog.bin","wb");
#endif
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void gf1_device::device_reset()
{
	int x;

	memset(m_voice, 0x00, sizeof(m_voice));
	// init voices
	for(x=0;x<32;x++)
	{
		m_voice[x].voice_ctrl = 0x01;  // stop all voices
		m_voice[x].vol_ramp_ctrl = 0x01; // stop all volume ramps
		m_voice[x].current_vol = 0;  // silence all voices
	}
	m_irq_source = 0xe0;
	m_reg_ctrl = 0;
	m_active_voices = 14;
	m_stream->set_sample_rate(clock() / (m_active_voices * 16));
	m_voltimer->adjust(attotime::zero,0,attotime::from_usec(1000/(1.6*m_active_voices)));
}

void gf1_device::device_stop()
{
#ifdef SAVE_WAVE_RAM
	FILE* f;
	f=fopen("waveout.bin","wb");
	fwrite(m_wave_ram,1024*1024,1,f);
	fclose(f);
#endif
#ifdef LOG_SOUND
	fclose(f);
#endif
}

void gf1_device::device_clock_changed()
{
	m_stream->set_sample_rate(clock() / (m_active_voices * 16));
}
// ------------------------------------------------
//   device I/O handlers
// ------------------------------------------------

uint8_t gf1_device::global_reg_select_r(offs_t offset)
{
	if(offset == 0)
		return m_current_voice;
	else
		return m_current_reg | 0xc0;
}

void gf1_device::global_reg_select_w(offs_t offset, uint8_t data)
{
	if(offset == 0)
		m_current_voice = data & 0x1f;
	else
		m_current_reg = data;
}

uint8_t gf1_device::global_reg_data_r(offs_t offset)
{
	uint16_t ret;

	switch(m_current_reg)
	{
	case 0x41:  // DMA DRAM control
		if(offset == 1)
		{
			ret = m_dma_dram_ctrl;
			m_dma_dram_ctrl &= ~0x40;
			m_dma_irq_handler(0);
			return ret;
		}
		break;
	case 0x45:  // Timer control
		if(offset == 1)
			return m_timer_ctrl & 0x0c;
		break;
	case 0x49:  // Sampling control
		if(offset == 1)
			return m_sampling_ctrl & 0xe7;
		break;
	case 0x4c:  // Reset
		if(offset == 1)
			return m_reset;
		break;
	case 0x80: // Voice control
/* bit 0 - 1 if voice is stopped
 * bit 6 - 1 if addresses are decreasing, can change when looping is enabled
 * bit 7 - 1 if Wavetable IRQ is pending */
		if(offset == 1)
			return m_voice[m_current_voice].voice_ctrl & 0xff;
		break;
	case 0x81:  // Frequency Control
		ret = m_voice[m_current_voice].freq_ctrl;
		if(offset == 0)
			return ret & 0x00ff;
		else
			return (ret >> 8) & 0x00ff;
	case 0x82:  // Starting address (high 13 bits)
		ret = (m_voice[m_current_voice].start_addr >> 16);
		if(offset == 0)
			return ret & 0x00ff;
		else
			return (ret >> 8) & 0x00ff;
	case 0x83:  // Starting address (low 7 bits plus 4 bits fractional)
		ret = (m_voice[m_current_voice].start_addr & 0xffff);
		if(offset == 0)
			return ret & 0x00ff;
		else
			return (ret >> 8) & 0x00ff;
	case 0x84:  // End address (high 13 bits)
		ret = (m_voice[m_current_voice].end_addr >> 16);
		if(offset == 0)
			return ret & 0x00ff;
		else
			return (ret >> 8) & 0x00ff;
	case 0x85:  // End address (low 7 bits plus 4 bits fractional)
		ret = (m_voice[m_current_voice].end_addr & 0xffff);
		if(offset == 0)
			return ret & 0x00ff;
		else
			return (ret >> 8) & 0x00ff;
	case 0x86:  // Volume Ramp rate
		if(offset == 1)
			return m_voice[m_current_voice].vol_ramp_rate;
		break;
	case 0x87:  // Volume Ramp start (high 4 bits = exponent, low 4 bits = mantissa)
		if(offset == 1)
			return m_voice[m_current_voice].vol_ramp_start;
		break;
	case 0x88:  // Volume Ramp end (high 4 bits = exponent, low 4 bits = mantissa)
		if(offset == 1)
			return m_voice[m_current_voice].vol_ramp_end;
		break;
	case 0x89:  // Current Volume (high 4 bits = exponent, middle 8 bits = mantissa, low 4 bits = 0 [reserved])
		ret = m_voice[m_current_voice].current_vol;
		if(offset == 0)
			return ret & 0x00ff;
		else
			return (ret >> 8) & 0x00ff;
	case 0x8a:  // Current position (high 13 bits)
		ret = (m_voice[m_current_voice].current_addr >> 16);
		if(offset == 0)
			return ret & 0x00ff;
		else
			return (ret >> 8) & 0x00ff;
	case 0x8b:  // Current position (low 7 bits, plus 9 bit fractional)
		ret = (m_voice[m_current_voice].current_addr & 0xffff);
		if(offset == 0)
			return ret & 0x00ff;
		else
			return (ret >> 8) & 0x00ff;
	case 0x8c:  // Pan position (4 bits, 0=full left, 15=full right)
		if(offset == 1)
			return m_voice[m_current_voice].pan_position;
		break;
	case 0x8d:  // Volume Ramp control
/* bit 0 - Ramp has stopped
 * bit 6 - Ramp direction
 * bit 7 - Ramp IRQ pending */
		if(offset == 1)
			return m_voice[m_current_voice].vol_ramp_ctrl;
		break;
	case 0x8e:  // Active voices (6 bits, high 2 bits are always 1)
		if(offset == 1)
			return (m_active_voices - 1) | 0xc0;
		break;
	case 0x8f:  // IRQ source register
		if(offset == 1)
		{
			ret = m_voice_irq_fifo[m_voice_irq_current % 32];
			if((m_voice_irq_current % 32) != (m_voice_irq_ptr % 32))
				m_voice_irq_current++;
			else
				ret = 0xe0;
			m_wave_irq_handler(0);
			m_ramp_irq_handler(0);
			return ret;
		}
		break;
	default:
		logerror("GUS: Read from unimplemented or unknown global register %02x\n",m_current_reg);
		return 0xff;
	}
	return 0xff;
}

void gf1_device::global_reg_data_w(offs_t offset, uint8_t data)
{
	switch(m_current_reg)
	{
	case 0x00: // Voice control
/* bit 1 - set to 1 to stop current voice
 * bit 2 - set to 1 for 16-bit wave data, otherwise is 8-bit
 * bit 3 - set to 1 to loop to start address when the end address is reached
 * bit 4 - set to 1 to enable bi-directional looping
 * bit 5 - set to 1 to enable wavetable IRQ when end address is reached */
		if(offset == 1)
		{
			m_voice[m_current_voice].voice_ctrl = data & 0xff;
			m_voice[m_current_voice].rollover = false;
			if(data & 0x02)
				m_voice[m_current_voice].voice_ctrl |= 0x01;
		}
		logerror("GUS: Ch%i Voice control write %02x\n", m_current_voice,data);
		break;
	case 0x01: // Frequency Control
/* bits 15-10 - Integer portion
 * bits 9-1   - Fractional portion
 * bit 0      - not used */
		if(offset == 0)
			m_voice[m_current_voice].freq_ctrl = (m_voice[m_current_voice].freq_ctrl & 0xff00) | data;
		else
			m_voice[m_current_voice].freq_ctrl = (m_voice[m_current_voice].freq_ctrl & 0x00ff) | (data << 8);
		logerror("GUS: Ch%i Frequency control write %04x\n", m_current_voice, m_voice[m_current_voice].freq_ctrl);
		break;
	case 0x02:  // Starting address (high 13 bits)
		if(offset == 0)
			m_voice[m_current_voice].start_addr = (m_voice[m_current_voice].start_addr & 0xff00ffff) | (data << 16);
		else
			m_voice[m_current_voice].start_addr = (m_voice[m_current_voice].start_addr & 0x00ffffff) | (data << 24);
		logerror("GUS: Ch%i [high] Start address set to %08x\n", m_current_voice,m_voice[m_current_voice].start_addr);
		break;
	case 0x03:  // Starting address (low 7 bits plus 4 bits fractional)
		if(offset == 0)
			m_voice[m_current_voice].start_addr = (m_voice[m_current_voice].start_addr & 0xffffff00) | data;
		else
			m_voice[m_current_voice].start_addr = (m_voice[m_current_voice].start_addr & 0xffff00ff) | (data << 8);
		logerror("GUS: Ch%i [low] Start address set to %08x\n", m_current_voice,m_voice[m_current_voice].start_addr);
		break;
	case 0x04:  // End address (high 13 bits)
		if(offset == 0)
			m_voice[m_current_voice].end_addr = (m_voice[m_current_voice].end_addr & 0xff00ffff) | (data << 16);
		else
			m_voice[m_current_voice].end_addr = (m_voice[m_current_voice].end_addr & 0x00ffffff) | (data << 24);
		logerror("GUS: Ch%i [high] End address set to %08x\n", m_current_voice,m_voice[m_current_voice].end_addr);
		break;
	case 0x05:  // End address (low 7 bits plus 4 bits fractional)
		if(offset == 0)
			m_voice[m_current_voice].end_addr = (m_voice[m_current_voice].end_addr & 0xffffff00) | data;
		else
			m_voice[m_current_voice].end_addr = (m_voice[m_current_voice].end_addr & 0xffff00ff) | (data << 8);
		logerror("GUS: Ch%i [low] End address set to %08x\n", m_current_voice,m_voice[m_current_voice].end_addr);
		break;
	case 0x06:  // Volume Ramp rate
		if(offset == 1)
			m_voice[m_current_voice].vol_ramp_rate = data;
		logerror("GUS: Ch%i Volume ramp rate write %02x\n", m_current_voice,data);
		break;
	case 0x07:  // Volume Ramp start (high 4 bits = exponent, low 4 bits = mantissa)
		if(offset == 1)
			m_voice[m_current_voice].vol_ramp_start = data;
		logerror("GUS: Ch%i Volume ramp start write %02x\n", m_current_voice, data);
		break;
	case 0x08:  // Volume Ramp end (high 4 bits = exponent, low 4 bits = mantissa)
		if(offset == 1)
			m_voice[m_current_voice].vol_ramp_end = data;
		logerror("GUS: Ch%i Volume ramp end write %02x\n", m_current_voice, data);
		break;
	case 0x09:  // Current Volume (high 4 bits = exponent, middle 8 bits = mantissa, low 4 bits = 0 [reserved])
		if(offset == 0)
			m_voice[m_current_voice].current_vol = (m_voice[m_current_voice].current_vol & 0xff00) | data;
		else
			m_voice[m_current_voice].current_vol = (m_voice[m_current_voice].current_vol & 0x00ff) | (data << 8);
		logerror("GUS: Ch%i Current volume write %02x\n", m_current_voice, data);
		break;
	case 0x0a:  // Current position (high 13 bits)
		if(offset == 0)
			m_voice[m_current_voice].current_addr = (m_voice[m_current_voice].current_addr & 0xff00ffff) | (data << 16);
		else
			m_voice[m_current_voice].current_addr = (m_voice[m_current_voice].current_addr & 0x00ffffff) | (data << 24);
		logerror("GUS: Ch%i Current address write %08x\n", m_current_voice, m_voice[m_current_voice].current_addr);
		break;
	case 0x0b:  // Current position (low 7 bits, plus 9 bit fractional)
		if(offset == 0)
			m_voice[m_current_voice].current_addr = (m_voice[m_current_voice].current_addr & 0xffffff00) | data;
		else
			m_voice[m_current_voice].current_addr = (m_voice[m_current_voice].current_addr & 0xffff00ff) | (data << 8);
		logerror("GUS: Ch%i Current address write %08x\n", m_current_voice, m_voice[m_current_voice].current_addr);
		break;
	case 0x0c:  // Pan position (4 bits, 0=full left, 15=full right)
		if(offset == 1)
			m_voice[m_current_voice].pan_position = data & 0x0f;
		logerror("GUS: Ch%i Pan Position write %02x\n", m_current_voice, data);
		break;
	case 0x0d:  // Volume Ramp control
/* bit 1 - set to 1 to stop the ramp
 * bit 2 - roll over condition (generate IRQ, and not stop playing voice, no looping)
 * bit 3 - enable looping
 * bit 4 - enable bi-directional looping
 * bit 5 - enable IRQ at end of ramp */
		if(offset == 1)
		{
			m_voice[m_current_voice].vol_ramp_ctrl = data & 0x7f;
			if(!(data & 0x01))
			{
				m_voice[m_current_voice].vol_count = 0;
				if(m_voice[m_current_voice].vol_ramp_ctrl & 0x40)
					m_voice[m_current_voice].current_vol = (m_voice[m_current_voice].vol_ramp_end << 8);
				else
					m_voice[m_current_voice].current_vol = (m_voice[m_current_voice].vol_ramp_start << 8);
			}
			if(data & 0x02)
			{
				m_voice[m_current_voice].vol_ramp_ctrl |= 0x01;
			}
		}
		logerror("GUS: Ch%i Volume Ramp control write %02x\n", m_current_voice, data);
		break;
	case 0x0e:  // Active voices (6 bits, high 2 bits are always 1)
		if(offset == 1)
		{
			m_active_voices = (data & 0x3f) + 1;
			if(m_active_voices < 14)
				m_active_voices = 14;
			if(m_active_voices > 32)
				m_active_voices = 32;
			m_stream->set_sample_rate(clock() / (m_active_voices * 16));
			m_voltimer->adjust(attotime::zero,0,attotime::from_usec(1000/(1.6*m_active_voices)));
		}
		logerror("GUS: Active Voices write %02x (%d voices at %u Hz)\n", data, m_active_voices, clock() / (m_active_voices * 16));
		break;
	case 0x41:
/* bit 0 - Enable the DMA channel.
 * bit 1 - DMA transfer direction (1 = read from the GUS)
 * bit 2 - DMA channel width (0=8-bit, 1=16-bit)
 * bits 3,4 - DMA rate divider
 * bit 5 - DMA terminal count IRQ enable
 * bit 6 - DMA terminal count IRQ pending (read), Data size (write, 0=8bit, 1=16-bit, independent of channel size)
 * bit 7 - Invert MSB of data
 */
		if(offset == 1)
		{
			m_dma_dram_ctrl = data & 0xbf;
			m_dma_16bit = data & 0x40;
			if(data & 0x01)
			{
				m_dmatimer->adjust(attotime::zero,0,attotime::from_nsec(11489));  // based on 680Kb/sec mentioned in UltraMID docs
				logerror("GUS: DMA start from DRAM address 0x%05x\n",m_dma_start_addr<<4);
			}
			else
			{
				m_dmatimer->reset();  // stop transfer
				logerror("GUS: DMA aborted.\n");
			}
		}
		logerror("GUS: DMA DRAM control write %02x\n",data);
		break;
	case 0x42:  // DMA start address (high 16 bits, address lines 4-19)
		if(offset == 0)
			m_dma_start_addr = (m_dma_start_addr & 0xff00) | data;
		else
			m_dma_start_addr = (m_dma_start_addr & 0x00ff) | (data << 8);
		m_dma_current = m_dma_start_addr << 4;
		logerror("GUS: DMA start address set to %08x\n",m_dma_start_addr);
		break;
	case 0x43:  // DRAM I/O address (low 16 bits)
		if(offset == 0)
			m_dram_addr = (m_dram_addr & 0x000fff00) | data;
		else
			m_dram_addr = (m_dram_addr & 0x000f00ff) | (data << 8);
		//logerror("GUS: [low] DRAM I/O address set to %08x\n",m_dram_addr);
		break;
	case 0x44:  // DRAM I/O address (high 4 bits)
		if(offset == 1)
			m_dram_addr = (m_dram_addr & 0x0000ffff) | (data << 16);
		//logerror("GUS: [high] DRAM I/O address set to %08x\n",m_dram_addr);
		break;
	case 0x45:  // Timer control
/* bit 3 - Enable timer 1 IRQ
 * bit 4 - Enable timer 2 IRQ */
		if(offset == 1)
		{
			m_timer_ctrl = data;
			if(!(data & 0x20))
				m_adlib_status &= ~0x18;
			if(!(data & 0x02))
				m_adlib_status &= ~0x01;
			if(!(m_adlib_status & 0x19))
				m_sb_irq_handler(0);
			if(!(data & 0x04))
			{
				m_adlib_status &= ~0x40;
				m_timer1_irq_handler(0);
			}
			if(!(data & 0x08))
			{
				m_adlib_status &= ~0x20;
				m_timer2_irq_handler(0);
			}
			if((m_adlib_status & 0x60) != 0)
				m_adlib_status &= ~0x80;
		}
		logerror("GUS: Timer control write %02x\n",data);
		break;
	case 0x46:  // Timer 1 count
		if(offset == 1)
		{
			m_timer1_count = data;
			m_timer1_value = data;
			logerror("GUS: Timer 1 count write %02x (%d usec)\n",data,data*80);
		}
		break;
	case 0x47:  // Timer 2 count
		if(offset == 1)
		{
			m_timer2_count = data;
			m_timer2_value = data;
			logerror("GUS: Timer 2 count write %02x (%d usec)\n",data,data*320);
		}
		break;
	case 0x48:  // Sampling Frequency - 9878400/(16*(FREQ+2))
		if(offset == 0)
			m_sampling_freq = (m_sampling_freq & 0xff00) | data;
		else
			m_sampling_freq = (m_sampling_freq & 0x00ff) | (data << 8);
		logerror("GUS: Sampling frequency write %02x\n",data);
		break;
	case 0x49: // Sampling control
/* bit 0 - Start sampling
 * bit 1 - Mode (0=mono, 1=stereo)
 * bit 2 - DMA width (0=8-bit, 1=16-bit)
 * bit 5 - DMA IRQ enable
 * bit 6 - DMA IRQ pending (read only)
 * bit 7 - Invert MSB */
		if(offset == 1)
			m_sampling_ctrl = data;
		logerror("GUS: Sampling control write %02x\n",data);
		break;
	case 0x4b:  // Joystick trim DAC
		if(offset == 1)
			m_joy_trim_dac = data;
		logerror("GUS: Joystick trim DAC write %02x\n",data);
		break;
	case 0x4c:  // Reset
		if(offset == 1)
		{
			if(!(data & 0x01))
				device_reset();
			m_reset = data & 0xf9;
		}
		logerror("GUS: Reset write %02x\n",data);
		break;
	default:
		logerror("GUS: Write %02x to unimplemented or unknown global register %02x\n",data,m_current_reg);
	}
}

/* port 0x3X7 - DRAM I/O
 * read and write bytes directly to wavetable DRAM */
uint8_t gf1_device::dram_r(offs_t offset)
{
	if(offset == 1)
	{
		return m_wave_ram[m_dram_addr & 0xfffff];
	}
	else
		return 0xff;
}

void gf1_device::dram_w(offs_t offset, uint8_t data)
{
	if(offset == 1)
	{
		m_wave_ram[m_dram_addr & 0xfffff] = data;
	}
}

/* port 2XA - read selected adlib command?
 * the GUS driver installation writes 0x55 to port 0x388, then expects to reads the same from 0x2XA */
uint8_t gf1_device::adlib_cmd_r(offs_t offset)
{
	if(offset == 0)
	{
		return m_adlib_cmd;
	}
	else
	{
		// TODO
		return 0xff;
	}
}

/* port 0x2XB - set IRQ/DMA latch
 * if IRQ (bit 6 of 0x2X0 = 1)
 * bits 2-0 = channel 1 (GF1) IRQ selector
 * 0 = reserved, 1 = IRQ2, 2 = IRQ5, 3 = IRQ3, 4 = IRQ7, 5 = IRQ11, 6 = IRQ12, 7 = IRQ13
 * bits 5-3 = channel 2 (MIDI) IRQ selector
 * 0 = No interrupt selected, rest are as for the GF1
 * bit 6 = combine both IRQs using channel 1 IRQ
 * if DMA (bit 6 of 0x2X0 = 0)
 * bits 2-0 = DMA select register 1
 * 0 = No DMA, 1 = DMA1, 2 = DMA3, 3 = DMA5, 4 = DMA6, 5 = DMA7
 * bits 5-3 = DMA select register 2 (values same as reg 1)
 * bit 6 = combine both on same DMA channel
 */
void gf1_device::adlib_cmd_w(offs_t offset, uint8_t data)
{
	if(offset == 1)
	{
		switch(m_reg_ctrl & 0x07)
		{
		case 0x00:
			if(m_mix_ctrl & 0x40)
			{
				switch(data & 0x07)
				{
				case 1:
					m_gf1_irq = 2;
					break;
				case 2:
					m_gf1_irq = 5;
					break;
				case 3:
					m_gf1_irq = 3;
					break;
				case 4:
					m_gf1_irq = 7;
					break;
				case 5:
					m_gf1_irq = 11;
					break;
				case 6:
					m_gf1_irq = 12;
					break;
				case 7:
					m_gf1_irq = 15;
					break;
				default:
					m_gf1_irq = 0;
					logerror("GUS: Invalid GF1 IRQ set! [%02x]\n",data);
				}
				switch((data >> 3) & 0x07)
				{
				case 0:
					m_midi_irq = 0;
					break;
				case 1:
					m_midi_irq = 2;
					break;
				case 2:
					m_midi_irq = 5;
					break;
				case 3:
					m_midi_irq = 3;
					break;
				case 4:
					m_midi_irq = 7;
					break;
				case 5:
					m_midi_irq = 11;
					break;
				case 6:
					m_midi_irq = 12;
					break;
				case 7:
					m_midi_irq = 15;
					break;
				default:
					logerror("GUS: Invalid MIDI IRQ set! [%02x]\n",data);
				}
				if(data & 0x40)
					m_irq_combine = 1;
				else
					m_irq_combine = 0;
				logerror("GUS: IRQs set: GF1 = IRQ%i, MIDI = IRQ%i\n",m_gf1_irq,m_midi_irq);
			}
			else
			{
				switch(data & 0x07)
				{
				case 0:
					m_dma_channel1 = 0;
					break;
				case 1:
					m_dma_channel1 = 1;
					break;
				case 2:
					m_dma_channel1 = 3;
					break;
				case 3:
					m_dma_channel1 = 5;
					break;
				case 4:
					m_dma_channel1 = 6;
					break;
				case 5:
					m_dma_channel1 = 7;
					break;
				default:
					logerror("GUS: Invalid DMA channel #1 set! [%02x]\n",data);
				}
				switch((data >> 3) & 0x07)
				{
				case 0:
					m_dma_channel2 = 0;
					break;
				case 1:
					m_dma_channel2 = 1;
					break;
				case 2:
					m_dma_channel2 = 3;
					break;
				case 3:
					m_dma_channel2 = 5;
					break;
				case 4:
					m_dma_channel2 = 6;
					break;
				case 5:
					m_dma_channel2 = 7;
					break;
				default:
					logerror("GUS: Invalid DMA channel #2 set! [%02x]\n",data);
				}
				if(data & 0x40)
					m_dma_combine = 1;
				else
					m_dma_combine = 0;
				logerror("GUS: DMA channels set: DMA%i, DMA%i\n",m_dma_channel1,m_dma_channel2);
			}
			break;
		case 0x05:
			m_statread = 0;
			//m_other_irq_handler(0);
			break;
		case 0x06:
			// TODO: Jumper register (joy/MIDI enable)
			break;
		}
	}
	else
	{
		m_fake_adlib_status = data;
		logerror("GUS: Adlib status set to %02x\n",data);
	}
}

/* port 0x2X0 - Mix control register
 * bit 0 - 0=Enable Line In
 * bit 1 - 0=Enable Line Out
 * bit 2 - 1=Enable MIC In
 * bit 3 - Enable latches (once enabled, must remain enabled)
 * bit 4 - Combine GF1 IRQs with MIDI IRQs
 * bit 5 - Enable MIDI TxD to RxD loopback
 * bit 6 - Control Reg Select - set next I/O write to 0x2XB to be DMA (0) or IRQ (1) channel latches */
uint8_t gf1_device::mix_ctrl_r(offs_t offset)
{
	return 0xff;  // read only
}

void gf1_device::mix_ctrl_w(offs_t offset, uint8_t data)
{
	if(offset == 0)
		m_mix_ctrl = data;
}

uint8_t gf1_device::sb_r(offs_t offset)
{
	uint8_t val;

	switch(offset)
	{
	case 0x00:
		val = m_sb_data_2xc;
		if(m_statread & 0x20)
			m_sb_data_2xc ^= 0x80;  // flip MSB on read
		return val;
	// port 0x2XD is write-only
	case 0x02:
		if(m_reg_ctrl & 0x80)
		{
			m_statread |= 0x80;
			m_nmi_handler(1);
		}
		return m_sb_data_2xe;
	}
	return 0xff;
}

void gf1_device::sb_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
	case 0x00:
		if(m_timer_ctrl & 0x20)
		{
			m_adlib_status |= 0x10;
			m_nmi_handler(1);
			logerror("GUS: SB 0x2XC IRQ active\n");
		}
		break;
	case 0x01:
		m_sb_data_2xc = data;
		break;
	case 0x02:
		m_sb_data_2xe = data;
		break;
	}
}

void gf1_device::sb2x6_w(uint8_t data)
{
	if(m_timer_ctrl & 0x20)
	{
		m_adlib_status |= 0x08;
		m_nmi_handler(1);
		logerror("GUS: SB 0x2X6 IRQ active\n");
	}
}

uint8_t gf1_device::stat_r()
{
	uint8_t val = m_statread & 0xf9;
	if(m_mix_ctrl & 0x08)
		val |= 0x02;
	return val;
}

void gf1_device::stat_w(uint8_t data)
{
	m_reg_ctrl = data;
}

void gf1_device::set_irq(uint8_t source, uint8_t voice)
{
	if(source & IRQ_WAVETABLE)
	{
		m_irq_source = 0xe0 | (voice & 0x1f);
		m_irq_source &= ~0x80;
		m_wave_irq_handler(1);
		m_voice_irq_fifo[m_voice_irq_ptr % 32] = m_irq_source;
		m_voice_irq_ptr++;
		m_voice[voice].voice_ctrl |= 0x80;
	}
	if(source & IRQ_VOLUME_RAMP)
	{
		m_irq_source = 0xe0 | (voice & 0x1f);
		m_irq_source &= ~0x40;
		m_ramp_irq_handler(1);
		m_voice_irq_fifo[m_voice_irq_ptr % 32] = m_irq_source;
		m_voice_irq_ptr++;
	}
}

void gf1_device::reset_irq(uint8_t source)
{
	if(source & IRQ_WAVETABLE)
	{
		m_irq_source |= 0x80;
		m_wave_irq_handler(0);
	}
	if(source & IRQ_VOLUME_RAMP)
	{
		m_irq_source |= 0x40;
		m_ramp_irq_handler(0);
	}
}

// TODO: support 16-bit transfers
uint8_t gf1_device::dack_r(int line)
{
	return m_wave_ram[m_dma_current++ & 0xfffff];
}

void gf1_device::dack_w(int line,uint8_t data)
{
	if(m_dma_dram_ctrl & 0x80)  // flip data MSB
	{
		if(m_dma_16bit != 0) // if data is 16-bit
		{
			if((m_dma_current & 1))
				data ^= 0x80;
		}
		else  // data is 8-bit
		{
			data ^= 0x80;
		}
	}
	m_wave_ram[m_dma_current & 0xfffff] = data;
	m_dma_current++;
	m_drq1_handler(0);
}

void gf1_device::eop_w(int state)
{
	if(state == ASSERT_LINE) {
		// end of transfer
		m_dmatimer->reset();
		//m_drq1_handler(0);
		if(m_dma_dram_ctrl & 0x20)
		{
			m_dma_dram_ctrl |= 0x40;
			m_dma_irq_handler(1);
		}
		logerror("GUS: End of transfer. (%05x)\n",m_dma_current);
	}
}


/* 16-bit ISA card device implementation */

static INPUT_PORTS_START( gus_joy )
	PORT_START("gus_joy")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW,   IPT_UNUSED ) // x/y ad stick to digital converters
	PORT_BIT( 0x10, IP_ACTIVE_LOW,   IPT_BUTTON1) PORT_NAME("GUS Joystick Button 1")
	PORT_BIT( 0x20, IP_ACTIVE_LOW,   IPT_BUTTON2) PORT_NAME("GUS Joystick Button 2")
	PORT_BIT( 0x40, IP_ACTIVE_LOW,   IPT_BUTTON3) PORT_NAME("GUS Joystick Button 3")
	PORT_BIT( 0x80, IP_ACTIVE_LOW,   IPT_BUTTON4) PORT_NAME("GUS Joystick Button 4")

	PORT_START("gus_joy_1")
	PORT_BIT(0xff,0x80,IPT_AD_STICK_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_MINMAX(1,0xff) PORT_CODE_DEC(KEYCODE_LEFT) PORT_CODE_INC(KEYCODE_RIGHT) PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH) PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH)

	PORT_START("gus_joy_2")
	PORT_BIT(0xff,0x80,IPT_AD_STICK_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_MINMAX(1,0xff) PORT_CODE_DEC(KEYCODE_UP) PORT_CODE_INC(KEYCODE_DOWN) PORT_CODE_DEC(JOYCODE_Y_UP_SWITCH) PORT_CODE_INC(JOYCODE_Y_DOWN_SWITCH)
INPUT_PORTS_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void isa16_gus_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "speaker", 2).front();
	GGF1(config, m_gf1, GF1_CLOCK);
	m_gf1->add_route(0, "speaker", 0.50, 0);
	m_gf1->add_route(1, "speaker", 0.50, 1);

	m_gf1->txd_handler().set("mdout", FUNC(midi_port_device::write_txd));
	m_gf1->txirq_handler().set(FUNC(isa16_gus_device::midi_txirq));
	m_gf1->rxirq_handler().set(FUNC(isa16_gus_device::midi_rxirq));
	m_gf1->wave_irq_handler().set(FUNC(isa16_gus_device::wavetable_irq));
	m_gf1->ramp_irq_handler().set(FUNC(isa16_gus_device::volumeramp_irq));
	m_gf1->timer1_irq_handler().set(FUNC(isa16_gus_device::timer1_irq));
	m_gf1->timer2_irq_handler().set(FUNC(isa16_gus_device::timer2_irq));
	m_gf1->sb_irq_handler().set(FUNC(isa16_gus_device::sb_irq));
	m_gf1->dma_irq_handler().set(FUNC(isa16_gus_device::dma_irq));
	m_gf1->drq1_handler().set(FUNC(isa16_gus_device::drq1_w));
	m_gf1->drq2_handler().set(FUNC(isa16_gus_device::drq2_w));
	m_gf1->nmi_handler().set(FUNC(isa16_gus_device::nmi_w));

	MIDI_PORT(config, "mdin", midiin_slot, "midiin").rxd_handler().set(m_gf1, FUNC(acia6850_device::write_rxd));

	MIDI_PORT(config, "mdout", midiout_slot, "midiout");

	clock_device &acia_clock(CLOCK(config, "acia_clock", 31250*16));
	acia_clock.signal_handler().set(FUNC(isa16_gus_device::write_acia_clock));
}

ioport_constructor isa16_gus_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( gus_joy );
}


isa16_gus_device::isa16_gus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ISA16_GUS, tag, owner, clock),
	device_isa16_card_interface(mconfig, *this),
	m_gf1(*this, "gf1"),
	m_irq_status(0)
{
}

void isa16_gus_device::device_start()
{
	set_isa_device();
	m_isa->install_device(0x0200, 0x0201, read8sm_delegate(*this, FUNC(isa16_gus_device::joy_r)), write8sm_delegate(*this, FUNC(isa16_gus_device::joy_w)));
	m_isa->install_device(0x0220, 0x022f, read8sm_delegate(*this, FUNC(isa16_gus_device::board_r)), write8sm_delegate(*this, FUNC(isa16_gus_device::board_w)));
	m_isa->install_device(0x0320, 0x0327, read8sm_delegate(*this, FUNC(isa16_gus_device::synth_r)), write8sm_delegate(*this, FUNC(isa16_gus_device::synth_w)));
	m_isa->install_device(0x0388, 0x0389, read8sm_delegate(*this, FUNC(isa16_gus_device::adlib_r)), write8sm_delegate(*this, FUNC(isa16_gus_device::adlib_w)));
}

void isa16_gus_device::device_reset()
{
}

void isa16_gus_device::device_stop()
{
}

uint8_t isa16_gus_device::board_r(offs_t offset)
{
	switch(offset)
	{
	case 0x00:
	case 0x01:
		return m_gf1->mix_ctrl_r(offset);
		/* port 0x2X6 - IRQ status (active high)
		 * bit 0 - MIDI transmit IRQ
		 * bit 1 - MIDI receive IRQ
		 * bit 2 - Timer 1 IRQ
		 * bit 3 - Timer 2 IRQ
		 * bit 4 - reserved (always 0)
		 * bit 5 - wavetable IRQ
		 * bit 6 - volume ramp IRQ
		 * bit 7 - DRAM TC DMA IRQ
		 */
	case 0x06:
		return m_irq_status;
	case 0x08:
	case 0x09:
		return m_gf1->adlib_r(offset-8);
	case 0x0a:
	case 0x0b:
		return m_gf1->adlib_cmd_r(offset-10);
	case 0x0c:
	case 0x0d:
	case 0x0e:
		return m_gf1->sb_r(offset-12);
	case 0x0f:
		return m_gf1->stat_r();
	default:
		logerror("GUS: Invalid or unimplemented read of port 0x2X%01x\n",offset);
		return 0xff;
	}
}

void isa16_gus_device::board_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
	case 0x00:
	case 0x01:
		m_gf1->mix_ctrl_w(offset,data);
		break;
	case 0x06:
		m_gf1->sb2x6_w(data);
		break;
	case 0x08:
	case 0x09:
		m_gf1->adlib_w(offset-8,data);
		break;
	case 0x0a:
	case 0x0b:
		m_gf1->adlib_cmd_w(offset-10,data);
		break;
	case 0x0c:
	case 0x0d:
	case 0x0e:
		m_gf1->sb_w(offset-12,data);
		break;
	case 0x0f:
		m_gf1->stat_w(data);
		break;
	default:
		logerror("GUS: Invalid or unimplemented register write %02x of port 0x2X%01x\n",data,offset);
	}
}

uint8_t isa16_gus_device::synth_r(offs_t offset)
{
	switch(offset)
	{
	case 0x00:
		return m_gf1->status_r();
	case 0x01:
		return m_gf1->data_r();
	case 0x02:
	case 0x03:
		return m_gf1->global_reg_select_r(offset-2);
	case 0x04:
	case 0x05:
		return m_gf1->global_reg_data_r(offset-4);
	case 0x06:
	case 0x07:
		return m_gf1->dram_r(offset-6);
	default:
		logerror("GUS: Invalid or unimplemented register read of port 0x3X%01x\n",offset);
		return 0xff;
	}
}

void isa16_gus_device::synth_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
	case 0x00:
		m_gf1->control_w(data);
		break;
	case 0x01:
		m_gf1->data_w(data);
		break;
	case 0x02:
	case 0x03:
		m_gf1->global_reg_select_w(offset-2,data);
		break;
	case 0x04:
	case 0x05:
		m_gf1->global_reg_data_w(offset-4,data);
		break;
	case 0x06:
	case 0x07:
		m_gf1->dram_w(offset-6,data);
		break;
	default:
		logerror("GUS: Invalid or unimplemented register write %02x of port 0x3X%01x\n",data,offset);
	}
}

uint8_t isa16_gus_device::adlib_r(offs_t offset)
{
	return m_gf1->adlib_r(offset);
}

void isa16_gus_device::adlib_w(offs_t offset, uint8_t data)
{
	m_gf1->adlib_w(offset,data);
}

uint8_t isa16_gus_device::joy_r(offs_t offset)
{
	if(offset == 1)
	{
		uint8_t data;
		int delta;
		attotime new_time = machine().time();

		{
			data = ioport("gus_joy")->read() | 0x0f;

			{
				delta = ((new_time - m_joy_time) * 256 * 1000).seconds();

				if (ioport("gus_joy_1")->read() < delta) data &= ~0x01;
				if (ioport("gus_joy_2")->read() < delta) data &= ~0x02;
			}
		}
		return data;
	}
	return 0xff;
}

void isa16_gus_device::joy_w(offs_t offset, uint8_t data)
{
	m_joy_time = machine().time();
}

void isa16_gus_device::wavetable_irq(int state)
{
	if(state)
		set_irq(IRQ_WAVETABLE);
	else
		reset_irq(IRQ_WAVETABLE);
}

void isa16_gus_device::volumeramp_irq(int state)
{
	if(state)
		set_irq(IRQ_VOLUME_RAMP);
	else
		reset_irq(IRQ_VOLUME_RAMP);
}

void isa16_gus_device::timer1_irq(int state)
{
	if(state)
		set_irq(IRQ_TIMER1);
	else
		reset_irq(IRQ_TIMER1);
}

void isa16_gus_device::timer2_irq(int state)
{
	if(state)
		set_irq(IRQ_TIMER2);
	else
		reset_irq(IRQ_TIMER2);
}

void isa16_gus_device::dma_irq(int state)
{
	if(state)
		set_irq(IRQ_DRAM_TC_DMA);
	else
		reset_irq(IRQ_DRAM_TC_DMA);
}

void isa16_gus_device::sb_irq(int state)
{
	if(state)
		set_midi_irq(IRQ_SB);
	else
		reset_midi_irq(IRQ_SB);
}

void isa16_gus_device::drq1_w(int state)
{
	m_isa->set_dma_channel(m_gf1->dma_channel1(), this, true);
	switch(m_gf1->dma_channel1())
	{
	case 1:
		m_isa->drq1_w(state);
		break;
	case 3:
		m_isa->drq3_w(state);
		break;
	case 5:
		m_isa->drq5_w(state);
		break;
	case 6:
		m_isa->drq6_w(state);
		break;
	case 7:
		m_isa->drq7_w(state);
		break;
	default:
		logerror("GUS: Invalid DMA channel %i, ignoring.\n",m_gf1->dma_channel1());
	}
}

void isa16_gus_device::drq2_w(int state)
{
	m_isa->set_dma_channel(m_gf1->dma_channel2(), this, true);
	switch(m_gf1->dma_channel2())
	{
	case 1:
		m_isa->drq1_w(state);
		break;
	case 3:
		m_isa->drq3_w(state);
		break;
	case 5:
		m_isa->drq5_w(state);
		break;
	case 6:
		m_isa->drq6_w(state);
		break;
	case 7:
		m_isa->drq7_w(state);
		break;
	default:
		logerror("GUS: Invalid DMA channel %i, ignoring.\n",m_gf1->dma_channel2());
	}
}

void isa16_gus_device::set_irq(uint8_t source)
{
	m_irq_status |= source;

	switch(m_gf1->gf1_irq())
	{
	case 2:
		m_isa->irq2_w(1);
		break;
	case 3:
		m_isa->irq3_w(1);
		break;
	case 5:
		m_isa->irq5_w(1);
		break;
	case 7:
		m_isa->irq7_w(1);
		break;
	case 11:
		m_isa->irq11_w(1);
		break;
	case 12:
		m_isa->irq12_w(1);
		break;
	case 15:
		m_isa->irq15_w(1);
		break;
	}
	logerror("GUS: Set IRQ %02x\n",source);
}

void isa16_gus_device::reset_irq(uint8_t source)
{
	m_irq_status &= ~source;

	switch(m_gf1->gf1_irq())
	{
	case 2:
		m_isa->irq2_w(0);
		break;
	case 3:
		m_isa->irq3_w(0);
		break;
	case 5:
		m_isa->irq5_w(0);
		break;
	case 7:
		m_isa->irq7_w(0);
		break;
	case 11:
		m_isa->irq11_w(0);
		break;
	case 12:
		m_isa->irq12_w(0);
		break;
	case 15:
		m_isa->irq15_w(0);
		break;
	}
	logerror("GUS: Reset IRQ %02x\n",source);
}

void isa16_gus_device::set_midi_irq(uint8_t source)
{
	m_irq_status |= source;

	switch(m_gf1->midi_irq())
	{
	case 2:
		m_isa->irq2_w(1);
		break;
	case 3:
		m_isa->irq3_w(1);
		break;
	case 5:
		m_isa->irq5_w(1);
		break;
	case 7:
		m_isa->irq7_w(1);
		break;
	case 11:
		m_isa->irq11_w(1);
		break;
	case 12:
		m_isa->irq12_w(1);
		break;
	case 15:
		m_isa->irq15_w(1);
		break;
	}
	logerror("GUS: Set MIDI IRQ %02x\n",source);
}

void isa16_gus_device::reset_midi_irq(uint8_t source)
{
	m_irq_status &= ~source;

	switch(m_gf1->midi_irq())
	{
	case 2:
		m_isa->irq2_w(0);
		break;
	case 3:
		m_isa->irq3_w(0);
		break;
	case 5:
		m_isa->irq5_w(0);
		break;
	case 7:
		m_isa->irq7_w(0);
		break;
	case 11:
		m_isa->irq11_w(0);
		break;
	case 12:
		m_isa->irq12_w(0);
		break;
	case 15:
		m_isa->irq15_w(0);
		break;
	}
	logerror("GUS: Reset MIDI IRQ %02x\n",source);
}

void isa16_gus_device::midi_txirq(int state)
{
	if (state)
		set_midi_irq(IRQ_MIDI_TRANSMIT);
	else
		reset_midi_irq(IRQ_MIDI_TRANSMIT | IRQ_MIDI_RECEIVE);
}

void isa16_gus_device::midi_rxirq(int state)
{
	if (state)
		set_midi_irq(IRQ_MIDI_RECEIVE);
	else
		reset_midi_irq(IRQ_MIDI_TRANSMIT | IRQ_MIDI_RECEIVE);
}

void isa16_gus_device::write_acia_clock(int state)
{
	m_gf1->write_txc(state);
	m_gf1->write_rxc(state);
}

void isa16_gus_device::nmi_w(int state)
{
	m_irq_status |= IRQ_SB;
	m_isa->nmi();
}

uint8_t isa16_gus_device::dack_r(int line)
{
	if(line == m_gf1->dma_channel1())
		return m_gf1->dack_r(line);
	else
		return 0;
}

void isa16_gus_device::dack_w(int line,uint8_t data)
{
	if(line == m_gf1->dma_channel1())
		m_gf1->dack_w(line,data);
}

void isa16_gus_device::eop_w(int state)
{
	m_gf1->eop_w(state);
}
