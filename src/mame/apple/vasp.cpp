// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    Apple "VASP Integrated Controller" system ASIC
    Emulation by R. Belmont

    VASP contains the following:
    - A memory controller for up to 68 MB (4 MB on the motherboard + one 64 MB SIMM)
    - A VRAM controller and framebuffer controller, supporting monitor ID selection
    - A full VIA (VIA1) and a "pseudo-VIA", which is basically a combination GPIO and
      interrupt controller that looks somewhat like a VIA with no timers and no shift register
    - An ASC-like 4-channel audio controller
    - Support logic for various external subsystems (ADB, FDC, NuBus, SCC, SCSI)
*/

#include "emu.h"
#include "vasp.h"

#include "formats/ap_dsk35.h"
#include "layout/generic.h"

static constexpr u32 C7M  = 7833600;
static constexpr u32 C15M = (C7M * 2);

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VASP, vasp_device, "vasp", "Apple VASP system ASIC")

static INPUT_PORTS_START( vasp )
	PORT_START("MONTYPE")
	PORT_CONFNAME(0x0f, 0x06, "Connected monitor")
	PORT_CONFSETTING( 0x01, "15\" Portrait Display (640x870)")
	PORT_CONFSETTING( 0x02, "12\" RGB (512x384)")
	PORT_CONFSETTING( 0x06, "13\" RGB (640x480)")
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor vasp_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( vasp );
}

//-------------------------------------------------
//  ADDRESS_MAP
//-------------------------------------------------

void vasp_device::map(address_map &map)
{
	map(0x00000000, 0x000fffff).r(FUNC(vasp_device::rom_switch_r)).mirror(0x0ff00000);

	map(0x10000000, 0x10001fff).rw(FUNC(vasp_device::mac_via_r), FUNC(vasp_device::mac_via_w)).mirror(0x00f00000);
	map(0x10014000, 0x10015fff).rw(m_asc, FUNC(asc_device::read), FUNC(asc_device::write)).mirror(0x00f00000);
	map(0x10024000, 0x10025fff).rw(FUNC(vasp_device::dac_r), FUNC(vasp_device::dac_w)).mirror(0x00f00000);
	map(0x10026000, 0x10027fff).rw(FUNC(vasp_device::pseudovia_r), FUNC(vasp_device::pseudovia_w)).mirror(0x00f00000);

	map(0x20000000, 0x200fffff).ram().mirror(0x0ff00000).rw(FUNC(vasp_device::vram_r), FUNC(vasp_device::vram_w));
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void vasp_device::device_add_mconfig(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(25175000, 800, 0, 640, 525, 0, 480);
	m_screen->set_size(1024, 768);
	m_screen->set_visarea(0, 640 - 1, 0, 480 - 1);
	m_screen->set_screen_update(FUNC(vasp_device::screen_update));
	m_screen->screen_vblank().set(FUNC(vasp_device::vbl_w));
	config.set_default_layout(layout_monitors);

	PALETTE(config, m_palette).set_entries(256);

	R65NC22(config, m_via1, C7M / 10);
	m_via1->readpa_handler().set(FUNC(vasp_device::via_in_a));
	m_via1->readpb_handler().set(FUNC(vasp_device::via_in_b));
	m_via1->writepa_handler().set(FUNC(vasp_device::via_out_a));
	m_via1->writepb_handler().set(FUNC(vasp_device::via_out_b));
	m_via1->cb2_handler().set(FUNC(vasp_device::via_out_cb2));
	m_via1->irq_handler().set(FUNC(vasp_device::via1_irq));

	ASC(config, m_asc, C15M, asc_device::asc_type::VASP);
	m_asc->add_route(0, tag(), 1.0);
	m_asc->add_route(1, tag(), 1.0);
	m_asc->irqf_callback().set(FUNC(vasp_device::asc_irq));
}

//-------------------------------------------------
//  vasp_device - constructor
//-------------------------------------------------

vasp_device::vasp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VASP, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	write_pb4(*this),
	write_pb5(*this),
	write_cb2(*this),
	write_hdsel(*this),
	read_pb3(*this, 0),
	m_maincpu(*this, finder_base::DUMMY_TAG),
	m_montype(*this, "MONTYPE"),
	m_screen(*this, "screen"),
	m_palette(*this, "palette"),
	m_via1(*this, "via1"),
	m_asc(*this, "asc"),
	m_rom(*this, finder_base::DUMMY_TAG),
	m_overlay(false)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vasp_device::device_start()
{
	m_vram = std::make_unique<u32[]>(0x100000 / sizeof(u32));

	m_stream = stream_alloc(8, 2, m_asc->clock(), STREAM_SYNCHRONOUS);

	m_6015_timer = timer_alloc(FUNC(vasp_device::mac_6015_tick), this);
	m_6015_timer->adjust(attotime::never);

	save_pointer(NAME(m_vram), 0x100000/sizeof(u32));
	save_item(NAME(m_via_interrupt));
	save_item(NAME(m_via2_interrupt));
	save_item(NAME(m_scc_interrupt));
	save_item(NAME(m_last_taken_interrupt));
	save_item(NAME(m_pseudovia_regs));
	save_item(NAME(m_pseudovia_ier));
	save_item(NAME(m_pseudovia_ifr));
	save_item(NAME(m_pal_address));
	save_item(NAME(m_pal_idx));
	save_item(NAME(m_pal_control));
	save_item(NAME(m_pal_colkey));
	save_item(NAME(m_overlay));

	m_rom_ptr = &m_rom[0];
	m_rom_size = m_rom.length() << 2;

	m_pseudovia_ier = m_pseudovia_ifr = 0;
	m_pal_address = m_pal_idx = m_pal_control = m_pal_colkey = 0;
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void vasp_device::device_reset()
{
	// start 60.15 Hz timer
	m_6015_timer->adjust(attotime::from_hz(60.15), 0, attotime::from_hz(60.15));

	std::fill_n(m_pseudovia_regs, 256, 0);
	m_pseudovia_regs[0] = 0x4f;
	m_pseudovia_regs[1] = 0x06;
	m_pseudovia_regs[2] = 0x7f;
	m_pseudovia_regs[3] = 0;
	m_pseudovia_ier = 0;
	m_pseudovia_ifr = 0;
	m_via_interrupt = m_via2_interrupt = m_scc_interrupt = 0;
	m_last_taken_interrupt = -1;

	// main cpu shouldn't start until Egret wakes it up
	m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);

	m_overlay = true;

	// put ROM mirror at 0
	address_space &space = m_maincpu->space(AS_PROGRAM);
	const u32 memory_size = std::min((u32)0x3fffff, m_rom_size);
	const u32 memory_end = memory_size - 1;
	offs_t memory_mirror = memory_end & ~(memory_size - 1);

	space.unmap_write(0x00000000, memory_end);
	space.install_rom(0x00000000, memory_end & ~memory_mirror, memory_mirror, m_rom_ptr);
}

void vasp_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	for (int i = 0; i < inputs[0].samples(); i++)
	{
		outputs[0].put(i, inputs[0].get(i));
		outputs[1].put(i, inputs[1].get(i));
	}
}

