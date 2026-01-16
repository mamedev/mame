// license:LGPL-2.1+
// copyright-holders:Angelo Salese, Wilbert Pol

#include "emu.h"
#include "3do_madam.h"

#define LOG_CEL     (1U << 1)
#define LOG_REGIS   (1U << 2)
#define LOG_FENCE   (1U << 3)
#define LOG_MMU     (1U << 4)
#define LOG_DMA     (1U << 5)
#define LOG_MULT    (1U << 6)

#define VERBOSE (LOG_GENERAL | LOG_CEL | LOG_REGIS | LOG_MMU | LOG_MULT)
//#define LOG_OUTPUT_FUNC osd_printf_warning

#include "logmacro.h"

#define LOGCEL(...)     LOGMASKED(LOG_CEL,     __VA_ARGS__)
#define LOGREGIS(...)   LOGMASKED(LOG_REGIS,   __VA_ARGS__)
#define LOGFENCE(...)   LOGMASKED(LOG_FENCE,   __VA_ARGS__)
#define LOGMMU(...)     LOGMASKED(LOG_MMU,     __VA_ARGS__)
#define LOGDMA(...)     LOGMASKED(LOG_DMA,     __VA_ARGS__)
#define LOGMULT(...)    LOGMASKED(LOG_MULT,    __VA_ARGS__)

DEFINE_DEVICE_TYPE(MADAM, madam_device, "madam", "3DO MN7A020UDA \"Madam\" Address Decoder")

madam_device::madam_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MADAM, tag, owner, clock)
	, m_diag_cb(*this)
	, m_dma_read_cb(*this, 0)
	, m_dma_write_cb(*this)
	, m_irq_dply_cb(*this)
{
}

void madam_device::device_add_mconfig(machine_config &config)
{
	// TODO: at least CEL engine
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

	save_item(NAME(m_pip));
	save_item(NAME(m_fence));
	save_item(NAME(m_mmu));
	save_item(NAME(m_mult));
}

void madam_device::device_reset()
{
	m_mctl = 0;
	m_dma_playerbus_timer->adjust(attotime::never);
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
//	map(0x0100, 0x0103)  SPRSTRT - Start the CEL engine (W)
//	map(0x0104, 0x0107)  SPRSTOP - Stop the CEL engine (W)
//	map(0x0108, 0x010b)  SPRCNTU - Continue the CEL engine (W)
//	map(0x010c, 0x010f)  SPRPAUS - Pause the CEL engine (W)
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
	map(0x0130, 0x0133).lrw32(
		NAME([this] () { return m_regctl0; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOGREGIS("regctl0 (control word): %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_regctl0);
		})
	);
	map(0x0134, 0x0137).lrw32(
		NAME([this] () { return m_regctl1; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOGREGIS("regctl1 (X and Y clip values): %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_regctl1);
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
			return m_dma[channel][reg] & 0x1f'fffc;
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			const u16 channel = (offset >> 2) & 0x1f;
			const u8 reg = offset & 3;
			LOGDMA("DMA [%d] reg [%02x]: %08x & %08x\n", channel, reg, data, mem_mask);
			COMBINE_DATA(&m_dma[channel][reg]);
			m_dma[channel][reg] &= 0x1f'fffc;
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
//	map(0x07fc, 0x07ff) start process
}

u32 madam_device::mctl_r()
{
	return m_mctl;
}

/*
 * $0330'0008 MCTL (Green)
 * ---- ---- --x- ---- ---- ---- ---- ---- CPUVEN
 * ---- ---- ---P ---- ---- ---- ---- ---- NTSC/PAL (on Preen chipset, unused on Green)
 * ---- ---- ---- x--- ---- ---- ---- ---- FENCOP
 * ---- ---- ---- --x- ---- ---- ---- ---- SLIPXEN
 * ---- ---- ---- ---x ---- ---- ---- ---- FENCEEN
 * ---- ---- ---- ---- x--- ---- ---- ---- PLAYXEN
 * ---- ---- ---- ---- -x-- ---- ---- ---- VSCTXEN
 * ---- ---- ---- ---- --x- ---- ---- ---- CLUTXEN
 */
void madam_device::mctl_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (ACCESSING_BITS_8_15)
	{
		if (!BIT(m_mctl, 15) && BIT(data, 15))
			m_dma_playerbus_timer->adjust(attotime::from_ticks(2, this->clock()));
		if (BIT(m_mctl, 15) && !BIT(data, 15))
		{
			LOG("mctl: Player bus DMA abort?\n");
			m_dma_playerbus_timer->adjust(attotime::never);
		}
	}

	if (data != 0x0001e000)
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
		m_mctl &= ~0x8000;
		m_irq_dply_cb(1);
		m_dma_playerbus_timer->adjust(attotime::never);
		return;
	}

	// TODO: from RAM should likely first obtain the serial data from the controller(s)
	// TODO: should cause a privbits exception if mask outside bounds
	u32 src = m_dma[23][2];
	u32 dst = m_dma[23][0];

	const u32 data = m_dma_read_cb(src);
	m_dma_write_cb(dst, data);

	count -= 4;
	src += 4;
	dst += 4;
	m_dma[23][0] = dst;
	m_dma[23][1] = count;
	m_dma[23][2] = src;

	m_dma_playerbus_timer->adjust(attotime::from_ticks(2, this->clock()));
}

