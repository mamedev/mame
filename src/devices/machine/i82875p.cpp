// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "i82875p.h"

const device_type I82875P_HOST     = &device_creator<i82875p_host_device>;
const device_type I82875P_AGP      = &device_creator<i82875p_agp_device>;
const device_type I82875P_OVERFLOW = &device_creator<i82875p_overflow_device>;

DEVICE_ADDRESS_MAP_START(agp_translation_map, 32, i82875p_host_device)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START(config_map, 32, i82875p_host_device)
	AM_RANGE(0x50, 0x53) AM_READWRITE8 (agpm_r,     agpm_w,     0x0000ff00)
	AM_RANGE(0x50, 0x53) AM_READ8      (gc_r,                   0x00ff0000)
	AM_RANGE(0x50, 0x53) AM_READ8      (csabcont_r,             0xff000000)
	AM_RANGE(0x58, 0x5b) AM_READ       (eap_r)
	AM_RANGE(0x5c, 0x5f) AM_READ8      (derrsyn_r,              0x000000ff)
	AM_RANGE(0x5c, 0x5f) AM_READ8      (des_r,                  0x0000ff00)
	AM_RANGE(0x60, 0x63) AM_READWRITE8 (fpllcont_r, fpllcont_w, 0x000000ff)
	AM_RANGE(0x90, 0x97) AM_READWRITE8 (pam_r,      pam_w,      0xffffffff)
	AM_RANGE(0x9c, 0x9f) AM_READWRITE8 (smram_r,    smram_w,    0x0000ff00)
	AM_RANGE(0x9c, 0x9f) AM_READWRITE8 (esmramc_r,  esmramc_w,  0x00ff0000)
	AM_RANGE(0xa0, 0xa3) AM_READ       (acapid_r)
	AM_RANGE(0xa4, 0xa7) AM_READ       (agpstat_r)
	AM_RANGE(0xa8, 0xab) AM_READ       (agpcmd_r)
	AM_RANGE(0xb0, 0xb3) AM_READWRITE  (agpctrl_r,  agpctrl_w)
	AM_RANGE(0xb4, 0xb7) AM_READWRITE8 (apsize_r,   apsize_w,   0x000000ff)
	AM_RANGE(0xb8, 0xbb) AM_READWRITE  (attbase_r,  attbase_w)
	AM_RANGE(0xbc, 0xbf) AM_READWRITE8 (amtt_r,     amtt_w,     0x000000ff)
	AM_RANGE(0xbc, 0xbf) AM_READWRITE8 (lptt_r,     lptt_w,     0x0000ff00)
	AM_RANGE(0xc4, 0xc7) AM_READWRITE16(toud_r,     toud_w,     0x0000ffff)
	AM_RANGE(0xc4, 0xc7) AM_READWRITE16(mchcfg_r,   mchcfg_w,   0xffff0000)
	AM_RANGE(0xc8, 0xcb) AM_READ16     (errsts_r,               0x0000ffff)
	AM_RANGE(0xc8, 0xcb) AM_READWRITE16(errcmd_r,   errcmd_w,   0xffff0000)
	AM_RANGE(0xcc, 0xcf) AM_READWRITE16(smicmd_r,   smicmd_w,   0x0000ffff)
	AM_RANGE(0xcc, 0xcf) AM_READWRITE16(scicmd_r,   scicmd_w,   0xffff0000)
	AM_RANGE(0xdc, 0xdf) AM_READWRITE16(skpd_r,     skpd_w,     0xffff0000)
	AM_RANGE(0xe4, 0xe7) AM_READ       (capreg1_r)
	AM_RANGE(0xe8, 0xeb) AM_READ8      (capreg2_r,              0x000000ff)

	AM_INHERIT_FROM(pci_host_device::config_map)
ADDRESS_MAP_END

i82875p_host_device::i82875p_host_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: pci_host_device(mconfig, I82875P_HOST, "I82875P Northbridge", tag, owner, clock, "i82875p_host", __FILE__)
{
}

void i82875p_host_device::set_cpu_tag(const char *_cpu_tag)
{
	cpu_tag = _cpu_tag;
}

void i82875p_host_device::set_ram_size(int _ram_size)
{
	ram_size = _ram_size;
}

READ8_MEMBER(i82875p_host_device::capptr_r)
{
	return 0xe4;
}

