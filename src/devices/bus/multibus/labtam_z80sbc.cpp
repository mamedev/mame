// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Labtam 3000 Z80 SBC card.
 *
 * Sources:
 *  - https://arvutimuuseum.ut.ee/index.php?m=eksponaadid&id=223
 *
 * TODO:
 *  - interrupts and dma
 *  - floppy
 *  - serial
 */
/*
 * Part             Type         Function
 * ----             ----         --------
 * MCM93422PC * 3   256x4 RAM    memory mapper (8 maps of 32, 12-bit entries)
 * M5K4164ANP * 16  64x1 DRAM    128KiB main memory
 * M58725P          2048x8 SRAM  resident bus RAM
 * WD2793A-PL02                  floppy disk formatter/controller
 * Z80A CPU
 * Z80A DMA * 2                  fdc and sio dma
 * Z80A SIO/2
 * AM9513PC                      system timing controller
 * MM58167AN                     real time clock
 * AM9519APC                     universal interrupt controller
 *
 * D8203-1                       DRAM controller
 * AM2946PC * 4
 * DP8304BN
 *
 * 25MHz
 * 20MHz
 * 8MHz
 */

#include "emu.h"
#include "labtam_z80sbc.h"

#define VERBOSE 0
#include "logmacro.h"

enum mapwr1_mask : u8
{
	MAPWR1_MA19 = 0x10, // multibus address bit 19
	MAPWR1_WP   = 0x20, // write protect
	MAPWR1_MEM  = 0x40, // memory access
	MAPWR1_RESB = 0x80, // resident bus select
};

DEFINE_DEVICE_TYPE(LABTAM_Z80SBC, labtam_z80sbc_device, "labtam_z80sbc", "Labtam Z80 SBC")

labtam_z80sbc_device::labtam_z80sbc_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, LABTAM_Z80SBC, tag, owner, clock)
	, device_multibus_interface(mconfig, *this)
	, m_cpu(*this, "cpu")
	, m_stc(*this, "stc")
	, m_uic(*this, "uic")
	, m_rtc(*this, "rtc")
	, m_fdc(*this, "fdc")
	, m_dma(*this, "dma%u", 0U)
	, m_sio(*this, "sio")
	, m_fdd(*this, "fdd%u", 0U)
	, m_eprom(*this, "eprom%u", 0U)
	, m_ram0_select(*this, "E15E")
	, m_ram1_select(*this, "E15D")
	, m_pio0_select(*this, "E15A")
	, m_pio1_select(*this, "E15B")
	, m_pio2_select(*this, "E15C")
	, m_installed(false)
{
}

ROM_START(labtam_z80sbc)
	ROM_REGION(0x2000, "eprom0", 0)
	ROM_LOAD("z80_boot__a85_0.u59", 0x0000, 0x2000, CRC(4453c938) SHA1(b04987a07ff7e21f7eb354162ad14b59d17096dd))

	ROM_REGION(0x2000, "eprom1", 0)
	ROM_LOAD("z80_boot__a85_1.u53", 0x0000, 0x2000, CRC(b7d489ea) SHA1(5bd6f3dd1c1f6f3e07706293bfc46a9bfc43d1f2))
ROM_END

