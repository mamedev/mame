// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "i82439tx.h"

DEFINE_DEVICE_TYPE(I82439TX, i82439tx_host_device, "i82439tx", "Intel 82439TX northbridge")

void i82439tx_host_device::config_map(address_map &map)
{
	pci_host_device::config_map(map);
	map(0x50, 0x50).rw(FUNC(i82439tx_host_device::pcon_r), FUNC(i82439tx_host_device::pcon_w));
	map(0x52, 0x52).rw(FUNC(i82439tx_host_device::cc_r), FUNC(i82439tx_host_device::cc_w));
	map(0x56, 0x56).rw(FUNC(i82439tx_host_device::dramec_r), FUNC(i82439tx_host_device::dramec_w));
	map(0x57, 0x57).rw(FUNC(i82439tx_host_device::dramc_r), FUNC(i82439tx_host_device::dramc_w));
	map(0x58, 0x5f).rw(FUNC(i82439tx_host_device::pam_r), FUNC(i82439tx_host_device::pam_w));
	map(0x58, 0x58).rw(FUNC(i82439tx_host_device::dramt_r), FUNC(i82439tx_host_device::dramt_w));
	map(0x60, 0x67).rw(FUNC(i82439tx_host_device::drb_r), FUNC(i82439tx_host_device::drb_w));
	map(0x68, 0x68).rw(FUNC(i82439tx_host_device::drt_r), FUNC(i82439tx_host_device::drt_w));
	map(0x69, 0x69).rw(FUNC(i82439tx_host_device::drat_r), FUNC(i82439tx_host_device::drat_w));
	map(0x72, 0x72).rw(FUNC(i82439tx_host_device::smram_r), FUNC(i82439tx_host_device::smram_w));
	map(0x90, 0x90).rw(FUNC(i82439tx_host_device::errcmd_r), FUNC(i82439tx_host_device::errcmd_w));
	map(0x91, 0x91).rw(FUNC(i82439tx_host_device::errsts_r), FUNC(i82439tx_host_device::errsts_w));
	map(0x92, 0x92).r(FUNC(i82439tx_host_device::errsyn_r));
}

i82439tx_host_device::i82439tx_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_host_device(mconfig, I82439TX, tag, owner, clock)
	, cpu(*this, finder_base::DUMMY_TAG)
{
}

void i82439tx_host_device::set_ram_size(int _ram_size)
{
	ram_size = _ram_size;
}

void i82439tx_host_device::device_start()
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
}

void i82439tx_host_device::reset_all_mappings()
{
	pci_host_device::reset_all_mappings();
}

void i82439tx_host_device::device_reset()
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

void i82439tx_host_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
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


uint8_t i82439tx_host_device::pcon_r()
{
	return pcon;
}

void i82439tx_host_device::pcon_w(uint8_t data)
{
	pcon = data;
	logerror("pcon = %02x\n", pcon);
}

uint8_t i82439tx_host_device::cc_r()
{
	return cc;
}

void i82439tx_host_device::cc_w(uint8_t data)
{
	cc = data;
	logerror("cc = %02x\n", cc);
}

uint8_t i82439tx_host_device::dramec_r()
{
	return dramec;
}

void i82439tx_host_device::dramec_w(uint8_t data)
{
	dramec = data;
	logerror("dramec = %02x\n", dramec);
}

uint8_t i82439tx_host_device::dramc_r()
{
	return dramc;
}

void i82439tx_host_device::dramc_w(uint8_t data)
{
	dramc = data;
	logerror("dramc = %02x\n", dramc);
	remap_cb();
}

uint8_t i82439tx_host_device::dramt_r()
{
	return dramt;
}

void i82439tx_host_device::dramt_w(uint8_t data)
{
	dramt = data;
	logerror("dramt = %02x\n", dramt);
}

uint8_t i82439tx_host_device::pam_r(offs_t offset)
{
	return pam[offset - 1];
}

void i82439tx_host_device::pam_w(offs_t offset, uint8_t data)
{
	pam[offset - 1] = data;
	logerror("pam[%d] = %02x\n", offset - 1, pam[offset - 1]);
	remap_cb();
}

uint8_t i82439tx_host_device::drb_r(offs_t offset)
{
	return drb[offset];
}

void i82439tx_host_device::drb_w(offs_t offset, uint8_t data)
{
	drb[offset] = data;
	logerror("drb[%d] = %02x\n", offset, drb[offset]);
}

uint8_t i82439tx_host_device::drt_r()
{
	return drt;
}

void i82439tx_host_device::drt_w(uint8_t data)
{
	drt = data;
	logerror("drt = %02x\n", drt);
}

uint8_t i82439tx_host_device::drat_r()
{
	return drat;
}

void i82439tx_host_device::drat_w(uint8_t data)
{
	drat = data;
	logerror("drat = %02x\n", drat);
}

uint8_t i82439tx_host_device::smram_r()
{
	return smram;
}

void i82439tx_host_device::smram_w(uint8_t data)
{
	smram = data;
	logerror("smram = %02x\n", smram);
	remap_cb();
}

uint8_t i82439tx_host_device::errcmd_r()
{
	return errcmd;
}

void i82439tx_host_device::errcmd_w(uint8_t data)
{
	errcmd = data;
	logerror("errcmd = %02x\n", errcmd);
}

uint8_t i82439tx_host_device::errsts_r()
{
	return errsts;
}

void i82439tx_host_device::errsts_w(uint8_t data)
{
	errsts = data;
	logerror("errsts = %02x\n", errsts);
}

uint8_t i82439tx_host_device::errsyn_r()
{
	return errsyn;
}
