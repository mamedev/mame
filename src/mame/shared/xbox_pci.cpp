// license:BSD-3-Clause
// copyright-holders:Samuele Zannoli

#include "emu.h"
#include "xbox_pci.h"

#include "xbox.h"

#include "machine/pci.h"
#include "machine/idectrl.h"
#include "machine/ds128x.h"

#include <functional>

//#define LOG_AUDIO

/*
 * Host
 */

DEFINE_DEVICE_TYPE(NV2A_HOST, nv2a_host_device, "nv2a_host", "NV2A PCI Bridge Device - Host Bridge")

nv2a_host_device::nv2a_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	pci_host_device(mconfig, NV2A_HOST, tag, owner, clock),
	cpu(*this, finder_base::DUMMY_TAG)
{
	set_ids_host(0x10de02a5, 0, 0);
}

void nv2a_host_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
	uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	io_space->install_device(0, 0xffff, *static_cast<pci_host_device *>(this), &pci_host_device::io_configuration_access_map);
}

void nv2a_host_device::device_start()
{
	pci_host_device::device_start();
	memory_space = &cpu->space(AS_PROGRAM);
	io_space = &cpu->space(AS_IO);

	// do not change the next two
	memory_window_start = 0x10000000;
	memory_window_end = 0xfeefffff;
	memory_offset = 0;
	// do not change the next two
	io_window_start = 0x5000;
	io_window_end = 0xefff;
	io_offset = 0;
}

void nv2a_host_device::device_reset()
{
	pci_host_device::device_reset();
}

/*
 * Ram
 */

DEFINE_DEVICE_TYPE(NV2A_RAM, nv2a_ram_device, "nv2a_ram", "NV2A Memory Controller - SDRAM")

void nv2a_ram_device::config_map(address_map &map)
{
	pci_device::config_map(map);
	map(0x6c, 0x6f).rw(FUNC(nv2a_ram_device::config_register_r), FUNC(nv2a_ram_device::config_register_w));
}

nv2a_ram_device::nv2a_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, NV2A_RAM, tag, owner, clock)
{
	set_ids(0x10de02a6, 0, 0, 0);
}

void nv2a_ram_device::device_start()
{
	pci_device::device_start();
	ram.resize(ram_size * 1024 * 1024 / 4);
}

uint32_t nv2a_ram_device::config_register_r()
{
	return 0x08800044;
}

void nv2a_ram_device::config_register_w(uint32_t data)
{
}

void nv2a_ram_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
	uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	memory_space->install_ram(0x00000000, ram_size * 1024 * 1024 - 1, &ram[0]);
}

/*
 * LPC Bus
 */

DEFINE_DEVICE_TYPE(MCPX_ISALPC, mcpx_isalpc_device, "mcpx_isalpc", "MCPX HUB Interface - ISA Bridge")

void mcpx_isalpc_device::lpc_io(address_map &map)
{
	map(0x00000000, 0x000000ff).rw(FUNC(mcpx_isalpc_device::acpi_r), FUNC(mcpx_isalpc_device::acpi_w));
}

void mcpx_isalpc_device::internal_io_map(address_map &map)
{
	map(0x0020, 0x0023).rw("pic8259_1", FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x0040, 0x0043).rw("pit8254", FUNC(pit8254_device::read), FUNC(pit8254_device::write));
	map(0x0061, 0x0061).rw(FUNC(mcpx_isalpc_device::portb_r), FUNC(mcpx_isalpc_device::portb_w));
	map(0x0070, 0x0073).rw("rtc", FUNC(ds12885ext_device::read_extended), FUNC(ds12885ext_device::write_extended));
	map(0x0080, 0x0080).w(FUNC(mcpx_isalpc_device::boot_state_w));
	map(0x00a0, 0x00a3).rw("pic8259_2", FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x00e0, 0x00e3).nopw();
}

void mcpx_isalpc_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
	uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	io_space->install_device(0, 0xffff, *this, &mcpx_isalpc_device::internal_io_map);
	for (int a = 0; a < 16; a++)
		if (lpcdevices[a] != nullptr)
			lpcdevices[a]->map_extra(memory_space, io_space);
}

mcpx_isalpc_device::mcpx_isalpc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint32_t subsystem_id)
	: mcpx_isalpc_device(mconfig, tag, owner, clock)
{
	// revision id must be at least 0xb4 in the xbox, otherwise usb will require a hub
	// in the a7n266-c motherboard it has revision 0xc3
	set_ids(0x10de01b2, 0xb4, 0x060100, subsystem_id);
}

mcpx_isalpc_device::mcpx_isalpc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, MCPX_ISALPC, tag, owner, clock),
	m_smi_callback(*this),
	m_interrupt_output(*this),
	m_boot_state_hook(*this),
	pic8259_1(*this, "pic8259_1"),
	pic8259_2(*this, "pic8259_2"),
	pit8254(*this, "pit8254"),
	m_pm1_status(0),
	m_pm1_enable(0),
	m_pm1_control(0),
	m_pm1_timer(0),
	m_gpe0_status(0),
	m_gpe0_enable(0),
	m_global_smi_control(0),
	m_smi_command_port(0),
	m_speaker(0),
	m_refresh(false),
	m_pit_out2(0),
	m_spkrdata(0),
	m_channel_check(0)
{
}

void mcpx_isalpc_device::device_start()
{
	pci_device::device_start();
	set_multifunction_device(true);
	m_smi_callback.resolve_safe();
	m_interrupt_output.resolve_safe();
	m_boot_state_hook.resolve_safe();
	add_map(0x00000100, M_IO, FUNC(mcpx_isalpc_device::lpc_io));
	bank_infos[0].adr = 0x8000;
	status = 0x00b0;
	command = 0x0001;
	command_mask = 0x01be;
	for (int a = 0; a < 16; a++)
		lpcdevices[a] = nullptr;
	for (device_t &d : subdevices())
	{
		const char *t = d.basetag();
		int l = strlen(t);

		if (l == 1)
		{
			int address = strtol(t + l, nullptr, 16);

			address = address & 15;
			if (lpcdevices[address] == nullptr)
			{
				lpcbus_device_interface *i = dynamic_cast<lpcbus_device_interface *>(&d);
				lpcdevices[address] = i;
				if (i)
					i->set_host(address, this);
			}
			else
				logerror("Duplicate address for LPC bus device with tag %s\n", t);
			break;
		}
	}
}

