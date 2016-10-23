// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "i82371sb.h"

const device_type I82371SB_ISA = &device_creator<i82371sb_isa_device>;

DEVICE_ADDRESS_MAP_START(config_map, 32, i82371sb_isa_device)
	AM_RANGE(0x4c, 0x4f) AM_READWRITE8 (iort_r,    iort_w,    0x000000ff)
	AM_RANGE(0x4c, 0x4f) AM_READWRITE16(xbcs_r,    xbcs_w,    0xffff0000)
	AM_RANGE(0x60, 0x63) AM_READWRITE8 (pirqrc_r,  pirqrc_w,  0xffffffff)
	AM_RANGE(0x68, 0x6b) AM_READWRITE8 (tom_r,     tom_w,     0x000000ff)
	AM_RANGE(0x68, 0x6b) AM_READWRITE16(mstat_r,   mstat_w,   0xffff0000)
	AM_RANGE(0x70, 0x73) AM_READWRITE8 (mbirq0_r,  mbirq0_w,  0x000000ff)
	AM_RANGE(0x74, 0x77) AM_READWRITE8 (mbdma_r,   mbdma_w,   0xffff0000)
	AM_RANGE(0x80, 0x83) AM_READWRITE8 (dlc_r,     dlc_w,     0x00ff0000)
	AM_RANGE(0xa0, 0xa3) AM_READWRITE8 (smicntl_r, smicntl_w, 0x000000ff)
	AM_RANGE(0xa0, 0xa3) AM_READWRITE16(smien_r,   smien_w,   0xffff0000)
	AM_RANGE(0xa4, 0xa7) AM_READWRITE  (see_r,     see_w)
	AM_RANGE(0xa8, 0xab) AM_READWRITE8 (ftmr_r,    ftmr_w,    0x000000ff)
	AM_RANGE(0xa8, 0xab) AM_READWRITE16(smireq_r,  smireq_w,  0xffff0000)
	AM_RANGE(0xac, 0xaf) AM_READWRITE8 (ctltmr_r,  ctltmr_w,  0x000000ff)
	AM_RANGE(0xac, 0xaf) AM_READWRITE8 (cthtmr_r,  cthtmr_w,  0x00ff0000)

	AM_INHERIT_FROM(pci_device::config_map)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START(internal_io_map, 32, i82371sb_isa_device)
//  AM_RANGE(0x0060, 0x0063) AM_READ8      (reset_irq12_1_r,           0x000000ff)
//  AM_RANGE(0x0060, 0x0063) AM_READWRITE8 (nmi_st_r,        nmi_st_w, 0x0000ff00)
	AM_RANGE(0x0080, 0x0083) AM_WRITE8     (boot_state_w,              0x000000ff) // POST/non-existing, used for delays by the bios/os
	AM_RANGE(0x00ec, 0x00ef) AM_WRITE8     (nop_w,                     0x0000ff00) // Non-existing, used for delays by the bios/os
ADDRESS_MAP_END

i82371sb_isa_device::i82371sb_isa_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, I82371SB_ISA, "i82371sb southbridge ISA bridge", tag, owner, clock, "i82371sb_isa", __FILE__),
	  m_boot_state_hook(*this)
{
}

void i82371sb_isa_device::device_start()
{
	pci_device::device_start();
	m_boot_state_hook.resolve();
}

void i82371sb_isa_device::device_reset()
{
	pci_device::device_reset();

	iort = 0x4d;
	xbcs = 0x0003;
	memset(pirqrc, 0x80, sizeof(pirqrc));
	tom = 0x02;
	mstat = 0x0000;
	mbirq0 = 0x80;
	memset(mbdma, 0x0c, sizeof(mbdma));
	pcsc = 0x0002;
	apicbase = 0x00;
	dlc = 0x00;
	smicntl = 0x08;
	smien = 0x0000;
	see = 0x00000000;
	ftmr = 0x0f;
	smireq = 0x0000;
	ctlmtr = 0x00;
	cthmtr = 0x00;
}

void i82371sb_isa_device::reset_all_mappings()
{
	pci_device::reset_all_mappings();
}

void i82371sb_isa_device::boot_state_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_boot_state_hook((offs_t)0, data);
}

void i82371sb_isa_device::nop_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
}

uint8_t i82371sb_isa_device::iort_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return iort;
}

