// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * 3Com EtherLink Plus (3C505) ethernet adapter. The documentation refers to 3
 * revisions of the hardware and firmware; this implementation and the firmware
 * are from a revision 3 board. The card was designed to work when installed in
 * both 8-bit and 16-bit ISA bus slots.
 *
 * Sources:
 *
 *   http://lxr.free-electrons.com/source/drivers/net/3c505.h
 *   http://lxr.free-electrons.com/source/drivers/net/3c505.c
 *   http://stason.org/TULARC/pc/network-cards/O/OLIVETTI-Ethernet-NPU-9144-3C505.html
 *   http://www.bitsavers.org/pdf/3Com/3c505_Etherlink_Plus_Developers_Guide_May86.pdf
 *   http://www.bitsavers.org/pdf/3Com/1569-03_EtherLink_Plus_Technical_Reference_Jan89.pdf
 *
 * TODO
 *   - resolve intermittent diagnostics bug on 8-bit dma channels
 *   - 8-bit isa slot support
 *   - revision 1.0 and 2.0 hardware/firmware variants
 *   - 8023 loopback mode
 */

#include "emu.h"
#include "3c505.h"

#define LOG_GENERAL (1U << 0)
#define LOG_REG     (1U << 1)
#define LOG_DATA    (1U << 2)

//#define VERBOSE (LOG_GENERAL|LOG_REG|LOG_DATA)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(ISA16_3C505, isa16_3c505_device, "3c505", "3Com EtherLink Plus")

ROM_START(3c505)
	ROM_REGION16_LE(0x04000, "system", 0)
	// this system firmware reports revision 0x0301 (3.1)
	ROM_LOAD16_BYTE("0729-12_a.3h", 0x00000, 0x02000, CRC(5415fccd) SHA1(6a42d7f3acdb3e0213e1037fee1864819ac33991))
	ROM_LOAD16_BYTE("0729-62_a.3f", 0x00001, 0x02000, CRC(4240bd9d) SHA1(015d2f7282def85681bcf1a7c5a7f501a16d5a6c))

	ROM_SYSTEM_BIOS(0, "unused", "Unused")
	ROM_SYSTEM_BIOS(1, "apollo", "Apollo")
	ROM_SYSTEM_BIOS(2, "netware", "3C505-NW EtherLink Plus NetWare Boot PROM")

	// host firmware
	ROM_REGION(0x02000, "host", 0)
	ROMX_LOAD("3000_3c505_010728-00.bin", 0x00000, 0x02000, CRC(69b77ec6) SHA1(7ac36cc6fc90b90ddfc56c45303b514cbe18ae58), ROM_BIOS(1))
	ROMX_LOAD("3c505-nw.bin", 0x00000, 0x02000, NO_DUMP, ROM_BIOS(2))

	// station address prom
	ROM_REGION16_LE(0x10, "mac", 0)
	ROM_LOAD("3com.9h", 0x00, 0x10, CRC(8e207354) SHA1(ca5ddcb272ab2851e00473dc960b23039746e3d2))
	ROMX_LOAD("apollo.9h", 0x00, 0x10, CRC(490f283e) SHA1(fe4c26b6a41e643f4397990b066fdc04b6f4c5ae), ROM_BIOS(1))
ROM_END