void mcpx_isalpc_device::device_reset()
{
	pci_device::device_reset();
	memset(m_gpio_mode, 0, sizeof(m_gpio_mode));
	m_refresh = false;
	m_pit_out2 = 1;
	m_spkrdata = 0;
}

void mcpx_isalpc_device::device_add_mconfig(machine_config &config)
{
	pic8259_device &pic8259_1(PIC8259(config, "pic8259_1", 0));
	pic8259_1.out_int_callback().set(FUNC(mcpx_isalpc_device::interrupt_ouptut_changed));
	pic8259_1.in_sp_callback().set_constant(1);
	pic8259_1.read_slave_ack_callback().set(FUNC(mcpx_isalpc_device::get_slave_ack));

	pic8259_device &pic8259_2(PIC8259(config, "pic8259_2", 0));
	pic8259_2.out_int_callback().set(pic8259_1, FUNC(pic8259_device::ir2_w));
	pic8259_2.in_sp_callback().set_constant(0);

	pit8254_device &pit8254(PIT8254(config, "pit8254", 0));
	pit8254.set_clk<0>(1125000); /* heartbeat IRQ */
	pit8254.out_handler<0>().set(FUNC(mcpx_isalpc_device::pit8254_out0_changed));
	pit8254.set_clk<1>(1125000); /* originally dram refresh, now only legacy support */
	pit8254.out_handler<1>().set(FUNC(mcpx_isalpc_device::pit8254_out1_changed));
	pit8254.set_clk<2>(1125000); /* (unused) pio port c pin 4, and speaker polling enough */
	pit8254.out_handler<2>().set(FUNC(mcpx_isalpc_device::pit8254_out2_changed));

	ds12885ext_device &ds12885(DS12885EXT(config, "rtc", 0));
	ds12885.irq().set(pic8259_2, FUNC(pic8259_device::ir0_w));

	/*
	More devices are needed:
	    82093 compatible I/O APIC
	    dual 8237 DMA controllers
	*/
}

void mcpx_isalpc_device::update_smi_line()
{
	if (m_global_smi_control)
		m_smi_callback(1);
	else
		m_smi_callback(0);
}

uint32_t mcpx_isalpc_device::acpi_r(offs_t offset, uint32_t mem_mask)
{
	logerror("Acpi read from %04X mask %08X\n", (bank_infos[0].adr & 0xfffffffe) + offset * 4, mem_mask);
	if ((offset == 0xa) && ACCESSING_BITS_0_15)
		return m_global_smi_control;
	if ((offset == 0xb) && ACCESSING_BITS_16_23)
		return m_smi_command_port << 16;
	return 0;
}

void mcpx_isalpc_device::acpi_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	logerror("Acpi write %08X to %04X mask %08X\n", data, (bank_infos[0].adr & 0xfffffffe) + offset * 4, mem_mask);
	// Seen using word registers at offsets
	// 0x00 0x02 0x04 0x08 0x20 0x22 0x28 0xa0 0xa2 0xc0-0xd8
	// Byte access at 0x2e
	if ((offset == 0) && ACCESSING_BITS_0_15)
		// pm1 status register
		m_pm1_status = data & 0xffff;
	else if ((offset == 0) && ACCESSING_BITS_16_31)
		// pm1 enable register
		m_pm1_enable = data >> 16;
	else if ((offset == 1) && ACCESSING_BITS_0_15)
		// pm1 control register
		m_pm1_control = data & 0xffff;
	else if ((offset == 2) && ACCESSING_BITS_0_15)
		// pm1 timer register
		m_pm1_timer = data & 0xffff;
	else if ((offset == 8) && ACCESSING_BITS_0_15)
		// gpe0 status register
		m_gpe0_status = data & 0xffff;
	else if ((offset == 8) && ACCESSING_BITS_16_31)
		// gpe0 enable register
		m_gpe0_enable = data >> 16;
	else if ((offset == 0xa) && ACCESSING_BITS_0_15)
	{
		// Global SMI Control
		m_global_smi_control = m_global_smi_control & (~data & 0xffff);
		update_smi_line();
	}
	else if ((offset == 0xb) && ACCESSING_BITS_16_23)
	{
		// SMI Command Port
		// write to byte 0x2e must generate a SMI interrupt
		m_smi_command_port = (data >> 16) & 0xff;
		m_global_smi_control |= 0x200;
		update_smi_line();
		logerror("Generate software SMI with value %02X\n", m_smi_command_port);
	}
	else if (((offset >= 0x30) && (offset < 0x36)) || ((offset == 0x36) && ACCESSING_BITS_0_15))
	{
		int m = offset != 0x36 ? 4 : 2;
		int p = (offset - 0x30) * 4;

		for (int a = 0; a < m; a++)
		{
			m_gpio_mode[p] = (m_gpio_mode[p] & (~mem_mask & 0xff)) | (data & 0xff);
			p++;
			data = data >> 8;
			mem_mask = mem_mask >> 8;
		}
	}
	else
		logerror("Acpi write not recognized\n");
}

void mcpx_isalpc_device::boot_state_w(uint8_t data)
{
	if (m_boot_state_hook)
		m_boot_state_hook((offs_t)0, data);
}

WRITE_LINE_MEMBER(mcpx_isalpc_device::interrupt_ouptut_changed)
{
	m_interrupt_output(state);
}

uint8_t mcpx_isalpc_device::get_slave_ack(offs_t offset)
{
	if (offset == 2) // IRQ = 2
		return pic8259_2->acknowledge();
	return 0x00;
}

WRITE_LINE_MEMBER(mcpx_isalpc_device::pit8254_out0_changed)
{
	pic8259_1->ir0_w(state);
}

WRITE_LINE_MEMBER(mcpx_isalpc_device::pit8254_out1_changed)
{
	if (state)
		m_refresh = !m_refresh;
}

WRITE_LINE_MEMBER(mcpx_isalpc_device::pit8254_out2_changed)
{
	m_pit_out2 = state ? 1 : 0;
	//xbox_speaker_set_input(m_at_spkrdata & m_pit_out2);
}

WRITE_LINE_MEMBER(mcpx_isalpc_device::irq1)
{
	pic8259_1->ir1_w(state);
}

WRITE_LINE_MEMBER(mcpx_isalpc_device::irq3)
{
	pic8259_1->ir3_w(state);
}

WRITE_LINE_MEMBER(mcpx_isalpc_device::irq10)
{
	pic8259_2->ir2_w(state);
}

WRITE_LINE_MEMBER(mcpx_isalpc_device::irq11)
{
	pic8259_2->ir3_w(state);
}

