// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
// Hantro G1 register interface. H.264 and its post-processor are the only
// codecs/features currently targeted. IGS38 routes their shared IRQ to IRQ26.
// Register layout: original Hantro DWL client and Linux hantro_g1_regs.h.
#include "emu.h"
#include "hantro_g1.h"

#include "multibyte.h"

#include "openh264/codec/api/wels/codec_api.h"

#include <algorithm>
#include <memory>
#include <vector>

#define LOG_REGS (1U << 1)
#define LOG_JOBS (1U << 2)
#define VERBOSE (0)
#include "logmacro.h"

#define LOGREGS(...) LOGMASKED(LOG_REGS, __VA_ARGS__)
#define LOGJOBS(...) LOGMASKED(LOG_JOBS, __VA_ARGS__)

namespace {

// Reconstruct progressive 8-bit 4:2:0 SPS/PPS NAL units from the G1 registers.
class rbsp_writer
{
public:
	void bits(u32 value, u32 count)
	{
		while (count--)
		{
			if (!(m_bits & 7))
				m_data.push_back(0);
			m_data.back() |= BIT(value, count) << (7 - (m_bits++ & 7));
		}
	}

	void ue(u32 value)
	{
		u32 count = 0;
		for (u32 n = value + 1; n >>= 1;)
			++count;
		bits(0, count);
		bits(value + 1, count + 1);
	}

	void se(int value) { ue(value <= 0 ? -2 * value : 2 * value - 1); }

	void nal(std::vector<u8> &out, u8 header)
	{
		bits(1, 1);
		while (m_bits & 7)
			bits(0, 1);
		out.insert(out.end(), { 0, 0, 0, 1, header });
		u32 zeros = 0;
		for (auto b : m_data)
		{
			if (zeros == 2 && b <= 3)
			{
				out.push_back(3);
				zeros = 0;
			}
			out.push_back(b);
			zeros = b ? 0 : zeros + 1;
		}
	}

private:
	std::vector<u8> m_data;
	u32 m_bits = 0;
};

bool parameter_sets(u32 const *r, std::vector<u8> &out)
{
	u32 const width = (r[4] >> 23) & 511, height = (r[4] >> 11) & 255;
	u32 const frame_bits = (r[7] >> 16) & 31, poc_bits = r[9] & 255;
	u32 const refs = r[4] & 31, l0 = (r[9] >> 14) & 31, l1 = (r[9] >> 19) & 31;
	if (!width || !height || width > 120 || height > 68 || refs > 16 || frame_bits < 4 || frame_bits > 16 || poc_bits < 4 ||
			poc_bits > 16 || !l0 || !l1 || (r[3] & 0x08802400) || (r[5] & 0x01000001) || (r[7] & 0x40000000))
		return false; // RLC, interlace, MBAFF, MVC, custom matrices or monochrome.
	u32 const qp = (r[6] >> 25) & 63;
	if (qp > 51)
		return false;
	auto const signed5 = [] (u32 v)
	{
		return util::sext(v, 5);
	};
	rbsp_writer s;
	s.bits(100, 8);
	s.bits(0, 8);
	s.bits(42, 8); // High profile, level 4.2
	s.ue(0);
	s.ue(1);
	s.ue(0);
	s.ue(0);
	s.bits(0, 1);
	s.bits(0, 1);
	s.ue(frame_bits - 4);
	s.ue(0);
	s.ue(poc_bits - 4);
	s.ue(refs);
	s.bits(0, 1);
	s.ue(width - 1);
	s.ue(height - 1);
	s.bits(1, 1);
	s.bits((r[7] >> 29) & 1, 1);
	s.bits(0, 1);
	s.bits(0, 1);
	s.nal(out, 0x67);
	rbsp_writer p;
	p.ue(r[9] >> 24);
	p.ue(0);
	p.bits(BIT(r[7], 31), 1);
	p.bits(0, 1);
	p.ue(0);
	p.ue(l0 - 1);
	p.ue(l1 - 1);
	p.bits((r[7] >> 28) & 1, 1);
	p.bits((r[7] >> 26) & 3, 2);
	p.se(int(qp) - 26);
	p.se(0);
	p.se(signed5(r[5] >> 19));
	p.bits((r[8] >> 30) & 1, 1);
	p.bits(BIT(r[8], 31), 1);
	p.bits((r[8] >> 29) & 1, 1);
	p.bits((r[8] >> 28) & 1, 1);
	p.bits(0, 1);
	p.se(signed5(r[5] >> 14));
	p.nal(out, 0x68);
	return true;
}

} // anonymous namespace