static INPUT_PORTS_START(3c505)
	PORT_START("IO_BASE")
	PORT_DIPNAME(0x3f0, 0x300, "I/O Base")
	PORT_DIPSETTING(    0x010, "010h")
	PORT_DIPSETTING(    0x020, "020h")
	PORT_DIPSETTING(    0x030, "030h")
	PORT_DIPSETTING(    0x040, "040h")
	PORT_DIPSETTING(    0x050, "050h")
	PORT_DIPSETTING(    0x060, "060h")
	PORT_DIPSETTING(    0x070, "070h")
	PORT_DIPSETTING(    0x080, "080h")
	PORT_DIPSETTING(    0x090, "090h")
	PORT_DIPSETTING(    0x0a0, "0a0h")
	PORT_DIPSETTING(    0x0b0, "0b0h")
	PORT_DIPSETTING(    0x0c0, "0c0h")
	PORT_DIPSETTING(    0x0d0, "0d0h")
	PORT_DIPSETTING(    0x0e0, "0e0h")
	PORT_DIPSETTING(    0x0f0, "0f0h")
	PORT_DIPSETTING(    0x100, "0100h")
	PORT_DIPSETTING(    0x110, "0110h")
	PORT_DIPSETTING(    0x120, "0120h")
	PORT_DIPSETTING(    0x130, "0130h")
	PORT_DIPSETTING(    0x140, "0140h")
	PORT_DIPSETTING(    0x150, "0150h")
	PORT_DIPSETTING(    0x160, "0160h")
	PORT_DIPSETTING(    0x170, "0170h")
	PORT_DIPSETTING(    0x180, "0180h")
	PORT_DIPSETTING(    0x190, "0190h")
	PORT_DIPSETTING(    0x1a0, "01a0h")
	PORT_DIPSETTING(    0x1b0, "01b0h")
	PORT_DIPSETTING(    0x1c0, "01c0h")
	PORT_DIPSETTING(    0x1d0, "01d0h")
	PORT_DIPSETTING(    0x1e0, "01e0h")
	PORT_DIPSETTING(    0x1f0, "01f0h")
	PORT_DIPSETTING(    0x200, "0200h")
	PORT_DIPSETTING(    0x210, "0210h")
	PORT_DIPSETTING(    0x220, "0220h")
	PORT_DIPSETTING(    0x230, "0230h")
	PORT_DIPSETTING(    0x240, "0240h")
	PORT_DIPSETTING(    0x250, "0250h")
	PORT_DIPSETTING(    0x260, "0260h")
	PORT_DIPSETTING(    0x270, "0270h")
	PORT_DIPSETTING(    0x280, "0280h")
	PORT_DIPSETTING(    0x290, "0290h")
	PORT_DIPSETTING(    0x2a0, "02a0h")
	PORT_DIPSETTING(    0x2b0, "02b0h")
	PORT_DIPSETTING(    0x2c0, "02c0h")
	PORT_DIPSETTING(    0x2d0, "02d0h")
	PORT_DIPSETTING(    0x2e0, "02e0h")
	PORT_DIPSETTING(    0x2f0, "02f0h")
	PORT_DIPSETTING(    0x300, "0300h")
	PORT_DIPSETTING(    0x310, "0310h")
	PORT_DIPSETTING(    0x320, "0320h")
	PORT_DIPSETTING(    0x330, "0330h")
	PORT_DIPSETTING(    0x340, "0340h")
	PORT_DIPSETTING(    0x350, "0350h")
	PORT_DIPSETTING(    0x360, "0360h")
	PORT_DIPSETTING(    0x370, "0370h")
	PORT_DIPSETTING(    0x380, "0380h")
	PORT_DIPSETTING(    0x390, "0390h")
	PORT_DIPSETTING(    0x3a0, "03a0h")
	PORT_DIPSETTING(    0x3b0, "03b0h")
	PORT_DIPSETTING(    0x3c0, "03c0h")
	PORT_DIPSETTING(    0x3d0, "03d0h")
	PORT_DIPSETTING(    0x3e0, "03e0h")
	PORT_DIPSETTING(    0x3f0, "03f0h")

	PORT_START("IRQ_DRQ")
	PORT_DIPNAME(0x0f, 0x0a, "IRQ")
	// 8 or 16 bit slots
	PORT_DIPSETTING(   0x03, "IRQ 3")
	PORT_DIPSETTING(   0x04, "IRQ 4")
	PORT_DIPSETTING(   0x05, "IRQ 5")
	PORT_DIPSETTING(   0x06, "IRQ 6")
	PORT_DIPSETTING(   0x07, "IRQ 7")
	PORT_DIPSETTING(   0x09, "IRQ 9")
	// 16 bit slots only
	PORT_DIPSETTING(   0x0a, "IRQ 10")
	PORT_DIPSETTING(   0x0b, "IRQ 11")
	PORT_DIPSETTING(   0x0c, "IRQ 12")
	PORT_DIPSETTING(   0x0e, "IRQ 14")
	PORT_DIPSETTING(   0x0f, "IRQ 15")

	// TODO: uses two jumpers for each channel: dma mode selection?
	PORT_DIPNAME(0x70, 0x50, "DRQ")
	PORT_DIPSETTING(   0x00, "none")
	// 8 or 16 bit slots
	PORT_DIPSETTING(   0x10, "DRQ 1")
	PORT_DIPSETTING(   0x30, "DRQ 3")
	// 16 bit slots only
	PORT_DIPSETTING(   0x50, "DRQ 5")
	PORT_DIPSETTING(   0x60, "DRQ 6")
	PORT_DIPSETTING(   0x70, "DRQ 7")

	// 8-position jumper block marked EN,13-19 decodes address lines
	PORT_START("ROM_OPTS")
	PORT_DIPNAME(0x01, 0x00, "ROM Enable")
	PORT_DIPSETTING(   0x00, DEF_STR(Off))
	PORT_DIPSETTING(   0x01, DEF_STR(On))

	PORT_DIPNAME(0xfe, 0x00, "ROM Base")
	// Apollo host ROM addresses
	PORT_DIPSETTING(   0x00, "00000h")
	PORT_DIPSETTING(   0x02, "02000h")
	PORT_DIPSETTING(   0x04, "04000h")
	PORT_DIPSETTING(   0x06, "06000h")

	// conventional PC option ROM addresses
	PORT_DIPSETTING(   0xc8, "C8000h")
	PORT_DIPSETTING(   0xca, "CA000h")
	PORT_DIPSETTING(   0xcc, "CC000h")
	PORT_DIPSETTING(   0xce, "CE000h")
	PORT_DIPSETTING(   0xd0, "D0000h")
	PORT_DIPSETTING(   0xd2, "D2000h")
	PORT_DIPSETTING(   0xd4, "D4000h")
	PORT_DIPSETTING(   0xd6, "D6000h")
	PORT_DIPSETTING(   0xd8, "D8000h")
	PORT_DIPSETTING(   0xda, "DA000h")
	PORT_DIPSETTING(   0xdc, "DC000h")
	PORT_DIPSETTING(   0xde, "DE000h")
	PORT_DIPSETTING(   0xe0, "E0000h")
	PORT_DIPSETTING(   0xe2, "E2000h")
	PORT_DIPSETTING(   0xe4, "E4000h")
	PORT_DIPSETTING(   0xe6, "E6000h")
	PORT_DIPSETTING(   0xe8, "E8000h")
	PORT_DIPSETTING(   0xea, "EA000h")
	PORT_DIPSETTING(   0xec, "EC000h")
	PORT_DIPSETTING(   0xee, "EE000h")
	PORT_DIPSETTING(   0xf0, "F0000h")
	PORT_DIPSETTING(   0xf2, "F2000h")
	PORT_DIPSETTING(   0xf4, "F4000h")

	PORT_START("TEST")
	PORT_DIPNAME(0x01, 0x00, "TEST Mode")
	PORT_DIPSETTING(   0x00, DEF_STR(Off))
	PORT_DIPSETTING(   0x01, DEF_STR(On))
