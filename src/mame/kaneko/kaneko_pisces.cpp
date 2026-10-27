// license:BSD-3-Clause
// copyright-holders:Luca Elia, Andrea Bogazzi
/***************************************************************************

    Kaneko PISCES MCU high-level simulation for Gals Panic II and II' SE.

    The NEC uPD78324 internal ROM is undumped. This device substitutes the
    observed command protocol between the two 68000s. Image decompression
    still runs on the sub CPU; the coverage buffer is HLE bookkeeping,
    not a model of the MCU's internal RAM.

    Main RAM 100010.w: software watchdog?
    Main RAM 100020.b: command queue length
    Sub RAM  101012.b: command queue length

***************************************************************************/

#include "emu.h"
#include "kaneko_pisces.h"

#include "endianness.h"

DEFINE_DEVICE_TYPE(KANEKO_PISCES_HLE, kaneko_pisces_hle_device, "kaneko_pisces_hle", "Kaneko PISCES MCU (HLE)")

kaneko_pisces_hle_device::kaneko_pisces_hle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, KANEKO_PISCES_HLE, tag, owner, clock)
	, m_maincpu(*this, finder_base::DUMMY_TAG)
	, m_subcpu(*this, finder_base::DUMMY_TAG)
	, m_subdata(*this, finder_base::DUMMY_TAG)
	, m_bg15(*this, finder_base::DUMMY_TAG)
	, m_decoder_task_cb(*this, 0)
	, m_old_mcu_nmi1(0)
	, m_old_mcu_nmi2(0)
	, m_bg_decode_param_offset(2)
	, m_field_origin_x(48)
	, m_girl_bitmap_pending_addr(0)
	, m_girl_bitmap_pending_param(0)
	, m_girl_bitmap_pending_task(0)
{
}

void kaneko_pisces_hle_device::device_start()
{
	// The coverage tap needs the sub CPU's program address space.
	if (!m_maincpu->started() || !m_subcpu->started())
		throw device_missing_dependencies();

	scan_background_images();
	// The fixed BG15 mapping retains this tap across save/load. The address
	// space owns the handler; only the pending job and coverage are saved.
	m_subcpu->space(AS_PROGRAM).install_write_tap(
		0x400000, 0x4fffff, "bg15_coverage",
		[this](offs_t address, uint16_t &data, uint16_t mem_mask)
		{
			track_bg15_coverage(address, data, mem_mask);
		});

	m_bg15_coverage = util::make_unique_clear<uint8_t []>(BG15_COVERAGE_SIZE);

	save_item(NAME(m_old_mcu_nmi1));
	save_item(NAME(m_old_mcu_nmi2));
	save_item(NAME(m_girl_bitmap_pending_addr));
	save_item(NAME(m_girl_bitmap_pending_param));
	save_item(NAME(m_girl_bitmap_pending_task));
	save_pointer(NAME(m_bg15_coverage), BG15_COVERAGE_SIZE);
}

void kaneko_pisces_hle_device::device_reset()
{
	m_girl_bitmap_pending_addr = 0;
	m_girl_bitmap_pending_param = 0;
	m_girl_bitmap_pending_task = 0;
	std::fill_n(&m_bg15_coverage[0], BG15_COVERAGE_SIZE, 0);
}

