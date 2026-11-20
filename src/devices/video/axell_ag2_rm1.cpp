// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
/*
    Axell RM1 / RAPIC picture reconstruction.

    The bitstream syntax and I/P/B reconstruction are derived from the AG-2
    game ROMs. Reads are bounded and reference pictures are replaced only after
    a complete successful decode. Supported geometry is the observed 640x480
    planar Y/Cr/Cb 4:2:0; other geometries and encoding variants are rejected.

    Hardware comparisons are still needed for exact reciprocal precision,
    signed rounding and chroma interpolation. A flat selected basis contributes
    zero detail. Physical output packing, colour conversion, workspace placement
    and engine timing belong to axell_ag2.cpp and remain provisional there.
*/
#include "emu.h"

#include "axell_ag2_rm1.h"

#include <algorithm>
#include <numeric>
#include <string>
#include <utility>

namespace axell_rm1
{

namespace
{

constexpr std::uint32_t WIDTH = frame::WIDTH;
constexpr std::uint32_t HEIGHT = frame::HEIGHT;
using symbol_table = std::array<std::uint8_t, 256>;
using block_pixels = std::array<std::uint8_t, 16>;
using basis_sums = std::array<std::int64_t, 16>;
using basis_nest = std::array<std::uint8_t, 70 * 38>;

void require(bool condition, const char *message)
{
	if (!condition)
		throw decode_error(message);
}

// RM1's fixed-point arithmetic uses floor division, including for negative
// residuals and motion coordinates.
std::int64_t floor_div(std::int64_t value, std::int64_t divisor)
{
	return value >= 0 ? value / divisor : -1 - ((-1 - value) / divisor);
}

std::uint8_t clip(std::int64_t value)
{
	return std::uint8_t(std::clamp<std::int64_t>(value, 0, 255));
}

struct byte_view
{
	const std::uint8_t *data;
	std::size_t size;

	std::uint32_t read_uint(std::size_t position, std::uint32_t count) const
	{
		require(count <= 4 && position <= size && count <= size - position, "truncated header field");
		std::uint32_t result = 0;
		for (std::uint32_t i = 0; i < count; ++i)
			result = (result << 8) | data[position + i];
		return result;
	}

	byte_view slice(std::size_t first, std::size_t last) const
	{
		require(first <= last && last <= size, "invalid stream extent");
		return { data + first, last - first };
	}
};

class bit_reader
{
public:
	explicit bit_reader(byte_view view) : limit(view.size * 8), m_view(view) { }
	bit_reader(byte_view view, std::size_t bit_count) : limit(bit_count), m_view(view)
	{
		require(bit_count <= view.size * 8, "invalid bitstream length");
	}

	std::uint32_t read(std::uint32_t count)
	{
		require(count <= 32 && position <= limit && count <= limit - position, "truncated bitstream");
		std::uint32_t result = 0;
		for (std::uint32_t i = 0; i < count; ++i, ++position)
			result = (result << 1) | ((m_view.data[position / 8] >> (7 - position % 8)) & 1);
		return result;
	}

	std::uint32_t unary()
	{
		std::uint32_t value = 0;
		while (!read(1))
			require(++value < 256, "unary value exceeds symbol alphabet");
		return value;
	}

	std::uint32_t symbol(const symbol_table &table)
	{
		return table[unary()];
	}

	void finish()
	{
		require(position <= limit && limit - position <= 31, "unconsumed stream data");
		require(read(std::uint32_t(limit - position)) == 0, "nonzero alignment padding");
	}

