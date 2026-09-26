// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
// Functional Mali-200 fragment pipeline. Encoding references:
// https://github.com/cwabbott0/mali-isa-docs/blob/master/Utgard-PP.md
// https://gitlab.freedesktop.org/mesa/mesa/-/tree/main/src/gallium/drivers/lima/ir/pp
// Arithmetic currently uses host FP32, not the silicon's intermediate rounding.

namespace
{
float mali_modifier(float value, unsigned modifier)
{
	if (modifier & 1)
		value = std::fabs(value);
	return (modifier & 2) ? -value : value;
}

float mali_outmod(float value, unsigned modifier)
{
	switch (modifier)
	{
	case 1:
		return std::clamp(value, 0.0f, 1.0f);
	case 2:
		return std::max(0.0f, value);
	case 3:
		return std::trunc(value);
	default:
		return value;
	}
}

bool mali_alu(bool multiply, unsigned op, float a, float b, float &value)
{
	if (multiply)
	{
		if (op < 8)
		{
			value = a * b;
			if (op)
				value = std::ldexp(value, (op & 4) ? int(op) - 8 : int(op));
			return true;
		}
		switch (op)
		{
		case 8:
			value = !a;
			break;
		case 9:
			value = bool(a) && bool(b);
			break;
		case 10:
			value = bool(a) || bool(b);
			break;
		case 11:
			value = bool(a) != bool(b);
			break;
		case 12:
			value = a != b;
			break;
		case 13:
			value = a > b;
			break;
		case 14:
			value = a >= b;
			break;
		case 15:
			value = a == b;
			break;
		case 16:
			value = std::min(a, b);
			break;
		case 17:
			value = std::max(a, b);
			break;
		case 31:
			value = a;
			break;
		default:
			return false;
		}
	}
	else
	{
		switch (op)
		{
		case 0:
			value = a + b;
			break;
		case 4:
			value = a - std::floor(a);
			break;
		case 8:
			value = a != b;
			break;
		case 9:
			value = a > b;
			break;
		case 10:
			value = a >= b;
			break;
		case 11:
			value = a == b;
			break;
		case 12:
			value = std::floor(a);
			break;
		case 13:
			value = std::ceil(a);
			break;
		case 14:
			value = std::min(a, b);
			break;
		case 15:
			value = std::max(a, b);
			break;
		case 31:
			value = a;
			break;
		default:
			return false;
		}
	}
	return true;
}
}