// Scan the background ROM data to build the image offset table at init.
// Images are stored back-to-back as RLE-compressed streams with type
// headers (0x0020 = opaque with dimensions, 0x0030/0x0050 = transparent
// 256x256). This HLE lookup table is derived from each set's ROMs;
// the original MCU firmware and its lookup mechanism are not known.
void kaneko_pisces_hle_device::scan_background_images()
{
	memory_region &rom = *m_subdata;
	uint32_t const rom_size = rom.bytes();

	auto const byte = util::big_endian_cast<uint8_t const>(&rom.as_u16());
	auto word = [&byte](uint32_t offset) -> uint16_t
	{
		return (uint16_t(byte[offset]) << 8) | byte[offset + 1];
	};

	// Parse one image stream starting at pos. Returns the byte offset
	// just past the last data byte (before word-alignment), or 0 on failure.
	auto parse_image = [&byte, &word, rom_size](uint32_t pos) -> uint32_t
	{
		if (pos + 6 > rom_size)
			return 0;
		uint16_t const kind = word(pos);
		uint32_t p = pos + 2;
		uint32_t total_pixels;

		if (kind == 0x0020)
		{
			uint16_t const h = word(p);
			uint16_t const w = word(p + 2);
			if (!h || h > 0xff || !w || w > 0x1ff)
				return 0;
			p += 4;
			total_pixels = (h + 1) * (w + 1);
		}
		else if (kind == 0x0030 || kind == 0x0050)
			total_pixels = 256 * 256;
		else
			return 0;

		uint32_t decoded = 0;
		while (decoded < total_pixels && p < rom_size)
		{
			uint8_t const ctrl = byte[p++];
			uint32_t count;
			if (kind == 0x0050)
			{
				count = ctrl & 0x3f;
				if (ctrl & 0x40)
				{
					if (p >= rom_size) return 0;
					count = (count << 8) | byte[p++];
				}
			}
			else
				count = ctrl & 0x7f;
			count += 1;
			if (decoded + count > total_pixels)
				count = total_pixels - decoded;

			bool const skip = (kind != 0x0020) && (ctrl & 0x80);
			bool const repeat = (kind == 0x0020) && (ctrl & 0x80);
			uint32_t const data_bytes = skip ? 0 : (repeat ? 2 : count * 2);
			if (p + data_bytes > rom_size)
				return 0;
			// Strict: real image colors never have bit 15 set in the
			// compressed stream. The decoder adds it during rendering.
			for (uint32_t i = 0; i < data_bytes; i += 2)
				if (byte[p + i] & 0x80)
					return 0;
			p += data_bytes;
			decoded += count;
		}
		// Word-align
		if (p & 1) p++;
		return (decoded >= total_pixels) ? p : 0;
	};

	// Scan every word-aligned position for valid image headers.
	// Headers are 0x0020 (opaque with dimensions), 0x0030 or 0x0050
	// (transparent 256x256). Each candidate is parsed strictly: valid
	// dimensions, no bit-15 in source colors, complete stream.
	// Positions that parse successfully become table entries in ROM order.
	uint32_t consumed = 0;
	for (uint32_t pos = 0; pos + 2 <= rom_size; pos += 2)
	{
		if (pos < consumed)
			continue;
		uint16_t const kind = word(pos);
		if (kind != 0x0020 && kind != 0x0030 && kind != 0x0050)
			continue;
		if (kind == 0x0020)
		{
			if (pos + 6 > rom_size)
				continue;
			uint16_t const h = word(pos + 2);
			uint16_t const w = word(pos + 4);
			// Strict dimension check: ROM stores (pixels-1), only
			// sizes observed in these ROMs: 256x256, 320x240, 256x240
			if (!((w == 0xff && h == 0xff) || (w == 0x13f && h == 0xef) || (w == 0xff && h == 0xef)))
				continue;
		}
		uint32_t end = parse_image(pos);
		if (!end)
			continue;
		m_bg_image_offsets.push_back(pos);
		consumed = (end + 1) & ~1;
	}

	logerror("BG ROM scan: found %d images", int(m_bg_image_offsets.size()));
	for (size_t i = 0; i < std::min(m_bg_image_offsets.size(), size_t(5)); i++)
		logerror(" [%d]=0x%08x", int(i), m_bg_image_offsets[i]);
	logerror("\n");
}

void kaneko_pisces_hle_device::write_kaneko()
{
	address_space &dstspace = m_maincpu->space(AS_PROGRAM);
	static constexpr uint8_t test_pattern[] = { 0xff, 0x55, 0xaa, 0xdd, 0xbb, 0x99 };

	// Write "KANEKO" to 100000-100005 without clashing with the RAM test.
	for (offs_t address = 0x100000; address < 0x100007; address++)
	{
		uint8_t const value = dstspace.read_byte(address);
		for (uint8_t const pattern : test_pattern)
			if (value == pattern)
				return;
	}

	static constexpr char signature[] = "KANEKO";
	for (unsigned i = 0; i < std::size(signature) - 1; i++)
		dstspace.write_byte(0x100000 + i, signature[i]);
}

