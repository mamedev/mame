// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
// MaliGP2 command processor and functional vertex shader interpreter.
// ISA reference: Connor Abbott, https://github.com/cwabbott0/mali-isa-docs
// Command encodings cross-checked with limadriver/lima (Mali-200 support).

namespace
{
float mali_float(u32 bits)
{
	float value;
	std::memcpy(&value, &bits, sizeof(value));
	return value;
}

u32 mali_bits(float value)
{
	u32 bits;
	std::memcpy(&bits, &value, sizeof(bits));
	return bits;
}

u32 mali_field(u32 const *words, unsigned start, unsigned size)
{
	u64 value = words[start / 32] >> (start & 31);
	if ((start & 31) + size > 32)
		value |= u64(words[start / 32 + 1]) << (32 - (start & 31));
	return u32(value) & ((size == 32) ? ~0U : (1U << size) - 1);
}

float mali_half(u16 bits)
{
	unsigned const sign = bits >> 15, exponent = (bits >> 10) & 31, mantissa = bits & 1023;
	if (exponent == 31)
		return mali_float((sign << 31) | 0x7f800000 | (mantissa << 13));
	// Every normal binary16 value is exactly representable in binary32.
	// Adjust its exponent bias directly; texture/uniform fetches do this
	// conversion for every fragment, so avoid a library call in that path.
	if (exponent)
		return mali_float((sign << 31) | ((exponent + 112) << 23) | (mantissa << 13));
	float value = std::ldexp(float(mantissa), -24);
	return sign ? -value : value;
}

u16 mali_to_half(float value)
{
	u32 const bits = mali_bits(value), sign = (bits >> 16) & 0x8000;
	int exponent = int((bits >> 23) & 255) - 127;
	u32 mantissa = bits & 0x7fffff;
	if (exponent == 128)
		return sign | 0x7c00 | (mantissa ? 0x200 : 0);
	if (exponent > 15)
		return sign | 0x7c00;
	if (exponent < -25)
		return sign;
	if (exponent < -14)
	{
		mantissa |= 0x800000;
		unsigned const shift = -exponent - 1;
		u32 const round = (1U << (shift - 1)) - 1 + ((mantissa >> shift) & 1);
		return sign | ((mantissa + round) >> shift);
	}
	mantissa += 0xfff + ((mantissa >> 13) & 1);
	if (mantissa & 0x800000)
	{
		mantissa = 0;
		++exponent;
	}
	return sign | (u32(exponent + 15) << 10) | (mantissa >> 13);
}
}

bool mali200_device::read_bytes(u32 address, unsigned size, u32 &data, unsigned bus)
{
	// Aligned accesses cannot cross a GPU page. Preserve byte accesses for
	// packed/unaligned formats, but avoid walking the MMU four times per texel.
	if (size == 4 && !(address & 3))
		return read_word(address, data, bus);
	if (size == 2 && !(address & 1))
	{
		u32 physical;
		if (!translate(address, false, physical, bus))
			return false;
		data = m_dma_access.read_word(physical);
		return true;
	}
	data = 0;
	for (unsigned i = 0; i < size; ++i)
	{
		u32 physical;
		if (!translate(address + i, false, physical, bus))
			return false;
		data |= u32(m_dma_access.read_byte(physical)) << (8 * i);
	}
	return true;
}

bool mali200_device::write_bytes(u32 address, unsigned size, u32 data, unsigned bus)
{
	if (size == 4 && !(address & 3))
		return write_word(address, data, bus);
	if (size == 2 && !(address & 1))
	{
		u32 physical;
		if (!translate(address, true, physical, bus))
			return false;
		m_dma_access.write_word(physical, u16(data));
		return true;
	}
	for (unsigned i = 0; i < size; ++i)
	{
		u32 physical;
		if (!translate(address + i, true, physical, bus))
			return false;
		m_dma_access.write_byte(physical, u8(data >> (8 * i)));
	}
	return true;
}