bool mali200_device::pp_shader(u32 const *state, pp_fragment &fragment, bool &constant)
{
	constant = true;
	float registers[16][4], temporary[1024];
	bool registers_initialized = false;
	bool temporary_initialized = false;
	auto initialize_temporary = [&]
	{
		if (!temporary_initialized)
		{
			std::fill(std::begin(temporary), std::end(temporary), 0);
			temporary_initialized = true;
		}
	};
	u32 address = state[9] & ~31U;
	static constexpr unsigned FIELD_SIZE[12] = { 34, 62, 41, 43, 30, 44, 31, 30, 41, 73, 64, 64 };
	for (unsigned instructions = 0; instructions < 4096; ++instructions)
	{
		pp_instruction *&cached = m_pp_instruction_cache[(address >> 2) & 63];
		bool inserted = false;
		if (!cached || cached->address != address)
		{
			auto const entry = m_pp_instructions.try_emplace(address);
			cached = &entry.first->second;
			inserted = entry.second;
		}
		pp_instruction &instruction = *cached;
		if (inserted)
		{
			instruction.address = address;
			u32 words[32]{};
			if (!read_word(address, words[0], 1))
				return false;
			instruction.control = words[0];
			instruction.count = words[0] & 31;
			instruction.fields = (words[0] >> 7) & 0xfff;
			if (!instruction.count)
			{
				logerror("invalid PP shader length at %08x\n", address);
				return false;
			}
			for (unsigned i = 1; i < instruction.count; ++i)
				if (!read_word(address + i * 4, words[i], 1))
					return false;
			unsigned next = 32;
			for (unsigned field = 0; field < 12; ++field)
			{
				if (!BIT(instruction.fields, field))
					continue;
				unsigned const size = FIELD_SIZE[field];
				if (next + size > instruction.count * 32)
				{
					logerror("truncated PP shader at %08x\n", address);
					return false;
				}
				instruction.field[field] = mali_field(words, next, std::min(size, 32U));
				if (size > 32)
					instruction.field[field] |= u64(mali_field(words, next + 32, std::min(size - 32, 32U))) << 32;
				if (size > 64)
					instruction.upper[field] = mali_field(words, next + 64, size - 64);
				next += size;
			}
			// Constants belong to the decoded instruction, so their exact
			// half-to-float conversion need only happen once for this job.
			for (unsigned n = 0; n < 2; ++n)
				if (BIT(instruction.fields, 10 + n))
					for (unsigned c = 0; c < 4; ++c)
						instruction.constant[n][c] = mali_half(instruction.field[10 + n] >> (c * 16));
			// A single fetch followed by an unmodified texture-to-color MOV
			// needs no register-file interpreter. Keep sampling in pp_texture
			// so formats, addressing, filtering and memory faults are shared.
			if (BIT(instruction.control, 5) && (instruction.fields == 0x23 || instruction.fields == 0x0b))
			{
				u64 const varying = instruction.field[0], texture = instruction.field[1];
				u64 const move = instruction.field[instruction.fields == 0x23 ? 5 : 3];
				instruction.texture_copy = (varying & 15) == 0 && ((varying >> 24) & 15) == 15 &&
						!(texture & ((1U << 18) | (31U << 24) | (1U << 29))) && (move & 0x3fff) == (14 | (0xe4 << 4)) &&
						((move >> 32) & 15) == 15 && ((move >> 36) & 3) == 0 && ((move >> 38) & 31) == 31 && !BIT(move, 43);
				unsigned const alignment = (varying >> 5) & 3;
				instruction.texture_components = alignment == 3 ? 4 : alignment + 1;
				instruction.texture_varying = ((varying >> 18) & 63) * instruction.texture_components;
				instruction.texture_sampler = (texture >> 30) & 0xfff;
				instruction.texture_destination = (move >> 28) & 15;
			}
		}
		unsigned const count = instruction.count, fields = instruction.fields;
		// Only immediate constants and arithmetic can be reused for another
		// fragment. Exclude varying/texture/uniform fetches, framebuffer or
		// temporary access, and all control flow (including discard).
		constant &= !(fields & 0x307);
		if (!instructions && instruction.texture_copy && !BIT(state[3], 11) && ((state[8] >> 16) & 15) == instruction.texture_destination)
		{
			float coordinate[4]{};
			for (unsigned c = 0; c < instruction.texture_components; ++c)
				if (instruction.texture_varying + c < fragment.varying_count)
					coordinate[c] = fragment.varying[instruction.texture_varying + c];
			return pp_texture(state, instruction.texture_sampler, coordinate, 0, fragment.color);
		}
		if (!registers_initialized)
		{
			for (auto &reg : registers)
				std::fill(std::begin(reg), std::end(reg), 0);
			registers_initialized = true;
		}
		auto f = [&](unsigned field, unsigned start, unsigned size) -> u32
		{
			u64 value = instruction.field[field] >> start;
			if (start + size > 64)
				value |= u64(instruction.upper[field]) << (64 - start);
			return u32(value) & (size == 32 ? ~0U : (1U << size) - 1);
		};
		auto scalar = [&](unsigned reg)
		{
			return registers[(reg >> 2) & 15][reg & 3];
		};
		for (unsigned constant = 0; constant < 2; ++constant)
			if (BIT(fields, 10 + constant))
				std::copy(std::begin(instruction.constant[constant]), std::end(instruction.constant[constant]), registers[12 + constant]);
		float coordinate[4]{};
		if (BIT(fields, 0))
		{
			unsigned const source = f(0, 0, 4), dest = f(0, 24, 4), mask = f(0, 28, 4);
			if ((source & 12) == 0 || source == 8)
			{
				unsigned const alignment = f(0, 5, 2), stride = alignment == 3 ? 4 : alignment + 1;
				int index = f(0, 18, 6);
				unsigned const offset_reg = (f(0, 10, 4) << 2) | f(0, 16, 2);
				if (offset_reg != 63)
					index += int(scalar(offset_reg));
				index *= stride;
				for (unsigned c = 0; c < stride; ++c)
					if (index + int(c) >= 0 && index + c < fragment.varying_count)
						coordinate[c] = fragment.varying[index + c];
			}
			else if ((source & 12) == 4 || source == 9 || source == 10)
			{
				unsigned const reg = f(0, 10, 4), swizzle = f(0, 16, 8);
				for (unsigned c = 0; c < 4; ++c)
					coordinate[c] = mali_modifier(registers[reg][(swizzle >> (2 * c)) & 3], f(0, 15, 1) | (f(0, 14, 1) << 1));
				if (source == 10)
				{
					float const length =
							std::sqrt(coordinate[0] * coordinate[0] + coordinate[1] * coordinate[1] + coordinate[2] * coordinate[2]);
					for (unsigned c = 0; c < 3; ++c)
						coordinate[c] /= length;
				}
			}
			else if (source == 11)
				std::copy(std::begin(fragment.coordinate), std::end(fragment.coordinate), coordinate);
			else if (source == 13)
				coordinate[0] = fragment.front ? 1.0f : -1.0f;
			else
			{
				logerror("unsupported PP varying source %u\n", source);
				return false;
			}
			if (source < 8 && (source & 3) >= 2)
			{
				float const divisor = coordinate[source & 3];
				for (unsigned c = 0; c < 4; ++c)
					coordinate[c] /= divisor;
			}
			if (dest != 15)
				for (unsigned c = 0; c < 4; ++c)
					if (BIT(mask, c))
						registers[dest][c] = coordinate[c];
		}
		if (BIT(fields, 1))
		{
			unsigned sampler = f(1, 30, 12);
			if (f(1, 29, 1))
				sampler += int(scalar(f(1, 6, 6)));
			float const lod = f(1, 18, 1) ? scalar(f(1, 0, 6)) : 0;
			if (f(1, 24, 5) != 0)
			{
				logerror("unsupported PP cube texture\n");
				return false;
			}
			if (!pp_texture(state, sampler, coordinate, lod, registers[14]))
				return false;
		}
		if (BIT(fields, 2))
		{
			unsigned const alignment = f(2, 10, 2), components = 1U << std::min(alignment, 2U);
			int index = s16(f(2, 25, 16));
			if (f(2, 24, 1))
				index += int(scalar(f(2, 18, 6)));
			index *= components;
			if (f(2, 0, 2) == 3)
			{
				initialize_temporary();
				for (unsigned c = 0; c < components; ++c)
				{
					if (index + int(c) < 0 || index + c >= std::size(temporary))
						return false;
					registers[15][c] = temporary[index + c];
				}
			}
			else
			{
				if (index < 0)
					return false;
				for (unsigned c = 0; c < components; ++c)
				{
					u32 base, half;
					unsigned const scalar_index = index + c;
					if (!read_word((state[11] & ~15U) + (scalar_index / 128) * 4, base, 2))
						return false;
					if (!read_bytes((base & ~3U) + (scalar_index % 128) * 2, 2, half, 2))
						return false;
					registers[15][c] = mali_half(half);
				}
			}
		}
		float vector_product[4]{}, scalar_product = 0;
		for (unsigned stage = 0; stage < 2; ++stage)
		{
			unsigned const vec = 3 + stage * 2, scl = vec + 1;
			float vector_value[4]{}, scalar_value = 0;
			if (BIT(fields, vec))
			{
				float a[4], b[4];
				for (unsigned c = 0; c < 4; ++c)
				{
					a[c] = (stage && f(vec, 43, 1)) ? vector_product[(f(vec, 4, 8) >> (2 * c)) & 3]
							: registers[f(vec, 0, 4)][(f(vec, 4, 8) >> (2 * c)) & 3];
					a[c] = mali_modifier(a[c], f(vec, 12, 2));
					b[c] = mali_modifier(registers[f(vec, 14, 4)][(f(vec, 18, 8) >> (2 * c)) & 3], f(vec, 26, 2));
				}
				unsigned const op = f(vec, 38, 5);
				for (unsigned c = 0; c < 4; ++c)
				{
					if (stage && (op == 16 || op == 17))
						vector_value[c] = a[0] + a[1] + a[2] + (op == 17 ? a[3] : 0);
					else if (stage && op == 23)
						vector_value[c] = scalar_product != 0 ? a[c] : b[c];
					else if (!mali_alu(!stage, op, a[c], b[c], vector_value[c]))
					{
						logerror("unsupported PP vector op stage=%u op=%u\n", stage, op);
						return false;
					}
					vector_value[c] = mali_outmod(vector_value[c], f(vec, 36, 2));
				}
			}
			if (BIT(fields, scl))
			{
				float a = (stage && f(scl, 30, 1)) ? scalar_product : scalar(f(scl, 0, 6));
				a = mali_modifier(a, f(scl, 6, 2));
				float const b = mali_modifier(scalar(f(scl, 8, 6)), f(scl, 14, 2));
				unsigned const op = f(scl, 25, 5);
				if (stage && op == 23)
					scalar_value = scalar_product != 0 ? a : b;
				else if (!mali_alu(!stage, op, a, b, scalar_value))
				{
					logerror("unsupported PP scalar op stage=%u op=%u\n", stage, op);
					return false;
				}
				scalar_value = mali_outmod(scalar_value, f(scl, 23, 2));
			}
			// The vector and scalar units read the same incoming register bank.
			if (BIT(fields, vec))
				for (unsigned c = 0; c < 4; ++c)
					if (BIT(f(vec, 32, 4), c))
						registers[f(vec, 28, 4)][c] = vector_value[c];
			if (BIT(fields, scl) && f(scl, 22, 1))
				registers[f(scl, 16, 6) / 4][f(scl, 16, 6) % 4] = scalar_value;
			if (!stage)
			{
				std::copy(std::begin(vector_value), std::end(vector_value), vector_product);
				scalar_product = scalar_value;
			}
		}
		if (BIT(fields, 7))
		{
			unsigned const mode = f(7, 0, 2);
			float a = mali_modifier(scalar(f(7, 16, 6)), f(7, 14, 2));
			if (mode == 3)
			{
				float result[4];
				for (unsigned c = 0; c < 4; ++c)
					result[c] = a * registers[f(7, 10, 4)][(f(7, 2, 8) >> (c * 2)) & 3];
				for (unsigned c = 0; c < 4; ++c)
					if (BIT(f(7, 22, 4), c))
						registers[f(7, 26, 4)][c] = result[c];
			}
			else if (mode == 0)
			{
				float value;
				switch (f(7, 2, 4))
				{
				case 0:
					value = 1.0f / a;
					break;
				case 1:
					value = a;
					break;
				case 2:
					value = std::sqrt(a);
					break;
				case 3:
					value = 1.0f / std::sqrt(a);
					break;
				case 4:
					value = std::exp2(a);
					break;
				case 5:
					value = std::log2(a);
					break;
				case 6:
					value = std::sin(a * float(2 * std::numbers::pi));
					break;
				case 7:
					value = std::cos(a * float(2 * std::numbers::pi));
					break;
				default:
					logerror("unsupported PP combine op %u\n", f(7, 2, 4));
					return false;
				}
				unsigned const dest = f(7, 24, 6);
				registers[dest / 4][dest % 4] = mali_outmod(value, f(7, 22, 2));
			}
			else
			{
				logerror("unsupported PP atan combine mode %u\n", mode);
				return false;
			}
		}
		if (BIT(fields, 8))
		{
			if (f(8, 2, 2) == 3)
			{
				unsigned const dest = f(8, 6, 4);
				if (f(8, 0, 1))
					std::copy(std::begin(fragment.color), std::end(fragment.color), registers[dest]);
				else
					registers[dest][0] = fragment.depth;
			}
			else
			{
				unsigned const count = 1U << std::min(2U, f(8, 10, 2)), reg = f(8, 4, 6);
				int index = s16(f(8, 25, 16));
				if (f(8, 24, 1))
					index += int(scalar(f(8, 18, 6)));
				index *= count;
				initialize_temporary();
				for (unsigned c = 0; c < count; ++c)
				{
					if (index + int(c) < 0 || index + c >= std::size(temporary))
						return false;
					temporary[index + c] = scalar((reg & ~(count - 1)) + c);
				}
			}
		}
		u32 next_address = address + 4 * count;
		if (BIT(fields, 9))
		{
			if (f(9, 0, 32) == 0x007f0003)
			{
				fragment.discard = true;
				return true;
			}
			float const a = scalar(f(9, 10, 6)), b = scalar(f(9, 4, 6));
			unsigned const condition = f(9, 16, 3);
			if ((BIT(condition, 0) && a > b) || (BIT(condition, 1) && a == b) || (BIT(condition, 2) && a < b))
			{
				s32 const relative = s32(f(9, 41, 27) << 5) >> 5;
				next_address = address + relative * 4;
			}
		}
		if (BIT(instruction.control, 5))
		{
			unsigned const color_register = (state[8] >> 16) & 15;
			std::copy(std::begin(registers[color_register]), std::end(registers[color_register]), fragment.color);
			if (BIT(state[3], 11))
				fragment.depth = scalar((state[3] >> 6) & 15);
			return true;
		}
		address = next_address;
	}
	logerror("PP shader instruction limit at %08x\n", address);
	return false;
}