void kaneko_pisces_hle_device::init_w(uint8_t data)
{
	address_space &srcspace = m_maincpu->space(AS_PROGRAM);
	address_space &dstspace = m_subcpu->space(AS_PROGRAM);

	for (uint32_t mcu_address = 0x100010; mcu_address < (0x100010 + 6); mcu_address++)
	{
		uint8_t const mcu_data = srcspace.read_byte(mcu_address);
		dstspace.write_byte(mcu_address - 0x10, mcu_data);
	}
	m_subcpu->set_input_line(INPUT_LINE_IRQ7, HOLD_LINE); // MCU initialised
}

// The native decompressor supplies coverage as it writes pixels. Track it
// separately because an opaque background can precede a transparent overlay
// in the same framebuffer. This is HLE bookkeeping, not recovered MCU firmware.
void kaneko_pisces_hle_device::track_bg15_coverage(offs_t address, uint16_t data, uint16_t mem_mask)
{
	if (!m_girl_bitmap_pending_addr || !m_girl_bitmap_pending_param)
		return;
	address_space &sub = m_subcpu->space(AS_PROGRAM);
	uint32_t const task = m_decoder_task_cb();
	if (task < 0x100000 || task > 0x13fff0)
		return;
	uint32_t const param = sub.read_dword(task + 8);
	if (param != m_girl_bitmap_pending_param)
		return;
	uint32_t const dest = sub.read_dword(param + 4);
	if (dest < 0x400000 || dest >= 0x500000 || address < dest)
		return;
	unsigned const height = sub.read_word(param + 8) + 1;
	unsigned const width = sub.read_word(param + 10) + 1;
	if (width != 256 || height != 256)
		return;
	uint32_t const delta = address - dest;
	if (delta / 0x400 >= height || (delta % 0x400) / 2 >= width)
		return;

	uint32_t const pixel = (delta / 0x400) * 256 + (delta % 0x400) / 2;
	uint16_t const value = (m_bg15[(address - 0x400000) / 2] & ~mem_mask) | (data & mem_mask);
	uint8_t const bit = 0x80 >> (pixel & 7);
	if ((value & 0x7fff) > 1)
		m_bg15_coverage[pixel / 8] |= bit;
	else
		m_bg15_coverage[pixel / 8] &= ~bit;
	m_girl_bitmap_pending_task = task;
}

uint16_t kaneko_pisces_hle_device::generate_girl_bitmap(uint32_t dest_addr)
{
	// Derive BG15 plane from the sub CPU framebuffer destination.
	if (dest_addr < 0x400000 || dest_addr >= 0x500000)
		return 0;
	int const plane = (dest_addr - 0x400000) / 0x40000;

	uint16_t const *const regs = &m_bg15[0x80000 + plane * 0x20000];

	int const scrollx = regs[0x400 / 2] + 0x42;
	int const scrolly = regs[0xc00 / 2] + 0x0b;
	int const dest_col = (dest_addr / 2) & 0x1ff;
	int const dest_row = (dest_addr / 0x400) & 0xff;

	uint16_t popcount = 0;
	int byte_idx = 0;
	address_space &mspace = m_maincpu->space(AS_PROGRAM);

	// Pack the observed native coverage using the field's current pixel origin.
	//const int      FIELD_COLS         = 256;
	const int      FIELD_ROWS         = 224;

	for (int row = 0; row < FIELD_ROWS; row++)
	{
		for (int longword = 0; longword < 8; longword++)
		{
			uint32_t bits = 0;
			for (int bit = 31; bit >= 0; bit--)
			{
				int const bx = longword * 32 + (31 - bit);
				int const bg15_col = (m_field_origin_x + bx + scrollx - dest_col) & 0x1ff;
				int const bg15_row = (8 + row + scrolly - dest_row) & 0xff;
				uint32_t const pixel = bg15_row * 256 + bg15_col;
				if (bg15_col < 256 && (m_bg15_coverage[pixel / 8] & (0x80 >> (pixel & 7))))
				{
					bits |= (1u << bit);
					popcount++;
				}
			}
			mspace.write_dword_unaligned(GIRL_BITMAP_ADDR + byte_idx, bits);
			byte_idx += 4;
		}
	}
	logerror("MCU girl bitmap: plane %d dest %06x, %d pixels\n", plane, dest_addr, popcount);
	return popcount;
}