WRITE_LINE_MEMBER(mcpx_isalpc_device::irq14)
{
	pic8259_2->ir6_w(state);
}

WRITE_LINE_MEMBER(mcpx_isalpc_device::irq15)
{
	pic8259_2->ir7_w(state);
}

uint8_t mcpx_isalpc_device::portb_r()
{
	uint8_t data = m_speaker;

	data &= ~0xd0; /* AT BIOS don't likes this being set */
	/* 0x10 is the dram refresh line bit on the 5170, just a timer here, 15.085us. */
	data |= m_refresh ? 0x10 : 0;
	if (m_pit_out2)
		data |= 0x20;
	else
		data &= ~0x20; /* ps2m30 wants this */

	return data;
}

void mcpx_isalpc_device::portb_w(uint8_t data)
{
	m_speaker = data;
	pit8254->write_gate2(BIT(data, 0));
	speaker_set_spkrdata(BIT(data, 1));
	m_channel_check = BIT(data, 3);
	//if (m_channel_check) m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

uint32_t mcpx_isalpc_device::acknowledge()
{
	return pic8259_1->acknowledge();
}

void mcpx_isalpc_device::speaker_set_spkrdata(uint8_t data)
{
	m_spkrdata = data ? 1 : 0;
	//xbox_speaker_set_input(m_at_spkrdata & m_pit_out2);
}

void mcpx_isalpc_device::debug_generate_irq(int irq, int state)
{
	set_virtual_line(irq, state);
}

void mcpx_isalpc_device::set_virtual_line(int line, int state)
{
	if (line < 16)
	{
		switch (line)
		{
		case 0:
			pic8259_1->ir0_w(state);
			break;
		case 1:
			pic8259_1->ir1_w(state);
			break;
		case 3:
			pic8259_1->ir3_w(state);
			break;
		case 4:
			pic8259_1->ir4_w(state);
			break;
		case 5:
			pic8259_1->ir5_w(state);
			break;
		case 6:
			pic8259_1->ir6_w(state);
			break;
		case 7:
			pic8259_1->ir7_w(state);
			break;
		case 8:
			pic8259_2->ir0_w(state);
			break;
		case 9:
			pic8259_2->ir1_w(state);
			break;
		case 10:
			pic8259_2->ir2_w(state);
			break;
		case 11:
			pic8259_2->ir3_w(state);
			break;
		case 12:
			pic8259_2->ir4_w(state);
			break;
		case 13:
			pic8259_2->ir5_w(state);
			break;
		case 14:
			pic8259_2->ir6_w(state);
			break;
		case 15:
			pic8259_2->ir7_w(state);
			break;
		}
		return;
	}
/* Will be updated to support dma
    line = line - 16;
    if (line < 4)
    {
        switch (line)
        {
        case 0:
            break;
        case 1:
            break;
        case 2:
            break;
        case 3:
            break;
        }
    }
*/
}

void mcpx_isalpc_device::remap()
{
	remap_cb();
}

/*
 * SMBus
 */

DEFINE_DEVICE_TYPE(MCPX_SMBUS, mcpx_smbus_device, "mcpx_smbus", "MCPX SMBus Controller")

void mcpx_smbus_device::config_map(address_map &map)
{
	pci_device::config_map(map);
	map(0x3e, 0x3e).r(FUNC(mcpx_smbus_device::minimum_grant_r));
	map(0x3f, 0x3f).r(FUNC(mcpx_smbus_device::maximum_latency_r));
}

void mcpx_smbus_device::smbus_io0(address_map &map)
{
	map(0x00000000, 0x0000000f).rw(FUNC(mcpx_smbus_device::smbus0_r), FUNC(mcpx_smbus_device::smbus0_w));
}

void mcpx_smbus_device::smbus_io1(address_map &map)
{
	map(0x00000000, 0x0000000f).rw(FUNC(mcpx_smbus_device::smbus1_r), FUNC(mcpx_smbus_device::smbus1_w));
}

void mcpx_smbus_device::smbus_io2(address_map &map)
{
	map(0x00000000, 0x0000001f).noprw();
}

mcpx_smbus_device::mcpx_smbus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint32_t subsystem_id)
	: mcpx_smbus_device(mconfig, tag, owner, clock)
{
	set_ids(0x10de01b4, 0xc1, 0x0c0500, subsystem_id);
}

mcpx_smbus_device::mcpx_smbus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, MCPX_SMBUS, tag, owner, clock),
	m_interrupt_handler(*this)
{
}

void mcpx_smbus_device::device_start()
{
	pci_device::device_start();
	set_multifunction_device(true);
	m_interrupt_handler.resolve_safe();
	add_map(0x00000010, M_IO, FUNC(mcpx_smbus_device::smbus_io0));
	bank_infos[0].adr = 0x1000;
	add_map(0x00000010, M_IO, FUNC(mcpx_smbus_device::smbus_io1));
	bank_infos[1].adr = 0xc000;
	add_map(0x00000020, M_IO, FUNC(mcpx_smbus_device::smbus_io2));
	bank_infos[2].adr = 0xc200;
	status = 0x00b0;
	command = 0x0001;
	// Min Grant 3
	// Max Latency 1
	intr_pin = 1;
	memset(&smbusst, 0, sizeof(smbusst));
	for (int b = 0; b < 2; b++)
		for (int a = 0; a < 128; a++)
			smbusst[b].devices[a] = nullptr;
	for (device_t &d : subdevices())
	{
		const char *t = d.tag();
		int l = strlen(t);

		while (l > 0)
		{
			l--;
			if (t[l] == ':')
			{
				l++;
				int address = strtol(t + l, nullptr, 16);
				int bus;

				bus = address >> 8;
				address = address & 0xff;
				if ((address > 0) && (address < 128) && (bus >= 0) && (bus <= 1))
				{
					if (smbusst[bus].devices[address] == nullptr)
					{
						smbus_interface *i = dynamic_cast<smbus_interface *>(&d);
						smbusst[bus].devices[address] = i;
					}
					else
						logerror("Duplicate address for SMBus device with tag %s\n", t);
				}
				else
					logerror("Invalid address for SMBus device with tag %s\n", t);
				break;
			}
		}
	}
}

void mcpx_smbus_device::device_reset()
{
	pci_device::device_reset();
}

