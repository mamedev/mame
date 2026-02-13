// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay, O. Galibert

#include "emu.h"
#include "luna_68k_ioc.h"
#include "bus/nscsi/hd.h"

luna_68k_ioc_device::luna_68k_ioc_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, LUNA_68K_IOC, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_cpu(*this, "cpu")
	, m_dma(*this, "dma%u", 0U)
	, m_spc(*this, "scsi%u:7:spc", 0U)
	, m_fdc(*this, "fdc")
	, m_floppy(*this, "fdc:0")
	, m_scc(*this, "scc")
	, m_cio(*this, "cio")
	, m_ram(*this, "ram")
	, m_boot(*this, "boot")
	, m_main_space(*this, finder_base::DUMMY_TAG, 0)
	, m_interrupt_cb(*this)
	, m_interrupt_scsii_cb(*this)
	, m_interrupt_scsie_cb(*this)
	, m_interrupt_dma0_cb(*this)
	, m_interrupt_dma1_cb(*this)
	, m_scsi0_dma_read(*this, 0)
	, m_scsi0_dma_write(*this)
	, m_dma_config("dma", ENDIANNESS_BIG, 8, 24, 0)
{
}

void luna_68k_ioc_device::device_start()
{
	save_item(NAME(m_interrupt_hack));
	save_item(NAME(m_m2i));
	save_item(NAME(m_i2m));
	save_item(NAME(m_direction));
	save_item(NAME(m_scsi_drq0));
	save_item(NAME(m_packed_data));
	save_item(NAME(m_packed_index));

	space(0).install_readwrite_handler(0, 0xffffff,
									   emu::rw_delegate(*this, FUNC(luna_68k_ioc_device::dma_r)),
									   emu::rw_delegate(*this, FUNC(luna_68k_ioc_device::dma_w)));
}

void luna_68k_ioc_device::device_reset()
{
	// there must be a select/motor/side select on somewhere
	auto *floppy = m_floppy->get_device();
	m_fdc->set_floppy(floppy);
	if(floppy) {
		floppy->ss_w(0);
		floppy->mon_w(0);
	}

	m_m2i = 0;
	m_i2m = 0;
	m_interrupt_hack = 2;
	m_direction = 0;
	m_packed_data = 0;
	m_packed_index = 0;

	// Overlay the rom at 0 on reset
	m_boot.select(0);
}

device_memory_interface::space_config_vector luna_68k_ioc_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_dma_config)
	};
}

// ioc/cpu shared ram, cpu is 32-bits wide, ioc is 16, hence the need for trampolines
u16 luna_68k_ioc_device::shared_ram_r(offs_t offset)
{
	return m_ram[offset];
}

void luna_68k_ioc_device::shared_ram_w(offs_t offset, u16 data, u16 mem_mask)
{
	m_ram[offset] = (m_ram[offset] & ~mem_mask) | (data & mem_mask);
}

void luna_68k_ioc_device::reset_cb(int)
{
	m_boot.disable();
}
void luna_68k_ioc_device::m2i_w(u32 data)
{
	logerror("m2i_w %08x\n", data);
	m_m2i = data;
	m_cpu->set_input_line(1, ASSERT_LINE);
}

u8 luna_68k_ioc_device::m2i_r()
{
	return m_m2i;
}

void luna_68k_ioc_device::m2i_int_clear(u8 data)
{
	logerror("m2i interrupt clear %d\n", data);
	m_cpu->set_input_line(1, CLEAR_LINE);
}

void luna_68k_ioc_device::i2m_w(u8 data)
{
	logerror("%s i2m_w %02x\n", machine().time().to_string(), data);
	m_i2m = data;

	// First two clears happen before the interrupt is raised, wtf?
	if (m_interrupt_hack > 0) {
		m_interrupt_hack --;
		return;
	}

	m_interrupt_cb(1);
}

u32 luna_68k_ioc_device::i2m_r()
{
	return m_i2m;
}

u8 luna_68k_ioc_device::cio_vector_r()
{
	return 0x60 | (m_cio->intack_r() & 0xf);
}

void luna_68k_ioc_device::i2m_int_clear(u32 data)
{
	logerror("%s i2m interrupt clear %d\n", machine().time().to_string(), data);
	m_interrupt_cb(0);
}

// Special dma expander
//
// It uses an hd63450 as the dma engine, but only uses its address
// generation capability.  It does 32-bits accesses, packing four
// bytes in one bus transaction.  It uses the 24 address bits as bits
// 2-25, giving 64M addressing capability instead of the normal 16M.

// It seems to be only connected to the internal scsi

void luna_68k_ioc_device::direction_w(u16 data)
{
	logerror("direction %04x\n", data);
	m_direction = data;
}

u16 luna_68k_ioc_device::direction_r()
{
	return m_direction;
}

