// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Silicon Graphics GR1 (aka Eclipse) graphics hardware.
 *
 * This graphics system was first designed for the Personal Iris 4D/20,
 * connecting via a unique graphics port interface. The system was later
 * adapted and released under the IrisVision brand, initially with a Micro
 * Channel Architecture bus interface and support for IBM RS/6000 and some IBM
 * PS/2 systems. A final variant has a 16-bit ISA bus interface enabling use in
 * PC-AT and compatible systems.
 *
 * The TG-V variation fits into the VME chassis and slots of the SGI V20 and
 * V30/35, however control and data signals are delivered via a ribbon cable to
 * the CPU and I/O boards, and only draws power and ground from the VME slot.
 *
 * All versions of the hardware consist of two primary boards which mount into
 * host system slots, plus several daughter-boards which mount on the two
 * primary boards.
 *
 * The board set consists of:
 *
 *   Name     Description      Function
 *            Geometry Engine  host interface and geometry engine (HQ1, GE5)
 *            Display Engine   raster video and display subsystem (RE2 + 1-2 Bt431 cursor chips + 8 pixel, 2 overlay/underlay, 2 wid bitplanes)
 *            Video Driver     basic video daughter-board (5 XPC1 ASICs, Bt458 RGB RAMDAC)
 *            Video Driver     enhanced video daughter-board (5 XMAP2 ASICs, 8K color map, 3 Bt457 RAMDACs)
 *   OP1      Bitplane Board   optional bitplane expansion (adds 2 overlay/underlay, 2 wid planes)
 *   BP4      Bitplane Board   optional bitplane expansion (adds 16 pixel, 2 overlay/underlay, 2 wid planes, 4k color map)
 *   ZB3      Z-buffer         Z-buffer option (adds 24 Z-buffer planes, requires BP4)
 *   GT1/GT2  Turbo Board      turbo graphics option (4 TMS320C30 DSPs)
 *
 * Sources:
 *   - http://ohlandl.ipv7.net/video/Iris.html
 *   - https://www.4crawler.com/IrisVision/index.shtml
 *   - http://archive.irix.cc/sgistuff/hardware/graphics/eclipse.html
 *
 * TODO:
 *   - host dma
 *   - display registers
 *   - save state
 *   - slotify (SGI, MCA, ISA)
 *   - separate raster and display systems?
 */
/*
 * Irix 4.0.5 IDE WIP
 *
 * usage: buffon; ge5load; <test>; gr_exit; buffoff
 *
 * these diagnostics fail (all others pass):
 *   bitp  - options not detected
 *   ctl2  - graphics strobe
 *   gedma - ge5 failed to finish
 *   re    - fails line drawing test
 *   redma - ge5 failed to finish
 */

#include "emu.h"
#include "sgi_gr1.h"

#define LOG_GENERAL (1U << 0)
#define LOG_READS   (1U << 1)

//#define VERBOSE (LOG_GENERAL)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(SGI_GR1, sgi_gr1_device, "sgi_gr1", "SGI GR1 Graphics")

sgi_gr1_device::sgi_gr1_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SGI_GR1, tag, owner, clock)
	, m_bank(*this, "bank")
	, m_screen(*this, "screen")
	, m_ge(*this, "ge5")
	, m_re(*this, "re2")
	, m_xmap(*this, "xmap%u", 0U)
	, m_cursor(*this, "cursor%u", 0U)
	, m_ramdac(*this, "ramdac%u", 0U)
	, m_int_fifo_cb(*this)
{
}

static INPUT_PORTS_START(sgi_gr1)
	PORT_START("options")
	PORT_CONFNAME(0x08, 0x08, "Turbo")
	PORT_CONFSETTING(0x00, DEF_STR(Yes))
	PORT_CONFSETTING(0x08, DEF_STR(No))
	PORT_CONFNAME(0x10, 0x00, "Z Buffer")
	PORT_CONFSETTING(0x00, DEF_STR(Yes))
	PORT_CONFSETTING(0x10, DEF_STR(No))