bool mali200_device::vs_input(unsigned stream, u32 vertex, float *value)
{
	u32 const spec = m_vs_config[stream * 2 + 1];
	unsigned const format = spec & 63, group = format >> 2;
	std::fill(value, value + 4, 0.0f);
	value[3] = 1.0f;
	if (format == 63)
		return true;
	unsigned const size = (group == 3 || group == 4 || group == 5 || group == 10 || group == 11) ? 2
			: (group == 6 || group == 7 || group == 8 || group == 9) ? 1
			: 4;
	if (group == 12 || group == 15)
	{
		logerror("unsupported VS input format %u\n", format);
		return false;
	}
	u32 const address = m_vs_config[stream * 2] + vertex * ((spec >> 11) & 0xfffff);
	for (unsigned c = 0; c <= (format & 3); ++c)
	{
		u32 bits;
		if (!read_bytes(address + c * size, size, bits, 6))
			return false;
		if (BIT(spec, 31))
			bits = (size == 4) ? swapendian_int32(bits) : (size == 2) ? swapendian_int16(u16(bits)) : bits;
		s32 const signed_value = size == 1 ? s8(bits) : size == 2 ? s16(bits) : s32(bits);
		switch (group)
		{
		case 0:
			value[c] = mali_float(bits);
			break;
		case 3:
			value[c] = mali_half(bits);
			break;
		case 1:
		case 4:
		case 6:
			value[c] = std::ldexp(float(signed_value), -int((spec >> 6) & 31));
			break;
		case 2:
		case 5:
		case 7:
			value[c] = std::ldexp(float(bits), -int((spec >> 6) & 31));
			break;
		case 8:
		case 10:
		case 13:
			value[c] = std::max(-1.0f, float(double(signed_value) / double((u64(1) << (size * 8 - 1)) - 1)));
			break;
		case 9:
		case 11:
		case 14:
			value[c] = float(double(bits) / double((u64(1) << (size * 8)) - 1));
			break;
		default:
			return false;
		}
	}
	return true;
}

bool mali200_device::vs_output(unsigned stream, u32 vertex, float const *value)
{
	u32 const spec = m_vs_config[0x21 + stream * 2];
	unsigned const format = spec & 63, group = format >> 2;
	if (format == 63)
		return true;
	u32 const address = m_vs_config[0x20 + stream * 2] + vertex * ((spec >> 11) & 0xfffff);
	unsigned const count = format == 32 ? 4 : format == 33 ? 1 : (format & 3) + 1;
	unsigned const size = (group == 3 || group == 4 || group == 5) ? 2 : (group == 6 || group == 7) ? 1 : 4;
	if (format != 32 && format != 33 && group > 7)
	{
		logerror("unsupported VS output format %u\n", format);
		return false;
	}
	for (unsigned c = 0; c < count; ++c)
	{
		u32 bits;
		if (group == 0 || format == 32 || format == 33)
			bits = mali_bits(value[c]);
		else if (group == 3)
			bits = mali_to_half(value[c]);
		else
		{
			double const scaled = std::ldexp(double(value[c]), (spec >> 6) & 31);
			bool const sign = group == 1 || group == 4 || group == 6;
			double const low = sign ? -double(u64(1) << (size * 8 - 1)) : 0;
			double const high = double((u64(1) << (size * 8 - (sign ? 1 : 0))) - 1);
			bits = u32(s64(std::clamp(scaled, low, high)));
		}
		if (BIT(spec, 31))
			bits = (size == 4) ? swapendian_int32(bits) : (size == 2) ? swapendian_int16(u16(bits)) : bits;
		if (!write_bytes(address + c * size, size, bits, 8))
			return false;
	}
	return true;
}

