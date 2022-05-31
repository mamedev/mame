// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Labtam 3000 Z80 SBC card.
 *
 * Sources:
 *  - https://arvutimuuseum.ut.ee/index.php?m=eksponaadid&id=223
 *
 * TODO:
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

enum drvstatus_mask : u8
{
	DRVSTATUS_A  = 0x01, // FD0 is mini floppy
	DRVSTATUS_C  = 0x02, // FD1 is mini floppy
	DRVSTATUS_E  = 0x04, // FD2 is mini floppy
	DRVSTATUS_G  = 0x08, // FD3 is mini floppy
	DRVSTATUS_B  = 0x10, // not used
	DRVSTATUS_D  = 0x20, // not used
	DRVSTATUS_SS = 0x40, // 8" floppy is single-sided
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
	, m_e15(*this, "E15%c", 'A')
	, m_e21(*this, "E21")
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

	PORT_START("E21")
	PORT_DIPNAME(0x01, 0x01, "FD0 is 5.25\"") PORT_DIPLOCATION("E21:1")
	PORT_DIPSETTING(0x00, DEF_STR(Yes))
	PORT_DIPSETTING(0x01, DEF_STR(No))
	PORT_DIPNAME(0x02, 0x00, "FD1 is 5.25\"") PORT_DIPLOCATION("E21:3")
	PORT_DIPSETTING(0x00, DEF_STR(Yes))
	PORT_DIPSETTING(0x02, DEF_STR(No))
	PORT_DIPNAME(0x04, 0x00, "FD2 is 5.25\"") PORT_DIPLOCATION("E21:5")
	PORT_DIPSETTING(0x00, DEF_STR(Yes))
	PORT_DIPSETTING(0x04, DEF_STR(No))
	PORT_DIPNAME(0x08, 0x08, "FD3 is 5.25\"") PORT_DIPLOCATION("E21:7")
	PORT_DIPSETTING(0x00, DEF_STR(Yes))
	PORT_DIPSETTING(0x08, DEF_STR(No))
	PORT_DIPUNUSED_DIPLOC(0x70, 0x70, "E21:2,4,6")
INPUT_PORTS_END

const tiny_rom_entry *labtam_z80sbc_device::device_rom_region() const
{
	return ROM_NAME(labtam_z80sbc);
}

ioport_constructor labtam_z80sbc_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(labtam_z80sbc);
}