INPUT_PORTS_END

void sgi_gr1_device::map(address_map &map)
{
	map(0x0000, 0x7fff).m(m_bank, FUNC(address_map_bank_device::amap32));
}

void sgi_gr1_device::map_bank(address_map &map)
{
	// bit 15 of the map offset represents mar_msb
	map(0x0000, 0x03ff).rw(m_ge, FUNC(sgi_ge5_device::code_r<false>), FUNC(sgi_ge5_device::code_w<false>));
	map(0x8000, 0x83ff).rw(m_ge, FUNC(sgi_ge5_device::code_r<true>), FUNC(sgi_ge5_device::code_w<true>));

	map(0x8400, 0x841f).rw(m_xmap[0], FUNC(sgi_xmap2_device::reg_r), FUNC(sgi_xmap2_device::reg_w)).umask32(0x000000ff);
	map(0x8420, 0x843f).rw(m_xmap[1], FUNC(sgi_xmap2_device::reg_r), FUNC(sgi_xmap2_device::reg_w)).umask32(0x000000ff);
	map(0x8440, 0x845f).rw(m_xmap[2], FUNC(sgi_xmap2_device::reg_r), FUNC(sgi_xmap2_device::reg_w)).umask32(0x000000ff);
	map(0x8460, 0x847f).rw(m_xmap[3], FUNC(sgi_xmap2_device::reg_r), FUNC(sgi_xmap2_device::reg_w)).umask32(0x000000ff);
	map(0x8480, 0x849f).rw(m_xmap[4], FUNC(sgi_xmap2_device::reg_r), FUNC(sgi_xmap2_device::reg_w)).umask32(0x000000ff);
	map(0x84a0, 0x84bf).lw8(
			[this](offs_t offset, u8 data)
			{
				for (sgi_xmap2_device *xmap : m_xmap)
					xmap->reg_w(offset, data);
			}, "xmap_broadcast").umask32(0x000000ff);

	map(0x84c0, 0x84c3).rw(FUNC(sgi_gr1_device::dr1_r), FUNC(sgi_gr1_device::dr1_w)).umask32(0xff000000);
	map(0x84e0, 0x84e3).rw(FUNC(sgi_gr1_device::dr0_r), FUNC(sgi_gr1_device::dr0_w)).umask32(0xff000000);

	map(0x8500, 0x850f).m(m_ramdac[0], FUNC(bt457_device::map)).umask32(0x000000ff);
	map(0x8520, 0x852f).m(m_ramdac[1], FUNC(bt457_device::map)).umask32(0x000000ff);
	map(0x8540, 0x854f).m(m_ramdac[2], FUNC(bt457_device::map)).umask32(0x000000ff);

	map(0x8560, 0x856f).m(m_cursor[0], FUNC(bt431_device::map)).umask32(0x000000ff);
	map(0x8580, 0x858f).m(m_cursor[1], FUNC(bt431_device::map)).umask32(0x000000ff);

	map(0x85a0, 0x85a3).rw(FUNC(sgi_gr1_device::dr4_r), FUNC(sgi_gr1_device::dr4_w)).umask32(0xff000000);
	map(0x85c0, 0x85c3).rw(FUNC(sgi_gr1_device::dr3_r), FUNC(sgi_gr1_device::dr3_w)).umask32(0xff000000);
	map(0x85e0, 0x85e3).rw(FUNC(sgi_gr1_device::dr2_r), FUNC(sgi_gr1_device::dr2_w)).umask32(0xff000000);

	map(0x8640, 0x8783).w(m_ge, FUNC(sgi_ge5_device::command_w)).umask32(0xffff0000);
	map(0x8740, 0x8743).r(m_ge, FUNC(sgi_ge5_device::pc_r)).umask32(0xffff0000);

	map(0x0800, 0x0bff).r(m_ge, FUNC(sgi_ge5_device::buffer_r)).mirror(0x8000);
	map(0x0800, 0x0bff).w(FUNC(sgi_gr1_device::fifo_w)).mirror(0x8000);
	map(0x0c00, 0x0dff).w(m_ge, FUNC(sgi_ge5_device::mar_w)).mirror(0x8000);
	map(0x0e00, 0x0e07).lw32([this](offs_t offset, u32 data) { m_bank->set_bank(offset); }, "mar_msb").mirror(0x8000);

	map(0x1400, 0x17ff).rw(m_ge, FUNC(sgi_ge5_device::data_r), FUNC(sgi_ge5_device::data_w));
	map(0x2000, 0x2007).rw(m_ge, FUNC(sgi_ge5_device::finish_r), FUNC(sgi_ge5_device::finish_w)).mirror(0x8000);

	//map(0x207c, 0x207f); // gr1 vs gr2
}

