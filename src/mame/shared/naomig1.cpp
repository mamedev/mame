// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "machine/naomig1.h"

void naomi_g1_device::amap(address_map &map)
{
	map(0x04, 0x07).rw(FUNC(naomi_g1_device::sb_gdstar_r), FUNC(naomi_g1_device::sb_gdstar_w));
	map(0x08, 0x0b).rw(FUNC(naomi_g1_device::sb_gdlen_r), FUNC(naomi_g1_device::sb_gdlen_w));
	map(0x0c, 0x0f).rw(FUNC(naomi_g1_device::sb_gddir_r), FUNC(naomi_g1_device::sb_gddir_w));
	map(0x14, 0x17).rw(FUNC(naomi_g1_device::sb_gden_r), FUNC(naomi_g1_device::sb_gden_w));
	map(0x18, 0x1b).rw(FUNC(naomi_g1_device::sb_gdst_r), FUNC(naomi_g1_device::sb_gdst_w));
	map(0x80, 0x83).w(FUNC(naomi_g1_device::sb_g1rrc_w));
	map(0x84, 0x87).w(FUNC(naomi_g1_device::sb_g1rwc_w));
	map(0x88, 0x8b).w(FUNC(naomi_g1_device::sb_g1frc_w));
	map(0x8c, 0x8f).w(FUNC(naomi_g1_device::sb_g1fwc_w));
	map(0x90, 0x93).w(FUNC(naomi_g1_device::sb_g1crc_w));
	map(0x94, 0x97).w(FUNC(naomi_g1_device::sb_g1cwc_w));
	map(0xa0, 0xa3).w(FUNC(naomi_g1_device::sb_g1gdrc_w));
	map(0xa4, 0xa7).w(FUNC(naomi_g1_device::sb_g1gdwc_w));
	map(0xb0, 0xb3).r(FUNC(naomi_g1_device::sb_g1sysm_r));
	map(0xb4, 0xb7).w(FUNC(naomi_g1_device::sb_g1crdyc_w));
	map(0xb8, 0xbb).w(FUNC(naomi_g1_device::sb_gdapro_w));
	map(0xf4, 0xf7).r(FUNC(naomi_g1_device::sb_gdstard_r));
	map(0xf8, 0xfb).r(FUNC(naomi_g1_device::sb_gdlend_r));
}

naomi_g1_device::naomi_g1_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, irq_cb(*this)
	, ext_irq_cb(*this)
	, reset_out_cb(*this)
{
}

void naomi_g1_device::device_start()
{
	timer = timer_alloc(FUNC(naomi_g1_device::trigger_gdrom_irq), this);
	irq_cb.resolve_safe();
	ext_irq_cb.resolve_safe();
	reset_out_cb.resolve_safe();

	save_item(NAME(gdstar));
	save_item(NAME(gdlen));
	save_item(NAME(gddir));
	save_item(NAME(gden));
	save_item(NAME(gdst));
}

void naomi_g1_device::device_reset()
{
	gdstar = 0;
	gdlen = 0;
	gddir = 0;
	gden = 0;
	gdst = 0;
	set_ext_irq(CLEAR_LINE);
}

TIMER_CALLBACK_MEMBER(naomi_g1_device::trigger_gdrom_irq)
{
	if(!gdst)
		return;
	gdst = 0;
	irq_cb(DMA_GDROM_IRQ);
}

uint32_t naomi_g1_device::sb_gdstar_r()
{
	return gdstar;
}

void naomi_g1_device::sb_gdstar_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&gdstar);
	logerror("G1: gdstar_w %08x @ %08x\n", data, mem_mask);
}

uint32_t naomi_g1_device::sb_gdlen_r()
{
	return gdlen;
}

void naomi_g1_device::sb_gdlen_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&gdlen);
	logerror("G1: gdlen_w %08x @ %08x\n", data, mem_mask);
}

uint32_t naomi_g1_device::sb_gddir_r()
{
	return gddir;
}

void naomi_g1_device::sb_gddir_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&gddir);
	gddir &= 1;
	logerror("G1: gddir_w %08x @ %08x\n", data, mem_mask);
}

