// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    Capcom System QSound™

    Sixteen-channel sample player.  Previous HLE implementation by Paul
    Leaman and Miguel Angel Horna, with thanks to CAB (author of Amuse).

    The key components are a DSP16A, a TDA1543 dual 16-bit DAC with I2S
    input, and a TC9185P electronic volume control.  The TDA1543 is
    simulated here; no attempt is being made to emulate the TC9185P.

    Commands work by writing an address/data word pair to be written to
    DSP's internal RAM.  In theory it's possible to write anywhere in
    DSP RAM, but the glue logic only allows writing to the first 256
    words.  The host writes the high and low bytes the data word to
    offsets 0 and 1, respectively, and the address to offset 2.  Writing
    the address also asserts the DSP's INT pin.  The host can read back
    a single bit, which I've assumed reflects the current state of the
    DSP's INT pin (low when asserted).  The host won't send further
    commands until this bit goes high.

    On servicing an external interrupt, the DSP reads pdx0 three times,
    expecting to get the address and data in that order (the third read
    is needed because DSP16 has latent PIO reads in active mode).  I've
    assumed that reading PIO with PSEL low when INT is asserted will
    return the address and cause INT to be de-asserted, and reading PIO
    with PSEL low when INT is not asserted will return the data word.
    The DSP program will only respond to one external interrupt per
    sample interval (i.e. the maximum command rate is the same as the
    sample rate).

    The DSP program uses 2 kilowords of internal RAM and reads data from
    external ROM while executing from internal ROM.  As such, it
    requires a DSP16A core (the original DSP16 only has 512 words of
    internal RAM and can't read external ROM with internal ROM enabled).

    To read external ROM, the DSP writes the desired sample offset to
    PDX0, then reads external ROM at address (bank | 0x8000), for a
    theoretical maximum of 2 gigasamples.  The bank applies to the next
    read, not the current read.  A dummy read is required to set the
    bank for the very first read.  This latency could just be a quirk of
    how Capcom hooks the DSP up to the sample ROMs.  In theory, samples
    are 16-bit signed values, but Capcom only has 8-bit ROMs connected.
    I'm assuming byte smearing, but it may be zero-padded in the LSBs.

    The DSP sends out 16-bit samples on its SIO port clocked at 5 MHz.
    The stereo samples aren't loaded fast enough for consecutive frames
    so there's an empty frame between them.  Sample pairs are loaded
    every 1,248 machine cycles, giving a sample rate of 24.03846 kHz
    (60 MHz / 2 / 1248).  The glue logic seems to generate the WS signal
    for the DAC from the PSEL line and the SIO control lines, but it
    isn't clear exactly how this is achieved.

    The DSP writes values to pdx1 every sample cycle (alternating
    between zero and non-zero values).  This may be for the volume
    control chip or something else.

    The photographs of the DL-1425 die (WEDSP16A-M14) show 12 kilowords
    of internal ROM compared to 4 kilowords as documented.  It's unknown
    if/how the additional ROM is mapped in the DSP's internal ROM space.
    The internal program only uses internal ROM from 0x0000 to 0x0fff
    and external space from 0x8000 onwards.  The additional ROM could
    be anywhere in between.

    Meanings for known command words:
    (((ch - 1) << 3) & 0x78     sample bank
    (ch << 3) | 0x01            channel sample offset within bank
    (ch << 3) | 0x02            channel playback rate
    (ch << 3) | 0x03            channel sample period counter
    (ch << 3) | 0x04            channel loop offset (relative to end)
    (ch << 3) | 0x05            channel end sample offset
    (ch << 3) | 0x06            channel volume
    ch | 0x80                   left/right position on sound stage
    0x93                        delayed reverb volume
    ch + 0xba                   channel reverb contribution
    0xd9                        reverb delay (need to add 0x0554)
    0xde                        left output filtered component delay
    0xdf                        left output unfiltered component delay
    0xe0                        right output filtered component delay
    0xe1                        right output unfiltered component delay
    0xe2                        write non-zero to set delays
    0xe4                        left output filtered component volume
    0xe5                        left output unfiltered component volume
    0xe6                        right output filtered component volume
    0xe7                        right output unfiltered component volume

    The weird way of setting the sample bank is due to the one-read
    latency described above.  Since the bank applies to the next read,
    you need to set it on the channel before the desired channel.

    Links:
    * https://siliconpr0n.org/map/capcom/dl-1425

***************************************************************************/

#include "emu.h"
#define QSOUND_LLE
#include "qsound.h"

#include <algorithm>
#include <fstream>

#define LOG_COMMAND     (1U << 1)
#define LOG_SAMPLE      (1U << 2)

//#define VERBOSE (LOG_GENERAL | LOG_COMMAND | LOG_SAMPLE)
//#define LOG_OUTPUT_STREAM std::cout
#include "logmacro.h"

#define LOGCOMMAND(...)     LOGMASKED(LOG_COMMAND, __VA_ARGS__)
#define LOGSAMPLE(...)      LOGMASKED(LOG_SAMPLE, __VA_ARGS__)


// device type definition
DEFINE_DEVICE_TYPE(QSOUND, qsound_device, "qsound", "QSound")


// DSP internal ROM region
ROM_START( qsound )
	ROM_REGION16_BE( 0x2000, "dsp", 0 )
	ROM_LOAD16_WORD_SWAP( "dl-1425.bin", 0x0000, 0x2000, CRC(d6cf5ef5) SHA1(555f50fe5cdf127619da7d854c03f4a244a0c501) )
	ROM_IGNORE( 0x4000 )
ROM_END


//-------------------------------------------------
//  qsound_device - constructor
//-------------------------------------------------

qsound_device::qsound_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, QSOUND, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, device_rom_interface(mconfig, *this)
	, m_dsp(*this, "dsp"), m_stream(nullptr)
	, m_rom_bank(0U), m_rom_offset(0U), m_cmd_addr(0U), m_cmd_data(0U), m_new_data(0U), m_cmd_pending(0U), m_dsp_ready(1U)
	, m_samples{ 0, 0 }, m_sr(0U), m_fsr(0U), m_ock(1U), m_old(1U), m_ready(0U), m_channel(0U)
{
}

qsound_device::~qsound_device()
{
}


void qsound_device::qsound_w(offs_t offset, u8 data)
{
	switch (offset)
	{
	case 0:
		LOGCOMMAND(
				"QSound: set command data[h] = %02X (%04X -> %04X)\n",
				data, m_new_data, (m_new_data & 0x00ffU) | (u16(data) << 8));
		m_new_data = (m_new_data & 0x00ffU) | (u16(data) << 8);
		break;
	case 1:
		LOGCOMMAND(
				"QSound: set command data[l] = %02X (%04X -> %04X)\n",
				data, m_new_data, (m_new_data & 0xff00U) | data);
		m_new_data = (m_new_data & 0xff00U) | data;
		break;
	case 2:
		m_dsp_ready = 0U;
		machine().scheduler().synchronize(
				timer_expired_delegate(FUNC(qsound_device::set_cmd), this),
				(unsigned(data) << 16) | m_new_data);
		break;
	default:
		logerror("QSound: host write to unknown register %01X = %02X (%s)\n", offset, data, machine().describe_context());
	}
}


u8 qsound_device::qsound_r()
{
	return m_dsp_ready ? 0x80 : 0x00;
}


//-------------------------------------------------
//  rom_region - return a pointer to the device's
//  internal ROM region
//-------------------------------------------------

const tiny_rom_entry *qsound_device::device_rom_region() const
{
	return ROM_NAME( qsound );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void qsound_device::device_add_mconfig(machine_config &config)
{
	DSP16A(config, m_dsp, DERIVED_CLOCK(1, 1));
	m_dsp->set_addrmap(AS_IO, &qsound_device::dsp_io_map);
	m_dsp->ock_cb().set(FUNC(qsound_device::dsp_ock_w));
	m_dsp->pio_r_cb().set(FUNC(qsound_device::dsp_pio_r));
	m_dsp->pio_w_cb().set(FUNC(qsound_device::dsp_pio_w));
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void qsound_device::device_start()
{
	// hope we get good synchronisation between the DSP and the sound system
	m_stream = stream_alloc(0, 2, clock() / 2 / 1248);

	// save DSP communication state
	save_item(NAME(m_rom_bank));
	save_item(NAME(m_rom_offset));
	save_item(NAME(m_cmd_addr));
	save_item(NAME(m_cmd_data));
	save_item(NAME(m_new_data));
	save_item(NAME(m_cmd_pending));
	save_item(NAME(m_dsp_ready));

	// save serial sample recovery state
	save_item(NAME(m_samples));
	save_item(NAME(m_sr));
	save_item(NAME(m_fsr));
	save_item(NAME(m_ock));
	save_item(NAME(m_old));
	save_item(NAME(m_ready));
	save_item(NAME(m_channel));
}

//-------------------------------------------------
//  device_clock_changed
//-------------------------------------------------

void qsound_device::device_clock_changed()
{
	m_stream->set_sample_rate(clock() / 2 / 1248);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void qsound_device::device_reset()
{
	// TODO: does this get automatically cleared on reset or not?
	m_cmd_pending = 0U;
	m_dsp_ready = 1U;
	m_dsp->set_input_line(DSP16_INT_LINE, CLEAR_LINE);
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void qsound_device::sound_stream_update(sound_stream &stream)
{
	stream.fill(0, sound_stream::sample_t(m_samples[0]) * (1.0 / 32768.0));
	stream.fill(1, sound_stream::sample_t(m_samples[1]) * (1.0 / 32768.0));
}


//-------------------------------------------------
//  rom_bank_post_change - called after the ROM
//  bank is changed
//-------------------------------------------------

void qsound_device::rom_bank_post_change()
{
	machine().scheduler().synchronize();
}


// DSP external ROM space
void qsound_device::dsp_io_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).mirror(0x8000).r(FUNC(qsound_device::dsp_sample_r));
}


u16 qsound_device::dsp_sample_r(offs_t offset)
{
	// on CPS2, bit 0-7 of external ROM data is tied to ground
	u8 const byte(read_byte((u32(m_rom_bank) << 16) | m_rom_offset));
	if (!machine().side_effects_disabled())
		m_rom_bank = (m_rom_bank & 0x8000U) | offset;
	return u16(byte) << 8;
}

void qsound_device::dsp_ock_w(int state)
{
	// detect active edge
	if (bool(state) == bool(m_ock))
		return;
	m_ock = state;
	if (!state)
		return;

	// detect start of word
	if (m_ready && !m_fsr && !m_dsp->ose_r())
	{
		// FIXME: PSEL at beginning of word seems to select channel, but how does the logic derive WS from the DSP outputs?
		m_channel = m_dsp->psel_r();
		m_fsr = 0xffffU;
	}

	// shift in data
	if (m_fsr)
	{
		m_sr = (m_sr << 1) | (m_dsp->do_r() ? 0x0001U : 0x0000U);
		m_fsr >>= 1;
		if (!m_fsr)
		{
			LOGSAMPLE("QSound: recovered channel %u sample %04X\n", m_channel, m_sr);
			if (!m_channel)
				m_stream->update();
			m_samples[m_channel] = m_sr;
#if 0 // enable to log PCM to a file - can be imported with "ffmpeg -f s16be -ar 24038 -ac 2 -i qsound.pcm qsound.wav"
			static std::ofstream logfile("qsound.pcm", std::ios::binary);
			logfile.put(u8(m_sr >> 8));
			logfile.put(u8(m_sr));
#endif
		}
	}

	// detect falling OLD - indicates next bit could be start of a word
	u8 const old(m_dsp->old_r());
	m_ready = (m_old && !old);
	m_old = old;
}

void qsound_device::dsp_pio_w(offs_t offset, u16 data)
{
	// PDX0 is used for QSound ROM offset, and PDX1 is used for ADPCM ROM offset
	// this prevents spurious PSEL transitions between sending samples to the DAC
	// it could still be used to have separate QSound/ADPCM ROM banks
	m_rom_bank = (m_rom_bank & 0x7fffU) | u16(offset << 15);
	m_rom_offset = data;
}


u16 qsound_device::dsp_pio_r()
{
	LOGCOMMAND(
			"QSound: DSP PIO read returning %s = %04X\n",
			m_cmd_pending ? "addr" : "data", m_cmd_pending ? m_cmd_addr : m_cmd_data);
	if (m_cmd_pending)
	{
		m_cmd_pending = 0U;
		m_dsp->set_input_line(DSP16_INT_LINE, CLEAR_LINE);
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(qsound_device::set_dsp_ready), this));
		return m_cmd_addr;
	}
	else
	{
		return m_cmd_data;
	}
}

void qsound_device::set_dsp_ready(s32 param)
{
	m_dsp_ready = 1U;
}

void qsound_device::set_cmd(s32 param)
{
	/*
	 *  I don't believe the data word is actually double-buffered in
	 *  real life.  In practice it works because the DSP's instruction
	 *  throughput is so much higher than the Z80's that it can always
	 *  read the data word before the Z80 can realise it's read the
	 *  address.
	 *
	 *  In MAME, there's a scheduler synchronisation barrier when the
	 *  DSP reads the address but before it reads the data.  When this
	 *  happens, MAME may give the Z80 enough time to see that the DSP
	 *  has read the address and write more data before scheduling the
	 *  DSP again.  The DSP then reads the new data and stores it at
	 *  the old command address.
	 *
	 *  You can see this happening in megaman2 test mode by playing
	 *  command 0x11 (Gyro Man's theme).  Within two minutes, some
	 *  channels' sample banks/offsets will likely be overwritten.
	 */
	LOGCOMMAND("QSound: DSP command @%02X = %04X\n", u32(param) >> 16, u16(u32(param)));
	m_cmd_addr = u16(u32(param) >> 16);
	m_cmd_data = u16(u32(param));
	m_cmd_pending = 1U;
	m_dsp->set_input_line(DSP16_INT_LINE, ASSERT_LINE);
}
