// license:BSD-3-Clause
// copyright-holders:superctr
// thanks-to: giulioz, nukeykt
/*
 *  Roland GP-2/GP-4
 *
 *  Register interface skeleton. The CPU sees the 32 slot register files (28 voices, the
 *  effect processor's work registers), the key mask with its handshake bit and the wave
 *  ROM byte readback, and the keyed bit of the voice control registers follows the
 *  key mask, which is what the firmware needs to initialise and run. The sound
 *  generation itself is not implemented. What follows is what is known about it.
 *
 *  Bus (64 bytes, 8-bit accesses, multi-byte registers go through a write latch
 *  committed on the last byte and a read latch loaded on the first byte and read back
 *  at 0x39-0x3b):
 *    0x00-0x03  W  key mask, big-endian, 28 bits (0x00 bits 3-0 = slots 27-24 ... 0x03 =
 *                  slots 7-0), written into a pending register; R commits the pending
 *                  mask and clears status bit 5
 *    0x04-0x0f, 0x24-0x2f  20-bit slot registers, 0x10-0x1f, 0x30-0x37  16-bit slot
 *                  registers (see the header), all of the slot selected by 0x3e
 *    0x20-0x23  W  wave ROM address, 24 bits big-endian in 0x21-0x23; writing 0x23
 *                  fetches the byte, read at 0x3f
 *    0x39-0x3b  R  read latch bits 19-16, 15-8, 7-0
 *    0x3c       W  output configuration: bits 1-0 DC offset before truncation (1 = half
 *                  LSB, 2 = two LSB), bits 3-2 dither noise bits below the DAC LSB, bits
 *                  5-4 DAC width (0 = 18-bit, else 16-bit; 3 also adds 1 << 12), bit 6
 *                  two samples per frame, bit 7 error feedback of the truncated bits
 *    0x3d       W  bits 4-0 active slot count - 1, bit 5 ROM chip select from address
 *                  bits 23-21 (2 MB per select) instead of 21-19 (512 KB)
 *    0x3e       W  slot select (5 bits)
 *    0x3c/0x3e  R  status: bits 4-0 slot of the last interrupt, bit 5 key mask pending;
 *                  reading 0x3e deasserts the interrupt
 *
 *  Timing: one frame per (slots + 1) * 25 clocks produces one sample pair (two with
 *  the double-rate bit). All arithmetic is 20-bit two's complement with a saturating
 *  adder and 20 x 8 signed multiplies. A 14-bit frame counter, loaded from slot 31's
 *  PHASE register on the first frame after reset, is the effect RAM write pointer and
 *  the envelope clock/dither source. Per frame: output stage on the previous frame's
 *  mix, effect processor (reverb + chorus in the external 16K x 16 effect RAM, whose
 *  words hold a 14-bit mantissa and a 2-bit exponent), the chorus modulator (slot 31's
 *  address generator), then voices 0 .. count-1 in order.
 *
 *  Voice: the phase increment is added to the 14-bit sub-phase, the carry (0-4) is the
 *  number of ROM bytes consumed; the address generator walks four steps ahead comparing
 *  against END (or LOOP running backwards), jumping to LOOP or reversing direction for
 *  the alternate loop. The wave ROM holds 8-bit DPCM deltas from offset 0x8000 of each
 *  1 MB bank; bytes 0-0x7fff are a nibble table with the shift of every 16-byte block
 *  (byte n: low nibble for block n * 32, high nibble for n * 32 + 16), a delta d adds
 *  d << 11 >> ((10 - shift) & 15) to DPCM_REF. The output is a four-tap interpolation
 *  over the deltas at the current and next two addresses with 128-entry weight tables
 *  indexed by sub-phase bits 13-7, then a Chamberlin state variable filter (cutoff from
 *  TVF_LEVEL, resonance from FLAGS, lowpass or highpass output; the GP-2 datapath is
 *  narrower than the GP-4's), then the two amplitude envelopes, pan and the two sends.
 *  Gains (envelope levels, PAN, SEND bytes) apply as sample * (hi + fine / 128) / 64
 *  with hi = bits 14-8 signed and fine = bits 7-1 of the level.
 *
 *  Envelope rate byte: bits 7-6 both set = exponential, moving (distance >> n) towards
 *  target << 7 every 4/16/64/128 frames (bits 5-4) with n = (10 - bits 3-0) & 15;
 *  otherwise linear with a constant step that snaps to the target when it would cross it.
 *
 *  Interrupt: a keyed voice with FLAGS bit 0 set and PHASE bit 14 clear raises the line
 *  once its address passes LOOP (inverted for reverse) when no interrupt is pending; the
 *  slot number appears in the status register and PHASE bit 14 is set for the note.
 *
 *  The SC-55mkII firmware uses 28 slots (0x3d = 0x3b), output config 0xcc (16-bit DAC,
 *  error feedback, double rate) and polls CONTROL bit 5 after a key-on.
 */

