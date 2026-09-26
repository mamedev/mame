// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
// Mali-200 tile-list reader, triangle rasterizer and writeback.
// Direct PP rectangle/base packets are emitted by _mali_projob_add_pp_drawcall
// in the original IGS38 userspace library; the master-list format is also used
// by the original limadriver/lima implementation.

namespace
{
bool mali_compare(unsigned function, float a, float b)
{
	switch (function & 7)
	{
	case 0:
		return false;
	case 1:
		return a < b;
	case 2:
		return a == b;
	case 3:
		return a <= b;
	case 4:
		return a > b;
	case 5:
		return a != b;
	case 6:
		return a >= b;
	default:
		return true;
	}
}

float mali_blend_factor(unsigned function, unsigned component, float const *src, float const *dst, float const *constant)
{
	float factor;
	unsigned const c = BIT(function, 4) ? 3 : component;
	switch (function & 7)
	{
	case 0:
		factor = src[c];
		break;
	case 1:
		factor = dst[c];
		break;
	case 2:
		factor = constant[c];
		break;
	case 3:
		factor = 0;
		break;
	case 4:
		factor = component == 3 ? 1 : std::min(src[3], 1 - dst[3]);
		break;
	default:
		factor = 0;
		break;
	}
	return BIT(function, 3) ? 1 - factor : factor;
}

float mali_blend(unsigned function, float src, float dst, float destination)
{
	switch (function)
	{
	case 0:
		return src - dst;
	case 1:
		return dst - src;
	case 2:
		return src + dst;
	// MIN/MAX compare the weighted sum with the original destination.
	// Lima consequently selects ONE/ZERO factors for GL_MIN and GL_MAX.
	case 4:
		return std::min(src + dst, destination);
	case 5:
		return std::max(src + dst, destination);
	default:
		return src;
	}
}

u8 mali_stencil(unsigned operation, u8 value, u8 reference)
{
	switch (operation & 7)
	{
	case 0:
		return value;
	case 1:
		return reference;
	case 2:
		return 0;
	case 3:
		return ~value;
	case 4:
		return value + 1;
	case 5:
		return value - 1;
	case 6:
		return value == 255 ? 255 : value + 1;
	default:
		return value == 0 ? 0 : value - 1;
	}
}

u8 mali_logic(unsigned operation, u8 src, u8 dst)
{
	// Each operation is the four-bit truth table indexed by (src, dst).
	return (BIT(operation, 0) ? u8(~src & ~dst) : 0) | (BIT(operation, 1) ? u8(~src & dst) : 0) | (BIT(operation, 2) ? u8(src & ~dst) : 0) |
			(BIT(operation, 3) ? u8(src & dst) : 0);
}
}