DEFINE_DEVICE_TYPE(HANTRO_G1, hantro_g1_device, "hantro_g1", "Hantro G1 H.264 decoder")

hantro_g1_device::hantro_g1_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, HANTRO_G1, tag, owner, clock)
	, m_dma(*this, finder_base::DUMMY_TAG, -1)
	, m_irq(*this)
	, m_decoder(nullptr)
{
}

void hantro_g1_device::device_start()
{
	m_dma->cache(m_dma_access);
	m_irq_state = false;
	m_irq(0);
	m_decode_timer = timer_alloc(FUNC(hantro_g1_device::decode_done), this);
	m_pp_timer = timer_alloc(FUNC(hantro_g1_device::pp_done), this);
	m_history = make_unique_clear<u8[]>(HISTORY_SIZE);
	save_item(NAME(m_regs));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_history_size));
	save_item(NAME(m_history_units));
	save_item(NAME(m_jobs));
	save_pointer(NAME(m_history), HISTORY_SIZE);
}

void hantro_g1_device::device_reset()
{
	std::fill(std::begin(m_regs), std::end(m_regs), 0);
	m_history_size = m_history_units = m_jobs = 0;
	m_decode_timer->adjust(attotime::never);
	m_pp_timer->adjust(attotime::never);
	update_irq();
	reset_decoder();
}

void hantro_g1_device::device_stop()
{
	release_decoder();
}

void hantro_g1_device::release_decoder()
{
	if (m_decoder)
	{
		m_decoder->Uninitialize();
		WelsDestroyDecoder(m_decoder);
		m_decoder = nullptr;
	}
}

bool hantro_g1_device::reset_decoder()
{
	release_decoder();
	if (WelsCreateDecoder(&m_decoder))
		return false;
	SDecodingParam params{};
	params.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_AVC;
	params.eEcActiveIdc = ERROR_CON_DISABLE;
	return !m_decoder->Initialize(&params);
}

bool hantro_g1_device::decode_packet(u8 const *data, u32 size, u8 **planes, u32 &width, u32 &height, u32 *strides)
{
	if (!m_decoder)
		return false;
	SBufferInfo info{};
	auto const result = m_decoder->DecodeFrameNoDelay(data, size, planes, &info);
	if (result || info.iBufferStatus != 1)
	{
		logerror("H.264 decode error=%x picture=%d\n", u32(result), info.iBufferStatus);
		return false;
	}
	width = info.UsrData.sSystemBuffer.iWidth;
	height = info.UsrData.sSystemBuffer.iHeight;
	strides[0] = info.UsrData.sSystemBuffer.iStride[0];
	strides[1] = strides[2] = info.UsrData.sSystemBuffer.iStride[1];
	return true;
}

void hantro_g1_device::device_post_load()
{
	// Reconstruct the host codec's reference pictures from the saved NAL units.
	bool valid = reset_decoder();
	u32 pos = 0, units = 0;
	while (valid && pos + 4 <= m_history_size)
	{
		u32 const size = get_u32le(m_history.get() + pos);
		pos += 4;
		if (!size || size > m_history_size - pos)
		{
			valid = false;
			break;
		}
		u8 *planes[3]{};
		u32 width, height, strides[3];
		valid = decode_packet(m_history.get() + pos, size, planes, width, height, strides);
		pos += size;
		++units;
	}
	if (!valid || pos != m_history_size || units != m_history_units)
		fatalerror("Hantro G1: could not reconstruct saved H.264 reference pictures");
}

void hantro_g1_device::map(address_map &map)
{
	map(0, 0x3ff).rw(FUNC(hantro_g1_device::read), FUNC(hantro_g1_device::write));
}

void hantro_g1_device::update_irq()
{
	bool const state = ((m_regs[1] & 0x110) == 0x100) || ((m_regs[60] & 0x110) == 0x100);
	if (state != m_irq_state)
	{
		m_irq_state = state;
		m_irq(state);
	}
}

u32 hantro_g1_device::read(offs_t offset)
{
	switch (offset)
	{
	case 0:
		return 0x81900000; // G1 product family; silicon revision unverified.
	case 50:
		return (3U << 24) | 1920; // H.264 High Profile, max width 1920.
	case 54:
		return 0; // No extra codec, tiled output, MVC or reference cache.
	case 57:
		return 0x80008000; // H.264 enabled, width fuse 1920.
	case 99:
		return 0x80008000; // Post-processor enabled, width fuse 1920.
	case 100:
		return 0x00010000 | 1920;
	default:
		return m_regs[offset];
	}
}

