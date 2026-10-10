// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
// Mali texture descriptors and texel fetch. Common descriptor layout documented
// by the Lima project; IGS38 uses the original Mali-200 pixel processor.

namespace
{
unsigned mali_uorder(unsigned x, unsigned y, unsigned bits)
{
	unsigned offset = 0;
	for (unsigned bit = 0; bit < bits; ++bit)
	{
		offset |= ((x ^ y) & (1U << bit)) << bit;
		offset |= (y & (1U << bit)) << (bit + 1);
	}
	return offset;
}

void mali_etc1(u32 low_address, u32 high_address, unsigned x, unsigned y, float *color)
{
	// ETC1 blocks are eight bytes in big-endian bit order, independently of
	// the CPU/GPU bus byte order.  Khronos OES_compressed_ETC1_RGB8_texture.
	u32 const header = swapendian_int32(low_address);
	u32 const indices = swapendian_int32(high_address);
	unsigned const subblock = BIT(header, 0) ? (y >> 1) : (x >> 1);
	unsigned const table = (header >> (subblock ? 2 : 5)) & 7;
	unsigned const bit = x * 4 + y;
	static constexpr int MODIFIERS[8][2] = { { 2, 8 }, { 5, 17 }, { 9, 29 }, { 13, 42 }, { 18, 60 }, { 24, 80 }, { 33, 106 }, { 47, 183 } };
	int const modifier = MODIFIERS[table][BIT(indices, bit)] * (BIT(indices, bit + 16) ? -1 : 1);
	for (unsigned c = 0; c < 3; ++c)
	{
		unsigned const shift = 24 - c * 8;
		int base;
		if (BIT(header, 1))
		{
			base = (header >> (shift + 3)) & 31;
			if (subblock)
				base += util::sext((header >> shift) & 7, 3);
			// Overflowing differential blocks have undefined ETC1 results.
			base = std::clamp(base, 0, 31);
			base = (base << 3) | (base >> 2);
		}
		else
			base = ((header >> (shift + (subblock ? 0 : 4))) & 15) * 17;
		color[c] = std::clamp(base + modifier, 0, 255) / 255.0f;
	}
	color[3] = 1;
}
}