u32 vasp_device::rom_switch_r(offs_t offset)
{
	// disable the overlay
	if (m_overlay && !machine().side_effects_disabled())
	{
		address_space &space = m_maincpu->space(AS_PROGRAM);
		const u32 memory_end = m_ram_size - 1;
		void *memory_data = m_ram_ptr;
		offs_t memory_mirror = memory_end & ~memory_end;

		space.install_ram(0x00000000, memory_end & ~memory_mirror, memory_mirror, memory_data);
		m_overlay = false;
	}

	return m_rom_ptr[offset & ((m_rom_size - 1) >> 2)];
}

void vasp_device::set_ram_info(u32 *ram, u32 size)
{
	m_ram_ptr = ram;
	m_ram_size = size;
}

TIMER_CALLBACK_MEMBER(vasp_device::mac_6015_tick)
{
	m_via1->write_ca1(CLEAR_LINE);
	m_via1->write_ca1(ASSERT_LINE);
}

uint8_t vasp_device::via_in_a()
{
	return 0xd5;
}

uint8_t vasp_device::via_in_b()
{
	return read_pb3() << 3;
}

void vasp_device::via_out_cb2(int state)
{
	write_cb2(state & 1);
}

void vasp_device::via_out_a(uint8_t data)
{
	write_hdsel(BIT(data, 5));
}

void vasp_device::via_out_b(uint8_t data)
{
	write_pb4(BIT(data, 4));
	write_pb5(BIT(data, 5));
}

void vasp_device::via1_irq(int state)
{
	m_via_interrupt = state;
	field_interrupts();
}

