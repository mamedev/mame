// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
// MaliGP2 polygon list builder.
// Command encodings: original Mali-200 userspace library and limadriver/lima.
// TODO: semaphore interleaving, lines/points, near-plane clipping of crossing triangles.

void mali200_device::reset_plb()
{
	std::fill(std::begin(m_plb_config), std::end(m_plb_config), 0);
	m_plb_config[15] = 0x3f800000;
	for (auto &bin : m_plb_bins)
		bin[0] = bin[1] = 0;
	m_plb_array = m_plb_stride = m_plb_state = m_plb_vertex = 0;
	m_plb_scissor[0] = 0xffff8000;
	m_plb_scissor[1] = 0x7fffe000;
	m_plb_scissor_set = false;
	m_plb_append_done = 0;
}

bool mali200_device::plb_commands()
{
	auto &config = m_plb_config;
	auto &bins = m_plb_bins;
	u32 &array = m_plb_array, &stride = m_plb_stride;
	u32 &state_base = m_plb_state, &vertex_base = m_plb_vertex;
	u32 &scissor_lo = m_plb_scissor[0], &scissor_hi = m_plb_scissor[1];
	bool &scissor_set = m_plb_scissor_set;
	u32 &heap_end = m_gp[5], &heap = m_gp[4];
	u64 append_index = 0;
	auto append = [&](unsigned index, u32 lo, u32 hi) -> bool
	{
		if (index >= std::size(bins))
		{
			logerror("PLB bin index overflow %u\n", index);
			return false;
		}
		// A command can broadcast to many bins and stall halfway through it.
		// On allocation resume, skip its already committed packets; do not
		// duplicate state/triangles or overwrite previous heap blocks.
		if (append_index++ < m_plb_append_done)
			return true;
		auto &b = bins[index];
		unsigned const size = 128U << ((config[11] >> 8) & 3);
		if (!b[0])
		{
			if (!read_word(array + index * 4, b[0], 7))
				return false;
			if (!b[0] || (b[0] & (size - 1)))
			{
				logerror("PLB unaligned initial block %08x size=%u\n", b[0], size);
				return false;
			}
			b[1] = b[0] + size;
		}
		if (b[0] + 16 > b[1])
		{
			heap = (heap + size - 1) & ~(size - 1);
			if (heap > heap_end || heap_end - heap < size)
			{
				m_gp[0x24 / 4] |= 4; // PLB_OUT_OF_MEM
				m_gp[0x68 / 4] |= 0x20; // PLB_STALLED, retain PLB_ACTIVE
				LOGMASKED(LOG_JOBS, "PLB heap exhausted at %08x\n", heap);
				return false;
			}
			if (!write_word(b[0], (heap >> 3) & 0x1ffffffc, 9) || !write_word(b[0] + 4, 0xb0000000, 9))
				return false;
			b[0] = heap;
			b[1] = heap + size;
			heap += size;
			m_gp[4] = heap;
		}
		if (!write_word(b[0], lo, 9) || !write_word(b[0] + 4, hi, 9))
			return false;
		b[0] += 8;
		++m_plb_append_done;
		return true;
	};
	auto bounds = [&](unsigned &left, unsigned &right, unsigned &top, unsigned &bottom) -> bool
	{
		unsigned const xs = config[12] & 63, ys = (config[12] >> 16) & 63;
		if (xs > 8 || ys > 8 || !array || !stride)
			return false;
		left = ((config[9] >> 16) & 255) >> xs;
		right = (config[9] >> 24) >> xs;
		top = (config[9] & 255) >> ys;
		bottom = ((config[9] >> 8) & 255) >> ys;
		return left <= right && top <= bottom && right < stride && bottom * stride + right < std::size(bins);
	};
	auto broadcast = [&](u32 lo, u32 hi) -> bool
	{
		unsigned l, r, t, b;
		if (!bounds(l, r, t, b))
			return false;
		for (unsigned y = t; y <= b; ++y)
			for (unsigned x = l; x <= r; ++x)
				if (!append(y * stride + x, lo, hi))
					return false;
		return true;
	};
	auto triangle = [&](u32 ia, u32 ib, u32 ic, bool rectangle) -> bool
	{
		u32 const indices[3] = { ia, ib, ic };
		float v[3][4];
		for (unsigned i = 0; i < 3; ++i)
		{
			if (indices[i] > 0x1ffff)
			{
				logerror("PLB vertex index exceeds packet range\n");
				return false;
			}
			for (unsigned c = 0; c < 4; ++c)
			{
				u32 word;
				if (!read_word(vertex_base + indices[i] * 16 + c * 4, word, 7))
					return false;
				v[i][c] = mali_float(word);
				if (!std::isfinite(v[i][c]))
					return true;
			}
		}
		float const area = (v[1][0] - v[0][0]) * (v[2][1] - v[0][1]) - (v[1][1] - v[0][1]) * (v[2][0] - v[0][0]);
		if (!std::isfinite(area) || area == 0 || (!rectangle && ((area > 0 && BIT(config[11], 17)) || (area < 0 && BIT(config[11], 18)))))
			return true;
		if ((v[0][2] < mali_float(config[14]) && v[1][2] < mali_float(config[14]) && v[2][2] < mali_float(config[14])) ||
				(v[0][2] > mali_float(config[15]) && v[1][2] > mali_float(config[15]) && v[2][2] > mali_float(config[15])))
			return true;
		float xmin = std::max(std::min({ v[0][0], v[1][0], v[2][0] }), mali_float(config[7]));
		float xmax = std::min(std::max({ v[0][0], v[1][0], v[2][0] }), mali_float(config[8]));
		float ymin = std::max(std::min({ v[0][1], v[1][1], v[2][1] }), mali_float(config[5]));
		float ymax = std::min(std::max({ v[0][1], v[1][1], v[2][1] }), mali_float(config[6]));
		if (scissor_set)
		{
			xmin = std::max(xmin, float((scissor_lo >> 30) | ((scissor_hi & 0x1fff) << 2)));
			xmax = std::min(xmax, float(((scissor_hi >> 13) & 0x7fff) + 1));
			ymin = std::max(ymin, float(scissor_lo & 0x7fff));
			ymax = std::min(ymax, float(((scissor_lo >> 15) & 0x7fff) + 1));
		}
		if (!(xmin < xmax && ymin < ymax))
			return true;
		unsigned l, r, t, b;
		if (!bounds(l, r, t, b))
			return false;
		float const bw = float(16U << (config[12] & 63)), bh = float(16U << ((config[12] >> 16) & 63));
		int const x0 = std::max(int(l), int(std::floor(std::clamp(xmin, -65536.0f, 65536.0f) / bw)));
		int const x1 = std::min(int(r), int(std::floor(std::clamp(xmax, -65536.0f, 65536.0f) / bw)));
		int const y0 = std::max(int(t), int(std::floor(std::clamp(ymin, -65536.0f, 65536.0f) / bh)));
		int const y1 = std::min(int(b), int(std::floor(std::clamp(ymax, -65536.0f, 65536.0f) / bh)));
		// The three-vertex pixel rectangle is confirmed by the original
		// library's direct PP submissions. The ordinary triangle type still
		// needs comparison against a hardware-generated polygon-list dump.
		u64 const packet =
				u64(ia) | (u64(ib) << 17) | (u64(ic) << 34) | (u64(config[11] & 255) << 51) | (u64(rectangle ? 0x20000000 : 0) << 32);
		for (int y = y0; y <= y1; ++y)
			for (int x = x0; x <= x1; ++x)
				if (!append(y * stride + x, u32(packet), u32(packet >> 32)))
					return false;
		return true;
	};
	u32 position = m_gp[0x84 / 4];
	for (unsigned commands = 0; position != m_gp[3] && commands < 1048576; ++commands)
	{
		u32 lo, hi;
		if (!read_word(position, lo, 7) || !read_word(position + 4, hi, 7))
			return false;
		if (m_gp_jobs <= 4)
			LOGMASKED(LOG_LISTS, "PLB command %08x: %08x %08x\n", position, lo, hi);
		position += 8;
		append_index = 0;
		switch (hi >> 28)
		{
		case 0:
		{
			unsigned const count = ((hi & 0xffff) << 8) | (lo >> 24), start = lo & 0xffffff, mode = (hi >> 16) & 31;
			bool const indexed = BIT(hi, 21);
			bool const rectangle = mode == 15; // _mali200_draw_quad, three corners
			if ((!rectangle && (mode < 4 || mode > 6)) || ((config[11] >> 15) & 3))
			{
				logerror("unsupported PLB primitive mode %u\n", mode);
				return false;
			}
			std::vector<u32> index(count);
			for (unsigned i = 0; i < count; ++i)
			{
				index[i] = start + i;
				if (indexed)
				{
					unsigned const format = (config[11] >> 10) & 3;
					if (format == 3)
						return false;
					unsigned const size = 1U << format;
					if (!read_bytes(config[1] + (start + i) * size, size, index[i], 7))
						return false;
					if (BIT(config[11], 19))
						index[i] = size == 4 ? swapendian_int32(index[i]) : size == 2 ? swapendian_int16(u16(index[i])) : index[i];
				}
				index[i] += config[10];
			}
			for (unsigned i = 2; i < count; i += (mode == 4 || rectangle) ? 3 : 1)
			{
				u32 a = mode == 6 ? index[0] : index[i - 2], b = index[i - 1];
				if (mode == 5 && (i & 1))
					std::swap(a, b);
				if (!triangle(a, b, index[i], rectangle))
					return false;
			}
			break;
		}
		case 1:
			if ((hi & 0xfffffff) < 0x100 || (hi & 0xfffffff) > 0x10f)
				return false;
			config[hi & 15] = lo;
			if ((hi & 15) == 0)
				vertex_base = lo;
			if ((hi & 15) == 3)
				heap = m_gp[4] = lo & ~127U;
			if ((hi & 15) == 4)
				heap_end = m_gp[5] = lo & ~127U;
			break;
		case 2:
			array = lo;
			break;
		case 3:
			stride = lo & 0xff;
			break;
		case 5:
			if (!broadcast(0, 0xb0000000))
				return false;
			break;
		case 6:
			break; // command lists currently execute VS before PLB
		case 7:
			scissor_lo = lo;
			scissor_hi = hi;
			scissor_set = true;
			// The packed 15-bit scissor rectangle is passed through to the
			// PP. This encoding remains to be compared with hardware.
			if (!broadcast(lo, hi))
				return false;
			break;
		case 8:
			state_base = lo;
			vertex_base = (hi & 0x0fffffff) << 4;
			// Explicitly include new base addresses among the commands
			// broadcast to every bin in the configured screen box.
			if (!broadcast(state_base >> 3, 0x80000000 | (vertex_base >> 6)))
				return false;
			break;
		case 13:
			break; // cache/write barrier, DMA writes are synchronous
		case 15:
			position = lo;
			break;
		default:
			logerror("unsupported PLB command %08x %08x\n", lo, hi);
			return false;
		}
		m_gp[0x84 / 4] = position;
		m_plb_append_done = 0;
	}
	return position == m_gp[3];
}
