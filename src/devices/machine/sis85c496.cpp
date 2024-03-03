// license:BSD-3-Clause
// copyright-holders:R. Belmont, O. Galibert
/***************************************************************************

    sis85c496.cpp - SiS 85C496/497 PCI chipset
    by R. Belmont (based on i82439hx.cpp/i82439tx.cpp by O. Galibert)

    Unlike Intel chipsets, the southbridge is not a PCI device;
    it connects via a proprietary bus to the northbridge, and the two
    chips appear to software/the BIOS as a single chip.  Thus we emulate
    them in a single file.

***************************************************************************/

#include "emu.h"
#include "sis85c496.h"

#include "bus/pc_kbd/keyboards.h"
#include "speaker.h"

DEFINE_DEVICE_TYPE(SIS85C496_HOST, sis85c496_host_device, "sis85c496_host", "SiS 85C496/497 chipset")

void sis85c496_host_device::config_map(address_map &map)
{
	pci_host_device::config_map(map);
	map(0x40, 0x40).rw(FUNC(sis85c496_host_device::dram_config_r), FUNC(sis85c496_host_device::dram_config_w));
	map(0x44, 0x45).rw(FUNC(sis85c496_host_device::shadow_config_r), FUNC(sis85c496_host_device::shadow_config_w));
	map(0x48, 0x4f).lrw8(
		NAME([this] (offs_t offset) { return m_dram_boundary[offset]; }),
		NAME([this] (offs_t offset, u8 data) {
			logerror("SiS496: DRAM boundary [%02x] set %02x\n", offset, data);
			m_dram_boundary[offset] = data;
		})
	);
	map(0x58, 0x59).rw(FUNC(sis85c496_host_device::ide_vesa_config_r), FUNC(sis85c496_host_device::ide_vesa_config_w));
	map(0x5a, 0x5a).rw(FUNC(sis85c496_host_device::smram_ctrl_r), FUNC(sis85c496_host_device::smram_ctrl_w));
	map(0xc8, 0xcb).rw(FUNC(sis85c496_host_device::mailbox_r), FUNC(sis85c496_host_device::mailbox_w));
	map(0xd0, 0xd0).rw(FUNC(sis85c496_host_device::bios_config_r), FUNC(sis85c496_host_device::bios_config_w));
	map(0xd1, 0xd1).rw(FUNC(sis85c496_host_device::isa_decoder_r), FUNC(sis85c496_host_device::isa_decoder_w));
}

void sis85c496_host_device::internal_io_map(address_map &map)
{
	pci_host_device::io_configuration_access_map(map);
	map(0x0000, 0x001f).rw("dma8237_1", FUNC(am9517a_device::read), FUNC(am9517a_device::write));
	map(0x0020, 0x0021).rw("pic8259_master", FUNC(pic8259_device::read), FUNC(pic8259_device::write));
//	map(0x0022, 0x0023) 85C497 super I/O
	map(0x0040, 0x005f).rw("pit8254", FUNC(pit8254_device::read), FUNC(pit8254_device::write));
	map(0x0060, 0x0063).rw(FUNC(sis85c496_host_device::at_keybc_r), FUNC(sis85c496_host_device::at_keybc_w));
	map(0x0064, 0x0067).rw("keybc", FUNC(at_keyboard_controller_device::status_r), FUNC(at_keyboard_controller_device::command_w));
	map(0x0070, 0x0070).lr8(NAME([this] () { return m_ds12885->get_address(); })).w(FUNC(sis85c496_host_device::rtc_address_nmi_w));
	map(0x0071, 0x0071).rw("rtc", FUNC(ds12885_device::data_r), FUNC(ds12885_device::data_w));
	map(0x0080, 0x009f).rw(FUNC(sis85c496_host_device::at_page8_r), FUNC(sis85c496_host_device::at_page8_w));
	map(0x00a0, 0x00a1).rw("pic8259_slave", FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x00c0, 0x00df).rw(FUNC(sis85c496_host_device::at_dma8237_2_r), FUNC(sis85c496_host_device::at_dma8237_2_w));
	map(0x00e0, 0x00ef).noprw();
}

