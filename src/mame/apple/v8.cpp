// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    Apple "V8", "Eagle", "Spice", and "Tinkerbell" system ASICs
    Emulation by R. Belmont

    V8 (343S0116 or 343-0155) contains the following:
    - A memory controller for up to 10 MB (2 or 4 MB on the motherboard + 6 or 8 MB in SIMMs)
    - A VRAM controller and framebuffer controller, supporting monitor ID selection
    - A full VIA (VIA1) and a "pseudo-VIA", which is basically a combination GPIO and
      interrupt controller that looks somewhat like a VIA with no timers and no shift register
    - An ASC-like 4-channel audio controller, with a 1 KB FIFO stored in main memory
    - Support logic for various external subsystems (ADB, FDC, PDS, SCC, SCSI)

    The VRAM data path is 16 bits wide and connects to the Ariel RAMDAC (343S1045 or 344S0145
    on the original LC, 343S1069 or 344S1069 on later models).

    Eagle (343S1054) is similar to V8 but only supports a 512x342 monochrome monitor.
    It generates the video output directly without a RAMDAC.

    Spice (343S0132) is an enhanced version of V8 that incorporates a SWIM2 floppy controller
    and adds the option for an optional ROM expansion plus support for pushbutton sound volume
    and display intensity controls and power saver mode.

    Tinker Bell (343S1109) is an evolution of Spice with a simpler memory controller that has
    an 8MB limit (4MB on the motherboard, 1, 2, or 4MB in the SIMM slot) and support for the
    Macintosh TV's video input feature.

    VISA (343S0101) is a predecessor of V8 and Eagle without support for dedicated VRAM banks
    or VGA modes.  It was coupled to a non-customized Bt450 RAMDAC on the Elsie prototype, and
    did not offer video modes with more than 4 bits per pixel.

    The RAM controller in these chips is weird.  The motherboard RAM maps after any SIMMs, and
    the first 2MB of the motherboard RAM always exists in the 0x800000-0x9FFFFF range.  Later
    ASICs like Sonora and VASP simply always put the motherboard RAM at 0 and the SIMMs after it.
*/

#include "emu.h"
#include "v8.h"

#include "cpu/m68000/m68030.h"
#include "formats/ap_dsk35.h"
#include "layout/generic.h"

#define LOG_RAM (1U << 1)

#define VERBOSE (0)
#include "logmacro.h"

static constexpr u32 C7M  = 7833600;
static constexpr u32 C15M = (C7M * 2);

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(V8, v8_device, "v8", "Apple V8 system ASIC")
DEFINE_DEVICE_TYPE(EAGLE, eagle_device, "v8eagle", "Apple Eagle system ASIC")
DEFINE_DEVICE_TYPE(SPICE, spice_device, "v8spice", "Apple Spice system ASIC")
DEFINE_DEVICE_TYPE(TINKERBELL, tinkerbell_device, "v8tkbell", "Apple Tinker Bell system ASIC")

static INPUT_PORTS_START( v8 )
	PORT_START("MONTYPE")
	PORT_CONFNAME(0x0f, 0x06, "Connected monitor")
	PORT_CONFSETTING( 0x01, "15\" Portrait Display (640x870)")
	PORT_CONFSETTING( 0x02, "12\" RGB (512x384)")
	PORT_CONFSETTING( 0x06, "13\" RGB (640x480)")
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor v8_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( v8 );
}

//-------------------------------------------------
//  ADDRESS_MAP
//-------------------------------------------------