INPUT_PORTS_END

isa16_3c505_device::isa16_3c505_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ISA16_3C505, tag, owner, clock)
	, device_isa16_card_interface(mconfig, *this)
	, m_cpu(*this, "cpu")
	, m_net(*this, "net")
	, m_ram(*this, "ram")
	, m_led(*this, "led%u", 0U)
	, m_iobase(*this, "IO_BASE")
	, m_irqdrq(*this, "IRQ_DRQ")
	, m_romopts(*this, "ROM_OPTS")
	, m_test(*this, "TEST")
	, m_installed(false)
{
}

const tiny_rom_entry *isa16_3c505_device::device_rom_region() const
{
	return ROM_NAME(3c505);
}

void isa16_3c505_device::device_add_mconfig(machine_config &config)
{
	I80186(config, m_cpu, 16_MHz_XTAL);
	m_cpu->set_addrmap(AS_PROGRAM, &isa16_3c505_device::map_main);
	m_cpu->set_addrmap(AS_IO, &isa16_3c505_device::map_io);

	I82586(config, m_net, 8_MHz_XTAL);
	m_net->set_addrmap(0, &isa16_3c505_device::map_main);
	m_net->out_irq_cb().set(m_cpu, FUNC(i80186_cpu_device::int1_w));

	// 1986 document indicates 128KiB is default, but 1988 document does not
	// include it as an option
	RAM(config, m_ram);
	m_ram->set_default_size("256KiB");
	m_ram->set_extra_options("128KiB,256KiB,384KiB,512KiB");
}

