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
 *   - refactor display subsystem
 *   - display registers
 *   - save state
 *   - slotify (SGI, MCA, ISA)
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
	, m_screen(*this, "screen")
	, m_re(*this, "re2")
	, m_ge(*this, "ge5")
	, m_cursor(*this, "cursor%u", 0U)
	, m_ramdac(*this, "ramdac%u", 0U)
	, m_vblank_cb(*this)
	, m_int_fifo_cb(*this)
{
}

void sgi_gr1_device::map(address_map &map)
{
	// TODO: bank based on mar_msb

	map(0x0000, 0x03ff).rw(m_ge, FUNC(sgi_ge5_device::code_r), FUNC(sgi_ge5_device::code_w));

	map(0x0400, 0x041f).rw(FUNC(sgi_gr1_device::xmap2_r<0>), FUNC(sgi_gr1_device::xmap2_w<0>)).umask32(0x000000ff);
	map(0x0420, 0x043f).rw(FUNC(sgi_gr1_device::xmap2_r<1>), FUNC(sgi_gr1_device::xmap2_w<1>)).umask32(0x000000ff);
	map(0x0440, 0x045f).rw(FUNC(sgi_gr1_device::xmap2_r<2>), FUNC(sgi_gr1_device::xmap2_w<2>)).umask32(0x000000ff);
	map(0x0460, 0x047f).rw(FUNC(sgi_gr1_device::xmap2_r<3>), FUNC(sgi_gr1_device::xmap2_w<3>)).umask32(0x000000ff);
	map(0x0480, 0x049f).rw(FUNC(sgi_gr1_device::xmap2_r<4>), FUNC(sgi_gr1_device::xmap2_w<4>)).umask32(0x000000ff);
	map(0x04a0, 0x04bf).w(FUNC(sgi_gr1_device::xmap2_bc_w)).umask32(0x000000ff);

	map(0x04c0, 0x04c3).rw(FUNC(sgi_gr1_device::dr1_r), FUNC(sgi_gr1_device::dr1_w)).umask32(0xff000000);
	map(0x04e0, 0x04e3).rw(FUNC(sgi_gr1_device::dr0_r), FUNC(sgi_gr1_device::dr0_w)).umask32(0xff000000);

	map(0x0500, 0x050f).m(m_ramdac[0], FUNC(bt457_device::map)).umask32(0x000000ff);
	map(0x0520, 0x052f).m(m_ramdac[1], FUNC(bt457_device::map)).umask32(0x000000ff);
	map(0x0540, 0x054f).m(m_ramdac[2], FUNC(bt457_device::map)).umask32(0x000000ff);

	map(0x0560, 0x056f).m(m_cursor[0], FUNC(bt431_device::map)).umask32(0x000000ff);
	map(0x0580, 0x058f).m(m_cursor[1], FUNC(bt431_device::map)).umask32(0x000000ff);

	map(0x05a0, 0x05a3).rw(FUNC(sgi_gr1_device::dr4_r), FUNC(sgi_gr1_device::dr4_w)).umask32(0xff000000);
	map(0x05c0, 0x05c3).rw(FUNC(sgi_gr1_device::dr3_r), FUNC(sgi_gr1_device::dr3_w)).umask32(0xff000000);
	map(0x05e0, 0x05e3).rw(FUNC(sgi_gr1_device::dr2_r), FUNC(sgi_gr1_device::dr2_w)).umask32(0xff000000);

	map(0x0640, 0x783).w(m_ge, FUNC(sgi_ge5_device::command_w)).umask32(0xffff0000);
	map(0x0740, 0x743).r(m_ge, FUNC(sgi_ge5_device::pc_r)).umask32(0xffff0000);

	map(0x0800, 0x0bff).rw(FUNC(sgi_gr1_device::fifo_r), FUNC(sgi_gr1_device::fifo_w));
	map(0x0c00, 0x0dff).w(m_ge, FUNC(sgi_ge5_device::mar_w));
	map(0x0e00, 0x0e07).w(m_ge, FUNC(sgi_ge5_device::mar_msb_w));

	map(0x1400, 0x17ff).rw(m_ge, FUNC(sgi_ge5_device::data_r), FUNC(sgi_ge5_device::data_w));
	map(0x2000, 0x2007).rw(m_ge, FUNC(sgi_ge5_device::finish_r), FUNC(sgi_ge5_device::finish_w));

	//map(0x207c, 0x207f); // gr1 vs gr2
}