void v8_device::map(address_map &map)
{
	map(0x000000, 0x0fffff).r(FUNC(v8_device::rom_switch_r));

	map(0x500000, 0x501fff).rw(FUNC(v8_device::mac_via_r), FUNC(v8_device::mac_via_w));
	map(0x514000, 0x515fff).rw(m_asc, FUNC(asc_device::read), FUNC(asc_device::write));
	map(0x524000, 0x525fff).rw(FUNC(v8_device::dac_r), FUNC(v8_device::dac_w));
	map(0x526000, 0x527fff).rw(m_pseudovia, FUNC(pseudovia_device::read), FUNC(pseudovia_device::write));

	map(0x540000, 0x5bffff).rw(FUNC(v8_device::vram_r), FUNC(v8_device::vram_w));
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void v8_device::device_add_mconfig(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(25175000, 800, 0, 640, 525, 0, 480);
	m_screen->set_screen_update(FUNC(v8_device::screen_update));
	m_screen->screen_vblank().set(m_pseudovia, FUNC(pseudovia_device::slot_irq_w<0x40>));
	config.set_default_layout(layout_monitors);

	PALETTE(config, m_palette).set_entries(256);

	R65NC22(config, m_via1, C7M / 10);
	m_via1->readpa_handler().set(FUNC(v8_device::via_in_a));
	m_via1->readpb_handler().set(FUNC(v8_device::via_in_b));
	m_via1->writepa_handler().set(FUNC(v8_device::via_out_a));
	m_via1->writepb_handler().set(FUNC(v8_device::via_out_b));
	m_via1->cb2_handler().set(FUNC(v8_device::via_out_cb2));
	m_via1->irq_handler().set(FUNC(v8_device::via1_irq));

	ASC(config, m_asc, C15M, asc_device::asc_type::V8);
	m_asc->irqf_callback().set(m_pseudovia, FUNC(pseudovia_device::asc_irq_w));
	m_asc->add_route(0, tag(), 1.0);
	m_asc->add_route(1, tag(), 1.0);

	APPLE_PSEUDOVIA(config, m_pseudovia, C15M);
	m_pseudovia->writepb_handler().set(FUNC(v8_device::via2_pb_w));
	m_pseudovia->readconfig_handler().set(FUNC(v8_device::via2_config_r));
	m_pseudovia->writeconfig_handler().set(FUNC(v8_device::via2_config_w));
	m_pseudovia->readvideo_handler().set(FUNC(v8_device::via2_video_config_r));
	m_pseudovia->writevideo_handler().set(FUNC(v8_device::via2_video_config_w));
	m_pseudovia->irq_callback().set(FUNC(v8_device::via2_irq));
}

//-------------------------------------------------
//  v8_device - constructor
//-------------------------------------------------

v8_device::v8_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	v8_device(mconfig, V8, tag, owner, clock)
{
}

v8_device::v8_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	m_maincpu(*this, finder_base::DUMMY_TAG),
	m_screen(*this, "screen"),
	m_palette(*this, "palette"),
	m_asc(*this, "asc"),
	m_pseudovia(*this, "pseudovia"),
	m_overlay(false),
	m_video_config(0),
	write_pb4(*this),
	write_pb5(*this),
	write_cb2(*this),
	write_hdsel(*this),
	write_hmmu_enable(*this),
	read_pb3(*this, 0),
	m_montype(*this, "MONTYPE"),
	m_via1(*this, "via1"),
	m_rom(*this, finder_base::DUMMY_TAG),
	m_config(0),
	m_baseIs4M(false)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void v8_device::device_start()
{
	m_vram = std::make_unique<u32[]>(0x100000 / sizeof(u32));

	m_stream = stream_alloc(8, 2, m_asc->clock(), STREAM_SYNCHRONOUS);

	m_6015_timer = timer_alloc(FUNC(v8_device::mac_6015_tick), this);
	m_6015_timer->adjust(attotime::never);

	save_pointer(NAME(m_vram), 0x100000/sizeof(u32));
	save_item(NAME(m_via_interrupt));
	save_item(NAME(m_via2_interrupt));
	save_item(NAME(m_scc_interrupt));
	save_item(NAME(m_last_taken_interrupt));
	save_item(NAME(m_pal_address));
	save_item(NAME(m_pal_idx));
	save_item(NAME(m_pal_control));
	save_item(NAME(m_pal_colkey));
	save_item(NAME(m_config));
	save_item(NAME(m_video_config));
	save_item(NAME(m_overlay));

	m_pal_address = m_pal_idx = m_pal_control = m_pal_colkey = 0;
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void v8_device::device_reset()
{
	// start 60.15 Hz timer
	m_6015_timer->adjust(attotime::from_hz(60.15), 0, attotime::from_hz(60.15));

	m_via_interrupt = m_via2_interrupt = m_scc_interrupt = 0;
	m_last_taken_interrupt = -1;

	// main cpu shouldn't start until Egret wakes it up
	m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);

	m_overlay = true;

	// put ROM mirror at 0
	address_space &space = m_maincpu->space(AS_PROGRAM);
	const u32 memory_size = std::min<u32>(0x3fffff, m_rom.length() << 2);
	const u32 memory_end = memory_size - 1;
	offs_t memory_mirror = memory_end & ~(memory_size - 1);

	space.unmap_write(0x00000000, memory_end);
	space.install_rom(0x00000000, memory_end & ~memory_mirror, memory_mirror, &m_rom[0]);
}

void v8_device::sound_stream_update(sound_stream &stream)
{
	stream.copy(0, 0);
	stream.copy(1, 1);
}

u32 v8_device::rom_switch_r(offs_t offset)
{
	// disable the overlay
	if (m_overlay && !machine().side_effects_disabled())
	{
		ram_size(0xc0); // full SIMM, full 4MB onboard
		m_overlay = false;
	}

	return m_rom[offset & (m_rom.length() - 1)];
}

void v8_device::set_ram_info(u32 *ram, u32 size)
{
	m_ram_ptr = ram;
	m_ram_size = size;
}

TIMER_CALLBACK_MEMBER(v8_device::mac_6015_tick)
{
	m_via1->write_ca1(CLEAR_LINE);
	m_via1->write_ca1(ASSERT_LINE);
}

u8 v8_device::via_in_a()
{
	return 0xd5;
}

u8 v8_device::via_in_b()
{
	return read_pb3() << 3;
}

void v8_device::via_out_cb2(int state)
{
	write_cb2(state & 1);
}

void v8_device::via_out_a(u8 data)
{
	write_hdsel(BIT(data, 5));
}

void v8_device::via_out_b(u8 data)
{
	write_pb4(BIT(data, 4));
	write_pb5(BIT(data, 5));
}

void v8_device::via1_irq(int state)
{
	m_via_interrupt = state;
	field_interrupts();
}

void v8_device::via2_irq(int state)
{
	m_via2_interrupt = state;
	field_interrupts();
}

void v8_device::field_interrupts()
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

void v8_device::scc_irq_w(int state)
{
	m_scc_interrupt = (state == ASSERT_LINE);
	field_interrupts();
}

void v8_device::slot2_irq_w(int state)
{
	m_pseudovia->slot_irq_w<0x20>(state);
}

u8 v8_device::via2_config_r()
{
	return m_config | 0x04;
}

void v8_device::via2_config_w(u8 data)
{
	m_config = data;
	ram_size(data);
}

u8 v8_device::via2_video_config_r()
{
	return m_montype->read() << 3;
}

void v8_device::via2_video_config_w(u8 data)
{
	m_video_config = data;
}

void v8_device::via2_pb_w(u8 data)
{
	write_hmmu_enable(BIT(data, 3));
}

void v8_device::ram_size(u8 config)
{
	if (!m_overlay)
	{
		address_space &space = m_maincpu->space(AS_PROGRAM);
		const void *mb_ram = m_ram_ptr;
		u32 simm_size = m_ram_size - (m_baseIs4M ? 0x400000 : 0x200000);
		void *simm_ram = &m_ram_ptr[(m_baseIs4M ? 0x400000 : 0x200000)/4];

		// for the full 10M config on a machine with 4MB on the motherboard,
		// compensate for the fact that 2MB of motherboard RAM isn't used.
		if (m_ram_size == 0xa00000)
		{
			simm_size = 0x800000;
			simm_ram = &m_ram_ptr[(0x200000/4)];
		}

		space.unmap_readwrite(0x000000, 0x9fffff);

		// the first two megs of MB RAM is always at 0x800000
		space.install_ram(0x800000, 0x9fffff, 0, (void *)mb_ram);
		/*
		    bit 5 = bank B (motherboard soldered) size, 0 = 4MB, 1 = 2MB
		    bits 6-7 = bank A (SIMM) size,
		        0 = 0MB
		        1 = 2MB
		        2 = 4MB
		        3 = 8MB
		*/

		u32 mb_location = 0x000000;

		// is SIMM RAM enabled?  it always goes at zero.
		if (simm_size > 0)
		{
			if ((config & 0xc0) != 0)
			{
				LOGMASKED(LOG_RAM, "SIMM RAM at %x to %x\n", mb_location, simm_size - 1);
				space.install_ram(0x000000, simm_size - 1, 0, (void *)simm_ram);

				static u32 simm_sizes[4] = { 0, 0x200000, 0x400000, 0x800000 };
				mb_location += simm_sizes[(config >> 6) & 3];
				LOGMASKED(LOG_RAM, "Configured SIMM size is %x\n", simm_sizes[(config >> 6) & 3]);
			}
		}
		else
		{
			LOGMASKED(LOG_RAM, "Base config, no SIMM\n");
		}

		// finally place the motherboard RAM if we can
		if ((config & 0xc0) != 0xc0)
		{
			u32 mb_size = m_baseIs4M ? 0x400000 : 0x200000;
			// force 2MB only of motherboard RAM if that bit is set, or
			// if the SIMM is all the way to
			if (config & 0x20)
			{
				mb_size = 0x200000;
			}
			LOGMASKED(LOG_RAM, "Motherboard RAM at %x to %x\n", mb_location, (mb_location + mb_size) - 1);
			space.install_ram(mb_location, (mb_location + mb_size) - 1, 0, (void *)mb_ram);
		}
		else
		{
			LOGMASKED(LOG_RAM, "SIMM is 8MB, motherboard is 0x800000 mirror only\n");
		}
	}
}

void v8_device::cb1_w(int state)
{
	m_via1->write_cb1(state);
}

void v8_device::cb2_w(int state)
{
	m_via1->write_cb2(state);
}

u16 v8_device::mac_via_r(offs_t offset)
{
	u16 data;

	offset >>= 8;
	offset &= 0x0f;

	if (!machine().side_effects_disabled())
		via_sync();

	data = m_via1->read(offset);

	return (data & 0xff) | (data << 8);
}

void v8_device::mac_via_w(offs_t offset, u16 data, u16 mem_mask)
{
	offset >>= 8;
	offset &= 0x0f;

	via_sync();

	if (ACCESSING_BITS_0_7)
		m_via1->write(offset, data & 0xff);
	if (ACCESSING_BITS_8_15)
		m_via1->write(offset, (data >> 8) & 0xff);
}

void v8_device::via_sync()
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

u32 v8_device::vram_r(offs_t offset)
{
	return m_vram[offset];
}

void v8_device::vram_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_vram[offset]);
}