static INPUT_PORTS_START(labtam_z80sbc)
	PORT_START("E15A")
	PORT_DIPNAME(0x00f0, 0x00f0, "I/O Low")
	PORT_DIPSETTING(0x0000, "0000h")
	PORT_DIPSETTING(0x0010, "0010h")
	PORT_DIPSETTING(0x0020, "0020h")
	PORT_DIPSETTING(0x0030, "0030h")
	PORT_DIPSETTING(0x0040, "0040h")
	PORT_DIPSETTING(0x0050, "0050h")
	PORT_DIPSETTING(0x0060, "0060h")
	PORT_DIPSETTING(0x0070, "0070h")
	PORT_DIPSETTING(0x0080, "0080h")
	PORT_DIPSETTING(0x0090, "0090h")
	PORT_DIPSETTING(0x00a0, "00a0h")
	PORT_DIPSETTING(0x00b0, "00b0h")
	PORT_DIPSETTING(0x00c0, "00c0h")
	PORT_DIPSETTING(0x00d0, "00d0h")
	PORT_DIPSETTING(0x00e0, "00e0h")
	PORT_DIPSETTING(0x00f0, "00f0h")

	PORT_START("E15B")
	PORT_DIPNAME(0x0f00, 0x0000, "I/O Middle")
	PORT_DIPSETTING(0x0000, "0000h")
	PORT_DIPSETTING(0x0100, "0100h")
	PORT_DIPSETTING(0x0200, "0200h")
	PORT_DIPSETTING(0x0300, "0300h")
	PORT_DIPSETTING(0x0400, "0400h")
	PORT_DIPSETTING(0x0500, "0500h")
	PORT_DIPSETTING(0x0600, "0600h")
	PORT_DIPSETTING(0x0700, "0700h")
	PORT_DIPSETTING(0x0800, "0800h")
	PORT_DIPSETTING(0x0900, "0900h")
	PORT_DIPSETTING(0x0a00, "0a00h")
	PORT_DIPSETTING(0x0b00, "0b00h")
	PORT_DIPSETTING(0x0c00, "0c00h")
	PORT_DIPSETTING(0x0d00, "0d00h")
	PORT_DIPSETTING(0x0e00, "0e00h")
	PORT_DIPSETTING(0x0f00, "0f00h")

	PORT_START("E15C")
	PORT_DIPNAME(0xf000, 0x0000, "I/O High")
	PORT_DIPSETTING(0x0000, "0000h")
	PORT_DIPSETTING(0x1000, "1000h")
	PORT_DIPSETTING(0x2000, "2000h")
	PORT_DIPSETTING(0x3000, "3000h")
	PORT_DIPSETTING(0x4000, "4000h")
	PORT_DIPSETTING(0x5000, "5000h")
	PORT_DIPSETTING(0x6000, "6000h")
	PORT_DIPSETTING(0x7000, "7000h")
	PORT_DIPSETTING(0x8000, "8000h")
	PORT_DIPSETTING(0x9000, "9000h")
	PORT_DIPSETTING(0xa000, "a000h")
	PORT_DIPSETTING(0xb000, "b000h")
	PORT_DIPSETTING(0xc000, "c000h")
	PORT_DIPSETTING(0xd000, "d000h")
	PORT_DIPSETTING(0xe000, "e000h")
	PORT_DIPSETTING(0xf000, "f000h")

	PORT_START("E15D")
	PORT_DIPNAME(0xf0000, 0xf0000, "Memory Bank 1")
	PORT_DIPSETTING(0x00000, "00000h")
	PORT_DIPSETTING(0x10000, "10000h")
	PORT_DIPSETTING(0x20000, "20000h")
	PORT_DIPSETTING(0x30000, "30000h")
	PORT_DIPSETTING(0x40000, "40000h")
	PORT_DIPSETTING(0x50000, "50000h")
	PORT_DIPSETTING(0x60000, "60000h")
	PORT_DIPSETTING(0x70000, "70000h")
	PORT_DIPSETTING(0x80000, "80000h")
	PORT_DIPSETTING(0x90000, "90000h")
	PORT_DIPSETTING(0xa0000, "a0000h")
	PORT_DIPSETTING(0xb0000, "b0000h")
	PORT_DIPSETTING(0xc0000, "c0000h")
	PORT_DIPSETTING(0xd0000, "d0000h")
	PORT_DIPSETTING(0xe0000, "e0000h")
	PORT_DIPSETTING(0xf0000, "f0000h")

	PORT_START("E15E")
	PORT_DIPNAME(0xf0000, 0xe0000, "Memory Bank 0")
	PORT_DIPSETTING(0x00000, "00000h")
	PORT_DIPSETTING(0x10000, "10000h")
	PORT_DIPSETTING(0x20000, "20000h")
	PORT_DIPSETTING(0x30000, "30000h")
	PORT_DIPSETTING(0x40000, "40000h")
	PORT_DIPSETTING(0x50000, "50000h")
	PORT_DIPSETTING(0x60000, "60000h")
	PORT_DIPSETTING(0x70000, "70000h")
	PORT_DIPSETTING(0x80000, "80000h")
	PORT_DIPSETTING(0x90000, "90000h")
	PORT_DIPSETTING(0xa0000, "a0000h")
	PORT_DIPSETTING(0xb0000, "b0000h")
	PORT_DIPSETTING(0xc0000, "c0000h")
	PORT_DIPSETTING(0xd0000, "d0000h")
	PORT_DIPSETTING(0xe0000, "e0000h")
	PORT_DIPSETTING(0xf0000, "f0000h")
