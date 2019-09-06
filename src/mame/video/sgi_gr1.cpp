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
 *   - ge5/hq1
 *   - raster engine
 *   - dma
 *   - slotify (SGI, MCA, ISA)
 *   - everything else
 */

#include "emu.h"
#include "sgi_gr1.h"

#define LOG_GENERAL (1U << 0)
#define LOG_READS   (1U << 1)

//#define VERBOSE (LOG_GENERAL)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(SGI_GR11, sgi_gr11_device, "sgi_gr11", "SGI GR1.1 Graphics")
DEFINE_DEVICE_TYPE(SGI_GR12, sgi_gr12_device, "sgi_gr12", "SGI GR1.2 Graphics")

sgi_gr1_device::sgi_gr1_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_screen(*this, "screen")
	, m_re(*this, "re2")
	, m_ge(*this, "ge5")
	, m_cursor(*this, "cursor%u", 0U)
	, m_vblank_cb(*this)
	, m_int_fifo_cb(*this)
{
}

sgi_gr11_device::sgi_gr11_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: sgi_gr1_device(mconfig, SGI_GR11, tag, owner, clock)
	, m_ramdac(*this, "ramdac")
{
}

sgi_gr12_device::sgi_gr12_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: sgi_gr1_device(mconfig, SGI_GR12, tag, owner, clock)
	, m_ramdac(*this, "ramdac%u", 0U)
{
}

void sgi_gr1_device::map(address_map &map)
{
	// TODO: bank based on mar_msb

	map(0x0000, 0x03ff).rw(m_ge, FUNC(sgi_ge5_device::code_r), FUNC(sgi_ge5_device::code_w));

	map(0x04c0, 0x04c3).rw(FUNC(sgi_gr1_device::dr1_r), FUNC(sgi_gr1_device::dr1_w)).umask32(0xff000000);
	map(0x04e0, 0x04e3).rw(FUNC(sgi_gr1_device::dr0_r), FUNC(sgi_gr1_device::dr0_w)).umask32(0xff000000);

	map(0x0560, 0x056f).m(m_cursor[0], FUNC(bt431_device::map)).umask32(0xff000000);
	map(0x0580, 0x058f).m(m_cursor[1], FUNC(bt431_device::map)).umask32(0xff000000);

	map(0x05a0, 0x05a3).rw(FUNC(sgi_gr1_device::dr4_r), FUNC(sgi_gr1_device::dr4_w)).umask32(0xff000000);
	map(0x05c0, 0x05c3).rw(FUNC(sgi_gr1_device::dr3_r), FUNC(sgi_gr1_device::dr3_w)).umask32(0xff000000);
	map(0x05e0, 0x05e3).rw(FUNC(sgi_gr1_device::dr2_r), FUNC(sgi_gr1_device::dr2_w)).umask32(0xff000000);

	map(0x0640, 0x783).w(m_ge, FUNC(sgi_ge5_device::command_w)).umask32(0xffff0000);
	map(0x0740, 0x743).r(m_ge, FUNC(sgi_ge5_device::pc_r)).umask32(0xffff0000);

	map(0x0800, 0x0bff).w(FUNC(sgi_gr1_device::fifo_w));
	map(0x0c00, 0x0dff).w(m_ge, FUNC(sgi_ge5_device::mar_w));
	map(0x0e00, 0x0e07).w(m_ge, FUNC(sgi_ge5_device::mar_msb_w));

	map(0x1400, 0x17ff).rw(m_ge, FUNC(sgi_ge5_device::data_r), FUNC(sgi_ge5_device::data_w));
	map(0x2000, 0x2007).lrw32("ff",
		[this](offs_t offset) { LOG("read finish flag %d\n", offset); return m_ff[offset]; },
		[this](offs_t offset, u32 data, u32 mem_mask) { LOG("write finish flag %d data %d\n", offset, data); m_ff[offset] = data; });
	//map(0x207c, 0x207f); // gr1 vs gr2
}

