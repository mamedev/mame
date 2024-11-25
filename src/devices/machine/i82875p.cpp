// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "i82875p.h"

DEFINE_DEVICE_TYPE(I82875P_HOST,     i82875p_host_device,     "i82875p_host",     "Intel 82875P Northbridge")
DEFINE_DEVICE_TYPE(I82875P_AGP,      i82875p_agp_device,      "i82875p_agp",      "Intel 82875P AGP Bridge")
DEFINE_DEVICE_TYPE(I82875P_OVERFLOW, i82875p_overflow_device, "i82875p_overflow", "Intel 82875P Configuration Overflow")

void i82875p_host_device::agp_translation_map(address_map &map)
{
}

void i82875p_host_device::config_map(address_map &map)
{
	pci_host_device::config_map(map);
	map(0x51, 0x51).rw(FUNC(i82875p_host_device::agpm_r), FUNC(i82875p_host_device::agpm_w));
	map(0x52, 0x52).r(FUNC(i82875p_host_device::gc_r));
	map(0x53, 0x53).r(FUNC(i82875p_host_device::csabcont_r));
	map(0x58, 0x5b).r(FUNC(i82875p_host_device::eap_r));
	map(0x5c, 0x5c).r(FUNC(i82875p_host_device::derrsyn_r));
	map(0x5d, 0x5d).r(FUNC(i82875p_host_device::des_r));
	map(0x60, 0x60).rw(FUNC(i82875p_host_device::fpllcont_r), FUNC(i82875p_host_device::fpllcont_w));
	map(0x90, 0x97).rw(FUNC(i82875p_host_device::pam_r), FUNC(i82875p_host_device::pam_w));
	map(0x9d, 0x9d).rw(FUNC(i82875p_host_device::smram_r), FUNC(i82875p_host_device::smram_w));
	map(0x9e, 0x9e).rw(FUNC(i82875p_host_device::esmramc_r), FUNC(i82875p_host_device::esmramc_w));
	map(0xa0, 0xa3).r(FUNC(i82875p_host_device::acapid_r));
	map(0xa4, 0xa7).r(FUNC(i82875p_host_device::agpstat_r));
	map(0xa8, 0xab).r(FUNC(i82875p_host_device::agpcmd_r));
	map(0xb0, 0xb3).rw(FUNC(i82875p_host_device::agpctrl_r), FUNC(i82875p_host_device::agpctrl_w));
	map(0xb4, 0xb4).rw(FUNC(i82875p_host_device::apsize_r), FUNC(i82875p_host_device::apsize_w));
	map(0xb8, 0xbb).rw(FUNC(i82875p_host_device::attbase_r), FUNC(i82875p_host_device::attbase_w));
	map(0xbc, 0xbc).rw(FUNC(i82875p_host_device::amtt_r), FUNC(i82875p_host_device::amtt_w));
	map(0xbd, 0xbd).rw(FUNC(i82875p_host_device::lptt_r), FUNC(i82875p_host_device::lptt_w));
	map(0xc4, 0xc5).rw(FUNC(i82875p_host_device::toud_r), FUNC(i82875p_host_device::toud_w));
	map(0xc6, 0xc7).rw(FUNC(i82875p_host_device::mchcfg_r), FUNC(i82875p_host_device::mchcfg_w));
	map(0xc8, 0xc9).r(FUNC(i82875p_host_device::errsts_r));
	map(0xca, 0xcb).rw(FUNC(i82875p_host_device::errcmd_r), FUNC(i82875p_host_device::errcmd_w));
	map(0xcc, 0xcd).rw(FUNC(i82875p_host_device::smicmd_r), FUNC(i82875p_host_device::smicmd_w));
	map(0xce, 0xcf).rw(FUNC(i82875p_host_device::scicmd_r), FUNC(i82875p_host_device::scicmd_w));
	map(0xde, 0xdf).rw(FUNC(i82875p_host_device::skpd_r), FUNC(i82875p_host_device::skpd_w));
	map(0xe4, 0xe7).r(FUNC(i82875p_host_device::capreg1_r));
	map(0xe8, 0xe8).r(FUNC(i82875p_host_device::capreg2_r));
}

i82875p_host_device::i82875p_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_host_device(mconfig, I82875P_HOST, tag, owner, clock)
	, cpu(*this, finder_base::DUMMY_TAG)
{
}

void i82875p_host_device::set_ram_size(int _ram_size)
{
	ram_size = _ram_size;
}

uint8_t i82875p_host_device::capptr_r()
{
	return 0xe4;
}