ioport_constructor isa16_3c505_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(3c505);
}

void isa16_3c505_device::device_start()
{
	set_isa_device();

	// install ram in both i80186 and i82586 address spaces
	if (!m_ram->started())
		throw device_missing_dependencies();

	m_cpu->space(0).install_ram(0x00000, m_ram->mask() & 0xfffff, m_ram->pointer());
	m_net->space(0).install_ram(0x00000, m_ram->mask() & 0xfffff, m_ram->pointer());

	m_led.resolve();

	save_item(NAME(m_acmdr));
	save_item(NAME(m_acr));
	save_item(NAME(m_asr));

	save_item(NAME(m_hcmdr));
	save_item(NAME(m_hcr));
	save_item(NAME(m_hsr));
	save_item(NAME(m_hdr));

	//save_item(NAME(m_data);

	save_item(NAME(m_cpu_drq_asserted));
	save_item(NAME(m_cpu_irq_asserted));
	save_item(NAME(m_isa_drq_asserted));
	save_item(NAME(m_isa_irq_asserted));
}

void isa16_3c505_device::device_reset()
{
	if (!m_installed)
	{
		u16 const base = m_iobase->read();
		m_isa->install_device(base, base | 0xf, *this, &isa16_3c505_device::map_isa);

		m_isa_irq = m_irqdrq->read() & 0xf;
		m_isa_drq = (m_irqdrq->read() >> 4) & 0x7;

		if (m_romopts->read() & 1)
		{
			offs_t const rom_base = (m_romopts->read() & 0xfe) << 12;

			if (m_isa->is_option_rom_space_available(rom_base, 0x2000))
				m_isa->install_rom(this, rom_base, rom_base | 0x01fff, "host", "host");
		}

		m_isa->set_dma_channel(m_isa_drq, this, true);

		m_installed = true;
	}

	m_cpu->reset();

	// adapter registers
	m_acmdr = 0;
	m_acr = 0;
	m_asr = ASR_ACRE | ASR_8_16;
	if (m_test->read())
		m_asr |= ASR_SWTC;

	// host registers
	m_hcmdr = 0;
	m_hcr = 0;
	m_hsr = HSR_HCRE;
	m_hdr = 0;

	m_data.clear();

	m_cpu_drq_asserted = false;
	m_cpu_irq_asserted = false;
	m_isa_drq_asserted = false;
	m_isa_irq_asserted = false;

	update_rdy(m_acr, m_hcr);
}

void isa16_3c505_device::map_main(address_map &map)
{
	// i82586 upper 4 address lines are ignored
	map.global_mask(0x0fffff);

	map(0xfc000, 0xfffff).rom().region("system", 0);
}