uint32_t naomi_g1_device::sb_gden_r()
{
	return gden;
}

void naomi_g1_device::sb_gden_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&gden);
	gden &= 1;
	logerror("G1: gden_w %08x @ %08x\n", data, mem_mask);
}

uint32_t naomi_g1_device::sb_gdst_r()
{
	return gdst;
}

void naomi_g1_device::sb_gdst_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t old = gdst;
	COMBINE_DATA(&gdst);
	gdst &= 1;
	logerror("G1: gdst_w %08x @ %08x\n", data, mem_mask);
	if(!old && gdst && gden) {
		// DMA starts

		uint32_t adr = gdstar;
		uint32_t len = gdlen;

		// Deunan says round up to 32, doc says complete with zeroes.
		// Virtua Tennis requires one of the two to boot
		// We'll go with DK for now.
		//
		// In any case, low bit is ignored
		len = (len + 30) & ~30;

		bool to_mainram = true;
		while(len) {
			uint8_t *base;
			uint32_t limit = len;
			dma_get_position(base, limit, to_mainram);

			if(!limit)
				break;
			uint32_t tlen = limit > len ? len : limit;
			dma(base, adr, tlen, to_mainram);
			adr += tlen;
			len -= tlen;
			dma_advance(tlen);
		}

		while(len && to_mainram) {
			unsigned char zero[32];
			memset(zero, 0, sizeof(zero));
			uint32_t tlen = len > 32 ? 32 : len;
			dma(zero, adr, tlen, to_mainram);
			adr += tlen;
			len -= tlen;
		}

		/* 12x * 75 Hz = 0,00(1) secs per sector */
		/* TODO: make DMA to be single step */
		timer->adjust(attotime::from_usec(1111*(gdlen/2048)));
	}
}

void naomi_g1_device::sb_g1rrc_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	logerror("G1: g1rrc_w %08x @ %08x\n", data, mem_mask);
}

void naomi_g1_device::sb_g1rwc_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	logerror("G1: g1rwc_w %08x @ %08x\n", data, mem_mask);
}

void naomi_g1_device::sb_g1crc_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	logerror("G1: g1crc_w %08x @ %08x\n", data, mem_mask);
}

void naomi_g1_device::sb_g1cwc_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	logerror("G1: g1cwc_w %08x @ %08x\n", data, mem_mask);
}

void naomi_g1_device::sb_g1frc_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	logerror("G1: g1frc_w %08x @ %08x\n", data, mem_mask);
}

void naomi_g1_device::sb_g1fwc_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	logerror("G1: g1fwc_w %08x @ %08x\n", data, mem_mask);
}

void naomi_g1_device::sb_g1gdrc_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	logerror("G1: g1gdrc_w %08x @ %08x\n", data, mem_mask);
}

void naomi_g1_device::sb_g1gdwc_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	logerror("G1: g1gdwc_w %08x @ %08x\n", data, mem_mask);
}

uint32_t naomi_g1_device::sb_g1sysm_r(offs_t offset, uint32_t mem_mask)
{
	logerror("G1: g1sysm_r @ %08x\n", mem_mask);
	return 0;
}

void naomi_g1_device::sb_g1crdyc_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	logerror("G1: g1crdyc_w %08x @ %08x\n", data, mem_mask);
}

void naomi_g1_device::sb_gdapro_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	logerror("G1: gdapro_w %08x @ %08x\n", data, mem_mask);
}

uint32_t naomi_g1_device::sb_gdstard_r(offs_t offset, uint32_t mem_mask)
{
	logerror("G1: gdstard_r @ %08x\n", mem_mask);
	return 0;
}

uint32_t naomi_g1_device::sb_gdlend_r(offs_t offset, uint32_t mem_mask)
{
	logerror("G1: gdlend_r @ %08x\n", mem_mask);
	return 0;
}

void naomi_g1_device::dma(void *dma_ptr, uint32_t main_adr, uint32_t size, bool to_mainram)
{
	if(!_dma_cb.isnull())
		_dma_cb(main_adr, dma_ptr, size >> 5, 32, to_mainram);
}
