// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "machine/naomig1.h"

DEVICE_ADDRESS_MAP_START(amap, 32, naomi_g1_device)
	AM_RANGE(0x04, 0x07) AM_READWRITE(sb_gdstar_r, sb_gdstar_w)
	AM_RANGE(0x08, 0x0b) AM_READWRITE(sb_gdlen_r, sb_gdlen_w)
	AM_RANGE(0x0c, 0x0f) AM_READWRITE(sb_gddir_r, sb_gddir_w)
	AM_RANGE(0x14, 0x17) AM_READWRITE(sb_gden_r, sb_gden_w)
	AM_RANGE(0x18, 0x1b) AM_READWRITE(sb_gdst_r, sb_gdst_w)
	AM_RANGE(0x80, 0x83) AM_WRITE(sb_g1rrc_w)
	AM_RANGE(0x84, 0x87) AM_WRITE(sb_g1rwc_w)
	AM_RANGE(0x88, 0x8b) AM_WRITE(sb_g1frc_w)
	AM_RANGE(0x8c, 0x8f) AM_WRITE(sb_g1fwc_w)
	AM_RANGE(0x90, 0x93) AM_WRITE(sb_g1crc_w)
	AM_RANGE(0x94, 0x97) AM_WRITE(sb_g1cwc_w)
	AM_RANGE(0xa0, 0xa3) AM_WRITE(sb_g1gdrc_w)
	AM_RANGE(0xa4, 0xa7) AM_WRITE(sb_g1gdwc_w)
	AM_RANGE(0xb0, 0xb3) AM_READ(sb_g1sysm_r)
	AM_RANGE(0xb4, 0xb7) AM_WRITE(sb_g1crdyc_w)
	AM_RANGE(0xb8, 0xbb) AM_WRITE(sb_gdapro_w)
	AM_RANGE(0xf4, 0xf7) AM_READ(sb_gdstard_r)
	AM_RANGE(0xf8, 0xfb) AM_READ(sb_gdlend_r)
ADDRESS_MAP_END

naomi_g1_device::naomi_g1_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		irq_cb(*this)
{
}

void naomi_g1_device::device_start()
{
	timer = timer_alloc(G1_TIMER_ID);
	irq_cb.resolve_safe();

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
}

void naomi_g1_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	timer.adjust(attotime::never);
	if(!gdst)
		return;
	gdst = 0;
	irq_cb(DMA_GDROM_IRQ);
}

READ32_MEMBER(naomi_g1_device::sb_gdstar_r)
{
	return gdstar;
}

WRITE32_MEMBER(naomi_g1_device::sb_gdstar_w)
{
	COMBINE_DATA(&gdstar);
	logerror("G1: gdstar_w %08x @ %08x\n", data, mem_mask);
}

READ32_MEMBER(naomi_g1_device::sb_gdlen_r)
{
	return gdlen;
}

WRITE32_MEMBER(naomi_g1_device::sb_gdlen_w)
{
	COMBINE_DATA(&gdlen);
	logerror("G1: gdlen_w %08x @ %08x\n", data, mem_mask);
}

READ32_MEMBER(naomi_g1_device::sb_gddir_r)
{
	return gddir;
}

WRITE32_MEMBER(naomi_g1_device::sb_gddir_w)
{
	COMBINE_DATA(&gddir);
	gddir &= 1;
	logerror("G1: gddir_w %08x @ %08x\n", data, mem_mask);
}

READ32_MEMBER(naomi_g1_device::sb_gden_r)
{
	return gden;
}

WRITE32_MEMBER(naomi_g1_device::sb_gden_w)
{
	COMBINE_DATA(&gden);
	gden &= 1;
	logerror("G1: gden_w %08x @ %08x\n", data, mem_mask);
}

READ32_MEMBER(naomi_g1_device::sb_gdst_r)
{
	return gdst;
}

