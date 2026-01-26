// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Wilbert Pol

#include "emu.h"
#include "3do_madam.h"

#define LOG_FENCE   (1U << 1)
#define LOG_MMU     (1U << 2)
#define LOG_DMA     (1U << 3)
#define LOG_MULT    (1U << 4)
#define LOG_VDLP    (1U << 5)
#define LOG_CEL     (1U << 6)
#define LOG_REGIS   (1U << 7)

#define VERBOSE (LOG_GENERAL | LOG_MMU | LOG_MULT)
//#define VERBOSE (LOG_VDLP)
//#define VERBOSE (LOG_CEL | LOG_REGIS)
//#define LOG_OUTPUT_FUNC osd_printf_warning

#include "logmacro.h"

#define LOGFENCE(...)   LOGMASKED(LOG_FENCE,   __VA_ARGS__)
#define LOGMMU(...)     LOGMASKED(LOG_MMU,     __VA_ARGS__)
#define LOGDMA(...)     LOGMASKED(LOG_DMA,     __VA_ARGS__)
#define LOGMULT(...)    LOGMASKED(LOG_MULT,    __VA_ARGS__)
#define LOGVDLP(...)    LOGMASKED(LOG_VDLP,    __VA_ARGS__)
#define LOGCEL(...)     LOGMASKED(LOG_CEL,     __VA_ARGS__)
#define LOGREGIS(...)   LOGMASKED(LOG_REGIS,   __VA_ARGS__)

DEFINE_DEVICE_TYPE(MADAM, madam_device, "madam", "3DO MN7A020UDA \"Madam\" Address Decoder")

madam_device::madam_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MADAM, tag, owner, clock)
	, m_amy(*this, finder_base::DUMMY_TAG)
	, m_diag_cb(*this)
	, m_dma8_read_cb(*this, 0)
	, m_dma32_read_cb(*this, 0)
	, m_dma32_write_cb(*this)
	, m_playerbus_read_cb(*this, 0)
	, m_irq_dply_cb(*this)
{
}

void madam_device::device_start()
{
	// bit 31-16 Green Madam (0x0102)
	// bit 23 Internal ARM
	// bit 15-8 manufacturer ID (0x02: MEC -> Panasonic, 0x09: Sanyo, 0x0b: Goldstar)
	// bit 0 wirewrap system
	m_revision = 0x01020200;
	m_msysbits = 0x51;

	m_dma_playerbus_timer = timer_alloc(FUNC(madam_device::dma_playerbus_cb), this);
	m_cel_timer = timer_alloc(FUNC(madam_device::cel_tick_cb), this);

	// max vcnt x max tlhpcnt, for Packed stuff
	// TODO: reduce footprint
	// - a possible Cel this big should tank the system a lot
	// - there's just not enough work RAM in base system
	m_cel.buffer.resize(0x1000*0x800);

	save_item(NAME(m_pip));
	save_item(NAME(m_fence));
	save_item(NAME(m_mmu));
	save_item(NAME(m_mult));
	save_item(NAME(m_dma));

	save_item(NAME(m_msysbits));
	save_item(NAME(m_mctl));
	save_item(NAME(m_sltime));
	save_item(NAME(m_abortbits));
	save_item(NAME(m_privbits));
	save_item(NAME(m_statbits));
	save_item(NAME(m_diag));
	save_item(NAME(m_ccobctl0));
	save_item(NAME(m_ppmpc));
	save_item(NAME(m_regctl0));
	save_item(STRUCT_MEMBER(m_regis, fb_pitch));
	save_item(NAME(m_regctl1));
	save_item(STRUCT_MEMBER(m_regis, xclip));
	save_item(STRUCT_MEMBER(m_regis, yclip));
	save_item(NAME(m_regctl2));
	save_item(NAME(m_regctl3));
	save_item(NAME(m_xyposh));
	save_item(NAME(m_xyposl));
	save_item(NAME(m_linedxyh));
	save_item(NAME(m_linedxyl));
	save_item(NAME(m_dxyh));
	save_item(NAME(m_dxyl));
	save_item(NAME(m_ddxyh));
	save_item(NAME(m_ddxyl));
	save_item(NAME(m_mult_control));
	save_item(NAME(m_mult_status));

	save_item(STRUCT_MEMBER(m_vdlp, address));
	save_item(STRUCT_MEMBER(m_vdlp, scanlines));
	save_item(STRUCT_MEMBER(m_vdlp, modulo));
	save_item(STRUCT_MEMBER(m_vdlp, fb_address));
	save_item(STRUCT_MEMBER(m_vdlp, fetch));
	save_item(STRUCT_MEMBER(m_vdlp, y_dest));
	save_item(STRUCT_MEMBER(m_vdlp, y_src));
	save_item(STRUCT_MEMBER(m_vdlp, link));
	save_item(STRUCT_MEMBER(m_vdlp, video_dma));
}

void madam_device::device_reset()
{
	m_vdlp.address = 0x20'0000;

	m_mctl = 0;
	m_cel.state = IDLE;
	m_statbits = 0;
	m_dma_playerbus_timer->adjust(attotime::never);
	m_cel_timer->adjust(attotime::never);
}