u8 v8_device::dac_r(offs_t offset)
{
	switch (offset)
	{
	case 2:
		return m_pal_control;

	default:
		return 0;
	}
}

void v8_device::dac_w(offs_t offset, u8 data)
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

u32 v8_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
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

	switch (m_video_config & 7)
	{
	case 0: // 1bpp
	{
		auto const vram8 = util::big_endian_cast<u8 const>(&m_vram[0]);

		for (int y = 0; y < vres; y++)
		{
			u32 *scanline = &bitmap.pix(y);
			for (int x = 0; x < hres; x += 8)
			{
				u8 const pixels = vram8[(y * 1024) + (x / 8)];

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
		auto const vram8 = util::big_endian_cast<u8 const>(&m_vram[0]);

		for (int y = 0; y < vres; y++)
		{
			u32 *scanline = &bitmap.pix(y);
			for (int x = 0; x < hres / 4; x++)
			{
				u8 const pixels = vram8[(y * 1024) + x];

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
		auto const vram8 = util::big_endian_cast<u8 const>(&m_vram[0]);

		for (int y = 0; y < vres; y++)
		{
			u32 *scanline = &bitmap.pix(y);

			for (int x = 0; x < hres / 2; x++)
			{
				u8 const pixels = vram8[(y * 1024) + x];

				*scanline++ = pens[0x0f | (pixels & 0xf0)];
				*scanline++ = pens[0x0f | ((pixels << 4) & 0xf0)];
			}
		}
	}
	break;

	case 3: // 8bpp
	{
		auto const vram8 = util::big_endian_cast<u8 const>(&m_vram[0]);

		for (int y = 0; y < vres; y++)
		{
			u32 *scanline = &bitmap.pix(y);

			for (int x = 0; x < hres; x++)
			{
				u8 const pixels = vram8[(y * 1024) + x];
				*scanline++ = pens[pixels];
			}
		}
	}
	break;

	case 4: // 16bpp
	{
		auto const vram16 = util::big_endian_cast<u16 const>(&m_vram[0]);

		for (int y = 0; y < vres; y++)
		{
			u32 *scanline = &bitmap.pix(y);
			for (int x = 0; x < hres; x++)
			{
				u16 const pixels = vram16[(y * hres) + x];
				*scanline++ = rgb_t(((pixels >> 10) & 0x1f) << 3, ((pixels >> 5) & 0x1f) << 3, (pixels & 0x1f) << 3);
			}
		}
	}
	break;
	}

	return 0;
}

// ================ eagle_device

static INPUT_PORTS_START( eagle )
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor eagle_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( eagle );
}

void eagle_device::device_add_mconfig(machine_config &config)
{
	v8_device::device_add_mconfig(config);
	m_screen->set_raw(15.6672_MHz_XTAL, 704, 0, 512, 370, 0, 342);

	ASC(config.replace(), m_asc, C15M, asc_device::asc_type::EAGLE);
	m_asc->add_route(0, tag(), 1.0);
	m_asc->add_route(1, tag(), 1.0);
	m_asc->irqf_callback().set(m_pseudovia, FUNC(pseudovia_device::asc_irq_w));

	m_pseudovia->readvideo_handler().set(FUNC(eagle_device::via2_video_config_r));
}

eagle_device::eagle_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: v8_device(mconfig, EAGLE, tag, owner, clock)
{
}

u8 eagle_device::via_in_a()
{
	return 0x93;
}

u8 eagle_device::via2_video_config_r()
{
	return 0;
}

u32 eagle_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const u32 pens[2] = { 0xffffffff, 0x00000000 };
	auto const vram8 = util::big_endian_cast<u8 const>(&m_ram_ptr[0x1f9a80/4]);

	for (int y = 0; y < 342; y++)
	{
		u32 *scanline = &bitmap.pix(y);
		for (int x = 0; x < 512; x += 8)
		{
			u8 const pixels = vram8[(y * (512/8)) + (x / 8)];

			*scanline++ = pens[((pixels >> 7) & 0x1)];
			*scanline++ = pens[((pixels >> 6) & 0x1)];
			*scanline++ = pens[((pixels >> 5) & 0x1)];
			*scanline++ = pens[((pixels >> 4) & 0x1)];
			*scanline++ = pens[((pixels >> 3) & 0x1)];
			*scanline++ = pens[((pixels >> 2) & 0x1)];
			*scanline++ = pens[((pixels >> 1) & 0x1)];
			*scanline++ = pens[(pixels & 0x1)];
		}
	}

	return 0;
}

// ================ spice_device

void spice_device::map(address_map &map)
{
	v8_device::map(map);

	map(0x516000, 0x517fff).rw(FUNC(spice_device::swim_r), FUNC(spice_device::swim_w));
	map(0x518000, 0x518001).w(FUNC(spice_device::bright_contrast_w));
}

static INPUT_PORTS_START(spice)
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------
ioport_constructor spice_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(spice);
}