void luna_68k_ioc_device::scsi_drq0_w(int state)
{
	if(state == m_scsi_drq0)
		return;
	m_scsi_drq0 = state;
	if(m_scsi_drq0) {
		if(BIT(m_direction, 3)) {
			while(m_scsi_drq0 && m_packed_index != 4) {
				m_packed_data |= m_scsi0_dma_read() << (24 - 8*m_packed_index);
				m_packed_index ++;
			}
			if(m_packed_index == 4)
				m_dma[0]->drq2_w(1);

		} else {
			if(m_packed_index == 0)
				m_dma[0]->drq2_w(1);
			
			else {
				while(m_scsi_drq0 && m_packed_index != 5) {
					m_scsi0_dma_write(m_packed_data >> (24 - 8*(m_packed_index-1)));
					m_packed_index ++;
				}
				if(m_packed_index == 5) {
					m_packed_index = 0;
					m_packed_data = 0;
					if(m_scsi_drq0)
						m_dma[0]->drq2_w(1);
				}
			}
		}
	} else {
		m_dma[0]->drq2_w(0);
	}
}

void luna_68k_ioc_device::dma_w(offs_t offset, u8)
{
	if(!BIT(m_direction, 3))
		return;
	m_dma[0]->drq2_w(0);
	m_main_space->write_dword(offset*4, m_packed_data);
	m_packed_data = 0;
	m_packed_index = 0;
	if(m_scsi_drq0) {
		scsi_drq0_w(0);
		scsi_drq0_w(1);
	}
}

u8 luna_68k_ioc_device::dma_r(offs_t offset)
{
	if(BIT(m_direction, 3))
		return 0;
	m_packed_data = m_main_space->read_dword(offset*4);
	m_packed_index = 1;
	m_dma[0]->drq2_w(0);
	if(m_scsi_drq0) {
		scsi_drq0_w(0);
		scsi_drq0_w(1);
	}
	return 0;
}

void luna_68k_ioc_device::vme_map(address_map &map)
{
	// 2* is 24-bits "short" VME space

	map(0x20280000, 0x2029ffff).rw(FUNC(luna_68k_ioc_device::shared_ram_r), FUNC(luna_68k_ioc_device::shared_ram_w));

	// 3* is 32-bits "long" VME space

	map(0x30000834, 0x30000837).rw(FUNC(luna_68k_ioc_device::i2m_r), FUNC(luna_68k_ioc_device::m2i_w));
	map(0x30000838, 0x3000083b).w(FUNC(luna_68k_ioc_device::i2m_int_clear));

	map(0x30000900, 0x300009ff).rw(m_dma[0], FUNC(hd63450_device::read), FUNC(hd63450_device::write));
	map(0x30000b80, 0x30000b81).rw(FUNC(luna_68k_ioc_device::direction_r), FUNC(luna_68k_ioc_device::direction_w));

	map(0x30000d00, 0x30000d1f).m(m_spc[0], FUNC(mb89352_device::map)).umask32(0x00ff00ff);
	map(0x30000d20, 0x30000d3f).m(m_spc[1], FUNC(mb89352_device::map)).umask32(0x00ff00ff);
	map(0x30001200, 0x300012ff).rw(m_dma[1], FUNC(hd63450_device::read), FUNC(hd63450_device::write));
}

void luna_68k_ioc_device::cpu_map(address_map &map)
{
	// am8530h-6pc scc @ 4.9152MHz
	// mb89352 x 2 scsi
	// mb8877a
	// hd63450ps10 x 2 dma
	// z0853606psc cio

	map(0x000000, 0x01ffff).ram(); // HM62256LP-10x4 (32768x4) - 128KB
	map(0x000000, 0x000fff).view(m_boot);
	m_boot[0](0x000000, 0x000fff).rom().region("ioc", 0);
	map(0x100000, 0x11ffff).ram().share(m_ram); // HM62256LP-10x8 (32768x4) - 128KB

	map(0xf86001, 0xf86001).w(FUNC(luna_68k_ioc_device::m2i_int_clear));
	map(0xf87000, 0xf87000).w(FUNC(luna_68k_ioc_device::i2m_w));
	map(0xf87001, 0xf87001).r(FUNC(luna_68k_ioc_device::m2i_r));
	map(0xfc0000, 0xfcffff).rom().region("ioc", 0);
	map(0xfe0000, 0xfe0fff).rom().region("ioc", 0);
	map(0xfef300, 0xfef307).rw(m_fdc, FUNC(mb8877_device::read), FUNC(mb8877_device::write)).umask16(0x00ff);

	map(0xfef400, 0xfef400).rw(m_scc, FUNC(z80scc_device::da_r), FUNC(z80scc_device::da_w));
	map(0xfef401, 0xfef401).rw(m_scc, FUNC(z80scc_device::ca_r), FUNC(z80scc_device::ca_w));
	map(0xfef402, 0xfef402).rw(m_scc, FUNC(z80scc_device::db_r), FUNC(z80scc_device::db_w));
	map(0xfef403, 0xfef403).rw(m_scc, FUNC(z80scc_device::cb_r), FUNC(z80scc_device::cb_w));

	map(0xfef600, 0xfef607).rw(m_cio, FUNC(z8536_device::read), FUNC(z8536_device::write)).umask16(0x00ff);
}