// $0330'0000 base
void madam_device::map(address_map &map)
{
	map(0x0000, 0x0003).lrw32(
		NAME([this] () { return m_revision; }),
		NAME([this] (u32 data) {
			m_diag_cb(data & 0xff);
		})
	);

	// 03300004 - Memory configuration 29 = 2MB DRAM, 1MB VRAM
	map(0x0004, 0x0007).lrw32(
		NAME([this] () { return m_msysbits; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOG("msysbits: %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_msysbits);
		})
	);
	map(0x0008, 0x000b).rw(FUNC(madam_device::mctl_r), FUNC(madam_device::mctl_w));
	map(0x000c, 0x000f).lrw32(
		NAME([this] () { return m_sltime; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOG("sltime: %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_sltime);
		})
	);
	map(0x0020, 0x0023).lrw32(
		NAME([this] () { return m_abortbits; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOG("abortbits: %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_abortbits);
		})
	);
	map(0x0024, 0x0027).lrw32(
		NAME([this] () { return m_privbits; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOG("privbits: %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_privbits);
		})
	);
	map(0x0028, 0x002b).lrw32(
		NAME([this] () { return m_statbits; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOG("statbits: %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_statbits);
		})
	);
	// 0x0040, 0x0047 madam diag 0/1
	map(0x0040, 0x0043).lrw32(
		NAME([this] () { return m_diag; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			// TODO: correct?
			LOG("diag: %08x & %08x (forced to 1)\n", data, mem_mask);
			m_diag = 1;
		})
	);
	// 0x0044: Anvil "feature"

	// CEL engine
	// SPRSTRT - Start the CEL engine (W)
	map(0x0100, 0x0103).w(FUNC(madam_device::cel_start_w));
	// SPRSTOP - Stop the CEL engine (W)
	map(0x0104, 0x0107).w(FUNC(madam_device::cel_stop_w));
//  map(0x0108, 0x010b)  SPRCNTU - Continue the CEL engine (W)
//  map(0x010c, 0x010f)  SPRPAUS - Pause the CEL engine (W)
	map(0x0110, 0x0113).lrw32(
		NAME([this] () { return m_ccobctl0; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOGCEL("ccobtcl0: %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_ccobctl0);
		})
	);
	map(0x0120, 0x0123).lrw32(
		NAME([this] () { return m_ppmpc; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOGCEL("ppmpc: %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_ppmpc);
		})
	);

	// Regis
	map(0x0130, 0x0133).rw(FUNC(madam_device::regctl0_r), FUNC(madam_device::regctl0_w));
	map(0x0134, 0x0137).lrw32(
		NAME([this] () { return m_regctl1; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOGREGIS("regctl1 (X and Y clip values): %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_regctl1);
			m_regis.xclip = m_regctl1 & 0xffff;
			m_regis.yclip = m_regctl1 >> 16;
		})
	);
	map(0x0138, 0x013b).lrw32(
		NAME([this] () { return m_regctl2; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOGREGIS("regctl2 (read base address): %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_regctl2);
		})
	);
	// Write base address
	map(0x013c, 0x013f).lrw32(
		NAME([this] () { return m_regctl3; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOGREGIS("regctl3 (write base address): %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_regctl3);
		})
	);
	// TODO: these are in 16.16 format
	map(0x0140, 0x0143).lrw32(
		NAME([this] () { return m_xyposh; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOGREGIS("xyposh: %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_xyposh);
		})
	);
	map(0x0144, 0x0147).lrw32(
		NAME([this] () { return m_xyposl; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOGREGIS("xyposl: %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_xyposl);
		})
	);
	map(0x0148, 0x014b).lrw32(
		NAME([this] () { return m_linedxyh; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOGREGIS("linedxyh: %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_linedxyh);
		})
	);
	map(0x014c, 0x014f).lrw32(
		NAME([this] () { return m_linedxyl; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOGREGIS("linedxyl: %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_linedxyl);
		})
	);
	// TODO: these are in 12.20 format
	map(0x0150, 0x0153).lrw32(
		NAME([this] () { return m_dxyh; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOGREGIS("dxyh: %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_dxyh);
		})
	);
	map(0x0154, 0x0157).lrw32(
		NAME([this] () { return m_dxyl; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOGREGIS("dxyl: %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_dxyl);
		})
	);
	map(0x0158, 0x015b).lrw32(
		NAME([this] () { return m_ddxyh; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOGREGIS("ddxyh: %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_ddxyh);
		})
	);
	map(0x015c, 0x015f).lrw32(
		NAME([this] () { return m_ddxyl; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOGREGIS("ddxyl: %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_ddxyl);
		})
	);
	// PIP: Pen Index Palette (or PLT: Palette Look-up Table)
	// Reads are split in words, but writes are dword (cutting at 0x1bf)
	// TODO: does writing to 0x1c0-0x1ff go to mirror or they are just ignored?
	map(0x0180, 0x01ff).lrw32(
		NAME([this] (offs_t offset) {
			const u16 reg = offset >> 1;
			if (offset & 1)
				return m_pip[reg & 0x0f] >> 16;

			return m_pip[reg & 0x0f] & 0xffff;
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (offset & 0x10)
			{
				LOG("Warning: write to PIP at %04x (ignored)\n", (offset * 4) + 0x180);
				return;
			}
			COMBINE_DATA(&m_pip[offset & 0xf]);
		})
	);
	// part 1 of fence
	map(0x0218, 0x021f).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOGFENCE("fence [%d]: %08x & %08x\n", offset, data, mem_mask);
			COMBINE_DATA(&m_fence[offset]);
		})
	);
	map(0x0230, 0x023f).lr32(
		NAME([this] (offs_t offset) {
			const u16 reg = offset >> 1;
			if (offset & 1)
				return m_fence[reg] >> 16;

			return m_fence[reg] & 0xffff;
		})
	);
	// part 2 of fence
	map(0x0238, 0x023f).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOGFENCE("fence [%d]: %08x & %08x\n", offset | 2, data, mem_mask);
			COMBINE_DATA(&m_fence[offset | 2]);
		})
	);
	map(0x0270, 0x027f).lr32(
		NAME([this] (offs_t offset) {
			const u16 reg = offset >> 1;
			if (offset & 1)
				return m_fence[reg | 2] >> 16;

			return m_fence[reg | 2] & 0xffff;
		})
	);
	map(0x0300, 0x03ff).lrw32(
		NAME([this] (offs_t offset) { return m_mmu[offset & 0x3f]; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOGMMU("MMU [%d]: %08x & %08x\n", offset, data, mem_mask);
			COMBINE_DATA(&m_mmu[offset & 0x3f]);
		})
	);
	map(0x0400, 0x05ff).lrw32(
		NAME([this] (offs_t offset) {
			const u16 channel = (offset >> 2) & 0x1f;
			const u8 reg = offset & 3;
			return m_dma[channel][reg] & 0x3f'fffc;
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			const u16 channel = (offset >> 2) & 0x1f;
			const u8 reg = offset & 3;
			LOGDMA("DMA [%d] reg [%02x]: %08x & %08x\n", channel, reg, data, mem_mask);
			COMBINE_DATA(&m_dma[channel][reg]);
			// TODO: despite documentation mask really depends on what channel is
			// (video DMA definitely sets it with 0x20'0000 high)
			m_dma[channel][reg] &= 0x3f'fffc;
		})
	);
	/* Hardware multiplier */
	// NOTE: only 40 registers, cutoff beyond that
	map(0x0600, 0x069f).lrw32(
		NAME([this] (offs_t offset) {
			return m_mult[offset & 0x3f];
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOGMULT("Mult [%d]: %08x & %08x\n", offset, data, mem_mask);
			COMBINE_DATA(&m_mult[offset & 0x3f]);
		})
	);
	map(0x07f0, 0x07f3).lrw32(
		NAME([this] () { return m_mult_control; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			m_mult_control |= data;
			LOGMULT("Mult control set: %08x & %08x -> %08x\n", data, mem_mask, m_mult_control);
		})
	);
	map(0x07f4, 0x07f7).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOGMULT("Mult control clear: %08x & %08x -> %08x\n", data, mem_mask, m_mult_control);
			m_mult_control &= ~data;
		})
	);
	map(0x07f8, 0x07fb).lr32(NAME([this] () { return m_mult_status; }));
//  map(0x07fc, 0x07ff) start process
}

u32 madam_device::mctl_r()
{
	return m_mctl;
}

/*
 * $0330'0008 MCTL (Green)
 * ---- ---- --x- ---- ---- ---- ---- ---- CPUVEN enable CPU write to H counter
 * ---- ---- ---P ---- ---- ---- ---- ---- NTSC/PAL (on Preen chipset, unused on Green)
 * ---- ---- ---- x--- ---- ---- ---- ---- FENCOP non-system option in fence
 * ---- ---- ---- --x- ---- ---- ---- ---- SLIPXEN SlipStream enable
 * ---- ---- ---- ---x ---- ---- ---- ---- FENCEEN Fence enable
 * ---- ---- ---- ---- x--- ---- ---- ---- PLAYXEN enable Player DMA
 * ---- ---- ---- ---- -x-- ---- ---- ---- VSCTXEN enable video transfers
 * ---- ---- ---- ---- --x- ---- ---- ---- CLUTXEN enable Clut transfers
 */
void madam_device::mctl_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (ACCESSING_BITS_8_15)
	{
		if (!BIT(m_mctl, 15) && BIT(data, 15))
		{
			m_dma_playerbus_timer->adjust(attotime::from_ticks(2, this->clock()));
			// HACK: Fake inputs here
			// Madam can access Player bus from DMA only, and the port(s) are daisy chained thru
			// bidirectional serial i/f (which also handle headphone jack and ROM device transfers)
			// Smells a lot like an internal MCU doing the job ...
			m_dma32_write_cb(m_dma[23][2] + 0x4, m_playerbus_read_cb(0));
		}
		if (BIT(m_mctl, 15) && !BIT(data, 15))
		{
			LOG("mctl: Player bus DMA abort?\n");
			m_dma_playerbus_timer->adjust(attotime::never);
		}

		m_amy->dac_enable(!!BIT(data, 13));
	}

	if (data & 0xffee'1fff)
		LOG("mctl: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_mctl);
}

TIMER_CALLBACK_MEMBER(madam_device::dma_playerbus_cb)
{
	if (!BIT(m_mctl, 15))
		return;

	u32 count = m_dma[23][1];

	if (BIT(count, 31))
	{
		m_mctl &= ~(1 << 15);
		m_irq_dply_cb(1);
		m_dma_playerbus_timer->adjust(attotime::never);
		return;
	}

	// TODO: should cause a privbits exception if mask outside bounds
	u32 src = m_dma[23][2];
	u32 dst = m_dma[23][0];

	const u32 data = m_dma32_read_cb(src);
	m_dma32_write_cb(dst, data);

	count -= 4;
	src += 4;
	dst += 4;
	m_dma[23][0] = dst;
	m_dma[23][1] = count;
	m_dma[23][2] = src;

	m_dma_playerbus_timer->adjust(attotime::from_ticks(2, this->clock()));
}

/******************
 *
 * VDLP engine
 *
 ******************/

void madam_device::vdlp_start_w(int state)
{
	// PLAYXEN
	if (state || !BIT(m_mctl, 14))
	{
		m_vdlp.fetch = false;
		return;
	}

//  if (m_vdlp.address == m_dma[24][0])
//  {
//      m_vdlp.fetch = false;
//      return;
//  }

	m_vdlp.address = m_dma[24][0];
	m_vdlp.scanlines = 0;
	m_vdlp.fetch = true;
	m_vdlp.y_dest = 6;

	LOGVDLP("VDLP start %08x\n", m_vdlp.address);
}

void madam_device::vdlp_continue_w(int state)
{
	if (!state || !BIT(m_mctl, 14) || !m_vdlp.fetch)
		return;

	if (m_vdlp.scanlines == 0)
	{
		// abort if we are out of VRAM space
		if (!(m_vdlp.address & 0x20'0000))
		{
			m_vdlp.fetch = false;
			LOGVDLP("line=%d abort with address %08x\n", m_vdlp.address);
			return;
		}

		const u32 control_word = m_dma32_read_cb(m_vdlp.address);

		m_vdlp.scanlines = (control_word & 0x1ff);

		if (m_vdlp.scanlines == 0)
		{
			m_vdlp.fetch = false;
			return;
		}

		// upper limit of 34 due of hblank
		const u16 clut_words = std::min<u16>((control_word >> 9) & 0x3f, 34) << 2;
		static const u16 modulo_values[8] = { 320, 384, 512, 640, 1024, 1, 1, 1 };
		m_vdlp.modulo = modulo_values[(control_word >> 23) & 7] >> 1;
		if (m_vdlp.modulo == 0)
		{
			m_vdlp.fetch = false;
			popmessage("3do_madam.cpp: undocumented modulo fetch %08x", (control_word >> 23) & 7);
			return;
		}

		const bool current_fb = !!BIT(control_word, 16);
		m_vdlp.video_dma = !!BIT(control_word, 21);

		LOGVDLP("%08x: line=%d Control word %08x\n", m_vdlp.address, m_vdlp.y_dest, control_word);
		LOGVDLP("    scanline length %d mode=%d| CLUT words %02x FB mode %d|%d|%d|%d video DMA %d| modulo %d\n"
			, m_vdlp.scanlines
			, BIT(control_word, 19) ? 480 : 240
			, clut_words
			, !!BIT(control_word, 15) // previous FB
			, current_fb
			, !!BIT(control_word, 17) // previous modulo
			, !!BIT(control_word, 18) // type of address
			// 20: reserved
			, m_vdlp.video_dma
			// 22: reserved
			, m_vdlp.modulo
		);

		if (current_fb)
		{
			m_vdlp.fb_address = m_dma32_read_cb(m_vdlp.address + 0x04);
			LOGVDLP("Current fb %08x\n", m_vdlp.fb_address);
		}

		// TODO: previous fb handling

		// CLUT transfers
		for (int c = 0; c < clut_words; c+= 4)
			m_amy->clut_write(m_dma32_read_cb(m_vdlp.address + 0x10 + c));

		m_vdlp.link = m_dma32_read_cb(m_vdlp.address + 0x0c);
		m_vdlp.y_src = 0;
	}

	if (m_vdlp.video_dma)
	{
		u32 y_base = (m_vdlp.y_src & ~1) * (m_vdlp.modulo);
		u8 shift = ((m_vdlp.y_src & 1) ^ 1) * 16;
		for (int x = 0; x < 320; x++)
		{
			const u32 dot = m_dma32_read_cb(m_vdlp.fb_address + ((x + y_base) << 2));
			m_amy->pixel_xfer(x, m_vdlp.y_dest, dot >> shift);
		}
	}
	else
		m_amy->blank_line(m_vdlp.y_dest);

	m_vdlp.scanlines --;
	m_vdlp.y_dest ++;
	m_vdlp.y_src ++;
	if (m_vdlp.scanlines == 0)
	{
		m_vdlp.address = m_vdlp.link;
	}
}

/******************
 *
 * CEL engine
 *
 *****************/

u32 madam_device::regctl0_r()
{
	return m_regctl0;
}

void madam_device::regctl0_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGREGIS("regctl0 (control word): %08x & %08x\n", data, mem_mask);
	if (ACCESSING_BITS_0_15 && data != m_regctl0)
	{
		COMBINE_DATA(&m_regctl0);
		for (int i = 0; i < 2; i ++)
		{
			const u8 bit_base = i << 8;
			u16 g1 = 0, g2 = 0;
			bool illegal = false;
			switch ((m_regctl0 >> bit_base) & 0xf)
			{
				case 0x00:  break;
				case 0x01:  g1 += 32; break;
				case 0x04:  g1 += 256; break;
				case 0x08:  g1 += 1024; break;
				default:    illegal = true; break;
			}

			switch ((m_regctl0 >> bit_base) & 0xf0)
			{
				case 0x00:  break;
				case 0x10:  g2 += 64; break;
				case 0x20:  g2 += 128; break;
				case 0x40:  g2 += 256; break;
				default:    illegal = true; break;
			}

			// TODO: verify these two statements
			// - bit 1 and 7 are undefined;
			// - multiple bits set in a nibble are illegal;
			// these would be rejected by Portfolio, so we won't see this on normal means ...
			if (illegal)
			{
				popmessage("3do_madam.cpp: illegal regctl0 setup %04x", m_regctl0);
				return;
			}

			m_regis.fb_pitch[i] = (g1 + g2) >> 1;
		}
	}
}


void madam_device::cel_start_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGCEL("Start CEL engine\n");
	m_cel.state = FETCH_PARAMS;
	m_cel.address = m_dma[26][1];
	m_statbits |= 1 << 4;
	m_statbits &= ~(1 << 6);
	m_cel_timer->adjust(attotime::from_ticks(2, this->clock()));
}

void madam_device::cel_stop_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGCEL("Stop CEL engine\n");
	m_cel.state = IDLE;
	m_statbits &= ~(1 << 4);
	m_cel_timer->adjust(attotime::never);
}

// TODO: is all of this madness burst, cycle steal or a mix of the two?
// 3do_try alternating Sanyo/3do logos spinning seems that some bus hog is ought to happen ...
TIMER_CALLBACK_MEMBER(madam_device::cel_tick_cb)
{
	u32 tick_time;

	switch(m_cel.state)
	{
		case IDLE:
			break;
		case FETCH_PARAMS:
		{
			tick_time = 1;

			m_cel.current_ccb = m_dma32_read_cb(m_cel.address);
			LOGCEL("CEL fetch params [%08x] flags=%08x\n", m_cel.address, m_cel.current_ccb);
			m_cel.skip = !!BIT(m_cel.current_ccb, 31);
			m_cel.last = !!BIT(m_cel.current_ccb, 30);
			m_cel.next_ptr = m_dma32_read_cb(m_cel.address + 0x04);

			if (m_cel.skip && m_cel.last)
			{
				LOGCEL("Skip + Last, done\n");
				m_statbits |= (1 << 6);
				cel_stop_w(0, 0, 0xffffffff);
				return;
			}
			else if (m_cel.skip)
			{
				LOGCEL("Skip, move to next CCB\n");
				m_cel.state = FETCH_PARAMS;
				m_cel.address = m_cel.next_ptr;
				m_cel_timer->adjust(attotime::from_ticks(2 * 2, this->clock()));
				return;
			}
			else if (!m_cel.last)
			{
				// add an extra tick for actually fetching next ptr
				tick_time += 1;
			}

			const bool npabs = !!BIT(m_cel.current_ccb, 29);
			const bool spabs = !!BIT(m_cel.current_ccb, 28);
			const bool ppabs = !!BIT(m_cel.current_ccb, 27);
			const bool ldsize = !!BIT(m_cel.current_ccb, 26);
			const bool ldprs = !!BIT(m_cel.current_ccb, 25);
			const bool ldpixc = !!BIT(m_cel.current_ccb, 24);
			LOGCEL("    CCB skip=%d last=%d npabs=%d spabs=%d ppabs=%d ldsize=%d ldprs=%d ldpixc=%d\n"
				, m_cel.skip
				, m_cel.last
				, npabs
				, spabs
				, ppabs
				, ldsize
				, ldprs
				, ldpixc
			);
			const bool ldplut = !!BIT(m_cel.current_ccb, 23);
			m_cel.ccbpre = !!BIT(m_cel.current_ccb, 22);
			const bool yoxy = !!BIT(m_cel.current_ccb, 21);
			LOGCEL("        ldplut=%d ccbpre=%d yoxy=%d acsc=%d alsc=%d acw=%d accw=%d twd=%d\n"
				, ldplut
				, m_cel.ccbpre
				, yoxy
				, BIT(m_cel.current_ccb, 20)
				, BIT(m_cel.current_ccb, 19)
				, BIT(m_cel.current_ccb, 18)
				, BIT(m_cel.current_ccb, 17)
				, BIT(m_cel.current_ccb, 16)
			);
			m_cel.packed = !!BIT(m_cel.current_ccb, 9);
			LOGCEL("        lce=%d ace=%d maria=%d pxor=%d useav=%d packed=%d\n"
				, BIT(m_cel.current_ccb, 15)
				, BIT(m_cel.current_ccb, 14)
				//, BIT(m_cel.current_ccb, 13) spare
				, BIT(m_cel.current_ccb, 12)
				, BIT(m_cel.current_ccb, 11)
				, BIT(m_cel.current_ccb, 10)
				, m_cel.packed
			);
			m_cel.bgnd = !!BIT(m_cel.current_ccb, 5);
			LOGCEL("        pover=%d plutpos=%d bgnd=%d noblk=%d pluta=%d\n"
				, (m_cel.current_ccb & 0x180) >> 7
				, BIT(m_cel.current_ccb, 6)
				, m_cel.bgnd
				, BIT(m_cel.current_ccb, 4)
				, (m_cel.current_ccb & 0xe) >> 1
			);
			// FIXME: relative to what?
			// - 3do_fz1 / 3do_fz10 RGB dots (scaled by hdx/vdy=8.0) are the first & last entry setup, relative to zero?
			if (!npabs || !spabs || !ppabs)
			{
				popmessage("CEL relative address use at %08x %d|%d|%d", m_cel.address, npabs, spabs, ppabs);
				m_statbits |= (1 << 6);
				cel_stop_w(0, 0, 0xffffffff);
				return;
			}
			m_cel.source_ptr = m_dma32_read_cb(m_cel.address + 0x08);
			m_cel.plut_ptr = m_dma32_read_cb(m_cel.address + 0x0c);
			tick_time += 2;
			LOGCEL("    NEXTPTR %08x SOURCEPTR %08x PLUTPTR %08x\n", m_cel.next_ptr, m_cel.source_ptr, m_cel.plut_ptr);
			if (!ldsize || !ldprs || !yoxy || !ldpixc)
			{
				popmessage("CEL using existing values at %08x %d|%d|%d|%d", m_cel.address, ldsize, ldprs, yoxy, ldpixc);
				m_statbits |= (1 << 6);
				cel_stop_w(0, 0, 0xffffffff);
				return;
			}
			m_cel.xpos = (s32)(m_dma32_read_cb(m_cel.address + 0x10));
			m_cel.ypos = (s32)(m_dma32_read_cb(m_cel.address + 0x14));
			tick_time += 2;
			// TODO: can be in 17.15 format (?)
			LOGCEL("    xpos=%f ypos=%f\n",  (double)m_cel.xpos / 65536.0, (double)m_cel.ypos / 65536.0);

			m_cel.hdx = m_dma32_read_cb(m_cel.address + 0x18);
			m_cel.hdy = m_dma32_read_cb(m_cel.address + 0x1c);
			m_cel.vdx = m_dma32_read_cb(m_cel.address + 0x20);
			m_cel.vdy = m_dma32_read_cb(m_cel.address + 0x24);
			tick_time += 4;
			LOGCEL("    hdx=%f hdy=%f vdx=%f vdy=%f\n"
				, (double)m_cel.hdx / 1048576.0, (double)m_cel.hdy / 1048576.0
				, (double)m_cel.vdx / 65536.0, (double)m_cel.vdy / 65536.0
			);

			m_cel.hddx = m_dma32_read_cb(m_cel.address + 0x28);
			m_cel.hddy = m_dma32_read_cb(m_cel.address + 0x2c);
			tick_time += 2;
			LOGCEL("    hddx=%f hddy=%f\n", (double)m_cel.hddx / 1048576.0, (double)m_cel.hddy / 1048576.0);

			m_cel.pixc = m_dma32_read_cb(m_cel.address + 0x30);
			tick_time += 1;
			LOGCEL("    pixc=%08x\n", m_cel.pixc);

			// fetch the Preamble words
			// May as well do it here because ...
			if (m_cel.ccbpre)
			{
				m_cel.pre0 = m_dma32_read_cb(m_cel.address + 0x34);
				tick_time ++;
				LOGCEL("    pre0=%08x ", m_cel.pre0);
				if (!m_cel.packed)
				{
					m_cel.pre1 = m_dma32_read_cb(m_cel.address + 0x38);
					LOGCEL("pre1=%08x", m_cel.pre1);
					tick_time ++;
				}
				LOGCEL("\n");
			}
			else
			{
				m_cel.pre0 = m_dma32_read_cb(m_cel.source_ptr + 0x00);
				m_cel.source_ptr += 4;
				tick_time ++;
				LOGCEL("    pre0=%08x ", m_cel.pre0);
				if (!m_cel.packed)
				{
					m_cel.pre1 = m_dma32_read_cb(m_cel.source_ptr + 0x00);
					m_cel.source_ptr += 4;
					tick_time ++;
					LOGCEL("pre1=%08x", m_cel.pre1);
				}
				LOGCEL("\n");
			}

			// ... we have to take an intermediate step in case the CEL is compressed
			m_cel.state = m_cel.packed ? DECOMPRESS : DRAW;
			m_cel_timer->adjust(attotime::from_ticks(2 * tick_time, this->clock()));

			break;
		}
		case DECOMPRESS:
		{
			tick_time = cel_decompress();
			if (tick_time)
			{
				m_cel.state = DRAW;
				m_cel_timer->adjust(attotime::from_ticks(2 * tick_time, this->clock()));
			}
			break;
		}
		case DRAW:
		{
			tick_time = 1;

			// TODO: doubled with lrform = 1 (?)
			const u16 vcnt = ((m_cel.pre0 >> 6) & 0xfff) + 1;
			const bool uncoded = !!BIT(m_cel.pre0, 4);
			const u8 bpp = (m_cel.pre0 >> 0) & 0x7;
			static const char *const BPP_VALUES[8] = { "<0 reserved>", "1bpp", "2bpp", "4bpp", "6bpp", "8bpp", "16bpp", "<7 reserved>" };

			LOGCEL("    skipx=%d vcnt=%d uncoded=%d rep8=%d bpp=%d (%s)\n"
				, (m_cel.pre0 >> 24) & 0xf
				, vcnt
				, uncoded
				, BIT(m_cel.pre0, 3)
				, bpp
				, BPP_VALUES[bpp]
			);
			const u16 woffset10 = ((m_cel.pre1 >> 16) & 0x3ff) + 2;
			const bool lrform = !!BIT(m_cel.pre1, 11);
			const u16 tlhpcnt = ((m_cel.pre1 >> 0) & 0x7ff) + 1;
			LOGCEL("    woffset(8)=%d woffset(10)=%d noswap=%d unclsb=%d lrform=%d tlhpcnt=%d\n"
				, ((m_cel.pre1 >> 24) & 0x7f) + 2
				, woffset10
				, BIT(m_cel.pre1, 14)
				, (m_cel.pre1 >> 12) & 3
				, lrform
				, tlhpcnt
			);

			const u16 xclip = m_regis.xclip;
			const u16 yclip = m_regis.yclip;
			//const u16 src_pitch = m_regis.fb_pitch[0];
			const u16 dst_pitch = m_regis.fb_pitch[1];

			// encode source addressing in an easy to digest inner loop form
			const u8 actual_src_mode = (m_cel.packed << 1) | lrform;

			// - 3do_gdo101 uses mostly this, with lrform + packed enabled on several elements
			//   (the 3DO logo)
			// - 3do_fz1 uses uncoded=0 + bpp=4
			if ((m_cel.packed && bpp == 3) || (m_cel.packed && bpp == 4) || (bpp == 6))
			{
				for (int y = 0; y < vcnt; y++)
				{
					int ypos = (m_cel.ypos >> 16) + y;

					if (ypos != std::clamp<unsigned>(ypos, 0, yclip))
						continue;

					for (int x = 0; x < tlhpcnt; x++)
					{
						int xpos = (m_cel.xpos >> 16) + x;
						if (xpos != std::clamp<unsigned>(xpos, 0, xclip))
							continue;

						u16 src_data = (this->*get_pixel_table[actual_src_mode])(x, y, woffset10);

						// opaque check
						if (!src_data && !m_cel.bgnd)
							continue;

						u32 dst_address = m_regctl3;
						dst_address += ((ypos & ~1) * dst_pitch) << 2;
						dst_address += (xpos << 2);

						u32 dst_data = m_dma32_read_cb(dst_address);
						u8 dst_shift = ((ypos ^ 1) & 1) * 16;
						dst_data &= dst_shift ? 0xffff : 0xffff0000;

						m_dma32_write_cb(dst_address, (src_data << dst_shift) | dst_data);

						tick_time += 3;
					}
				}

				LOGCEL("CEL Time drawing %d\n", tick_time);
			}

			if (m_cel.last)
			{
				m_statbits |= (1 << 6);
				cel_stop_w(0, 0, 0xffffffff);
			}
			else
			{
				m_cel.state = FETCH_PARAMS;
				m_cel.address = m_cel.next_ptr;
				m_cel_timer->adjust(attotime::from_ticks(2 * tick_time, this->clock()));
			}

			break;
		}
	}
}

/******************
 *
 * Decompression
 *
 *****************/

const madam_device::get_woffset_func madam_device::get_woffset_table[2] =
{
	&madam_device::get_woffset8,
	&madam_device::get_woffset10
};

u16 madam_device::get_woffset8(u32 ptr)
{
	return m_dma8_read_cb(ptr) + 2;
}

u16 madam_device::get_woffset10(u32 ptr)
{
	return ((m_dma8_read_cb(ptr) << 8) | (m_dma8_read_cb(ptr + 1))) + 2;
}

std::tuple<u8, u32> madam_device::fetch_byte(u32 ptr, u8 frac)
{
	u16 res = m_dma8_read_cb(ptr) << frac;
	if (frac)
	{
		const u8 shift = (8 - frac);
		const u8 mask = 0xff >> shift;
		ptr ++;
		res |= (m_dma8_read_cb(ptr) >> shift) & mask;
	}
	return std::make_tuple(res & 0xff, ptr);
}

const madam_device::fetch_rle_func madam_device::fetch_rle_table[16] =
{
	&madam_device::get_unemulated,  // 0: illegal
	&madam_device::get_unemulated,
	&madam_device::get_unemulated,  // 1: 1bpp
	&madam_device::get_unemulated,
	&madam_device::get_unemulated,  // 2: 2bpp
	&madam_device::get_unemulated,
	&madam_device::get_coded_4bpp,  // 3: 4bpp
	&madam_device::get_unemulated,
	&madam_device::get_coded_6bpp,  // 4: 6bpp
	&madam_device::get_unemulated,
	&madam_device::get_unemulated,  // 5: 8bpp
	&madam_device::get_unemulated,
	&madam_device::get_coded_16bpp, // 6: 16bpp
	&madam_device::get_uncoded_16bpp,
	&madam_device::get_unemulated,  // 7: illegal
	&madam_device::get_unemulated
};

// Stub for unemulated/illegal paths
std::tuple<u16, u32> madam_device::get_unemulated(u32 ptr, u8 frac)
{
	return std::make_tuple(0, ptr + 1);
};

// - 3do_try "3" charset
std::tuple<u16, u32> madam_device::get_coded_4bpp(u32 ptr, u8 frac)
{
	u8 idx;
	const u32 plut_ptr = m_cel.plut_ptr;
	std::tie(idx, ptr) = fetch_byte(ptr, frac);

	// idx >>= 4;
	// idx &= 0x0f;
	idx >>= 3;
	idx &= 0x1e;

	return std::make_tuple((m_dma8_read_cb(plut_ptr + idx) << 8) | m_dma8_read_cb(plut_ptr + idx + 1), ptr);
}

// - 3do_fz1 / 3do_fz10
std::tuple<u16, u32> madam_device::get_coded_6bpp(u32 ptr, u8 frac)
{
	u8 idx;
	const u32 plut_ptr = m_cel.plut_ptr;
	std::tie(idx, ptr) = fetch_byte(ptr, frac);

	// idx >>= 2;
	// idx &= 0x3f;
	idx >>= 1;
	idx &= 0x7e;

	return std::make_tuple((m_dma8_read_cb(plut_ptr + idx) << 8) | m_dma8_read_cb(plut_ptr + idx + 1), ptr);
}

// - 3do_try on Sanyo 3DO logo
// A wasteful mode, sets woffset10 and 2 bytes per color fetch for a PLUT lookup trip.
std::tuple<u16, u32> madam_device::get_coded_16bpp(u32 ptr, u8 frac)
{
	const u32 plut_ptr = m_cel.plut_ptr;
	u8 idx = m_dma8_read_cb(ptr + 1);

	idx <<= 1;
	idx &= 0x7e;

	return std::make_tuple((m_dma8_read_cb(plut_ptr + idx) << 8) | m_dma8_read_cb(plut_ptr + idx + 1), ptr + 2);
}

// - 3do_gdo101
std::tuple<u16, u32> madam_device::get_uncoded_16bpp(u32 ptr, u8 frac)
{
	return std::make_tuple((m_dma8_read_cb(ptr) << 8) | m_dma8_read_cb(ptr + 1), ptr + 2);
}

// LZ77 / LZSS alike
// NOTE: documentation claims that is actually faster to use packed CEL
u32 madam_device::cel_decompress()
{
	u32 tick_time = 1;
	u32 source_ptr = m_cel.source_ptr;

	const u16 vcnt = ((m_cel.pre0 >> 6) & 0xfff) + 1;
	const bool uncoded = !!BIT(m_cel.pre0, 4);
	const u8 bpp = (m_cel.pre0 >> 0) & 0x7;

	if ((bpp == 0 || bpp == 7) ||
		(bpp == 1) ||
		(bpp == 2) ||
		(bpp == 3 && uncoded) ||
		(bpp == 4 && uncoded) ||
		(bpp == 5))
	{
		popmessage("3do_madam.cpp: unsupported Packed CEL %d %d %08x", bpp, uncoded, source_ptr);
		m_statbits |= (1 << 6);
		cel_stop_w(0, 0, 0xffffffff);
		return 0;
	}

	u16 tlhpcnt = 1;
	const u16 pitch = 0x1000;
	const u8 woffset_type = bpp >= 5;
	const u8 woffset_inc = woffset_type + 1;
	// Reminders:
	// - bpp == 0 and bpp == 7 are illegal
	// - bpp == 5 and 6 don't use the fractional part (matters only for anything below that)
	static const u8 frac_bits[8] = { 0, 1, 2, 4, 6, 8, 16, 0 };
	const u8 frac_inc = frac_bits[bpp];
	const u8 actual_rle_mode = (bpp << 1) | uncoded;

	for (u16 yline = 0; yline < vcnt; yline ++)
	{
		u32 line_ptr = source_ptr;
		u16 woffset = (this->*get_woffset_table[woffset_type])(line_ptr);
		u32 next_ptr = line_ptr + woffset * 4;
		line_ptr += woffset_inc;
		tick_time += 1;

		u16 xpos = 0;
		u8 frac_bit = 0;
		u8 header = 0;
		while (line_ptr < next_ptr)
		{
			std::tie(header, line_ptr) = fetch_byte(line_ptr, frac_bit);

			const u8 packet_type = header >> 6;
			const u8 num_bytes = (header & 0x3f) + 1;
			u8 src;

			//frac_bit += 8;
			//frac_bit &= 7;
			// round up on even address
			if (frac_bit == 0)
				line_ptr ++;
			tick_time ++;
			u16 pixel_data = 0;
			switch (packet_type)
			{
				// PACK_TRANSPARENT
				// TODO: doesn't really work properly, particularly when bgnd is 1
				// - 3do_fz10 CD overlays uses plenty of these, which currently fills solid black
				// We could cheat and pull bit 15 high, but then we have to deal accordingly
				// when writing to framebuffer (that uses it as cornerweight or CLUT selector) ...
				case 2:
					for (src = 0; src < num_bytes; src++)
						m_cel.buffer[yline * pitch + ((src + xpos) % pitch)] = 0;

					tick_time ++;
					xpos += num_bytes;
					//line_ptr += 2;
					break;
				// PACK_REPEAT
				case 3:
					std::tie(pixel_data, line_ptr) = (this->*fetch_rle_table[actual_rle_mode])(line_ptr, frac_bit);
					frac_bit += frac_inc;
					frac_bit &= 7;

					for (src = 0; src < num_bytes; src++)
						m_cel.buffer[yline * pitch + ((src + xpos) % pitch)] = pixel_data;

					tick_time ++;
					xpos += num_bytes;
					break;
				// PACK_LITERAL
				case 1:
					for (src = 0; src < num_bytes; src++)
					{
						//pixel_data = (m_dma8_read_cb(line_ptr) << 8) | m_dma8_read_cb(line_ptr + 1);
						std::tie(pixel_data, line_ptr) = (this->*fetch_rle_table[actual_rle_mode])(line_ptr, frac_bit);
						frac_bit += frac_inc;
						frac_bit &= 7;
						tick_time ++;
						m_cel.buffer[yline * pitch + ((src + xpos) % pitch)] = pixel_data;
					}

					xpos += num_bytes;
					break;
				// PACK_EOL
				case 0:
					line_ptr = next_ptr;
					break;
			}
		}

		// TODO: it is unclear what happens if the source data is uneven
		// Documentation claims "not necessarily rectangle" ...
		tlhpcnt = std::max<unsigned>(tlhpcnt, xpos);
		source_ptr = next_ptr;
	}

	// setup the preamble so that DRAW state knows what to do
	m_cel.pre1 = (tlhpcnt - 1);
	return tick_time;
}

/******************
 *
 * FB fetch fns
 *
 *****************/

const madam_device::get_pixel_func madam_device::get_pixel_table[4] =
{
	&madam_device::get_uncompressed_16bpp_lrform0,
	&madam_device::get_uncompressed_16bpp_lrform1,
	&madam_device::get_compressed_16bpp,
	&madam_device::get_compressed_16bpp
};

u16 madam_device::get_uncompressed_16bpp_lrform0(int x, int y, u16 woffset)
{
	u32 cel_address = m_cel.source_ptr;

	cel_address += ((y) * woffset) << 2;
	cel_address += ((x & ~1) << 1);
	u8 src_shift = (x & 1) ^ 1;

	u16 src_data = m_dma32_read_cb(cel_address) >> (src_shift * 16);

	return src_data;
}

u16 madam_device::get_uncompressed_16bpp_lrform1(int x, int y, u16 woffset)
{
	u32 cel_address = m_cel.source_ptr;

	cel_address += ((y & ~1) * (woffset)) << 2;
	cel_address += ((x) << 2);
	u8 src_shift = (y & 1) ^ 1;

	u16 src_data = m_dma32_read_cb(cel_address) >> (src_shift * 16);

	return src_data;
}

u16 madam_device::get_compressed_16bpp(int x, int y, u16 woffset)
{
	const u16 pitch = 0x1000;

	u16 src_data = m_cel.buffer[x + (y * pitch)];
	return src_data;
}