void spice_device::device_add_mconfig(machine_config &config)
{
	v8_device::device_add_mconfig(config);
	m_screen->set_raw(15.6672_MHz_XTAL, 640, 0, 512, 407, 0, 384);

	ASC(config.replace(), m_asc, C15M, asc_device::asc_type::SONORA);
	m_asc->add_route(0, tag(), 1.0);
	m_asc->add_route(1, tag(), 1.0);
	m_asc->irqf_callback().set(m_pseudovia, FUNC(pseudovia_device::asc_irq_w));

	SWIM2(config, m_fdc, C15M);
	m_fdc->devsel_cb().set(FUNC(spice_device::devsel_w));
	m_fdc->phases_cb().set(FUNC(spice_device::phases_w));

	applefdintf_device::add_35_hd(config, m_floppy[0]);
	applefdintf_device::add_35_nc(config, m_floppy[1]);

	m_pseudovia->readvideo_handler().set(FUNC(spice_device::via2_video_config_r));
}

spice_device::spice_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: v8_device(mconfig, type, tag, owner, clock),
	  m_fdc(*this, "fdc"),
	  m_floppy(*this, "fdc:%d", 0U),
	  m_cur_floppy(nullptr),
	  m_hdsel(0)
{
}

spice_device::spice_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: spice_device(mconfig, SPICE, tag, owner, clock)
{
}