#include "emu.h"
#include "roland_gp.h"

#include <algorithm>


namespace {

constexpr u32 with_byte(u32 value, int byte, u8 data)
{
	return (value & ~(0xffu << (byte * 8))) | (u32(data) << (byte * 8));
}

} // anonymous namespace


//**************************************************************************
//  bus interface
//**************************************************************************

void roland_gp_device::write(offs_t offset, u8 data)
{
	m_stream->update();

	const u8 address = offset & 0x3f;
	slot &selected = m_slots[m_selected_slot];

	if (address < 0x04)
	{
		// key mask, applied on the next read of it
		m_key_mask_pending = with_byte(m_key_mask_pending, 3 - address, data) & 0x0fffffff;
		m_key_mask_dirty = true;
	}
	else if (address < 0x10 || (address >= 0x24 && address < 0x30))
	{
		// 20-bit slot registers, three bytes written big-endian through a latch
		const int byte = address & 3;
		if (byte == 0)
			return;
		m_write_latch = with_byte(m_write_latch, 3 - byte, data) & MASK20;
		if (byte == 3)
			selected.wide[(address >> 2) - (address < 0x20 ? 1 : 6)] = m_write_latch;
	}
	else if (address < 0x20 || (address >= 0x30 && address < 0x38))
	{
		// 16-bit slot registers
		const int byte = address & 1;
		m_write_latch = with_byte(m_write_latch, 1 - byte, data);
		if (byte == 1)
			selected.narrow[((address & 0x0f) >> 1) + (address < 0x20 ? 0 : 8)] = u16(m_write_latch);
	}
	else if (address < 0x24)
	{
		// wave ROM address, the byte is fetched when the low byte is written
		if (address != 0x20)
			m_rom_address = with_byte(m_rom_address, 0x23 - address, data);
		if (address == 0x23)
			m_rom_byte = read_byte(m_rom_address & 0xffffff);
	}
	else if (address == 0x3c)
	{
		m_output_config = data;
		update_rate();
	}
	else if (address == 0x3d)
	{
		m_slot_config = data;
		update_rate();
	}
	else if (address == 0x3e)
	{
		m_selected_slot = data & 0x1f;
	}
}

u8 roland_gp_device::read(offs_t offset)
{
	m_stream->update();

	const u8 address = offset & 0x3f;
	const slot &selected = m_slots[m_selected_slot];

	if (address < 0x04)
	{
		if (!machine().side_effects_disabled())
		{
			if (m_key_mask_dirty)
				m_key_mask = m_key_mask_pending;
			m_key_mask_dirty = false;
		}
	}
	else if (address < 0x10 || (address >= 0x24 && address < 0x30))
	{
		if ((address & 3) == 1 && !machine().side_effects_disabled())
			m_read_latch = selected.wide[(address >> 2) - (address < 0x20 ? 1 : 6)] & MASK20;
	}
	else if (address < 0x20 || (address >= 0x30 && address < 0x38))
	{
		if ((address & 1) == 0 && !machine().side_effects_disabled())
			m_read_latch = selected.narrow[((address & 0x0f) >> 1) + (address < 0x20 ? 0 : 8)];
	}
	else if (address >= 0x39 && address <= 0x3b)
	{
		// read latch, big-endian
		return u8(m_read_latch >> ((0x3b - address) * 8)) & (address == 0x39 ? 0x0f : 0xff);
	}
	else if (address == 0x3c || address == 0x3e)
	{
		// status: bits 0-4 = slot of the pending interrupt (none here), bit 5 = key mask pending
		return m_key_mask_dirty ? 0x20 : 0;
	}
	else if (address == 0x3f)
	{
		return m_rom_byte;
	}

	return 0;
}


