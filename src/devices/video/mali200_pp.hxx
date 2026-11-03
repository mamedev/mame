// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
// Functional Mali-200 fragment pipeline. Encoding references:
// https://github.com/cwabbott0/mali-isa-docs/blob/master/Utgard-PP.md
// https://gitlab.freedesktop.org/mesa/mesa/-/tree/main/src/gallium/drivers/lima/ir/pp
// Arithmetic currently uses host FP32, not the silicon's intermediate rounding.

namespace
{
float mali_modifier(float value, u32 modifier)
{
	if (modifier & 1)
		value = std::fabs(value);
	return (modifier & 2) ? -value : value;
}

float mali_outmod(float value, u32 modifier)
{
	switch (modifier)
	{
	case 1:
		return mali_clamp(value);
	case 2:
		return std::max(0.0f, value);
	case 3:
		return std::trunc(value);
	default:
		return value;
	}
}

bool mali_alu(bool multiply, u32 op, float a, float b, float &value)
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
	auto const initialize_temporary = [&temporary, &temporary_initialized]
	{
		if (!temporary_initialized)
		{
			std::fill(std::begin(temporary), std::end(temporary), 0);
			temporary_initialized = true;
		}
	};
	u32 address = state[9] & ~31U;
	static constexpr u32 FIELD_SIZE[12] = { 34, 62, 41, 43, 30, 44, 31, 30, 41, 73, 64, 64 };
	for (u32 instructions = 0; instructions < 4096; ++instructions)
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
			for (u32 i = 1; i < instruction.count; ++i)
				if (!read_word(address + i * 4, words[i], 1))
					return false;
			u32 next = 32;
			for (u32 field = 0; field < 12; ++field)
			{
				if (!BIT(instruction.fields, field))
					continue;
				u32 const size = FIELD_SIZE[field];
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
			for (u32 n = 0; n < 2; ++n)
				if (BIT(instruction.fields, 10 + n))
					for (u32 c = 0; c < 4; ++c)
						instruction.constant[n][c] = mali_half(instruction.field[10 + n] >> (c * 16));
			// Single texture fetch followed by an unmodified color MOV.
			if (BIT(instruction.control, 5) && (instruction.fields == 0x23 || instruction.fields == 0x0b))
			{
				u64 const varying = instruction.field[0], texture = instruction.field[1];
				u64 const move = instruction.field[instruction.fields == 0x23 ? 5 : 3];
				instruction.texture_copy = (varying & 15) == 0 && ((varying >> 24) & 15) == 15 &&
						!(texture & ((1U << 18) | (31U << 24) | (1U << 29))) && (move & 0x3fff) == (14 | (0xe4 << 4)) &&
						((move >> 32) & 15) == 15 && ((move >> 36) & 3) == 0 && ((move >> 38) & 31) == 31 && !BIT(move, 43);
				u32 const alignment = (varying >> 5) & 3;
				instruction.texture_components = alignment == 3 ? 4 : alignment + 1;
				instruction.texture_varying = ((varying >> 18) & 63) * instruction.texture_components;
				instruction.texture_sampler = (texture >> 30) & 0xfff;
				instruction.texture_destination = (move >> 28) & 15;
			}
		}
		u32 const count = instruction.count, fields = instruction.fields;
		// Only immediate constants and arithmetic can be reused for another
		// fragment. Exclude varying/texture/uniform fetches, framebuffer or
		// temporary access, and all control flow (including discard).
		constant &= !(fields & 0x307);
		if (!instructions && instruction.texture_copy && !BIT(state[3], 11) && ((state[8] >> 16) & 15) == instruction.texture_destination)
		{
			float coordinate[4]{};
			for (u32 c = 0; c < instruction.texture_components; ++c)
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
		auto const f = [&instruction] (u32 field, u32 start, u32 size) -> u32
		{
			u64 value = instruction.field[field] >> start;
			if (start + size > 64)
				value |= u64(instruction.upper[field]) << (64 - start);
			return u32(value) & util::make_bitmask<u32>(size);
		};
		auto const scalar = [&registers] (u32 reg)
		{
			return registers[(reg >> 2) & 15][reg & 3];
		};
		for (u32 slot = 0; slot < 2; ++slot)
			if (BIT(fields, 10 + slot))
				std::copy(std::begin(instruction.constant[slot]), std::end(instruction.constant[slot]), registers[12 + slot]);
		float coordinate[4]{};
		if (BIT(fields, 0))
		{
			u32 const source = f(0, 0, 4), dest = f(0, 24, 4), mask = f(0, 28, 4);
			if ((source & 12) == 0 || source == 8)
			{
				u32 const alignment = f(0, 5, 2), stride = alignment == 3 ? 4 : alignment + 1;
				s64 index = f(0, 18, 6);
				u32 const offset_reg = (f(0, 10, 4) << 2) | f(0, 16, 2);
				if (offset_reg != 63)
					index += mali_to_integer(scalar(offset_reg));
				index *= stride;
				for (u32 c = 0; c < stride; ++c)
					if (index + c >= 0 && index + c < fragment.varying_count)
						coordinate[c] = fragment.varying[index + c];
			}
			else if ((source & 12) == 4 || source == 9 || source == 10)
			{
				u32 const reg = f(0, 10, 4), swizzle = f(0, 16, 8);
				for (u32 c = 0; c < 4; ++c)
					coordinate[c] = mali_modifier(registers[reg][(swizzle >> (2 * c)) & 3], f(0, 15, 1) | (f(0, 14, 1) << 1));
				if (source == 10)
				{
					float const length =
							std::sqrt(coordinate[0] * coordinate[0] + coordinate[1] * coordinate[1] + coordinate[2] * coordinate[2]);
					for (u32 c = 0; c < 3; ++c)
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
				for (u32 c = 0; c < 4; ++c)
					coordinate[c] /= divisor;
			}
			if (dest != 15)
				for (u32 c = 0; c < 4; ++c)
					if (BIT(mask, c))
						registers[dest][c] = coordinate[c];
		}
		if (BIT(fields, 1))
		{
			s64 sampler = f(1, 30, 12);
			if (f(1, 29, 1))
				sampler += mali_to_integer(scalar(f(1, 6, 6)));
			if (sampler < 0 || sampler >= 16)
				return false;
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
			u32 const alignment = f(2, 10, 2), components = 1U << std::min(alignment, 2U);
			s64 index = s16(f(2, 25, 16));
			if (f(2, 24, 1))
				index += mali_to_integer(scalar(f(2, 18, 6)));
			index *= components;
			if (f(2, 0, 2) == 3)
			{
				initialize_temporary();
				for (u32 c = 0; c < components; ++c)
				{
					if (index + c < 0 || index + c >= s64(std::size(temporary)))
						return false;
					registers[15][c] = temporary[index + c];
				}
			}
			else
			{
				if (index < 0 || index + components > 0x1'0000'0000LL)
					return false;
				for (u32 c = 0; c < components; ++c)
				{
					u32 base, half;
					u32 const scalar_index = index + c;
					if (!read_word((state[11] & ~15U) + (scalar_index / 128) * 4, base, 2))
						return false;
					if (!read_bytes((base & ~3U) + (scalar_index % 128) * 2, 2, half, 2))
						return false;
					registers[15][c] = mali_half(half);
				}
			}
		}
		float vector_product[4]{}, scalar_product = 0;
		for (u32 stage = 0; stage < 2; ++stage)
		{
			u32 const vec = 3 + stage * 2, scl = vec + 1;
			float vector_value[4]{}, scalar_value = 0;
			if (BIT(fields, vec))
			{
				float a[4], b[4];
				for (u32 c = 0; c < 4; ++c)
				{
					a[c] = (stage && f(vec, 43, 1)) ? vector_product[(f(vec, 4, 8) >> (2 * c)) & 3]
							: registers[f(vec, 0, 4)][(f(vec, 4, 8) >> (2 * c)) & 3];
					a[c] = mali_modifier(a[c], f(vec, 12, 2));
					b[c] = mali_modifier(registers[f(vec, 14, 4)][(f(vec, 18, 8) >> (2 * c)) & 3], f(vec, 26, 2));
				}
				u32 const op = f(vec, 38, 5);
				for (u32 c = 0; c < 4; ++c)
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
				u32 const op = f(scl, 25, 5);
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
				for (u32 c = 0; c < 4; ++c)
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
			u32 const mode = f(7, 0, 2);
			float a = mali_modifier(scalar(f(7, 16, 6)), f(7, 14, 2));
			if (mode == 3)
			{
				float result[4];
				for (u32 c = 0; c < 4; ++c)
					result[c] = a * registers[f(7, 10, 4)][(f(7, 2, 8) >> (c * 2)) & 3];
				for (u32 c = 0; c < 4; ++c)
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
				u32 const dest = f(7, 24, 6);
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
				u32 const dest = f(8, 6, 4);
				if (f(8, 0, 1))
					std::copy(std::begin(fragment.color), std::end(fragment.color), registers[dest]);
				else
					registers[dest][0] = fragment.depth;
			}
			else
			{
				u32 const components = 1U << std::min(2U, f(8, 10, 2)), reg = f(8, 4, 6);
				s64 index = s16(f(8, 25, 16));
				if (f(8, 24, 1))
					index += mali_to_integer(scalar(f(8, 18, 6)));
				index *= components;
				initialize_temporary();
				for (u32 c = 0; c < components; ++c)
				{
					if (index + c < 0 || index + c >= s64(std::size(temporary)))
						return false;
					temporary[index + c] = scalar((reg & ~(components - 1)) + c);
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
			u32 const condition = f(9, 16, 3);
			if ((BIT(condition, 0) && a > b) || (BIT(condition, 1) && a == b) || (BIT(condition, 2) && a < b))
			{
				s32 const relative = util::sext(f(9, 41, 27), 27);
				next_address = address + relative * 4;
			}
		}
		if (BIT(instruction.control, 5))
		{
			u32 const color_register = (state[8] >> 16) & 15;
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