bool mali200_device::pp_writeback(pp_tile const &tile)
{
	for (unsigned unit = 0; unit < 3; ++unit)
	{
		u32 const *wb = &m_pp[(0x100 + unit * 0x100) / 4];
		if (!(wb[0] & 3))
			continue;
		unsigned const format = wb[2] & 15, flags = wb[6] & 63;
		if (wb[3] || wb[7])
		{
			logerror("unsupported PP writeback supersampling/MRT\n");
			return false;
		}
		unsigned const size = format == 4 || format == 13 ? 1 : format == 3 || format == 8 || format == 15 ? 4 : format == 6 ? 8 : 2;
		for (unsigned y = 0; y < 16; ++y)
			for (unsigned x = 0; x < 16; ++x)
			{
				unsigned const pixel = y * 16 + x, px = tile.x * 16 + x, py = tile.y * 16 + y;
				if (BIT(flags, 0) && !tile.dirty[pixel])
					continue;
				if (BIT(flags, 1) && (px < ((m_pp[10] >> 16) & 15) || px > (m_pp[10] & 0x3fff) || py >= (m_pp[11] & 0x3fff)))
					continue;
				float rgba[4];
				std::copy(std::begin(tile.color[pixel]), std::end(tile.color[pixel]), rgba);
				if (wb[9] & 1)
				{
					float const value = (wb[0] & 3) == 1 ? tile.stencil[pixel] : rgba[3];
					float const reference = (wb[0] & 3) == 1 ? (wb[10] & 255)
							: BIT(m_pp[3], 0) ? mali_half(wb[10])
							: (wb[10] & 255) / 255.0f;
					if (!mali_compare(wb[11], value, reference))
						continue;
				}
				if (BIT(flags, 2))
					std::swap(rgba[0], rgba[2]);
				unsigned r = std::clamp(int(rgba[0] * 255 + 0.5f), 0, 255), g = std::clamp(int(rgba[1] * 255 + 0.5f), 0, 255);
				unsigned b = std::clamp(int(rgba[2] * 255 + 0.5f), 0, 255), a = std::clamp(int(rgba[3] * 255 + 0.5f), 0, 255);
				u64 value;
				switch (format)
				{
				case 0:
					value = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
					break;
				case 1:
					value = ((a >> 7) << 15) | ((r >> 3) << 10) | ((g >> 3) << 5) | (b >> 3);
					break;
				case 2:
					value = ((a >> 4) << 12) | ((r >> 4) << 8) | ((g >> 4) << 4) | (b >> 4);
					break;
				case 3:
					value = (a << 24) | (r << 16) | (g << 8) | b;
					break;
				case 4:
					value = BIT(flags, 3) ? a : b;
					break;
				case 5:
					value = BIT(flags, 3) ? (r << 8) | a : (g << 8) | b;
					break;
				case 6:
					value = u64(mali_to_half(rgba[3])) << 48 | u64(mali_to_half(rgba[0])) << 32 | u64(mali_to_half(rgba[1])) << 16 |
							mali_to_half(rgba[2]);
					break;
				case 7:
					value = mali_to_half(rgba[BIT(flags, 3) ? 3 : 2]);
					break;
				case 8:
					value = BIT(flags, 3) ? (u32(mali_to_half(rgba[0])) << 16) | mali_to_half(rgba[3])
							: (u32(mali_to_half(rgba[1])) << 16) | mali_to_half(rgba[2]);
					break;
				case 13:
					value = tile.stencil[pixel];
					break;
				case 14:
					value = std::clamp(int(tile.depth[pixel] * 65535 + 0.5f), 0, 65535);
					break;
				case 15:
					value = (u32(tile.stencil[pixel]) << 24) | std::clamp(int(tile.depth[pixel] * 16777215 + 0.5f), 0, 16777215);
					break;
				default:
					logerror("unsupported PP writeback format %u\n", format);
					return false;
				}
				if (BIT(flags, 3))
				{
					if (format == 1)
						value = ((value & 31) << 11) | (((value >> 5) & 31) << 6) | (((value >> 10) & 31) << 1) | ((value >> 15) & 1);
					if (format == 2)
						value = ((value & 15) << 12) | (((value >> 4) & 15) << 8) | (((value >> 8) & 15) << 4) | ((value >> 12) & 15);
					if (format == 3)
						value = swapendian_int32(u32(value));
					if (format == 6)
						value = ((value & 0xffff) << 48) | (((value >> 16) & 0xffff) << 32) | (((value >> 32) & 0xffff) << 16) |
								(value >> 48);
				}
				u32 address = wb[1];
				switch (wb[4] & 3)
				{
				case 0:
					address += py * (wb[5] & 0xffff) * 8 + px * size;
					break;
				case 1:
					address += mali_uorder(px, py, 14) * size;
					break;
				case 2:
					address += ((tile.y * (wb[5] & 0xffff) + tile.x) * 256 + mali_uorder(x, y, 4)) * size;
					break;
				default:
					return false;
				}
				if (BIT(flags, 5))
				{
					u64 reversed = 0;
					for (unsigned i = 0; i < size; ++i)
						reversed |= ((value >> (i * 8)) & 255) << ((size - i - 1) * 8);
					value = reversed;
				}
				for (unsigned i = 0; i < size; i += 4)
					if (!write_bytes(address + i, std::min(4U, size - i), value >> (i * 8), 4))
						return false;
			}
	}
	return true;
}