void labtam_z80sbc_device::device_resolve_objects()
{
	// TODO: Multibus interrupt lines may optionally be wire-wrapped to
	// Am9517 interrupt request inputs 0, 1, 3 or 7.
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
		u32 const ram0_select = m_e15[4]->read();
		u32 const ram1_select = m_e15[3]->read();

		m_bus->space(AS_PROGRAM).install_ram(ram0_select, ram0_select | 0xffff, m_ram0.get());
		m_bus->space(AS_PROGRAM).install_ram(ram1_select, ram1_select | 0xffff, m_ram0.get());

		u16 const pio_select = m_e15[2]->read() | m_e15[1]->read() | m_e15[0]->read();

		m_bus->space(AS_IO).install_write_handler(pio_select | 0, pio_select | 0, write8smo_delegate(*this, FUNC(labtam_z80sbc_device::fdcclr_w)));
		m_bus->space(AS_IO).install_write_handler(pio_select | 2, pio_select | 2, write8smo_delegate(*this, FUNC(labtam_z80sbc_device::netclr_w)));
		m_bus->space(AS_IO).install_write_handler(pio_select | 4, pio_select | 4, write8smo_delegate(*this, FUNC(labtam_z80sbc_device::fdcattn_w)));
		m_bus->space(AS_IO).install_read_handler(pio_select | 8, pio_select | 8, read8smo_delegate(*this, FUNC(labtam_z80sbc_device::fdcstatus_r)));

		m_installed = true;
	}

	m_map_enabled = false;
	m_map_num = 0;

	m_dma[0]->iei_w(1);
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

	/*
	 * While not a full Z80 daisy chain, the Z80DMAs, Z80SIO and Am9519 have
	 * their EI/EO lines connected such that only one will assert the Z80
	 * /INT line at once.
	 */

	Z80DMA(config, m_dma[0], 20_MHz_XTAL / 4);
	m_dma[0]->out_int_callback().set_inputline(m_cpu, INPUT_LINE_IRQ0);
	m_dma[0]->out_ieo_callback().set(m_dma[1], FUNC(z80dma_device::iei_w));
	m_dma[0]->out_busreq_callback().set(m_dma[0], FUNC(z80dma_device::bai_w));
	m_dma[0]->in_mreq_callback().set(FUNC(labtam_z80sbc_device::map_r<7>));
	m_dma[0]->out_mreq_callback().set(FUNC(labtam_z80sbc_device::map_w<7>));
	m_dma[0]->in_iorq_callback().set(m_fdc, FUNC(wd2793_device::data_r));
	m_dma[0]->out_iorq_callback().set(m_fdc, FUNC(wd2793_device::data_w));

	Z80DMA(config, m_dma[1], 20_MHz_XTAL / 4);
	m_dma[1]->out_int_callback().set_inputline(m_cpu, INPUT_LINE_IRQ0);
	//m_dma[1]->out_ieo_callback().set(m_sio, FUNC(z80sio_device::iei_w));
	m_dma[1]->out_busreq_callback().set(m_dma[1], FUNC(z80dma_device::bai_w));
	m_dma[1]->in_mreq_callback().set(FUNC(labtam_z80sbc_device::map_r<7>));
	m_dma[1]->out_mreq_callback().set(FUNC(labtam_z80sbc_device::map_w<7>));

	// TODO: implement iei/ieo on z80sio
	Z80SIO(config, m_sio, 20_MHz_XTAL / 4);
	//m_sio->out_int_callback().set_inputline(m_cpu, INPUT_LINE_IRQ0);
	//m_sio->out_ieo_callback().set(m_uic, FUNC(am9519_device::iei_w));

	// TODO: implement iei/ieo on am9519
	AM9519(config, m_uic);
	m_uic->out_int_callback().set_inputline(m_cpu, INPUT_LINE_IRQ0);

	WD2793(config, m_fdc, 2'000'000);
	m_fdc->intrq_wr_callback().set(m_uic, FUNC(am9519_device::ireq2_w));
	m_fdc->intrq_wr_callback().append(FUNC(labtam_z80sbc_device::int_w<3>));
	m_fdc->drq_wr_callback().set(m_dma[0], FUNC(z80dma_device::rdy_w));

	// WD1002 irq -> Am9519 ireq3

	AM9513(config, m_stc, 4'000'000);
	m_stc->out4_cb().set(m_uic, FUNC(am9519_device::ireq5_w));

	MM58167(config, m_rtc, 32.768_kHz_XTAL);
	m_rtc->irq().set(m_uic, FUNC(am9519_device::ireq6_w));

	FLOPPY_CONNECTOR(config, m_fdd[0], z80sbc_floppies, "dsdd8", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_fdd[1], z80sbc_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_fdd[2], z80sbc_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_fdd[3], z80sbc_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
}

void labtam_z80sbc_device::cpu_mem(address_map &map)
{
	/*
	 * Resident Bus Decoding
	 * 2000..27ff  sram
	 * 4000..5fff  eprom 1
	 * e000..ffff  eprom 0
	 */
	map(0x0000, 0xffff).rw(FUNC(labtam_z80sbc_device::mem_r), FUNC(labtam_z80sbc_device::mem_w));
}

