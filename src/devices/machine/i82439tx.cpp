// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "i82439tx.h"

#include <algorithm>

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"

DEFINE_DEVICE_TYPE(I82439TX, i82439tx_host_device, "i82439tx", "Intel 82439TX northbridge")

void i82439tx_host_device::config_map(address_map &map)
{
	pci_host_device::config_map(map);
	map(0x10, 0x4f).noprw();
	map(0x4f, 0x4f).rw(FUNC(i82439tx_host_device::acon_r), FUNC(i82439tx_host_device::acon_w));
	map(0x50, 0x50).rw(FUNC(i82439tx_host_device::pcon_r), FUNC(i82439tx_host_device::pcon_w));
	map(0x52, 0x52).rw(FUNC(i82439tx_host_device::cc_r), FUNC(i82439tx_host_device::cc_w));
	map(0x53, 0x53).rw(FUNC(i82439tx_host_device::cec_r), FUNC(i82439tx_host_device::cec_w));
	map(0x54, 0x55).rw(FUNC(i82439tx_host_device::sdramc_r), FUNC(i82439tx_host_device::sdramc_w));
	map(0x56, 0x56).rw(FUNC(i82439tx_host_device::dramec_r), FUNC(i82439tx_host_device::dramec_w));
	map(0x57, 0x57).rw(FUNC(i82439tx_host_device::dramc_r), FUNC(i82439tx_host_device::dramc_w));
	map(0x58, 0x58).rw(FUNC(i82439tx_host_device::dramt_r), FUNC(i82439tx_host_device::dramt_w));
	map(0x59, 0x5f).rw(FUNC(i82439tx_host_device::pam_r), FUNC(i82439tx_host_device::pam_w));
	map(0x60, 0x65).rw(FUNC(i82439tx_host_device::drb_r), FUNC(i82439tx_host_device::drb_w));
	map(0x67, 0x67).rw(FUNC(i82439tx_host_device::drth_r), FUNC(i82439tx_host_device::drth_w));
	map(0x68, 0x68).rw(FUNC(i82439tx_host_device::drtl_r), FUNC(i82439tx_host_device::drtl_w));
	// map(0x70, 0x70) MTT test mode register & 0xfc
	map(0x71, 0x71).rw(FUNC(i82439tx_host_device::esmramc_r), FUNC(i82439tx_host_device::esmramc_w));
	map(0x72, 0x72).rw(FUNC(i82439tx_host_device::smram_r), FUNC(i82439tx_host_device::smram_w));
	map(0x79, 0x79).rw(FUNC(i82439tx_host_device::mctl_r), FUNC(i82439tx_host_device::mctl_w));
}

i82439tx_host_device::i82439tx_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_host_device(mconfig, I82439TX, tag, owner, clock)
	, cpu(*this, finder_base::DUMMY_TAG)
	, m_ram_dev(*this, finder_base::DUMMY_TAG)
	, m_ram(nullptr)
	, smiact_n(true)
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
	// Bus Master hardwired to 1, allow setting Memory Access to 0
	command = 0x0006;
	command_mask = 0x0002;
	// Medium DEVSEL#
	status = 0x0200;

	if (m_ram_dev)
	{
		ram_size = m_ram_dev->size();
		m_ram = reinterpret_cast<uint32_t *>(m_ram_dev->pointer());
	}
	else
	{
		ram.resize(ram_size/4);
		m_ram = ram.data();
	}

	save_item(NAME(acon));
	save_item(NAME(pcon));
	save_item(NAME(cc));
	save_item(NAME(cec));
	save_item(NAME(sdramc));
	save_item(NAME(dramec));
	save_item(NAME(dramc));
	save_item(NAME(dramt));
	save_item(NAME(pam));
	save_item(NAME(drb));
	save_item(NAME(drt));
	save_item(NAME(drth));
	save_item(NAME(smram));
	save_item(NAME(mctl));
	save_item(NAME(smiact_n));
}