void sgi_gr1_device::device_add_mconfig(machine_config &config)
{
	unsigned pixel_clock = 107'352'000;

	ADDRESS_MAP_BANK(config, m_bank).set_map(&sgi_gr1_device::map_bank).set_options(ENDIANNESS_NATIVE, 32, 32, 0x8000);

	/*
	 * 1280x1024 @ 60Hz
	 * 107.352MHz pixel clock
	 * horizontal sync 63.9kHz, pulse width == 120 pixels
	 * vertical sync 60Hz, front/sync/back == 3/3/35 lines
	 *
	 // SGI GR1 offsets:
	 // 1280: Cx:D+H-P=0+246-57, Cy:V-32=39-32 (189, 7)
	 // 1024: Cx:D+H-P=0+126-57, Cy:V-32=43-32
	 // NTSC: Cx:D+H-P=-6, Cy:V-32=-554
	 //  PAL: Cx:D+H-P=24, Cy:V-32=-457
	 // 30HZ: Cx:D+H-P=39, Cy:V-32=18
	 // STEREO: Cx:D+H-P=0+246-57, Cy:V-32=39-32+1
	 *
	 */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(pixel_clock, 1680, 246, 246 + 1280, 1065, 39, 39 + 1024);
	m_screen->set_screen_update(m_re.finder_tag(), FUNC(sgi_re2_device::screen_update));

	SGI_GE5(config, m_ge, 10_MHz_XTAL);
	m_ge->fifo_empty().set([this]() { return int(m_fifo.empty()); });
	m_ge->fifo_read().set(FUNC(sgi_gr1_device::fifo_r));
	m_ge->re_r().set(m_re, FUNC(sgi_re2_device::reg_r));
	m_ge->re_w().set(m_re, FUNC(sgi_re2_device::reg_w));

	SGI_RE2(config, m_re, 0);
	m_re->out_rdy().set(m_ge, FUNC(sgi_ge5_device::re_rdy_w));
	m_re->out_drq().set(m_ge, FUNC(sgi_ge5_device::re_drq_w));

	SGI_XMAP2(config, m_xmap[0], pixel_clock / 5);
	SGI_XMAP2(config, m_xmap[1], pixel_clock / 5);
	SGI_XMAP2(config, m_xmap[2], pixel_clock / 5);
	SGI_XMAP2(config, m_xmap[3], pixel_clock / 5);
	SGI_XMAP2(config, m_xmap[4], pixel_clock / 5);

	BT431(config, m_cursor[0], pixel_clock / 5);
	BT431(config, m_cursor[1], pixel_clock / 5);

	BT457(config, m_ramdac[0], pixel_clock);
	BT457(config, m_ramdac[1], pixel_clock);
	BT457(config, m_ramdac[2], pixel_clock);
}


ioport_constructor sgi_gr1_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(sgi_gr1);
}

void sgi_gr1_device::device_start()
{
	m_int_fifo_cb.resolve_safe();

	//save_item(NAME());

	m_reset = true;
}

void sgi_gr1_device::device_reset()
{
	m_bank->set_bank(0);

	m_dr0 = DR0_GRF1EN | DR0_SMALLMON0;
	m_dr1 = DR1_TURBO;
	//m_dr2 = 0;
	//m_dr3 = DR3_FIFOFULL;
	m_dr4 = DR4_MEGOPT;

	m_fifo.clear();
	m_int_fifo_cb(CLEAR_LINE);
}

