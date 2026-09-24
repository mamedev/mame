// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
// Hantro G1 register interface. H.264 and its post-processor are the only
// codecs/features currently targeted. IGS38 routes their shared IRQ to IRQ26.
// Register layout: original Hantro DWL client and Linux hantro_g1_regs.h.
#include "emu.h"
#include "hantro_g1.h"
#include "hantro_g1_h264.h"
#include "openh264/codec/api/wels/codec_api.h"

#define LOG_REGS (1U << 1)
#define LOG_JOBS (1U << 2)
#define VERBOSE (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(HANTRO_G1, hantro_g1_device, "hantro_g1", "Hantro G1 H.264 decoder")

hantro_g1_device::hantro_g1_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: device_t(mconfig, HANTRO_G1, tag, owner, clock)
		, m_dma(*this, finder_base::DUMMY_TAG, -1)
		, m_irq(*this)
{
}

void hantro_g1_device::device_start()
{
	m_decode_timer = timer_alloc(FUNC(hantro_g1_device::decode_done), this);
	m_pp_timer = timer_alloc(FUNC(hantro_g1_device::pp_done), this);
	m_history = std::make_unique<u8[]>(16 * 1024 * 1024);
	save_item(NAME(m_regs));
	save_item(NAME(m_history_size));
	save_item(NAME(m_history_units));
	save_item(NAME(m_jobs));
	save_pointer(NAME(m_history), 16 * 1024 * 1024);
	machine().save().register_postload(save_prepost_delegate(FUNC(hantro_g1_device::postload), this));
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
	if (m_decoder)
	{
		m_decoder->Uninitialize();
		WelsDestroyDecoder(m_decoder);
		m_decoder = nullptr;
	}
}

bool hantro_g1_device::reset_decoder()
{
	device_stop();
	if (WelsCreateDecoder(&m_decoder))
		return false;
	SDecodingParam params{};
	params.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_AVC;
	params.eEcActiveIdc = ERROR_CON_DISABLE;
	return !m_decoder->Initialize(&params);
}

bool hantro_g1_device::decode_packet(u8 const *data, u32 size, u8 **planes, unsigned &width, unsigned &height, unsigned *strides)
{
	if (!m_decoder)
		return false;
	SBufferInfo info{};
	auto const result = m_decoder->DecodeFrameNoDelay(data, size, planes, &info);
	if (result || info.iBufferStatus != 1)
	{
		logerror("H.264 decode error=%x picture=%d\n", unsigned(result), info.iBufferStatus);
		return false;
	}
	width = info.UsrData.sSystemBuffer.iWidth;
	height = info.UsrData.sSystemBuffer.iHeight;
	strides[0] = info.UsrData.sSystemBuffer.iStride[0];
	strides[1] = strides[2] = info.UsrData.sSystemBuffer.iStride[1];
	return true;
}

void hantro_g1_device::postload()
{
	// Reconstruct only host codec references. DMA and interrupts belong to the
	// saved device state and must not be repeated during this replay.
	bool valid = reset_decoder();
	u32 pos = 0, units = 0;
	while (valid && pos + 4 <= m_history_size)
	{
		u32 const size =
				u32(m_history[pos]) | (u32(m_history[pos + 1]) << 8) | (u32(m_history[pos + 2]) << 16) | (u32(m_history[pos + 3]) << 24);
		pos += 4;
		if (!size || size > m_history_size - pos)
		{
			valid = false;
			break;
		}
		u8 *planes[3]{};
		unsigned width, height, strides[3];
		valid = decode_packet(m_history.get() + pos, size, planes, width, height, strides);
		pos += size;
		++units;
	}
	if (!valid || pos != m_history_size || units != m_history_units)
		fatalerror("Hantro G1: could not reconstruct saved H.264 reference pictures");
	update_irq();
}

void hantro_g1_device::map(address_map &map)
{
	map(0, 0x3ff).rw(FUNC(hantro_g1_device::read), FUNC(hantro_g1_device::write));
}

