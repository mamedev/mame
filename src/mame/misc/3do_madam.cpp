// license:LGPL-2.1+
// copyright-holders:Angelo Salese, Wilbert Pol

#include "emu.h"
#include "3do_madam.h"

DEFINE_DEVICE_TYPE(MADAM, madam_device, "madam", "3DO MN7A020UDA \"Madam\" Address Decoder")

madam_device::madam_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MADAM, tag, owner, clock)
	, m_diag_cb(*this)
{
}

void madam_device::device_add_mconfig(machine_config &config)
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
}

void madam_device::device_reset()
{
}

// $0330'0000 base
void madam_device::map(address_map &map)
{
	map(0x0000, 0x0003).lrw32(
		NAME([this] () { return m_revision; }),
		// echo for terminal?
		NAME([this] (u32 data) {
			m_diag_cb(data & 0xff);
		})
	);

	// 03300004 - Memory configuration 29 = 2MB DRAM, 1MB VRAM
	map(0x0004, 0x0007).lrw32(
		NAME([this] () { return m_msysbits; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_msysbits); })
	);
	map(0x0008, 0x000b).lrw32(
		NAME([this] () { return m_mctl; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_mctl); })
	);
	map(0x000c, 0x000f).lrw32(
		NAME([this] () { return m_sltime; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_sltime); })
	);
	map(0x0020, 0x0023).lrw32(
		NAME([this] () { return m_abortbits; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_abortbits); })
	);
	map(0x0024, 0x0027).lrw32(
		NAME([this] () { return m_privbits; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_privbits); })
	);
	map(0x0028, 0x002b).lrw32(
		NAME([this] () { return m_statbits; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_statbits); })
	);
	// 0x0040, 0x0047 madam diag 0/1
	map(0x0040, 0x0043).lrw32(
		NAME([this] () { return m_diag; }),
		// TODO: correct?
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { m_diag = 1; })
	);
	// 0x0044: Anvil "feature"

	// CEL engine
//	map(0x0100, 0x0103)  SPRSTRT - Start the CEL engine (W)
//	map(0x0104, 0x0107)  SPRSTOP - Stop the CEL engine (W)
//	map(0x0108, 0x010b)  SPRCNTU - Continue the CEL engine (W)
//	map(0x010c, 0x010f)  SPRPAUS - Pause the CEL engine (W)
	map(0x0110, 0x0113).lrw32(
		NAME([this] () { return m_ccobctl0; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_ccobctl0); })
	);
	// TODO: writes were 0x0129/4, typing mistake?
	map(0x0120, 0x0123).lrw32(
		NAME([this] () { return m_ppmpc; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_ppmpc); })
	);

	// Regis
	// Regis control word
	map(0x0130, 0x0133).lrw32(
		NAME([this] () { return m_regctl0; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_regctl0); })
	);
	// X and Y clip values
	map(0x0134, 0x0137).lrw32(
		NAME([this] () { return m_regctl1; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_regctl1); })
	);
	// Read base address
	map(0x0138, 0x013b).lrw32(
		NAME([this] () { return m_regctl2; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_regctl2); })
	);
	// Write base address
	map(0x013c, 0x013f).lrw32(
		NAME([this] () { return m_regctl3; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_regctl3); })
	);
	map(0x0140, 0x0143).lrw32(
		NAME([this] () { return m_xyposh; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_xyposh); })
	);
	map(0x0144, 0x0147).lrw32(
		NAME([this] () { return m_xyposl; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_xyposl); })
	);
	map(0x0148, 0x014b).lrw32(
		NAME([this] () { return m_linedxyh; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_linedxyh); })
	);
	map(0x014c, 0x014f).lrw32(
		NAME([this] () { return m_linedxyl; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_linedxyl); })
	);
	map(0x0150, 0x0153).lrw32(
		NAME([this] () { return m_dxyh; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_dxyh); })
	);
	map(0x0154, 0x0157).lrw32(
		NAME([this] () { return m_dxyl; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_dxyl); })
	);
	map(0x0158, 0x015b).lrw32(
		NAME([this] () { return m_ddxyh; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_ddxyh); })
	);
	map(0x015c, 0x015f).lrw32(
		NAME([this] () { return m_ddxyl; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_ddxyl); })
	);
	// PIP: Pen Index Palette (or PLT: Palette Look-up Table)
	// Reads are split in words, but writes are dword (cutting at 0x1bf)
	// TODO: does writing to 0x1c0-0x1ff go to mirror or they are just ignored
	map(0x0180, 0x01ff).lrw32(
		NAME([this] (offs_t offset) {
			const u16 reg = offset >> 1;
			if (offset & 1)
				return m_pip[reg & 0x0f] >> 16;

			return m_pip[reg & 0x0f] & 0xffff;
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_pip[offset & 0xf]);
		})
	);
	// part 1 of fence
	map(0x0218, 0x021f).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_fence[offset]); })
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
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_fence[offset | 2]); })
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
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_mmu[offset & 0x3f]); })
	);
	map(0x0400, 0x05ff).lrw32(
		NAME([this] (offs_t offset) {
			const u16 channel = offset >> 2;
			return m_dma[channel & 0x1f][offset & 0x03];
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			const u16 channel = offset >> 2;
			COMBINE_DATA(&m_dma[channel & 0x1f][offset & 0x03]);
		})
	);
	/* Hardware multiplier */
	map(0x0600, 0x069f).lrw32(
		NAME([this] (offs_t offset) {
			return m_mult[offset & 0x3f];
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_mult[offset & 0x3f]);
		})
	);
	map(0x07f0, 0x07f3).lrw32(
		NAME([this] () { return m_mult_control; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { m_mult_control |= data; })
	);
	map(0x07f4, 0x07f7).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { m_mult_control &= ~data; })
	);
	map(0x07f8, 0x07fb).lr32(NAME([this] () { return m_mult_status; }));
//	map(0x07fc, 0x07ff) start process
}