bool mali200_device::pp_texture(u32 const *state, unsigned sampler, float const *coordinate, float lod, float *color)
{
	if (sampler >= 16)
		return false;
	u32 const pointer_address = (state[12] & ~15U) + sampler * 4;
	pp_texture_state *&cached = m_pp_texture_cache[(pointer_address >> 2) & 63];
	bool inserted = false;
	if (!cached || cached->address != pointer_address)
	{
		auto const entry = m_pp_textures.try_emplace(pointer_address);
		cached = &entry.first->second;
		inserted = entry.second;
	}
	pp_texture_state &texture = *cached;
	if (inserted)
	{
		texture.address = pointer_address;
		u32 descriptor_address, descriptor[16];
		if (!read_word(pointer_address, descriptor_address, 3))
			return false;
		for (unsigned i = 0; i < 16; ++i)
			if (!read_word((descriptor_address & ~63U) + i * 4, descriptor[i], 3))
				return false;
		auto f = [&](unsigned start, unsigned width)
		{
			return mali_field(descriptor, start, width);
		};
		texture.format = f(0, 6);
		texture.layout = f(205, 2);
		texture.max_level = std::min(10U, f(52, 8) / 16);
		texture.width = f(86, 13);
		texture.height = f(99, 13);
		for (unsigned level = 0; level < 11; ++level)
			texture.base[level] = f(222 + level * 26, 26) << 6;
		unsigned const format = texture.format;
		texture.compressed = format == 0x20;
		texture.size = texture.compressed ? 8
				: format >= 0x09 && format <= 0x0b ? 1
				: (format >= 0x0e && format <= 0x14) || (format >= 0x22 && format <= 0x24) ? 2
				: format == 0x15 ? 3
				: format == 0x26 ? 8
				: format == 0x2f ? 6
				: 4;
		texture.explicit_stride = f(72, 1);
		texture.stride = f(16, 15);
		texture.unnormalized = f(39, 1);
		texture.wrap_s = f(77, 3);
		texture.wrap_t = f(80, 3);
		texture.reverse = f(6, 1);
		texture.swap_rb = f(7, 1);
		texture.range = f(8, 2);
		texture.nearest_min = f(75, 1);
		texture.nearest_mag = f(76, 1);
		for (unsigned c = 0; c < 4; ++c)
			texture.border[c] = f(125 + 16 * c, 16) / 65535.0f;
	}
	unsigned const format = texture.format, layout = texture.layout;
	unsigned const level = std::clamp(int(std::floor(lod + 0.5f)), 0, int(texture.max_level));
	unsigned const width = std::max(1U, texture.width >> level), height = std::max(1U, texture.height >> level);
	if (width > 4096 || height > 4096)
		return false;
	u32 const base = texture.base[level];
	bool const compressed = texture.compressed;
	unsigned const size = texture.size;
	// The implicit linear pitch is the packed texel row. Bus beat alignment
	// does not add padding between rows; padded surfaces specify a stride.
	unsigned const stride = texture.explicit_stride ? texture.stride : (compressed ? (width + 3) / 4 : width) * size;
	float u = coordinate[0], v = coordinate[1];
	if (!std::isfinite(u) || !std::isfinite(v))
	{
		std::fill(color, color + 4, 0);
		return true;
	}
	if (!texture.unnormalized)
	{
		u *= width;
		v *= height;
	}
	u = std::clamp(u, -1073741824.0f, 1073741824.0f);
	v = std::clamp(v, -1073741824.0f, 1073741824.0f);
	auto wrap = [](int position, unsigned size, unsigned mode, bool &border) -> unsigned
	{
		// Every supported wrap mode leaves an in-range coordinate unchanged.
		// Most samples take this path, avoiding repeat/mirror divisions.
		if (unsigned(position) < size)
			return unsigned(position);
		if (mode == 4)
		{
			int const period = int(size) * 2;
			int const wrapped = (position % period + period) % period;
			return wrapped < int(size) ? wrapped : period - 1 - wrapped;
		}
		if (mode >= 5)
		{
			position = position < 0 ? -position - 1 : position;
			mode -= 4;
		}
		if (mode == 0)
			return (position % int(size) + size) % size;
		if (mode >= 2 && (position < 0 || position >= int(size)))
			border = true;
		return std::clamp(position, 0, int(size) - 1);
	};
	auto texel = [&](int sx, int sy, float *result) -> bool
	{
		bool border = false;
		unsigned const x = wrap(sx, width, texture.wrap_s, border), y = wrap(sy, height, texture.wrap_t, border);
		if (border)
		{
			for (unsigned c = 0; c < 4; ++c)
				result[c] = texture.border[c];
			return true;
		}
		u32 address;
		if (layout == 0)
			address = base + (compressed ? y >> 2 : y) * stride + (compressed ? x >> 2 : x) * size;
		else if (layout == 3)
		{
			// A 16x16 tile contains sixteen 4x4 ETC1 blocks, in U order.
			unsigned const index = compressed ? mali_uorder((x >> 2) & 3, (y >> 2) & 3, 2) : mali_uorder(x & 15, y & 15, 4);
			address = base + (((y >> 4) * ((width + 15) >> 4) + (x >> 4)) * (compressed ? 16 : 256) + index) * size;
		}
		else
		{
			logerror("unsupported Mali texture layout %u\n", layout);
			return false;
		}
		u32 data = 0, upper = 0;
		if (!read_bytes(address, std::min(size, 4U), data, 3))
			return false;
		if (size > 4 && !read_bytes(address + 4, size - 4, upper, 3))
			return false;
		result[0] = result[1] = result[2] = 0;
		result[3] = 1;
		switch (format)
		{
		case 0x09:
			result[0] = result[1] = result[2] = data / 255.0f;
			break;
		case 0x0a:
			result[3] = data / 255.0f;
			break;
		case 0x0b:
			std::fill(result, result + 4, data / 255.0f);
			break;
		case 0x0e:
			result[0] = (data & 31) / 31.0f;
			result[1] = ((data >> 5) & 63) / 63.0f;
			result[2] = ((data >> 11) & 31) / 31.0f;
			break;
		case 0x0f:
			result[0] = ((data >> 1) & 31) / 31.0f;
			result[1] = ((data >> 6) & 31) / 31.0f;
			result[2] = ((data >> 11) & 31) / 31.0f;
			result[3] = data & 1;
			break;
		case 0x10:
			result[0] = ((data >> 4) & 15) / 15.0f;
			result[1] = ((data >> 8) & 15) / 15.0f;
			result[2] = ((data >> 12) & 15) / 15.0f;
			result[3] = (data & 15) / 15.0f;
			break;
		case 0x11:
			result[0] = result[1] = result[2] = (data & 255) / 255.0f;
			result[3] = (data >> 8) / 255.0f;
			break;
		case 0x12:
			result[0] = result[1] = result[2] = data / 65535.0f;
			break;
		case 0x13:
			result[3] = data / 65535.0f;
			break;
		case 0x14:
			std::fill(result, result + 4, data / 65535.0f);
			break;
		case 0x15:
		case 0x16:
		case 0x17:
			for (unsigned c = 0; c < (format == 0x16 ? 4U : 3U); ++c)
				result[c] = ((data >> (8 * c)) & 255) / 255.0f;
			break;
		case 0x20:
			mali_etc1(data, upper, x & 3, y & 3, result);
			break;
		case 0x22:
			result[0] = result[1] = result[2] = mali_half(data);
			break;
		case 0x23:
			result[3] = mali_half(data);
			break;
		case 0x24:
			std::fill(result, result + 4, mali_half(data));
			break;
		case 0x25:
			result[0] = result[1] = result[2] = mali_half(data);
			result[3] = mali_half(data >> 16);
			break;
		case 0x26:
		case 0x2f:
			result[0] = mali_half(data);
			result[1] = mali_half(data >> 16);
			result[2] = mali_half(upper);
			if (format == 0x26)
				result[3] = mali_half(upper >> 16);
			break;
		case 0x2c:
			result[0] = result[1] = result[2] = (data >> 8) / 16777215.0f;
			break;
		default:
			logerror("unsupported Mali texel format %02x\n", format);
			return false;
		}
		if (texture.reverse)
			std::reverse(result, result + 4);
		if (texture.swap_rb)
			std::swap(result[0], result[2]);
		if (texture.range == 3)
			for (unsigned c = 0; c < 4; ++c)
				result[c] = 1 - result[c];
		else if (texture.range != 0)
		{
			logerror("unsupported Mali signed texture range\n");
			return false;
		}
		return true;
	};
	bool const nearest = lod > 0 ? texture.nearest_min : texture.nearest_mag;
	if (nearest)
		return texel(int(std::floor(u)), int(std::floor(v)), color);
	float const x = u - 0.5f, y = v - 0.5f;
	int const ix = std::floor(x), iy = std::floor(y);
	float const fx = x - ix, fy = y - iy;
	float samples[4][4];
	if (!texel(ix, iy, samples[0]) || !texel(ix + 1, iy, samples[1]) || !texel(ix, iy + 1, samples[2]) ||
			!texel(ix + 1, iy + 1, samples[3]))
		return false;
	for (unsigned c = 0; c < 4; ++c)
		color[c] = (samples[0][c] * (1 - fx) + samples[1][c] * fx) * (1 - fy) + (samples[2][c] * (1 - fx) + samples[3][c] * fx) * fy;
	return true;
}