void labtam_z80sbc_device::cpu_pio(address_map &map)
{
	//map(0x0000, 0x0000); // TODO: fdcset: set fdc interrupt
	//map(0x0008, 0x0008); // TODO: serset: set sio interrupt

	map(0x0010, 0x0010).select(0xff00).lw8([this](offs_t offset, u8 data) { m_map_lo[offset >> 8] = data; }, "mapwr0");
	map(0x0018, 0x0018).select(0xff00).lw8([this](offs_t offset, u8 data) { m_map_hi[offset >> 8] = data & 0xf0; }, "mapwr1");
	map(0x0020, 0x0020).mirror(0xff00).lw8([this](u8 data) { m_map_enabled = true; LOG("intswt 0x%02x mapnum 0x%02x (%s)\n", data, m_map_num, machine().describe_context()); }, "intswt");
	map(0x0028, 0x0028).mirror(0xff00).lw8([this](u8 data) { m_map_enabled = true; m_map_num = data & 0x0f; LOG("mapnum 0x%02x (%s)\n", data, machine().describe_context()); }, "mapnum");

	map(0x0030, 0x0037).mirror(0xff00).w(FUNC(labtam_z80sbc_device::drive_w));
	map(0x0038, 0x0038).mirror(0xff00).lw8([this](u8 data) { LOG("reset drive fault\n"); }, "fltrest");
	map(0x0040, 0x0043).mirror(0xff00).rw(m_fdc, FUNC(wd2793_device::read), FUNC(wd2793_device::write));
	map(0x0048, 0x004b).mirror(0xff00).rw(m_sio, FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
	map(0x0050, 0x0051).mirror(0xff00).rw(m_stc, FUNC(am9513_device::read8), FUNC(am9513_device::write8));
	map(0x0058, 0x0058).mirror(0xff00).rw(m_uic, FUNC(am9519_device::data_r), FUNC(am9519_device::data_w));
	map(0x0059, 0x0059).mirror(0xff00).rw(m_uic, FUNC(am9519_device::stat_r), FUNC(am9519_device::cmd_w));

	//map(0x0060, 0x0067); // TODO: wd1001

	map(0x0068, 0x0068).mirror(0xff00).r(FUNC(labtam_z80sbc_device::drvstatus_r));

	map(0x0070, 0x0070).select(0xff00).lr8([this](offs_t offset) { return m_map_lo[offset >> 8]; }, "maprd0");
	map(0x0078, 0x0078).select(0xff00).lr8([this](offs_t offset) { return m_map_hi[offset >> 8] | m_map_num; }, "maprd1");

	map(0x0080, 0x0080).mirror(0xff00).rw(m_dma[0], FUNC(z80dma_device::read), FUNC(z80dma_device::write));
	map(0x00a0, 0x00a0).mirror(0xff00).rw(m_dma[1], FUNC(z80dma_device::read), FUNC(z80dma_device::write));
	map(0x00e0, 0x00ff).mirror(0xff00).rw(m_rtc, FUNC(mm58167_device::read), FUNC(mm58167_device::write));
}

u8 labtam_z80sbc_device::mem_r(offs_t offset)
{
	if (m_map_enabled)
		return map_r(m_map_num, offset);
	else
		// Theory: when mapper is deactivated, its outputs are all forced high, resulting
		// in set RESB|MEM|WP flags and resident bus address |= 0xf800.
		return m_eprom[0][0x1800 | (offset & 0x7ff)];
}

void labtam_z80sbc_device::mem_w(offs_t offset, u8 data)
{
	if (m_map_enabled)
		map_w(m_map_num, offset, data);
	else
		LOG("mem_w unmapped offset 0x%04x data 0x%02x (%s)\n", offset, data, machine().describe_context());
}

u8 labtam_z80sbc_device::map_r(unsigned map_num, offs_t offset)
{
	u8 const entry = ((offset >> 8) & 0xf8) | (map_num & 0x07);
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
			LOG("map_r hi 0x%02x lo 0x%02x address 0x%05x (%s)\n", hi, lo, address, machine().describe_context());
			return 0;
		}
	}
	else
		return m_bus->space((hi & MAPWR1_MEM) ? AS_PROGRAM : AS_IO).read_byte(address);
}

void labtam_z80sbc_device::map_w(unsigned map_num, offs_t offset, u8 data)
{
	u8 const entry = ((offset >> 8) & 0xf8) | (map_num & 0x07);
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
			LOG("map_w hi 0x%02x lo 0x%02x address 0x%05x data 0x%02x (%s)\n", hi, lo, address, data, machine().describe_context());
			break;
		}
	}
	else
		m_bus->space((hi & MAPWR1_MEM) ? AS_PROGRAM : AS_IO).write_byte(address, data);
}

void labtam_z80sbc_device::drive_w(offs_t offset, u8 data)
{
	switch (offset)
	{
	case 0:
	case 1:
	case 2:
	case 3:
		if (BIT(data, 0))
		{
			LOG("drive_w select drive %d (%s)\n", offset, machine().describe_context());
			m_drive = offset;
			m_fdc->set_floppy(m_fdd[*m_drive]->get_device());
		}
		else
		{
			m_drive.reset();
			m_fdc->set_floppy(nullptr);
		}
		break;
	case 4:
		m_fdc->dden_w(BIT(data, 0));
		break;
	case 5:
		// FIXME: make side select persistent
		if (m_drive)
		{
			LOG("drive_w select side %d (%s)\n", BIT(data, 0), machine().describe_context());
			m_fdd[*m_drive]->get_device()->ss_w(BIT(data, 0));
		}
		break;
	case 6:
		// TODO: precomp
		break;
	case 7:
		LOG("drive_w mini-floppy %s (%s)\n", BIT(data, 0) ? "disable" : "enable", machine().describe_context());
		m_fdc->enmf_w(BIT(data, 0));
		break;
	}
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
	m_uic->ireq4_w(1);
}

u8 labtam_z80sbc_device::fdcstatus_r()
{
	LOG("fdcstatus_r (%s)\n", machine().describe_context());
	return 0x3c;
}

u8 labtam_z80sbc_device::drvstatus_r()
{
	u8 data = m_e21->read();

	// TODO: read selected floppy twosid_r()
#if 0
	floppy_image_device *fid = m_fdd[*m_drive]->get_device();
	if (fid && !fid->twosid_r())
		data |= DRVSTATUS_SS;
#endif

	return data;
}