void sis85c496_host_device::device_add_mconfig(machine_config &config)
{
	AM9517A(config, m_dma8237_1, XTAL(14'318'181)/3);
	m_dma8237_1->out_hreq_callback().set(m_dma8237_2, FUNC(am9517a_device::dreq0_w));
	m_dma8237_1->out_eop_callback().set(FUNC(sis85c496_host_device::at_dma8237_out_eop));
	m_dma8237_1->in_memr_callback().set(FUNC(sis85c496_host_device::pc_dma_read_byte));
	m_dma8237_1->out_memw_callback().set(FUNC(sis85c496_host_device::pc_dma_write_byte));
	m_dma8237_1->in_ior_callback<0>().set(FUNC(sis85c496_host_device::pc_dma8237_0_dack_r));
	m_dma8237_1->in_ior_callback<1>().set(FUNC(sis85c496_host_device::pc_dma8237_1_dack_r));
	m_dma8237_1->in_ior_callback<2>().set(FUNC(sis85c496_host_device::pc_dma8237_2_dack_r));
	m_dma8237_1->in_ior_callback<3>().set(FUNC(sis85c496_host_device::pc_dma8237_3_dack_r));
	m_dma8237_1->out_iow_callback<0>().set(FUNC(sis85c496_host_device::pc_dma8237_0_dack_w));
	m_dma8237_1->out_iow_callback<1>().set(FUNC(sis85c496_host_device::pc_dma8237_1_dack_w));
	m_dma8237_1->out_iow_callback<2>().set(FUNC(sis85c496_host_device::pc_dma8237_2_dack_w));
	m_dma8237_1->out_iow_callback<3>().set(FUNC(sis85c496_host_device::pc_dma8237_3_dack_w));
	m_dma8237_1->out_dack_callback<0>().set(FUNC(sis85c496_host_device::pc_dack0_w));
	m_dma8237_1->out_dack_callback<1>().set(FUNC(sis85c496_host_device::pc_dack1_w));
	m_dma8237_1->out_dack_callback<2>().set(FUNC(sis85c496_host_device::pc_dack2_w));
	m_dma8237_1->out_dack_callback<3>().set(FUNC(sis85c496_host_device::pc_dack3_w));

	AM9517A(config, m_dma8237_2, XTAL(14'318'181)/3);
	m_dma8237_2->out_hreq_callback().set(FUNC(sis85c496_host_device::pc_dma_hrq_changed));
	m_dma8237_2->in_memr_callback().set(FUNC(sis85c496_host_device::pc_dma_read_word));
	m_dma8237_2->out_memw_callback().set(FUNC(sis85c496_host_device::pc_dma_write_word));
	m_dma8237_2->in_ior_callback<1>().set(FUNC(sis85c496_host_device::pc_dma8237_5_dack_r));
	m_dma8237_2->in_ior_callback<2>().set(FUNC(sis85c496_host_device::pc_dma8237_6_dack_r));
	m_dma8237_2->in_ior_callback<3>().set(FUNC(sis85c496_host_device::pc_dma8237_7_dack_r));
	m_dma8237_2->out_iow_callback<1>().set(FUNC(sis85c496_host_device::pc_dma8237_5_dack_w));
	m_dma8237_2->out_iow_callback<2>().set(FUNC(sis85c496_host_device::pc_dma8237_6_dack_w));
	m_dma8237_2->out_iow_callback<3>().set(FUNC(sis85c496_host_device::pc_dma8237_7_dack_w));
	m_dma8237_2->out_dack_callback<0>().set(FUNC(sis85c496_host_device::pc_dack4_w));
	m_dma8237_2->out_dack_callback<1>().set(FUNC(sis85c496_host_device::pc_dack5_w));
	m_dma8237_2->out_dack_callback<2>().set(FUNC(sis85c496_host_device::pc_dack6_w));
	m_dma8237_2->out_dack_callback<3>().set(FUNC(sis85c496_host_device::pc_dack7_w));

	PIC8259(config, m_pic8259_master, 0);
	m_pic8259_master->out_int_callback().set_inputline(m_maincpu, 0);
	m_pic8259_master->in_sp_callback().set_constant(1);
	m_pic8259_master->read_slave_ack_callback().set(FUNC(sis85c496_host_device::get_slave_ack));

	PIC8259(config, m_pic8259_slave, 0);
	m_pic8259_slave->out_int_callback().set(m_pic8259_master, FUNC(pic8259_device::ir2_w));
	m_pic8259_slave->in_sp_callback().set_constant(0);

	PIT8254(config, m_pit8254, 0);
	m_pit8254->set_clk<0>(4772720/4); // heartbeat IRQ
	m_pit8254->out_handler<0>().set(m_pic8259_master, FUNC(pic8259_device::ir0_w));
	m_pit8254->set_clk<1>(4772720/4); // DRAM refresh
	m_pit8254->out_handler<1>().set(FUNC(sis85c496_host_device::at_pit8254_out1_changed));
	m_pit8254->set_clk<2>(4772720/4); // PIO port C pin 4, and speaker polling enough
	m_pit8254->out_handler<2>().set(FUNC(sis85c496_host_device::at_pit8254_out2_changed));

	// TODO: PS/2
	AT_KEYBOARD_CONTROLLER(config, m_keybc, XTAL(12'000'000));
	m_keybc->hot_res().set(FUNC(sis85c496_host_device::cpu_reset_w));
	m_keybc->gate_a20().set(FUNC(sis85c496_host_device::cpu_a20_w));
	m_keybc->kbd_irq().set(m_pic8259_master, FUNC(pic8259_device::ir1_w));
	m_keybc->kbd_clk().set(m_pc_kbdc, FUNC(pc_kbdc_device::clock_write_from_mb));
	m_keybc->kbd_data().set(m_pc_kbdc, FUNC(pc_kbdc_device::data_write_from_mb));

	PC_KBDC(config, m_pc_kbdc, pc_at_keyboards, STR_KBD_MICROSOFT_NATURAL);
	m_pc_kbdc->out_clock_cb().set("keybc", FUNC(at_keyboard_controller_device::kbd_clk_w));
	m_pc_kbdc->out_data_cb().set("keybc", FUNC(at_keyboard_controller_device::kbd_data_w));

	DS12885(config, m_ds12885);
	m_ds12885->irq().set(m_pic8259_slave, FUNC(pic8259_device::ir0_w));
	m_ds12885->set_century_index(0x32);

	// TODO: ISA bus clock, irqs
	ISA16(config, m_isabus, 0);
	m_isabus->set_memspace(m_maincpu, AS_PROGRAM);
	m_isabus->set_iospace(m_maincpu, AS_IO);
	m_isabus->irq3_callback().set(FUNC(sis85c496_host_device::pc_irq3_w));
	m_isabus->irq4_callback().set(FUNC(sis85c496_host_device::pc_irq4_w));
	m_isabus->irq5_callback().set(FUNC(sis85c496_host_device::pc_irq5_w));
	m_isabus->irq6_callback().set(FUNC(sis85c496_host_device::pc_irq6_w));
	m_isabus->irq7_callback().set(FUNC(sis85c496_host_device::pc_irq7_w));
	m_isabus->irq2_callback().set(FUNC(sis85c496_host_device::pc_irq9_w));
	m_isabus->irq10_callback().set(FUNC(sis85c496_host_device::pc_irq10_w));
	m_isabus->irq11_callback().set(FUNC(sis85c496_host_device::pc_irq11_w));
	m_isabus->irq12_callback().set(FUNC(sis85c496_host_device::pc_irq12m_w));
	m_isabus->irq14_callback().set(FUNC(sis85c496_host_device::pc_irq14_w));
	m_isabus->irq15_callback().set(FUNC(sis85c496_host_device::pc_irq15_w));
	m_isabus->drq0_callback().set(m_dma8237_1, FUNC(am9517a_device::dreq0_w));
	m_isabus->drq1_callback().set(m_dma8237_1, FUNC(am9517a_device::dreq1_w));
	m_isabus->drq2_callback().set(m_dma8237_1, FUNC(am9517a_device::dreq2_w));
	m_isabus->drq3_callback().set(m_dma8237_1, FUNC(am9517a_device::dreq3_w));
	m_isabus->drq5_callback().set(m_dma8237_2, FUNC(am9517a_device::dreq1_w));
	m_isabus->drq6_callback().set(m_dma8237_2, FUNC(am9517a_device::dreq2_w));
	m_isabus->drq7_callback().set(m_dma8237_2, FUNC(am9517a_device::dreq3_w));
	m_isabus->iochck_callback().set(FUNC(sis85c496_host_device::iochck_w));

	IDE_CONTROLLER_32(config, m_ide[0]).options(ata_devices, "hdd", nullptr, false);
	m_ide[0]->irq_handler().set(m_pic8259_slave, FUNC(pic8259_device::ir6_w));

	IDE_CONTROLLER_32(config, m_ide[1]).options(ata_devices, "cdrom", nullptr, false);
	m_ide[1]->irq_handler().set(m_pic8259_slave, FUNC(pic8259_device::ir7_w));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);
}