void sgi_gr11_device::map(address_map &map)
{
	sgi_gr1_device::map(map);

	map(0x0510, 0x0517).rw(FUNC(sgi_gr11_device::xpc1_r<0>), FUNC(sgi_gr11_device::xpc1_w<0>)).umask32(0x000000ff);
	map(0x0530, 0x0537).rw(FUNC(sgi_gr11_device::xpc1_r<1>), FUNC(sgi_gr11_device::xpc1_w<1>)).umask32(0x000000ff);
	map(0x0550, 0x0557).rw(FUNC(sgi_gr11_device::xpc1_r<2>), FUNC(sgi_gr11_device::xpc1_w<2>)).umask32(0x000000ff);
	map(0x0570, 0x0577).rw(FUNC(sgi_gr11_device::xpc1_r<3>), FUNC(sgi_gr11_device::xpc1_w<3>)).umask32(0x000000ff);
	map(0x0590, 0x0597).rw(FUNC(sgi_gr11_device::xpc1_r<4>), FUNC(sgi_gr11_device::xpc1_w<4>)).umask32(0x000000ff);

	map(0x05b0, 0x05b7).w(FUNC(sgi_gr11_device::xpc1_bc_w)).umask32(0x000000ff);

	map(0x05d0, 0x05df).m(m_ramdac, FUNC(bt458_device::map));
}

void sgi_gr12_device::map(address_map &map)
{
	sgi_gr1_device::map(map);

	map(0x0400, 0x041f).rw(FUNC(sgi_gr12_device::xmap2_r<0>), FUNC(sgi_gr12_device::xmap2_w<0>)).umask32(0x000000ff);
	map(0x0420, 0x043f).rw(FUNC(sgi_gr12_device::xmap2_r<1>), FUNC(sgi_gr12_device::xmap2_w<1>)).umask32(0x000000ff);
	map(0x0440, 0x045f).rw(FUNC(sgi_gr12_device::xmap2_r<2>), FUNC(sgi_gr12_device::xmap2_w<2>)).umask32(0x000000ff);
	map(0x0460, 0x047f).rw(FUNC(sgi_gr12_device::xmap2_r<3>), FUNC(sgi_gr12_device::xmap2_w<3>)).umask32(0x000000ff);
	map(0x0480, 0x049f).rw(FUNC(sgi_gr12_device::xmap2_r<4>), FUNC(sgi_gr12_device::xmap2_w<4>)).umask32(0x000000ff);
	map(0x04a0, 0x04bf).w(FUNC(sgi_gr12_device::xmap2_bc_w)).umask32(0x000000ff);

	map(0x0500, 0x050f).m(m_ramdac[0], FUNC(bt457_device::map)).umask32(0x000000ff);
	map(0x0520, 0x052f).m(m_ramdac[1], FUNC(bt457_device::map)).umask32(0x000000ff);
	map(0x0540, 0x054f).m(m_ramdac[2], FUNC(bt457_device::map)).umask32(0x000000ff);
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

	SGI_RE2(config, m_re, 0);

	SGI_GE5(config, m_ge, 10_MHz_XTAL);
	m_ge->fifo_empty().set([this]() { return int(m_fifo.empty()); });
	m_ge->fifo_read().set([this]() { return m_fifo.dequeue(); });
	m_ge->re_r().set(m_re, FUNC(sgi_re2_device::reg_r));
	m_ge->re_w().set(m_re, FUNC(sgi_re2_device::reg_w));

	BT431(config, m_cursor[0], pixel_clock / 5);
	BT431(config, m_cursor[1], pixel_clock / 5);
}

void sgi_gr11_device::device_add_mconfig(machine_config &config)
{
	sgi_gr1_device::device_add_mconfig(config);

	BT458(config, m_ramdac, 107'352'000);
}

void sgi_gr12_device::device_add_mconfig(machine_config &config)
{
	sgi_gr1_device::device_add_mconfig(config);

	BT457(config, m_ramdac[0], 107'352'000);
	BT457(config, m_ramdac[1], 107'352'000);
	BT457(config, m_ramdac[2], 107'352'000);
}

void sgi_gr1_device::device_start()
{
	m_vblank_cb.resolve_safe();
	m_int_fifo_cb.resolve_safe();

	//save_item(NAME());
}

void sgi_gr11_device::device_start()
{
	sgi_gr1_device::device_start();
}

void sgi_gr12_device::device_start()
{
	sgi_gr1_device::device_start();

	m_vram = std::make_unique<u32[]>(1280 * 1024);
}

void sgi_gr1_device::device_reset()
{
	//m_dr0 = DR0_ZBIN;
	//m_dr2 = DR2_BPIN;
	//m_dr3 = DR3_MS;
	m_dr4 = 0;

	m_fifo.clear();
}

void sgi_gr11_device::device_reset()
{
	sgi_gr1_device::device_reset();
}

void sgi_gr12_device::device_reset()
{
	sgi_gr1_device::device_reset();

	//m_dr0 &= ~DR0_ZBIN;
	//m_dr2 &= ~DR2_BPIN;
}