void i82439tx_host_device::reset_all_mappings()
{
	pci_host_device::reset_all_mappings();
}

void i82439tx_host_device::device_reset()
{
	pci_host_device::device_reset();

	acon = 0x00;
	pcon = 0x00;
	// TODO: strapping option, disable cache for now
	cc = 0x02;
	cec = 0x14;
	sdramc = 0x0000;
	dramec = 0x00;
	dramc = 0x01;
	dramt = 0x00;
	memset(pam, 0x00, sizeof(pam));
	memset(drb, 0x02, sizeof(drb));
	drt = 0x00;
	// TODO: strapping option (1 = 66 MHz)
	drth = 0x80;
	esmramc = 0x00;
	smram = 0x02;
	mctl = 0x00;
	smiact_n = true;
}

void i82439tx_host_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
									 uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	io_space->install_device(0, 0xffff, *static_cast<pci_host_device *>(this), &pci_host_device::io_configuration_access_map);

	if((dramc & 0xc0) == 0x40)
		memory_space->install_ram      (0x00000000, 0x0007ffff, &m_ram[0x00000000/4]);
	else
		memory_space->install_ram      (0x00000000, 0x0009ffff, &m_ram[0x00000000/4]);

	// assume that map_extra of the northbridge is called after the video card has mapped its memory here
	// FIXME: D_CLS and D_LCK not emulated
	if (smram & 0x08)
	{
		if (smiact_n == 0)
			memory_space->install_ram(0x000a0000, 0x000bffff, &m_ram[0x000a0000 / 4]);
		else
			if (smram & 0x40)
				memory_space->install_ram(0x000a0000, 0x000bffff, &m_ram[0x000a0000 / 4]);
	}

	// TODO: cleanup this block
	if(pam[1] & 0x01)
		memory_space->install_rom      (0x000c0000, 0x000c3fff, &m_ram[0x000c0000/4]);
	if(pam[1] & 0x02)
		memory_space->install_writeonly(0x000c0000, 0x000c3fff, &m_ram[0x000c0000/4]);
	if(pam[1] & 0x10)
		memory_space->install_rom      (0x000c4000, 0x000c7fff, &m_ram[0x000c4000/4]);
	if(pam[1] & 0x20)
		memory_space->install_writeonly(0x000c4000, 0x000c7fff, &m_ram[0x000c4000/4]);
	if(pam[2] & 0x01)
		memory_space->install_rom      (0x000c8000, 0x000cbfff, &m_ram[0x000c8000/4]);
	if(pam[2] & 0x02)
		memory_space->install_writeonly(0x000c8000, 0x000cbfff, &m_ram[0x000c8000/4]);
	if(pam[2] & 0x10)
		memory_space->install_rom      (0x000cc000, 0x000cffff, &m_ram[0x000cc000/4]);
	if(pam[2] & 0x20)
		memory_space->install_writeonly(0x000cc000, 0x000cffff, &m_ram[0x000cc000/4]);
	if(pam[3] & 0x01)
		memory_space->install_rom      (0x000d0000, 0x000d3fff, &m_ram[0x000d0000/4]);
	if(pam[3] & 0x02)
		memory_space->install_writeonly(0x000d0000, 0x000d3fff, &m_ram[0x000d0000/4]);
	if(pam[3] & 0x10)
		memory_space->install_rom      (0x000d4000, 0x000d7fff, &m_ram[0x000d4000/4]);
	if(pam[3] & 0x20)
		memory_space->install_writeonly(0x000d4000, 0x000d7fff, &m_ram[0x000d4000/4]);
	if(pam[4] & 0x01)
		memory_space->install_rom      (0x000d8000, 0x000dbfff, &m_ram[0x000d8000/4]);
	if(pam[4] & 0x02)
		memory_space->install_writeonly(0x000d8000, 0x000dbfff, &m_ram[0x000d8000/4]);
	if(pam[4] & 0x10)
		memory_space->install_rom      (0x000dc000, 0x000dffff, &m_ram[0x000dc000/4]);
	if(pam[4] & 0x20)
		memory_space->install_writeonly(0x000dc000, 0x000dffff, &m_ram[0x000dc000/4]);
	if(pam[5] & 0x01)
		memory_space->install_rom      (0x000e0000, 0x000e3fff, &m_ram[0x000e0000/4]);
	if(pam[5] & 0x02)
		memory_space->install_writeonly(0x000e0000, 0x000e3fff, &m_ram[0x000e0000/4]);
	if(pam[5] & 0x10)
		memory_space->install_rom      (0x000e4000, 0x000e7fff, &m_ram[0x000e4000/4]);
	if(pam[5] & 0x20)
		memory_space->install_writeonly(0x000e4000, 0x000e7fff, &m_ram[0x000e4000/4]);
	if(pam[6] & 0x01)
		memory_space->install_rom      (0x000e8000, 0x000ebfff, &m_ram[0x000e8000/4]);
	if(pam[6] & 0x02)
		memory_space->install_writeonly(0x000e8000, 0x000ebfff, &m_ram[0x000e8000/4]);
	if(pam[6] & 0x10)
		memory_space->install_rom      (0x000ec000, 0x000effff, &m_ram[0x000ec000/4]);
	if(pam[6] & 0x20)
		memory_space->install_writeonly(0x000ec000, 0x000effff, &m_ram[0x000ec000/4]);
	if(pam[0] & 0x10)
		memory_space->install_rom      (0x000f0000, 0x000fffff, &m_ram[0x000f0000/4]);
	if(pam[0] & 0x20)
		memory_space->install_writeonly(0x000f0000, 0x000fffff, &m_ram[0x000f0000/4]);

	// extended memory, only as far as there is RAM
	const offs_t ram_top = ram_size;
	if(ram_top > 0x00100000)
		memory_space->install_ram      (0x00100000, std::min<offs_t>(ram_top, 0x00f00000) - 1, &m_ram[0x00100000/4]);
	if(ram_top > 0x00f00000 && (dramc & 0xc0) != 0x80)
		memory_space->install_ram      (0x00f00000, std::min<offs_t>(ram_top, 0x01000000) - 1, &m_ram[0x00f00000/4]);

	if(ram_top > 0x01000000)
		memory_space->install_ram      (0x01000000, ram_top - 1, &m_ram[0x01000000/4]);
}