uint32_t mcpx_smbus_device::smbus_read(int bus, offs_t offset, uint32_t mem_mask)
{
	if (offset == 0) // 0 smbus status
		smbusst[bus].words[offset] = (smbusst[bus].words[offset] & ~0xffff) | ((smbusst[bus].status & 0xffff) << 0);
	if (offset == 1) // 6 smbus data
		smbusst[bus].words[offset] = (smbusst[bus].words[offset] & ~(0xffff << 16)) | ((smbusst[bus].data & 0xffff) << 16);
	return smbusst[bus].words[offset];
}

void mcpx_smbus_device::smbus_write(int bus, offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(smbusst[bus].words);
	if ((offset == 0) && (ACCESSING_BITS_0_7 || ACCESSING_BITS_8_15)) // 0 smbus status
	{
		if (!((smbusst[bus].status ^ data) & 0x10)) // clearing interrupt
		{
			if (m_interrupt_handler)
				m_interrupt_handler(0);
		}
		smbusst[bus].status &= ~data;
	}
	if ((offset == 0) && ACCESSING_BITS_16_23) // 2 smbus control
	{
		data = data >> 16;
		smbusst[bus].control = data;
		int cycletype = smbusst[bus].control & 7;
		if (smbusst[bus].control & 8) { // start
			if ((cycletype & 6) == 2)
			{
				if (smbusst[bus].devices[smbusst[bus].address])
					if (smbusst[bus].rw == 0)
						smbusst[bus].devices[smbusst[bus].address]->execute_command(smbusst[bus].command, smbusst[bus].rw, smbusst[bus].data);
					else
						smbusst[bus].data = smbusst[bus].devices[smbusst[bus].address]->execute_command(smbusst[bus].command, smbusst[bus].rw, smbusst[bus].data);
				else
					logerror("SMBUS: access to missing device at bus %d address %d\n", bus, smbusst[bus].address);
				smbusst[bus].status |= 0x10;
				if (smbusst[bus].control & 0x10)
				{
					if (m_interrupt_handler)
						m_interrupt_handler(1);
				}
			}
		}
	}
	if ((offset == 1) && ACCESSING_BITS_0_7) // 4 smbus address
	{
		smbusst[bus].address = data >> 1;
		smbusst[bus].rw = data & 1;
	}
	if ((offset == 1) && (ACCESSING_BITS_16_23 || ACCESSING_BITS_16_31)) // 6 smbus data
	{
		data = data >> 16;
		smbusst[bus].data = data;
	}
	if ((offset == 2) && ACCESSING_BITS_0_7) // 8 smbus command
		smbusst[bus].command = data;
}

uint32_t mcpx_smbus_device::smbus0_r(offs_t offset, uint32_t mem_mask)
{
	return smbus_read(0, offset, mem_mask);
}

void mcpx_smbus_device::smbus0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	smbus_write(0, offset, data, mem_mask);
}

uint32_t mcpx_smbus_device::smbus1_r(offs_t offset, uint32_t mem_mask)
{
	return smbus_read(1, offset, mem_mask);
}

void mcpx_smbus_device::smbus1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	smbus_write(1, offset, data, mem_mask);
}

/*
 * OHCI USB Controller
 */

DEFINE_DEVICE_TYPE(MCPX_OHCI, mcpx_ohci_device, "mcpx_ohci", "MCPX OHCI USB Controller")

void mcpx_ohci_device::config_map(address_map &map)
{
	pci_device::config_map(map);
	map(0x3e, 0x3e).r(FUNC(mcpx_ohci_device::minimum_grant_r));
	map(0x3f, 0x3f).r(FUNC(mcpx_ohci_device::maximum_latency_r));
}

void mcpx_ohci_device::ohci_mmio(address_map &map)
{
	map(0x00000000, 0x00000fff).rw(FUNC(mcpx_ohci_device::ohci_r), FUNC(mcpx_ohci_device::ohci_w));
}

mcpx_ohci_device::mcpx_ohci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint32_t subsystem_id)
	: mcpx_ohci_device(mconfig, tag, owner, clock)
{
	set_ids(0x10de01c2, 0xc3, 0x0c0310, subsystem_id);
}

mcpx_ohci_device::mcpx_ohci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, MCPX_OHCI, tag, owner, clock),
	ohci_usb(nullptr),
	m_interrupt_handler(*this),
	timer(nullptr),
	maincpu(*this, ":maincpu"),
	connecteds_count(0)
{
}

void mcpx_ohci_device::plug_usb_device(int port, device_usb_ohci_function_interface *function)
{
	function->set_bus_manager(ohci_usb);
	ohci_usb->usb_ohci_plug(port, function);
}

void mcpx_ohci_device::device_start()
{
	pci_device::device_start();
	m_interrupt_handler.resolve_safe();
	add_map(0x00001000, M_MEM, FUNC(mcpx_ohci_device::ohci_mmio));
	bank_infos[0].adr = 0xfed00000;
	status = 0x00b0;
	command = 0x0002;
	intr_pin = 1;
	ohci_usb = new ohci_usb_controller();
	ohci_usb->set_cpu(maincpu.target());
	ohci_usb->set_irq_callback(
		[&](int state)
		{
			m_interrupt_handler(state);
		}
	);
	timer = timer_alloc(FUNC(mcpx_ohci_device::usb_update), this);
	ohci_usb->set_timer(timer);
	ohci_usb->start();
	for (int i=0;i < connecteds_count;i++)
		plug_usb_device(connecteds[i].port, connecteds[i].dev);
}

void mcpx_ohci_device::device_reset()
{
	pci_device::device_reset();
	if (ohci_usb)
		ohci_usb->reset();
}

void mcpx_ohci_device::device_config_complete()
{
	char id[8];

	for (int i = 1; i<=4; i++)
	{
		sprintf(id, "port%d", i);
		ohci_usb_connector *conn = downcast<ohci_usb_connector *>(subdevice(id));
		if (conn)
		{
			device_usb_ohci_function_interface *func = conn->get_card_device();
			if (func)
			{
				connecteds[connecteds_count].dev = func;
				connecteds[connecteds_count].port = i;
				connecteds_count++;
			}
		}
	}
}

TIMER_CALLBACK_MEMBER(mcpx_ohci_device::usb_update)
{
	if (ohci_usb)
		ohci_usb->timer(param);
}

uint32_t mcpx_ohci_device::ohci_r(offs_t offset)
{
	if (!ohci_usb)
		return 0;
	if (offset == 0) // hacks needed until usb (and jvs) is implemented
	{
		hack_callback();
	}
	return ohci_usb->read(offset);
}