void sgi_gr1_device::device_add_mconfig(machine_config &config)
{
	unsigned pixel_clock = 107'352'000;

	//ADDRESS_MAP_BANK(config, m_ext).set_map(&lle_device_base::ext_map).set_options(ENDIANNESS_NATIVE, 8, 12, 0x100);

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
	m_screen->set_screen_update(FUNC(sgi_gr1_device::screen_update));
	m_screen->screen_vblank().set([this](int state) { m_vblank_cb(state); });


	SGI_GE5(config, m_ge, 10_MHz_XTAL);
	m_ge->fifo_empty().set([this]() { return int(m_fifo.empty()); });
	m_ge->fifo_read().set(FUNC(sgi_gr1_device::ge_fifo_r));
	m_ge->re_r().set(m_re, FUNC(sgi_re2_device::reg_r));
	m_ge->re_w().set(m_re, FUNC(sgi_re2_device::reg_w));

	BT431(config, m_cursor[0], pixel_clock / 5);
	BT431(config, m_cursor[1], pixel_clock / 5);

	SGI_RE2(config, m_re, 0);
	m_re->out_rdy().set(m_ge, FUNC(sgi_ge5_device::re_rdy_w));
	m_re->out_drq().set(m_ge, FUNC(sgi_ge5_device::re_drq_w));
	m_re->vram_r().set(
		[this](offs_t offset)
		{
			return m_vram[offset];
		});
	m_re->vram_w().set(
		[this](offs_t offset, u32 data, u32 mem_mask)
		{
			COMBINE_DATA(&m_vram[offset]);
		});

	BT457(config, m_ramdac[0], pixel_clock);
	BT457(config, m_ramdac[1], pixel_clock);
	BT457(config, m_ramdac[2], pixel_clock);
}

void sgi_gr1_device::device_start()
{
	m_vblank_cb.resolve_safe();
	m_int_fifo_cb.resolve_safe();

	//save_item(NAME());

	m_vram = std::make_unique<u32[]>(1280 * 1024);
	m_dram = std::make_unique<u32[]>(1280 * 1024);

	m_reset = true;
}

void sgi_gr1_device::device_reset()
{
	m_dr0 = DR0_GRF1EN | DR0_SMALLMON0;
	m_dr1 = DR1_TURBO;
	//m_dr2 = 0;
	//m_dr3 = DR3_FIFOFULL;
	m_dr4 = DR4_MEGOPT;

	m_fifo.clear();
}

u32 sgi_gr1_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect)
{
	// TODO: underlay

	// TODO: variable topscan row and column
	for (unsigned screen_y = screen.visible_area().min_y, mem_y = 1023; screen_y <= screen.visible_area().max_y; screen_y++, mem_y--)
		for (unsigned screen_x = screen.visible_area().min_x, mem_x = 0; screen_x <= screen.visible_area().max_x; screen_x++, mem_x++)
		{
			unsigned const offset = (mem_y * 0x500) + mem_x;
			unsigned const bank = mem_x % 5;

			u32 const data = m_vram[offset];
			u8 const overlay = (m_cursor[0]->cur_r(screen_x, screen_y) ? 1 : 0) | (m_cursor[1]->cur_r(screen_x, screen_y) ? 2 : 0);
			u16 const mode = m_xmap2[bank].mode[data >> 28];

			switch (mode & 0x07)
			{
			case 0: // 8 bit ci single buffer
			{
				u16 const ci = BIT(mode, 9) ? (mode & 0x0f00) | u8(data) : u8(data);
				rgb_t const cm = m_xmap2[bank].color[(m_dr4 & DR4_MS) ? ci | 0x1000 : ci];

				bitmap.pix(screen_y, screen_x) = rgb_t(
					m_ramdac[0]->lookup(cm.r(), overlay),
					m_ramdac[1]->lookup(cm.g(), overlay),
					m_ramdac[2]->lookup(cm.b(), overlay));
			}
			break;

			case 1: // 4 bit ci double buffer
			{
				u8 const fb = BIT(mode, 3) ? u8(data) >> 4 : data & 0x0f;
				u16 const ci = BIT(mode, 9) ? (mode & 0x0f00) | fb : fb;
				rgb_t const cm = m_xmap2[bank].color[(m_dr4 & DR4_MS) ? ci | 0x1000 : ci];

				bitmap.pix(screen_y, screen_x) = rgb_t(
					m_ramdac[0]->lookup(cm.r(), overlay),
					m_ramdac[1]->lookup(cm.g(), overlay),
					m_ramdac[2]->lookup(cm.b(), overlay));
			}
			break;

			case 2: // 12 bit ci double buffer
			{
				u8 const fb = u16(BIT(mode, 3) ? data >> 12 : data) & 0x0fff;
				u16 const ci = BIT(mode, 9) ? (mode & 0x0f00) | (fb & 0xff) : fb;
				rgb_t const cm = m_xmap2[bank].color[(m_dr4 & DR4_MS) ? ci | 0x1000 : ci];

				bitmap.pix(screen_y, screen_x) = rgb_t(
					m_ramdac[0]->lookup(cm.r(), overlay),
					m_ramdac[1]->lookup(cm.g(), overlay),
					m_ramdac[2]->lookup(cm.b(), overlay));
			}
			break;

			case 4: // 24 bit rgb single buffer
				bitmap.pix(screen_y, screen_x) = rgb_t(
					m_ramdac[0]->lookup(u8(data >> 0), overlay),
					m_ramdac[1]->lookup(u8(data >> 8), overlay),
					m_ramdac[2]->lookup(u8(data >> 16), overlay));
				break;

			case 5: // 12 bit rgb double buffer
				bitmap.pix(screen_y, screen_x) = BIT(mode, 3) ?
					rgb_t(
						m_ramdac[0]->lookup((u8(data >> 0) & 0xf0) | (u8(data >> 4) & 0x0f), overlay),
						m_ramdac[1]->lookup((u8(data >> 8) & 0xf0) | (u8(data >> 12) & 0x0f), overlay),
						m_ramdac[2]->lookup((u8(data >> 16) & 0xf0) | (u8(data >> 20) & 0x0f), overlay)) :
					rgb_t(
						m_ramdac[0]->lookup((u8(data << 4) & 0xf0) | (u8(data >> 0) & 0x0f), overlay),
						m_ramdac[1]->lookup((u8(data >> 4) & 0xf0) | (u8(data >> 8) & 0x0f), overlay),
						m_ramdac[2]->lookup((u8(data >> 12) & 0xf0) | (u8(data >> 16) & 0x0f), overlay));
				break;
			}
		}

	return 0;
}