void i82875p_host_device::device_start()
{
	pci_host_device::device_start();
	set_spaces(&cpu->space(AS_PROGRAM), &cpu->space(AS_IO));

	memory_window_start = 0;
	memory_window_end   = 0xffffffff;
	memory_offset       = 0;
	io_window_start = 0;
	io_window_end   = 0xffff;
	io_offset       = 0;
	status = 0x0010;

	ram.resize(ram_size/4);

	// Resizeable with the apsize register
	add_map(256*1024*1024, M_MEM, FUNC(i82875p_host_device::agp_translation_map));
}

uint8_t i82875p_host_device::agpm_r()
{
	return agpm;
}

void i82875p_host_device::agpm_w(uint8_t data)
{
	agpm = data;
	logerror("%s: agpm = %02x\n", tag(), agpm);
}

uint8_t i82875p_host_device::gc_r()
{
	return 0x08;
}

uint8_t i82875p_host_device::csabcont_r()
{
	return 0x00;
}

uint32_t i82875p_host_device::eap_r()
{
	return 0x00000000;
}

uint8_t i82875p_host_device::derrsyn_r()
{
	return 0x00;
}

uint8_t i82875p_host_device::des_r()
{
	return 0x00;
}

uint8_t i82875p_host_device::fpllcont_r()
{
	return fpllcont;
}

void i82875p_host_device::fpllcont_w(uint8_t data)
{
	fpllcont = data;
	logerror("%s: fpllcont = %02x\n", tag(), data);
}

uint8_t i82875p_host_device::pam_r(offs_t offset)
{
	return pam[offset];
}

void i82875p_host_device::pam_w(offs_t offset, uint8_t data)
{
	pam[offset] = data;
	logerror("%s: pam[%d] = %02x\n", tag(), offset, data);
	remap_cb();
}

uint8_t i82875p_host_device::smram_r()
{
	return smram;
}

void i82875p_host_device::smram_w(uint8_t data)
{
	if(!(smram & 0x10))
		smram = (data & 0xfe) | 0x02;
	logerror("%s: smram = %02x\n", tag(), smram);
	remap_cb();
}

uint8_t i82875p_host_device::esmramc_r()
{
	return esmramc;
}

void i82875p_host_device::esmramc_w(uint8_t data)
{
	if(!(smram & 0x10))
		esmramc = (data & 0x87) | 0x38;
	logerror("%s: esmramc = %02x\n", tag(), smram);
	remap_cb();
}

uint32_t i82875p_host_device::acapid_r()
{
	return 0x00300002;
}

uint32_t i82875p_host_device::agpstat_r()
{
	return 0x1f004a13;
}

uint32_t i82875p_host_device::agpcmd_r()
{
	return 0x00000a00;
}

uint32_t i82875p_host_device::agpctrl_r()
{
	return agpctrl;
}

void i82875p_host_device::agpctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&agpctrl);
	logerror("%s: agpctrl = %08x\n", tag(), agpctrl);
}

uint8_t i82875p_host_device::apsize_r()
{
	return apsize;
}

void i82875p_host_device::apsize_w(uint8_t data)
{
	apsize = data;
	logerror("%s: apsize = %02x\n", tag(), apsize);
}

uint32_t i82875p_host_device::attbase_r()
{
	return attbase;
}

void i82875p_host_device::attbase_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&attbase);
	logerror("%s: attbase = %08x\n", tag(), attbase);
}

uint8_t i82875p_host_device::amtt_r()
{
	return amtt;
}

void i82875p_host_device::amtt_w(uint8_t data)
{
	amtt = data;
}

uint8_t i82875p_host_device::lptt_r()
{
	return lptt;
}

void i82875p_host_device::lptt_w(uint8_t data)
{
	lptt = data;
}

uint16_t i82875p_host_device::toud_r()
{
	return toud;
}

void i82875p_host_device::toud_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&toud);
	toud &= ~7;
	logerror("%s: toud = %08x\n", tag(), toud << 16);
	remap_cb();
}

uint16_t i82875p_host_device::mchcfg_r()
{
	return mchcfg;
}

void i82875p_host_device::mchcfg_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&mchcfg);
}

uint16_t i82875p_host_device::errsts_r()
{
	return 0x0000;
}

uint16_t i82875p_host_device::errcmd_r()
{
	return errcmd;
}

void i82875p_host_device::errcmd_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&errcmd);
}

uint16_t i82875p_host_device::smicmd_r()
{
	return smicmd;
}

void i82875p_host_device::smicmd_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&smicmd);
}

uint16_t i82875p_host_device::scicmd_r()
{
	return scicmd;
}