sis85c496_host_device::sis85c496_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_host_device(mconfig, SIS85C496_HOST, tag, owner, clock),
	m_maincpu(*this, finder_base::DUMMY_TAG),
	m_pic8259_master(*this, "pic8259_master"),
	m_pic8259_slave(*this, "pic8259_slave"),
	m_dma8237_1(*this, "dma8237_1"),
	m_dma8237_2(*this, "dma8237_2"),
	m_pit8254(*this, "pit8254"),
	m_keybc(*this, "keybc"),
	m_speaker(*this, "speaker"),
	m_ds12885(*this, "rtc"),
	m_pc_kbdc(*this, "kbd"),
	m_isabus(*this, "isabus"),
	m_ide(*this, "ide%u", 1U),
	m_at_spkrdata(0), m_pit_out2(0), m_dma_channel(0), m_cur_eop(false), m_dma_high_byte(0), m_at_speaker(0), m_refresh(false), m_channel_check(0), m_nmi_enabled(0)
{
}

void sis85c496_host_device::set_cpu_tag(const char *cpu_tag)
{
	m_maincpu.set_tag(cpu_tag);
}

void sis85c496_host_device::set_ram_size(int _ram_size)
{
	ram_size = _ram_size;
}

void sis85c496_host_device::device_start()
{
	pci_host_device::device_start();

	set_spaces(&m_maincpu->space(AS_PROGRAM), &m_maincpu->space(AS_IO));

	memory_window_start = 0;
	memory_window_end   = 0xffffffff;
	memory_offset       = 0;
	io_window_start = 0;
	io_window_end   = 0xffff;
	io_offset       = 0;
	status = 0x0010;

	m_bios_config = 0x78;
	m_dram_config = 0;
	m_isa_decoder = 0xff;
	m_shadctrl = 0;
	m_smramctrl = 0;

	ram.resize(ram_size/4);

	save_item(NAME(m_shadctrl));
}