u64 sgi_gr1_device::ge_fifo_r()
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
	LOG("fifo_w 0x%010x\n", (u64(offset) << 32) | data);

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

template <unsigned Channel> u8 sgi_gr1_device::xmap2_r(offs_t offset)
{
	xmap2 const x = m_xmap2[Channel];

	switch (offset)
	{
	case 0: // nop
		break;

	case 1: // blue data
		if (x.addr & 0x1000)
			return x.color[(m_dr4 & DR4_MS) ? x.addr : (x.addr & 0xfff)].b();
		else if (x.addr < 0x10)
			return x.overlay[x.addr].b();
		break;

	case 2: // green data
		if (x.addr & 0x1000)
			return x.color[(m_dr4 & DR4_MS) ? x.addr : (x.addr & 0xfff)].g();
		else if (x.addr < 0x10)
			return x.overlay[x.addr].g();
		break;

	case 3: // red data
		if (x.addr & 0x1000)
			return x.color[(m_dr4 & DR4_MS) ? x.addr : (x.addr & 0xfff)].r();
		else if (x.addr < 0x10)
			return x.overlay[x.addr].r();
		break;

	case 4: // increment address
		// TODO: should reading increment the address register?
		//x.addr = (x.addr + 1) & 0x1fff;
		LOG("read address increment\n");
		break;

	case 5: // other data
		if (x.addr < 0x20)
		{
			u16 const mode = x.mode[(x.addr >> 1) & 0xf];

			return BIT(x.addr, 0) ? (mode >> 8) : u8(mode);
		}
		else if (x.addr == 0x20)
			return x.wid_aux;
		else if (x.addr == 0x21)
			return 0x0; // 0x08==no turbo option, 0x10==no z buffer
		break;

	case 6: // address msb
		return x.addr >> 8;

	case 7: // address lsb
		return u8(x.addr);
	}

	return 0;
}

template <unsigned Channel> void sgi_gr1_device::xmap2_w(offs_t offset, u8 data)
{
	xmap2 &x = m_xmap2[Channel];

	switch (offset)
	{
	case 0: // nop
		break;

	case 1: // blue data
		if (x.addr & 0x1000)
			x.color[(m_dr4 & DR4_MS) ? x.addr : (x.addr & 0xfff)].set_b(data);
		else if (x.addr < 0x10)
			x.overlay[x.addr].set_b(data);
		break;

	case 2: // green data
		if (x.addr & 0x1000)
			x.color[(m_dr4 & DR4_MS) ? x.addr : (x.addr & 0xfff)].set_g(data);
		else if (x.addr < 0x10)
			x.overlay[x.addr].set_g(data);
		break;

	case 3: // red data
		if (x.addr & 0x1000)
			x.color[(m_dr4 & DR4_MS) ? x.addr : (x.addr & 0xfff)].set_r(data);
		else if (x.addr < 0x10)
			x.overlay[x.addr].set_r(data);
		break;

	case 4: // increment address
		x.addr = (x.addr + 1) & 0x1fff;
		break;

	case 5: // other data
		if (x.addr < 0x20)
		{
			u16 &mode = x.mode[(x.addr >> 1) & 0xf];

			if (BIT(x.addr, 0))
				mode = (u16(data & 0x3f) << 8) | (mode & 0x00ff);
			else
				mode = (mode & 0x3f00) | data;
		}
		else if (x.addr == 0x20)
			x.wid_aux = BIT(data, 0);
		break;

	case 6: // address msb
		x.addr = u16((data & 0x1f) << 8) | (x.addr & 0x00ff);
		break;

	case 7: // address lsb
		x.addr = (x.addr & 0x1f00) | data;
		break;
	}
}