INPUT_PORTS_END

const tiny_rom_entry *labtam_z80sbc_device::device_rom_region() const
{
	return ROM_NAME(labtam_z80sbc);
}

ioport_constructor labtam_z80sbc_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(labtam_z80sbc);
}

void labtam_z80sbc_device::device_start()
{
	m_ram0 = std::make_unique<u8[]>(0x10000);
	m_ram1 = std::make_unique<u8[]>(0x10000);
	m_sram = std::make_unique<u8[]>(0x800);

	m_map_lo = std::make_unique<u8[]>(256);
	m_map_hi = std::make_unique<u8[]>(256);

	save_pointer(NAME(m_ram0), 0x10000);
	save_pointer(NAME(m_ram1), 0x10000);
	save_pointer(NAME(m_sram), 0x800);

	save_pointer(NAME(m_map_lo), 256);
	save_pointer(NAME(m_map_hi), 256);

	save_item(NAME(m_map_enabled));
	save_item(NAME(m_map_num));
}

void labtam_z80sbc_device::device_reset()
{
	if (!m_installed)
	{
		u32 const ram0_select = m_ram0_select->read();
		u32 const ram1_select = m_ram1_select->read();

		m_bus->space(AS_PROGRAM).install_ram(ram0_select, ram0_select | 0xffff, m_ram0.get());
		m_bus->space(AS_PROGRAM).install_ram(ram1_select, ram1_select | 0xffff, m_ram0.get());

		u16 const pio_select = m_pio2_select->read() | m_pio1_select->read() | m_pio0_select->read();

		m_bus->space(AS_IO).install_write_handler(pio_select | 0, pio_select | 0, write8smo_delegate(*this, FUNC(labtam_z80sbc_device::fdcclr_w)));
		m_bus->space(AS_IO).install_write_handler(pio_select | 2, pio_select | 2, write8smo_delegate(*this, FUNC(labtam_z80sbc_device::netclr_w)));
		m_bus->space(AS_IO).install_write_handler(pio_select | 4, pio_select | 4, write8smo_delegate(*this, FUNC(labtam_z80sbc_device::fdcattn_w)));
		m_bus->space(AS_IO).install_read_handler(pio_select | 8, pio_select | 8, read8smo_delegate(*this, FUNC(labtam_z80sbc_device::fdcstatus_r)));

		// TODO: Multibus interrupt lines may optionally be wire-wrapped to
		// Am9517 interrupt request inputs 0, 1, 3 or 7.

		m_installed = true;
	}

	m_map_enabled = false;
	m_map_num = 0;

	// TODO: unsure what the bits represent
	m_drvstatus = 0xf0;
}

static void z80sbc_floppies(device_slot_interface &device)
{
	device.option_add("dssd5", FLOPPY_525_SD);
	device.option_add("dsdd5", FLOPPY_525_DD);
	device.option_add("dssd8", FLOPPY_8_DSSD);
	device.option_add("dsdd8", FLOPPY_8_DSDD);
}