void sis85c496_host_device::reset_all_mappings()
{
	pci_host_device::reset_all_mappings();
}

void sis85c496_host_device::device_reset()
{
	pci_host_device::device_reset();

	m_at_spkrdata = 0;
	m_pit_out2 = 1;
	m_dma_channel = -1;
	m_cur_eop = false;
	m_nmi_enabled = 0;
	m_refresh = false;

	m_bios_config = 0x78;
	m_dram_config = 0;
	m_isa_decoder = 0xff;
	m_shadctrl = 0;
	m_smramctrl = 0;
	m_ide_vesa_ctrl = 0;
}

void sis85c496_host_device::device_config_complete()
{
	auto isabus = m_isabus.finder_target();
	isabus.first.subdevice<isa16_device>(isabus.second)->set_memspace(m_maincpu, AS_PROGRAM);
	isabus.first.subdevice<isa16_device>(isabus.second)->set_iospace(m_maincpu, AS_IO);

	pci_device::device_config_complete();
}

void sis85c496_host_device::map_bios(address_space *memory_space, uint32_t start, uint32_t end)
{
	uint32_t mask = m_region->bytes() - 1;
	memory_space->install_rom(start, end, m_region->base() + (start & mask));
}

void sis85c496_host_device::map_shadowram(address_space *memory_space, offs_t addrstart, offs_t addrend, void *baseptr)
{
	if (m_shadctrl & 0x100) // write protected?
	{
		memory_space->install_rom(addrstart, addrend, baseptr);
	}
	else
	{
		memory_space->install_ram(addrstart, addrend, baseptr);
	}
}