void hantro_g1_device::write(offs_t offset, u32 data, u32 mem_mask)
{
	if (!offset || offset == 50 || offset == 54 || offset == 57 || offset == 99 || offset == 100)
		return;
	u32 const old = m_regs[offset];
	COMBINE_DATA(&m_regs[offset]);
	LOGREGS("%s: write %03x=%08x mask=%08x\n", machine().describe_context(), offset * 4, data, mem_mask);
	if (offset == 1)
	{
		if (BIT(m_regs[1], 0) && !BIT(old, 0))
		{
			++m_jobs;
			LOGJOBS("decode %u mode=%u stream=%08x bit=%u length=%u output=%08x\n", m_jobs, m_regs[3] >> 28, m_regs[12], m_regs[5] >> 26,
					m_regs[6] & 0xffffff, m_regs[13]);
			m_decode_timer->adjust(attotime::from_usec(10));
		}
		else if (!BIT(m_regs[1], 0))
			m_decode_timer->adjust(attotime::never);
	}
	if (offset == 60)
	{
		if (BIT(m_regs[60], 0) && !BIT(old, 0))
			m_pp_timer->adjust(attotime::from_usec(10));
		else if (!BIT(m_regs[60], 0))
			m_pp_timer->adjust(attotime::never);
	}
	update_irq();
}