void mcpx_ohci_device::ohci_w(offs_t offset, uint32_t data)
{
	if (ohci_usb)
		ohci_usb->write(offset, data);
}

/*
 * Ethernet
 */

DEFINE_DEVICE_TYPE(MCPX_ETH, mcpx_eth_device, "mcpx_eth", "MCP Networking Adapter")

void mcpx_eth_device::eth_mmio(address_map &map)
{
	map(0x00000000, 0x0000003ff).rw(FUNC(mcpx_eth_device::eth_r), FUNC(mcpx_eth_device::eth_w));
}

void mcpx_eth_device::eth_io(address_map &map)
{
	map(0x00000000, 0x000000007).rw(FUNC(mcpx_eth_device::eth_io_r), FUNC(mcpx_eth_device::eth_io_w));
}

mcpx_eth_device::mcpx_eth_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, MCPX_ETH, tag, owner, clock)
{
	set_ids(0x10de01c3, 0, 0, 0);
}

void mcpx_eth_device::device_start()
{
	pci_device::device_start();
	add_map(0x00001000, M_MEM, FUNC(mcpx_eth_device::eth_mmio));
	bank_infos[0].adr = 0xfef00000;
	add_map(0x00000100, M_IO, FUNC(mcpx_eth_device::eth_io));
	bank_infos[1].adr = 0xe000;
}

void mcpx_eth_device::device_reset()
{
	pci_device::device_reset();
}

uint32_t mcpx_eth_device::eth_r()
{
	return 0;
}

void mcpx_eth_device::eth_w(uint32_t data)
{
}

uint32_t mcpx_eth_device::eth_io_r()
{
	return 0;
}

void mcpx_eth_device::eth_io_w(uint32_t data)
{
}

/*
 * Audio Processing Unit
 */

DEFINE_DEVICE_TYPE(MCPX_APU, mcpx_apu_device, "mcpx_apu", "MCP APU")

void mcpx_apu_device::config_map(address_map &map)
{
	pci_device::config_map(map);
	map(0x3e, 0x3e).r(FUNC(mcpx_apu_device::minimum_grant_r));
	map(0x3f, 0x3f).r(FUNC(mcpx_apu_device::maximum_latency_r));
}

void mcpx_apu_device::apu_mmio(address_map &map)
{
	map(0x00000000, 0x00007ffff).rw(FUNC(mcpx_apu_device::apu_r), FUNC(mcpx_apu_device::apu_w));
}

mcpx_apu_device::mcpx_apu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	pci_device(mconfig, MCPX_APU, tag, owner, clock),
	cpu(*this, finder_base::DUMMY_TAG)
{
}

void mcpx_apu_device::device_start()
{
	pci_device::device_start();
	add_map(0x00080000, M_MEM, FUNC(mcpx_apu_device::apu_mmio));
	bank_infos[0].adr = 0xfe800000;
	status = 0x00b0;
	command = 0x0002;
	intr_pin = 1;
	memset(apust.memory, 0, sizeof(apust.memory));
	memset(apust.voices_heap_blockaddr, 0, sizeof(apust.voices_heap_blockaddr));
	memset(apust.voices_active, 0, sizeof(apust.voices_active));
	memset(apust.voices_position, 0, sizeof(apust.voices_position));
	memset(apust.voices_position_start, 0, sizeof(apust.voices_position_start));
	memset(apust.voices_position_end, 0, sizeof(apust.voices_position_end));
	memset(apust.voices_position_increment, 0, sizeof(apust.voices_position_increment));
	apust.space = &cpu->space();
	apust.timer = timer_alloc(FUNC(mcpx_apu_device::audio_update), this);
	apust.timer->enable(false);
}

void mcpx_apu_device::device_reset()
{
	pci_device::device_reset();
}

TIMER_CALLBACK_MEMBER(mcpx_apu_device::audio_update)
{
	// this works only for outr2
	// value at 0x810 is modified by the firmware that has been loaded on the gp dsp
	int cmd = apust.space->read_dword(apust.gpdsp_address + 0x800 + 0x10);
	if (cmd == 3)
		apust.space->write_dword(apust.gpdsp_address + 0x800 + 0x10, 0);
	/*else
	logerror("Audio_APU: unexpected value at address %d\n",apust.gpdsp_address+0x800+0x10);*/
	// check all the 256 possible voices
	for (int b = 0; b < 4; b++) {
		uint64_t bv = 1;
		for (int bb = 0; bb < 64; bb++) {
			if (apust.voices_active[b] & bv) {
				int v = bb + (b << 6);
				apust.voices_position[v] += apust.voices_position_increment[v];
				while (apust.voices_position[v] >= apust.voices_position_end[v])
					apust.voices_position[v] = apust.voices_position_start[v] + apust.voices_position[v] - apust.voices_position_end[v] - 1000;
				uint32_t phys = apust.voicedata_address + 0x80 * v;
				apust.space->write_dword(phys + 0x58, apust.voices_position[v] / 1000);
			}
			bv = bv << 1;
		}
	}
}

uint32_t mcpx_apu_device::apu_r(offs_t offset, uint32_t mem_mask)
{
#ifdef LOG_AUDIO
	logerror("Audio_APU: read from %08X mask %08X\n", 0xfe800000 + offset * 4, mem_mask);
#endif
	if (offset == 0x20010 / 4) // some kind of internal counter or state value
		return 0x20 + 4 + 8 + 0x48 + 0x80;
	return apust.memory[offset];
}

void mcpx_apu_device::apu_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t v;

#ifdef LOG_AUDIO
	logerror("Audio_APU: write at %08X mask %08X value %08X\n", 0xfe800000 + offset * 4, mem_mask, data);