void sis85c496_host_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
									 uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	logerror("SiS496: mapping!\n");
	io_space->install_device(0, 0xffff, *this, &sis85c496_host_device::internal_io_map);

	// is SMRAM at e0000?  overrides shadow if so
	if ((m_smramctrl & 0x16) == 0x16)
	{
		if (m_smramctrl & 0x08)
		{
			memory_space->install_ram(0x000e0000, 0x000effff, &ram[0x000b0000/4]);
			logerror("SiS496: SMRAM at Exxxx, phys Bxxxx\n");
		}
		else
		{
			memory_space->install_ram(0x000e0000, 0x000effff, &ram[0x000a0000/4]);
			logerror("SiS496: SMRAM at Exxxx, phys Axxxx\n");
		}

		// map the high BIOS at FFFExxxx if enabled
		if (m_bios_config & 0x40)
		{
			map_bios(memory_space, 0xfffe0000, 0xfffeffff);
		}
	}
	else
	{
		// does shadow RAM actually require this to be set?  can't tell w/Megatouch BIOS.
		if (m_bios_config & 0x40)
		{
			logerror("SiS496: BIOS at Exxxx\n");
			map_bios(memory_space, 0xfffe0000, 0xfffeffff);

			if ((m_shadctrl & 0x30) == 0)
			{
				map_bios(memory_space, 0x000e0000, 0x000effff);
			}
			else    // at least one 32K block has shadow memory
			{
				if (m_shadctrl & 0x20)
				{
					logerror("SiS496: shadow RAM at e8000\n");
					map_shadowram(memory_space, 0x000e8000, 0x000effff, &ram[0x000e8000/4]);
				}

				if (m_shadctrl & 0x10)
				{
					logerror("SiS496: shadow RAM at e0000\n");
					map_shadowram(memory_space, 0x000e0000, 0x000e7fff, &ram[0x000e0000/4]);
				}
			}
		}
	}
	if (m_bios_config & 0x20)
	{
		map_bios(memory_space, 0xffff0000, 0xffffffff);

		if ((m_shadctrl & 0xc0) == 0)
		{
			map_bios(memory_space, 0x000f0000, 0x000fffff);
			logerror("SiS496: BIOS at Fxxxx\n");
		}
		else    // at least one 32K block has shadow memory
		{
			if (m_shadctrl & 0x80)
			{
				logerror("SiS496: shadow RAM at f8000\n");
				map_shadowram(memory_space, 0x000f8000, 0x000fffff, &ram[0x000f8000/4]);
			}

			if (m_shadctrl & 0x40)
			{
				logerror("SiS496: shadow RAM at f0000\n");
				map_shadowram(memory_space, 0x000f0000, 0x000f7fff, &ram[0x000f0000/4]);
			}
		}
	}

	if (m_shadctrl & 0x08)
	{
		logerror("SiS496: shadow RAM at d8000\n");
		memory_space->install_ram(0x000d8000, 0x000dffff, &ram[0x000d8000/4]);
	}
	if (m_shadctrl & 0x04)
	{
		logerror("SiS496: shadow RAM at d0000\n");
		memory_space->install_ram(0x000d0000, 0x000d7fff, &ram[0x000d0000/4]);
	}
	if (m_shadctrl & 0x02)
	{
		logerror("SiS496: shadow RAM at c8000\n");
		memory_space->install_ram(0x000c8000, 0x000cffff, &ram[0x000c8000/4]);
	}
	if (m_shadctrl & 0x01)
	{
		logerror("SiS496: shadow RAM at d8000\n");
		memory_space->install_ram(0x000c0000, 0x000c7fff, &ram[0x000c0000/4]);
	}

	// is SMRAM enabled at 6xxxx?
	if ((m_smramctrl & 0x12) == 0x02)
	{
		throw emu_fatalerror("SiS496: SMRAM enabled at 6xxxx, not yet supported!\n");
	}

	// TODO: determined by ISA decoder
	m_isabus->remap(AS_PROGRAM, 0, 1 << 24);
	m_isabus->remap(AS_IO, 0, 0xffff);

	if (m_isa_decoder & 0x01)
	{
		logerror("SiS496: ISA base 640K enabled\n");
		memory_space->install_ram(0x00000000, 0x0009ffff, &ram[0x00000000/4]);
	}

	if (BIT(m_ide_vesa_ctrl, 8))
	{
		// TODO: doesn't seem to work right for anything after IDE[0] primary
		// bit 9: swaps IDE channels
		io_space->install_readwrite_handler(0x170, 0x177, read32s_delegate(*m_ide[1], FUNC(ide_controller_32_device::cs0_r)), write32s_delegate(*m_ide[1], FUNC(ide_controller_32_device::cs0_w)));

		io_space->install_readwrite_handler(0x1f0, 0x1f7, read32s_delegate(*m_ide[0], FUNC(ide_controller_32_device::cs0_r)), write32s_delegate(*m_ide[0], FUNC(ide_controller_32_device::cs0_w)));

		//if (!BIT(m_ide_vesa_ctrl, 6))
		io_space->install_readwrite_handler(0x370, 0x377, read32s_delegate(*m_ide[1], FUNC(ide_controller_32_device::cs1_r)), write32s_delegate(*m_ide[1], FUNC(ide_controller_32_device::cs1_w)));

		//if (!BIT(m_ide_vesa_ctrl, 7))
		io_space->install_readwrite_handler(0x3f0, 0x3f7, read32s_delegate(*m_ide[0], FUNC(ide_controller_32_device::cs1_r)), write32s_delegate(*m_ide[0], FUNC(ide_controller_32_device::cs1_w)));
	}

	// 32 megs of RAM (todo: don't hardcode)
	memory_space->install_ram(0x00100000, 0x01ffffff, &ram[0x00100000/4]);
}