void vasp_device::via2_irq(int state)
{
	m_via2_interrupt = state;
	field_interrupts();
}

void vasp_device::field_interrupts()
{
	int take_interrupt = -1;

	if (m_scc_interrupt)
	{
		take_interrupt = 4;
	}
	else if (m_via2_interrupt)
	{
		take_interrupt = 2;
	}
	else if (m_via_interrupt)
	{
		take_interrupt = 1;
	}

	if (m_last_taken_interrupt > -1)
	{
		m_maincpu->set_input_line(m_last_taken_interrupt, CLEAR_LINE);
		m_last_taken_interrupt = -1;
	}

	if (take_interrupt > -1)
	{
		m_maincpu->set_input_line(take_interrupt, ASSERT_LINE);
		m_last_taken_interrupt = take_interrupt;
	}
}

void vasp_device::scc_irq_w(int state)
{
	m_scc_interrupt = (state == ASSERT_LINE);
	field_interrupts();
}

void vasp_device::vbl_w(int state)
{
	if (!state)
	{
		return;
	}

	m_pseudovia_regs[2] &= ~0x40; // set vblank signal

	if (m_pseudovia_regs[0x12] & 0x40)
	{
		pseudovia_recalc_irqs();
	}
}

void vasp_device::slot0_irq_w(int state)
{
	if (state)
	{
		m_pseudovia_regs[2] &= ~0x08;
	}
	else
	{
		m_pseudovia_regs[2] |= 0x08;
	}

	pseudovia_recalc_irqs();
}

void vasp_device::slot1_irq_w(int state)
{
	if (state)
	{
		m_pseudovia_regs[2] &= ~0x10;
	}
	else
	{
		m_pseudovia_regs[2] |= 0x10;
	}

	pseudovia_recalc_irqs();
}

void vasp_device::slot2_irq_w(int state)
{
	if (state)
	{
		m_pseudovia_regs[2] &= ~0x20;
	}
	else
	{
		m_pseudovia_regs[2] |= 0x20;
	}

	pseudovia_recalc_irqs();
}

void vasp_device::asc_irq(int state)
{
	if (state == ASSERT_LINE)
	{
		m_pseudovia_regs[3] |= 0x10; // any VIA 2 interrupt | sound interrupt
		pseudovia_recalc_irqs();
	}
	else
	{
		m_pseudovia_regs[3] &= ~0x10;
		pseudovia_recalc_irqs();
	}
}

void vasp_device::pseudovia_recalc_irqs()
{
	// check slot interrupts and bubble them down to IFR
	uint8_t slot_irqs = (~m_pseudovia_regs[2]) & 0x78;
	slot_irqs &= (m_pseudovia_regs[0x12] & 0x78);

	if (slot_irqs)
	{
		m_pseudovia_regs[3] |= 2; // any slot
	}
	else // no slot irqs, clear the pending bit
	{
		m_pseudovia_regs[3] &= ~2; // any slot
	}

	uint8_t ifr = (m_pseudovia_regs[3] & m_pseudovia_ier) & 0x1b;

	if (ifr != 0)
	{
		m_pseudovia_regs[3] = ifr | 0x80;
		m_pseudovia_ifr = ifr | 0x80;

		via2_irq(ASSERT_LINE);
	}
	else
	{
		via2_irq(CLEAR_LINE);
	}
}

uint8_t vasp_device::pseudovia_r(offs_t offset)
{
	int data = 0;

	if (offset < 0x100)
	{
		data = m_pseudovia_regs[offset];

		if (offset == 0x10)
		{
			data &= ~0x38;
			data |= (m_montype->read() << 3);
		}

		// bit 7 of these registers always reads as 0 on pseudo-VIAs
		if ((offset == 0x12) || (offset == 0x13))
		{
			data &= ~0x80;
		}
	}
	else
	{
		offset >>= 9;

		switch (offset)
		{
		case 13: // IFR
			data = m_pseudovia_ifr;
			break;

		case 14: // IER
			data = m_pseudovia_ier;
			break;

		default:
			logerror("pseudovia_r: Unknown pseudo-VIA register %d access\n", offset);
			break;
		}
	}
	return data;
}

