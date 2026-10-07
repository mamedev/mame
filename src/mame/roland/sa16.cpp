// license:BSD-3-Clause
// copyright-holders:AJR, Carl Lom
/****************************************************************************

    Roland RF5C36 (15229840) & SA-16 (15229874) Sampler Custom ICs

    SA-16 voice playback and envelopes are emulated from the register usage
    of the S-330 and W-30 firmware. The RF5C36 shares the implementation but
    hasn't been checked against S-50/S-550 firmware.

    Waveform data is 12 bits, and is normally stored in DRAM banks, though
    at least one Roland product also uses ROMs. 16-bit output can be
    connected directly to a PCM54 or MD6209 DAC or conditioned through a
    MB654419 TVF interface.

    Sampling rate is either 30kHz or 15kHz.

    Voice registers (regs800, selected through the d001+2n window, data
    through ports 2/3). Derived from the register usage of the W-30 system
    1.10 firmware (LOBANK2 1137-1336, 0871, 0c02) and confirmed by the same
    write sequence in the S-330 system. Voice v uses regs v*0x10 + 0..6;
    its envelope registers sit one block higher, at (v+1)*0x10 + 3/7/8.

      reg 0      pitch, 2.14 (0x4000 = original pitch)
      reg 1:2    32-bit address counter, 18.14 (reg 1 = high word): the
                 distance from the sample end, counting down. The firmware
                 writes (end - start) << 14.
      reg 4      mode: bits 0-1 wave RAM bank (CAS 0-3, 256K words each),
                 bits 2-3 = anchor bits 16-17, bits 4-5 = loop length bits
                 16-17, bit 6 alternate loop, bit 7 reverse
      reg 5      loop length: when the counter passes 0 (the end) it is
                 wound back by this many samples. One-shot and reverse
                 tones use 4 (a short loop at the end, keyed off by the
                 firmware).
      reg 6      anchor address (bits 0-15): the sample end. The read
                 address is anchor - counter (anchor + counter in reverse
                 mode, where the anchor is start + 4).
      env reg 3  [rate:target]; rate is signed, logarithmic (8 steps per
                 doubling of speed). Reads give the current level.
      env reg 7  read: current envelope level (firmware waits for 0)
      env reg 8  write: set the envelope level immediately

    Key-on is the 16-bit mask in ports 0/1 (bit per voice).

    Envelope events: when a ramp reaches its target the voice is flagged
    and ENVINT (HSI.0 on the host) goes low while any voice is flagged.
    Port 1 reads return (voice + 1) of the lowest flagged voice (0 when
    none) without side effects; writing the voice's env reg 3, 7 or 8
    clears its flag. The S-330 and W-30 envelope services read port 1, program
    the next segment, then read it again as a dummy before re-checking
    ENVINT, so a read cannot be the acknowledge. Inferred from firmware
    usage, not measured.

    Wave RAM ports 4/5 (c009/c00b) access the 12-bit sample word
    left-justified in 16 bits (low nibble reads 0, as the c809/c80b sample
    port); the W-30 packs 3 bytes into 2 words relying on this. The word is
    selected by c011 (block = bank << 6 | address >> 12, 4K words) and an
    access, read or write, to e001 + 2n (word n).

    The S-330 confirms the layout: a sample loaded at word 0x1e000 is
    programmed as anchor 0x225ef (mode bits 2-3 = 2), counter 0x45ef << 14,
    loop length 0x2fd.

****************************************************************************/

#include "emu.h"
#include "sa16.h"

#define LOG_REG    (1U << 1) // register accesses
#define LOG_WRAM   (1U << 2) // wave RAM accesses
#define LOG_UNIMPL (1U << 3) // unimplemented register accesses

#define VERBOSE (0)
#include "logmacro.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definitions
DEFINE_DEVICE_TYPE(RF5C36, rf5c36_device, "rf5c36", "Roland RF5C36 Sampler")
DEFINE_DEVICE_TYPE(SA16, sa16_device, "sa16", "Roland SA-16 Wave Gate Array")

// default address map
void sa16_base_device::sa16(address_map &map)
{
	// The chip addresses wave memory in 256K-word banks (CAS0-3), selected per
	// voice by mode register bits 0-1; this map is the linear concatenation.
	if (!has_configured_map(0))
		map(0x000000, 0x1fffff).ram(); // total address bus width is 20 bits (1M 12-bit words)
}

//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector sa16_base_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_WRAM,     &m_space_config),
		std::make_pair(AS_REGS,     &m_space_regs_config),
		std::make_pair(AS_CHANREGS, &m_space_chanregs_config)
	};
}

