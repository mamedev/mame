// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    Intel 82750DB display processor

    Functional scanout of the planar 8-bit YUV mode.  The DB's VRAM transfers
    are serviced by the 82750PB, so the display pointers and VRAM come from it.

    TODO:
    - genlock dot placement and serial VRAM transfer timing
    - 2x Y interpolator, interlaced modes, other pixel formats and the cursor
*/

#include "emu.h"
#include "i82750db.h"

#include <algorithm>
#include <cstdlib>

DEFINE_DEVICE_TYPE(I82750DB, i82750db_device, "i82750db", "Intel 82750DB Display Processor")

i82750db_device::i82750db_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, I82750DB, tag, owner, clock)
	, m_pb(*this, finder_base::DUMMY_TAG)
	, m_shutdown_cb(*this)
	, m_clut{}
	, m_y(0)
	, m_vu(0)
	, m_y_pitch(0)
	, m_vu_pitch(0)
	, m_reset(true)
{
}

void i82750db_device::device_start()
{
	m_pb->space(AS_DATA).cache(m_vram);

	save_item(NAME(m_reg));
	save_item(NAME(m_clut));
	save_item(NAME(m_y));
	save_item(NAME(m_vu));
	save_item(NAME(m_y_pitch));
	save_item(NAME(m_vu_pitch));
	save_item(NAME(m_reset));
	save_item(NAME(m_load));
	save_item(NAME(m_enabled));
	save_item(NAME(m_shutdown));
}

void i82750db_device::device_reset()
{
	std::fill(std::begin(m_reg), std::end(m_reg), 0);
	m_load = true;
	m_enabled = false;
	m_shutdown = false;
	m_shutdown_cb(0);
}

void i82750db_device::reset_w(int state)
{
	// the reset pin is edge triggered and has the same effect as a power-on reset
	if (state && !m_reset)
	{
		device_reset();
	}
	m_reset = bool(state);
}

u32 i82750db_device::pointer(unsigned index) const
{
	// display bitmaps and register loads start on a doubleword boundary
	return (m_pb->pointer_r(index) | (u32(m_pb->pointer_r(index + 1)) << 16)) & ~3;
}

s16 i82750db_device::pitch(unsigned index) const
{
	return s16(m_pb->pointer_r(index) & ~3);
}

void i82750db_device::load_registers()
{
	u32 address = pointer(30);
	if (!address)
	{
		return;
	}

	// a register load only takes effect if its checksum matches
	u32 reg[REG_COUNT];
	u32 clut[CLUT_SIZE];
	std::copy(std::begin(m_reg), std::end(m_reg), std::begin(reg));
	std::copy(std::begin(m_clut), std::end(m_clut), std::begin(clut));

	u32 checksum = 0;
	for (unsigned i = 0; i < 1024; i++, address += 4)
	{
		const u32 data = m_vram.read_dword(address);
		checksum ^= data;

		const unsigned index = data >> 24;
		if (index == 0xff)
		{
			if (checksum == m_vram.read_dword(address + 4))
			{
				std::copy(std::begin(reg), std::end(reg), std::begin(m_reg));
				std::copy(std::begin(clut), std::end(clut), std::begin(m_clut));
				m_load = false;
				m_enabled = true;
			}
			else
			{
				// shut down until the host pulses reset, and notify the PB
				logerror("Register load checksum mismatch at %08x, shutting down\n", address);
				m_enabled = false;
				m_shutdown = true;
				m_shutdown_cb(1);
			}
			return;
		}

		if (index < 0x40)
		{
			clut[(index | reg[0x40]) & 255] = data & 0xffffff;
		}
		else if (index < REG_COUNT)
		{
			reg[index] = data & 0xffffff;
		}
	}
}

void i82750db_device::vblank_w(int state)
{
	// the host swaps buffers during blanking, so latch the new ones at the end of it
	if (state || m_reset || m_shutdown)
	{
		return;
	}

	// initial register load, then the programmed periodic reloads
	if (m_load || (BIT(m_reg[0x49], 12, 12) != 0xfff))
	{
		load_registers();
	}

	m_y = pointer(20);
	m_vu = pointer(26);
	m_y_pitch = pitch(24);
	m_vu_pitch = pitch(28);
}