void vasp_device::pseudovia_w(offs_t offset, uint8_t data)
{
	if (offset < 0x100)
	{
		switch (offset)
		{
		case 0x02:
			m_pseudovia_regs[offset] |= (data & 0x40);
			pseudovia_recalc_irqs();
			break;

		case 0x03:           // write here to ack
			if (data & 0x80) // 1 bits write 1s
			{
				m_pseudovia_regs[offset] |= data & 0x7f;
				m_pseudovia_ifr |= data & 0x7f;
			}
			else // 1 bits write 0s
			{
				m_pseudovia_regs[offset] &= ~(data & 0x7f);
				m_pseudovia_ifr &= ~(data & 0x7f);
			}
			pseudovia_recalc_irqs();
			break;

		case 0x10:
			m_pseudovia_regs[offset] = data;
			break;

		case 0x12:
			if (data & 0x80) // 1 bits write 1s
			{
				m_pseudovia_regs[offset] |= data & 0x7f;
			}
			else // 1 bits write 0s
			{
				m_pseudovia_regs[offset] &= ~(data & 0x7f);
			}
			pseudovia_recalc_irqs();
			break;

		case 0x13:
			if (data & 0x80) // 1 bits write 1s
			{
				m_pseudovia_regs[offset] |= data & 0x7f;

				if (data == 0xff)
					m_pseudovia_regs[offset] = 0x1f; // I don't know why this is special, but the IIci ROM's POST demands it
			}
			else // 1 bits write 0s
			{
				m_pseudovia_regs[offset] &= ~(data & 0x7f);
			}
			break;

		default:
			m_pseudovia_regs[offset] = data;
			break;
		}
	}
	else
	{
		offset >>= 9;

		switch (offset)
		{
		case 13: // IFR
			if (data & 0x80)
			{
				data = 0x7f;
			}
			pseudovia_recalc_irqs();
			break;

		case 14:             // IER
			if (data & 0x80) // 1 bits write 1s
			{
				m_pseudovia_ier |= data & 0x7f;
			}
			else // 1 bits write 0s
			{
				m_pseudovia_ier &= ~(data & 0x7f);
			}
			pseudovia_recalc_irqs();
			break;

		default:
			logerror("pseudovia_w: Unknown extended pseudo-VIA register %d access\n", offset);
			break;
		}
	}
}

void vasp_device::cb1_w(int state)
{
	m_via1->write_cb1(state);
}

void vasp_device::cb2_w(int state)
{
	m_via1->write_cb2(state);
}

uint16_t vasp_device::mac_via_r(offs_t offset)
{
	uint16_t data;

	offset >>= 8;
	offset &= 0x0f;

	if (!machine().side_effects_disabled())
		via_sync();

	data = m_via1->read(offset);

	return (data & 0xff) | (data << 8);
}

void vasp_device::mac_via_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	offset >>= 8;
	offset &= 0x0f;

	via_sync();

	if (ACCESSING_BITS_0_7)
		m_via1->write(offset, data & 0xff);
	if (ACCESSING_BITS_8_15)
		m_via1->write(offset, (data >> 8) & 0xff);
}

void vasp_device::via_sync()
{
	// The via runs at 783.36KHz while the main cpu runs at 15MHz or
	// more, so we need to sync the access with the via clock.  Plus
	// the whole access takes half a (via) cycle and ends when synced
	// with the main cpu again.

	// Get the main cpu time
	u64 cycle = m_maincpu->total_cycles();

	// Get the number of the cycle the via is in at that time
	u64 via_cycle = cycle * m_via1->clock() / m_maincpu->clock();

	// The access is going to start at via_cycle+1 and end at
	// via_cycle+1.5, compute what that means in maincpu cycles (the
	// +1 rounds up, since the clocks are too different to ever be
	// synced).
	u64 main_cycle = (via_cycle * 2 + 3) * m_maincpu->clock() / (2 * m_via1->clock()) + 1;

	// Finally adjust the main cpu icount as needed.
	m_maincpu->adjust_icount(-int(main_cycle - cycle));
}

u32 vasp_device::vram_r(offs_t offset)
{
	return m_vram[offset];
}

void vasp_device::vram_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_vram[offset]);
}

u8 vasp_device::dac_r(offs_t offset)
{
	switch (offset)
	{
	case 2:
		return m_pal_control;

	default:
		return 0;
	}
}