bool mali200_device::vs_vertex(u32 vertex)
{
	float attributes[16][4]{}, varying[16][4]{}, registers[16][4]{}, uniforms[304][4]{};
	for (unsigned i = 0; i <= ((m_vs_config[0x42] >> 24) & 15); ++i)
		if (!vs_input(i, vertex, attributes[i]))
			return false;
	for (unsigned i = 0; i < 304 * 4; ++i)
		uniforms[i / 4][i % 4] = mali_float(m_vs_uniform[i]);

	struct result
	{
		float unit[7]{}, reg0[4]{};
		unsigned complex = 0;
		float argument = 0;
	} previous{}, older{};

	struct delayed_write
	{
		unsigned cycle, kind, index, component;
		float value;
	};

	std::vector<delayed_write> pending;
	int address_register[4]{};
	unsigned pc = m_vs_config[0x40] & 1023, last = (m_vs_config[0x40] >> 10) & 1023;
	if (pc > last || last >= 512)
		return false;
	for (unsigned cycle = 0; pc <= last; ++cycle)
	{
		if (cycle >= 65536)
		{
			logerror("VS shader instruction limit, vertex=%u pc=%03x\n", vertex, pc);
			return false;
		}
		for (auto it = pending.begin(); it != pending.end();)
		{
			if (it->cycle > cycle)
			{
				++it;
				continue;
			}
			if (it->kind == 0)
				registers[it->index][it->component] = it->value;
			else if (it->kind == 1)
				uniforms[it->index][it->component] = it->value;
			else
				address_register[it->index] = int(it->value);
			it = pending.erase(it);
		}
		u32 const *instruction = &m_vs_shader[pc * 4];
		auto f = [&](unsigned bit, unsigned width)
		{
			return mali_field(instruction, bit, width);
		};
		result current{};
		unsigned const register0 = f(58, 4), register1 = f(63, 4);
		for (unsigned c = 0; c < 4; ++c)
			current.reg0[c] = f(62, 1) ? attributes[register0][c] : registers[register0][c];
		int load = f(46, 9);
		unsigned const offset = f(55, 3);
		if (offset < 4)
			load += address_register[offset];
		auto src = [&](unsigned mux, int identity = -1) -> float
		{
			if (mux < 4)
				return current.reg0[mux];
			if (mux < 8)
				return registers[register1][mux - 4];
			if (mux >= 12 && mux <= 15)
				return (load >= 0 && load < 304) ? uniforms[load][mux - 12] : 0;
			if (mux >= 16 && mux <= 20)
				return previous.unit[mux - 16];
			if (mux == 22)
				return identity < 0 ? previous.unit[6] : float(identity);
			if (mux == 23)
				return older.unit[4];
			if (mux >= 24 && mux <= 27)
				return older.unit[mux - 24];
			if (mux >= 28)
				return previous.reg0[mux - 28];
			return 0;
		};
		unsigned const mul_op = f(100, 3), acc_op = f(83, 3);
		for (unsigned unit = 0; unit < 2; ++unit)
		{
			float const a = src(f(unit * 10, 5)), b = src(f(unit * 10 + 5, 5), 1);
			float value = a * b;
			if (mul_op == 4 && unit == 0)
				value = b != 0 ? a : src(f(10, 5));
			else if (mul_op == 1)
			{
				// The native complex unit produces polynomial coefficients, then
				// complex1 combines them with the argument. Evaluate the complete
				// function here; coefficient bits/rounding remain unmodelled.
				if (!previous.complex)
				{
					logerror("VS complex1 without coefficient operation at %03x\n", pc);
					return false;
				}
				float const x = previous.argument;
				switch (previous.complex)
				{
				case 2:
					value = std::exp2(x);
					break;
				case 3:
					value = std::log2(x);
					break;
				case 4:
					value = 1.0f / std::sqrt(x);
					break;
				case 5:
					value = 1.0f / x;
					break;
				default:
					return false;
				}
			}
			else if (mul_op == 3 && unit == 0)
				value = a;
			else if (mul_op != 0 && mul_op != 3 && mul_op != 4)
			{
				logerror("unsupported VS multiply opcode %u\n", mul_op);
				return false;
			}
			current.unit[2 + unit] = f(20 + unit, 1) ? -value : value;
			float aa = src(f(22 + unit * 10, 5)), bb = src(f(27 + unit * 10, 5), 0);
			if (f(42 + unit * 2, 1))
				aa = -aa;
			if (f(43 + unit * 2, 1))
				bb = -bb;
			switch (acc_op)
			{
			case 0:
				current.unit[unit] = aa + bb;
				break;
			case 1:
				current.unit[unit] = std::floor(aa);
				break;
			case 2:
				current.unit[unit] = (aa > 0) - (aa < 0);
				break;
			case 4:
				current.unit[unit] = aa >= bb;
				break;
			case 5:
				current.unit[unit] = aa < bb;
				break;
			case 6:
				current.unit[unit] = std::min(aa, bb);
				break;
			case 7:
				current.unit[unit] = std::max(aa, bb);
				break;
			default:
				logerror("unsupported VS accumulator opcode %u\n", acc_op);
				return false;
			}
		}
		float const pass = src(f(111, 5));
		switch (f(103, 3))
		{
		case 0:
		case 2:
		case 4:
		case 5:
			current.unit[4] = pass;
			break;
		case 6:
			// The viewport reciprocal-W clamp uses (minimum, maximum) in
			// uniform.xy. IGS38's compiler supplies (-1e10, +1e10) here.
			current.unit[4] = (load >= 0 && load < 304) ? std::min(std::max(pass, uniforms[load][0]), uniforms[load][1]) : pass;
			break;
		default:
			logerror("unsupported VS passthrough opcode %u\n", f(103, 3));
			return false;
		}
		float const complex = src(f(106, 5));
		unsigned const complex_op = f(86, 4);
		if (complex_op >= 2 && complex_op <= 5)
		{
			current.complex = complex_op;
			current.argument = complex;
		}
		else if (complex_op == 9)
			current.unit[6] = complex;
		else if (complex_op == 10)
		{
			address_register[0] = int(pass);
			pending.push_back({ cycle + 4, 2, 1, 0, pass });
		}
		else if (complex_op == 12)
			address_register[0] = int(complex);
		else if (complex_op >= 13)
			pending.push_back({ cycle + 4, 2, complex_op - 12, 0, complex });
		else if (complex_op != 0)
		{
			logerror("unsupported VS complex opcode %u\n", complex_op);
			return false;
		}
		for (unsigned component = 0; component < 4; ++component)
		{
			unsigned const source = f(71 + component * 3, 3);
			if (source == 7)
				continue;
			if (source == 5)
				return false;
			unsigned const half = component / 2, index = f(half ? 95 : 90, 4);
			float const value = current.unit[source];
			if (f(67 + half, 1))
			{
				if (address_register[0] < 0 || address_register[0] >= 304)
					return false;
				pending.push_back({ cycle + 4, 1, unsigned(address_register[0]), component, value });
			}
			else if (f(half ? 99 : 94, 1))
				varying[index][component] = value;
			else
				pending.push_back({ cycle + 3, 0, index, component, value });
		}
		++pc;
		if (f(69, 1) && current.unit[4] != 0)
			pc = f(120, 8) | (f(70, 1) ? 0 : 256);
		older = previous;
		previous = current;
	}
	for (unsigned i = 0; i <= ((m_vs_config[0x42] >> 8) & 15); ++i)
		if (!vs_output(i, vertex, varying[i]))
			return false;
	return true;
}