void kaneko_pisces_hle_device::vblank()
{
	if (!m_girl_bitmap_pending_addr || !m_girl_bitmap_pending_param || !m_girl_bitmap_pending_task)
		return;

	// The sub CPU writes the BG15 destination at param+4 after decode setup.
	address_space &sub = m_subcpu->space(AS_PROGRAM);
	// A picture script may remain active after its decompressor has finished.
	if ((sub.read_word(m_girl_bitmap_pending_task) & 0xc000) != 0xc000)
		return;
	uint32_t const dest = sub.read_dword(m_girl_bitmap_pending_param + 4);
	if (dest < 0x400000 || dest >= 0x500000)
		return;

	// Wait for the scroll register to flip to the decoded half.
	int const plane = (dest - 0x400000) / 0x40000;
	uint16_t const *const regs = &m_bg15[0x80000 + plane * 0x20000];
	uint32_t const scroll_col = (regs[0x400 / 2] + 0x42 + m_field_origin_x) & 0x1ff;
	uint32_t const dest_col = (dest / 2) & 0x1ff;
	if (scroll_col != dest_col)
		return;

	uint16_t const total = generate_girl_bitmap(dest);
	if (!total)
		return;
	address_space &main = m_maincpu->space(AS_PROGRAM);
	main.write_word(m_girl_bitmap_pending_addr + 2, GIRL_BITMAP_OFFSET);
	main.write_word(m_girl_bitmap_pending_addr, total);
	logerror("MCU girl bitmap: reply %u pixels\n", unsigned(total));
	m_girl_bitmap_pending_addr = 0;
	m_girl_bitmap_pending_param = 0;
	m_girl_bitmap_pending_task = 0;
}

void kaneko_pisces_hle_device::process_main_queue()
{
	address_space &mspace = m_maincpu->space(AS_PROGRAM);
	address_space &sspace = m_subcpu->space(AS_PROGRAM);

	uint8_t len = mspace.read_byte(0x100020);
	for (uint8_t slot = 0; slot < len; slot += 4)
	{
		uint8_t command = mspace.read_byte(0x100021 + slot);
		uint32_t address = 0x100000 | mspace.read_word(0x100022 + slot);

		switch (command)
		{
		case 0x02:
		{ // Copy N bytes from RAM2 to RAM1?, gp2se is the only one to use it, often!
			uint16_t src = mspace.read_word(address + 2);
			uint16_t dst = mspace.read_word(address + 6);
			uint16_t size = mspace.read_word(address + 8);
			logerror("MCU master %02x:%06x copy s:%04x, m:%04x, size:%04x\n", slot, address, src, dst, size);

			for (uint16_t i = 0; i != size; i++)
			{
				mspace.write_byte(0x100000 + dst, sspace.read_byte(0x100000 + src));
				src++;
				dst++;
			}
			break;
		}

		case 0x0a:
		{ // Copy N bytes from RAM1 to RAM2
			uint16_t src = mspace.read_word(address + 2);
			uint16_t dst = mspace.read_word(address + 6);
			uint16_t size = mspace.read_word(address + 8);
			logerror("MCU master %02x:%06x copy m:%04x, s:%04x, size:%04x\n", slot, address, src, dst, size);

			for (uint16_t i = 0; i != size; i++)
			{
				sspace.write_byte(0x100000 + dst, mspace.read_byte(0x100000 + src));
				src++;
				dst++;
			}
			break;
		}

		case 0x10:
			// image_index=0 (clear/init, no specific girl image).
			// No decode will follow, so acknowledge immediately.
			logerror("MCU master %02x:%06x: girl clear (cmd 10)\n", slot, address);
			m_girl_bitmap_pending_addr = 0;
			m_girl_bitmap_pending_param = 0;
			m_girl_bitmap_pending_task = 0;
			mspace.write_word(address, 0xffff);
			continue;

		default:
		{
			// Girl-area bitmap request. Defer the response until
			// the sub CPU finishes decoding the image to BG15.
			// vblank publishes its observed coverage on completion.
			logerror("MCU master %02x:%06x: girl bitmap cmd %02x pending\n",
				slot, address, command);
			mspace.write_word(address, 0);
			m_girl_bitmap_pending_addr = address;
			m_girl_bitmap_pending_param = 0;
			m_girl_bitmap_pending_task = 0;
			std::fill_n(&m_bg15_coverage[0], BG15_COVERAGE_SIZE, 0);
			continue;
		}

		}

		/* Raise a "job done" flag */
		if (command)
			mspace.write_word(address, 0xffff);
	}
	if (len)
		mspace.write_byte(0x100020, 0x00);
}