//**************************************************************************
//  DEVICE IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  sa16_base_device - constructor
//-------------------------------------------------

sa16_base_device::sa16_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, device_memory_interface(mconfig, *this)
	, m_space_config("waveram", ENDIANNESS_LITTLE, 16, 21, 0, address_map_constructor(FUNC(sa16_base_device::sa16), this))
	, m_space_regs_config("regs", ENDIANNESS_LITTLE, 8, 4, 0, address_map_constructor(FUNC(sa16_base_device::regs_map), this))
	, m_space_chanregs_config("chanregs", ENDIANNESS_LITTLE, 16, 11, 0, address_map_constructor(FUNC(sa16_base_device::chanregs_map), this))
	, m_int_callback(*this)
	, m_sh_callback(*this)
	, m_stream(nullptr)
{
}


//-------------------------------------------------
//  rf5c36_device - constructor
//-------------------------------------------------

rf5c36_device::rf5c36_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: sa16_base_device(mconfig, RF5C36, tag, owner, clock)
{
}


//-------------------------------------------------
//  sa16_device - constructor
//-------------------------------------------------

sa16_device::sa16_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: sa16_base_device(mconfig, SA16, tag, owner, clock)
{
}


//-------------------------------------------------
//  regs_map - address map for direct registers
//  (visible as "regs" in debugger memory view)
//-------------------------------------------------

void sa16_base_device::regs_map(address_map &map)
{
	map(0x0, 0xf).lrw8(
		NAME([this] (offs_t offset) -> u8 {
			switch (offset) {
			case 0: return m_active_channels & 0xff;
			case 1: return m_active_channels >> 8;
			case 2: return m_regs800[m_reg800_roffset] & 0xff;
			case 3: return m_regs800[m_reg800_roffset] >> 8;
			case 8: return m_block;
			default: return u8(0);
			}
		}),
		NAME([this] (offs_t offset, u8 data) {
			switch (offset) {
			case 0: m_active_channels = (m_active_channels & 0xff00) | data; break;
			case 1: m_active_channels = (m_active_channels & 0x00ff) | (data << 8); break;
			case 8: m_block = data; m_smpcounter = 0; break;
			}
		})
	);
}

//-------------------------------------------------
//  chanregs_map - address map for channel registers
//  (visible as "chanregs" in debugger memory view)
//-------------------------------------------------