WRITE32_MEMBER(naomi_g1_device::sb_gdst_w)
{
	UINT32 old = gdst;
	COMBINE_DATA(&gdst);
	gdst &= 1;
	logerror("G1: gdst_w %08x @ %08x\n", data, mem_mask);
	if(!old && gdst && gden) {
		// DMA starts

		UINT32 adr = gdstar;
		UINT32 len = gdlen;

		// Deunan says round up to 32, doc says complete with zeroes.
		// Virtua Tennis requires one of the two to boot
		// We'll go with DK for now.
		//
		// In any case, low bit is ignored
		len = (len + 30) & ~30;

		bool to_mainram = true;
		while(len) {
			UINT8 *base;
			UINT32 limit = len;
			dma_get_position(base, limit, to_mainram);

			if(!limit)
				break;
			UINT32 tlen = limit > len ? len : limit;
			dma(base, adr, tlen, to_mainram);
			adr += tlen;
			len -= tlen;
			dma_advance(tlen);
		}

		while(len && to_mainram) {
			unsigned char zero[32];
			memset(zero, 0, sizeof(zero));
			UINT32 tlen = len > 32 ? 32 : len;
			dma(zero, adr, tlen, to_mainram);
			adr += tlen;
			len -= tlen;
		}

		/* 12x * 75 Hz = 0,00(1) secs per sector */
		/* TODO: make DMA to be single step */
		timer->adjust(attotime::from_usec(1111*(gdlen/2048)));
	}
}

WRITE32_MEMBER(naomi_g1_device::sb_g1rrc_w)
{
	logerror("G1: g1rrc_w %08x @ %08x\n", data, mem_mask);
}

WRITE32_MEMBER(naomi_g1_device::sb_g1rwc_w)
{
	logerror("G1: g1rwc_w %08x @ %08x\n", data, mem_mask);
}

WRITE32_MEMBER(naomi_g1_device::sb_g1crc_w)
{
	logerror("G1: g1crc_w %08x @ %08x\n", data, mem_mask);
}

WRITE32_MEMBER(naomi_g1_device::sb_g1cwc_w)
{
	logerror("G1: g1cwc_w %08x @ %08x\n", data, mem_mask);
}

WRITE32_MEMBER(naomi_g1_device::sb_g1frc_w)
{
	logerror("G1: g1frc_w %08x @ %08x\n", data, mem_mask);
}

WRITE32_MEMBER(naomi_g1_device::sb_g1fwc_w)
{
	logerror("G1: g1fwc_w %08x @ %08x\n", data, mem_mask);
}

WRITE32_MEMBER(naomi_g1_device::sb_g1gdrc_w)
{
	logerror("G1: g1gdrc_w %08x @ %08x\n", data, mem_mask);
}

WRITE32_MEMBER(naomi_g1_device::sb_g1gdwc_w)
{
	logerror("G1: g1gdwc_w %08x @ %08x\n", data, mem_mask);
}

READ32_MEMBER(naomi_g1_device::sb_g1sysm_r)
{
	logerror("G1: g1sysm_r @ %08x\n", mem_mask);
	return 0;
}

WRITE32_MEMBER(naomi_g1_device::sb_g1crdyc_w)
{
	logerror("G1: g1crdyc_w %08x @ %08x\n", data, mem_mask);
}

WRITE32_MEMBER(naomi_g1_device::sb_gdapro_w)
{
	logerror("G1: gdapro_w %08x @ %08x\n", data, mem_mask);
}

READ32_MEMBER(naomi_g1_device::sb_gdstard_r)
{
	logerror("G1: gdstard_r @ %08x\n", mem_mask);
	return 0;
}

READ32_MEMBER(naomi_g1_device::sb_gdlend_r)
{
	logerror("G1: gdlend_r @ %08x\n", mem_mask);
	return 0;
}

void naomi_g1_device::dma(void *dma_ptr, UINT32 main_adr, UINT32 size, bool to_mainram)
{
	if(!_dma_cb.isnull())
		_dma_cb(main_adr, dma_ptr, size >> 5, 32, to_mainram);
}