void kaneko_pisces_hle_device::process_sub_queue()
{
	// The HLE image table is built from this set's background ROMs.
	const uint32_t imlist_count = uint32_t(m_bg_image_offsets.size());

	address_space &sspace = m_subcpu->space(AS_PROGRAM);

	uint8_t len = sspace.read_byte(0x101012);
	for (uint8_t slot = 0; slot < len; slot += 4)
	{
		uint8_t command = sspace.read_byte(0x101013 + slot);
		uint32_t address = 0x100000 | sspace.read_word(0x101014 + slot);

		switch (command)
		{
		case 0x0c:
		{ // Resolve an image ID to a raw offset in the background ROMs.
			uint16_t const img = sspace.read_word(address);
			uint32_t const iadr = (img < imlist_count) ? m_bg_image_offsets[img] : 0;
			// The ROM script interpreter selects the bank and reads the header.
			// It then queues the appropriate native decompressor with this raw offset.
			logerror("MCU slave %02x:%06x: image lookup %04x -> %08x\n", slot, address, img, iadr);
			sspace.write_dword(address + 2, iadr);
			// Full-height pictures/overlays supply the field mask; the
			// 240-row scenery jobs must not bind a pending girl request.
			if (m_girl_bitmap_pending_addr && !m_girl_bitmap_pending_param && iadr)
			{
				memory_region &rom = *m_subdata;
				if (rom.as_u16(iadr / 2) != 0x20
					|| (rom.as_u16(iadr / 2 + 1) == 0xff && rom.as_u16(iadr / 2 + 2) == 0xff))
					m_girl_bitmap_pending_param = address + m_bg_decode_param_offset;
			}
			break;
		}

		default:
			logerror("MCU slave %02x:%06x: unhandled command %02x\n", slot, address, command);
			break;
		}

		/* Raise a "job done" flag */
		if (command)
			sspace.write_word(address, 0xffff);
	}
	if (len)
		sspace.write_byte(0x101012, 0x00);

	write_kaneko();
	//logerror("%s : MCU executes CHECKs synchro\n", machine().describe_context());
}

void kaneko_pisces_hle_device::main_nmi_w(uint8_t data) //driven by CPU1's int5 ISR
{
//for galpan2t:
//Triggered from 'maincpu' (00007D60),once, with no command, using alternate line, during init
//Triggered from 'maincpu' (000080BE),once, for unknown command, during init
//Triggered from 'maincpu' (0000741E),from here on...driven by int5, even if there's no command
	if ((data & 1) && !(m_old_mcu_nmi1 & 1))
		process_main_queue();
	// Bit 4 is also pulsed at startup; its function is unknown.
	m_old_mcu_nmi1 = data;
}

void kaneko_pisces_hle_device::sub_nmi_w(uint8_t data) //driven by CPU2's int5 ISR
{
	if ((data & 1) && !(m_old_mcu_nmi2 & 1))
		process_sub_queue();
	m_old_mcu_nmi2 = data;
}