void sa16_base_device::chanregs_map(address_map &map)
{
	map(0x000, 0x7ff).lrw16(
		NAME([this] (offs_t offset) -> u16 { return m_regs800[offset]; }),
		NAME([this] (offs_t offset, u16 data) { m_regs800[offset] = data; })
	);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sa16_base_device::device_start()
{
	// Set up wave RAM cache for sound_stream_update
	space(AS_WRAM).cache(m_wram_cache);

	// Allocate sound stream: one output per voice at 30 kHz (clock / 896).
	// The voices are mixed (and filtered) by the TVF (MB654419U) or the board.
	m_stream = stream_alloc(0, NUM_VOICES, clock() / CLOCK_DIVIDER);

	// Envelope events are detected inside the stream update; without this the
	// interrupt could be up to one sound batch (~10 ms) late, which made chained
	// release segments (one per firmware interrupt) far too slow.
	m_update_timer = timer_alloc(FUNC(sa16_base_device::update_tick), this);
	const attotime period = attotime::from_ticks(16 * CLOCK_DIVIDER, clock());
	m_update_timer->adjust(period, 0, period);

	m_active_channels = 0;
	m_env_pending = 0;
	m_reg800_woffset = 0;
	m_reg800_roffset = 0;
	m_blockoffset = 0;
	m_smpcounter = 0;
	m_port_smp16 = 0;
	m_block = 0;
	std::fill(std::begin(m_regs800), std::end(m_regs800), 0);

	for (int v = 0; v < NUM_VOICES; v++)
	{
		m_voice[v].m_env_level = 0;
		m_voice[v].m_env_frac = 0;
		m_voice[v].m_env_armed = false;
		m_voice[v].m_alt_back = false;
	}

	save_item(NAME(m_active_channels));
	save_item(NAME(m_env_pending));
	save_item(NAME(m_reg800_woffset));
	save_item(NAME(m_reg800_roffset));
	save_item(NAME(m_blockoffset));
	save_item(NAME(m_smpcounter));
	save_item(NAME(m_port_smp16));
	save_item(NAME(m_block));
	save_item(NAME(m_regs800));
	save_item(STRUCT_MEMBER(m_voice, m_env_level));
	save_item(STRUCT_MEMBER(m_voice, m_env_frac));
	save_item(STRUCT_MEMBER(m_voice, m_env_armed));
	save_item(STRUCT_MEMBER(m_voice, m_alt_back));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sa16_base_device::device_reset()
{
	m_block = 0;
	std::fill(std::begin(m_regs800), std::end(m_regs800), 0);
	m_active_channels = 0;
	m_env_pending = 0;
	for (int v = 0; v < NUM_VOICES; v++)
	{
		m_voice[v].m_env_level = 0;
		m_voice[v].m_env_frac = 0;
		m_voice[v].m_env_armed = false;
		m_voice[v].m_alt_back = false;
	}
}

void sa16_device::device_reset()
{
	sa16_base_device::device_reset();

	// ENVINT (HSI0 on the host CPU) is high while idle. Measured on the S-330 with
	// nothing playing: the line stays high and does not pulse.
	update_int();
}


//-------------------------------------------------
//  voice register helpers
//-------------------------------------------------

s32 sa16_base_device::voice_counter(int voice) const
{
	return s32(u32(m_regs800[voice_reg(voice, 1)]) << 16 | m_regs800[voice_reg(voice, 2)]);
}

void sa16_base_device::set_voice_counter(int voice, s32 counter)
{
	m_regs800[voice_reg(voice, 1)] = u32(counter) >> 16;
	m_regs800[voice_reg(voice, 2)] = u32(counter) & 0xffff;
}

void sa16_base_device::env_level_write(int voice, u8 level)
{
	m_voice[voice].m_env_level = level;
	m_voice[voice].m_env_frac = 0;
}

void sa16_base_device::raise_env_event(int voice)
{
	m_voice[voice].m_env_armed = false;
	m_env_pending |= 1 << voice;
	update_int();
}

TIMER_CALLBACK_MEMBER(sa16_base_device::update_tick)
{
	m_stream->update();
}

void sa16_base_device::update_int()
{
	// ENVINT is active low
	m_int_callback(m_env_pending ? 0 : 1);
}


//-------------------------------------------------
//  sound_stream_update - generate audio output
//-------------------------------------------------

void sa16_base_device::sound_stream_update(sound_stream &stream)
{
	for (int i = 0; i < stream.samples(); i++)
	{
		for (int v = 0; v < NUM_VOICES; v++)
		{
			voice_t &voice = m_voice[v];

			// --- Envelope ---
			// [rate:target] in env reg 3. The rate is signed (negative while falling);
			// its magnitude is logarithmic: 3-bit mantissa, 4-bit exponent.
			const u16 r3 = m_regs800[env_reg(v, 3)];
			const u8 target = r3 & 0xff;
			const u8 mag = std::abs(s8(r3 >> 8)) & 0x7f;

			if (voice.m_env_level != target && mag != 0)
			{
				// Rate law checked against an S-330
				// Attack and release timings match: rising and falling ramps use the same law.
				voice.m_env_frac += u32(8 | (mag & 7)) << (mag >> 3);
				u32 steps = voice.m_env_frac >> ENV_RATE_SHIFT;
				voice.m_env_frac &= (1 << ENV_RATE_SHIFT) - 1;
				if (voice.m_env_level < target)
					voice.m_env_level = std::min<u32>(target, voice.m_env_level + steps);
				else
					voice.m_env_level = std::max<s32>(target, s32(voice.m_env_level) - s32(steps));
			}
			if (voice.m_env_armed && voice.m_env_level == target)
				raise_env_event(v);

			// --- Wave playback (key-on mask) ---
			if (!BIT(m_active_channels, v))
			{
				stream.put(v, i, 0);
				continue;
			}

			const u16 mode   = m_regs800[voice_reg(v, 4)];
			const u32 pitch  = m_regs800[voice_reg(v, 0)];
			const u32 anchor = u32(m_regs800[voice_reg(v, 6)]) | (u32(BIT(mode, 2, 2)) << 16);
			const s32 loop   = s32(u32(m_regs800[voice_reg(v, 5)]) | (u32(BIT(mode, 4, 2)) << 16));
			s32 counter = voice_counter(v);

			const s32 offset = counter >> 14;
			const u32 addr = (BIT(mode, 7) ? anchor + offset : anchor - offset) & 0x3ffff;
			const u32 word = (u32(BIT(mode, 0, 2)) << 18) | addr;

			// Read 12-bit sample from wave RAM, sign-extend to 16-bit
			const s16 sample = s16(m_wram_cache.read_word(word << 1) << 4) >> 4;
			// 12-bit sample x 8-bit level -> 16 bits per voice
			stream.put_int(v, i, (s32(sample) * voice.m_env_level) >> 4, 32768);

			// Advance: the counter runs down to the end, then loops back
			if (voice.m_alt_back)
			{
				counter += pitch; // alternate loop, playing backwards towards the loop point
				if (counter >= (loop << 14))
				{
					counter = (loop << 14) * 2 - counter;
					voice.m_alt_back = false;
				}
			}
			else
			{
				counter -= pitch;
				if (counter < 0)
				{
					if (BIT(mode, 6))
					{
						counter = -counter; // alternate: turn around at the end
						voice.m_alt_back = true;
					}
					else
						counter += loop << 14;
				}
			}
			set_voice_counter(v, counter);
		}
	}
}


//-------------------------------------------------
//  read - read data to CPU bus
//-------------------------------------------------

u8 sa16_base_device::read(offs_t offset)
{
	u8 value = -1;

	// 0x1000+: Set byte offset within block. Reads select the word just like
	// writes do (the firmware sets the offset with a dummy read before reading
	// ports 4/5).
	if (offset >= 0x1000) {
		if (!machine().side_effects_disabled())
			m_blockoffset = (offset - 0x1000) << 1;
		return 0;
	} else if (offset >= 0x800) { // 800h+ Set register offset
		if (!machine().side_effects_disabled())
		{
			m_reg800_roffset = offset - 0x800;
			LOGMASKED(LOG_REG, "%s: offsetRegister => %03x\n", machine().describe_context(), m_reg800_roffset);
		}
		return 0;
	}

	switch(offset)
	{
	case 0: // Play tone/channel
		value = m_active_channels;
		LOGMASKED(LOG_REG, "%s: active_channels[0..7] => %02x\n", machine().describe_context(), value);
		break;
	case 1: // Envelope event: (voice + 1) of the lowest flagged voice, 0 if none.
		// No side effect: the flag is cleared by writing the voice's envelope
		// registers (the firmware reads this port a second time as a dummy).
		m_stream->update();
		value = 0;
		if (m_env_pending)
		{
			int voice = 0;
			while (!BIT(m_env_pending, voice))
				voice++;
			value = voice + 1;
		}
		LOGMASKED(LOG_REG, "%s: env event voice => %02x (pending %04x)\n", machine().describe_context(), value, m_env_pending);
		break;
	case 2:
		{
			m_stream->update();
			const int block = m_reg800_roffset >> 4;
			const int reg   = m_reg800_roffset & 0x0F;

			if (block >= 1 && block <= NUM_VOICES && (reg == 0x03 || reg == 0x07))
			{
				// Envelope reg 3 (low byte) and reg 7 read back the current level.
				// The firmware waits for reg 7 == 0 before reusing a voice and uses
				// it as the loudness when stealing voices.
				value = m_voice[block - 1].m_env_level;
			}
			else
			{
				value = m_regs800[m_reg800_roffset];
			}
		}
		LOGMASKED(LOG_REG, "%s: regs800[%03x].lo => %02x\n", machine().describe_context(), m_reg800_roffset, value);
		break;
	case 3:
		m_stream->update();
		value = m_regs800[m_reg800_roffset] >> 8;
		LOGMASKED(LOG_REG, "%s: regs800[%03x].hi => %02x\n", machine().describe_context(), m_reg800_roffset, value);
		break;
	case 4: // Waveram port low (12-bit word, left-justified: low nibble reads 0)
		value = u8(wram_r16(block_base() + m_blockoffset) << 4);
		LOGMASKED(LOG_WRAM, "%s: WRAM[%08x].lo => %02x\n", machine().describe_context(), block_base() + m_blockoffset, value);
		break;
	case 5: // Waveram port high
		value = u8(wram_r16(block_base() + m_blockoffset) >> 4);
		LOGMASKED(LOG_WRAM, "%s: WRAM[%08x].hi => %02x\n", machine().describe_context(), block_base() + m_blockoffset, value);
		break;
	case 8:
		value = m_block;
		break;
	case 0x404: // Sample port low (16-bit value)
		value = wram_r16(block_base() + (m_smpcounter << 1)) << 4;
		break;
	case 0x405: // Sample port high (16-bit value); advances to the next word
		value = wram_r16(block_base() + (m_smpcounter << 1)) >> 4;
		if (!machine().side_effects_disabled())
			m_smpcounter++;
		break;
	default:
		LOGMASKED(LOG_UNIMPL, "%s: read from address %04x unimplemented\n", machine().describe_context(), offset);
		value = 0;
	}
	return value;
}


//-------------------------------------------------
//  write - write data from CPU bus
//-------------------------------------------------

void sa16_base_device::write(offs_t offset, u8 data)
{
	if (offset >= 0x1000) { // 1000h+: Set offset within block
		m_blockoffset = (offset - 0x1000) << 1;
		return;
	} else if (offset >= 0x800) { // 800h+ Set "800"-register number to be written using regs800[] port below
		m_reg800_woffset = offset - 0x800;
		LOGMASKED(LOG_REG, "%s: offsetRegister <= %02x\n", machine().describe_context(), m_reg800_woffset);
		return;
	}

	switch(offset)
	{
	case 0: // Key-on mask, voices 0-7
		m_stream->update();
		m_active_channels = (m_active_channels & 0xFF00) | data;
		LOGMASKED(LOG_REG, "%s: active_channels[0..7] <= %02x\n", machine().describe_context(), data);
		break;
	case 1: // Key-on mask, voices 8-15
		m_stream->update();
		m_active_channels = (m_active_channels & 0x00FF) | (data << 8);
		LOGMASKED(LOG_REG, "%s: active_channels[8..F] <= %02x\n", machine().describe_context(), data);
		break;
	case 2:
		LOGMASKED(LOG_REG, "%s: regs800[%03x].lo <= %02x\n", machine().describe_context(), m_reg800_woffset, data);
		m_stream->update();
		m_regs800[m_reg800_woffset] = (0xFF00 & m_regs800[m_reg800_woffset]) | data;
		break;
	case 3:
		LOGMASKED(LOG_REG, "%s: regs800[%03x].hi <= %02x\n", machine().describe_context(), m_reg800_woffset, data);
		m_stream->update();
		m_regs800[m_reg800_woffset] = (0x00FF & m_regs800[m_reg800_woffset]) | data<<8;
		{
			// The high byte completes a register write
			const int block = m_reg800_woffset >> 4;
			const int reg   = m_reg800_woffset & 0x0F;
			if (block >= 1 && block <= NUM_VOICES && (reg == 0x03 || reg == 0x07 || reg == 0x08))
			{
				// Writing the envelope acknowledges the voice's pending event
				const int v = block - 1;
				voice_t &voice = m_voice[v];
				if (reg == 0x03)
					voice.m_env_armed = true; // new target: raise an event when it is reached
				else
					voice.m_env_armed = false; // level set directly, no ramp to report
				if (reg == 0x08)
					env_level_write(v, m_regs800[m_reg800_woffset] & 0xff);
				if (BIT(m_env_pending, v))
				{
					m_env_pending &= ~(1 << v);
					update_int();
				}
			}
			if (block < NUM_VOICES && (reg == 0x01 || reg == 0x02))
				m_voice[block].m_alt_back = false; // new address counter
		}
		break;
	case 4: // Waveram port low: the RAM keeps the top 12 bits of the 16-bit word
	case 5: // Waveram port high
		LOGMASKED(LOG_WRAM, "%s: WRAM[%08x].%s <= %02x\n", machine().describe_context(), block_base() + m_blockoffset, (offset == 4) ? "lo" : "hi", data);
		{
			u16 word = wram_r16(block_base() + m_blockoffset) << 4;
			word = (offset == 4) ? ((word & 0xff00) | data) : ((word & 0x00ff) | (data << 8));
			wram_w16(block_base() + m_blockoffset, word >> 4);
		}
		break;
	case 8:
		LOGMASKED(LOG_REG, "%s: set block = %04x\n", machine().describe_context(), data);
		m_block = data;
		m_smpcounter = 0;
		break;
	case 0x404: // Sample port low (16-bit value)
		m_port_smp16 = (0xFF00 & m_port_smp16) | data;
		break;
	case 0x405: // Sample port high (16-bit value)
		m_port_smp16 = (0x00FF & m_port_smp16) | data<<8;
		LOGMASKED(LOG_REG, "%s: sample port write[%08x] <= %04x\n", machine().describe_context(), block_base() + (m_smpcounter << 1), m_port_smp16 >> 4);
		wram_w16(block_base() + (m_smpcounter << 1), m_port_smp16 >> 4);
		m_smpcounter++;
		break;
	default:
		LOGMASKED(LOG_UNIMPL, "%s: write to address %04x unimplemented (data=%02x)\n", machine().describe_context(), offset, data);
	}
}