#endif
	apust.memory[offset] = data;
	if (offset == 0x02040 / 4) // address of memory area with scatter-gather info (gpdsp scratch dma)
		apust.gpdsp_sgaddress = data;
	if (offset == 0x020d4 / 4) { // block count (gpdsp)
		apust.gpdsp_sgblocks = data;
		apust.gpdsp_address = apust.space->read_dword(apust.gpdsp_sgaddress); // memory address of first block
		apust.timer->enable();
		apust.timer->adjust(attotime::from_msec(1), 0, attotime::from_msec(1));
	}
	if (offset == 0x02048 / 4) // (epdsp scratch dma)
		apust.epdsp_sgaddress = data;
	if (offset == 0x020dc / 4) // (epdsp)
		apust.epdsp_sgblocks = data;
	if (offset == 0x0204c / 4) // address of memory area with information about blocks
		apust.epdsp_sgaddress2 = data;
	if (offset == 0x020e0 / 4) // block count - 1
		apust.epdsp_sgblocks2 = data;
	if (offset == 0x0202c / 4) { // address of memory area with 0x80 bytes for each voice
		apust.voicedata_address = data;
		return;
	}
	if (offset == 0x04024 / 4) // offset in memory area indicated by 0x204c (analog output ?)
		return;
	if (offset == 0x04034 / 4) // size
		return;
	if (offset == 0x04028 / 4) // offset in memory area indicated by 0x204c (digital output ?)
		return;
	if (offset == 0x04038 / 4) // size
		return;
	if (offset == 0x20804 / 4) { // block number for scatter-gather heap that stores sampled audio to be played
		if (data >= 1024) {
			logerror("Audio_APU: sg block number too high, increase size of voices_heap_blockaddr\n");
			apust.memory[offset] = 1023;
		}
		return;
	}
	if (offset == 0x20808 / 4) { // block address for scatter-gather heap that stores sampled audio to be played
		apust.voices_heap_blockaddr[apust.memory[0x20804 / 4]] = data;
		return;
	}
	if (offset == 0x202f8 / 4) { // voice number for parameters ?
		apust.voice_number = data;
		return;
	}
	if (offset == 0x202fc / 4) // 1 when accessing voice parameters 0 otherwise
		return;
	if (offset == 0x20304 / 4) { // format
		 /*
		 bits 28-31 sample format:
		 0  8-bit pcm
		 5  16-bit pcm
		 10 adpcm ?
		 14 24-bit pcm
		 15 32-bit pcm
		 bits 16-20 number of channels - 1:
		 0  mono
		 1  stereo
		 */
		return;
	}
	if (offset == 0x2037c / 4) { // value related to sample rate
		int16_t v0 = (int16_t)(data >> 16); // upper 16 bits as a signed 16 bit value
		float vv = ((float)v0) / 4096.0f; // divide by 4096
		float vvv = powf(2, vv); // two to the vv
		int f = vvv*48000.0f; // sample rate
		apust.voices_frequency[apust.voice_number] = f;
		return;
	}
	if (offset == 0x203a0 / 4) // start offset of data in scatter-gather heap
		return;
	if (offset == 0x203a4 / 4) { // first sample to play
		apust.voices_position_start[apust.voice_number] = data * 1000;
		return;
	}
	if (offset == 0x203dc / 4) { // last sample to play
		apust.voices_position_end[apust.voice_number] = data * 1000;
		return;
	}
	if (offset == 0x2010c / 4) // voice processor 0 idle 1 not idle ?
		return;
	if (offset == 0x20124 / 4) { // voice number to activate ?
		v = apust.voice_number;
		apust.voices_active[v >> 6] |= ((uint64_t)1 << (v & 63));
		apust.voices_position[v] = apust.voices_position_start[apust.voice_number];
		apust.voices_position_increment[apust.voice_number] = apust.voices_frequency[apust.voice_number];
		return;
	}
	if (offset == 0x20128 / 4) { // voice number to deactivate ?
		v = apust.voice_number;
		apust.voices_active[v >> 6] &= ~(1 << (v & 63));
		return;
	}
	if (offset == 0x20140 / 4) // voice number to ?
		return;
	if ((offset >= 0x20200 / 4) && (offset < 0x20280 / 4)) // headroom for each of the 32 mixbins
		return;
	if (offset == 0x20280 / 4) // hrtf headroom ?
		return;
}

/*
 * AC97 Audio Controller
 */

DEFINE_DEVICE_TYPE(MCPX_AC97_AUDIO, mcpx_ac97_audio_device, "mcpx_ac97_audio", "MCPX AC'97 Audio Codec Interface")

void mcpx_ac97_audio_device::config_map(address_map &map)
{
	pci_device::config_map(map);
	map(0x3e, 0x3e).r(FUNC(mcpx_ac97_audio_device::minimum_grant_r));
	map(0x3f, 0x3f).r(FUNC(mcpx_ac97_audio_device::maximum_latency_r));
}

void mcpx_ac97_audio_device::ac97_mmio(address_map &map)
{
	map(0x00000000, 0x000000fff).rw(FUNC(mcpx_ac97_audio_device::ac97_audio_r), FUNC(mcpx_ac97_audio_device::ac97_audio_w));
}

void mcpx_ac97_audio_device::ac97_io0(address_map &map)
{
	map(0x00000000, 0x0000000ff).rw(FUNC(mcpx_ac97_audio_device::ac97_audio_io0_r), FUNC(mcpx_ac97_audio_device::ac97_audio_io0_w));
}

void mcpx_ac97_audio_device::ac97_io1(address_map &map)
{
	map(0x00000000, 0x00000007f).rw(FUNC(mcpx_ac97_audio_device::ac97_audio_io1_r), FUNC(mcpx_ac97_audio_device::ac97_audio_io1_w));
}

mcpx_ac97_audio_device::mcpx_ac97_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint32_t subsystem_id)
	: mcpx_ac97_audio_device(mconfig, tag, owner, clock)
{
	set_ids(0x10de01b1, 0xc2, 0x040100, subsystem_id);
}

mcpx_ac97_audio_device::mcpx_ac97_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, MCPX_AC97_AUDIO, tag, owner, clock)
{
}

void mcpx_ac97_audio_device::device_start()
{
	pci_device::device_start();
	set_multifunction_device(true);
	add_map(0x00000100, M_IO, FUNC(mcpx_ac97_audio_device::ac97_io0));
	bank_infos[0].adr = 0xd000;
	add_map(0x00000080, M_IO, FUNC(mcpx_ac97_audio_device::ac97_io1));
	bank_infos[1].adr = 0xd200;
	add_map(0x00001000, M_MEM, FUNC(mcpx_ac97_audio_device::ac97_mmio));
	bank_infos[2].adr = 0xfec00000;
	status = 0x00b0;
	command = 0x0003;
	intr_pin = 1;
	memset(&ac97st, 0, sizeof(ac97st));
}

void mcpx_ac97_audio_device::device_reset()
{
	pci_device::device_reset();
}

uint32_t mcpx_ac97_audio_device::ac97_audio_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t ret = 0;

#ifdef LOG_AUDIO
	logerror("Audio_AC3: read from %08X mask %08X\n", 0xfec00000 + offset * 4, mem_mask);