rgb_t i82750db_device::color(u8 y, u8 v, u8 u) const
{
	const u32 misc = m_reg[0x43];
	const bool video = BIT(misc, 11) && (!BIT(m_reg[0x45], 8) || BIT(BIT(misc, 13) ? y : u, 0));
	if (!video)
	{
		v = u = y;
	}

	if (!BIT(misc, video ? 18 : 17))
	{
		u32 index = y | (u32(v) << 8) | (u32(u) << 16);
		index = (index & ~m_reg[0x41]) | (m_reg[0x42] & m_reg[0x41]);
		if (BIT(misc, 16))
		{
			index = (index & 0xfefefe) | (video ? 0x010101 : 0);
		}
		y = m_clut[index & 255];
		v = m_clut[(index >> 8) & 255] >> 8;
		u = m_clut[(index >> 16) & 255] >> 16;
	}

	if (BIT(misc, 15))
	{
		return rgb_t(v, y, u);
	}

	// CCIR 601 levels to full-range RGB
	const int c = int(y) - 16;
	const int d = int(u) - 128;
	const int e = int(v) - 128;
	return rgb_t(std::clamp((298 * c + 409 * e + 128) >> 8, 0, 255),
			std::clamp((298 * c - 100 * d - 208 * e + 128) >> 8, 0, 255),
			std::clamp((298 * c + 516 * d + 128) >> 8, 0, 255));
}

u8 i82750db_device::chroma(u32 address, unsigned x, unsigned y, unsigned samples, unsigned rows, unsigned step, s32 pitch)
{
	const unsigned sx = std::min(x / step, samples - 1);
	const unsigned sy = std::min(y / step, rows - 1);
	const unsigned nx = std::min(sx + 1, samples - 1);
	const unsigned ny = std::min(sy + 1, rows - 1);
	const unsigned h = x % step;
	const unsigned v = y % step;
	const u32 row = address + s32(sy) * pitch;
	const u32 next_row = address + s32(ny) * pitch;

	const unsigned a = m_vram.read_byte(row + sx);
	const unsigned b = m_vram.read_byte(row + nx);
	const unsigned c = m_vram.read_byte(next_row + sx);
	const unsigned d = m_vram.read_byte(next_row + nx);
	const unsigned numerator = a * (step - h) * (step - v) + b * h * (step - v) + c * (step - h) * v + d * h * v;

	unsigned result = numerator / (step * step);
	if (BIT(m_reg[0x45], 7) && (numerator % (step * step)))
	{
		result |= 1;
	}
	return result;
}

void i82750db_device::draw(bitmap_rgb32 &bitmap, const rectangle &cliprect, const rectangle &visible)
{
	if (!m_enabled || m_reset || m_shutdown || !m_y_pitch || (BIT(m_reg[0x45], 19, 3) != 1))
	{
		return;
	}

	static constexpr unsigned SAMPLE_COUNTS[8] = { 0, 256, 192, 160, 128, 96, 64, 32 };
	const unsigned samples = SAMPLE_COUNTS[BIT(m_reg[0x45], 11, 3)];
	const unsigned step = BIT(m_reg[0x45], 10) ? 4 : 2;
	const unsigned width = samples ? (samples * step) : unsigned(std::abs(m_y_pitch));
	const unsigned start = BIT(m_reg[0x4e], 12, 12);
	const unsigned stop = BIT(m_reg[0x4f], 12, 12);
	if ((stop <= start) || !width)
	{
		return;
	}

	const unsigned height = stop - start;
	const unsigned rows = (height + step - 1) / step;

	// V and U rows alternate, so each component's pitch is twice the VU pitch
	const s32 vu_pitch = m_vu_pitch * 2;

	// TODO: active video currently fills the host viewport
	for (int y = std::max(cliprect.top(), visible.top()); y <= std::min(cliprect.bottom(), visible.bottom()); y++)
	{
		const unsigned sy = (y - visible.top()) * height / visible.height();
		const u32 line = m_y + s32(sy) * m_y_pitch;
		u32 *const dest = &bitmap.pix(y);
		for (int x = std::max(cliprect.left(), visible.left()); x <= std::min(cliprect.right(), visible.right()); x++)
		{
			const unsigned sx = (x - visible.left()) * width / visible.width();
			const u8 luma = m_vram.read_byte(line + sx);
			u8 v = 0;
			u8 u = 0;
			if (samples)
			{
				v = chroma(m_vu, sx, sy, samples, rows, step, vu_pitch);
				u = chroma(m_vu + m_vu_pitch, sx, sy, samples, rows, step, vu_pitch);
			}
			dest[x] = color(luma, v, u);
		}
	}
}