void luna_68k_ioc_device::cpuspace_map(address_map &map)
{
	map(0xfffff0, 0xffffff).m(m_cpu, FUNC(m68000_device::autovectors_map));
	map(0xfffff5, 0xfffff5).r(FUNC(luna_68k_ioc_device::cio_vector_r)); // Actual level unknown, just not 2 or 7
}


void luna_68k_ioc_device::scsi_devices(device_slot_interface &device)
{
	device.option_add("harddisk", NSCSI_HARDDISK)
		.machine_config([](device_t *hd) {
							static_cast<nscsi_harddisk_device *>(hd)->set_default_model_name("CDC     94161-156");
						});
}

void luna_68k_ioc_device::floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}

void luna_68k_ioc_device::device_add_mconfig(machine_config &config)
{
	M68000(config, m_cpu, 10_MHz_XTAL);
	m_cpu->set_addrmap(AS_PROGRAM, &luna_68k_ioc_device::cpu_map);
	m_cpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &luna_68k_ioc_device::cpuspace_map);
	m_cpu->reset_cb().set(*this, FUNC(luna_68k_ioc_device::reset_cb));

	HD63450(config, m_dma[0], 10_MHz_XTAL, DEVICE_SELF, 0);
	m_dma[0]->set_clocks(attotime::from_nsec(80), attotime::from_nsec(80), attotime::from_nsec(80), attotime::from_nsec(80));
	m_dma[0]->set_burst_clocks(attotime::from_nsec(80), attotime::from_nsec(80), attotime::from_nsec(80), attotime::from_nsec(80));
	m_dma[0]->irq_callback().set([this](int state) { m_interrupt_dma0_cb(state); });

	HD63450(config, m_dma[1], 10_MHz_XTAL, DEVICE_SELF, 0);
	m_dma[1]->set_clocks(attotime::from_nsec(80), attotime::from_nsec(80), attotime::from_nsec(80), attotime::from_nsec(80));
	m_dma[1]->set_burst_clocks(attotime::from_nsec(80), attotime::from_nsec(80), attotime::from_nsec(80), attotime::from_nsec(80));
	m_dma[1]->irq_callback().set([this](int state) { m_interrupt_dma1_cb(state); });

	// internal SCSI
	NSCSI_BUS(config, "scsi0");
	NSCSI_CONNECTOR(config, "scsi0:0", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi0:1", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi0:2", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi0:3", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi0:4", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi0:5", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi0:6", scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi0:7").option_set("spc", MB89352)
		.machine_config(
						[this, dma=m_dma[0].target()](device_t *device)
						{
							mb89352_device &spc = downcast<mb89352_device &>(*device);
							
							spc.set_clock(10'000'000);
							spc.out_irq_callback().set([this](int state) { m_interrupt_scsii_cb(state); });
							spc.out_dreq_callback().set(*this, FUNC(luna_68k_ioc_device::scsi_drq0_w));
							m_scsi0_dma_read.bind().set(spc, FUNC(mb89352_device::dma_r));
							m_scsi0_dma_write.bind().set(spc, FUNC(mb89352_device::dma_w));
						});

	// external SCSI
	NSCSI_BUS(config, "scsi1");
	NSCSI_CONNECTOR(config, "scsi1:0", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:1", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:2", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:3", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:4", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:5", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:6", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:7").option_set("spc", MB89352)
		.machine_config(
						[this](device_t *device)
						{
							mb89352_device &spc = downcast<mb89352_device &>(*device);
							
							spc.set_clock(10'000'000);
							spc.out_irq_callback().set([this](int state) { m_interrupt_scsie_cb(state); });
						});

	MB8877(config, m_fdc, 32_MHz_XTAL / 4);
	FLOPPY_CONNECTOR(config, m_floppy, floppies, "525qd", floppy_image_device::default_mfm_floppy_formats);

	SCC8530(config, m_scc, 4.9152_MHz_XTAL); // AM8530H-6PC
	m_scc->configure_channels(4'915'200, 4'915'200, 4'915'200, 4'915'200);
	Z8536(config, m_cio, 10'000'000);
	m_cio->irq_wr_cb().set_inputline(m_cpu, 2);
}

ROM_START( luna_68k_ioc )
	ROM_REGION16_BE(0x10000, "ioc", 0)
	ROM_LOAD16_BYTE("8145__h__3.24.ic108", 0x0000, 0x8000, CRC(d2dde582) SHA1(e34c15e43869be573272503d1f47e9e244536396))
	ROM_LOAD16_BYTE("8145__l__3.24.ic100", 0x0001, 0x8000, CRC(4863329b) SHA1(881623c3a64260f5cc1be066dbb47799d1f2ce14))
ROM_END

const tiny_rom_entry *luna_68k_ioc_device::device_rom_region() const
{
	return ROM_NAME(luna_68k_ioc);
}

DEFINE_DEVICE_TYPE(LUNA_68K_IOC, luna_68k_ioc_device, "luna_68k_ioc", "Omron Luna 68k IOC2 VME board")