void i82875p_host_device::device_start()
{
	pci_host_device::device_start();
	cpu = machine().device<cpu_device>(cpu_tag);
	memory_space = &cpu->space(AS_PROGRAM);
	io_space = &cpu->space(AS_IO);

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

READ8_MEMBER(  i82875p_host_device::agpm_r)
{
	return agpm;
}

WRITE8_MEMBER( i82875p_host_device::agpm_w)
{
	agpm = data;
	logerror("%s: agpm = %02x\n", tag(), agpm);
}

READ8_MEMBER(  i82875p_host_device::gc_r)
{
	return 0x08;
}

READ8_MEMBER(  i82875p_host_device::csabcont_r)
{
	return 0x00;
}

READ32_MEMBER( i82875p_host_device::eap_r)
{
	return 0x00000000;
}

READ8_MEMBER(  i82875p_host_device::derrsyn_r)
{
	return 0x00;
}

READ8_MEMBER(  i82875p_host_device::des_r)
{
	return 0x00;
}

READ8_MEMBER(  i82875p_host_device::fpllcont_r)
{
	return fpllcont;
}

WRITE8_MEMBER( i82875p_host_device::fpllcont_w)
{
	fpllcont = data;
	logerror("%s: fpllcont = %02x\n", tag(), data);
}

READ8_MEMBER(  i82875p_host_device::pam_r)
{
	return pam[offset];
}

WRITE8_MEMBER( i82875p_host_device::pam_w)
{
	pam[offset] = data;
	logerror("%s: pam[%d] = %02x\n", tag(), offset, data);
	remap_cb();
}

READ8_MEMBER(  i82875p_host_device::smram_r)
{
	return smram;
}

WRITE8_MEMBER( i82875p_host_device::smram_w)
{
	if(!(smram & 0x10))
		smram = (data & 0xfe) | 0x02;
	logerror("%s: smram = %02x\n", tag(), smram);
	remap_cb();
}

READ8_MEMBER(  i82875p_host_device::esmramc_r)
{
	return esmramc;
}

WRITE8_MEMBER( i82875p_host_device::esmramc_w)
{
	if(!(smram & 0x10))
		esmramc = (data & 0x87) | 0x38;
	logerror("%s: esmramc = %02x\n", tag(), smram);
	remap_cb();
}

READ32_MEMBER( i82875p_host_device::acapid_r)
{
	return 0x00300002;
}

READ32_MEMBER( i82875p_host_device::agpstat_r)
{
	return 0x1f004a13;
}

READ32_MEMBER( i82875p_host_device::agpcmd_r)
{
	return 0x00000a00;
}

READ32_MEMBER( i82875p_host_device::agpctrl_r)
{
	return agpctrl;
}

WRITE32_MEMBER(i82875p_host_device::agpctrl_w)
{
	COMBINE_DATA(&agpctrl);
	logerror("%s: agpctrl = %08x\n", tag(), agpctrl);
}

READ8_MEMBER(  i82875p_host_device::apsize_r)
{
	return apsize;
}

WRITE8_MEMBER( i82875p_host_device::apsize_w)
{
	apsize = data;
	logerror("%s: apsize = %02x\n", tag(), apsize);
}

READ32_MEMBER( i82875p_host_device::attbase_r)
{
	return attbase;
}

WRITE32_MEMBER(i82875p_host_device::attbase_w)
{
	COMBINE_DATA(&attbase);
	logerror("%s: attbase = %08x\n", tag(), attbase);
}

READ8_MEMBER(  i82875p_host_device::amtt_r)
{
	return amtt;
}

WRITE8_MEMBER( i82875p_host_device::amtt_w)
{
	amtt = data;
}

READ8_MEMBER(  i82875p_host_device::lptt_r)
{
	return lptt;
}

WRITE8_MEMBER( i82875p_host_device::lptt_w)
{
	lptt = data;
}

READ16_MEMBER( i82875p_host_device::toud_r)
{
	return toud;
}

WRITE16_MEMBER(i82875p_host_device::toud_w)
{
	COMBINE_DATA(&toud);
	toud &= ~7;
	logerror("%s: toud = %08x\n", tag(), toud << 16);
	remap_cb();
}

READ16_MEMBER( i82875p_host_device::mchcfg_r)
{
	return mchcfg;
}

WRITE16_MEMBER(i82875p_host_device::mchcfg_w)
{
	COMBINE_DATA(&mchcfg);
}

READ16_MEMBER( i82875p_host_device::errsts_r)
{
	return 0x0000;
}

READ16_MEMBER( i82875p_host_device::errcmd_r)
{
	return errcmd;
}

WRITE16_MEMBER(i82875p_host_device::errcmd_w)
{
	COMBINE_DATA(&errcmd);
}

READ16_MEMBER( i82875p_host_device::smicmd_r)
{
	return smicmd;
}

WRITE16_MEMBER(i82875p_host_device::smicmd_w)
{
	COMBINE_DATA(&smicmd);
}

READ16_MEMBER( i82875p_host_device::scicmd_r)
{
	return scicmd;
}

WRITE16_MEMBER(i82875p_host_device::scicmd_w)
{
	COMBINE_DATA(&scicmd);
}

READ16_MEMBER( i82875p_host_device::skpd_r)
{
	return skpd;
}

WRITE16_MEMBER(i82875p_host_device::skpd_w)
{
	COMBINE_DATA(&skpd);
}

READ32_MEMBER( i82875p_host_device::capreg1_r)
{
	return 0x0106a009;
}

READ8_MEMBER(  i82875p_host_device::capreg2_r)
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

void i82875p_host_device::map_extra(UINT64 memory_window_start, UINT64 memory_window_end, UINT64 memory_offset, address_space *memory_space,
									UINT64 io_window_start, UINT64 io_window_end, UINT64 io_offset, address_space *io_space)
{
	io_space->install_device(0, 0xffff, *static_cast<pci_host_device *>(this), &pci_host_device::io_configuration_access_map);

	UINT32 top = toud << 16;
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


i82875p_agp_device::i82875p_agp_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: agp_bridge_device(mconfig, I82875P_AGP, "I82875P AGP Bridge", tag, owner, clock, "i82875p_agp", __FILE__)
{
}

void i82875p_agp_device::device_start()
{
	agp_bridge_device::device_start();
}

void i82875p_agp_device::device_reset()
{
	agp_bridge_device::device_reset();
}

DEVICE_ADDRESS_MAP_START(overflow_map, 32, i82875p_overflow_device)
	AM_RANGE(0x000, 0x007) AM_READWRITE8(dram_row_boundary_r,    dram_row_boundary_w,  0xffffffff)
	AM_RANGE(0x010, 0x013) AM_READWRITE8(dram_row_attribute_r,   dram_row_attribute_w, 0xffffffff)
	AM_RANGE(0x060, 0x064) AM_READWRITE (dram_timing_r,          dram_timing_w)
	AM_RANGE(0x068, 0x06b) AM_READWRITE (dram_controller_mode_r, dram_controller_mode_w)
ADDRESS_MAP_END


i82875p_overflow_device::i82875p_overflow_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: pci_device(mconfig, I82875P_OVERFLOW, "I82875P Configuration Overflow", tag, owner, clock, "i82875p_overflow", __FILE__)
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

READ8_MEMBER  (i82875p_overflow_device::dram_row_boundary_r)
{
	return dram_row_boundary[offset];
}

WRITE8_MEMBER (i82875p_overflow_device::dram_row_boundary_w)
{
	dram_row_boundary[offset] = data;
	logerror("%s: dram_row_boundary_w %d, %02x\n", tag(), offset, data);
}

READ8_MEMBER  (i82875p_overflow_device::dram_row_attribute_r)
{
	return dram_row_attribute[offset];
}

WRITE8_MEMBER (i82875p_overflow_device::dram_row_attribute_w)
{
	dram_row_attribute[offset] = data;
	logerror("%s: dram_row_attribute_w %d, %02x\n", tag(), offset, data);
}

READ32_MEMBER (i82875p_overflow_device::dram_timing_r)
{
	return dram_timing;
}

WRITE32_MEMBER(i82875p_overflow_device::dram_timing_w)
{
	COMBINE_DATA(&dram_timing);
	logerror("%s: dram_timing_w %08x\n", tag(), dram_timing);
}

READ32_MEMBER (i82875p_overflow_device::dram_controller_mode_r)
{
	return dram_controller_mode;
}

WRITE32_MEMBER(i82875p_overflow_device::dram_controller_mode_w)
{
	COMBINE_DATA(&dram_controller_mode);
	logerror("%s: dram_controller_mode_w %08x\n", tag(), dram_controller_mode);
}
