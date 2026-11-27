// license:BSD-3-Clause
// copyright-holders:AJR, Carl Lom, giulioz, superctr
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

    Interpolation: each output sample sums six words around the read
    position, weighted by a windowed sinc. The mask ROM holds half of it,
    3 x 128 phases of 12-bit sign-magnitude words, read from a die shot;
    72 words not legible there are fitted. Guessed, not measured: the six
    words are consecutive addresses (a loop point does not wrap them),
    reverse mode puts the fraction on the other side of the read word, and
    the sum is truncated to the sample's scale, the output saturating.

    Voice registers (regs800, selected through the d001+2n window, data
    through ports 2/3). Derived from the register usage of the W-30 system
    1.10 firmware (LOBANK2 1137-1336, 0871, 0c02) and confirmed by the same
    write sequence in the S-330 system. Voice v uses regs v*0x10 + 0..6;
    its envelope registers sit one block higher, at (v+1)*0x10 + 3/7/8.
    The registers are one SRAM of 16 blocks of 9 rows: the select decodes
    4 bits of block and 4 of index, indices 9-f hold nothing, and voice
    15's envelope registers (103/107/108 as the firmware addresses them)
    are rows 3, 7 and 8 of block 0.

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
                 doubling of speed); a target written with rate 0 is
                 taken at once. Reads give the current level.
      env reg 7  read: current envelope level (firmware waits for 0)
      env reg 8  written 8080 by the S-50 and S-550 at every release and
                 0 at a voice kill, the target after a rate 0 write by the
                 S-330 and W-30; no effect modelled

    Key-on is the 16-bit mask in ports 0/1 (bit per voice).

    Envelope events: when a ramp reaches its target the voice is flagged
    and ENVINT (HSI.0 on the host) goes low while any voice is flagged; a
    target written with rate 0 raises none.
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
    access, read or write, to e001 + 2n (word n). That access also sets the
    c809/c80b sample port to word n + 1, from where it counts on within the
    4K-word block, wrapping; c011 leaves it. The firmware loads a disk's
    blocks after selecting word fff, one c011 write and 4096 words a block,
    and zeroes a take's last eight words after selecting ff7.

    The S-330 confirms the layout: a sample loaded at word 0x1e000 is
    programmed as anchor 0x225ef (mode bits 2-3 = 2), counter 0x45ef << 14,
    loop length 0x2fd.

    Sampling, from the W-30's sampling code: port 6 bit 0 turns the input
    on and bit 1 records. Each frame takes the input as a 12-bit sample;
    port 0 bit 0 then reads 1 until the port 5 read takes it (ports 4/5
    read it while the input is on). Recording writes it to wave RAM at
    the read address of voice 12, the voice the firmware sets up as a
    forward tone, which is then silent. Port 7 is written 01 by the boot
    ROM.

****************************************************************************/

#include "emu.h"
#include "roland_sa16.h"

#include <algorithm>
#include <cstdlib>
#include <iterator>

#define LOG_REG    (1U << 1) // register accesses
#define LOG_WRAM   (1U << 2) // wave RAM accesses
#define LOG_UNIMPL (1U << 3) // unimplemented register accesses

#define VERBOSE (0)
#include "logmacro.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

