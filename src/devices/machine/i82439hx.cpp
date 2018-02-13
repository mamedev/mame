// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "i82439hx.h"

DEFINE_DEVICE_TYPE(I82439HX, i82439hx_host_device, "i82439hx", "Intel 82439HX northbridge")

ADDRESS_MAP_START(i82439hx_host_device::config_map)
	AM_IMPORT_FROM(pci_host_device::config_map)
	AM_RANGE(0x50, 0x53) AM_READWRITE8(pcon_r,   pcon_w,   0x000000ff)
	AM_RANGE(0x50, 0x53) AM_READWRITE8(cc_r,     cc_w,     0x00ff0000)
	AM_RANGE(0x54, 0x57) AM_READWRITE8(dramec_r, dramec_w, 0x00ff0000)
	AM_RANGE(0x54, 0x57) AM_READWRITE8(dramc_r,  dramc_w,  0xff000000)
	AM_RANGE(0x58, 0x5f) AM_READWRITE8(pam_r,    pam_w,    0xffffffff)
	AM_RANGE(0x58, 0x5b) AM_READWRITE8(dramt_r,  dramt_w,  0x000000ff)
	AM_RANGE(0x60, 0x67) AM_READWRITE8(drb_r,    drb_w,    0xffffffff)
	AM_RANGE(0x68, 0x6b) AM_READWRITE8(drt_r,    drt_w,    0x000000ff)
	AM_RANGE(0x68, 0x6b) AM_READWRITE8(drat_r,   drat_w,   0x0000ff00)
	AM_RANGE(0x70, 0x73) AM_READWRITE8(smram_r,  smram_w,  0x00ff0000)
	AM_RANGE(0x90, 0x93) AM_READWRITE8(errcmd_r, errcmd_w, 0x000000ff)
	AM_RANGE(0x90, 0x93) AM_READWRITE8(errsts_r, errsts_w, 0x0000ff00)
	AM_RANGE(0x90, 0x93) AM_READ8     (errsyn_r,           0x00ff0000)
ADDRESS_MAP_END

i82439hx_host_device::i82439hx_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_host_device(mconfig, I82439HX, tag, owner, clock)
{
}

void i82439hx_host_device::set_cpu_tag(const char *_cpu_tag)
{
	cpu_tag = _cpu_tag;
}

void i82439hx_host_device::set_ram_size(int _ram_size)
{
	ram_size = _ram_size;
}

void i82439hx_host_device::device_start()
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
}

void i82439hx_host_device::reset_all_mappings()
{
	pci_host_device::reset_all_mappings();
}

void i82439hx_host_device::device_reset()
{
	pci_host_device::device_reset();

	pcon = 0x00;
	cc = 0xa2;
	dramec = 0x00;
	dramc = 0x01;
	dramt = 0x00;
	memset(pam, 0x00, sizeof(pam));
	memset(drb, 0x02, sizeof(drb));
	drt = 0x00;
	drat = 0x00;
	smram = 0x02;
	errcmd = 0x00;
	errsts = 0x00;
	errsyn = 0x00;
}

void i82439hx_host_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
									 uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	io_space->install_device(0, 0xffff, *static_cast<pci_host_device *>(this), &pci_host_device::io_configuration_access_map);

	if((dramc & 0xc0) == 0x40)
		memory_space->install_ram      (0x00000000, 0x0007ffff, &ram[0x00000000/4]);
	else
		memory_space->install_ram      (0x00000000, 0x0009ffff, &ram[0x00000000/4]);

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
	if((dramc & 0xc0) != 0x80)
		memory_space->install_ram      (0x00f00000, 0x00ffffff, &ram[0x00f00000/4]);

	memory_space->install_ram          (0x01000000, ram_size-1, &ram[0x01000000/4]);
}


READ8_MEMBER (i82439hx_host_device::pcon_r)
{
	return pcon;
}

WRITE8_MEMBER(i82439hx_host_device::pcon_w)
{
	pcon = data;
	logerror("pcon = %02x\n", pcon);
}

READ8_MEMBER (i82439hx_host_device::cc_r)
{
	return cc;
}

WRITE8_MEMBER(i82439hx_host_device::cc_w)
{
	cc = data;
	logerror("cc = %02x\n", cc);
}

READ8_MEMBER (i82439hx_host_device::dramec_r)
{
	return dramec;
}

WRITE8_MEMBER(i82439hx_host_device::dramec_w)
{
	dramec = data;
	logerror("dramec = %02x\n", dramec);
}

READ8_MEMBER (i82439hx_host_device::dramc_r)
{
	return dramc;
}

WRITE8_MEMBER(i82439hx_host_device::dramc_w)
{
	dramc = data;
	logerror("dramc = %02x\n", dramc);
	remap_cb();
}

READ8_MEMBER (i82439hx_host_device::dramt_r)
{
	return dramt;
}

WRITE8_MEMBER(i82439hx_host_device::dramt_w)
{
	dramt = data;
	logerror("dramt = %02x\n", dramt);
}

READ8_MEMBER (i82439hx_host_device::pam_r)
{
	return pam[offset - 1];
}

WRITE8_MEMBER(i82439hx_host_device::pam_w)
{
	pam[offset - 1] = data;
	logerror("pam[%d] = %02x\n", offset - 1, pam[offset - 1]);
	remap_cb();
}

READ8_MEMBER (i82439hx_host_device::drb_r)
{
	return drb[offset];
}

WRITE8_MEMBER(i82439hx_host_device::drb_w)
{
	drb[offset] = data;
	logerror("drb[%d] = %02x\n", offset, drb[offset]);
}

READ8_MEMBER (i82439hx_host_device::drt_r)
{
	return drt;
}

WRITE8_MEMBER(i82439hx_host_device::drt_w)
{
	drt = data;
	logerror("drt = %02x\n", drt);
}

READ8_MEMBER (i82439hx_host_device::drat_r)
{
	return drat;
}

WRITE8_MEMBER(i82439hx_host_device::drat_w)
{
	drat = data;
	logerror("drat = %02x\n", drat);
}

READ8_MEMBER (i82439hx_host_device::smram_r)
{
	return smram;
}

WRITE8_MEMBER(i82439hx_host_device::smram_w)
{
	smram = data;
	logerror("smram = %02x\n", smram);
	remap_cb();
}

READ8_MEMBER (i82439hx_host_device::errcmd_r)
{
	return errcmd;
}

WRITE8_MEMBER(i82439hx_host_device::errcmd_w)
{
	errcmd = data;
	logerror("errcmd = %02x\n", errcmd);
}

READ8_MEMBER (i82439hx_host_device::errsts_r)
{
	return errsts;
}

WRITE8_MEMBER(i82439hx_host_device::errsts_w)
{
	errsts = data;
	logerror("errsts = %02x\n", errsts);
}

READ8_MEMBER (i82439hx_host_device::errsyn_r)
{
	return errsyn;
}