void i82371sb_isa_device::iort_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	iort = data;
	logerror("iort = %02x\n", iort);
}

uint16_t i82371sb_isa_device::xbcs_r(address_space &space, offs_t offset, uint16_t mem_mask)
{
	return xbcs;
}

void i82371sb_isa_device::xbcs_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&xbcs);
	logerror("xbcs = %04x\n", xbcs);
}

uint8_t i82371sb_isa_device::pirqrc_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return pirqrc[offset];
}

void i82371sb_isa_device::pirqrc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	pirqrc[offset] = data;
	logerror("pirqrc[%d] = %02x\n", offset, pirqrc[offset]);
}

uint8_t i82371sb_isa_device::tom_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return tom;
}

void i82371sb_isa_device::tom_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	tom = data;
	logerror("tom = %02x\n", tom);
}

uint16_t i82371sb_isa_device::mstat_r(address_space &space, offs_t offset, uint16_t mem_mask)
{
	return mstat;
}

void i82371sb_isa_device::mstat_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&mstat);
	logerror("mstat = %04x\n", mstat);
}

uint8_t i82371sb_isa_device::mbirq0_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return mbirq0;
}

void i82371sb_isa_device::mbirq0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	mbirq0 = data;
	logerror("mbirq0 = %02x\n", mbirq0);
}

uint8_t i82371sb_isa_device::mbdma_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return mbdma[offset];
}

void i82371sb_isa_device::mbdma_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	mbdma[offset] = data;
	logerror("mbdma[%d] = %02x\n", offset, mbdma);
}

uint8_t i82371sb_isa_device::apicbase_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return apicbase;
}

void i82371sb_isa_device::apicbase_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	apicbase = data;
	logerror("apicbase = %02x\n", apicbase);
}

uint8_t i82371sb_isa_device::dlc_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return dlc;
}

void i82371sb_isa_device::dlc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	dlc = data;
	logerror("dlc = %02x\n", dlc);
}

uint8_t i82371sb_isa_device::smicntl_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return smicntl;
}

void i82371sb_isa_device::smicntl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	smicntl = data;
	logerror("smicntl = %02x\n", smicntl);
}

uint16_t i82371sb_isa_device::smien_r(address_space &space, offs_t offset, uint16_t mem_mask)
{
	return smien;
}

void i82371sb_isa_device::smien_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&smien);
	logerror("smien = %04x\n", smien);
}

uint32_t i82371sb_isa_device::see_r(address_space &space, offs_t offset, uint32_t mem_mask)
{
	return see;
}

void i82371sb_isa_device::see_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&see);
	logerror("see = %08x\n", see);
}

uint8_t i82371sb_isa_device::ftmr_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return ftmr;
}

void i82371sb_isa_device::ftmr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	ftmr = data;
	logerror("ftmr = %02x\n", ftmr);
}

uint16_t i82371sb_isa_device::smireq_r(address_space &space, offs_t offset, uint16_t mem_mask)
{
	return smireq;
}

void i82371sb_isa_device::smireq_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&smireq);
	logerror("smireq = %04x\n", smireq);
}

uint8_t i82371sb_isa_device::ctltmr_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return ctlmtr;
}

void i82371sb_isa_device::ctltmr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	ctlmtr = data;
	logerror("ctlmtr = %02x\n", ctlmtr);
}

uint8_t i82371sb_isa_device::cthtmr_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return cthmtr;
}

void i82371sb_isa_device::cthtmr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	cthmtr = data;
	logerror("cthmtr = %02x\n", cthmtr);
}





void i82371sb_isa_device::map_bios(address_space *memory_space, uint32_t start, uint32_t end)
{
	uint32_t mask = m_region->bytes() - 1;
	memory_space->install_rom(start, end, m_region->base() + (start & mask));
}

void i82371sb_isa_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
									uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	map_bios(memory_space, 0xfffe0000, 0xffffffff);
	map_bios(memory_space, 0x000e0000, 0x000fffff);

	io_space->install_device(0, 0xffff, *this, &i82371sb_isa_device::internal_io_map);