void spice_device::device_start()
{
	v8_device::device_start();

	save_item(NAME(m_hdsel));
}

u8 spice_device::via_in_a()
{
	return 0x83;
}

u8 spice_device::via2_video_config_r()
{
	return 0x02 << 3;
}

u32 spice_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int hres, vres;
	hres = 512;
	vres = 384;

	const pen_t *pens = m_palette->pens();
	switch (m_video_config & 7)
	{
	case 0: // 1bpp
	{
		auto const vram8 = util::big_endian_cast<u8 const>(&m_vram[0]);

		for (int y = 0; y < vres; y++)
		{
			u32 *scanline = &bitmap.pix(y);
			for (int x = 0; x < hres; x += 8)
			{
				u8 const pixels = vram8[(y * 1024) + (x / 8)];

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
		auto const vram8 = util::big_endian_cast<u8 const>(&m_vram[0]);

		for (int y = 0; y < vres; y++)
		{
			u32 *scanline = &bitmap.pix(y);
			for (int x = 0; x < hres / 4; x++)
			{
				u8 const pixels = vram8[(y * 1024) + x];

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
		auto const vram8 = util::big_endian_cast<u8 const>(&m_vram[0]);

		for (int y = 0; y < vres; y++)
		{
			u32 *scanline = &bitmap.pix(y);

			for (int x = 0; x < hres / 2; x++)
			{
				u8 const pixels = vram8[(y * 1024) + x];

				*scanline++ = pens[0x0f | (pixels & 0xf0)];
				*scanline++ = pens[0x0f | ((pixels << 4) & 0xf0)];
			}
		}
	}
	break;

	case 3: // 8bpp
	{
		auto const vram8 = util::big_endian_cast<u8 const>(&m_vram[0]);

		for (int y = 0; y < vres; y++)
		{
			u32 *scanline = &bitmap.pix(y);

			for (int x = 0; x < hres; x++)
			{
				u8 const pixels = vram8[(y * 1024) + x];
				*scanline++ = pens[pixels];
			}
		}
	}
	break;

	case 4: // 16bpp
	{
		auto const vram16 = util::big_endian_cast<u16 const>(&m_vram[0]);

		for (int y = 0; y < vres; y++)
		{
			u32 *scanline = &bitmap.pix(y);
			for (int x = 0; x < hres; x++)
			{
				u16 const pixels = vram16[(y * hres) + x];
				*scanline++ = rgb_t(((pixels >> 10) & 0x1f) << 3, ((pixels >> 5) & 0x1f) << 3, (pixels & 0x1f) << 3);
			}
		}
	}
	break;
	}

	return 0;
}

u16 spice_device::swim_r(offs_t offset, u16 mem_mask)
{
	if (!machine().side_effects_disabled())
	{
		m_maincpu->adjust_icount(-5);
	}

	u16 result = m_fdc->read((offset >> 8) & 0xf);
	return result << 8;
}
void spice_device::swim_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_fdc->write((offset >> 8) & 0xf, data & 0xff);
	else
		m_fdc->write((offset >> 8) & 0xf, data >> 8);
}

void spice_device::phases_w(u8 phases)
{
	if (m_cur_floppy)
		m_cur_floppy->seek_phase_w(phases);
}

void spice_device::devsel_w(u8 devsel)
{
	if (devsel == 1)
		m_cur_floppy = m_floppy[0]->get_device();
	else if (devsel == 2)
		m_cur_floppy = m_floppy[1]->get_device();
	else
		m_cur_floppy = nullptr;

	m_fdc->set_floppy(m_cur_floppy);
	if (m_cur_floppy)
		m_cur_floppy->ss_w(m_hdsel);
}

void spice_device::via_out_a(u8 data)
{
	int hdsel = BIT(data, 5);
	if (hdsel != m_hdsel)
	{
		if (m_cur_floppy)
		{
			m_cur_floppy->ss_w(hdsel);
		}
	}
	m_hdsel = hdsel;
}

void spice_device::bright_contrast_w(offs_t offset, u8 data)
{
	// offset 0 = brightness (0-255)
	// offset 1 = contrast (0-255)
}

// ================ tinkerbell_device

void tinkerbell_device::device_add_mconfig(machine_config &config)
{
	spice_device::device_add_mconfig(config);
	m_screen->set_raw(25175000, 800, 0, 640, 525, 0, 480);

	m_pseudovia->readvideo_handler().set(FUNC(tinkerbell_device::via2_video_config_r));
}

tinkerbell_device::tinkerbell_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: spice_device(mconfig, TINKERBELL, tag, owner, clock)
{
}

u8 tinkerbell_device::via_in_a()
{
	return 0x84;
}

u8 tinkerbell_device::via2_video_config_r()
{
	return 0x06 << 3; // ID as an Apple 13" 640x480 monitor
}

u32 tinkerbell_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int hres, vres;
	hres = 640;
	vres = 480;

	const pen_t *pens = m_palette->pens();
	switch (m_video_config & 7)
	{
	case 0: // 1bpp
	{
		auto const vram8 = util::big_endian_cast<u8 const>(&m_vram[0]);

		for (int y = 0; y < vres; y++)
		{
			u32 *scanline = &bitmap.pix(y);
			for (int x = 0; x < hres; x += 8)
			{
				u8 const pixels = vram8[(y * 1024) + (x / 8)];

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
		auto const vram8 = util::big_endian_cast<u8 const>(&m_vram[0]);

		for (int y = 0; y < vres; y++)
		{
			u32 *scanline = &bitmap.pix(y);
			for (int x = 0; x < hres / 4; x++)
			{
				u8 const pixels = vram8[(y * 1024) + x];

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
		auto const vram8 = util::big_endian_cast<u8 const>(&m_vram[0]);

		for (int y = 0; y < vres; y++)
		{
			u32 *scanline = &bitmap.pix(y);

			for (int x = 0; x < hres / 2; x++)
			{
				u8 const pixels = vram8[(y * 1024) + x];

				*scanline++ = pens[0x0f | (pixels & 0xf0)];
				*scanline++ = pens[0x0f | ((pixels << 4) & 0xf0)];
			}
		}
	}
	break;

	case 3: // 8bpp
	{
		auto const vram8 = util::big_endian_cast<u8 const>(&m_vram[0]);

		for (int y = 0; y < vres; y++)
		{
			u32 *scanline = &bitmap.pix(y);

			for (int x = 0; x < hres; x++)
			{
				u8 const pixels = vram8[(y * 1024) + x];
				*scanline++ = pens[pixels];
			}
		}
	}
	break;

	case 4: // 16bpp
	{
		auto const vram16 = util::big_endian_cast<u16 const>(&m_vram[0]);

		for (int y = 0; y < vres; y++)
		{
			u32 *scanline = &bitmap.pix(y);
			for (int x = 0; x < hres; x++)
			{
				u16 const pixels = vram16[(y * hres) + x];
				*scanline++ = rgb_t(((pixels >> 10) & 0x1f) << 3, ((pixels >> 5) & 0x1f) << 3, (pixels & 0x1f) << 3);
			}
		}
	}
	break;
	}

	return 0;
}

/*
    Tinker Bell is different from the V8, but it still needs to kind of act like one.
    The major difference: the RAM limit is 8MB instead of 10, and no RAM appears
    above 0x7FFFFFFF.  Also, when the bits are set for what on V8 would be 8MB SIMM
    and no motherboard RAM except the 0x800000 image, that means 4MB of motherboard
    and 4MB of SIMM here.
*/
void tinkerbell_device::ram_size(u8 config)
{
	if (!m_overlay)
	{
		address_space &space = m_maincpu->space(AS_PROGRAM);
		const void *mb_ram = m_ram_ptr;
		u32 simm_size = m_ram_size - 0x400000;
		void *simm_ram = &m_ram_ptr[0x400000 / 4];

		space.unmap_readwrite(0x000000, 0x9fffff);

		// place the motherboard RAM at 0
		LOGMASKED(LOG_RAM, "Motherboard RAM at 0x00000000 to 0x003fffff\n");
		space.install_ram(0, 0x3fffff, 0, (void *)mb_ram);

		// is SIMM RAM present?  it always goes at 0x400000
		if (simm_size > 0)
		{
			if ((config & 0xc0) != 0)
			{
				LOGMASKED(LOG_RAM, "SIMM RAM at 0x400000 to %x\n", simm_size - 1);
				space.install_ram(0x400000, (0x400000 + simm_size) - 1, 0, (void *)simm_ram);
			}
		}
		else
		{
			LOGMASKED(LOG_RAM, "Base config, no SIMM\n");
		}
	}
}