void isa16_3c505_device::map_io(address_map &map)
{
	/*
	 * A read or write to I/O location 00H will cause an active transition on
	 * the CA input.
	 */
	map(0x0000, 0x0000).lrw8("ca",
		[this]()
		{
			m_net->ca(1);
			m_net->ca(0);

			return 0;
		},
		[this](u8 data)
		{
			m_net->ca(1);
			m_net->ca(0);
		});

	/*
	 * A read or write to I/O location 80H will produce a CAS before RAS cycle
	 * in all banks simultaneously.
	 */
	map(0x0080, 0x0081).noprw();

	map(0x0100, 0x0100).rw(FUNC(isa16_3c505_device::acmd_r), FUNC(isa16_3c505_device::acmd_w));
	map(0x0102, 0x0102).r(FUNC(isa16_3c505_device::acr_r));
	map(0x0103, 0x0103).rw(FUNC(isa16_3c505_device::asr_r), FUNC(isa16_3c505_device::acr_w));
	map(0x0104, 0x0105).rw(FUNC(isa16_3c505_device::adata_r), FUNC(isa16_3c505_device::adata_w));

	map(0x0180, 0x018f).rom().region("mac", 0);
}

void isa16_3c505_device::map_isa(address_map &map)
{
	map(0, 0).rw(FUNC(isa16_3c505_device::hcmd_r), FUNC(isa16_3c505_device::hcmd_w));
	map(2, 2).rw(FUNC(isa16_3c505_device::hsr_r), FUNC(isa16_3c505_device::hdr_w));
	map(4, 5).rw(FUNC(isa16_3c505_device::hdata_r), FUNC(isa16_3c505_device::hdata_w));
	map(6, 6).rw(FUNC(isa16_3c505_device::hcr_r), FUNC(isa16_3c505_device::hcr_w));
}

u8 isa16_3c505_device::acmd_r()
{
	u8 const data = m_hcmdr;

	m_asr &= ~ASR_HCRF;
	m_hsr |= HSR_HCRE;

	update_cpu_irq(0);

	return data;
}

void isa16_3c505_device::acmd_w(u8 data)
{
	LOGMASKED(LOG_REG, "acmd_w 0x%02x (%s)\n", data, machine().describe_context());

	m_asr &= ~ASR_ACRE;
	m_hsr |= HSR_ACRF;

	m_acmdr = data;

	if (m_hcr & HCR_CMDE)
		update_isa_irq(1);
}

void isa16_3c505_device::acr_w(u8 data)
{
	LOGMASKED(LOG_REG, "acr_w 0x%02x (%s)\n", data, machine().describe_context());

	// update adapter status flags
	if ((data ^ m_acr) & ACR_ASF)
		m_hsr = (m_hsr & ~HSR_ASF) | (data & ACR_ASF);

	if ((data ^ m_acr) & ACR_LED1)
		m_led[0] = !!(data & ACR_LED1);

	if ((data ^ m_acr) & ACR_LED2)
		m_led[1] = !!(data & ACR_LED2);

	m_net->reset_w((data & ACR_R586) ? 1 : 0);

	if ((data ^ m_acr) & ACR_FLSH)
	{
		if (data & ACR_FLSH)
		{
			LOGMASKED(LOG_REG, "adapter flushed data fifo (%d bytes)\n", m_data.queue_length());
			m_data.clear();
		}

		update_rdy(data, m_hcr);
	}

	// loopback is active low
	if ((m_acr & ACR_LPBK) && !(data & ACR_LPBK))
	{
		LOGMASKED(LOG_REG, "loopback enabled\n");

		// TODO: enable loopback on 8023
	}

	m_acr = data;
}

u16 isa16_3c505_device::adata_r()
{
	if (!(m_asr & ASR_DIR) && m_data.queue_length() > 1)
	{
		u16 data = m_data.dequeue();
		data |= u16(m_data.dequeue()) << 8;

		LOGMASKED(LOG_DATA, "adata_r 0x%04x\n", data);

		update_rdy(m_acr, m_hcr);

		return data;
	}
	else
		fatalerror("%s: adata_r read fifo while %s (%s)\n",
			tag(), (m_asr & ASR_DIR) ? "write-only" : "empty", machine().describe_context().c_str());
}

void isa16_3c505_device::adata_w(u16 data)
{
	if ((m_asr & ASR_DIR) && m_data.queue_length() < (FIFO_SIZE - 1))
	{
		LOGMASKED(LOG_DATA, "adata_w 0x%04x\n", data);
		m_data.enqueue(u8(data));
		m_data.enqueue(data >> 8);

		update_rdy(m_acr, m_hcr);
	}
	else
		fatalerror("%s: adata_w write fifo while %s (%s)\n",
			tag(), !(m_asr & ASR_DIR) ? "read-only" : "full", machine().describe_context().c_str());
}