u8 sgi_gr1_device::dr0_r()
{
	LOG("dr0_r 0x%02x (%s)\n", m_dr0, machine().describe_context());

	return m_dr0;
}

u8 sgi_gr1_device::dr1_r()
{
	LOG("dr1_r 0x%02x (%s)\n", m_dr1, machine().describe_context());

	return m_dr1;
}

u8 sgi_gr1_device::dr2_r()
{
	LOG("dr2_r 0x%02x (%s)\n", m_dr2, machine().describe_context());

	return m_dr2;
}

u8 sgi_gr1_device::dr3_r()
{
	LOG("dr3_r 0x%02x (%s)\n", m_dr3, machine().describe_context());

	return m_dr3;
}

u8 sgi_gr1_device::dr4_r()
{
	LOG("dr4_r 0x%02x (%s)\n", m_dr4, machine().describe_context());

	return (m_dr4 | (m_ge->suspended() ? 0 : DR4_GESTALL)) & DR4_RM;
}

void sgi_gr1_device::dr0_w(u8 data)
{
	LOG("dr0_w 0x%02x (%s)\n", data, machine().describe_context());

	m_dr0 = (m_dr0 & ~DR0_WM) | (data & DR0_WM);
}

void sgi_gr1_device::dr1_w(u8 data)
{
	LOG("dr1_w 0x%02x (%s)\n", data, machine().describe_context());

	m_dr1 = (m_dr1 & ~DR1_WM) | (data & DR1_WM);

	m_ge->cwen_w(BIT(data, 1));
}

void sgi_gr1_device::dr2_w(u8 data)
{
	LOG("dr2_w 0x%02x (%s)\n", data, machine().describe_context());

	m_dr2 = (m_dr2 & ~DR2_WM) | (data & DR2_WM);
}

void sgi_gr1_device::dr3_w(u8 data)
{
	LOG("dr3_w 0x%02x (%s)\n", data, machine().describe_context());

	m_dr3 = (m_dr3 & ~DR3_WM) | (data & DR3_WM);
}

void sgi_gr1_device::dr4_w(u8 data)
{
	LOG("dr4_w 0x%02x (%s)\n", data, machine().describe_context());

	m_dr4 = (m_dr4 & ~DR4_WM) | (data & DR4_WM);

	for (sgi_xmap2_device *xmap : m_xmap)
		xmap->map_select_w(m_dr4 & DR4_MS);
}

u64 sgi_gr1_device::fifo_r()
{
	u64 data = m_fifo.dequeue();

	if (m_fifo.empty())
		m_dr3 &= DR3_FIFOEMPTY;

	if (!(m_dr3 & DR3_FIFOFULL) && (m_fifo.queue_length() <= 256))
	{
		m_dr3 |= DR3_FIFOFULL;
		m_int_fifo_cb(CLEAR_LINE);
	}

	return data;
}

void sgi_gr1_device::fifo_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOG("fifo_w 0x%010x (%s)\n", (u64(offset) << 32) | data, machine().describe_context());

	m_fifo.enqueue((u64(offset) << 32) | data);

	if (!(m_dr3 & DR3_FIFOEMPTY))
		m_dr3 |= DR3_FIFOEMPTY;

	if ((m_dr3 & DR3_FIFOFULL) && (m_fifo.queue_length() > 256))
	{
		m_dr3 &= ~DR3_FIFOFULL;
		m_int_fifo_cb(ASSERT_LINE);
	}
}

void sgi_gr1_device::reset_w(int state)
{
	if (!m_reset && !state)
	{
		LOG("reset_w %d (%s)\n", state, machine().describe_context());
		m_ge->pulse_input_line(INPUT_LINE_RESET, attotime::from_ticks(1, 10_MHz_XTAL));
		m_re->reset();
	}

	m_reset = !state;
}