TIMER_CALLBACK_MEMBER(hantro_g1_device::decode_done)
{
	auto const fail = [this] (char const *reason)
	{
		logerror("decode %u failed: %s\n", m_jobs, reason);
		m_regs[1] = (m_regs[1] & ~1U) | 0x10100;
		if (BIT(m_regs[60], 1))
			m_regs[60] = (m_regs[60] & ~1U) | 0x2100;
		update_irq();
	};
	std::vector<u8> packet;
	if ((m_regs[3] >> 28) || !parameter_sets(m_regs, packet))
		return fail("unsupported codec or H.264 parameter set");
	u32 const length = m_regs[6] & 0xffffff, start = (m_regs[5] >> 26) / 8;
	if ((m_regs[5] >> 26) & 7 || start >= length)
		return fail("invalid stream bit position/length");
	// Little-endian software uses both stream endian/swap controls set. The
	// alternate settings permute bytes within each 64-bit input bus beat.
	u32 const swap = (BIT(m_regs[2], 21) ? 0 : 3) | (BIT(m_regs[2], 22) ? 0 : 4);
	std::vector<u8> stream;
	bool vcl = false, idr = false;
	u32 consumed = length;
	for (u32 p = start; p < length; ++p)
	{
		stream.push_back(m_dma_access.read_byte(m_regs[12] + (p ^ swap)));
		if (p == start && !BIT(m_regs[6], 31))
		{
			vcl = (stream[0] & 31) == 1 || (stream[0] & 31) == 5;
			idr = (stream[0] & 31) == 5;
		}
		size_t const n = stream.size();
		// Recognise a complete NAL prefix, header and first RBSP byte. A VCL
		// beginning with first_mb_in_slice=0 starts the next decoded picture.
		if (n >= 5 && stream[n - 5] == 0 && stream[n - 4] == 0 && stream[n - 3] == 1)
		{
			u32 const type = stream[n - 2] & 31;
			bool const next_picture = (type == 1 || type == 5) && BIT(stream[n - 1], 7);
			if (vcl && (next_picture || type == 6 || type == 7 || type == 8 || type == 9 || type == 10 || type == 11))
			{
				size_t boundary = n - 5;
				if (boundary && stream[boundary - 1] == 0)
					--boundary;
				consumed = start + boundary;
				stream.resize(boundary);
				break;
			}
			if (type == 1 || type == 5)
			{
				vcl = true;
				idr |= type == 5;
			}
		}
	}
	if (!BIT(m_regs[6], 31))
	{
		if (stream.empty() || ((stream[0] & 31) != 1 && (stream[0] & 31) != 5))
			return fail("raw NAL input has no supported slice");
		vcl = true;
		idr = (stream[0] & 31) == 5;
		packet.insert(packet.end(), { 0, 0, 0, 1 });
	}
	if (!vcl)
		return fail("no H.264 picture in supplied DMA range");
	if (BIT(m_regs[6], 31))
	{
		// SPS/PPS fields come from the register bank, not in-stream NAL units.
		size_t nal = 0;
		while (nal < stream.size())
		{
			size_t header = nal;
			while (header + 2 < stream.size() && !(stream[header] == 0 && stream[header + 1] == 0 && stream[header + 2] == 1))
				++header;
			if (header + 3 >= stream.size())
				break;
			header += 3;
			size_t end = header + 1;
			while (end + 2 < stream.size() && !(stream[end] == 0 && stream[end + 1] == 0 && stream[end + 2] == 1))
				++end;
			if (end + 2 >= stream.size())
				end = stream.size();
			else if (end > header && stream[end - 1] == 0)
				--end;
			u32 const type = stream[header] & 31;
			if (type != 7 && type != 8)
			{
				packet.insert(packet.end(), { 0, 0, 0, 1 });
				packet.insert(packet.end(), stream.begin() + header, stream.begin() + end);
			}
			nal = end;
		}
	}
	else
		packet.insert(packet.end(), stream.begin(), stream.end());
	if (idr)
	{
		if (!reset_decoder())
			return fail("could not initialise H.264 backend");
		m_history_size = m_history_units = 0;
	}
	if (packet.size() + 4 > HISTORY_SIZE - m_history_size)
		return fail("reference history exceeds supported GOP buffer");
	u8 *planes[3]{};
	u32 width, height, strides[3];
	if (!decode_packet(packet.data(), packet.size(), planes, width, height, strides))
		return fail("invalid or unsupported H.264 picture");
	if (width != ((m_regs[4] >> 23) & 511) * 16 || height != ((m_regs[4] >> 11) & 255) * 16)
		return fail("decoded picture dimensions differ from programmed dimensions");
	put_u32le(m_history.get() + m_history_size, packet.size());
	m_history_size += 4;
	std::copy(packet.begin(), packet.end(), m_history.get() + m_history_size);
	m_history_size += packet.size();
	++m_history_units;
	if (!BIT(m_regs[3], 15))
	{
		u32 const out = m_regs[13];
		u32 const outswap = (BIT(m_regs[2], 8) ? 0 : 3) | (BIT(m_regs[2], 19) ? 0 : 4);
		auto const write_pixels = [this, out, outswap] (u32 offset, u32 pixels)
		{
			if (!(out & 3))
				m_dma_access.write_dword(out + (offset ^ (outswap & 4)), (outswap & 3) ? swapendian_int32(pixels) : pixels);
			else
				for (u32 i = 0; i < 4; ++i)
					m_dma_access.write_byte(out + ((offset + i) ^ outswap), pixels >> (i * 8));
		};
		for (u32 y = 0; y < height; ++y)
			for (u32 x = 0; x < width; x += 4)
			{
				u8 const *p = planes[0] + y * strides[0] + x;
				write_pixels(y * width + x, u32(p[0]) | (u32(p[1]) << 8) | (u32(p[2]) << 16) | (u32(p[3]) << 24));
			}
		for (u32 y = 0; y < height / 2; ++y)
			for (u32 x = 0; x < width / 2; x += 2)
			{
				u8 const *cb = planes[1] + y * strides[1] + x, *cr = planes[2] + y * strides[2] + x;
				write_pixels(width * height + y * width + x * 2, u32(cb[0]) | (u32(cr[0]) << 8) | (u32(cb[1]) << 16) | (u32(cr[1]) << 24));
			}
	}
	if (BIT(m_regs[60], 1))
	{
		if (!post_process(planes, width, height, strides))
			return fail("unsupported post-processor configuration");
		m_regs[60] = (m_regs[60] & ~1U) | 0x1100;
	}
	m_regs[12] += consumed; // The guest computes consumed bytes from this pointer.
	m_regs[5] &= 0x03ffffff;
	m_regs[1] = (m_regs[1] & ~1U) | 0x1100;
	LOGJOBS("decoded %u %ux%u consumed=%u remaining=%u\n", m_jobs, width, height, consumed - start, length - consumed);
	update_irq();
}

TIMER_CALLBACK_MEMBER(hantro_g1_device::pp_done)
{
	bool const ready = post_process();
	m_regs[60] = (m_regs[60] & ~1U) | (ready ? 0x1100 : 0x2100);
	update_irq();
}