uint8_t i82439tx_host_device::acon_r()
{
	return acon;
}

void i82439tx_host_device::acon_w(uint8_t data)
{
	acon = data & 0x80;
	LOG("acon = %02x -> %02x\n", data, acon);
}

uint8_t i82439tx_host_device::pcon_r()
{
	return pcon;
}

void i82439tx_host_device::pcon_w(uint8_t data)
{
	pcon = data & 0x08;
	LOG("pcon = %02x -> %02x\n", data, pcon);
}

uint8_t i82439tx_host_device::cc_r()
{
	return cc;
}

// xx-- ---- SCS Secondary cache (strapped)
// 00-- ---- Cache not populated
// 01-- ---- 256 Kbytes
// 10-- ---- 512 Kbytes
// 11-- ---- <reserved>
// --xx ---- L2SRAMT L2 SRAM Type (strapped)
// --00 ---- Pipelined Burst SRAM
// --11 ---- Two Banks of Pipelined Burst
// ---- x--- NAD NA Disable
// ---- --x- SCFMI Secondary Cache Force Miss or Invalidate
// ---- ---x FLCE First Level Cache Enable
void i82439tx_host_device::cc_w(uint8_t data)
{
	cc = data & 0xfb;
	LOG("cc = %02x -> %02x\n", data, cc);
}

uint8_t i82439tx_host_device::cec_r()
{
	// TODO: bit 5: DRAM CACHE L2 Present (read only)
	return cec;
}