u8 isa16_3c505_device::hcmd_r()
{
	u8 const data = m_acmdr;

	m_asr |= ASR_ACRE;
	m_hsr &= ~HSR_ACRF;

	update_isa_irq(0);

	return data;
}

void isa16_3c505_device::hcmd_w(u8 data)
{
	LOGMASKED(LOG_REG, "hcmd_w 0x%02x (%s)\n", data, machine().describe_context());

	m_asr |= ASR_HCRF;
	m_hsr &= ~HSR_HCRE;

	m_hcmdr = data;

	update_cpu_irq(1);
}

void isa16_3c505_device::hcr_w(u8 data)
{
	LOGMASKED(LOG_REG, "hcr_w 0x%02x (%s)\n", data, machine().describe_context());

	// update host status flags
	if ((data ^ m_hcr) & HCR_HSF)
		m_asr = (m_asr & ~ASR_HSF) | (data & HCR_HSF);

	// update direction flag
	if ((data ^ m_hcr) & HCR_DIR)
	{
		if (data & HCR_DIR)
		{
			LOGMASKED(LOG_REG, "transfer from adapter to host\n");

			// transfer from adapter to host
			m_hsr |= HSR_DIR;
			m_asr |= ASR_DIR;

			update_rdy(m_acr, data);
		}
		else
		{
			LOGMASKED(LOG_REG, "transfer from host to adapter\n");

			// transfer from host to adapter
			m_hsr &= ~HSR_DIR;
			m_asr &= ~ASR_DIR;

			update_rdy(m_acr, data);
		}
	}

	if (!(data & HCR_DMAE))
		m_hsr &= ~HSR_DONE;

	if ((data ^ m_hcr) & HCR_FLSH)
	{
		if (data & HCR_FLSH)
		{
			LOGMASKED(LOG_REG, "host flushed data fifo (%d bytes)\n", m_data.queue_length());
			m_data.clear();
		}

		update_rdy(m_acr, data);
	}

	// attention condition
	if (!(m_hcr & HCR_ATTN) && (data & HCR_ATTN))
	{
		if (!(data & HCR_FLSH))
		{
			LOGMASKED(LOG_REG, "soft reset\n");

			// soft reset
			m_cpu->set_input_line(INPUT_LINE_NMI, 1);
			m_cpu->set_input_line(INPUT_LINE_NMI, 0);
		}
		else
		{
			LOGMASKED(LOG_REG, "hard reset\n");

			// hard reset
			reset();
		}
	}

	m_hcr = data;
}

u16 isa16_3c505_device::hdata_r(offs_t offset, u16 mem_mask)
{
	unsigned const word = (mem_mask == 0xffff);
	u16 data = 0;

	if ((m_hsr & HSR_DIR) && (m_data.queue_length() > word))
	{
		if (ACCESSING_BITS_0_7)
			data |= m_data.dequeue();

		if (ACCESSING_BITS_8_15)
			data |= u16(m_data.dequeue()) << 8;

		LOGMASKED(LOG_DATA, "hdata_r 0x%04x mem_mask 0x%04x (%s)\n", data, mem_mask, machine().describe_context());

		update_rdy(m_acr, m_hcr);
	}
	else
		logerror("hdata_r read fifo while %s (%s)\n",
			!(m_hsr & HSR_DIR) ? "write-only" : "empty", machine().describe_context());

	return data;
}