bool hantro_g1_device::post_process(u8 const *const *planes, u32 width, u32 height, u32 const *strides)
{
	u32 const in_width = (m_regs[72] & 511) * 16;
	u32 const in_height = ((m_regs[72] >> 9) & 255) * 16;
	u32 const out_width = (m_regs[85] >> 4) & 2047;
	u32 const out_height = (m_regs[85] >> 15) & 2047;
	u32 const in_format = m_regs[85] >> 29, out_format = (m_regs[85] >> 26) & 7;
	u32 const pitch = (m_regs[92] & 4095) ? m_regs[92] & 4095 : out_width;
	// Only progressive NV12 to RGB without cropping, rotation, masks or contrast adjustment.
	if (!in_width || !in_height || in_width > 1920 || in_height > 1088 || in_width != out_width || in_height != out_height ||
			pitch < out_width || in_format != 1 || out_format != 0 || (m_regs[71] & 0x3ffc0000) || (m_regs[72] & 0xff000000) ||
			(m_regs[79] & 0xc0000000) || (m_regs[88] & 0x00400000) || (m_regs[89] & 0x00400000) || (m_regs[90] & 0x3c000000) ||
			(m_regs[68] & 0x000fffff) || ((m_regs[69] >> 8) & 1023) != ((m_regs[69] >> 18) & 1023))
	{
		logerror("PP unsupported in=%ux%u fmt=%u out=%ux%u fmt=%u pitch=%u\n", in_width, in_height, in_format, out_width, out_height,
				out_format, pitch);
		return false;
	}
	if (planes && (width != in_width || height != in_height))
		return false;
	u32 const bytes = BIT(m_regs[79], 28) ? 2 : 4;
	u32 const a = (m_regs[69] >> 8) & 1023;
	u32 const b = m_regs[70] & 1023, c = (m_regs[70] >> 10) & 1023;
	u32 const d = (m_regs[70] >> 20) & 1023, e = m_regs[71] & 1023;
	int const brightness = s8(m_regs[71] >> 10);
	u32 const alpha = m_regs[82] & m_regs[83] & m_regs[84];
	u32 const padding[3] = { (m_regs[79] >> 23) & 31, (m_regs[79] >> 18) & 31, (m_regs[80] >> 18) & 31 };
	u32 const inswap = (BIT(m_regs[61], 7) ? 0 : 3) | (BIT(m_regs[61], 10) ? 0 : 4);
	u32 const outswap = (BIT(m_regs[61], 6) ? 0 : 3) | (BIT(m_regs[61], 5) ? 0 : 4);
	u32 const source_pitch = ((m_regs[88] >> 23) & 511) * 16;
	if (!planes && source_pitch < in_width)
		return false;
	auto const channel = [] (int value) -> u32
	{
		return std::clamp(value >> 8, 0, 255);
	};
	for (u32 y = 0; y < out_height; ++y)
		for (u32 x = 0; x < out_width; ++x)
		{
			int luma, cb, cr;
			if (planes)
			{
				luma = planes[0][y * strides[0] + x];
				cb = planes[1][(y / 2) * strides[1] + x / 2] - 128;
				cr = planes[2][(y / 2) * strides[2] + x / 2] - 128;
			}
			else
			{
				luma = m_dma_access.read_byte(m_regs[63] + ((y * source_pitch + x) ^ inswap));
				u32 const chroma = (y / 2) * source_pitch + (x & ~1U);
				cb = m_dma_access.read_byte(m_regs[64] + (chroma ^ inswap)) - 128;
				cr = m_dma_access.read_byte(m_regs[64] + ((chroma + 1) ^ inswap)) - 128;
			}
			// Colour conversion coefficients are unsigned 8.8 values.
			int const yy = int(a) * (luma - (BIT(m_regs[79], 29) ? 0 : 16)) + brightness * 256;
			u32 const rgb[3] = { channel(yy + int(b) * cr), channel(yy - int(c) * cr - int(d) * cb), channel(yy + int(e) * cb) };
			u32 pixel = alpha;
			for (u32 k = 0; k < 3; ++k)
				pixel |= ((rgb[k] << 24) >> padding[k]) & (m_regs[82 + k] & ~alpha);
			if (bytes == 2)
				pixel >>= 16;
			u32 const offset = (y * pitch + x) * bytes;
			u32 const address = m_regs[66] + (offset ^ (outswap & ~(bytes - 1)));
			if (bytes == 4 && !(address & 3))
				m_dma_access.write_dword(address, (outswap & 3) ? swapendian_int32(pixel) : pixel);
			else if (bytes == 2 && !(address & 1))
				m_dma_access.write_word(address, (outswap & 1) ? swapendian_int16(u16(pixel)) : u16(pixel));
			else
				for (u32 k = 0; k < bytes; ++k)
					m_dma_access.write_byte(m_regs[66] + ((offset + k) ^ outswap), pixel >> (k * 8));
		}
	LOGJOBS("PP %ux%u RGB%u output=%08x\n", out_width, out_height, bytes * 8, m_regs[66]);
	return true;
}