// Southbridge
uint8_t sis85c496_host_device::get_slave_ack(offs_t offset)
{
	if (offset==2) // IRQ = 2
		return m_pic8259_slave->acknowledge();

	return 0x00;
}

void sis85c496_host_device::at_speaker_set_spkrdata(uint8_t data)
{
	m_at_spkrdata = data ? 1 : 0;
	m_speaker->level_w(m_at_spkrdata & m_pit_out2);
}

void sis85c496_host_device::at_pit8254_out1_changed(int state)
{
	if(state)
		m_refresh = !m_refresh;
}

void sis85c496_host_device::at_pit8254_out2_changed(int state)
{
	m_pit_out2 = state ? 1 : 0;
	m_speaker->level_w(m_at_spkrdata & m_pit_out2);
}

uint8_t sis85c496_host_device::at_page8_r(offs_t offset)
{
	uint8_t data = m_at_pages[offset % 0x10];

	switch(offset % 8)
	{
	case 1:
		data = m_dma_offset[BIT(offset, 3)][2];
		break;
	case 2:
		data = m_dma_offset[BIT(offset, 3)][3];
		break;
	case 3:
		data = m_dma_offset[BIT(offset, 3)][1];
		break;
	case 7:
		data = m_dma_offset[BIT(offset, 3)][0];
		break;
	}
	return data;
}


void sis85c496_host_device::at_page8_w(offs_t offset, uint8_t data)
{
	m_at_pages[offset % 0x10] = data;

	switch(offset % 8)
	{
	case 0:
		//m_boot_state_hook((offs_t)0, data);
		break;
	case 1:
		m_dma_offset[BIT(offset, 3)][2] = data;
		break;
	case 2:
		m_dma_offset[BIT(offset, 3)][3] = data;
		break;
	case 3:
		m_dma_offset[BIT(offset, 3)][1] = data;
		break;
	case 7:
		m_dma_offset[BIT(offset, 3)][0] = data;
		break;
	}
}


void sis85c496_host_device::pc_dma_hrq_changed(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	/* Assert HLDA */
	m_dma8237_2->hack_w( state );
}

uint8_t sis85c496_host_device::pc_dma_read_byte(offs_t offset)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM); // get the right address space
	if(m_dma_channel == -1)
		return 0xff;
	uint8_t result;
	offs_t page_offset = ((offs_t) m_dma_offset[0][m_dma_channel]) << 16;

	result = prog_space.read_byte(page_offset + offset);
	return result;
}


void sis85c496_host_device::pc_dma_write_byte(offs_t offset, uint8_t data)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM); // get the right address space
	if(m_dma_channel == -1)
		return;
	offs_t page_offset = ((offs_t) m_dma_offset[0][m_dma_channel]) << 16;

	prog_space.write_byte(page_offset + offset, data);
}


uint8_t sis85c496_host_device::pc_dma_read_word(offs_t offset)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM); // get the right address space
	if(m_dma_channel == -1)
		return 0xff;
	uint16_t result;
	offs_t page_offset = ((offs_t) m_dma_offset[1][m_dma_channel & 3]) << 16;

	result = prog_space.read_word((page_offset & 0xfe0000) | (offset << 1));
	m_dma_high_byte = result & 0xFF00;

	return result & 0xFF;
}


void sis85c496_host_device::pc_dma_write_word(offs_t offset, uint8_t data)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM); // get the right address space
	if(m_dma_channel == -1)
		return;
	offs_t page_offset = ((offs_t) m_dma_offset[1][m_dma_channel & 3]) << 16;

	prog_space.write_word((page_offset & 0xfe0000) | (offset << 1), m_dma_high_byte | data);
}


uint8_t sis85c496_host_device::pc_dma8237_0_dack_r() { return 0; } //m_isabus->dack_r(0); }
uint8_t sis85c496_host_device::pc_dma8237_1_dack_r() { return 0; } //m_isabus->dack_r(1); }
uint8_t sis85c496_host_device::pc_dma8237_2_dack_r() { return 0; } //m_isabus->dack_r(2); }
uint8_t sis85c496_host_device::pc_dma8237_3_dack_r() { return 0; } //m_isabus->dack_r(3); }
uint8_t sis85c496_host_device::pc_dma8237_5_dack_r() { return 0; } //m_isabus->dack_r(5); }
uint8_t sis85c496_host_device::pc_dma8237_6_dack_r() { return 0; } //m_isabus->dack_r(6); }
uint8_t sis85c496_host_device::pc_dma8237_7_dack_r() { return 0; } //m_isabus->dack_r(7); }