void isa16_3c505_device::hdata_w(offs_t offset, u16 data, u16 mem_mask)
{
	unsigned const word = (mem_mask == 0xffff);

	if (!(m_hsr & HSR_DIR) && (m_data.queue_length() < (FIFO_SIZE - word)))
	{
		if (ACCESSING_BITS_0_7)
			m_data.enqueue(u8(data));

		if (ACCESSING_BITS_8_15)
			m_data.enqueue(data >> 8);

		LOGMASKED(LOG_DATA, "hdata_w 0x%04x mem_mask 0x%04x (%s)\n", data, mem_mask, machine().describe_context());

		update_rdy(m_acr, m_hcr);
	}
	else
		logerror("hdata_w write fifo while %s (%s)\n",
			(m_hsr & HSR_DIR) ? "read-only" : "full", machine().describe_context());
}

void isa16_3c505_device::update_rdy(u8 const acr, u8 const hcr)
{
	if (!(acr & ACR_FLSH) && !(hcr & HCR_FLSH))
	{
		if (hcr & HCR_DIR)
		{
			// adapter to host
			if (m_data.empty())
				m_hsr &= ~HSR_HRDY;
			else
				m_hsr |= HSR_HRDY;

			if (m_data.queue_length() > (FIFO_SIZE - 2))
				m_asr &= ~ASR_ARDY;
			else
				m_asr |= ASR_ARDY;
		}
		else
		{
			// host to adapter
			if (m_data.queue_length() < 2)
				m_asr &= ~ASR_ARDY;
			else
				m_asr |= ASR_ARDY;

			if (m_data.full())
				m_hsr &= ~HSR_HRDY;
			else
				m_hsr |= HSR_HRDY;
		}
	}
	else
	{
		m_asr &= ~ASR_ARDY;
		m_hsr &= ~HSR_HRDY;
	}

	update_cpu_drq(!!(m_asr & ASR_ARDY));
	update_isa_drq((m_hsr & HSR_HRDY) && (hcr & HCR_DMAE));
}

void isa16_3c505_device::update_cpu_drq(int state)
{
	if (bool(state) != m_cpu_drq_asserted)
	{
		m_cpu_drq_asserted = bool(state);
		m_cpu->drq1_w(state);
	}
}

void isa16_3c505_device::update_cpu_irq(int state)
{
	if (bool(state) != m_cpu_irq_asserted)
	{
		m_cpu_irq_asserted = bool(state);
		m_cpu->int0_w(state);
	}
}

void isa16_3c505_device::update_isa_drq(int state)
{
	if (bool(state) != m_isa_drq_asserted)
	{
		LOG("update_isa_drq %d\n", state);

		switch (m_isa_drq)
		{
		case 1: m_isa->drq1_w(state); break;
		case 3: m_isa->drq3_w(state); break;
		case 5: m_isa->drq5_w(state); break;
		case 6: m_isa->drq6_w(state); break;
		case 7: m_isa->drq7_w(state); break;

		default:
			fatalerror("%s: invalid isa drq %d\n", tag(), m_isa_drq);
		}

		m_isa_drq_asserted = bool(state);
	}
}

void isa16_3c505_device::update_isa_irq(int state)
{
	if (bool(state) != m_isa_irq_asserted)
	{
		LOG("update_isa_irq %d\n", state);

		switch (m_isa_irq)
		{
		case 3: m_isa->irq3_w(state); break;
		case 4: m_isa->irq4_w(state); break;
		case 5: m_isa->irq5_w(state); break;
		case 6: m_isa->irq6_w(state); break;
		case 7: m_isa->irq7_w(state); break;
		case 9: m_isa->irq2_w(state); break;
		case 10: m_isa->irq10_w(state); break;
		case 11: m_isa->irq11_w(state); break;
		case 12: m_isa->irq12_w(state); break;
		case 14: m_isa->irq14_w(state); break;
		case 15: m_isa->irq15_w(state); break;

		default:
			fatalerror("%s: invalid isa irq %d\n", tag(), m_isa_irq);
		}

		m_isa_irq_asserted = bool(state);
	}
}

void isa16_3c505_device::eop_w(int state)
{
	LOG("eop_w %d fifo %d\n", state, m_data.queue_length());

	if (state)
	{
		m_hsr |= HSR_DONE;

		update_isa_drq(0);

		if (m_hcr & HCR_TCEN)
			update_isa_irq(1);
	}
}