	std::size_t position = 0;
	std::size_t limit;

private:
	byte_view m_view;
};

std::uint32_t run_value(bit_reader &bits, const symbol_table &table, std::uint32_t low_width)
{
	const std::uint32_t q = bits.symbol(table);
	const std::uint32_t low = bits.read(low_width);
	return (q << low_width) + low;
}

symbol_table alphabet(byte_view payload, std::uint32_t offset, std::uint32_t length)
{
	symbol_table table{};
	std::array<bool, 256> used{};
	std::uint32_t count = 0;
	for (std::uint32_t i = 0; i < length; ++i)
	{
		const std::uint32_t value = payload.read_uint(offset + i, 1);
		if (used[value])
			break;
		table[count++] = std::uint8_t(value);
		used[value] = true;
	}
	for (std::uint32_t value = 0; value < 256; ++value)
		if (!used[value])
			table[count++] = std::uint8_t(value);
	return table;
}

std::array<std::size_t, 5> offsets(byte_view payload, bool intra)
{
	const std::uint32_t base = intra ? 0x24 : 0x50;
	std::array<std::size_t, 5> result{};
	for (std::uint32_t i = 0; i < 4; ++i)
		result[i] = payload.read_uint(base + i * 4, 4);
	result[4] = payload.size;
	require(result[0] == (intra ? 0x34U : 0x60U)
			&& std::is_sorted(result.begin(), result.end()), "invalid picture region offsets");
	return result;
}

int balanced(std::uint32_t quotient, std::uint32_t suffix, std::uint32_t width)
{
	require(width <= 8 && suffix < (1U << (width + bool(quotient))), "invalid balanced code");
	int remainder = int(suffix & ((1U << width) - 1));
	if (width && (remainder & (1 << (width - 1))))
		remainder -= 1 << width;
	const int magnitude = int(quotient * (1U << width)) + remainder;
	return quotient && (suffix >> width) ? -magnitude : magnitude;
}

int dc_delta(std::uint32_t quotient, std::uint32_t suffix, std::uint32_t width)
{
	require(width == 0 || width == 2, "unsupported DC precision");
	return balanced(quotient, suffix, width) * (1 << (2 - width));
}

struct macroblock
{
	std::uint32_t reference = 0;
	std::uint32_t processing = 0;
	int dx = 0;
	int dy = 0;
};

struct picture_fields
{
	std::array<std::vector<std::uint8_t>, 3> modes;
	std::array<std::vector<std::int16_t>, 3> means;
	std::array<macroblock, 80 * 60> controls;
};

void decode_intra_fields(byte_view payload, picture_fields &fields, basis_nest &nest)
{
	const auto region = offsets(payload, true);
	const std::uint32_t nx = payload.read_uint(0, 2);
	const std::uint32_t ny = payload.read_uint(2, 2);
	require(nx + 70 <= 160 && ny + 38 <= 120, "nest exceeds DC plane");
	const std::uint32_t k = payload.read_uint(7, 1);
	require(k == 0 || k == 2, "unsupported intra DC precision");
	const auto values = alphabet(payload, 8, 4);
	const auto runs = alphabet(payload, 20, 4);
	const auto mode_table = alphabet(payload, 12, 8);
	bit_reader c(payload.slice(region[2], region[3]));
	auto &luma = fields.means[0];
	luma.reserve(160 * 120);
	for (std::uint32_t y = 0; y < 120; ++y)
	{
		if (y == ny)
			require(c.position == payload.read_uint(0x20, 4), "nest C entry offset mismatch");
		std::uint32_t run = 0;
		for (std::uint32_t x = 0; x < 160; ++x)
		{
			const std::uint32_t q = run ? 0 : c.symbol(values);
			if (!q && !run)
			{
				run = run_value(c, runs, 4) + 1;
				require(run <= 160 - x, "luma DC run exceeds row");
			}
			if (!q)
				--run;
			const std::uint32_t suffix = c.read(k + bool(q));
			const int top = (y && (y != ny)) ? luma[(y - 1) * 160 + x] : 127;
			const int predicted = x ? (luma.back() + top + 1) / 2 : top;
			const int value = predicted + dc_delta(q, suffix, k);
			require(value >= 0 && value <= 255, "luma DC outside byte range");
			luma.push_back(std::int16_t(value));
		}
	}
	c.finish();
	for (std::uint32_t y = 0; y < 38; ++y)
		for (std::uint32_t x = 0; x < 70; ++x)
			nest[y * 70 + x] = std::uint8_t(luma[(ny + y) * 160 + nx + x] / 16);

	bit_reader d(payload.slice(region[3], region[4]));
	const auto modes = [&d, &mode_table, &runs, &fields] (std::uint32_t plane)
	{
		const std::uint32_t columns = plane == 0 ? 160 : 80;
		std::uint32_t x = 0;
		while (x < columns)
		{
			const std::uint32_t mode = d.symbol(mode_table);
			require(mode <= 7, "invalid intra block mode");
			const std::uint32_t count = mode ? 1 : run_value(d, runs, 4) + 1;
			require(count <= columns - x, "intra mode run exceeds row");
			fields.modes[plane].insert(fields.modes[plane].end(), count, std::uint8_t(mode));
			x += count;
		}
	};
	const auto means = [&fields, &d, &values, &runs, k] (std::uint32_t plane, std::uint32_t y)
	{
		auto &output = fields.means[plane];
		std::uint32_t run = 0;
		for (std::uint32_t x = 0; x < 80; ++x)
		{
			const std::uint32_t q = run ? 0 : d.symbol(values);
			if (!q && !run)
			{
				run = run_value(d, runs, 4) + 1;
				require(run <= 80 - x, "chroma DC run exceeds row");
			}
			if (!q)
				--run;
			const int delta = dc_delta(q, d.read(k + bool(q)), k);
			const int top = y ? output[(y - 1) * 80 + x] : 127;
			const int predicted = x ? (output.back() + top + 1) / 2 : top;
			const int value = predicted + delta;
			require(value >= 0 && value <= 255, "chroma DC outside byte range");
			output.push_back(std::int16_t(value));
		}
	};
	for (std::uint32_t y = 0; y < 60; ++y)
	{
		if (y >= 2)
			modes(0);
		modes(1);
		modes(2);
		modes(0);
		means(1, y);
		means(2, y);
	}
	modes(0);
	modes(0);
	d.finish();
}

void decode_inter_fields(byte_view payload, picture_fields &fields)
{
	const auto region = offsets(payload, false);
	const auto types = alphabet(payload, 32, 16);
	const auto processing = alphabet(payload, 48, 16);
	const auto motion = alphabet(payload, 64, 16);
	bit_reader c(payload.slice(region[2], region[3]));
	std::uint32_t previous_reference = 0;
	std::array<int, 2> vector{};
	for (std::uint32_t y = 0; y < 60; ++y)
	{
		std::array<std::uint32_t, 80> reference{};
		std::array<std::uint32_t, 80> process{};
		std::uint32_t code = c.read(2);
		std::uint32_t x = 0;
		std::uint32_t active = 0;
		require(code <= 2, "invalid macroblock reference");
		while (x < 80)
		{
			const std::uint32_t run = run_value(c, types, 2) + 1;
			require(run <= 80 - x, "reference run exceeds row");
			for (std::uint32_t i = 0; i < run; ++i)
				reference[x++] = code;
			if (code)
				active += run;
			if (x < 80)
				code = (code + (c.read(1) ? 2 : 1)) % 3;
		}
		if (active)
		{
			code = c.read(1);
			x = 0;
			while (x < active)
			{
				const std::uint32_t run = run_value(c, processing, 2) + 1;
				require(run <= active - x, "processing run exceeds row");
				for (std::uint32_t i = 0; i < run; ++i)
					process[x++] = code;
				code ^= 1;
			}
		}
		std::uint32_t pi = 0;
		for (x = 0; x < 80; ++x)
		{
			auto &block = fields.controls[y * 80 + x];
			block.reference = reference[x];
			if (!block.reference)
				continue;
			if (block.reference != previous_reference)
				vector = { 0, 0 };
			previous_reference = block.reference;
			for (std::uint32_t axis = 0; axis < 2; ++axis)
			{
				const std::uint32_t k = payload.read_uint(4 + axis + (block.reference == 2 ? 2 : 0), 1);
				require(k <= 8, "unsupported motion width");
				const std::uint32_t q = c.symbol(motion);
				const std::uint32_t suffix = c.read(k + bool(q));
				const int signed_q = q && (suffix >> k) ? -int(q) : int(q);
				const int delta = signed_q * (1 << k) + int(suffix & ((1U << k) - 1));
				const int limit = 1 << (k + 3);
				int value = (vector[axis] + delta + limit) % (2 * limit);
				if (value < 0)
					value += 2 * limit;
				vector[axis] = value - limit;
			}
			block.processing = process[pi++];
			block.dx = vector[0];
			block.dy = vector[1];
		}
	}
	c.finish();

	bit_reader d(payload.slice(region[3], region[4]));
	const auto mode_table = alphabet(payload, 12, 8);
	const auto runs = alphabet(payload, 20, 4);
	const auto values = alphabet(payload, 8, 4);
	const std::uint32_t k = payload.read_uint(3, 1);
	require(k == 0 || k == 2, "unsupported inter DC precision");
	const auto field = [&fields, &d, &values, &mode_table, &runs, k] (std::uint32_t plane, std::uint32_t y, bool is_dc)
	{
		const std::uint32_t row = plane == 0 ? y / 2 : y;
		const std::uint32_t count = plane == 0 ? 2 : 1;
		std::uint32_t run = 0;
		int previous = 127;
		for (std::uint32_t x = 0; x < 80; ++x)
		{
			const auto &block = fields.controls[row * 80 + x];
			const bool eligible = is_dc ? !block.reference : (!block.reference || !block.processing);
			for (std::uint32_t i = 0; i < count; ++i)
			{
				if (!eligible)
				{
					if (is_dc)
					{
						fields.means[plane].push_back(-1);
						previous = 127;
					}
					else
						fields.modes[plane].push_back(0);
					continue;
				}
				if (is_dc)
				{
					const std::uint32_t q = d.symbol(values);
					previous += dc_delta(q, d.read(k + bool(q)), k);
					require(previous >= 0 && previous <= 255, "inter DC outside byte range");
					fields.means[plane].push_back(std::int16_t(previous));
				}
				else
				{
					std::uint32_t mode = 0;
					if (run)
						--run;
					else
					{
						mode = d.symbol(mode_table);
						require(mode <= 7, "invalid inter block mode");
						if (!mode)
							run = run_value(d, runs, 4);
					}
					fields.modes[plane].push_back(std::uint8_t(mode));
				}
			}
		}
		require(!run, "inter mode run exceeds eligible row");
	};
	for (std::uint32_t y = 0; y < 60; ++y)
	{
		field(1, y, false);
		field(2, y, false);
		field(0, y * 2, false);
		field(1, y, true);
		field(2, y, true);
		field(0, y * 2, true);
		field(0, y * 2 + 1, false);
		field(0, y * 2 + 1, true);
	}
	d.finish();
}

struct basis
{
	std::uint16_t descriptor = 0;
	std::uint32_t quotient = 0;
};

struct block_detail
{
	bool present = false;
	bool raw = false;
	std::uint32_t count = 0;
	block_pixels pixels{};
	std::array<basis, 5> bases;
	std::array<int, 2> corrections{};
};
using picture_details = std::array<std::vector<block_detail>, 3>;

enum class basis_source
{
	INTRA_DC_NIBBLES,
	INTER_REFERENCE_BYTES
};

picture_details decode_details(byte_view payload, bool intra, const picture_fields &fields)
{
	const auto region = offsets(payload, intra);
	std::size_t position = region[0];
	bit_reader luma(payload.slice(region[1], region[2]));
	const auto coefficients = alphabet(payload, 24, 8);
	picture_details details;
	for (std::uint32_t p = 0; p < 3; ++p)
		details[p].resize(p == 0 ? 160 * 120 : 80 * 60);
	const auto block = [&fields, &details, &coefficients, intra] (bit_reader &bits, std::uint32_t plane, std::uint32_t x, std::uint32_t y)
	{
		const std::uint32_t index = y * (plane == 0 ? 160 : 80) + x;
		const std::uint32_t mode = fields.modes[plane][index];
		if (mode == 0 || mode == 7)
			return;
		auto &item = details[plane][index];
		item.present = true;
		if (mode == 6)
		{
			item.raw = true;
			for (auto &sample : item.pixels)
				sample = std::uint8_t(bits.read(8));
			return;
		}
		require(mode <= 5, "invalid detail mode");
		const std::uint32_t mb = plane == 0 ? (y / 2) * 80 + x / 2 : index;
		const bool residual = !intra && fields.controls[mb].reference != 0;
		item.count = mode - std::uint32_t(residual);
		for (std::uint32_t i = 0; i < item.count; ++i)
		{
			item.bases[i].descriptor = std::uint16_t(bits.read(16));
			item.bases[i].quotient = bits.symbol(coefficients);
		}
		if (residual)
			for (int &value : item.corrections)
			{
				// Corrections use a direct unary quotient, not the header's run
				// ranking at 0x14, which applies to metadata zero runs.
				const std::uint32_t q = bits.unary();
				value = balanced(q, bits.read(2 + bool(q)), 2);
			}
	};
	for (std::uint32_t row = 0; row < 60; ++row)
	{
		bool active = false;
		for (std::uint32_t plane = 1; plane < 3; ++plane)
			for (std::uint32_t x = 0; x < 80; ++x)
			{
				const std::uint32_t mode = fields.modes[plane][row * 80 + x];
				active |= mode != 0 && mode != 7;
			}
		byte_view record{ payload.data, 0 };
		std::size_t length = 0;
		if (active)
		{
			require(position <= region[1] && region[1] - position >= 4, "missing chroma record");
			const std::uint32_t length_field = payload.read_uint(position, 4);
			const std::uint64_t padded_bytes = (std::uint64_t(length_field) / 32 + 1) * 4;
			require(padded_bytes <= region[1] - position - 4, "chroma record exceeds region A");
			length = std::size_t(length_field) + 1;
			const std::size_t bytes = std::size_t(padded_bytes);
			record = payload.slice(position + 4, position + 4 + bytes);
			position += 4 + bytes;
		}
		bit_reader chroma(record, length);
		for (std::uint32_t x = 0; x < 80; ++x)
		{
			block(chroma, 1, x, row);
			block(chroma, 2, x, row);
		}
		require(chroma.position == chroma.limit, "unconsumed chroma detail bits");
		bit_reader padding(record);
		padding.position = length;
		padding.finish();
		for (std::uint32_t y = row * 2; y < row * 2 + 2; ++y)
			for (std::uint32_t x = 0; x < 160; ++x)
				block(luma, 0, x, y);
	}
	require(position == region[1], "unconsumed chroma records");
	luma.finish();
	return details;
}

block_pixels spatial(int center, const std::array<int, 4> &nearby)
{
	constexpr std::array<int, 4> NEAR_WEIGHTS{ 2, 0, -1, -1 };
	constexpr std::array<int, 4> FAR_WEIGHTS{ -1, -1, 0, 2 };
	block_pixels result;
	for (std::uint32_t y = 0; y < 4; ++y)
		for (std::uint32_t x = 0; x < 4; ++x)
			result[y * 4 + x] = clip(floor_div(8 * center + NEAR_WEIGHTS[y] * (nearby[0] - center)
					+ FAR_WEIGHTS[y] * (nearby[1] - center) + NEAR_WEIGHTS[x] * (nearby[2] - center)
					+ FAR_WEIGHTS[x] * (nearby[3] - center) + 4, 8));
	return result;
}

template <typename Provider>
basis_sums basis_sum(const block_detail &detail, Provider samples, basis_source source)
{
	basis_sums result{};
	std::int64_t upper = 0;
	for (std::uint32_t i = 0; i < detail.count; ++i)
	{
		const auto &word = detail.bases[i];
		const block_pixels basis = samples(word.descriptor);
		const auto limits = std::minmax_element(basis.begin(), basis.end());
		const int span = *limits.second - *limits.first;
		upper += word.quotient * 4;
		std::int64_t coefficient = upper + ((word.descriptor >> 13) & 3);
		if (word.descriptor & 0x8000)
			coefficient = -coefficient;
		// Intra selects four-bit DC samples and uses a sixteen-aligned
		// reciprocal. Inter selects full-byte reference pixels and retains
		// all twelve reciprocal bits.
		const int reciprocal = !span ? 0 : source == basis_source::INTRA_DC_NIBBLES
				? (256 / span) * 16 : 4096 / span;
		const std::int64_t factor = coefficient * reciprocal;
		for (std::uint32_t j = 0; j < 16; ++j)
			result[j] += factor * basis[j];
	}
	return result;
}

block_pixels nest_basis(const basis_nest &nest, std::uint32_t descriptor)
{
	const std::uint32_t x = descriptor & 63;
	const std::uint32_t y = (descriptor >> 6) & 31;
	const std::uint32_t sx = 1U << ((descriptor >> 11) & 1);
	const std::uint32_t sy = 1U << ((descriptor >> 12) & 1);
	block_pixels result;
	for (std::uint32_t yy = 0; yy < 4; ++yy)
		for (std::uint32_t xx = 0; xx < 4; ++xx)
			result[yy * 4 + xx] = nest[(y + yy * sy) * 70 + x + xx * sx];
	return result;
}

block_pixels intra_block(std::uint32_t plane, std::uint32_t x, std::uint32_t y, const picture_fields &fields,
		const picture_details &details, const basis_nest &nest, std::uint32_t precision)
{
	const int columns = plane == 0 ? 160 : 80;
	const int rows = plane == 0 ? 120 : 60;
	const std::uint32_t index = y * columns + x;
	const auto &item = details[plane][index];
	const std::uint32_t mode = fields.modes[plane][index];
	const int center = fields.means[plane][index];
	require(center >= 0 && center <= 255, "missing intra DC");
	if (mode == 6)
		return item.pixels;
	if (mode == 7)
	{
		block_pixels result;
		result.fill(std::uint8_t(center));
		return result;
	}
	if (!mode)
	{
		std::array<int, 4> neighbours{};
		constexpr std::array<int, 4> DX{ 0, 0, -1, 1 };
		constexpr std::array<int, 4> DY{ -1, 1, 0, 0 };
		for (std::uint32_t i = 0; i < 4; ++i)
		{
			const int nx = int(x) + DX[i];
			const int ny = int(y) + DY[i];
			neighbours[i] = center;
			if (nx >= 0 && ny >= 0 && nx < columns && ny < rows)
			{
				const std::uint32_t ni = ny * columns + nx;
				const std::uint32_t nm = fields.modes[plane][ni];
				if (fields.means[plane][ni] >= 0 && (nm == 0 || nm == 7))
					neighbours[i] = fields.means[plane][ni];
			}
		}
		return spatial(center, neighbours);
	}
	require(item.present, "missing intra basis data");
	const auto values = basis_sum(item, [&nest] (std::uint32_t descriptor) { return nest_basis(nest, descriptor); },
			basis_source::INTRA_DC_NIBBLES);
	const std::int64_t scale = std::int64_t(1) << precision;
	const std::int64_t offset = center * scale - floor_div(std::accumulate(values.begin(), values.end(), std::int64_t(0)), 16);
	block_pixels result;
	for (std::uint32_t i = 0; i < 16; ++i)
		result[i] = clip(floor_div(values[i] + offset, scale));
	return result;
}

void store(frame &picture, std::uint32_t plane, std::uint32_t x, std::uint32_t y, std::uint32_t side, const std::uint8_t *pixels)
{
	const std::uint32_t stride = plane == 0 ? WIDTH : WIDTH / 2;
	const std::uint32_t height = plane == 0 ? HEIGHT : HEIGHT / 2;
	require(x <= stride && side <= stride - x && y <= height && side <= height - y, "destination block outside frame");
	auto &output = picture.plane(plane);
	for (std::uint32_t row = 0; row < side; ++row)
		std::copy_n(pixels + row * side, side, output.begin() + (y + row) * stride + x);
}

std::array<std::uint8_t, 64> prediction(const frame &reference, std::uint32_t plane,
		int x_half, int y_half, std::uint32_t side)
{
	const int stride = plane == 0 ? 640 : 320;
	const int height = plane == 0 ? 480 : 240;
	const int x = int(floor_div(x_half, 2));
	const int y = int(floor_div(y_half, 2));
	const std::uint32_t fx = std::uint32_t(x_half - x * 2);
	const std::uint32_t fy = std::uint32_t(y_half - y * 2);
	require(x >= 0 && y >= 0 && x + int(side + fx) <= stride && y + int(side + fy) <= height,
			"motion samples outside reference frame");
	std::array<std::uint8_t, 64> result{};
	const auto &source = reference.plane(plane);
	for (std::uint32_t yy = 0; yy < side; ++yy)
		for (std::uint32_t xx = 0; xx < side; ++xx)
		{
			const std::uint32_t p = (y + yy) * stride + x + xx;
			std::uint32_t value = source[p];
			if (fx && fy)
				value = (value + source[p + 1] + source[p + stride] + source[p + stride + 1] + 2) / 4;
			else if (fx)
				value = (value + source[p + 1] + 1) / 2;
			else if (fy)
				value = (value + source[p + stride] + 1) / 2;
			result[yy * side + xx] = std::uint8_t(value);
		}
	return result;
}

block_pixels reference_basis(const frame &reference, int ox, int oy, std::uint32_t descriptor)
{
	const int x = ox + int(descriptor & 63);
	const int y = oy + int((descriptor >> 6) & 31);
	const int sx = 1 << ((descriptor >> 11) & 1);
	const int sy = 1 << ((descriptor >> 12) & 1);
	require(x >= 0 && y >= 0 && x + 3 * sx < 640 && y + 3 * sy < 480, "reference basis outside frame");
	block_pixels result;
	for (std::uint32_t yy = 0; yy < 4; ++yy)
		for (std::uint32_t xx = 0; xx < 4; ++xx)
			result[yy * 4 + xx] = reference.y[(y + yy * sy) * 640 + x + xx * sx];
	return result;
}

block_pixels residual(const block_pixels &reference, const basis_sums &values, const std::array<int, 2> &corrections, std::uint32_t precision)
{
	const int mean = (std::accumulate(reference.begin(), reference.end(), 0) + 8) / 16;
	const auto limits = std::minmax_element(reference.begin(), reference.end());
	const int span = *limits.second - *limits.first;
	const std::int64_t factor = std::int64_t(corrections[1]) * (span ? 4096 / span : 0);
	const std::int64_t scale = std::int64_t(1) << precision;
	const std::int64_t offset = corrections[0] * scale - floor_div(std::accumulate(values.begin(), values.end(), std::int64_t(0)), 16);
	block_pixels result;
	for (std::uint32_t i = 0; i < 16; ++i)
		result[i] = clip(reference[i] + floor_div(values[i] + offset + (reference[i] - mean) * factor, scale));
	return result;
}

frame reconstruct(byte_view payload, bool intra, std::uint32_t kind, const frame *past, const frame *future,
		basis_nest &nest)
{
	picture_fields fields;
	std::uint32_t precision;
	if (intra)
	{
		precision = payload.read_uint(4, 1);
		require((precision == 9 || precision == 10) && payload.read_uint(5, 2) == 0x0200, "unsupported intra detail precision");
		decode_intra_fields(payload, fields, nest);
	}
	else
	{
		precision = payload.read_uint(0, 1);
		require(precision == 9 && payload.read_uint(2, 1) == 0, "unsupported inter detail precision");
		decode_inter_fields(payload, fields);
	}
	for (std::uint32_t plane = 0; plane < 3; ++plane)
	{
		const std::uint32_t expected = plane == 0 ? 160 * 120 : 80 * 60;
		require(fields.modes[plane].size() == expected && fields.means[plane].size() == expected, "invalid metadata geometry");
	}
	const auto details = decode_details(payload, intra, fields);
	frame output;
	for (std::uint32_t my = 0; my < 60; ++my)
		for (std::uint32_t mx = 0; mx < 80; ++mx)
		{
			const auto &control = fields.controls[my * 80 + mx];
			if (intra || !control.reference)
			{
				for (std::uint32_t plane = 0; plane < 3; ++plane)
					for (std::uint32_t by = 0; by < (plane == 0 ? 2U : 1U); ++by)
						for (std::uint32_t bx = 0; bx < (plane == 0 ? 2U : 1U); ++bx)
						{
							const std::uint32_t x = plane == 0 ? mx * 2 + bx : mx;
							const std::uint32_t y = plane == 0 ? my * 2 + by : my;
							const auto pixels = intra_block(plane, x, y, fields, details, nest, precision);
							store(output, plane, x * 4, y * 4, 4, pixels.data());
						}
				continue;
			}
			const frame *reference = control.reference == 1 ? past : kind == 1 ? &output : future;
			require(reference, "missing selected reference frame");
			const int sx = int(mx * 16) + control.dx;
			const int sy = int(my * 16) + control.dy;
			std::array<std::array<std::uint8_t, 64>, 3> predicted;
			predicted[0] = prediction(*reference, 0, sx, sy, 8);
			predicted[1] = prediction(*reference, 1, int(floor_div(sx, 2)), int(floor_div(sy, 2)), 4);
			predicted[2] = prediction(*reference, 2, int(floor_div(sx, 2)), int(floor_div(sy, 2)), 4);
			const int ox = int(floor_div(sx, 2)) - 32;
			const int oy = int(floor_div(sy, 2)) - 16;
			for (std::uint32_t plane = 0; plane < 3; ++plane)
			{
				const std::uint32_t side = plane == 0 ? 8 : 4;
				if (control.processing)
				{
					store(output, plane, mx * side, my * side, side, predicted[plane].data());
					continue;
				}
				for (std::uint32_t by = 0; by < side / 4; ++by)
					for (std::uint32_t bx = 0; bx < side / 4; ++bx)
					{
						const std::uint32_t x = plane == 0 ? mx * 2 + bx : mx;
						const std::uint32_t y = plane == 0 ? my * 2 + by : my;
						const auto &item = details[plane][y * (plane == 0 ? 160 : 80) + x];
						block_pixels pixels;
						for (std::uint32_t yy = 0; yy < 4; ++yy)
							for (std::uint32_t xx = 0; xx < 4; ++xx)
								pixels[yy * 4 + xx] = predicted[plane][(by * 4 + yy) * side + bx * 4 + xx];
						if (item.raw)
							pixels = item.pixels;
						else if (item.present)
						{
							const auto values = basis_sum(item, [reference, ox, oy] (std::uint32_t descriptor) { return reference_basis(*reference, ox, oy, descriptor); },
									basis_source::INTER_REFERENCE_BYTES);
							pixels = residual(pixels, values, item.corrections, precision);
						}
						store(output, plane, x * 4, y * 4, 4, pixels.data());
					}
			}
		}
	return output;
}

} // anonymous namespace

frame::frame() :
	y(WIDTH * HEIGHT),
	cr(WIDTH * HEIGHT / 4),
	cb(WIDTH * HEIGHT / 4)
{
}

std::vector<std::uint8_t> &frame::plane(std::uint32_t index)
{
	require(index < 3, "invalid plane index");
	return index == 0 ? y : index == 1 ? cr : cb;
}

const std::vector<std::uint8_t> &frame::plane(std::uint32_t index) const
{
	require(index < 3, "invalid plane index");
	return index == 0 ? y : index == 1 ? cr : cb;
}

void decoder::reset()
{
	m_past.reset();
	m_future.reset();
	m_current.reset();
	m_nest.fill(0);
	m_have_nest = false;
}

const frame &decoder::decode_packet(std::uint32_t kind, const std::uint8_t *payload, std::size_t size)
{
	require(kind <= 2, "unsupported picture kind");
	require(payload && size >= (kind == 0 ? 0x34U : 0x60U) && size <= 64 * 1024 * 1024,
			"invalid picture payload extent");
	if (kind != 0)
		require(m_future && m_have_nest && (kind != 2 || m_past), "missing reference picture or intra nest");
	basis_nest nest = m_nest;
	auto picture = std::make_shared<frame>(reconstruct({ payload, size }, kind == 0, kind,
			kind == 1 ? m_future.get() : m_past.get(), m_future.get(), nest));
	if (kind != 2)
	{
		m_past = m_future;
		m_future = picture;
	}
	if (kind == 0)
	{
		m_nest = nest;
		m_have_nest = true;
	}
	m_current = std::move(picture);
	return *m_current;
}

} // namespace axell_rm1