bool mali200_device::pp_primitive(pp_tile &tile, u32 state_address, u32 vertex_address, u32 const *indices, bool rectangle)
{
	u32 state[16];
	float vertex[3][4], varying[3][64];
	for (unsigned i = 0; i < 16; ++i)
		if (!read_word(state_address + i * 4, state[i], 0))
			return false;
	for (unsigned v = 0; v < 3; ++v)
		for (unsigned c = 0; c < 4; ++c)
		{
			u32 word;
			if (!read_word(vertex_address + indices[v] * 16 + c * 4, word, 0))
				return false;
			vertex[v][c] = mali_float(word);
			if (!std::isfinite(vertex[v][c]))
				return true;
		}
	auto edge = [](float const *a, float const *b, float x, float y)
	{
		return (b[0] - a[0]) * (y - a[1]) - (b[1] - a[1]) * (x - a[0]);
	};
	float const area = edge(vertex[0], vertex[1], vertex[2][0], vertex[2][1]);
	if (!std::isfinite(area) || area == 0)
		return true;
	float const xmin = std::min({ vertex[0][0], vertex[1][0], vertex[2][0] });
	float const xmax = std::max({ vertex[0][0], vertex[1][0], vertex[2][0] });
	float const ymin = std::min({ vertex[0][1], vertex[1][1], vertex[2][1] });
	float const ymax = std::max({ vertex[0][1], vertex[1][1], vertex[2][1] });
	unsigned const stride = (state[13] & 31) * 8;
	unsigned varying_components = 0;
	for (unsigned v = 0; v < 3 && stride; ++v)
	{
		unsigned offset = 0, scalar = 0;
		for (unsigned slot = 0; slot < 12 && offset < stride; ++slot)
		{
			unsigned const type = slot < 10 ? (state[10] >> (slot * 3)) & 7
					: slot == 10 ? ((state[10] >> 30) | ((state[15] & 1) << 2))
					: ((state[15] >> 1) & 7);
			unsigned const components = type == 1 || type == 3 || type == 4 || type == 5 ? 2 : 4;
			unsigned const size = type < 2 ? 4 : type < 6 ? 2 : 1;
			for (unsigned c = 0; c < components; ++c)
			{
				u32 value;
				if (!read_bytes((state[15] & ~15U) + indices[v] * stride + offset + c * size, size, value, 2))
					return false;
				varying[v][scalar + c] = type < 2 ? mali_float(value)
						: type < 4 ? mali_half(value)
						: type == 4 ? std::max(-1.0f, s16(value) / 32767.0f)
						: type == 5 ? value / 65535.0f
						: value / 255.0f;
			}
			offset += components * size;
			scalar += components;
		}
		varying_components = scalar;
	}
	// Intersect the primitive's sample bounds and scissor with this tile.
	// Compare the same sample coordinates as the coverage test, avoiding
	// float-to-integer rounding at subpixel edges or extreme vertex values.
	// Keep vertex/varying reads above this rejection so DMA faults retain
	// their normal behavior even for a primitive outside the tile.
	unsigned x_first = 16, x_last = 0, y_first = 16, y_last = 0;
	float const sample_x = s16(m_pp[0x40 / 4]) * 0.5f;
	for (unsigned i = 0; i < 16; ++i)
	{
		unsigned const px = tile.x * 16 + i, py = tile.y * 16 + i;
		float const sx = px + sample_x, sy = py + 0.5f;
		if (px >= tile.scissor[0] && px <= tile.scissor[1] && sx >= xmin && sx < xmax)
		{
			x_first = std::min(x_first, i);
			x_last = i + 1;
		}
		if (py >= tile.scissor[2] && py <= tile.scissor[3] && sy >= ymin && sy < ymax)
		{
			y_first = std::min(y_first, i);
			y_last = i + 1;
		}
	}
	if (x_first == 16 || y_first == 16)
		return true;
	bool shader_constant = false;
	float constant_color[4]{}, constant_depth = 0;
	// These controls cannot change while this primitive is rasterized.
	bool const normalized = !BIT(m_pp[3], 0);
	bool const front = (area > 0) == BIT(state[14], 12);
	u32 const stencil = state[front ? 5 : 6];
	u8 const stencil_reference = stencil >> 16, stencil_mask = stencil >> 24;
	u8 const stencil_write_mask = state[7] >> (front ? 0 : 8);
	unsigned const alpha_function = state[8] & 7, depth_function = (state[3] >> 1) & 7;
	float const alpha_reference = ((state[7] >> 16) & 255) / 255.0f;
	unsigned const color_mask = state[2] >> 28;
	unsigned const source_factor[2] = { (state[2] >> 6) & 31, (state[2] >> 16) & 15 };
	unsigned const destination_factor[2] = { (state[2] >> 11) & 31, (state[2] >> 20) & 15 };
	unsigned const blend_operation[2] = { state[2] & 7, (state[2] >> 3) & 7 };
	float const blend_constant[4] = { float(state[1] & 255) / 255, float((state[0] >> 16) & 255) / 255, float(state[0] & 255) / 255,
		float((state[1] >> 16) & 255) / 255 };
	// Alpha factors, ZERO/ONE and alpha-saturate are identical for R/G/B.
	// Color-dependent factors still evaluate each component independently.
	bool const shared_source = BIT(source_factor[0], 4) || (source_factor[0] & 7) >= 3;
	bool const shared_destination = BIT(destination_factor[0], 4) || (destination_factor[0] & 7) >= 3;
	for (unsigned y = y_first; y < y_last; ++y)
		for (unsigned x = x_first; x < x_last; ++x)
		{
			float const sx = tile.x * 16 + x + sample_x;
			float const sy = tile.y * 16 + y + 0.5f;
			float const e[3] = { edge(vertex[1], vertex[2], sx, sy), edge(vertex[2], vertex[0], sx, sy),
				edge(vertex[0], vertex[1], sx, sy) };
			// Pixel rectangles interpolate the plane defined by three corners,
			// including the half outside the input triangle. This is also used
			// for the driver's 1x1 PP readback jobs, not just full-screen clears.
			bool inside = true;
			for (unsigned i = 0; i < 3 && !rectangle; ++i)
			{
				float const *a = vertex[(i + 1) % 3], *b = vertex[(i + 2) % 3];
				float const dx = b[0] - a[0], dy = b[1] - a[1];
				bool const top_left = area > 0 ? (dy < 0 || (dy == 0 && dx > 0)) : (dy > 0 || (dy == 0 && dx < 0));
				if (e[i] / area < 0 || (e[i] == 0 && !top_left))
					inside = false;
			}
			if (!inside)
				continue;
			float const weight[3] = { e[0] / area, e[1] / area, e[2] / area };
			unsigned const pixel = y * 16 + x;
			pp_fragment fragment;
			fragment.varying_count = varying_components;
			fragment.front = front;
			fragment.depth = weight[0] * vertex[0][2] + weight[1] * vertex[1][2] + weight[2] * vertex[2][2];
			float const inverse_w = weight[0] * vertex[0][3] + weight[1] * vertex[1][3] + weight[2] * vertex[2][3];
			fragment.coordinate[0] = sx;
			fragment.coordinate[1] = BIT(m_pp[3], 5) ? s16(m_pp[17]) * 0.5f - (tile.y * 16 + y) : sy;
			fragment.coordinate[2] = fragment.depth;
			fragment.coordinate[3] = inverse_w;
			std::copy(std::begin(tile.color[pixel]), std::end(tile.color[pixel]), fragment.color);
			// Shader fetches return zero for unconfigured varying slots.
			// A textured rectangle commonly supplies just two scalars, so avoid
			// perspective interpolation of the remaining unused slots per pixel.
			for (unsigned c = 0; c < varying_components; ++c)
				fragment.varying[c] = inverse_w
						? (weight[0] * varying[0][c] * vertex[0][3] + weight[1] * varying[1][c] * vertex[1][3] +
								weight[2] * varying[2][c] * vertex[2][3]) / inverse_w
						: 0;
			if (shader_constant)
			{
				std::copy(std::begin(constant_color), std::end(constant_color), fragment.color);
				if (BIT(state[3], 11))
					fragment.depth = constant_depth;
			}
			else
			{
				if (!pp_shader(state, fragment, shader_constant))
					return false;
				if (shader_constant)
				{
					std::copy(std::begin(fragment.color), std::end(fragment.color), constant_color);
					constant_depth = fragment.depth;
				}
			}
			// Normalized tile buffers clamp the shader output before fragment
			// tests and blending. In particular, palette shaders can produce an
			// alpha greater than one to make every nonzero index opaque.
			if (normalized)
				for (float &component : fragment.color)
					component = std::clamp(component, 0.0f, 1.0f);
			if (fragment.discard || !mali_compare(alpha_function, fragment.color[3], alpha_reference))
				continue;
			bool const stencil_pass = mali_compare(stencil & 7, stencil_reference & stencil_mask, tile.stencil[pixel] & stencil_mask);
			bool const depth_pass = mali_compare(depth_function, fragment.depth, tile.depth[pixel]);
			unsigned const stencil_operation = (stencil >> (!stencil_pass ? 3 : !depth_pass ? 6 : 9)) & 7;
			u8 const old_stencil = tile.stencil[pixel];
			tile.stencil[pixel] = (tile.stencil[pixel] & ~stencil_write_mask) |
					(mali_stencil(stencil_operation, tile.stencil[pixel], stencil_reference) & stencil_write_mask);
			if (tile.stencil[pixel] != old_stencil)
				tile.dirty[pixel] = true;
			if (!stencil_pass || !depth_pass)
				continue;
			if (BIT(state[3], 0))
				tile.depth[pixel] = fragment.depth;
			float const rgb_source_factor = blend_operation[0] != 3 && shared_source
					? mali_blend_factor(source_factor[0], 0, fragment.color, tile.color[pixel], blend_constant)
					: 0;
			float const rgb_destination_factor = blend_operation[0] != 3 && shared_destination
					? mali_blend_factor(destination_factor[0], 0, fragment.color, tile.color[pixel], blend_constant)
					: 0;
			float result[4];
			for (unsigned c = 0; c < 4; ++c)
			{
				unsigned const src = source_factor[c == 3], dst = destination_factor[c == 3];
				unsigned const operation = blend_operation[c == 3];
				if (operation == 3)
				{
					u8 const s = std::clamp(fragment.color[c], 0.0f, 1.0f) * 255 + 0.5f;
					u8 const d = std::clamp(tile.color[pixel][c], 0.0f, 1.0f) * 255 + 0.5f;
					result[c] = mali_logic(src & 15, s, d) / 255.0f;
				}
				else
				{
					float const sf = c < 3 && shared_source ? rgb_source_factor
							: mali_blend_factor(src, c, fragment.color, tile.color[pixel], blend_constant);
					float const df = c < 3 && shared_destination
							? rgb_destination_factor
							: mali_blend_factor(dst, c, fragment.color, tile.color[pixel], blend_constant);
					result[c] = mali_blend(operation, fragment.color[c] * sf, tile.color[pixel][c] * df, tile.color[pixel][c]);
				}
			}
			for (unsigned c = 0; c < 4; ++c)
				if (BIT(color_mask, c))
					tile.color[pixel][c] = normalized ? std::clamp(result[c], 0.0f, 1.0f) : result[c];
			tile.dirty[pixel] = true;
		}
	return true;
}