u32 sgi_gr11_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect)
{
	/*
	 * fb_pixel (8) bits, cursor (2 bits), pup (2 bits), wid (2 bits)
	 *
	 * iterate through 5 xpc1 channels
	 *   mode = xpc1[channel][wid]
	 *
	 */

	return 0;
}

u32 sgi_gr12_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect)
{
	// TODO: underlay

	unsigned offset = 0;

	for (unsigned y = screen.visible_area().min_y; y <= screen.visible_area().max_y; y++)
		for (unsigned x = screen.visible_area().min_x; x <= screen.visible_area().max_x; x += 5)
			for (unsigned c = 0; c < 5; c++)
			{
				u8 const overlay = (m_cursor[0]->cur_r(x + c, y) ? 1 : 0) | (m_cursor[1]->cur_r(x + c, y) ? 2 : 0);
				u32 const data = m_vram[offset++];
				u16 const mode = m_xmap2[c].mode[data >> 28];

				switch (mode & 0x07)
				{
				case 0: // 8 bit ci single buffer
					{
						u16 const ci = BIT(mode, 9) ? (mode & 0x0f00) | u8(data) : u8(data);
						rgb_t const cm = m_xmap2[c].color[(m_dr4 & DR4_MS) ? ci | 0x1000 : ci];

						bitmap.pix(y, x) = rgb_t(
							m_ramdac[0]->lookup(cm.r(), overlay),
							m_ramdac[1]->lookup(cm.g(), overlay),
							m_ramdac[2]->lookup(cm.b(), overlay));
					}
					break;

				case 1: // 4 bit ci double buffer
					{
						u8 const fb = BIT(mode, 3) ? u8(data) >> 4 : data & 0x0f;
						u16 const ci = BIT(mode, 9) ? (mode & 0x0f00) | fb : fb;
						rgb_t const cm = m_xmap2[c].color[(m_dr4 & DR4_MS) ? ci | 0x1000 : ci];

						bitmap.pix(y, x) = rgb_t(
							m_ramdac[0]->lookup(cm.r(), overlay),
							m_ramdac[1]->lookup(cm.g(), overlay),
							m_ramdac[2]->lookup(cm.b(), overlay));
					}
					break;

				case 2: // 12 bit ci double buffer
					{
						u8 const fb = u16(BIT(mode, 3) ? data >> 12 : data) & 0x0fff;
						u16 const ci = BIT(mode, 9) ? (mode & 0x0f00) | (fb & 0xff) : fb;
						rgb_t const cm = m_xmap2[c].color[(m_dr4 & DR4_MS) ? ci | 0x1000 : ci];

						bitmap.pix(y, x) = rgb_t(
							m_ramdac[0]->lookup(cm.r(), overlay),
							m_ramdac[1]->lookup(cm.g(), overlay),
							m_ramdac[2]->lookup(cm.b(), overlay));
					}
					break;

				case 4: // 24 bit rgb single buffer
					bitmap.pix(y, x) = rgb_t(
						m_ramdac[0]->lookup(u8(data >> 0), overlay),
						m_ramdac[1]->lookup(u8(data >> 8), overlay),
						m_ramdac[2]->lookup(u8(data >> 16), overlay));
					break;

				case 5: // 12 bit rgb double buffer
					bitmap.pix(y, x) = BIT(mode, 3) ?
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

void sgi_gr1_device::fifo_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (m_fifo.full())
		fatalerror("fifo_w: fifo overflow (%s)", machine().describe_context().c_str());

	LOG("fifo_w 0x%010x\n", (u64(offset) << 32) | data);

	m_fifo.enqueue((u64(offset) << 32) | data);

	if (m_fifo.queue_length() > 256)
		m_int_fifo_cb(ASSERT_LINE);

	//if (m_ge->suspended())
	//  m_ge->resume(SUSPEND_REASON_HALT);
}

void sgi_gr1_device::reset_w(int state)
{
	if (state)
	{
		LOG("reset_w %d\n", state);
		m_ge->pulse_input_line(INPUT_LINE_RESET, attotime::from_ticks(1, 10_MHz_XTAL));
	}
}

template <unsigned Channel> u8 sgi_gr12_device::xmap2_r(offs_t offset)
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
			return 0x18; // z-buffer and bit-plane expansion not present?
		break;

	case 6: // address msb
		return x.addr >> 8;

	case 7: // address lsb
		return u8(x.addr);
	}

	return 0;
}

template <unsigned Channel> void sgi_gr12_device::xmap2_w(offs_t offset, u8 data)
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