namespace {

// h(b + (p + 0.5) / 128) for b = 0-2, p = 0-127, unity 2048
constexpr s16 INTERP_KERNEL[3][128] =
{
	{
		2034, 2033, 2032, 2031, 2029, 2027, 2024, 2021, 2017, 2013, 2008, 2003, 1997, 1991, 1985, 1978,
		1970, 1963, 1954, 1946, 1937, 1927, 1917, 1907, 1896, 1885, 1873, 1861, 1849, 1836, 1823, 1809,
		1795, 1781, 1766, 1752, 1736, 1721, 1705, 1688, 1672, 1655, 1638, 1621, 1603, 1585, 1567, 1548,
		1529, 1510, 1491, 1472, 1452, 1432, 1412, 1392, 1372, 1351, 1331, 1310, 1289, 1268, 1247, 1225,
		1204, 1182, 1161, 1139, 1117, 1095, 1074, 1052, 1030, 1008, 986, 964, 942, 920, 898, 876,
		855, 833, 811, 789, 768, 746, 725, 704, 682, 661, 640, 620, 599, 578, 558, 538,
		518, 498, 478, 458, 439, 420, 401, 382, 364, 345, 327, 309, 292, 274, 257, 240,
		224, 207, 191, 175, 160, 144, 129, 114, 100, 86, 72, 58, 45, 31, 19, 6,
	},
	{
		-6, -18, -30, -41, -52, -63, -73, -83, -93, -103, -112, -121, -129, -138, -146, -154,
		-161, -168, -175, -182, -188, -194, -200, -205, -210, -215, -220, -224, -228, -232, -236, -239,
		-242, -245, -247, -250, -252, -254, -255, -257, -258, -259, -260, -260, -260, -260, -260, -260,
		-260, -259, -258, -257, -256, -255, -253, -251, -250, -248, -246, -243, -241, -239, -236, -233,
		-230, -227, -224, -221, -218, -215, -211, -208, -204, -201, -197, -193, -189, -185, -181, -177,
		-173, -169, -165, -161, -157, -153, -149, -145, -140, -136, -132, -128, -124, -120, -115, -111,
		-107, -103, -99, -95, -91, -87, -83, -79, -75, -71, -67, -64, -60, -56, -53, -49,
		-46, -42, -39, -36, -33, -29, -26, -23, -20, -17, -14, -12, -9, -6, -4, -1,
	},
	{
		1, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 23, 25, 26, 28, 29,
		30, 32, 33, 34, 35, 36, 37, 38, 39, 39, 40, 41, 41, 42, 42, 42,
		43, 43, 43, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 43, 43, 43,
		43, 42, 42, 42, 41, 41, 40, 40, 39, 39, 38, 38, 37, 37, 36, 36,
		35, 34, 34, 33, 32, 32, 31, 31, 30, 29, 29, 28, 27, 27, 26, 25,
		25, 24, 23, 23, 22, 21, 21, 20, 19, 19, 18, 18, 17, 16, 16, 15,
		15, 14, 14, 13, 12, 12, 11, 11, 10, 10, 9, 9, 8, 8, 7, 7,
		6, 6, 6, 5, 5, 4, 4, 3, 3, 3, 2, 2, 1, 1, 1, 0,
	},
};

} // anonymous namespace

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
	, m_space_chanregs_config("chanregs", ENDIANNESS_LITTLE, 16, 9, 0, address_map_constructor(FUNC(sa16_base_device::chanregs_map), this))
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
			switch (offset)
			{
			case 0: return m_active_channels & 0xff;
			case 1: return m_active_channels >> 8;
			case 2: return reg_r(m_reg800_roffset) & 0xff;
			case 3: return reg_r(m_reg800_roffset) >> 8;
			case 6: return m_sampling;
			case 7: return m_port7;
			case 8: return m_block;
			default: return u8(0);
			}
		}),
		NAME([this] (offs_t offset, u8 data) {
			switch (offset)
			{
			case 0: m_active_channels = (m_active_channels & 0xff00) | data; break;
			case 1: m_active_channels = (m_active_channels & 0x00ff) | (data << 8); break;
			case 6: m_sampling = data; break;
			case 7: m_port7 = data; break;
			case 8: m_block = data; break;
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
	map(0x000, 0x1ff).lrw16(
		NAME([this] (offs_t offset) -> u16 { return reg_r(offset); }),
		NAME([this] (offs_t offset, u16 data)
		{
			if (const int r = row(offset); r >= 0)
				m_regs[r] = data;
		})
	);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sa16_base_device::device_start()
{
	// Set up wave RAM cache for sound_stream_update
	space(AS_WRAM).cache(m_wram_cache);

	// Allocate sound stream: the sampling input, one output per voice at
	// 30 kHz (clock / 896). The voices are mixed (and filtered) by the TVF
	// (MB654419U) or the board.
	m_stream = stream_alloc(1, NUM_VOICES, clock() / CLOCK_DIVIDER);

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
	std::fill(std::begin(m_regs), std::end(m_regs), 0);
	m_sampling = 0;
	m_port7 = 0;
	m_input_latch = 0;
	m_input_ready = false;

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
	save_item(NAME(m_regs));
	save_item(NAME(m_sampling));
	save_item(NAME(m_port7));
	save_item(NAME(m_input_latch));
	save_item(NAME(m_input_ready));
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
	std::fill(std::begin(m_regs), std::end(m_regs), 0);
	m_active_channels = 0;
	m_env_pending = 0;
	m_sampling = 0;
	m_port7 = 0;
	m_input_latch = 0;
	m_input_ready = false;
	for (int v = 0; v < NUM_VOICES; v++)
	{
		m_voice[v].m_env_level = 0;
		m_voice[v].m_env_frac = 0;
		m_voice[v].m_env_armed = false;
		m_voice[v].m_alt_back = false;
	}

	update_int();
}


//-------------------------------------------------
//  voice register helpers
//-------------------------------------------------

int sa16_base_device::row(offs_t address)
{
	const int index = address & 0x0f;
	return (index < ROWS_PER_BLOCK) ? (BIT(address, 4, 4) * ROWS_PER_BLOCK + index) : -1;
}

u16 sa16_base_device::reg_r(offs_t address) const
{
	const int r = row(address);
	return (r < 0) ? 0 : m_regs[r];
}

u32 sa16_base_device::voice_counter(int voice) const
{
	return u32(m_regs[voice_reg(voice, 1)]) << 16 | m_regs[voice_reg(voice, 2)];
}

void sa16_base_device::set_voice_counter(int voice, u32 counter)
{
	m_regs[voice_reg(voice, 1)] = counter >> 16;
	m_regs[voice_reg(voice, 2)] = counter & 0xffff;
}

void sa16_base_device::raise_env_event(int voice)
{
	m_voice[voice].m_env_armed = false;
	const bool was_idle = !m_env_pending;
	m_env_pending |= 1 << voice;
	if (was_idle)
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
		if (BIT(m_sampling, 0))
		{
			m_input_latch = std::clamp<s32>(s32(stream.get(0, i) * 2048.0), -0x800, 0x7ff) & 0xfff;
			m_input_ready = true;
		}

		for (int v = 0; v < NUM_VOICES; v++)
		{
			voice_t &voice = m_voice[v];

			// --- Envelope ---
			// [rate:target] in env reg 3. The rate is signed (negative while falling);
			// its magnitude is logarithmic: 3-bit mantissa, 4-bit exponent.
			const u16 r3 = m_regs[env_reg(v, 3)];
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

			const u16 mode   = m_regs[voice_reg(v, 4)];
			const u32 pitch  = m_regs[voice_reg(v, 0)];
			const u32 anchor = u32(m_regs[voice_reg(v, 6)]) | (u32(BIT(mode, 2, 2)) << 16);
			const s64 loop   = s64(u32(m_regs[voice_reg(v, 5)]) | (u32(BIT(mode, 4, 2)) << 16)) << 14;

			// The counter is advanced in a wider type, so that passing the end
			// or the loop point can't overflow, and reduced to 32 bits after
			s64 counter = voice_counter(v);

			const u32 offset = u32(counter >> 14);
			const u32 phase = u32(counter >> 7) & 0x7f;
			const u32 bank = u32(BIT(mode, 0, 2)) << 18;
			const u32 addr = (BIT(mode, 7) ? anchor + offset : anchor - offset) & 0x3ffff;
			const u32 word = bank | addr;

			if (BIT(m_sampling, 1) && v == RECORD_VOICE)
			{
				m_wram_cache.write_word(word << 1, m_input_latch);
				stream.put(v, i, 0);
			}
			else
			{
				// the read word and the two beyond it in phase order, the three before
				// it in the inverted phase
				const int ahead = BIT(mode, 7) ? -1 : 1;
				s32 sum = 0;
				for (int t = 0; t < 3; t++)
				{
					sum += wave_sample(bank | ((addr + ahead * t) & 0x3ffff)) * INTERP_KERNEL[t][phase];
					sum += wave_sample(bank | ((addr - ahead * (t + 1)) & 0x3ffff)) * INTERP_KERNEL[t][phase ^ 0x7f];
				}
				const s32 sample = sum >> 11;
				// 12-bit sample x 8-bit level -> 16 bits per voice
				stream.put_int_clamp(v, i, (sample * voice.m_env_level) >> 4, 32768);
			}

			// Advance: the counter runs down to the end, then loops back
			if (voice.m_alt_back)
			{
				counter += pitch; // alternate loop, playing backwards towards the loop point
				if (counter >= loop)
				{
					counter = loop * 2 - counter;
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
						counter += loop;
				}
			}
			set_voice_counter(v, u32(counter));
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
	if (offset >= 0x1000)
	{
		if (!machine().side_effects_disabled())
		{
			m_blockoffset = (offset - 0x1000) << 1;
			m_smpcounter = (offset - 0x1000 + 1) & 0xfff;
		}
		return 0;
	}
	else if (offset >= 0x800) // 800h+ Set register offset
	{
		if (!machine().side_effects_disabled())
		{
			m_reg800_roffset = offset - 0x800;
			LOGMASKED(LOG_REG, "%s: offsetRegister => %03x\n", machine().describe_context(), m_reg800_roffset);
		}
		return 0;
	}

	switch(offset)
	{
	case 0: // Play tone/channel; bit 0: an input sample is waiting, while sampling
		if (BIT(m_sampling, 0))
		{
			if (!machine().side_effects_disabled())
				m_stream->update();
			value = (m_active_channels & 0xfe) | (m_input_ready ? 1 : 0);
		}
		else
		{
			value = m_active_channels;
		}
		LOGMASKED(LOG_REG, "%s: active_channels[0..7] => %02x\n", machine().describe_context(), value);
		break;
	case 1: // Envelope event: (voice + 1) of the lowest flagged voice, 0 if none.
		// No side effect: the flag is cleared by writing the voice's envelope
		// registers (the firmware reads this port a second time as a dummy).
		if (!machine().side_effects_disabled())
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
			if (!machine().side_effects_disabled())
				m_stream->update();
			const int reg = m_reg800_roffset & 0x0f;

			if (reg == 0x03 || reg == 0x07)
			{
				// Envelope reg 3 (low byte) and reg 7 read back the current level.
				// The firmware waits for reg 7 == 0 before reusing a voice and uses
				// it as the loudness when stealing voices.
				value = m_voice[(BIT(m_reg800_roffset, 4, 4) + NUM_VOICES - 1) & (NUM_VOICES - 1)].m_env_level;
			}
			else
			{
				value = reg_r(m_reg800_roffset);
			}
		}
		LOGMASKED(LOG_REG, "%s: regs800[%03x].lo => %02x\n", machine().describe_context(), m_reg800_roffset, value);
		break;
	case 3:
		if (!machine().side_effects_disabled())
			m_stream->update();
		value = reg_r(m_reg800_roffset) >> 8;
		LOGMASKED(LOG_REG, "%s: regs800[%03x].hi => %02x\n", machine().describe_context(), m_reg800_roffset, value);
		break;
	case 4: // Waveram port low (12-bit word, left-justified: low nibble reads 0), or the input sample
		if (BIT(m_sampling, 0))
		{
			if (!machine().side_effects_disabled())
				m_stream->update();
			value = u8(m_input_latch << 4);
		}
		else
		{
			value = u8(wram_r16(block_base() + m_blockoffset) << 4);
		}
		LOGMASKED(LOG_WRAM, "%s: WRAM[%08x].lo => %02x\n", machine().describe_context(), block_base() + m_blockoffset, value);
		break;
	case 5: // Waveram port high, or the input sample, which the read takes
		if (BIT(m_sampling, 0))
		{
			if (!machine().side_effects_disabled())
			{
				m_stream->update();
				m_input_ready = false;
			}
			value = u8(m_input_latch >> 4);
		}
		else
		{
			value = u8(wram_r16(block_base() + m_blockoffset) >> 4);
		}
		LOGMASKED(LOG_WRAM, "%s: WRAM[%08x].hi => %02x\n", machine().describe_context(), block_base() + m_blockoffset, value);
		break;
	case 6:
		value = m_sampling;
		break;
	case 7:
		value = m_port7;
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
			m_smpcounter = (m_smpcounter + 1) & 0xfff;
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
	if (offset >= 0x1000) // 1000h+: Set offset within block
	{
		m_blockoffset = (offset - 0x1000) << 1;
		m_smpcounter = (offset - 0x1000 + 1) & 0xfff;
		return;
	}
	else if (offset >= 0x800) // 800h+ Set "800"-register number to be written using regs800[] port below
	{
		m_reg800_woffset = offset - 0x800;
		LOGMASKED(LOG_REG, "%s: offsetRegister <= %02x\n", machine().describe_context(), m_reg800_woffset);
		return;
	}

	switch(offset)
	{
	case 0: // Key-on mask, voices 0-7
		m_stream->update();
		m_active_channels = (m_active_channels & 0xff00) | data;
		LOGMASKED(LOG_REG, "%s: active_channels[0..7] <= %02x\n", machine().describe_context(), data);
		break;
	case 1: // Key-on mask, voices 8-15
		m_stream->update();
		m_active_channels = (m_active_channels & 0x00ff) | (data << 8);
		LOGMASKED(LOG_REG, "%s: active_channels[8..F] <= %02x\n", machine().describe_context(), data);
		break;
	case 2:
		LOGMASKED(LOG_REG, "%s: regs800[%03x].lo <= %02x\n", machine().describe_context(), m_reg800_woffset, data);
		m_stream->update();
		if (const int r = row(m_reg800_woffset); r >= 0)
			m_regs[r] = (0xff00 & m_regs[r]) | data;
		break;
	case 3:
		LOGMASKED(LOG_REG, "%s: regs800[%03x].hi <= %02x\n", machine().describe_context(), m_reg800_woffset, data);
		m_stream->update();
		if (const int r = row(m_reg800_woffset); r >= 0)
		{
			m_regs[r] = (0x00ff & m_regs[r]) | (data << 8);

			// The high byte completes a register write
			const int block = BIT(m_reg800_woffset, 4, 4);
			const int reg   = m_reg800_woffset & 0x0f;
			if (reg == 0x03 || reg == 0x07 || reg == 0x08)
			{
				// Writing the envelope acknowledges the voice's pending event
				const int v = (block + NUM_VOICES - 1) & (NUM_VOICES - 1);
				voice_t &voice = m_voice[v];
				const bool ramp = BIT(m_regs[r], 8, 8) != 0;
				voice.m_env_armed = reg == 0x03 && ramp; // new target: raise an event when it is reached
				if (reg == 0x03 && !ramp)
				{
					voice.m_env_level = m_regs[r] & 0xff;
					voice.m_env_frac = 0;
				}
				if (BIT(m_env_pending, v))
				{
					m_env_pending &= ~(1 << v);
					if (!m_env_pending)
						update_int();
				}
			}
			if (reg == 0x01 || reg == 0x02)
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
	case 6:
		LOGMASKED(LOG_REG, "%s: sampling <= %02x\n", machine().describe_context(), data);
		m_stream->update();
		m_sampling = data;
		if (!BIT(data, 0))
			m_input_ready = false;
		break;
	case 7:
		m_port7 = data;
		break;
	case 8:
		LOGMASKED(LOG_REG, "%s: set block = %04x\n", machine().describe_context(), data);
		m_block = data;
		break;
	case 0x404: // Sample port low (16-bit value)
		m_port_smp16 = (0xff00 & m_port_smp16) | data;
		break;
	case 0x405: // Sample port high (16-bit value)
		m_port_smp16 = (0x00ff & m_port_smp16) | data<<8;
		LOGMASKED(LOG_REG, "%s: sample port write[%08x] <= %04x\n", machine().describe_context(), block_base() + (m_smpcounter << 1), m_port_smp16 >> 4);
		wram_w16(block_base() + (m_smpcounter << 1), m_port_smp16 >> 4);
		m_smpcounter = (m_smpcounter + 1) & 0xfff;
		break;
	default:
		LOGMASKED(LOG_UNIMPL, "%s: write to address %04x unimplemented (data=%02x)\n", machine().describe_context(), offset, data);
	}
}