void vasp_device::dac_w(offs_t offset, u8 data)
{
	switch (offset)
	{
	case 0:
		m_pal_address = data;
		m_pal_idx = 0;
		break;

	case 1:
		switch (m_pal_idx)
		{
		case 0:
			m_palette->set_pen_red_level(m_pal_address, data);
			break;
		case 1:
			m_palette->set_pen_green_level(m_pal_address, data);
			break;
		case 2:
			m_palette->set_pen_blue_level(m_pal_address, data);
			break;
		}
		m_pal_idx++;
		if (m_pal_idx == 3)
		{
			m_pal_idx = 0;
			m_pal_address++;
		}
		break;

	case 2:
		m_pal_control = data;
		break;

	case 3:
		m_pal_colkey = data;
		break;
	}
}

u32 vasp_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int hres, vres;
	switch (m_montype->read())
	{
	case 1: // 15" portrait display
		hres = 640;
		vres = 870;
		break;

	case 2: // 12" RGB
		hres = 512;
		vres = 384;
		break;

	case 6: // 13" RGB
	default:
		hres = 640;
		vres = 480;
		break;
	}

	const pen_t *pens = m_palette->pens();

	switch (m_pseudovia_regs[0x10] & 7)
	{
	case 0: // 1bpp
	{
		auto const vram8 = util::big_endian_cast<uint8_t const>(&m_vram[0]);

		for (int y = 0; y < vres; y++)
		{
			uint32_t *scanline = &bitmap.pix(y);
			for (int x = 0; x < hres; x += 8)
			{
				uint8_t const pixels = vram8[(y * 2048) + (x / 8)];

				*scanline++ = pens[0x7f | (pixels & 0x80)];
				*scanline++ = pens[0x7f | ((pixels << 1) & 0x80)];
				*scanline++ = pens[0x7f | ((pixels << 2) & 0x80)];
				*scanline++ = pens[0x7f | ((pixels << 3) & 0x80)];
				*scanline++ = pens[0x7f | ((pixels << 4) & 0x80)];
				*scanline++ = pens[0x7f | ((pixels << 5) & 0x80)];
				*scanline++ = pens[0x7f | ((pixels << 6) & 0x80)];
				*scanline++ = pens[0x7f | ((pixels << 7) & 0x80)];
			}
		}
	}
	break;

	case 1: // 2bpp
	{
		auto const vram8 = util::big_endian_cast<uint8_t const>(&m_vram[0]);

		for (int y = 0; y < vres; y++)
		{
			uint32_t *scanline = &bitmap.pix(y);
			for (int x = 0; x < hres / 4; x++)
			{
				uint8_t const pixels = vram8[(y * 2048) + x];

				*scanline++ = pens[0x3f | (pixels & 0xc0)];
				*scanline++ = pens[0x3f | ((pixels << 2) & 0xc0)];
				*scanline++ = pens[0x3f | ((pixels << 4) & 0xc0)];
				*scanline++ = pens[0x3f | ((pixels << 6) & 0xc0)];
			}
		}
	}
	break;

	case 2: // 4bpp
	{
		auto const vram8 = util::big_endian_cast<uint8_t const>(&m_vram[0]);

		for (int y = 0; y < vres; y++)
		{
			uint32_t *scanline = &bitmap.pix(y);

			for (int x = 0; x < hres / 2; x++)
			{
				uint8_t const pixels = vram8[(y * 2048) + x];

				*scanline++ = pens[0x0f | (pixels & 0xf0)];
				*scanline++ = pens[0x0f | ((pixels << 4) & 0xf0)];
			}
		}
	}
	break;

	case 3: // 8bpp
	{
		auto const vram8 = util::big_endian_cast<uint8_t const>(&m_vram[0]);

		for (int y = 0; y < vres; y++)
		{
			uint32_t *scanline = &bitmap.pix(y);

			for (int x = 0; x < hres; x++)
			{
				uint8_t const pixels = vram8[(y * 2048) + x];
				*scanline++ = pens[pixels];
			}
		}
	}
	break;

	case 4: // 16bpp
	{
		auto const vram16 = util::big_endian_cast<uint16_t const>(&m_vram[0]);

		for (int y = 0; y < vres; y++)
		{
			uint32_t *scanline = &bitmap.pix(y);
			for (int x = 0; x < hres; x++)
			{
				uint16_t const pixels = vram16[(y * 1024) + x];
				*scanline++ = rgb_t(((pixels >> 10) & 0x1f) << 3, ((pixels >> 5) & 0x1f) << 3, (pixels & 0x1f) << 3);
			}
		}
	}
	break;
	}

	return 0;
}