void i82875p_host_device::scicmd_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&scicmd);
}

uint16_t i82875p_host_device::skpd_r()
{
	return skpd;
}

void i82875p_host_device::skpd_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&skpd);
}

uint32_t i82875p_host_device::capreg1_r()
{
	return 0x0106a009;
}

uint8_t i82875p_host_device::capreg2_r()
{
	return 0x00;
}

void i82875p_host_device::reset_all_mappings()
{
	pci_host_device::reset_all_mappings();

	toud = 0x0400;
	smram = 0x02;
	esmramc = 0x38;
	memset(pam, 0, sizeof(pam));
}

void i82875p_host_device::device_reset()
{
	pci_host_device::device_reset();

	agpm = 0x00;
	fpllcont = 0x00;
	agpctrl = 0x00000000;
	apsize = 0x00;
	attbase = 0x00000000;
	amtt = 0x10;
	lptt = 0x10;
	mchcfg = 0x0000;
	errcmd = 0x0000;
	smicmd = 0x0000;
	scicmd = 0x0000;
	skpd = 0x0000;
}

void i82875p_host_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
									uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	io_space->install_device(0, 0xffff, *static_cast<pci_host_device *>(this), &pci_host_device::io_configuration_access_map);

	uint32_t top = toud << 16;
	if(esmramc & 1) {
		switch((esmramc >> 1) & 3) {
		case 2: top += 512*1024; break;
		case 3: top += 1024*1024; break;
		}
	}

	if(top > ram_size)
		top = ram_size;

	memory_space->install_ram          (0x00000000, 0x0009ffff, &ram[0x00000000/4]);

	if(smram & 0x40)
		memory_space->install_ram      (0x000a0000, 0x000bffff, &ram[0x000a0000/4]);

	if(pam[1] & 0x01)
		memory_space->install_rom      (0x000c0000, 0x000c3fff, &ram[0x000c0000/4]);
	if(pam[1] & 0x02)
		memory_space->install_writeonly(0x000c0000, 0x000c3fff, &ram[0x000c0000/4]);
	if(pam[1] & 0x10)
		memory_space->install_rom      (0x000c4000, 0x000c7fff, &ram[0x000c4000/4]);
	if(pam[1] & 0x20)
		memory_space->install_writeonly(0x000c4000, 0x000c7fff, &ram[0x000c4000/4]);
	if(pam[2] & 0x01)
		memory_space->install_rom      (0x000c8000, 0x000cbfff, &ram[0x000c8000/4]);
	if(pam[2] & 0x02)
		memory_space->install_writeonly(0x000c8000, 0x000cbfff, &ram[0x000c8000/4]);
	if(pam[2] & 0x10)
		memory_space->install_rom      (0x000cc000, 0x000cffff, &ram[0x000cc000/4]);
	if(pam[2] & 0x20)
		memory_space->install_writeonly(0x000cc000, 0x000cffff, &ram[0x000cc000/4]);
	if(pam[3] & 0x01)
		memory_space->install_rom      (0x000d0000, 0x000d3fff, &ram[0x000d0000/4]);
	if(pam[3] & 0x02)
		memory_space->install_writeonly(0x000d0000, 0x000d3fff, &ram[0x000d0000/4]);
	if(pam[3] & 0x10)
		memory_space->install_rom      (0x000d4000, 0x000d7fff, &ram[0x000d4000/4]);
	if(pam[3] & 0x20)
		memory_space->install_writeonly(0x000d4000, 0x000d7fff, &ram[0x000d4000/4]);
	if(pam[4] & 0x01)
		memory_space->install_rom      (0x000d8000, 0x000dbfff, &ram[0x000d8000/4]);
	if(pam[4] & 0x02)
		memory_space->install_writeonly(0x000d8000, 0x000dbfff, &ram[0x000d8000/4]);
	if(pam[4] & 0x10)
		memory_space->install_rom      (0x000dc000, 0x000dffff, &ram[0x000dc000/4]);
	if(pam[4] & 0x20)
		memory_space->install_writeonly(0x000dc000, 0x000dffff, &ram[0x000dc000/4]);
	if(pam[5] & 0x01)
		memory_space->install_rom      (0x000e0000, 0x000e3fff, &ram[0x000e0000/4]);
	if(pam[5] & 0x02)
		memory_space->install_writeonly(0x000e0000, 0x000e3fff, &ram[0x000e0000/4]);
	if(pam[5] & 0x10)
		memory_space->install_rom      (0x000e4000, 0x000e7fff, &ram[0x000e4000/4]);
	if(pam[5] & 0x20)
		memory_space->install_writeonly(0x000e4000, 0x000e7fff, &ram[0x000e4000/4]);
	if(pam[6] & 0x01)
		memory_space->install_rom      (0x000e8000, 0x000ebfff, &ram[0x000e8000/4]);
	if(pam[6] & 0x02)
		memory_space->install_writeonly(0x000e8000, 0x000ebfff, &ram[0x000e8000/4]);
	if(pam[6] & 0x10)
		memory_space->install_rom      (0x000ec000, 0x000effff, &ram[0x000ec000/4]);
	if(pam[6] & 0x20)
		memory_space->install_writeonly(0x000ec000, 0x000effff, &ram[0x000ec000/4]);
	if(pam[0] & 0x10)
		memory_space->install_rom      (0x000f0000, 0x000fffff, &ram[0x000f0000/4]);
	if(pam[0] & 0x20)
		memory_space->install_writeonly(0x000f0000, 0x000fffff, &ram[0x000f0000/4]);

	memory_space->install_ram          (0x00100000, 0x00efffff, &ram[0x00100000/4]);
	if(!(pam[7] & 0x80))
		memory_space->install_ram      (0x00f00000, 0x00ffffff, &ram[0x00f00000/4]);
	if(top > 0x01000000)
		memory_space->install_ram      (0x01000000, top-1,      &ram[0x01000000/4]);

	if((esmramc & 0x40) && (smram & 0x08))
		memory_space->install_ram      (0xfeda0000, 0xfedbffff, &ram[0x000a0000/4]);
}