void i82439tx_host_device::cec_w(uint8_t data)
{
	cec = data & 0x1f;
	LOG("cec = %02x -> %02x\n", data, cec);
}

uint16_t i82439tx_host_device::sdramc_r()
{
	return sdramc;
}

void i82439tx_host_device::sdramc_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&sdramc);
	sdramc &= 0x01fa;
	LOG("sdramc = %04x & %04x -> %04x\n", data, mem_mask, sdramc);
}

uint8_t i82439tx_host_device::dramec_r()
{
	return dramec;
}

void i82439tx_host_device::dramec_w(uint8_t data)
{
	dramec = data & 0x76;
	LOG("dramec = %02x -> %02x\n", data, dramec);
}

uint8_t i82439tx_host_device::dramc_r()
{
	return dramc;
}

void i82439tx_host_device::dramc_w(uint8_t data)
{
	dramc = data & 0xdf;
	LOG("dramc = %02x -> %02x\n", data, dramc);
	remap_cb();
}

uint8_t i82439tx_host_device::dramt_r()
{
	return dramt;
}

void i82439tx_host_device::dramt_w(uint8_t data)
{
	dramt = data & 0x7b;
	LOG("dramt = %02x -> %02x\n", data, dramt);
}

uint8_t i82439tx_host_device::pam_r(offs_t offset)
{
	return pam[offset];
}

void i82439tx_host_device::pam_w(offs_t offset, uint8_t data)
{
	pam[offset] = data;
	if (offset == 0)
		pam[offset] &= 0xf0;
	LOG("pam[%d] = %02x -> %02x\n", offset, data, pam[offset]);
	remap_cb();
}

uint8_t i82439tx_host_device::drb_r(offs_t offset)
{
	return drb[offset];
}

void i82439tx_host_device::drb_w(offs_t offset, uint8_t data)
{
	drb[offset] = data & 0x7f;
	LOG("drb[%d] = %02x -> %02x\n", offset, data, drb[offset]);
}

uint8_t i82439tx_host_device::drth_r()
{
	return drth;
}

void i82439tx_host_device::drth_w(uint8_t data)
{
	// not explicitly mentioned but assume HFD not really writeable
	drth = (data & 0x37) | (drth & 0x80);
	LOG("drth = %02x -> %02x\n", data, drth);
}

uint8_t i82439tx_host_device::drtl_r()
{
	return drt;
}

void i82439tx_host_device::drtl_w(uint8_t data)
{
	drt = data;
	LOG("drtl = %02x\n", drt);
}

uint8_t i82439tx_host_device::esmramc_r()
{
	return esmramc;
}

void i82439tx_host_device::esmramc_w(uint8_t data)
{
	esmramc = data;
	LOG("esmramc = %02x\n", esmramc);
	// TODO: maps high SMRAM to appear at $100a'0000 ~ $100f'0000
	if (BIT(esmramc, 7))
	{
		LOG("Warning H_SMRAME enabled\n");
		// remap_cb();
	}
}

uint8_t i82439tx_host_device::smram_r()
{
	return smram;
}

void i82439tx_host_device::smram_w(uint8_t data)
{
	smram = data & 0x7f;
	LOG("smram = %02x -> %02x\n", data, smram);
	remap_cb();
}

uint8_t i82439tx_host_device::mctl_r()
{
	return mctl;
}

void i82439tx_host_device::mctl_w(uint8_t data)
{
	mctl = data & 0x74;
	// TODO: bit 6 is ACPI Control Enable
	LOG("mctl = %02x -> %02x (ACRE=%d)\n", data, mctl, BIT(mctl, 6));
	// remap_cb();
}

void i82439tx_host_device::smi_act_w(int state)
{
	// state is 0 when smm is not active
	// but smiact_n reflects the state of the SMIACT# pin
	if (state == 0)
		smiact_n = 1;
	else
		smiact_n = 0;
	remap_cb();
}