bool mali200_device::pp_render()
{
	pp_tile tile;
	bool have_tile = false;
	u32 address = m_pp[0], state_base = m_pp[1], vertex_base = m_pp[2];
	std::vector<u32> stack;
	// This is a host-side guard against malformed cyclic lists, not a
	// hardware job limit. A 1280x720 scene can legitimately revisit hundreds
	// of render-state packets for each of its 3,600 tiles (over 1M packets).
	for (unsigned commands = 0; commands < 16U * 1048576; ++commands)
	{
		u32 lo, hi;
		if (!read_word(address, lo, 0) || !read_word(address + 4, hi, 0))
			return false;
		m_pp[0x1004 / 4] = address;
		if (m_pp_jobs <= 3 && commands < 64)
			LOGMASKED(LOG_LISTS, "PP command %08x: %08x %08x\n", address, lo, hi);
		address += 8;
		if ((hi & 0xfc000000) == 0xb8000000)
		{
			if (have_tile && !pp_writeback(tile))
				return false;
			tile.x = hi & 255;
			tile.y = (hi >> 8) & 255;
			have_tile = true;
			for (unsigned pixel = 0; pixel < 256; ++pixel)
			{
				for (unsigned c = 0; c < 4; ++c)
					tile.color[pixel][c] =
							BIT(m_pp[3], 0) ? mali_half(m_pp[6 + c / 2] >> ((c % 2) * 16)) : ((m_pp[6] >> (c * 8)) & 255) / 255.0f;
				tile.depth[pixel] = (m_pp[4] & 0xffffff) / 16777215.0f;
				tile.stencil[pixel] = m_pp[5];
				tile.dirty[pixel] = false;
			}
		}
		else if (hi == 0xbc000000)
			return !have_tile || pp_writeback(tile);
		else if ((hi & 0xfc000000) == 0xb0000000)
		{
			if (!lo)
			{
				if (stack.empty())
					return !have_tile || pp_writeback(tile);
				address = stack.back();
				stack.pop_back();
			}
			else
			{
				if (stack.size() >= 32)
					return false;
				if (lo & 2)
					stack.push_back(address);
				address = (lo & 0x1ffffffc) << 3;
			}
		}
		else if ((hi & 0xc0000000) == 0x80000000 && (hi & 0x3c000000) == 0)
		{
			state_base = lo << 3;
			vertex_base = (hi & 0x03ffffff) << 6;
		}
		else if ((hi & 0xf0000000) == 0x70000000)
		{
			tile.scissor[0] = (lo >> 30) | ((hi & 0x1fff) << 2);
			tile.scissor[1] = (hi >> 13) & 0x7fff;
			tile.scissor[2] = lo & 0x7fff;
			tile.scissor[3] = (lo >> 15) & 0x7fff;
		}
		else if ((hi & 0xf8000000) == 0 || (hi & 0xf8000000) == 0x20000000)
		{
			if (!have_tile)
				return false;
			u64 const packet = u64(hi) << 32 | lo;
			u32 const indices[3] = { u32(packet & 0x1ffff), u32((packet >> 17) & 0x1ffff), u32((packet >> 34) & 0x1ffff) };
			if (!pp_primitive(tile, state_base + ((hi >> 19) & 255) * 64, vertex_base, indices, BIT(hi, 29)))
				return false;
		}
		else
		{
			logerror("unsupported PP polygon packet %08x %08x\n", lo, hi);
			return false;
		}
	}
	logerror("PP polygon command limit\n");
	return false;
}