#endif
	if (offset < 0x80 / 4)
	{
		ret = ac97st.mixer_regs[offset];
	}
	if ((offset >= 0x100 / 4) && (offset <= 0x138 / 4))
	{
		offset = offset - 0x100 / 4;
		if (offset == 0x18 / 4)
		{
			ac97st.controller_regs[offset] &= ~0x02000000; // REGRST: register reset
		}
		if (offset == 0x30 / 4)
		{
			ac97st.controller_regs[offset] |= 0x100; // PCRDY: primary codec ready
		}
		if (offset == 0x34 / 4)
		{
			ac97st.controller_regs[offset] &= ~1; // CAS: codec access semaphore
		}
		ret = ac97st.controller_regs[offset];
	}
	return ret;
}

void mcpx_ac97_audio_device::ac97_audio_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
#ifdef LOG_AUDIO
	logerror("Audio_AC3: write at %08X mask %08X value %08X\n", 0xfec00000 + offset * 4, mem_mask, data);
#endif
	if (offset < 0x80 / 4)
	{
		COMBINE_DATA(ac97st.mixer_regs + offset);
	}
	if ((offset >= 0x100 / 4) && (offset < 0x13c / 4))
	{
		offset = offset - 0x100 / 4;
		COMBINE_DATA(ac97st.controller_regs + offset);
	}
}

uint32_t mcpx_ac97_audio_device::ac97_audio_io0_r()
{
	return 0;
}

void mcpx_ac97_audio_device::ac97_audio_io0_w(uint32_t data)
{
}

uint32_t mcpx_ac97_audio_device::ac97_audio_io1_r()
{
	return 0;
}

void mcpx_ac97_audio_device::ac97_audio_io1_w(uint32_t data)
{
}

/*
 * AC97 Modem Controller
 */

DEFINE_DEVICE_TYPE(MCPX_AC97_MODEM, mcpx_ac97_modem_device, "mcpx_ac97_modem", "MCPX AC'97 Modem Controller")

mcpx_ac97_modem_device::mcpx_ac97_modem_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, MCPX_AC97_MODEM, tag, owner, clock)
{
	set_ids(0x10de01c1, 0, 0, 0);
}

/*
 * IDE Controller
 */

DEFINE_DEVICE_TYPE(MCPX_IDE, mcpx_ide_device, "mcpx_ide", "MCPX IDE Controller")

void mcpx_ide_device::config_map(address_map &map)
{
	pci_device::config_map(map);
	map(0x08, 0x0b).rw(FUNC(pci_device::class_rev_r), FUNC(mcpx_ide_device::class_rev_w));
	map(0x3e, 0x3e).r(FUNC(mcpx_ide_device::minimum_grant_r));
	map(0x3f, 0x3f).r(FUNC(mcpx_ide_device::maximum_latency_r));
}

void mcpx_ide_device::ide_pri_command(address_map &map)
{
	map(0, 7).rw("ide1", FUNC(bus_master_ide_controller_device::cs0_r), FUNC(bus_master_ide_controller_device::cs0_w));
}

void mcpx_ide_device::ide_pri_control(address_map &map)
{
	// 3f6
	map(2, 2).rw(FUNC(mcpx_ide_device::pri_read_cs1_r), FUNC(mcpx_ide_device::pri_write_cs1_w));
}

void mcpx_ide_device::ide_sec_command(address_map &map)
{
	map(0, 7).rw("ide2", FUNC(bus_master_ide_controller_device::cs0_r), FUNC(bus_master_ide_controller_device::cs0_w));
}

void mcpx_ide_device::ide_sec_control(address_map &map)
{
	// 376
	map(2, 2).rw(FUNC(mcpx_ide_device::sec_read_cs1_r), FUNC(mcpx_ide_device::sec_write_cs1_w));
}

void mcpx_ide_device::ide_io(address_map &map)
{
	map(0x0000, 0x0007).rw("ide1", FUNC(bus_master_ide_controller_device::bmdma_r), FUNC(bus_master_ide_controller_device::bmdma_w));
	map(0x0008, 0x000f).rw("ide2", FUNC(bus_master_ide_controller_device::bmdma_r), FUNC(bus_master_ide_controller_device::bmdma_w));
}

mcpx_ide_device::mcpx_ide_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint32_t subsystem_id)
	: mcpx_ide_device(mconfig, tag, owner, clock)
{
	set_ids(0x10de01bc, 0xc3, 0x01018a, subsystem_id);
}

mcpx_ide_device::mcpx_ide_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, MCPX_IDE, tag, owner, clock),
	m_pri(*this, "ide1"),
	m_sec(*this, "ide2"),
	m_pri_interrupt_handler(*this),
	m_sec_interrupt_handler(*this)
{
}

void mcpx_ide_device::device_start()
{
	pci_device::device_start();
	add_map(0x00000008, M_IO | M_DISABLED, FUNC(mcpx_ide_device::ide_pri_command)); // primary command block
	add_map(0x00000004, M_IO | M_DISABLED, FUNC(mcpx_ide_device::ide_pri_control)); // primary control block
	add_map(0x00000008, M_IO | M_DISABLED, FUNC(mcpx_ide_device::ide_sec_command)); // secondary command block
	add_map(0x00000004, M_IO | M_DISABLED, FUNC(mcpx_ide_device::ide_sec_control)); // secondary control block
	add_map(0x00000010, M_IO, FUNC(mcpx_ide_device::ide_io));
	bank_infos[4].adr = 0xff60;
	status = 0x00b0;
	command = 0x0001;
	m_pri_interrupt_handler.resolve_safe();
	m_sec_interrupt_handler.resolve_safe();
}

void mcpx_ide_device::device_reset()
{
	pci_device::device_reset();
}

void mcpx_ide_device::device_add_mconfig(machine_config &config)
{
	bus_master_ide_controller_device &ide1(BUS_MASTER_IDE_CONTROLLER(config, "ide1", 0));
	ide1.irq_handler().set(FUNC(mcpx_ide_device::ide_pri_interrupt));
	ide1.set_bus_master_space(":maincpu", AS_PROGRAM);

	bus_master_ide_controller_device &ide2(BUS_MASTER_IDE_CONTROLLER(config, "ide2", 0));
	ide2.irq_handler().set(FUNC(mcpx_ide_device::ide_sec_interrupt));
	ide2.set_bus_master_space(":maincpu", AS_PROGRAM);
}