i82875p_agp_device::i82875p_agp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: agp_bridge_device(mconfig, I82875P_AGP, tag, owner, clock)
{
	set_ids_bridge(0x80862579, 0x02);
}

void i82875p_agp_device::device_start()
{
	agp_bridge_device::device_start();
}

void i82875p_agp_device::device_reset()
{
	agp_bridge_device::device_reset();
}

void i82875p_overflow_device::overflow_map(address_map &map)
{
	map(0x000, 0x007).rw(FUNC(i82875p_overflow_device::dram_row_boundary_r), FUNC(i82875p_overflow_device::dram_row_boundary_w));
	map(0x010, 0x013).rw(FUNC(i82875p_overflow_device::dram_row_attribute_r), FUNC(i82875p_overflow_device::dram_row_attribute_w));
	map(0x060, 0x063).rw(FUNC(i82875p_overflow_device::dram_timing_r), FUNC(i82875p_overflow_device::dram_timing_w));
	map(0x068, 0x06b).rw(FUNC(i82875p_overflow_device::dram_controller_mode_r), FUNC(i82875p_overflow_device::dram_controller_mode_w));
}


i82875p_overflow_device::i82875p_overflow_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, I82875P_OVERFLOW, tag, owner, clock)
{
}

void i82875p_overflow_device::device_start()
{
	pci_device::device_start();

	add_map(4*1024, M_MEM, FUNC(i82875p_overflow_device::overflow_map));
}

void i82875p_overflow_device::device_reset()
{
	pci_device::device_reset();
	memset(dram_row_boundary, 1, sizeof(dram_row_boundary));
	memset(dram_row_attribute, 0, sizeof(dram_row_attribute));
	dram_timing = 0;
	dram_controller_mode = 0x00010001;
}

uint8_t i82875p_overflow_device::dram_row_boundary_r(offs_t offset)
{
	return dram_row_boundary[offset];
}

void i82875p_overflow_device::dram_row_boundary_w(offs_t offset, uint8_t data)
{
	dram_row_boundary[offset] = data;
	logerror("%s: dram_row_boundary_w %d, %02x\n", tag(), offset, data);
}

uint8_t i82875p_overflow_device::dram_row_attribute_r(offs_t offset)
{
	return dram_row_attribute[offset];
}

void i82875p_overflow_device::dram_row_attribute_w(offs_t offset, uint8_t data)
{
	dram_row_attribute[offset] = data;
	logerror("%s: dram_row_attribute_w %d, %02x\n", tag(), offset, data);
}

uint32_t i82875p_overflow_device::dram_timing_r()
{
	return dram_timing;
}

void i82875p_overflow_device::dram_timing_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&dram_timing);
	logerror("%s: dram_timing_w %08x\n", tag(), dram_timing);
}

uint32_t i82875p_overflow_device::dram_controller_mode_r()
{
	return dram_controller_mode;
}

void i82875p_overflow_device::dram_controller_mode_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&dram_controller_mode);
	logerror("%s: dram_controller_mode_w %08x\n", tag(), dram_controller_mode);
}
