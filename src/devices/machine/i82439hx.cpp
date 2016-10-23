// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "i82439hx.h"

const device_type I82439HX = &device_creator<i82439hx_host_device>;

DEVICE_ADDRESS_MAP_START(config_map, 32, i82439hx_host_device)
	AM_RANGE(0x50, 0x53) AM_READWRITE8(pcon_r,   pcon_w,   0x000000ff)
	AM_RANGE(0x50, 0x53) AM_READWRITE8(cc_r,     cc_w,     0x00ff0000)
	AM_RANGE(0x54, 0x57) AM_READWRITE8(dramec_r, dramec_w, 0x00ff0000)
	AM_RANGE(0x54, 0x57) AM_READWRITE8(dramc_r,  dramc_w,  0xff000000)
	AM_RANGE(0x58, 0x5b) AM_READWRITE8(dramt_r,  dramt_w,  0x000000ff)
	AM_RANGE(0x58, 0x5b) AM_READWRITE8(pam0_r,   pam0_w,   0xffffff00)
	AM_RANGE(0x5c, 0x5f) AM_READWRITE8(pam3_r,   pam3_w,   0xffffffff)
	AM_RANGE(0x60, 0x67) AM_READWRITE8(drb_r,    drb_w,    0xffffffff)
	AM_RANGE(0x68, 0x6b) AM_READWRITE8(drt_r,    drt_w,    0x000000ff)
	AM_RANGE(0x68, 0x6b) AM_READWRITE8(drat_r,   drat_w,   0x0000ff00)
	AM_RANGE(0x70, 0x73) AM_READWRITE8(smram_r,  smram_w,  0x00ff0000)
	AM_RANGE(0x90, 0x93) AM_READWRITE8(errcmd_r, errcmd_w, 0x000000ff)
	AM_RANGE(0x90, 0x93) AM_READWRITE8(errsts_r, errsts_w, 0x0000ff00)
	AM_RANGE(0x90, 0x93) AM_READ8     (errsyn_r,           0x00ff0000)


	AM_INHERIT_FROM(pci_host_device::config_map)
ADDRESS_MAP_END

i82439hx_host_device::i82439hx_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_host_device(mconfig, I82439HX, "i82439hx northbridge", tag, owner, clock, "i82439hx", __FILE__)
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


uint8_t i82439hx_host_device::pcon_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return pcon;
}

void i82439hx_host_device::pcon_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	pcon = data;
	logerror("pcon = %02x\n", pcon);
}

uint8_t i82439hx_host_device::cc_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return cc;
}

void i82439hx_host_device::cc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	cc = data;
	logerror("cc = %02x\n", cc);
}

uint8_t i82439hx_host_device::dramec_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return dramec;
}

void i82439hx_host_device::dramec_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	dramec = data;
	logerror("dramec = %02x\n", dramec);
}

uint8_t i82439hx_host_device::dramc_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return dramc;
}

void i82439hx_host_device::dramc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	dramc = data;
	logerror("dramc = %02x\n", dramc);
	remap_cb();
}

uint8_t i82439hx_host_device::dramt_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return dramt;
}

void i82439hx_host_device::dramt_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	dramt = data;
	logerror("dramt = %02x\n", dramt);
}

uint8_t i82439hx_host_device::pam0_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return pam[offset];
}

void i82439hx_host_device::pam0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	pam[offset] = data;
	logerror("pam[%d] = %02x\n", offset, pam[offset]);
	remap_cb();
}

uint8_t i82439hx_host_device::pam3_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return pam[3+offset];
}

void i82439hx_host_device::pam3_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	pam[3+offset] = data;
	logerror("pam[%d] = %02x\n", 3+offset, pam[3+offset]);
	remap_cb();
}

uint8_t i82439hx_host_device::drb_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return drb[offset];
}

void i82439hx_host_device::drb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	drb[offset] = data;
	logerror("drb[%d] = %02x\n", offset, drb[offset]);
}

uint8_t i82439hx_host_device::drt_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return drt;
}

void i82439hx_host_device::drt_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	drt = data;
	logerror("drt = %02x\n", drt);
}

uint8_t i82439hx_host_device::drat_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return drat;
}

void i82439hx_host_device::drat_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	drat = data;
	logerror("drat = %02x\n", drat);
}

uint8_t i82439hx_host_device::smram_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return smram;
}

void i82439hx_host_device::smram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	smram = data;
	logerror("smram = %02x\n", smram);
	remap_cb();
}

uint8_t i82439hx_host_device::errcmd_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return errcmd;
}

void i82439hx_host_device::errcmd_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	errcmd = data;
	logerror("errcmd = %02x\n", errcmd);
}

uint8_t i82439hx_host_device::errsts_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return errsts;
}

void i82439hx_host_device::errsts_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	errsts = data;
	logerror("errsts = %02x\n", errsts);
}

uint8_t i82439hx_host_device::errsyn_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return errsyn;
}