void mcpx_ide_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
	uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	// in compatibility mode the addresses are fixed, but the io enable bit in the command register is still used
	if (~pclass & 1) // compatibility mode
	{
		if (command & 1)
		{
			io_space->install_device(0x1f0, 0x1f7, *this, &mcpx_ide_device::ide_pri_command);
			io_space->install_device(0x3f4, 0x3f7, *this, &mcpx_ide_device::ide_pri_control);
		}
	}
	if (~pclass & 4)
	{
		if (command & 1)
		{
			io_space->install_device(0x170, 0x177, *this, &mcpx_ide_device::ide_sec_command);
			io_space->install_device(0x374, 0x377, *this, &mcpx_ide_device::ide_sec_control);
		}
	}
}

void mcpx_ide_device::class_rev_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_8_15)
	{
		uint32_t old = pclass;
		// bit 0 specifies if the primary channel is in compatibility or native-pci mode
		// bit 2 specifies if the secondary channel is in compatibility or native-pci mode
		pclass = (pclass & 0xfffffffa) | ((data >> 8) & 5);
		if (old ^ pclass)
		{
			if (~pclass & 1) // compatibility mode
			{
				bank_infos[0].flags |= M_DISABLED;
				bank_infos[1].flags |= M_DISABLED;
			}
			else
			{
				bank_infos[0].flags &= ~M_DISABLED;
				bank_infos[1].flags &= ~M_DISABLED;
			}
			if (~pclass & 4) // compatibility mode
			{
				bank_infos[2].flags |= M_DISABLED;
				bank_infos[3].flags |= M_DISABLED;
			}
			else
			{
				bank_infos[2].flags &= ~M_DISABLED;
				bank_infos[3].flags &= ~M_DISABLED;
			}
			remap_cb();
		}
	}
}

uint8_t mcpx_ide_device::pri_read_cs1_r()
{
	return m_pri->read_cs1(1, 0xff0000) >> 16;
}

void mcpx_ide_device::pri_write_cs1_w(uint8_t data)
{
	m_pri->write_cs1(1, data << 16, 0xff0000);
}

uint8_t mcpx_ide_device::sec_read_cs1_r()
{
	return m_sec->read_cs1(1, 0xff0000) >> 16;
}

void mcpx_ide_device::sec_write_cs1_w(uint8_t data)
{
	m_sec->write_cs1(1, data << 16, 0xff0000);
}

WRITE_LINE_MEMBER(mcpx_ide_device::ide_pri_interrupt)
{
	m_pri_interrupt_handler(state);
}

WRITE_LINE_MEMBER(mcpx_ide_device::ide_sec_interrupt)
{
	m_sec_interrupt_handler(state);
}

/*
 * AGP Bridge
 */

DEFINE_DEVICE_TYPE(NV2A_AGP, nv2a_agp_device, "nv2a_agp", "NV2A AGP Host to PCI Bridge")

void nv2a_agp_device::config_map(address_map& map)
{
	agp_bridge_device::config_map(map);
	map(0x40, 0xff).rw(FUNC(nv2a_agp_device::unknown_r), FUNC(nv2a_agp_device::unknown_w));
}

nv2a_agp_device::nv2a_agp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: agp_bridge_device(mconfig, NV2A_AGP, tag, owner, clock)
{
}

void nv2a_agp_device::device_start()
{
	agp_bridge_device::device_start();
}

void nv2a_agp_device::device_reset()
{
	agp_bridge_device::device_reset();
}

uint32_t nv2a_agp_device::unknown_r(offs_t offset, uint32_t mem_mask)
{
	// 4c 8 or 32
	// 44 8
	// 45 8
	// 46 8
	// 47 8
	//printf("R %08X %08X\n",0x40+offset*4,mem_mask);
	if (offset == 3)
		return 1;
	return 0;
}

void nv2a_agp_device::unknown_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	//printf("W %08X %08X %08X\n", 0x40+offset*4, mem_mask, data);
}

/*
 * NV2A 3D Accelerator
 */

DEFINE_DEVICE_TYPE(NV2A_GPU, nv2a_gpu_device, "nv2a_gpu", "NVIDIA NV2A GPU")

void nv2a_gpu_device::nv2a_mmio(address_map &map)
{
	map(0x00000000, 0x00ffffff).ram().rw(FUNC(nv2a_gpu_device::geforce_r), FUNC(nv2a_gpu_device::geforce_w));
}

void nv2a_gpu_device::nv2a_mirror(address_map &map)
{
	map(0x00000000, 0x07ffffff).ram().rw(FUNC(nv2a_gpu_device::nv2a_mirror_r), FUNC(nv2a_gpu_device::nv2a_mirror_w));
}

nv2a_gpu_device::nv2a_gpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	agp_device(mconfig, NV2A_GPU, tag, owner, clock),
	nvidia_nv2a(nullptr),
	cpu(*this, finder_base::DUMMY_TAG),
	m_interrupt_handler(*this),
	m_program(nullptr)
{
	set_ids(0x10de02a0, 0, 0, 0);
}

void nv2a_gpu_device::device_start()
{
	agp_device::device_start();
	m_interrupt_handler.resolve_safe();
	add_map(0x01000000, M_MEM, FUNC(nv2a_gpu_device::nv2a_mmio));
	bank_infos[0].adr = 0xfd000000;
	add_map(0x08000000, M_MEM, FUNC(nv2a_gpu_device::nv2a_mirror));
	bank_infos[1].adr = 0xf0000000;
	m_program = &cpu->space(AS_PROGRAM); // FIXME: isn't there a proper way to map stuff or do DMA via the PCI device interface?
	nvidia_nv2a = new nv2a_renderer(machine());
	nvidia_nv2a->set_irq_callbaclk(
		[&](int state)
		{
			m_interrupt_handler(state);
		}
	);
	nvidia_nv2a->start(m_program);
	nvidia_nv2a->savestate_items();
}

void nv2a_gpu_device::device_reset()
{
	agp_device::device_reset();
	nvidia_nv2a->set_ram_base(m_program->get_read_ptr(0));
}

uint32_t nv2a_gpu_device::geforce_r(offs_t offset, uint32_t mem_mask)
{
	return nvidia_nv2a->geforce_r(offset, mem_mask);
}

void nv2a_gpu_device::geforce_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask)
{
	nvidia_nv2a->geforce_w(space, offset, data, mem_mask);
}

uint32_t nv2a_gpu_device::nv2a_mirror_r(offs_t offset, uint32_t mem_mask)
{
	return m_program->read_dword(offset << 2);
}

void nv2a_gpu_device::nv2a_mirror_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	m_program->write_dword(offset << 2, data, mem_mask);
}