bool mali200_device::vs_commands()
{
	u32 position = m_gp[0x80 / 4];
	for (unsigned commands = 0; position != m_gp[1] && commands < 1048576; ++commands)
	{
		u32 lo, hi;
		if (!read_word(position, lo, 5) || !read_word(position + 4, hi, 5))
			return false;
		if (m_gp_jobs <= 4)
			LOGMASKED(LOG_LISTS, "VS command %08x: %08x %08x\n", position, lo, hi);
		position += 8;
		switch (hi >> 28)
		{
		case 0:
		{
			unsigned const count = (hi & 0xffff) << 8 | (lo >> 24);
			// Indexed PLB draws still consume an ordinary sequential VS batch;
			// the index array is interpreted by the polygon list builder.
			for (unsigned vertex = 0; vertex < count; ++vertex)
				if (!vs_vertex(vertex))
					return false;
			break;
		}
		case 1:
			if ((hi & 0xffff) >= std::size(m_vs_config))
				return false;
			m_vs_config[hi & 0xffff] = lo;
			break;
		case 2:
		{
			// MaliGP2 uses one 256-byte descriptor block for both inputs and
			// outputs (the split descriptors are a later Mali-400 feature).
			unsigned const start = (hi & 0xff) * 4;
			unsigned const count = std::min(64U - std::min(start, 64U), (hi >> 14) & 0x3fff);
			for (unsigned i = 0; i < count; ++i)
				if (!read_word(lo + 4 * i, m_vs_config[start + i], 5))
					return false;
			break;
		}
		case 3:
		case 4:
		{
			unsigned const byte_count = (hi >> 12) & 0xffff;
			unsigned const offset = (hi & 0xfff) * 4;
			u32 *target = (hi >> 28) == 3 ? m_vs_uniform : m_vs_shader;
			unsigned const capacity = (hi >> 28) == 3 ? std::size(m_vs_uniform) : std::size(m_vs_shader);
			if ((byte_count & 3) || offset + byte_count / 4 > capacity)
				return false;
			for (unsigned i = 0; i < byte_count / 4; ++i)
				if (!read_word(lo + i * 4, target[offset + i], 5))
					return false;
			break;
		}
		case 5:
			break; // semaphore ordering: VS runs before its dependent PLB work
		case 6:
			break; // flush: all VS writes are already visible
		case 15:
			position = lo;
			break;
		default:
			logerror("unsupported VS command %08x %08x\n", lo, hi);
			return false;
		}
		m_gp[0x80 / 4] = position;
	}
	return position == m_gp[1];
}