void labtam_z80sbc_device::device_add_mconfig(machine_config &config)
{
	Z80(config, m_cpu, 20_MHz_XTAL / 4);
	m_cpu->set_addrmap(AS_PROGRAM, &labtam_z80sbc_device::cpu_mem);
	m_cpu->set_addrmap(AS_IO, &labtam_z80sbc_device::cpu_pio);
	m_cpu->set_irq_acknowledge_callback(m_uic, FUNC(am9519_device::iack_cb));

	AM9513(config, m_stc, 20_MHz_XTAL / 4);

	AM9519(config, m_uic);
	m_uic->out_int_callback().set_inputline(m_cpu, INPUT_LINE_IRQ0);

	MM58167(config, m_rtc, 32.768_kHz_XTAL);
	Z80DMA(config, m_dma[0], 0);
	Z80DMA(config, m_dma[1], 0);
	Z80SIO(config, m_sio, 0);

	WD2793(config, m_fdc, 2_MHz_XTAL);
	m_fdc->intrq_wr_callback().set(m_uic, FUNC(am9519_device::ireq3_w));
	//m_fdc->drq_wr_callback().set(m_dma[0], FUNC(z80dma_device::rdy_w));
	m_fdc->ready_wr_callback().set([this](int state) { if (!state) m_drvstatus |= 0xf; else m_drvstatus &= ~0xf; });

	FLOPPY_CONNECTOR(config, m_fdd[0], z80sbc_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_fdd[1], z80sbc_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_fdd[2], z80sbc_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_fdd[3], z80sbc_floppies, "dsdd8", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
}

void labtam_z80sbc_device::cpu_mem(address_map &map)
{
	/*
	 * Resident Bus Decoding
	 * 2000..27ff  sram
	 * 4000..5fff  eprom 1
	 * e000..ffff  eprom 0
	 */
	map(0x0000, 0xffff).rw(FUNC(labtam_z80sbc_device::map_r), FUNC(labtam_z80sbc_device::map_w));
}

void labtam_z80sbc_device::cpu_pio(address_map &map)
{
	//map(0x0000, 0x0000); // TODO: fdcset: set fdc interrupt
	//map(0x0008, 0x0008); // TODO: serset: set sio interrupt

	map(0x0010, 0x0010).select(0xff00).lw8([this](offs_t offset, u8 data) { m_map_lo[offset >> 8] = data; }, "mapwr0");
	map(0x0018, 0x0018).select(0xff00).lw8([this](offs_t offset, u8 data) { m_map_hi[offset >> 8] = data & 0xf0; }, "mapwr1");
	map(0x0020, 0x0020).mirror(0xff00).lw8([this](u8 data) { m_map_enabled = true; }, "intswt");
	map(0x0028, 0x0028).mirror(0xff00).lw8([this](u8 data) { m_map_enabled = true; m_map_num = data & 0x0f; }, "mapnum");

	map(0x0030, 0x0033).mirror(0xff00).lw8([this](offs_t offset, u8 data) { m_fdc->set_floppy(!data ? m_fdd[offset]->get_device() : nullptr); }, "driveN");
	//map(0x0038, 0x0038); // TODO: fltrest: reset drive fault
	map(0x0040, 0x0043).mirror(0xff00).rw(m_fdc, FUNC(wd2793_device::read), FUNC(wd2793_device::write));
	map(0x0048, 0x004b).mirror(0xff00).rw(m_sio, FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
	map(0x0050, 0x0051).mirror(0xff00).rw(m_stc, FUNC(am9513_device::read8), FUNC(am9513_device::write8));
	map(0x0058, 0x0058).mirror(0xff00).rw(m_uic, FUNC(am9519_device::data_r), FUNC(am9519_device::data_w));
	map(0x0059, 0x0059).mirror(0xff00).rw(m_uic, FUNC(am9519_device::stat_r), FUNC(am9519_device::cmd_w));

	//map(0x0060, 0x0067); // TODO: wd1001
	map(0x0068, 0x0068).mirror(0xff00).lr8([this]() { return m_drvstatus; }, "drvstatus");

	map(0x0070, 0x0070).select(0xff00).lr8([this](offs_t offset) { return m_map_lo[offset >> 8]; }, "maprd0");
	map(0x0078, 0x0078).select(0xff00).lr8([this](offs_t offset) { return m_map_hi[offset >> 8]; }, "maprd1");

	map(0x0080, 0x0080).mirror(0xff00).rw(m_dma[0], FUNC(z80dma_device::read), FUNC(z80dma_device::write));
	map(0x00a0, 0x00a0).mirror(0xff00).rw(m_dma[1], FUNC(z80dma_device::read), FUNC(z80dma_device::write));
	map(0x00e0, 0x00ff).mirror(0xff00).rw(m_rtc, FUNC(mm58167_device::read), FUNC(mm58167_device::write));
}

u8 labtam_z80sbc_device::map_r(offs_t offset)
{
	if (m_map_enabled)
	{
		u8 const entry = ((offset >> 8) & 0xf8) | (m_map_num & 0x07);
		u8 const lo = m_map_lo[entry];
		u8 const hi = m_map_hi[entry];

		u32 const address = u32(hi & MAPWR1_MA19) << 15 | u32(lo) << 11 | (offset & 0x7ff);

		if (hi & MAPWR1_RESB)
		{
			// TODO: use address space to decode resident bus
			switch (address & 0xf000)
			{
			case 0x2000:
				return m_sram[address & 0x7ff];
			case 0x4000:
			case 0x5000:
				return m_eprom[1][address & 0x1fff];
			case 0xe000:
			case 0xf000:
				return m_eprom[0][address & 0x1fff];
			default:
				LOG("map_r hi 0x%02x lo 0x%02x address 0x%05x\n", hi, lo, address);
				return 0;
			}
		}
		else
			// Theory: when mapper is deactivated, its outputs are all forced high, resulting
			// in RESB|MEM|WP flags and resident bus address |= 0xf800.
			return m_bus->space((hi & MAPWR1_MEM) ? AS_PROGRAM : AS_IO).read_byte(address);
	}
	else
		return m_eprom[0][0x1800 | (offset & 0x7ff)];
}

void labtam_z80sbc_device::map_w(offs_t offset, u8 data)
{
	if (m_map_enabled)
	{
		u8 const entry = ((offset >> 8) & 0xf8) | (m_map_num & 0x07);
		u8 const lo = m_map_lo[entry];
		u8 const hi = m_map_hi[entry];

		u32 const address = u32(hi & MAPWR1_MA19) << 15 | u32(lo) << 11 | (offset & 0x7ff);

		// TODO: suppress write-protected accesses

		if (hi & MAPWR1_RESB)
		{
			// TODO: use address space to decode resident bus
			switch (address & 0xf000)
			{
			case 0x2000:
				m_sram[address & 0x7ff] = data;
				break;
			default:
				LOG("map_w hi 0x%02x lo 0x%02x address 0x%05x data 0x%02x\n", hi, lo, address, data);
				break;
			}
		}
		else
			m_bus->space((hi & MAPWR1_MEM) ? AS_PROGRAM : AS_IO).write_byte(address, data);
	}
	else
		LOG("map_w map disabled offset 0x%04x data 0x%02x (%s)\n", offset, data, machine().describe_context());
}

void labtam_z80sbc_device::fdcclr_w(u8 data)
{
	LOG("fdcclr_w 0x%02x (%s)\n", data, machine().describe_context());
}

void labtam_z80sbc_device::netclr_w(u8 data)
{
	LOG("netclr_w 0x%02x (%s)\n", data, machine().describe_context());
}

void labtam_z80sbc_device::fdcattn_w(u8 data)
{
	LOG("fdcattn_w 0x%02x (%s)\n", data, machine().describe_context());
}

u8 labtam_z80sbc_device::fdcstatus_r()
{
	LOG("fdcstatus_r (%s)\n", machine().describe_context());
	return 0x3c;
}