void hantro_g1_device::update_irq()
{
	m_irq(((m_regs[1] & 0x110) == 0x100) || ((m_regs[60] & 0x110) == 0x100));
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
	LOGMASKED(LOG_REGS, "write %03x=%08x mask=%08x\n", offset * 4, data, mem_mask);
	if (offset == 1)
	{
		if (BIT(m_regs[1], 0) && !BIT(old, 0))
		{
			++m_jobs;
			LOGMASKED(LOG_JOBS, "decode %u mode=%u stream=%08x bit=%u length=%u output=%08x\n", m_jobs, m_regs[3] >> 28, m_regs[12], m_regs[5] >> 26,
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
	auto fail = [this](char const *reason)
	{
		logerror("decode %u failed: %s\n", m_jobs, reason);
		m_regs[1] = (m_regs[1] & ~1U) | 0x10100;
		if (BIT(m_regs[60], 1))
			m_regs[60] = (m_regs[60] & ~1U) | 0x2100;
		update_irq();
	};
	std::vector<u8> packet;
	if ((m_regs[3] >> 28) || !hantro_g1::parameter_sets(m_regs, packet))
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
		stream.push_back(m_dma->read_byte(m_regs[12] + (p ^ swap)));
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
			unsigned const type = stream[n - 2] & 31;
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
		// The register bank is authoritative. A DMA range can include SPS/PPS
		// NALs already parsed by the guest; forwarding them would replace the
		// reconstructed sets (including irrelevant VUI/display parameters) and
		// can spuriously reset the software backend's reference picture buffer.
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
			unsigned const type = stream[header] & 31;
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
	if (packet.size() + 4 > 16 * 1024 * 1024 - m_history_size)
		return fail("reference history exceeds supported GOP buffer");
	u8 *planes[3]{};
	unsigned width, height, strides[3];
	if (!decode_packet(packet.data(), packet.size(), planes, width, height, strides))
		return fail("invalid or unsupported H.264 picture");
	if (width != ((m_regs[4] >> 23) & 511) * 16 || height != ((m_regs[4] >> 11) & 255) * 16)
		return fail("decoded picture dimensions differ from programmed dimensions");
	for (unsigned b = 0; b < 4; ++b)
		m_history[m_history_size++] = packet.size() >> (b * 8);
	std::copy(packet.begin(), packet.end(), m_history.get() + m_history_size);
	m_history_size += packet.size();
	++m_history_units;
	if (!BIT(m_regs[3], 15))
	{
		u32 const out = m_regs[13];
		u32 const outswap = (BIT(m_regs[2], 8) ? 0 : 3) | (BIT(m_regs[2], 19) ? 0 : 4);
		auto write_pixels = [&](u32 offset, u32 pixels)
		{
			if (!(out & 3))
				m_dma->write_dword(out + (offset ^ (outswap & 4)), (outswap & 3) ? swapendian_int32(pixels) : pixels);
			else
				for (unsigned i = 0; i < 4; ++i)
					m_dma->write_byte(out + ((offset + i) ^ outswap), pixels >> (i * 8));
		};
		for (unsigned y = 0; y < height; ++y)
			for (unsigned x = 0; x < width; x += 4)
			{
				u8 const *p = planes[0] + y * strides[0] + x;
				write_pixels(y * width + x, u32(p[0]) | (u32(p[1]) << 8) | (u32(p[2]) << 16) | (u32(p[3]) << 24));
			}
		for (unsigned y = 0; y < height / 2; ++y)
			for (unsigned x = 0; x < width / 2; x += 2)
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
	LOGMASKED(LOG_JOBS, "decoded %u %ux%u consumed=%u remaining=%u\n", m_jobs, width, height, consumed - start, length - consumed);
	update_irq();
}

TIMER_CALLBACK_MEMBER(hantro_g1_device::pp_done)
{
	bool const ready = post_process();
	m_regs[60] = (m_regs[60] & ~1U) | (ready ? 0x1100 : 0x2100);
	update_irq();
}

bool hantro_g1_device::post_process(u8 const *const *planes, unsigned width, unsigned height, unsigned const *strides)
{
	unsigned const in_width = (m_regs[72] & 511) * 16;
	unsigned const in_height = ((m_regs[72] >> 9) & 255) * 16;
	unsigned const out_width = (m_regs[85] >> 4) & 2047;
	unsigned const out_height = (m_regs[85] >> 15) & 2047;
	unsigned const in_format = m_regs[85] >> 29, out_format = (m_regs[85] >> 26) & 7;
	unsigned const pitch = (m_regs[92] & 4095) ? m_regs[92] & 4095 : out_width;
	// This path supports progressive NV12 to RGB with no crop, rotation, masks or
	// contrast adjustment. Other configurations must report an error until
	// their data paths are implemented, rather than returning a blank picture.
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
	unsigned const bytes = BIT(m_regs[79], 28) ? 2 : 4;
	unsigned const a = (m_regs[69] >> 8) & 1023;
	unsigned const b = m_regs[70] & 1023, c = (m_regs[70] >> 10) & 1023;
	unsigned const d = (m_regs[70] >> 20) & 1023, e = m_regs[71] & 1023;
	int const brightness = s8(m_regs[71] >> 10);
	u32 const alpha = m_regs[82] & m_regs[83] & m_regs[84];
	unsigned const padding[3] = { (m_regs[79] >> 23) & 31, (m_regs[79] >> 18) & 31, (m_regs[80] >> 18) & 31 };
	u32 const inswap = (BIT(m_regs[61], 7) ? 0 : 3) | (BIT(m_regs[61], 10) ? 0 : 4);
	u32 const outswap = (BIT(m_regs[61], 6) ? 0 : 3) | (BIT(m_regs[61], 5) ? 0 : 4);
	unsigned const source_pitch = ((m_regs[88] >> 23) & 511) * 16;
	if (!planes && source_pitch < in_width)
		return false;
	auto channel = [](int value) -> u32
	{
		return std::clamp(value >> 8, 0, 255);
	};
	for (unsigned y = 0; y < out_height; ++y)
		for (unsigned x = 0; x < out_width; ++x)
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
				luma = m_dma->read_byte(m_regs[63] + ((y * source_pitch + x) ^ inswap));
				unsigned const chroma = (y / 2) * source_pitch + (x & ~1U);
				cb = m_dma->read_byte(m_regs[64] + (chroma ^ inswap)) - 128;
				cr = m_dma->read_byte(m_regs[64] + ((chroma + 1) ^ inswap)) - 128;
			}
			// Colour conversion coefficients are unsigned 8.8 values.
			int const yy = int(a) * (luma - (BIT(m_regs[79], 29) ? 0 : 16)) + brightness * 256;
			u32 const rgb[3] = { channel(yy + int(b) * cr), channel(yy - int(c) * cr - int(d) * cb), channel(yy + int(e) * cb) };
			u32 pixel = alpha;
			for (unsigned k = 0; k < 3; ++k)
				pixel |= ((rgb[k] << 24) >> padding[k]) & (m_regs[82 + k] & ~alpha);
			if (bytes == 2)
				pixel >>= 16;
			u32 const offset = (y * pitch + x) * bytes;
			u32 const address = m_regs[66] + (offset ^ (outswap & ~(bytes - 1)));
			if (bytes == 4 && !(address & 3))
				m_dma->write_dword(address, (outswap & 3) ? swapendian_int32(pixel) : pixel);
			else if (bytes == 2 && !(address & 1))
				m_dma->write_word(address, (outswap & 1) ? swapendian_int16(u16(pixel)) : u16(pixel));
			else
				for (unsigned k = 0; k < bytes; ++k)
					m_dma->write_byte(m_regs[66] + ((offset + k) ^ outswap), pixel >> (k * 8));
		}
	LOGMASKED(LOG_JOBS, "PP %ux%u RGB%u output=%08x\n", out_width, out_height, bytes * 8, m_regs[66]);
	return true;
}