void sis85c496_host_device::pc_dma8237_0_dack_w(uint8_t data) { } //m_isabus->dack_w(0, data); }
void sis85c496_host_device::pc_dma8237_1_dack_w(uint8_t data) { } //m_isabus->dack_w(1, data); }
void sis85c496_host_device::pc_dma8237_2_dack_w(uint8_t data) { } //m_isabus->dack_w(2, data); }
void sis85c496_host_device::pc_dma8237_3_dack_w(uint8_t data) { } //m_isabus->dack_w(3, data); }
void sis85c496_host_device::pc_dma8237_5_dack_w(uint8_t data) { } //m_isabus->dack_w(5, data); }
void sis85c496_host_device::pc_dma8237_6_dack_w(uint8_t data) { } //m_isabus->dack_w(6, data); }
void sis85c496_host_device::pc_dma8237_7_dack_w(uint8_t data) { } //m_isabus->dack_w(7, data); }

void sis85c496_host_device::at_dma8237_out_eop(int state)
{
	m_cur_eop = state == ASSERT_LINE;
	//if(m_dma_channel != -1)
//      m_isabus->eop_w(m_dma_channel, m_cur_eop ? ASSERT_LINE : CLEAR_LINE );
}

void sis85c496_host_device::pc_select_dma_channel(int channel, bool state)
{
	//m_isabus->dack_line_w(channel, state);

	if(!state) {
		m_dma_channel = channel;
		//if(m_cur_eop)
//          m_isabus->eop_w(channel, ASSERT_LINE );

	} else if(m_dma_channel == channel) {
		m_dma_channel = -1;
		//if(m_cur_eop)
//          m_isabus->eop_w(channel, CLEAR_LINE );
	}
}


void sis85c496_host_device::pc_dack0_w(int state) { pc_select_dma_channel(0, state); }
void sis85c496_host_device::pc_dack1_w(int state) { pc_select_dma_channel(1, state); }
void sis85c496_host_device::pc_dack2_w(int state) { pc_select_dma_channel(2, state); }
void sis85c496_host_device::pc_dack3_w(int state) { pc_select_dma_channel(3, state); }
void sis85c496_host_device::pc_dack4_w(int state) { m_dma8237_1->hack_w( state ? 0 : 1); } // it's inverted
void sis85c496_host_device::pc_dack5_w(int state) { pc_select_dma_channel(5, state); }
void sis85c496_host_device::pc_dack6_w(int state) { pc_select_dma_channel(6, state); }
void sis85c496_host_device::pc_dack7_w(int state) { pc_select_dma_channel(7, state); }

void sis85c496_host_device::pc_irq1_w(int state)   { m_pic8259_master->ir1_w(state); }
void sis85c496_host_device::pc_irq3_w(int state)   { m_pic8259_master->ir3_w(state); }
void sis85c496_host_device::pc_irq4_w(int state)   { m_pic8259_master->ir4_w(state); }
void sis85c496_host_device::pc_irq5_w(int state)   { m_pic8259_master->ir5_w(state); }
void sis85c496_host_device::pc_irq6_w(int state)   { m_pic8259_master->ir6_w(state); }
void sis85c496_host_device::pc_irq7_w(int state)   { m_pic8259_master->ir7_w(state); }
void sis85c496_host_device::pc_irq8n_w(int state)  { m_pic8259_slave->ir0_w(state); }
void sis85c496_host_device::pc_irq9_w(int state)   { m_pic8259_slave->ir1_w(state); }
void sis85c496_host_device::pc_irq10_w(int state)  { m_pic8259_slave->ir2_w(state); }
void sis85c496_host_device::pc_irq11_w(int state)  { m_pic8259_slave->ir3_w(state); }
void sis85c496_host_device::pc_irq12m_w(int state) { m_pic8259_slave->ir4_w(state); }
void sis85c496_host_device::pc_irq14_w(int state)  { m_pic8259_slave->ir6_w(state); }
void sis85c496_host_device::pc_irq15_w(int state)  { m_pic8259_slave->ir7_w(state); }

uint8_t sis85c496_host_device::at_portb_r()
{
	uint8_t data = m_at_speaker;
	data &= ~0xd0; /* AT BIOS don't likes this being set */

	/* 0x10 is the dram refresh line bit on the 5170, just a timer here, 15.085us. */
	data |= m_refresh ? 0x10 : 0;

	if (m_pit_out2)
		data |= 0x20;
	else
		data &= ~0x20; /* ps2m30 wants this */

	return data;
}