//**************************************************************************
//  device
//**************************************************************************

DEFINE_DEVICE_TYPE(ROLAND_GP2, roland_gp2_device, "roland_gp2", "Roland GP-2 TC24SC201AF PCM")
DEFINE_DEVICE_TYPE(ROLAND_GP4, roland_gp4_device, "roland_gp4", "Roland GP-4 TC6116AF PCM")

roland_gp_device::roland_gp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, bool gp4)
	: device_t(mconfig, type, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, device_rom_interface(mconfig, *this)
	, m_int_callback(*this)
	, m_gp4(gp4)
	, m_stream(nullptr)
	, m_key_mask(0)
	, m_key_mask_pending(0)
	, m_key_mask_dirty(false)
	, m_write_latch(0)
	, m_read_latch(0)
	, m_rom_address(0)
	, m_rom_byte(0)
	, m_output_config(0)
	, m_slot_config(0)
	, m_selected_slot(0)
{
}

roland_gp2_device::roland_gp2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: roland_gp_device(mconfig, ROLAND_GP2, tag, owner, clock, false)
{
}

roland_gp4_device::roland_gp4_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: roland_gp_device(mconfig, ROLAND_GP4, tag, owner, clock, true)
{
}

void roland_gp_device::device_start()
{
	m_stream = stream_alloc(0, 2, output_rate());

	save_item(STRUCT_MEMBER(m_slots, wide));
	save_item(STRUCT_MEMBER(m_slots, narrow));
	save_item(NAME(m_key_mask));
	save_item(NAME(m_key_mask_pending));
	save_item(NAME(m_key_mask_dirty));
	save_item(NAME(m_write_latch));
	save_item(NAME(m_read_latch));
	save_item(NAME(m_rom_address));
	save_item(NAME(m_rom_byte));
	save_item(NAME(m_output_config));
	save_item(NAME(m_slot_config));
	save_item(NAME(m_selected_slot));
}

void roland_gp_device::device_reset()
{
	for (slot &s : m_slots)
		s = slot();

	m_key_mask = 0;
	m_key_mask_pending = 0;
	m_key_mask_dirty = false;

	m_write_latch = 0;
	m_read_latch = 0;
	m_rom_address = 0;
	m_rom_byte = 0;

	m_output_config = 0;
	m_slot_config = 0;
	m_selected_slot = 0;

	m_int_callback(CLEAR_LINE);
	update_rate();
}

void roland_gp_device::device_clock_changed()
{
	update_rate();
}

void roland_gp_device::rom_bank_pre_change()
{
	m_stream->update();
}


//**************************************************************************
//  output: one frame per (slots + 1) * 25 clocks producing one sample, or two when the
//  double-rate bit is set; the stream runs at the resulting rate and is silent
//**************************************************************************

u32 roland_gp_device::output_rate() const
{
	return (clock() / ((slot_count() + 1) * 25)) * (double_rate() ? 2 : 1);
}

void roland_gp_device::update_rate()
{
	const u32 rate = output_rate();
	if (rate != m_stream->sample_rate())
		m_stream->set_sample_rate(rate);
}

void roland_gp_device::sound_stream_update(sound_stream &stream)
{
	// per frame the chip latches the key mask into the keyed bit of each voice's control register,
	// which the firmware polls after a key-on
	const int voices = std::min(slot_count(), MAX_VOICES);
	for (int frame = 0; frame < stream.samples(); frame += double_rate() ? 2 : 1)
		for (int index = 0; index < voices; index++)
			if (BIT(m_key_mask, index))
				m_slots[index].narrow[CONTROL] |= 0x0020;
}