#if 0
	if(fwh_dec_en1 & 0x80) {
		map_bios(memory_space, 0xfff80000, 0xffffffff, 7);
		map_bios(memory_space, 0xffb80000, 0xffbfffff, 7);
		map_bios(memory_space, 0x000e0000, 0x000fffff, 7);
	}
	if(fwh_dec_en1 & 0x40) {
		map_bios(memory_space, 0xfff00000, 0xfff7ffff, 6);
		map_bios(memory_space, 0xffb00000, 0xffb7ffff, 6);
	}
	if(fwh_dec_en1 & 0x20) {
		map_bios(memory_space, 0xffe80000, 0xffefffff, 5);
		map_bios(memory_space, 0xffa80000, 0xffafffff, 5);
	}
	if(fwh_dec_en1 & 0x10) {
		map_bios(memory_space, 0xffe00000, 0xffe7ffff, 4);
		map_bios(memory_space, 0xffa00000, 0xffa7ffff, 4);
	}
	if(fwh_dec_en1 & 0x08) {
		map_bios(memory_space, 0xffd80000, 0xffdfffff, 3);
		map_bios(memory_space, 0xff980000, 0xff9fffff, 3);
	}
	if(fwh_dec_en1 & 0x04) {
		map_bios(memory_space, 0xffd00000, 0xffd7ffff, 2);
		map_bios(memory_space, 0xff900000, 0xff97ffff, 2);
	}
	if(fwh_dec_en1 & 0x02) {
		map_bios(memory_space, 0xffc80000, 0xffcfffff, 1);
		map_bios(memory_space, 0xff880000, 0xff8fffff, 1);
	}
	if(fwh_dec_en1 & 0x01) {
		map_bios(memory_space, 0xffc00000, 0xffc7ffff, 0);
		map_bios(memory_space, 0xff800000, 0xff87ffff, 0);
	}


	if(acpi_cntl & 0x10)
		acpi->map_device(memory_window_start, memory_window_end, 0, memory_space, io_window_start, io_window_end, pmbase, io_space);
	if(gpio_cntl & 0x10)
		logerror("%s: Warning: gpio range enabled at %04x-%04x\n", tag(), gpio_base, gpio_base+63);

	uint32_t hpet = 0xfed00000 + ((gen_cntl & 0x00018000) >> 3);
	logerror("%s: Warning: hpet at %08x-%08x\n", tag(), hpet, hpet+0x3ff);

	if(lpc_en & 0x1000)
		logerror("%s: Warning: superio at 2e-2f\n", tag());
	if(lpc_en & 0x0800)
		logerror("%s: Warning: mcu at 62/66\n", tag());
	if(lpc_en & 0x0400)
		logerror("%s: Warning: mcu at 60/64\n", tag());
	if(lpc_en & 0x0200)
		logerror("%s: Warning: gameport at 208-20f\n", tag());
	if(lpc_en & 0x0100)
		logerror("%s: Warning: gameport at 200-207\n", tag());

	if(lpc_en & 0x0008) {
		uint16_t fdc = lpc_if_fdd_lpt_range & 0x10 ? 0x370 : 0x3f0;
		logerror("%s: Warning: floppy at %04x-%04x\n", tag(), fdc, fdc+7);
	}

	if(lpc_en & 0x0004) {
		static const uint16_t lpt_pos[4] = { 0x378, 0x278, 0x3bc, 0x000 };
		uint16_t lpt = lpt_pos[lpc_if_fdd_lpt_range & 3];
		if(lpt)
			logerror("%s: Warning: lpt at %04x-%04x %04x-%04x\n", tag(), lpt, lpt+7, lpt+0x400, lpt+0x407);
	}

	static const uint16_t com_pos[8] = { 0x3f8, 0x2f8, 0x220, 0x228, 0x238, 0x2e8, 0x338, 0x3e8 };

	if(lpc_en & 0x0002) {
		uint16_t comb = com_pos[(lpc_if_com_range >> 4) & 7];
		logerror("%s: Warning: comb at %04x-%04x\n", tag(), comb, comb+7);
	}

	if(lpc_en & 0x0001) {
		uint16_t coma = com_pos[lpc_if_com_range & 7];
		logerror("%s: Warning: coma at %04x-%04x\n", tag(), coma, coma+7);
	}

	rtc->map_device(memory_window_start, memory_window_end, 0, memory_space, io_window_start, io_window_end, 0, io_space);
	if(rtc_conf & 4)
		rtc->map_extdevice(memory_window_start, memory_window_end, 0, memory_space, io_window_start, io_window_end, 0, io_space);
	pit->map_device(memory_window_start, memory_window_end, 0, memory_space, io_window_start, io_window_end, 0, io_space);
#endif
}