void sis85c496_host_device::at_portb_w(uint8_t data)
{
	m_at_speaker = data;
	m_pit8254->write_gate2(BIT(data, 0));
	at_speaker_set_spkrdata( BIT(data, 1));
	m_channel_check = BIT(data, 3);
	if (m_channel_check)
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

void sis85c496_host_device::iochck_w(int state)
{
	if (!state && !m_channel_check && m_nmi_enabled)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

uint8_t sis85c496_host_device::at_dma8237_2_r(offs_t offset)
{
	return m_dma8237_2->read(offset / 2);
}

void sis85c496_host_device::at_dma8237_2_w(offs_t offset, uint8_t data)
{
	m_dma8237_2->write(offset / 2, data);
}

uint8_t sis85c496_host_device::at_keybc_r(offs_t offset)
{
	switch (offset)
	{
	case 0: return m_keybc->data_r();
	case 1: return at_portb_r();
	}

	return 0xff;
}

void sis85c496_host_device::at_keybc_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0: m_keybc->data_w(data); break;
	case 1: at_portb_w(data); break;
	}
}

void sis85c496_host_device::rtc_address_nmi_w(uint8_t data)
{
	m_nmi_enabled = BIT(data, 7);
	//m_isabus->set_nmi_state((m_nmi_enabled==0) && (m_channel_check==0));
	m_ds12885->address_w(data);
}

void sis85c496_host_device::cpu_a20_w(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_A20, state);
}

void sis85c496_host_device::cpu_reset_w(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, state);
}

/*

after decompress to shadow RAM:

config_write 00:05.0:40 00000004 @ 000000ff
[:pci:05.0] ':maincpu' (000FF6D8): unmapped configuration_space memory write to 28058 = 00120000 & 00FF0000 SMRAM: e0000 to SMRAM, enable
config_write 00:05.0:58 00120000 @ 00ff0000
[:pci:05.0] ':maincpu' (000FF6D8): unmapped configuration_space memory write to 28058 = 00040000 & 00FF0000 SMRAM: always enable
config_write 00:05.0:58 00040000 @ 00ff0000
[:pci:05.0] ':maincpu' (000FF6D8): unmapped configuration_space memory write to 28058 = 00000000 & 00FF0000 SMRAM: disable
config_write 00:05.0:58 00000000 @ 00ff0000
[:pci:05.0] ':maincpu' (000FF6D8): unmapped configuration_space memory write to 280A0 = 0000FF00 & 0000FF00 SMI: clear all requests
config_write 00:05.0:a0 0000ff00 @ 0000ff00
[:pci:05.0] ':maincpu' (000FF6D8): unmapped configuration_space memory write to 280A0 = 000000FF & 000000FF SMI: clear all requests
config_write 00:05.0:a0 000000ff @ 000000ff
[:pci:05.0] ':maincpu' (000FF6D8): unmapped configuration_space memory write to 2808C = 00000500 & 0000FF00 SMI: timer count
config_write 00:05.0:8c 00000500 @ 0000ff00
[:pci:05.0] ':maincpu' (000FF6D8): unmapped configuration_space memory write to 2809C = 00020000 & 00FF0000 SMI: start countdown timer
config_write 00:05.0:9c 00020000 @ 00ff0000
[:pci:05.0] ':maincpu' (000FF6D8): unmapped configuration_space memory write to 28084 = 00000006 & 000000FF clear deturbo and break switch blocks
config_write 00:05.0:84 00000006 @ 000000ff
[:pci:05.0] ':maincpu' (000FF6D8): unmapped configuration_space memory write to 28080 = 00000004 & 000000FF enable soft-SMI
config_write 00:05.0:80 00000004 @ 000000ff
[:pci:05.0] ':maincpu' (000FF6D8): unmapped configuration_space memory write to 280A0 = 00100000 & 00FF0000 select software SMI request
config_write 00:05.0:a0 00100000 @ 00ff0000
[:pci:05.0] ':maincpu' (000FF6D8): unmapped configuration_space memory write to 2809C = 00010000 & 00FF0000 assert SMI
config_write 00:05.0:9c 00010000 @ 00ff0000
[:pci:05.0] ':maincpu' (000FF6D8): unmapped configuration_space memory write to 280C4 = 00080000 & 00FF0000 IRQ routing: undocumented value
config_write 00:05.0:c4 00080000 @ 00ff0000
[:pci:05.0] ':maincpu' (000FF6D8): unmapped configuration_space memory write to 28080 = 00000000 & 000000FF clear all SMI
config_write 00:05.0:80 00000000 @ 000000ff



*/
